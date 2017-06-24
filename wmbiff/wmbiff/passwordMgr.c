/* passwordMgr.c
 * Author: Neil Spring
 */
/* this module implements a password cache: the goal is to
   allow multiple wmbiff mailboxes that are otherwise
   independent get all their passwords while only asking the
   user for an account's password once. */
/* NOTE: it will fail if a user has different passwords for
   pop vs. imap on the same server; this seems too far
   fetched to be worth complexity */

/* NOTE: it verifies that the askpass program, which, if
   given with a full path, must be owned either by the user
   or by root.  There may be decent reasons not to do
   this. */

/* Intended properties: 1) exit()s if the user presses
   cancel from askpass - this is detected by no output from
   askpass.  2) allows the caller to remove a cached entry
   if it turns out to be wrong, and prompt the user
   again. This might be poor if the askpass program is
   replaced with something non-interactive. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "passwordMgr.h"
#include "Client.h"
#include "charutil.h"			/* chomp */
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>			/* index */
#include <sys/stat.h>
#include "assert.h"

#ifdef HAVE_MEMFROB
#define DEFROB(x) memfrob(x, x ## _len)
#define ENFROB(x) memfrob(x, x ## _len)
#else
#define DEFROB(x)
#define ENFROB(x)
#endif

typedef struct password_binding_struct {
	struct password_binding_struct *next;
	char user[BUF_SMALL];
	char server[BUF_BIG];
	char password[BUF_SMALL];	/* may be frobnicated */
	unsigned char password_len;	/* frobnicated *'s are nulls */
} *password_binding;

static password_binding pass_list = NULL;

/* verifies that askpass_fname, if it has no spaces, exists as
   a file, is owned by the user or by root, and is not world
   writeable.   This is just a sanity check, and is not intended
   to ensure the integrity of the password-asking program. */
/* would be static, but used in test_wmbiff */
int permissions_ok(Pop3 pc, const char *askpass_fname)
{
	struct stat st;
	if (index(askpass_fname, ' ')) {
		DM(pc, DEBUG_INFO,
		   "askpass has a space in it; not verifying ownership/permissions on '%s'\n",
		   askpass_fname);
		return (1);
	}
	if (stat(askpass_fname, &st)) {
		DM(pc, DEBUG_ERROR, "Can't stat askpass program: '%s'\n",
		   askpass_fname);
		if (askpass_fname[0] != '/') {
			DM(pc, DEBUG_ERROR,
			   "For your own good, use a full pathname.\n");
		}
		return (0);
	}
	if (st.st_uid != 0 && st.st_uid != getuid()) {
		DM(pc, DEBUG_ERROR,
		   "askpass program isn't owned by you or root: '%s'\n",
		   askpass_fname);
		return (0);
	}
	if (st.st_mode & S_IWOTH) {
		DM(pc, DEBUG_ERROR,
		   "askpass program is world writable: '%s'\n", askpass_fname);
		return (0);
	}
	return (1);
}

#ifdef HAVE_CORESERVICES_CORESERVICES_H
#ifdef HAVE_SECURITY_SECURITY_H
#define HAVE_APPLE_KEYCHAIN
#endif
#endif


#ifdef HAVE_APPLE_KEYCHAIN
/* routines to use apple's keychain to get a password
   without a user having to type.  this avoids some damage
   where although ssh-askpass can grab focus within X, it
   may not have a particularly secure keyboard. */

#include<CoreServices/CoreServices.h>
#include<Security/Security.h>

static void
get_password_from_keychain(Pop3 pc, const char *username,
						   const char *servername,
						   /*@out@ */ char *password,
						   /*@out@ */
						   unsigned char *password_len)
{
	SecKeychainRef kc;
	OSStatus rc;
	char *secpwd;
	UInt32 pwdlen;
	rc = SecKeychainCopyDefault(&kc);
	if (rc != noErr) {
		DM(pc, DEBUG_ERROR, "passmgr: unable to open keychain, exiting\n");
		exit(EXIT_FAILURE);
	}
	rc = SecKeychainFindInternetPassword(kc, strlen(servername),
										 servername, 0, NULL,
										 strlen(username), username, 0,
										 NULL, 0, NULL,
										 kSecAuthenticationTypeDefault,
										 &pwdlen, (void **) &secpwd, NULL);
	if (rc != noErr) {
		DM(pc, DEBUG_ERROR,
		   "passmgr: keychain password grab for %s at %s failed, exiting\n", username, servername);
		DM(pc, DEBUG_ERROR, "passmgr: (perhaps you pressed 'deny')\n");
		/* this seems like the sanest thing to do, for now */
		exit(EXIT_FAILURE);
	}

	if (pwdlen < *password_len) {
		strncpy(password, secpwd, pwdlen);
		password[pwdlen] = '\0';
		*password_len = pwdlen;
	} else {
		DM(pc, DEBUG_ERROR,
		   "passmgr: warning: your password appears longer (%lu) than expected (%d)\n",
		   strlen(secpwd), *password_len - 1);
	}
	rc = SecKeychainItemFreeContent(NULL, secpwd);
	return;
}
#endif							/* apple keychain */


