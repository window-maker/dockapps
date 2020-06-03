/* rewrite of the IMAP code by Neil Spring
 * (nspring@cs.washington.edu) to support gnutls and
 * persistent connections to servers.  */

/* Originally written by Yong-iL Joh (tolkien@mizi.com),
 * modified by Jorge Garcia (Jorge.Garcia@uv.es), and
 * modified by Jay Francis (jtf@u880.org) to support
 * CRAM-MD5 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Client.h"
#include "charutil.h"
#include "tlsComm.h"
#include "passwordMgr.h"
#include "regulo.h"
#include "MessageList.h"

#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <strings.h>

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#define	PCU	(pc->u).pop_imap

extern int Relax;

#define IMAP_DM(pc, lvl, args...) DM(pc, lvl, "imap4: " args)

#ifdef HAVE_MEMFROB
#define DEFROB(x) memfrob(x, x ## _len)
#define ENFROB(x) memfrob(x, x ## _len)
#else
#define DEFROB(x)
#define ENFROB(x)
#endif

/* this array maps server:port pairs to file descriptors, so
   that when more than one mailbox is queried from a server,
   we only use one socket.  It's limited in size by the
   number of different mailboxes displayed. */
#define FDMAP_SIZE 5
static struct fdmap_struct {
	char *user_server_port;		/* tuple, in string form */
	/*@owned@ */ struct connection_state *cs;
} fdmap[FDMAP_SIZE];

static void ask_user_for_password( /*@notnull@ */ Pop3 *pc,
								  int bFlushCache);

/* authentication callbacks */
#ifdef HAVE_GCRYPT_H
static int authenticate_md5( /*@notnull@ */ Pop3 *pc,
							struct connection_state *scs,
							const char *capabilities);
#endif
static int authenticate_plaintext( /*@notnull@ */ Pop3 *pc,
								  struct connection_state *scs,
								  const char *capabilities);

/* the auth_methods array maps authentication identifiers
   to the callback that will attempt to authenticate */
static struct imap_authentication_method {
	const char *name;
	/* callback returns 1 if successful, 0 if failed */
	int (*auth_callback) ( /*@notnull@ */ Pop3 *pc,
						 struct connection_state * scs,
						 const char *capabilities);
} auth_methods[] = {
	{
#ifdef HAVE_GCRYPT_H
	"cram-md5", authenticate_md5}, {
#endif
	"plaintext", authenticate_plaintext}, {
	NULL, NULL}
};


/* recover a socket from the connection cache */
/*@null@*/
/*@dependent@*/
static struct connection_state *state_for_pcu(Pop3 *pc)
{
	char *connection_id;
	struct connection_state *retval = NULL;
	int i;
	connection_id =
		malloc(strlen(PCU.userName) + strlen(PCU.serverName) + 22);
	sprintf(connection_id, "%s|%s|%d", PCU.userName, PCU.serverName,
			PCU.serverPort);
	for (i = 0; i < FDMAP_SIZE; i++)
		if (fdmap[i].user_server_port != NULL &&
			(strcmp(connection_id, fdmap[i].user_server_port) == 0)) {
			retval = fdmap[i].cs;
		}
	free(connection_id);
	return (retval);
}

/* bind to the connection cache */
static void bind_state_to_pcu(Pop3 *pc,
							  /*@owned@ */ struct connection_state *scs)
{
	char *connection_id;
	int i;
	if (scs == NULL) {
		abort();
	}
	connection_id =
		malloc(strlen(PCU.userName) + strlen(PCU.serverName) + 22);
	sprintf(connection_id, "%s|%s|%d", PCU.userName, PCU.serverName,
			PCU.serverPort);
	for (i = 0; i < FDMAP_SIZE && fdmap[i].cs != NULL; i++);
	if (i == FDMAP_SIZE) {
		/* should never happen */
		IMAP_DM(pc, DEBUG_ERROR,
				"Tried to open too many IMAP connections. Sorry!\n");
		exit(EXIT_FAILURE);
	}
	fdmap[i].user_server_port = connection_id;
	fdmap[i].cs = scs;
}

