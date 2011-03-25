/* $Id: Client.h,v 1.42 2005/05/08 21:31:22 bluehal Exp $ */
/* Author : Scott Holden ( scotth@thezone.net )
   Modified : Yong-iL Joh ( tolkien@mizi.com )
   Modified : Jorge García ( Jorge.Garcia@uv.es )
 *
 * Email Checker Pop3/Imap4/Licq/Gicu/mbox/maildir/finger
 *
 * Last Updated : $Date: 2005/05/08 21:31:22 $
 *
 */

#ifndef CLIENT
#define CLIENT
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef HAVE_GCRYPT_H
#include <gcrypt.h>
#endif

#ifdef __LCLINT__
typedef unsigned int off_t;
#endif

#define BUF_BIG 256
#define BUF_SMALL 32
#define BUF_SIZE 1024

struct msglst;
typedef struct _mbox_t *Pop3;
typedef struct _mbox_t {
	char label[BUF_SMALL];		/* Printed at left; max 5 chars */
	char path[BUF_BIG];			/* Path to mailbox */
	char notify[BUF_BIG];		/* Program to notify mail arrivation */
	char action[BUF_BIG];		/* Action to execute on mouse click, reduces to
								 *  what happens on button1. this is executed after
								 *  either actionnew or actionnonew (if they are
								 *  defined in the config file) */
	char actionnew[BUF_BIG];	/* Action to execute on mouse click when new mail */
	char actionnonew[BUF_BIG];	/* Action to execute on mouse click when no new mail */
	char actiondc[BUF_BIG];		/* Action to execute when icq is disconnected */
	char button2[BUF_BIG];		/* What to run on button2. (middle) */
	char fetchcmd[BUF_BIG];		/* Action for mail fetching for pop3/imap, reduces to what happens on button3 */
	int fetchinterval;
	int TotalMsgs;				/* Total messages in mailbox */
	int UnreadMsgs;				/* New (unread) messages in mailbox */
	int OldMsgs;
	int OldUnreadMsgs;
	char TextStatus[10];		/* if set to a string, toupper() and blit 
								 * that string. instead of a message count */
	int blink_stat;				/* blink digits flag-counter */
	int debug;					/* debugging status */

	struct msglst *headerCache;

	union {
		struct {
			time_t mtime;
			off_t size;
		} mbox;
		struct {
			char *detail;
		} shell;
		struct {
			time_t mtime_new;
			off_t size_new;
			time_t mtime_cur;
			off_t size_cur;
			unsigned int dircache_flush:1;	/* hack to flush directory caches */
		} maildir;
		struct {
			char password[BUF_SMALL];
			char userName[BUF_SMALL];
			char serverName[BUF_BIG];
			int serverPort;
			int localPort;
			char authList[100];
			unsigned int dossl:1;	/* use tls. */
			/* prompt the user if we can't login / password is empty */
			unsigned int interactive_password:1;
			/* using the msglst feature, fetch the headers
			   to have them on hand */
			unsigned int wantCacheHeaders:1;
			unsigned char password_len;	/* memfrob may shorten passwords */
		} pop_imap;
	} u;

	int (*checkMail) ( /*@notnull@ */ Pop3);

	/* collect the headers to show in a pop up */
	struct msglst *(*getHeaders) ( /*@notnull@ */ Pop3);
	/* allow the client to free the headers, or keep them cached */
	void (*releaseHeaders) ( /*@notnull@ */ Pop3, struct msglst * ml);

	time_t prevtime;
	time_t prevfetch_time;
	int loopinterval;			/* loop interval for this mailbox */

	/* command to execute to get a password, if needed */
	const char *askpass;
} mbox_t;

/* creation calls must have this prototype */
int pop3Create( /*@notnull@ */ Pop3 pc, const char *str);
int imap4Create( /*@notnull@ */ Pop3 pc, const char *str);
int licqCreate( /*@notnull@ */ Pop3 pc, const char *str);
int shellCreate( /*@notnull@ */ Pop3 pc, const char *str);
int mboxCreate( /*@notnull@ */ Pop3 pc, const char *str);
int maildirCreate( /*@notnull@ */ Pop3 pc, const char *str);

int sock_connect(const char *hostname, int port);
FILE *openMailbox(Pop3 pc, const char *mbox_filename);

/* backtickExpand returns null on failure */
/*@null@ */
char *backtickExpand(Pop3 pc, const char *path);
int fileHasChanged(const char *mbox_filename, time_t * atime,
				   time_t * mtime, off_t * size);
int grabCommandOutput(Pop3 pc, const char *command,
					  /*@out@ */ char **output,
					  /*@out@ *//*@null@ */ char **details);
int exists(const char *filename);	/* test -f */

/* _NONE is for silent operation.  _ERROR is for things that should
   be printed assuming that the user might possibly see them. _INFO is
   for reasonably useless but possibly interesting messages. _ALL is
   for everything.  Restated, _ERROR will always be printed, _INFO only
   if debugging messages were requested. */
#define DEBUG_NONE  0
#define DEBUG_ERROR 1
#define DEBUG_INFO  2
#define DEBUG_ALL   2
/* inspired by ksymoops-2.3.4/ksymoops.h */
#define DM(mbox, msglevel, X...) \
do { \
  if (mbox == NULL || (mbox)->debug >= msglevel) { \
     printf("wmbiff/%s ", (mbox != NULL) ? (mbox)->label : "NULL"); \
     printf(X); \
     (void)fflush(NULL); \
  } \
} while(0)

extern int debug_default;
#define DMA(msglevel, X...) \
do { \
  if (debug_default >= msglevel) { \
     printf("wmbiff: " X); \
(void)fflush(NULL); \
  } \
} while(0)

/* technique used in apache to allow GCC's attribute tags,
   without requiring gcc as the compiler */
#if !defined(__GNUC__) || __GNUC__ < 2 || \
  (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#ifndef __attribute__
#define __attribute__(__x)
#endif
#endif							/* gnuc */
#endif							/* client.h */

#ifndef FALSE
#define FALSE (0)
#define TRUE (!FALSE)
#endif

#ifdef __LCLINT__
/* lclint doesn't do typeof */
#define min(x,y) ((x > y) ? x : y)
#define max(x,y) ((x > y) ? x : y)
#else
/* from linux/kernel.h */
#define min(x,y) ({ \
        const typeof(x) _xi = (x);       \
        const typeof(y) _yi = (y);       \
        (void) (&_xi == &_yi);            \
        _xi < _yi ? _xi : _yi; })
#define max(x,y) ({ \
        const typeof(x) _xa = (x);       \
        const typeof(y) _ya = (y);       \
        (void) (&_xa == &_ya);            \
        _xa > _ya ? _xa : _ya; })
#endif


/* vim:set ts=4: */
/*
 * Local Variables:
 * tab-width: 4
 * c-indent-level: 4
 * c-basic-offset: 4
 * End:
 */
