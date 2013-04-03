/* $Id: Pop3Client.c,v 1.23 2004/12/12 00:01:53 bluehal Exp $ */
/* Author : Scott Holden ( scotth@thezone.net )
   Modified : Yong-iL Joh ( tolkien@mizi.com )
   Modified : Jorge García ( Jorge.Garcia@uv.es )
   Modified ; Mark Hurley ( debian4tux@telocity.com )
   Modified : Neil Spring ( nspring@cs.washington.edu )
 * 
 * Pop3 Email checker.
 *
 * Last Updated : Tue Nov 13 13:45:23 PST 2001
 *
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Client.h"
#include "charutil.h"
#include "regulo.h"
#include "MessageList.h"
#include <strings.h>
#include "tlsComm.h"
#include "passwordMgr.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

extern int Relax;
/* temp */
static void ask_user_for_password( /*@notnull@ */ Pop3 pc, int bFlushCache);

#define	PCU	(pc->u).pop_imap
#define POP_DM(pc, lvl, args...) DM(pc, lvl, "pop3: " args)

#ifdef HAVE_GCRYPT_H
static struct connection_state *authenticate_md5( /*@notnull@ */ Pop3 pc, struct connection_state * scs,
							  char *unused);
static struct connection_state *authenticate_apop( /*@notnull@ */ Pop3 pc, struct connection_state * scs,
							   char *apop_str);
#endif
static struct connection_state *authenticate_plaintext( /*@notnull@ */ Pop3 pc, struct connection_state * scs,
									char *unused);

void pop3_cacheHeaders( /*@notnull@ */ Pop3 pc);

extern void imap_releaseHeaders(Pop3 pc
								__attribute__ ((unused)),
								struct msglst *h);

extern struct connection_state *state_for_pcu(Pop3 pc);

static struct authentication_method {
	const char *name;
	/* callback returns the connection state pointer if successful, 
	   NULL if failed */
	struct connection_state  *(*auth_callback) (Pop3 pc, struct connection_state * scs, char *apop_str);
} auth_methods[] = {
	{
#ifdef HAVE_GCRYPT_H
	"cram-md5", authenticate_md5}, {
	"apop", authenticate_apop}, {
#endif
	"plaintext", authenticate_plaintext}, {
	NULL, NULL}
};

/*@null@*/
struct connection_state *pop3Login(Pop3 pc)
{
	int fd;
	char buf[BUF_SIZE];
	char apop_str[BUF_SIZE];
	char *ptr1, *ptr2;
	struct authentication_method *a;
	struct connection_state *scs;
	char *connection_name;


	apop_str[0] = '\0';			/* if defined, server supports apop */

	if ((fd = sock_connect(PCU.serverName, PCU.serverPort)) == -1) {
		POP_DM(pc, DEBUG_ERROR, "Not Connected To Server '%s:%d'\n",
			   PCU.serverName, PCU.serverPort);
		return NULL;
	}

	connection_name = malloc(strlen(PCU.serverName) + 20);
	sprintf(connection_name, "%s:%d", PCU.serverName, PCU.serverPort);

	if (PCU.dossl != 0) {
		scs = initialize_gnutls(fd, connection_name, pc, PCU.serverName);
		if (scs == NULL) {
			POP_DM(pc, DEBUG_ERROR, "Failed to initialize TLS\n");
			return NULL;
		}
	} else {
		scs = initialize_unencrypted(fd, connection_name, pc);
	}

    tlscomm_gets(buf, BUF_SIZE, scs);
	POP_DM(pc, DEBUG_INFO, "%s", buf);

	/* Detect APOP, copy challenge into apop_str */
	for (ptr1 = buf + strlen(buf), ptr2 = NULL; ptr1 > buf; --ptr1) {
		if (*ptr1 == '>') {
			ptr2 = ptr1;
		} else if (*ptr1 == '<') {
			if (ptr2) {
				*(ptr2 + 1) = 0;
				strncpy(apop_str, ptr1, BUF_SIZE);
			}
			break;
		}
	}