/* remove from the connection cache */
static
/*@owned@*/
/*@null@*/
struct connection_state *unbind(
/*@returned@*/ struct connection_state
								   *scs)
{
	int i;
	struct connection_state *retval = NULL;
	assert(scs != NULL);

	for (i = 0; i < FDMAP_SIZE && fdmap[i].cs != scs; i++);
	if (i < FDMAP_SIZE) {
		free(fdmap[i].user_server_port);
		fdmap[i].user_server_port = NULL;
		retval = fdmap[i].cs;
		fdmap[i].cs = NULL;
	}
	return (retval);
}

/* creates a connection to the server, if a matching one doesn't exist. */
/* *always* returns null, just declared this wasy to match other protocols. */
/*@null@*/
FILE *imap_open(Pop3 *pc)
{
	static int complained_already;	/* we have to succeed once before
									   complaining again about failure */
	struct connection_state *scs;
	struct imap_authentication_method *a;
	char *connection_name;
	int sd;
	char capabilities[BUF_SIZE];
	char buf[BUF_SIZE];


	if (state_for_pcu(pc) != NULL) {
		/* don't need to open. */
		return NULL;
	}

	/* got this far; we're going to create a connection_state
	   structure, although it might be a blacklist entry */
	connection_name = malloc(strlen(PCU.serverName) + 20);
	sprintf(connection_name, "%s:%d", PCU.serverName, PCU.serverPort);

	assert(pc != NULL);

	/* no cached connection */
	sd = sock_connect((const char *) PCU.serverName, PCU.serverPort);
	if (sd == -1) {
		if (complained_already == 0) {
			IMAP_DM(pc, DEBUG_ERROR, "Couldn't connect to %s:%d: %s\n",
					PCU.serverName, PCU.serverPort,
					errno ? strerror(errno) : "");
			complained_already = 1;
		}
		if (errno == ETIMEDOUT) {
			/* temporarily bump the interval, in a crude way:
			   fast forward time so that the mailbox isn't
			   checked for a while. */
			pc->prevtime = time(0) + 60 * 5;	/* now + 60 seconds per min * 5 minutes */
			/* TCP's retry (how much time has elapsed while
			   the connect times out) is around 3 minutes;
			   here we just try to allow checking local
			   mailboxes more often while remote things are
			   unavailable or disconnected.  */
		}
		free(connection_name);
		return NULL;
	}

	/* build the connection using STARTTLS */
	if (PCU.dossl != 0 && (PCU.serverPort == 143)) {
		/* setup an unencrypted binding long enough to invoke STARTTLS */
		scs = initialize_unencrypted(sd, connection_name, pc);

		/* can we? */
		tlscomm_printf(scs, "a000 CAPABILITY\r\n");
		if (tlscomm_expect(scs, "* CAPABILITY", capabilities, BUF_SIZE) ==
			0)
			goto communication_failure;

		if (!strstr(capabilities, "STARTTLS")) {
			IMAP_DM(pc, DEBUG_ERROR,
					"server doesn't support ssl imap on port 143.");
			goto communication_failure;
		}

		/* we sure can! */
		IMAP_DM(pc, DEBUG_INFO, "Negotiating TLS within IMAP");
		tlscomm_printf(scs, "a001 STARTTLS\r\n");

		if (tlscomm_expect(scs, "a001 ", buf, BUF_SIZE) == 0)
			goto communication_failure;

		if (strstr(buf, "a001 OK") == 0) {
			/* we didn't see the success message in the response */
			IMAP_DM(pc, DEBUG_ERROR, "couldn't negotiate tls. :(\n");
			goto communication_failure;
		}

		/* we don't need the unencrypted state anymore */
		/* note that communication_failure will close the
		   socket and free via tls_close() */
		free(scs);				/* fall through will scs = initialize_gnutls(sd); */
	}

	/* either we've negotiated ssl from starttls, or
	   we're starting an encrypted connection now */
	if (PCU.dossl != 0) {
		scs = initialize_gnutls(sd, connection_name, pc, PCU.serverName);
		if (scs == NULL) {
			IMAP_DM(pc, DEBUG_ERROR, "Failed to initialize TLS\n");
			return NULL;
		}
	} else {
		scs = initialize_unencrypted(sd, connection_name, pc);
	}

	/* authenticate; first find out how */
	/* note that capabilities may have changed since last
	   time we may have asked, if we called STARTTLS, my
	   server will allow plain password login within an
	   encrypted session. */
	tlscomm_printf(scs, "a000 CAPABILITY\r\n");
	if (tlscomm_expect(scs, "* CAPABILITY", capabilities, BUF_SIZE) == 0) {
		IMAP_DM(pc, DEBUG_ERROR, "unable to query capability string\n");
		goto communication_failure;
	}

	/* try each authentication method in turn. */
	for (a = auth_methods; a->name != NULL; a++) {
		/* was it specified or did the user leave it up to us? */
		if (PCU.authList[0] == '\0'
			|| strstr(PCU.authList, a->name) != NULL)
			/* try the authentication method */
			if ((a->auth_callback(pc, scs, capabilities)) != 0) {
				/* store this well setup connection in the cache */
				bind_state_to_pcu(pc, scs);
				complained_already = 0;
				return NULL;
			}
	}

	/* if authentication worked, we won't get here */
	IMAP_DM(pc, DEBUG_ERROR,
			"All authentication methods failed for '%s@%s:%d'\n",
			PCU.userName, PCU.serverName, PCU.serverPort);
  communication_failure:
	tlscomm_printf(scs, "a002 LOGOUT\r\n");
	tlscomm_close(scs);
	return NULL;

}