static void
get_password_from_command(Pop3 pc, const char *username,
						  const char *servername,
						  /*@out@ */ char *password,
						  /*@out@ */
						  unsigned char *password_len)
{
	password[*password_len - 1] = '\0';
	password[0] = '\0';
	/* check that the executed file is a good one. */
	if (permissions_ok(pc, pc->askpass)) {
		char *command;
		char *password_ptr;
		int len =
			strlen(pc->askpass) + strlen(username) +
			strlen(servername) + 40;
		command = malloc(len);
		snprintf(command, len, "%s 'password for wmbiff: %s@%s'",
				 pc->askpass, username, servername);

		(void) grabCommandOutput(pc, command, &password_ptr, NULL);
		/* it's not clear what to do with the exit
		   status, though we can get it from
		   grabCommandOutput if needed to deal with some
		   programs that will print a message but exit
		   non-zero on error */
		free(command);

		if (password_ptr == NULL) {
			/* this likely means that the user cancelled, and doesn't
			   want us to keep asking about the password. */
			DM(pc, DEBUG_ERROR,
			   "passmgr: fgets password failed, exiting\n");
			DM(pc, DEBUG_ERROR,
			   "passmgr: (it looks like you pressed 'cancel')\n");
			/* this seems like the sanest thing to do, for now */
			exit(EXIT_FAILURE);
		}
		strncpy(password, password_ptr, *password_len);
		if (password[*password_len - 1] != '\0') {
			DM(pc, DEBUG_ERROR,
			   "passmgr: warning: your password appears longer (%lu) than expected (%d)\n",
			   (unsigned long) strlen(password_ptr), *password_len - 1);
		}
		free(password_ptr);
		password[*password_len - 1] = '\0';
		*password_len = strlen(password);
	} else {
		/* consider this error to be particularly troublesome */
		DM(pc, DEBUG_ERROR,
		   "passmgr: permissions check of '%s' failed.", pc->askpass);
		exit(EXIT_FAILURE);
	}
}

char *passwordFor(const char *username,
				  const char *servername, Pop3 pc, int bFlushCache)
{

	password_binding p;

	assert(username != NULL);
	assert(username[0] != '\0');

	/* find the binding */
	for (p = pass_list;
		 p != NULL
		 && (strcmp(username, p->user) != 0 ||
			 strcmp(servername, p->server) != 0); p = p->next);

	/* if so, return the password */
	if (p != NULL) {
		if (p->password[0] != '\0') {
			if (bFlushCache == 0) {
				char *ret = strdup(p->password);
#ifdef HAVE_MEMFROB
				unsigned short ret_len = p->password_len;
				DEFROB(ret);
#endif
				return (ret);
			}
			/* else fall through, overwrite */
		} else if (pc) {
			/* if we've asked, but received nothing, disable this box */
			pc->checkMail = NULL;
			return (NULL);
		}
	} else {
		p = (password_binding)
			malloc(sizeof(struct password_binding_struct));
	}

	/* else, try to get it. */
	if (pc->askpass != NULL) {
		char *retval;

		p->password_len = 32;
#ifdef HAVE_APPLE_KEYCHAIN
		if (strcmp(pc->askpass, "internal:apple:keychain") == 0) {
			get_password_from_keychain(pc, username, servername,
									   p->password, &p->password_len);
		} else {
			DM(pc, DEBUG_ERROR,
			   "you could change your askpass line to:\n"
			   "    askpass = internal:apple:keychain\n"
			   "to use the OS X keychain instead of running a command\n");
#endif
			get_password_from_command(pc, username, servername,
									  p->password, &p->password_len);
#ifdef HAVE_APPLE_KEYCHAIN
		}
#endif
		retval = strdup(p->password);
		if (strlen(username) + 1 > BUF_SMALL) {
			DM(pc, DEBUG_ERROR, "username is too long.\n");
			memset(p->user, 0, BUF_SMALL);
		} else {
			strncpy(p->user, username, BUF_SMALL - 1);
		}
		if (strlen(servername) + 1 > BUF_BIG) {
			DM(pc, DEBUG_ERROR, "servername is too long.\n");
			memset(p->server, 0, BUF_BIG);
		} else {
			strncpy(p->server, servername, BUF_BIG - 1);
		}
		ENFROB(p->password);
		p->next = pass_list;
		pass_list = p;
		return (retval);
	}

	return (NULL);
}

/* vim:set ts=4: */
/*
 * Local Variables:
 * tab-width: 4
 * c-indent-level: 4
 * c-basic-offset: 4
 * End:
 */