	/* try each authentication method in turn. */
	for (a = auth_methods; a->name != NULL; a++) {
		/* was it specified or did the user leave it up to us? */
		if (PCU.authList[0] == '\0' || strstr(PCU.authList, a->name))
			/* did it work? */
			if ((a->auth_callback(pc, scs, apop_str)) != NULL)
				return (scs);
	}

	/* if authentication worked, we won't get here */
	POP_DM(pc, DEBUG_ERROR,
		   "All Pop3 authentication methods failed for '%s@%s:%d'\n",
		   PCU.userName, PCU.serverName, PCU.serverPort);
    tlscomm_printf(scs, "QUIT\r\n");
    tlscomm_close(scs);

	return NULL;
}

int pop3CheckMail( /*@notnull@ */ Pop3 pc)
{
	struct connection_state *scs;
	int read;
	char buf[BUF_SIZE];

	scs = pop3Login(pc);
	if (scs == NULL)
		return -1;

	tlscomm_printf(scs, "STAT\r\n");
	if( ! tlscomm_expect(scs, "+", buf, BUF_SIZE) ) {
		POP_DM(pc, DEBUG_ERROR,
			   "Error Receiving Stats '%s@%s:%d'\n",
			   PCU.userName, PCU.serverName, PCU.serverPort);
		POP_DM(pc, DEBUG_INFO, "It said: %s\n", buf);
		return -1;
	} else {
		sscanf(buf, "+OK %d", &(pc->TotalMsgs));
	}

	/*  - Updated - Mark Hurley - debian4tux@telocity.com
	 *  In compliance with RFC 1725
	 *  which removed the LAST command, any servers
	 *  which follow this spec will return:
	 *      -ERR unimplimented
	 *  We will leave it here for those servers which haven't
	 *  caught up with the spec.
	 */
    tlscomm_printf(scs, "LAST\r\n");
	tlscomm_gets(buf, BUF_SIZE, scs);
	if (buf[0] != '+') {
		/* it is not an error to receive this according to RFC 1725 */
		/* no error should be returned */
		pc->UnreadMsgs = pc->TotalMsgs;
        // there's also a LIST command... not sure how to make use of it. */ 
	} else {
		sscanf(buf, "+OK %d", &read);
		pc->UnreadMsgs = pc->TotalMsgs - read;
	}

	tlscomm_printf(scs, "QUIT\r\n");
	tlscomm_close(scs);

	return 0;
}


struct msglst *pop_getHeaders( /*@notnull@ */ Pop3 pc)
{
	if (pc->headerCache == NULL)
		pop3_cacheHeaders(pc);
	if (pc->headerCache != NULL)
		pc->headerCache->in_use = 1;
	return pc->headerCache;
}