void imap_cacheHeaders( /*@notnull@ */ Pop3 *pc);

int imap_checkmail( /*@notnull@ */ Pop3 *pc)
{
	/* recover connection state from the cache */
	struct connection_state *scs = state_for_pcu(pc);
	char buf[BUF_SIZE];
	char examine_expect[BUF_SIZE];
	static int command_id;

	/* if it's not in the cache, try to open */
	if (scs == NULL) {
		IMAP_DM(pc, DEBUG_INFO, "Need new connection to %s@%s\n",
				PCU.userName, PCU.serverName);
		(void) imap_open(pc);
		scs = state_for_pcu(pc);
	}
	if (scs == NULL) {
		return -1;
	}

	if (tlscomm_is_blacklisted(scs) != 0) {
		/* unresponsive server, don't bother. */
		return -1;
	}

	command_id++;
	tlscomm_printf(scs, "a%03d EXAMINE %s\r\n", command_id, pc->path);
	snprintf(examine_expect, BUF_SIZE, "a%03d OK", command_id);
	if (tlscomm_expect(scs, examine_expect, buf, 127) == 0) {
		tlscomm_close(unbind(scs));
		return -1;
	}

	command_id++;
	tlscomm_printf(scs, "a%03d CLOSE\r\n", command_id);
	snprintf(examine_expect, BUF_SIZE, "a%03d OK", command_id);
	if (tlscomm_expect(scs, examine_expect, buf, 127) == 0) {
		tlscomm_close(unbind(scs));
		return -1;
	}

	/* if we've got it by now, try the status query */
	command_id++;
	tlscomm_printf(scs, "a%03d STATUS %s (MESSAGES UNSEEN)\r\n",
				   command_id % 1000, pc->path);
	if (tlscomm_expect(scs, "* STATUS", buf, 127) != 0) {
		/* a valid response? */
		// doesn't support spaces: (void) sscanf(buf, "* STATUS %*s (MESSAGES %d UNSEEN %d)",
		const char *msg;
		msg = strstr(buf, "(MESSAGES");
		if (msg != NULL)
			(void) sscanf(msg, "(MESSAGES %d UNSEEN %d)",
						  &(pc->TotalMsgs), &(pc->UnreadMsgs));
		/* update the cached headers if evidence that change
		   has occurred; not necessarily complete. */
		if (pc->UnreadMsgs != pc->OldUnreadMsgs ||
			pc->TotalMsgs != pc->OldMsgs) {
			if (PCU.wantCacheHeaders) {
				imap_cacheHeaders(pc);
			}
		}
	} else {
		/* something went wrong. bail. */
		tlscomm_close(unbind(scs));
		return -1;
	}
	return 0;
}

void
imap_releaseHeaders(Pop3 *pc __attribute__((unused)), struct msglst *h)
{
	assert(h != NULL);
	/* allow the list to be released next time around */
	if (h->in_use <= 0) {
		/* free the old one */
		while (h != NULL) {
			struct msglst *n = h->next;
			free(h);
			h = n;
		}
	} else {
		h->in_use--;
	}
}

void imap_cacheHeaders( /*@notnull@ */ Pop3 *pc)
{
	struct connection_state *scs = state_for_pcu(pc);
	char *msgid;
	char buf[BUF_SIZE];

	if (scs == NULL) {
		(void) imap_open(pc);
		scs = state_for_pcu(pc);
	}
	if (scs == NULL) {
		return;
	}
	if (tlscomm_is_blacklisted(scs) != 0) {
		return;
	}

	if (pc->headerCache != NULL) {
		/* decrement the reference count, and free our version */
		imap_releaseHeaders(pc, pc->headerCache);
		pc->headerCache = NULL;
	}

	IMAP_DM(pc, DEBUG_INFO, "working headers\n");

	tlscomm_printf(scs, "a004 EXAMINE %s\r\n", pc->path);
	if (tlscomm_expect(scs, "a004 OK", buf, 127) == 0) {
		tlscomm_close(unbind(scs));
		return;
	}
	IMAP_DM(pc, DEBUG_INFO, "examine ok\n");

	/* if we've got it by now, try the status query */
	tlscomm_printf(scs, "a005 SEARCH UNSEEN\r\n");
	if (tlscomm_expect(scs, "* SEARCH", buf, 127) == 0) {
		tlscomm_close(unbind(scs));
		return;
	}
	IMAP_DM(pc, DEBUG_INFO, "search: %s", buf);
	if (strlen(buf) < 9)
		return;					/* search turned up nothing */
	msgid = strtok(buf + 9, " \r\n");
	pc->headerCache = NULL;
	/* the isdigit cruft is to deal with EOL */
	if (msgid != NULL && isdigit(msgid[0]))
		do {
			struct msglst *m = malloc(sizeof(struct msglst));
			char hdrbuf[128];
			int fetch_command_done = FALSE;
			tlscomm_printf(scs, "a04 FETCH %s (FLAGS "
						   "BODY[HEADER.FIELDS (FROM SUBJECT)])\r\n",
						   msgid);
			if (tlscomm_expect(scs, "* ", hdrbuf, 127)) {
				m->subj[0] = '\0';
				m->from[0] = '\0';
				while (m->subj[0] == '\0' || m->from[0] == '\0') {
					if (tlscomm_expect(scs, "", hdrbuf, 127)) {
						if (strncasecmp(hdrbuf, "Subject:", 8) == 0) {
							strncpy(m->subj, hdrbuf + 9, SUBJ_LEN - 1);
							m->subj[SUBJ_LEN - 1] = '\0';
						} else if (strncasecmp(hdrbuf, "From: ", 5) == 0) {
							strncpy(m->from, hdrbuf + 6, FROM_LEN - 1);
							m->from[FROM_LEN - 1] = '\0';
						} else if (strncasecmp
								   (hdrbuf, "a04 OK FETCH", 5) == 0) {
							/* server says we're done getting this header, which
							   may occur if the message has no subject */
							if (m->from[0] == '\0') {
								strcpy(m->from, " ");
							}
							if (m->subj[0] == '\0') {
								strcpy(m->subj, "(no subject)");
							}
							fetch_command_done = TRUE;
						}
					} else {
						IMAP_DM(pc, DEBUG_ERROR,
								"timedout looking for headers.: %s",
								hdrbuf);
						strcpy(m->from, "wmbiff");
						strcpy(m->subj, "failure");
					}
				}
				IMAP_DM(pc, DEBUG_INFO, "From: '%s' Subj: '%s'\n",
						m->from, m->subj);
				m->next = pc->headerCache;
				pc->headerCache = m;
				pc->headerCache->in_use = 0;	/* initialize that it isn't locked */
			} else {
				free(m);
				IMAP_DM(pc, DEBUG_ERROR, "error fetching: %s", hdrbuf);
			}
			if (!fetch_command_done) {
				tlscomm_expect(scs, "a04 OK", hdrbuf, 127);
			}
		}
		while ((msgid = strtok(NULL, " \r\n")) != NULL
			   && isdigit(msgid[0]));

	tlscomm_printf(scs, "a06 CLOSE\r\n");	/* return to polling state */
	/*  may be unneeded tlscomm_expect(scs, "a06 OK CLOSE\r\n" );  see if it worked? */
	IMAP_DM(pc, DEBUG_INFO, "worked headers\n");
}