int pop3Create(Pop3 pc, const char *str)
{
	/* POP3 format: pop3:user:password@server[:port] */
	/* new POP3 format: pop3:user password server [port] */
	/* If 'str' line is badly formatted, wmbiff won't display the mailbox. */
	int i;
	int matchedchars;
	/* ([^: ]+) user
	   ([^@]+) or ([^ ]+) password 
	   ([^: ]+) server 
	   ([: ][0-9]+)? optional port 
	   ' *' gobbles trailing whitespace before authentication types.
	   use separate regexes for old and new types to permit
	   use of '@' in passwords
	 */
	const char *regexes[] = {
		"pop3s?:([^: ]{1,32}):([^@]{0,32})@([A-Za-z1-9][-A-Za-z0-9_.]+)(:[0-9]+)?(  *([CcAaPp][-A-Za-z5 ]*))?$",
		"pop3s?:([^: ]{1,32}) ([^ ]{1,32}) ([A-Za-z1-9][-A-Za-z0-9_.]+)( [0-9]+)?(  *([CcAaPp][-A-Za-z5 ]*))?$",
		//      "pop3:([^: ]{1,32}) ([^ ]{1,32}) ([^: ]+)( [0-9]+)? *",
		// "pop3:([^: ]{1,32}):([^@]{0,32})@([^: ]+)(:[0-9]+)? *",
		NULL
	};
	struct regulo regulos[] = {
		{1, PCU.userName, regulo_strcpy},
		{2, PCU.password, regulo_strcpy},
		{3, PCU.serverName, regulo_strcpy},
		{4, &PCU.serverPort, regulo_atoi},
		{6, PCU.authList, regulo_strcpy_tolower},
		{0, NULL, NULL}
	};

	if (Relax) {
		regexes[0] =
			"pop3:([^: ]{1,32}):([^@]{0,32})@([^/: ]+)(:[0-9]+)?(  *(.*))?$";
		regexes[1] =
			"pop3:([^: ]{1,32}) ([^ ]{1,32}) ([^/: ]+)( [0-9]+)?(  *(.*))?$";
	}

	if (strncmp("pop3s:", str, 6) == 0) {
#ifdef HAVE_GNUTLS_GNUTLS_H
		static int haveBeenWarned;
		PCU.dossl = 1;
		if (!haveBeenWarned) {
			printf("wmbiff uses gnutls for TLS/SSL encryption support:\n"
				   "  If you distribute software that uses gnutls, don't forget\n"
				   "  to warn the users of your software that gnutls is at a\n"
				   "  testing phase and may be totally insecure.\n"
				   "\nConsider yourself warned.\n");
			haveBeenWarned = 1;
		}
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
	PCU.serverPort = (PCU.dossl != 0) ? 995 : 110;
	PCU.authList[0] = '\0';

	for (matchedchars = 0, i = 0;
		 regexes[i] != NULL && matchedchars <= 0; i++) {
		matchedchars = regulo_match(regexes[i], str, regulos);
	}

	/* failed to match either regex */
	if (matchedchars <= 0) {
		pc->label[0] = '\0';
		POP_DM(pc, DEBUG_ERROR, "Couldn't parse line %s (%d)\n"
			   "  If this used to work, run wmbiff with the -relax option, and\n "
			   "  send mail to wmbiff-devel@lists.sourceforge.net with the hostname\n"
			   "  of your mail server.\n", str, matchedchars);
		return -1;
	}
	// grab_authList(str + matchedchars, PCU.authList);

	PCU.password_len = strlen(PCU.password);
	if (PCU.password[0] == '\0') {
		PCU.interactive_password = 1;
	} else {
      // ENFROB(PCU.password);
	}

	POP_DM(pc, DEBUG_INFO, "userName= '%s'\n", PCU.userName);
	POP_DM(pc, DEBUG_INFO, "password is %ld chars long\n",
		   strlen(PCU.password));
	POP_DM(pc, DEBUG_INFO, "serverName= '%s'\n", PCU.serverName);
	POP_DM(pc, DEBUG_INFO, "serverPort= '%d'\n", PCU.serverPort);
	POP_DM(pc, DEBUG_INFO, "authList= '%s'\n", PCU.authList);

	pc->checkMail = pop3CheckMail;
	pc->getHeaders = pop_getHeaders;
	pc->TotalMsgs = 0;
	pc->UnreadMsgs = 0;
	pc->OldMsgs = -1;
	pc->OldUnreadMsgs = -1;

	return 0;
}


#ifdef HAVE_GCRYPT_H
static struct connection_state *authenticate_md5(Pop3 pc, struct connection_state * scs, char *apop_str
							  __attribute__ ((unused)))
{
	char buf[BUF_SIZE];
	char buf2[BUF_SIZE];
	unsigned char *md5;
	gcry_md_hd_t gmh;
	gcry_error_t rc;

	/* See if MD5 is supported */
	tlscomm_printf(scs, "AUTH CRAM-MD5\r\n");
	tlscomm_gets(buf, BUF_SIZE, scs);
	POP_DM(pc, DEBUG_INFO, "%s", buf);

	if (buf[0] != '+' || buf[1] != ' ') {
		/* nope, not supported. */
		return NULL;
	}

	Decode_Base64(buf + 2, buf2);
	POP_DM(pc, DEBUG_INFO, "CRAM-MD5 challenge: %s\n", buf2);

	strcpy(buf, PCU.userName);
	strcat(buf, " ");


	rc = gcry_md_open(&gmh, GCRY_MD_MD5, GCRY_MD_FLAG_HMAC);
	if (rc != 0) {
		POP_DM(pc, DEBUG_ERROR, "unable to initialize gcrypt md5.\n");
		return NULL;
	}
	gcry_md_setkey(gmh, PCU.password, strlen(PCU.password));
	gcry_md_write(gmh, (unsigned char *) buf2, strlen(buf2));
	gcry_md_final(gmh);
	md5 = gcry_md_read(gmh, 0);
	/* hmac_md5(buf2, strlen(buf2), PCU.password,
	   strlen(PCU.password), md5); */
	Bin2Hex(md5, 16, buf2);
	gcry_md_close(gmh);

	strcat(buf, buf2);
	POP_DM(pc, DEBUG_INFO, "CRAM-MD5 response: %s\n", buf);
	Encode_Base64(buf, buf2);

	tlscomm_printf(scs, "%s\r\n", buf2);
	tlscomm_gets(buf, BUF_SIZE, scs);

	if (!strncmp(buf, "+OK", 3))
		return scs;				/* AUTH successful */
	else {
		POP_DM(pc, DEBUG_ERROR,
			   "CRAM-MD5 AUTH failed for user '%s@%s:%d'\n",
			   PCU.userName, PCU.serverName, PCU.serverPort);
		fprintf(stderr, "It said %s", buf);
		return NULL;
	}
}

static struct connection_state *authenticate_apop(Pop3 pc, struct connection_state * scs, char *apop_str)
{
	gcry_md_hd_t gmh;
	gcry_error_t rc;
	char buf[BUF_SIZE];
	unsigned char *md5;


	if (apop_str[0] == '\0') {
		/* server doesn't support apop. */
		return (NULL);
	}
	POP_DM(pc, DEBUG_INFO, "APOP challenge: %s\n", apop_str);
	strcat(apop_str, PCU.password);

	rc = gcry_md_open(&gmh, GCRY_MD_MD5, 0);
	if (rc != 0) {
		POP_DM(pc, DEBUG_ERROR, "unable to initialize gcrypt md5.\n");
		return NULL;
	}
	gcry_md_write(gmh, (unsigned char *) apop_str, strlen(apop_str));
	gcry_md_final(gmh);
	md5 = gcry_md_read(gmh, 0);
	Bin2Hex(md5, 16, buf);
	gcry_md_close(gmh);

	POP_DM(pc, DEBUG_INFO, "APOP response: %s %s\n", PCU.userName, buf);
	tlscomm_printf(scs, "APOP %s %s\r\n", PCU.userName, buf);
	tlscomm_gets(buf, BUF_SIZE, scs);

	if (!strncmp(buf, "+OK", 3))
		return scs;				/* AUTH successful */
	else {
		POP_DM(pc, DEBUG_ERROR,
			   "APOP AUTH failed for user '%s@%s:%d'\n",
			   PCU.userName, PCU.serverName, PCU.serverPort);
		POP_DM(pc, DEBUG_INFO, "It said %s", buf);
		return NULL;
	}
}
#endif							/* HAVE_GCRYPT_H */

/*@null@*/
static struct connection_state *authenticate_plaintext( /*@notnull@ */ Pop3 pc,
									struct connection_state * scs, char *apop_str
									__attribute__ ((unused)))
{
	char buf[BUF_SIZE];

	tlscomm_printf(scs, "USER %s\r\n", PCU.userName);
	if (tlscomm_gets(buf, BUF_SIZE, scs) == 0) {
		POP_DM(pc, DEBUG_ERROR,
			   "Error reading from server authenticating '%s@%s:%d'\n",
			   PCU.userName, PCU.serverName, PCU.serverPort);
		return NULL;
	}
	if (buf[0] != '+') {
		POP_DM(pc, DEBUG_ERROR,
			   "Failed user name when authenticating '%s@%s:%d'\n",
			   PCU.userName, PCU.serverName, PCU.serverPort);
		/* deb #128863 might be easier if we printed: */
		POP_DM(pc, DEBUG_ERROR, "The server's error message was: %s\n",
			   buf);
		return NULL;
	};


	tlscomm_printf(scs, "PASS %s\r\n", PCU.password);
	if (tlscomm_gets(buf, BUF_SIZE, scs) == 0) {
		POP_DM(pc, DEBUG_ERROR,
			   "Error reading from server (2) authenticating '%s@%s:%d'\n",
			   PCU.userName, PCU.serverName, PCU.serverPort);
		return NULL;
	}
    if (strncmp(buf, "-ERR [AUTH] Password required", 20) == 0) {
      if (PCU.interactive_password) {
        PCU.password[0] = '\0';
        ask_user_for_password(pc, 1);	/* 1=overwrite the cache */
        tlscomm_printf(scs, "PASS %s\r\n", PCU.password);
        if (tlscomm_gets(buf, BUF_SIZE, scs) == 0) {
          POP_DM(pc, DEBUG_ERROR,
                 "Error reading from server (2) authenticating '%s@%s:%d'\n",
                 PCU.userName, PCU.serverName, PCU.serverPort);
          return NULL;
        }
      }
    }
	if (buf[0] != '+') {
		POP_DM(pc, DEBUG_ERROR,
			   "Failed password when authenticating '%s@%s:%d'\n",
			   PCU.userName, PCU.serverName, PCU.serverPort);
		POP_DM(pc, DEBUG_ERROR, "The server's error message was: %s\n",
			   buf);
		return NULL;
	};

	return scs;
}

void pop3_cacheHeaders( /*@notnull@ */ Pop3 pc)
{
	char buf[BUF_SIZE];
	struct connection_state *scs;
	int i;

	if (pc->headerCache != NULL) {
		/* decrement the reference count, and free our version */
		imap_releaseHeaders(pc, pc->headerCache);
		pc->headerCache = NULL;
	}

	POP_DM(pc, DEBUG_INFO, "working headers\n");
	/* login the server */
	scs = pop3Login(pc);
	if (scs == NULL)
		return;
	/* pc->UnreadMsgs = pc->TotalMsgs - read; */
	pc->headerCache = NULL;
	for (i = pc->TotalMsgs - pc->UnreadMsgs + 1; i <= pc->TotalMsgs; ++i) {
		struct msglst *m;
		m = malloc(sizeof(struct msglst));

		m->subj[0] = '\0';
		m->from[0] = '\0';
		POP_DM(pc, DEBUG_INFO, "search: %s", buf);

		tlscomm_printf(scs, "TOP %i 0\r\n", i);
		while (tlscomm_gets(buf, 256, scs) && buf[0] != '.') {
			if (!strncasecmp(buf, "From: ", 6)) {
				/* manage the from in heads */
				strncpy(m->from, buf + 6, FROM_LEN - 1);
				m->from[FROM_LEN - 1] = '\0';
			} else if (!strncasecmp(buf, "Subject: ", 9)) {
				/* manage subject */
				strncpy(m->subj, buf + 9, SUBJ_LEN - 1);
				m->subj[SUBJ_LEN - 1] = '\0';
			}
			if (!m->subj[0]) {
				strncpy(m->subj, "[NO SUBJECT]", 14);
			}
			if (!m->from[0]) {
				strncpy(m->from, "[ANONYMOUS]", 14);
			}
		}
		m->next = pc->headerCache;
		pc->headerCache = m;
		pc->headerCache->in_use = 0;
	}
	tlscomm_printf(scs, "QUIT\r\n");
	tlscomm_close(scs);
}

/* vim:set ts=4: */
static void ask_user_for_password( /*@notnull@ */ Pop3 pc, int bFlushCache)
{
	/* see if we already have a password, as provided in the config file, or
	   already requested from the user. */
	if (PCU.interactive_password) {
		if (strlen(PCU.password) == 0) {
			/* we need to grab the password from the user. */
			char *password;
			POP_DM(pc, DEBUG_INFO, "asking for password %d\n",
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
				// ENFROB(PCU.password);
			}
		}
	}
}