/* a client is asking for the headers, hand em a reference, increase the
   one-bit reference counter */
struct msglst *imap_getHeaders( /*@notnull@ */ Pop3 *pc)
{
	if (pc->headerCache == NULL)
		imap_cacheHeaders(pc);
	if (pc->headerCache != NULL)
		pc->headerCache->in_use = 1;
	return pc->headerCache;
}

/* parse the config line to setup the Pop3 structure */
int imap4Create( /*@notnull@ */ Pop3 *pc, const char *const str)
{
	int i;
	int matchedchars;
	/* special characters aren't allowed in hostnames, rfc 1034 */
	const char *regexes[] = {
		// type : username     :   password @ hostname (/ name)?(:port)?
               ".*imaps?:([^: ]{1,255}):([^@]{0,32})@([A-Za-z1-9][-A-Za-z0-9_.]+)(/(\"[^\"]+\")|([^:@ ]+))?(:[0-9]+)?(  *([CcAaPp][-A-Za-z5 ]*))?$",
               ".*imaps?:([^: ]{1,255}) ([^ ]{1,32}) ([A-Za-z1-9][-A-Za-z0-9_.]+)(/(\"[^\"]+\")|([^: ]+))?( [0-9]+)?(  *([CcAaPp][-A-Za-z5 ]*))?$",
		NULL
	};
	char *unaliased_str;
	/*
	 * regulo_atoi expects a pointer-to-int and pop_imap.serverPort is a
	 * uint16_t, so &pop_imap.serverPort is not compatible and we need to use an
	 * int temporary variable to avoid endianness problems.
	 */
	int serverPort;

	struct regulo regulos[] = {
		{1, PCU.userName, regulo_strcpy},
		{2, PCU.password, regulo_strcpy},
		{3, PCU.serverName, regulo_strcpy},
		{4, pc->path, regulo_strcpy_skip1},
		{7, &serverPort, regulo_atoi},
		{9, PCU.authList, regulo_strcpy_tolower},
		{0, NULL, NULL}
	};

	if (Relax) {
		regexes[0] =
			".*imaps?:([^: ]{1,256}):([^@]{0,32})@([^/: ]+)(/(\"[^\"]+\")|([^:@ ]+))?(:[0-9]+)?(  *(.*))?$";
		regexes[1] =
			".*imaps?:([^: ]{1,256}) ([^ ]{1,32}) ([^/: ]+)(/(\"[^\"]+\")|([^: ]+))?( [0-9]+)?(  *(.*))?$";
	}


	/* IMAP4 format: imap:user:password@server/mailbox[:port] */
	/* If 'str' line is badly formatted, wmbiff won't display the mailbox. */
	if (strncmp("sslimap:", str, 8) == 0 || strncmp("imaps:", str, 6) == 0) {
#ifdef HAVE_GNUTLS_GNUTLS_H
		PCU.dossl = 1;
#else
		printf("This copy of wmbiff was not compiled with gnutls;\n"
			   "imaps is unavailable.  Exiting to protect your\n"
			   "passwords and privacy.\n");
		exit(EXIT_FAILURE);
#endif
	} else {
		PCU.dossl = 0;
	}

	/* defaults */
	serverPort = (PCU.dossl != 0) ? 993 : 143;
	PCU.authList[0] = '\0';

	/* argh, str and pc->path are aliases, so we can't just write the default
	   value into the string we're about to parse. */
	unaliased_str = strdup(str);
	strcpy(pc->path, "INBOX");

	for (matchedchars = 0, i = 0;
		 regexes[i] != NULL && matchedchars <= 0; i++) {
		matchedchars = regulo_match(regexes[i], unaliased_str, regulos);
	}

	/* failed to match either regex */
	if (matchedchars <= 0) {
		pc->label[0] = '\0';
		IMAP_DM(pc, DEBUG_ERROR, "Couldn't parse line %s (%d)\n"
				"  If this used to work, run wmbiff with the -relax option, and\n"
				"  send mail to "PACKAGE_BUGREPORT" with the hostname\n"
				"  of your mail server.\n", unaliased_str, matchedchars);
		return -1;
	}

	PCU.serverPort = serverPort;

	PCU.password_len = strlen(PCU.password);
	if (PCU.password[0] == '\0') {
		PCU.interactive_password = 1;
	} else {
		ENFROB(PCU.password);
	}

	// grab_authList(unaliased_str + matchedchars, PCU.authList);

	free(unaliased_str);

	IMAP_DM(pc, DEBUG_INFO, "userName= '%s'\n", PCU.userName);
	IMAP_DM(pc, DEBUG_INFO, "password is %zu characters long\n",
			PCU.password_len);
	IMAP_DM(pc, DEBUG_INFO, "serverName= '%s'\n", PCU.serverName);
	IMAP_DM(pc, DEBUG_INFO, "serverPath= '%s'\n", pc->path);
	IMAP_DM(pc, DEBUG_INFO, "serverPort= '%d'\n", PCU.serverPort);
	IMAP_DM(pc, DEBUG_INFO, "authList= '%s'\n", PCU.authList);

	if (strcmp(pc->action, "msglst") == 0 ||
		strcmp(pc->fetchcmd, "msglst") == 0 ||
		strcmp(pc->button2, "msglst") == 0) {
		PCU.wantCacheHeaders = 1;
	} else {
		PCU.wantCacheHeaders = 0;
	}
	pc->checkMail = imap_checkmail;
	pc->getHeaders = imap_getHeaders;
	pc->releaseHeaders = imap_releaseHeaders;
	pc->TotalMsgs = 0;
	pc->UnreadMsgs = 0;
	pc->OldMsgs = -1;
	pc->OldUnreadMsgs = -1;
	return 0;
}

static int authenticate_plaintext( /*@notnull@ */ Pop3 *pc,
								  struct connection_state *scs,
								  const char *capabilities)
{
	char buf[BUF_SIZE];
	/* is login prohibited? */
	/* "An IMAP client which complies with [rfc2525, section 3.2]
	 *  MUST NOT issue the LOGIN command if this capability is present.
	 */
	if (strstr(capabilities, "LOGINDISABLED")) {
		IMAP_DM(pc, DEBUG_ERROR,
				"Plaintext auth prohibited by server: (LOGINDISABLED).\n");
		goto plaintext_failed;
	}

	ask_user_for_password(pc, 0);
	do {
		/* login */
		DEFROB(PCU.password);
		tlscomm_printf(scs, "a001 LOGIN %s \"%s\"\r\n", PCU.userName,
					   PCU.password);
		ENFROB(PCU.password);
		if (tlscomm_expect(scs, "a001 ", buf, BUF_SIZE) == 0) {
			IMAP_DM(pc, DEBUG_ERROR,
					"Did not get a response to the LOGIN command.\n");
			goto plaintext_failed;
		}

		if (buf[5] != 'O') {
			IMAP_DM(pc, DEBUG_ERROR, "IMAP Login failed: %s\n", buf);
			/* if we're prompting the user, ask again, else fail */
			if (PCU.interactive_password) {
				PCU.password[0] = '\0';
				ask_user_for_password(pc, 1);	/* 1=overwrite the cache */
			} else {
				goto plaintext_failed;
			}
		} else {
			return (1);
		}
	}
	while (1);

  plaintext_failed:
	return (0);
}

#ifdef HAVE_GCRYPT_H
static int
authenticate_md5(Pop3 *pc,
				 struct connection_state *scs, const char *capabilities)
{
	char buf[BUF_SIZE];
	char buf2[BUF_SIZE];
	unsigned char *md5;
	gcry_md_hd_t gmh;
	gcry_error_t rc;

	if (!strstr(capabilities, "AUTH=CRAM-MD5")) {
		/* server doesn't support cram-md5. */
		return 0;
	}

	tlscomm_printf(scs, "a007 AUTHENTICATE CRAM-MD5\r\n");
	if (tlscomm_expect(scs, "+ ", buf, BUF_SIZE) == 0)
		goto expect_failure;

	Decode_Base64(buf + 2, buf2);
	IMAP_DM(pc, DEBUG_INFO, "CRAM-MD5 challenge: %s\n", buf2);

	strcpy(buf, PCU.userName);
	strcat(buf, " ");
	ask_user_for_password(pc, 0);
	rc = gcry_md_open(&gmh, GCRY_MD_MD5, GCRY_MD_FLAG_HMAC);
	if (rc != 0) {
		IMAP_DM(pc, DEBUG_INFO, "unable to initialize gcrypt md5\n");
		return 0;
	}
	DEFROB(PCU.password);
	gcry_md_setkey(gmh, PCU.password, strlen(PCU.password));
	ENFROB(PCU.password);
	gcry_md_write(gmh, (unsigned char *) buf2, strlen(buf2));
	gcry_md_final(gmh);
	md5 = gcry_md_read(gmh, 0);
	Bin2Hex(md5, 16, buf2);
	gcry_md_close(gmh);

	strcat(buf, buf2);
	IMAP_DM(pc, DEBUG_INFO, "CRAM-MD5 response: %s\n", buf);
	Encode_Base64(buf, buf2);

	tlscomm_printf(scs, "%s\r\n", buf2);
	if (tlscomm_expect(scs, "a007 ", buf, BUF_SIZE) == 0)
		goto expect_failure;

	if (!strncmp(buf, "a007 OK", 7))
		return 1;				/* AUTH successful */

	IMAP_DM(pc, DEBUG_ERROR,
			"CRAM-MD5 AUTH failed for user '%s@%s:%d'\n",
			PCU.userName, PCU.serverName, PCU.serverPort);
	IMAP_DM(pc, DEBUG_INFO, "It said %s", buf);
	return 0;

  expect_failure:
	IMAP_DM(pc, DEBUG_ERROR,
			"tlscomm_expect failed during cram-md5 auth: %s", buf);
	IMAP_DM(pc, DEBUG_ERROR, "failed to authenticate using cram-md5.");
	return 0;
}
#endif

static void ask_user_for_password( /*@notnull@ */ Pop3 *pc, int bFlushCache)
{
	/* see if we already have a password, as provided in the config file, or
	   already requested from the user. */
	if (PCU.interactive_password) {
		if (strlen(PCU.password) == 0) {
			/* we need to grab the password from the user. */
			char *password;
			IMAP_DM(pc, DEBUG_INFO, "asking for password %d\n",
					bFlushCache);
			password =
				passwordFor(PCU.userName, PCU.serverName, pc, bFlushCache);
			if (password != NULL) {
				if (strlen(password) + 1 > BUF_SMALL) {
					DMA(DEBUG_ERROR, "Password is too long.\n");
					memset(PCU.password, 0, BUF_SMALL - 1);
				} else {
					strncpy(PCU.password, password, BUF_SMALL - 1);
					PCU.password_len = strlen(PCU.password);
				}
				free(password);
				ENFROB(PCU.password);
			}
		}
	}
}

/* vim:set ts=4: */
/*
 * Local Variables:
 * tab-width: 4
 * c-indent-level: 4
 * c-basic-offset: 4
 * End:
 */
