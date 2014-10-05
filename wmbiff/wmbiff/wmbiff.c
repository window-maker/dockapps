/* $Id: wmbiff.c,v 1.70 2005/10/07 03:07:58 bluehal Exp $ */

// typedef int gcry_error_t;

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <ctype.h>

#ifdef HAVE_POLL
#include <poll.h>
#else
#include <sys/time.h>
#endif

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/cursorfont.h>
#include <X11/XKBlib.h>

#include <errno.h>
#include <string.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"

#include "Client.h"
#include "charutil.h"
#include "MessageList.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif


#include "wmbiff-master-led.xpm"
#include "wmbiff-classic-master-led.xpm"
static char wmbiff_mask_bits[64 * 64];
static const int wmbiff_mask_width = 64;
// const int wmbiff_mask_height = 64;

#define CHAR_WIDTH  5
#define CHAR_HEIGHT 7

#define BLINK_TIMES 8
#define DEFAULT_SLEEP_INTERVAL 20000
#define BLINK_SLEEP_INTERVAL    200
#define DEFAULT_LOOP 5

#define MAX_NUM_MAILBOXES 40
static mbox_t mbox[MAX_NUM_MAILBOXES];

/* this is the normal pixmap. */
static const char *skin_filename = "wmbiff-master-led.xpm";
static const char *classic_skin_filename = "wmbiff-classic-master-led.xpm";
/* global notify action taken (if globalnotify option is set) */
static const char *globalnotify = NULL;

/* path to search for pixmaps */
/* /usr/share/wmbiff should have the default pixmap. */
/* /usr/local/share/wmbiff if compiled locally. */
/* / is there in case a user wants to specify a complete path */
/* . is there for development. */
static const char *skin_search_path = DEFAULT_SKIN_PATH;
/* for gnutls */
const char *certificate_filename = NULL;
/* gnutls: specify the priorities to use on the ciphers, key exchange methods,
   macs and compression methods. */
const char *tls = NULL;

/* it could be argued that a better default exists. */
#define DEFAULT_FONT  "-*-fixed-*-r-*-*-10-*-*-*-*-*-*-*"
static const char *font = NULL;

int debug_default = DEBUG_ERROR;

/* color from wmbiff's xpm, down to 24 bits. */
const char *foreground = "white";	/* foreground white */
const char *background = "#505075";	/* background blue */
static const char *highlight = "red";
const char *foreground_classic = "#21B3AF";		/* classic foreground cyan */
const char *background_classic = "#202020";		/* classic background gray */
static const char *highlight_classic = "yellow";	/* classic highlight color */
int SkipCertificateCheck = 0;
int Relax = 0;					/* be not paranoid */
static int notWithdrawn = 0;

static unsigned int num_mailboxes = 1;
static const int x_origin = 5;
static const int y_origin = 5;
static int forever = 1;			/* keep running. */
unsigned int custom_skin = 0;		/* user has choose a custom skin */
static int classic_mode = 0;		/* use classic behaviour/theme of wmbiff */

extern Window win;
extern Window iconwin;

Cursor busy_cursor, ready_cursor;

static __inline /*@out@ */ void *
malloc_ordie(size_t len)
{
	void *ret = malloc(len);
	if (ret == NULL) {
		fprintf(stderr, "unable to allocate %d bytes\n", (int) len);
		abort();
	}
	return (ret);
}

/* where vertically the mailbox sits for blitting characters. */
static int mbox_y(unsigned int mboxnum)
{
	return ((11 * mboxnum) + y_origin);
}

/* 	Read a line from a file to obtain a pair setting=value
	skips # and leading spaces
	NOTE: if setting finish with 0, 1, 2, 3 or 4 last char are deleted and
	index takes its value... if not index will get -1
	Returns -1 if no setting=value
*/
static int ReadLine(FILE * fp, /*@out@ */ char *setting,
					/*@out@ */ char *value, /*@out@ */ int *mbox_index)
{
	char buf[BUF_SIZE];
	char *p, *q;
	int len;

	*setting = '\0';
	*value = '\0';
	*mbox_index = -1;

	if (!fp || feof(fp))
		return -1;

	if (!fgets(buf, BUF_SIZE - 1, fp))
		return -1;

	len = strlen(buf);

	if (buf[len - 1] == '\n') {
		buf[len - 1] = '\0';	/* strip linefeed */
	}

	StripComment(buf);

	if (!(p = strtok(buf, "=")))
		return -1;
	if (!(q = strtok(NULL, "\n")))
		return -1;

	/* Chg - Mark Hurley
	 * Date: May 8, 2001
	 * Removed for loop (which removed leading spaces)
	 * Leading & Trailing spaces need to be removed
	 * to Fix Debian bug #95849
	 */
	FullTrim(p);
	FullTrim(q);

	/* strcpy(setting, p); nspring replaced with sscanf dec 2002 */
	strcpy(value, q);

	if (sscanf(p, "%[_a-z.]%d", setting, mbox_index) == 2) {
		/* mailbox-specific configuration, ends in a digit */
		if (*mbox_index < 0 || *mbox_index >= MAX_NUM_MAILBOXES) {
			DMA(DEBUG_ERROR, "invalid mailbox number %d\n", *mbox_index);
			exit(EXIT_FAILURE);
		}
	} else if (sscanf(p, "%[a-z]", setting) == 1) {
		/* global configuration, all text. */
		*mbox_index = -1;
	} else {
		/* we found an uncommented line that has an equals,
		   but is non-alphabetic. */
		DMA(DEBUG_INFO, "unparsed setting %s\n", p);
		return -1;
	}

	DMA(DEBUG_INFO, "@%s.%d=%s@\n", setting, *mbox_index, value);
	return 1;
}

struct path_demultiplexer {
	const char *id;				/* followed by a colon */
	int (*creator) ( /*@notnull@ */ Pop3 pc, const char *path);
};

static struct path_demultiplexer paths[] = {
	{"pop3:", pop3Create},
	{"pop3s:", pop3Create},
	{"shell:", shellCreate},
	{"imap:", imap4Create},
	{"imaps:", imap4Create},
	{"sslimap:", imap4Create},
	{"maildir:", maildirCreate},
	{"mbox:", mboxCreate},
	{NULL, NULL}
};


static void parse_mbox_path(unsigned int item)
{
	int i;
	/* find the creator */
	for (i = 0;
		 paths[i].id != NULL
		 && strncasecmp(mbox[item].path, paths[i].id, strlen(paths[i].id));
		 i++);
	/* if found, execute */
	if (paths[i].id != NULL) {
		if (paths[i].creator((&mbox[item]), mbox[item].path) != 0) {
			DMA(DEBUG_ERROR, "creator for mailbox %u returned failure\n",
				item);
		}
	} else {
		/* default are mbox */
		mboxCreate((&mbox[item]), mbox[item].path);
	}
}

static int Read_Config_File(char *filename, int *loopinterval)
{
	FILE *fp;
	char setting[BUF_SMALL], value[BUF_SIZE];
	int mbox_index;
	unsigned int i;
	char *skin = NULL;

	if (classic_mode) {
		skin = strdup_ordie(skin_filename);
	}

	if (!(fp = fopen(filename, "r"))) {
		DMA(DEBUG_ERROR, "Unable to open %s, no settings read: %s\n",
			filename, strerror(errno));
		return 0;
	}
	while (!feof(fp)) {
		/* skanky: -1 can represent an unparsed line
		   or an error */
		if (ReadLine(fp, setting, value, &mbox_index) == -1)
			continue;

		/* settings that can be global go here. */
		if (!strcmp(setting, "interval")) {
			*loopinterval = atoi(value);
			continue;
		} else if (!strcmp(setting, "askpass")) {
			const char *askpass = strdup_ordie(value);
			if (mbox_index == -1) {
				DMA(DEBUG_INFO, "setting all to askpass %s\n", askpass);
				for (i = 0; i < MAX_NUM_MAILBOXES; i++)
					mbox[i].askpass = askpass;
			} else {
				mbox[mbox_index].askpass = askpass;
			}
			continue;
		} else if (!strcmp(setting, "skinfile")) {
			skin_filename = strdup_ordie(value);
			custom_skin = 1;
			continue;
		} else if (!strcmp(setting, "certfile")) {	/* not yet supported */
			certificate_filename = strdup_ordie(value);
			continue;
		} else if (!strcmp(setting, "globalnotify")) {
			globalnotify = strdup_ordie(value);
			continue;
		} else if (!strcmp(setting, "tls")) {
			tls = strdup_ordie(value);
			continue;
		} else if (mbox_index == -1) {
			DMA(DEBUG_INFO, "Unknown global setting '%s'\n", setting);
			continue;			/* Didn't read any setting.[0-5] value */
		}

		if (mbox_index >= MAX_NUM_MAILBOXES) {
			DMA(DEBUG_ERROR, "Don't have %d mailboxes.\n", mbox_index);
			continue;
		}

		if (1U + mbox_index > num_mailboxes
			&& mbox_index + 1 <= MAX_NUM_MAILBOXES) {
			num_mailboxes = 1U + mbox_index;
		}

		/* now only local settings */
		if (!strcmp(setting, "label.")) {
			if (strlen(value) + 1 > BUF_SMALL) {
				DMA(DEBUG_ERROR,
					"Mailbox %i label string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].label, value, BUF_SMALL - 1);
			}
		} else if (!strcmp(setting, "path.")) {
			if (strlen(value) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"Mailbox %i path string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].path, value, BUF_BIG - 1);
			}
		} else if (!strcmp(setting, "notify.")) {
			if (strlen(value) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"Mailbox %i notify string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].notify, value, BUF_BIG - 1);
			}
		} else if (!strcmp(setting, "action.")) {
			if (strlen(value) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"Mailbox %i action string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].action, value, BUF_BIG - 1);
			}
		} else if (!strcmp(setting, "action_disconnected.")) {
			if (strlen(value) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"Mailbox %i action_disconnected string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].actiondc, value, BUF_BIG - 1);
			}
		} else if (!strcmp(setting, "action_new_mail.")) {
			if (strlen(value) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"Mailbox %i action_new_mail string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].actionnew, value, BUF_BIG - 1);
			}
		} else if (!strcmp(setting, "action_no_new_mail.")) {
			if (strlen(value) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"Mailbox %i action_no_new_mail string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].actionnonew, value, BUF_BIG - 1);
			}
		} else if (!strcmp(setting, "interval.")) {
			mbox[mbox_index].loopinterval = atoi(value);
		} else if (!strcmp(setting, "buttontwo.")) {
			if (strlen(value) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"Mailbox %i buttontwo string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].button2, value, BUF_BIG - 1);
			}
		} else if (!strcmp(setting, "fetchcmd.")) {
			if (strlen(value) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"Mailbox %i fetchcmd string '%s' is too long.\n",
					mbox_index, value);
				continue;
			} else {
				strncpy(mbox[mbox_index].fetchcmd, value, BUF_BIG - 1);
			}
		} else if (!strcmp(setting, "fetchinterval.")) {
			mbox[mbox_index].fetchinterval = atoi(value);
		} else if (!strcmp(setting, "debug.")) {
			int debug_value = debug_default;
			if (strcasecmp(value, "all") == 0) {
				debug_value = DEBUG_ALL;
			}
			/* could disable debugging, but I want the command
			   line argument to provide all information
			   possible. */
			mbox[mbox_index].debug = debug_value;
		} else {
			DMA(DEBUG_INFO, "Unknown setting '%s'\n", setting);
		}
	}
	(void) fclose(fp);

	if (!tls)
		// use GnuTLS's default ciphers.
		tls = "NORMAL";

	if (classic_mode && skin && !strcmp(skin, skin_filename))
			skin_filename = classic_skin_filename;

	if (skin)
		free(skin);

	for (i = 0; i < num_mailboxes; i++)
		if (mbox[i].label[0] != '\0')
			parse_mbox_path(i);
	return 1;
}


static void init_biff(char *config_file)
{
#ifdef HAVE_GCRYPT_H
	gcry_error_t rc;
#endif
	int loopinterval = DEFAULT_LOOP;
	unsigned int i;

	for (i = 0; i < MAX_NUM_MAILBOXES; i++) {
		memset(mbox[i].label, 0, BUF_SMALL);
		memset(mbox[i].path, 0, BUF_BIG);
		memset(mbox[i].notify, 0, BUF_BIG);
		memset(mbox[i].action, 0, BUF_BIG);
		memset(mbox[i].actiondc, 0, BUF_BIG);
		memset(mbox[i].actionnew, 0, BUF_BIG);
		memset(mbox[i].actionnonew, 0, BUF_BIG);
		memset(mbox[i].button2, 0, BUF_BIG);
		memset(mbox[i].fetchcmd, 0, BUF_BIG);
		mbox[i].loopinterval = 0;
		mbox[i].getHeaders = NULL;
		mbox[i].releaseHeaders = NULL;
		mbox[i].debug = debug_default;
		mbox[i].askpass = DEFAULT_ASKPASS;
	}

#ifdef HAVE_GCRYPT_H
	/* gcrypt is a little strange, in that it doesn't
	 * seem to initialize its memory pool by itself.
	 * I believe we *expect* "Warning: using insecure memory!"
	 */
	/* gcryctl_disable_secmem gets called before check_version -- in a message on
	   gcrypt-devel august 17 2004. */
	if ((rc = gcry_control(GCRYCTL_DISABLE_SECMEM, 0)) != 0) {
		DMA(DEBUG_ERROR,
			"Error: tried to disable gcrypt warning and failed: %d\n"
			" Message: %s\n" " libgcrypt version: %s\n", rc,
			gcry_strerror(rc), gcry_check_version(NULL));
	}

	/* recently made a requirement, section 2.4 of gcrypt manual */
	if (gcry_check_version("1.1.42") == NULL) {
		DMA(DEBUG_ERROR, "Error: incompatible gcrypt version.\n");
	}
#endif

	DMA(DEBUG_INFO, "config_file = %s.\n", config_file);
	if (!Read_Config_File(config_file, &loopinterval)) {
		char *m;
		/* setup defaults if there's no config */
		if ((m = getenv("MAIL")) != NULL) {
			/* we are using MAIL environment var. type mbox */
			if (strlen(m) + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"MAIL environment var '%s' is too long.\n", m);
			} else {
				DMA(DEBUG_INFO, "Using MAIL environment var '%s'.\n", m);
				strncpy(mbox[0].path, m, BUF_BIG - 1);
			}
		} else if ((m = getenv("USER")) != NULL) {
			/* we will use the USER env var to find an mbox name */
			if (strlen(m) + 10 + 1 > BUF_BIG) {
				DMA(DEBUG_ERROR,
					"USER environment var '%s' is too long.\n", m);
			} else {
				DMA(DEBUG_INFO, "Using /var/mail/%s.\n", m);
				strcpy(mbox[0].path, "/var/mail/");
				strncat(mbox[0].path, m, BUF_BIG - 1 - 10);
				if (mbox[0].path[9] != '/') {
					DMA(DEBUG_ERROR,
						"Unexpected failure to construct /var/mail/username, please "
						"report this with your operating system info and the version of wmbiff.");
				}
			}
		} else {
			DMA(DEBUG_ERROR, "Cannot open config file '%s' nor use the "
				"MAIL environment var.\n", config_file);
			exit(EXIT_FAILURE);
		}
		if (!exists(mbox[0].path)) {
			DMA(DEBUG_ERROR, "Cannot open config file '%s', and the "
				"default %s doesn't exist.\n", config_file, mbox[0].path);
			exit(EXIT_FAILURE);
		}
		strcpy(mbox[0].label, "Spool");
		mboxCreate((&mbox[0]), mbox[0].path);
	}

	/* Make labels look right */
	for (i = 0; i < num_mailboxes; i++) {
		if (mbox[i].label[0] != '\0') {
			/* append a colon, but skip if we're using fonts. */
			if (font == NULL) {
				int j = strlen(mbox[i].label);
				if (j < 5) {
					memset(mbox[i].label + j, ' ', 5 - j);
				}
			}
			/* but always end after 5 characters */
			mbox[i].label[6] = '\0';
			/* set global loopinterval to boxes with 0 loop */
			if (!mbox[i].loopinterval) {
				mbox[i].loopinterval = loopinterval;
			}
		}
	}
}

static char **LoadXPM(const char *pixmap_filename)
{
	char **xpm;
	int success;
	success = XpmReadFileToData((char *) pixmap_filename, &xpm);
	switch (success) {
	case XpmOpenFailed:
		DMA(DEBUG_ERROR, "Unable to open %s\n", pixmap_filename);
		break;
	case XpmFileInvalid:
		DMA(DEBUG_ERROR, "%s is not a valid pixmap\n", pixmap_filename);
		break;
	case XpmNoMemory:
		DMA(DEBUG_ERROR, "Insufficient memory to read %s\n",
			pixmap_filename);
		break;
	default:
		break;
	}
	return (xpm);
}

/* tests as "test -f" would */
int exists(const char *filename)
{
	struct stat st_buf;
	DMA(DEBUG_INFO, "looking for %s\n", filename);
	if (stat(filename, &st_buf) == 0 && S_ISREG(st_buf.st_mode)) {
		DMA(DEBUG_INFO, "found %s\n", filename);
		return (1);
	}
	return (0);
}

/* acts like execvp, with code inspired by it */
/* mustfree */
static char *search_path(const char *path, const char *find_me)
{
	char *buf;
	const char *p;
	int len, pathlen;
	if (strchr(find_me, '/') != NULL) {
		return (strdup_ordie(find_me));
	}
	pathlen = strlen(path);
	len = strlen(find_me) + 1;
	buf = malloc_ordie(pathlen + len + 1);
	memcpy(buf + pathlen + 1, find_me, len);
	buf[pathlen] = '/';

	for (p = path; p != NULL; path = p, path++) {
		char *startp;
		p = strchr(path, ':');
		if (p == NULL) {
			/* not found; p should point to the null char at the end */
			startp =
				memcpy(buf + pathlen - strlen(path), path, strlen(path));
		} else if (p == path) {
			/* double colon in a path apparently means try here */
			startp = &buf[pathlen + 1];
		} else {
			/* copy the part between the colons to the buffer */
			startp = memcpy(buf + pathlen - (p - path), path, p - path);
		}
		if (exists(startp) != 0) {
			char *ret = strdup_ordie(startp);
			free(buf);
			return (ret);
		}
	}
	free(buf);
	return (NULL);
}

/* verifies that .wmbiffrc, is a file, is owned by the user,
   is not world writeable, and is not world readable.  This
   is just to help keep passwords secure */
static int wmbiffrc_permissions_check(const char *wmbiffrc_fname)
{
	struct stat st;
	if (stat(wmbiffrc_fname, &st) != 0) {
		DMA(DEBUG_ERROR, "Can't stat wmbiffrc: '%s'\n", wmbiffrc_fname);
		return (1);				/* well, it's not a bad permission
								   problem: if you can't find it,
								   neither can the bad guys.. */
	}
	if ((st.st_mode & S_IFDIR) != 0) {
		DMA(DEBUG_ERROR, ".wmbiffrc '%s' is a directory!\n"
			"exiting.  don't do that.", wmbiffrc_fname);
		exit(EXIT_FAILURE);
	}
	if (st.st_uid != getuid()) {
		char *user = getenv("USER");
		DMA(DEBUG_ERROR,
			".wmbiffrc '%s' isn't owned by you.\n"
			"Verify its contents, then 'chown %s %s'\n",
			wmbiffrc_fname, ((user != NULL) ? user : "(your username)"),
			wmbiffrc_fname);
		return (0);
	}
	if ((st.st_mode & S_IWOTH) != 0) {
		DMA(DEBUG_ERROR, ".wmbiffrc '%s' is world writable.\n"
			"Verify its contents, then 'chmod 0600 %s'\n",
			wmbiffrc_fname, wmbiffrc_fname);
		return (0);
	}
	if ((st.st_mode & S_IROTH) != 0) {
		DMA(DEBUG_ERROR, ".wmbiffrc '%s' is world readable.\n"
			"Please run 'chmod 0600 %s' and consider changing your passwords.\n",
			wmbiffrc_fname, wmbiffrc_fname);
		return (0);
	}
	return (1);
}

static void ClearDigits(unsigned int i)
{
	if (font) {
		eraseRect(39, mbox_y(i), 58, mbox_y(i + 1) - 1, background);
	} else {
		/* overwrite the colon */
		copyXPMArea((10 * (CHAR_WIDTH + 1)), 64, (CHAR_WIDTH + 1),
					(CHAR_HEIGHT + 1), 35, mbox_y(i));
		/* blank out the number fields. */
		copyXPMArea(39, 84, (3 * (CHAR_WIDTH + 1)), (CHAR_HEIGHT + 1), 39,
					mbox_y(i));
	}
}

/* Blits a string at given co-ordinates. If a ``new''
   parameter is nonzero, all digits will be yellow */
static void BlitString(const char *name, int x, int y, int new)
{
	if (font != NULL) {
		/* an alternate behavior - draw the string using a font
		   instead of the pixmap.  should allow pretty colors */
		drawString(x, y + CHAR_HEIGHT + 1, name,
				   new ? highlight : foreground, background, 0);
	} else {
		/* normal, LED-like behavior. */
		int i, c, k = x;
		for (i = 0; name[i] != '\0'; i++) {
			c = toupper(name[i]);
			if (c >= 'A' && c <= 'Z') {	/* it's a letter */
				c -= 'A';
				copyXPMArea(c * (CHAR_WIDTH + 1), (new ? 95 : 74),
							(CHAR_WIDTH + 1), (CHAR_HEIGHT + 1), k, y);
				k += (CHAR_WIDTH + 1);
			} else {			/* it's a number or symbol */
				c -= '0';
				if (new) {
					copyXPMArea((c * (CHAR_WIDTH + 1)) + 65, 0,
								(CHAR_WIDTH + 1), (CHAR_HEIGHT + 1), k, y);
				} else {
					copyXPMArea((c * (CHAR_WIDTH + 1)), 64,
								(CHAR_WIDTH + 1), (CHAR_HEIGHT + 1), k, y);
				}
				k += (CHAR_WIDTH + 1);
			}
		}
	}
}


/* Blits number to give coordinates.. two 0's, right justified */
static void BlitNum(int num, int x, int y, int new)
{
	char buf[32];

	sprintf(buf, "%02i", num);

	if (font != NULL) {
		const char *color = (new) ? highlight : foreground;
		drawString(x + (CHAR_WIDTH * 2 + 4), y + CHAR_HEIGHT + 1, buf,
				   color, background, 1);
	} else {
		int newx = x;

		if (num > 99)
			newx -= (CHAR_WIDTH + 1);
		if (num > 999)
			newx -= (CHAR_WIDTH + 1);

		BlitString(buf, newx, y, new);
	}
}

/* helper function for displayMsgCounters, which has outgrown its name */
static void blitMsgCounters(unsigned int i)
{
	int y_row = mbox_y(i);		/* constant for each mailbox */
	ClearDigits(i);				/* Clear digits */
	if ((mbox[i].blink_stat & 0x01) == 0) {
		int newmail = (mbox[i].UnreadMsgs > 0) ? 1 : 0;
		if (mbox[i].TextStatus[0] != '\0') {
			BlitString(mbox[i].TextStatus, 39, y_row, newmail);
		} else {
			int mailcount =
				(newmail) ? mbox[i].UnreadMsgs : ( classic_mode ? mbox[i].TotalMsgs : 0 );
			BlitNum(mailcount, 45, y_row, newmail);
		}
	}
}

/*
 * void execnotify(1) : runs notify command, if given (not null)
 */
static void execnotify( /*@null@ */ const char *notifycmd)
{
	if (notifycmd != NULL) {	/* need to call notify() ? */
		if (classic_mode && !strcasecmp(notifycmd, "beep"))
			XBell(display, 100);
		else if (!strcasecmp(notifycmd, "true")) {
			/* Yes, nothing */
		} else {
			/* Else call external notifyer, ignoring the pid */
			(void) execCommand(notifycmd);
		}
	}
}


static void
displayMsgCounters(unsigned int i, int mail_stat, int *Blink_Mode)
{
	switch (mail_stat) {
	case 2:					/* New mail has arrived */
		/* Enter blink-mode for digits */
		mbox[i].blink_stat = BLINK_TIMES * 2;
		*Blink_Mode |= (1 << i);	/* Global blink flag set for this mailbox */
		blitMsgCounters(i);
		execnotify(mbox[i].notify);
		break;
	case 1:					/* mailbox has been rescanned/changed */
		blitMsgCounters(i);
		break;
	case 0:
		break;
	case -1:					/* Error was detected */
		ClearDigits(i);			/* Clear digits */
		BlitString("XX", 45, mbox_y(i), 0);
		break;
	}
}

/** counts mail in spool-file
   Returned value:
   -1 : Error was encountered
   0  : mailbox status wasn't changed
   1  : mailbox was changed (NO new mail)
   2  : mailbox was changed AND new mail has arrived
**/
static int count_mail(unsigned int item)
{
	int rc = 0;

	if (!mbox[item].checkMail) {
		return -1;
	}

	if (mbox[item].checkMail(&(mbox[item])) < 0) {
		/* we failed to obtain any numbers therefore set
		 * them to -1's ensuring the next pass (even if
		 * zero) will be captured correctly
		 */
		mbox[item].TotalMsgs = -1;
		mbox[item].UnreadMsgs = -1;
		mbox[item].OldMsgs = -1;
		mbox[item].OldUnreadMsgs = -1;
		return -1;
	}

	if (mbox[item].UnreadMsgs > mbox[item].OldUnreadMsgs &&
		mbox[item].UnreadMsgs > 0) {
		rc = 2;					/* New mail detected */
	} else if (mbox[item].UnreadMsgs < mbox[item].OldUnreadMsgs ||
			   mbox[item].TotalMsgs != mbox[item].OldMsgs) {
		rc = 1;					/* mailbox was changed - NO new mail */
	} else {
		rc = 0;					/* mailbox wasn't changed */
	}
	mbox[item].OldMsgs = mbox[item].TotalMsgs;
	mbox[item].OldUnreadMsgs = mbox[item].UnreadMsgs;
	return rc;
}

static int periodic_mail_check(void)
{
	int NeedRedraw = 0;
	static int Blink_Mode = 0;	/* Bit mask, digits are in blinking
								   mode or not. Each bit for separate
								   mailbox */
	int Sleep_Interval;			/* either DEFAULT_SLEEP_INTERVAL or
								   BLINK_SLEEP_INTERVAL */
	int NewMail = 0;			/* flag for global notify */
	unsigned int i;
	time_t curtime = time(0);
	for (i = 0; i < num_mailboxes; i++) {
		if (mbox[i].label[0] != '\0') {
			if (curtime >= mbox[i].prevtime + mbox[i].loopinterval) {
				int mailstat = 0;
				NeedRedraw = 1;
				DM(&mbox[i], DEBUG_INFO,
				   "working on [%u].label=>%s< [%u].path=>%s<\n", i,
				   mbox[i].label, i, mbox[i].path);
				DM(&mbox[i], DEBUG_INFO,
				   "curtime=%d, prevtime=%d, interval=%d\n",
				   (int) curtime, (int) mbox[i].prevtime,
				   mbox[i].loopinterval);
				mbox[i].prevtime = curtime;

				XDefineCursor(display, iconwin, busy_cursor);
				RedrawWindow();

				mailstat = count_mail(i);

				XUndefineCursor(display, iconwin);

				/* Global notify */
				if (mailstat == 2)
					NewMail = 1;

				displayMsgCounters(i, mailstat, &Blink_Mode);
				/* update our idea of current time, as it
				   may have changed as we check. */
				curtime = time(0);
			}
			if (mbox[i].blink_stat > 0) {
				if (--mbox[i].blink_stat <= 0) {
					Blink_Mode &= ~(1 << i);
					mbox[i].blink_stat = 0;
				}
				displayMsgCounters(i, 1, &Blink_Mode);
				NeedRedraw = 1;
			}
			if (mbox[i].fetchinterval > 0 && mbox[i].fetchcmd[0] != '\0'
				&& curtime >=
				mbox[i].prevfetch_time + mbox[i].fetchinterval) {

				XDefineCursor(display, iconwin, busy_cursor);
				RedrawWindow();

				(void) execCommand(mbox[i].fetchcmd);

				XUndefineCursor(display, iconwin);

				mbox[i].prevfetch_time = curtime;
			}
		}
	}

	/* exec globalnotify if there was any new mail */
	if (NewMail == 1)
		execnotify(globalnotify);

	if (Blink_Mode == 0) {
		for (i = 0; i < num_mailboxes; i++) {
			mbox[i].blink_stat = 0;
		}
		Sleep_Interval = DEFAULT_SLEEP_INTERVAL;
	} else {
		Sleep_Interval = BLINK_SLEEP_INTERVAL;
	}

	if (NeedRedraw) {
		NeedRedraw = 0;
		RedrawWindow();
	}

	return Sleep_Interval;
}

static int findTopOfMasterXPM(const char **skin_xpm)
{
	int i;
	for (i = 0; skin_xpm[i] != NULL; i++) {
		if (strstr(skin_xpm[i], "++++++++") != NULL)
			return i;
	}
	DMA(DEBUG_ERROR,
		"couldn't find the top of the xpm file using the simple method\n");
	exit(EXIT_FAILURE);
}

static char **CreateBackingXPM(int width, int height,
							   const char **skin_xpm)
{
	char **ret = malloc_ordie(sizeof(char *) * (height + 6)
							  + sizeof(void *) /* trailing null space */ );
	const int colors = 5;
	const int base = colors + 1;
	const int margin = 4;
	int i;
	int top = findTopOfMasterXPM(skin_xpm);
	ret[0] = malloc_ordie(30);
	sprintf(ret[0], "%d %d %d %d", width, height, colors, 1);
	ret[1] = (char *) " \tc #0000FF";	/* no color */
	ret[2] = (char *) ".\tc #505075";	/* background gray */
	ret[2] = malloc_ordie(30);
	sprintf(ret[2], ".\tc %s", background);
	ret[3] = (char *) "+\tc #000000";	/* shadowed */
	ret[4] = (char *) "@\tc #C7C3C7";	/* highlight */
	ret[5] = (char *) ":\tc #004941";	/* led off */
	for (i = base; i < base + height; i++) {
		ret[i] = malloc_ordie(width);
	}
	for (i = base; i < base + margin; i++) {
		memset(ret[i], ' ', width);
	}
	for (i = base + margin; i < height + base - margin; i++) {
		memset(ret[i], ' ', margin);

		if (i == base + margin) {
			memset(ret[i] + margin, '+', width - margin - margin);
		} else if (i == base + height - margin - 1) {
			memset(ret[i] + margin, '@', width - margin - margin);
		} else {
			// "    +..:::...:::...:::...:::...:::.......:::...:::...:::...@    "
			// "    +.:...:.:...:.:...:.:...:.:...:..:..:...:.:...:.:...:..@    "                                                                                               ",
			ret[i][margin] = '+';
			memset(ret[i] + margin + 1, '.', width - margin - margin - 1);
			ret[i][width - margin - 1] = '@';
			memcpy(ret[i],
				   skin_xpm[((i - (base + margin) - 1) % 11) + top + 1],
				   width);
		}

		memset(ret[i] + width - margin, ' ', margin);
	}
	for (i = base + height - margin; i < height + base; i++) {
		memset(ret[i], ' ', width);
	}
	ret[height + base] = NULL;	/* not sure if this is necessary, it just
								   seemed like a good idea  */
	return (ret);
}

/*
 * NOTE: this function assumes that the ConnectionNumber() macro
 *       will return the file descriptor of the Display struct
 *       (it does under XFree86 and solaris' openwin X)
 */
static void XSleep(int millisec)
{
#ifdef HAVE_POLL
	struct pollfd timeout;

	timeout.fd = ConnectionNumber(display);
	timeout.events = POLLIN;

	poll(&timeout, 1, millisec);
#else
	struct timeval to;
	struct timeval *timeout = NULL;
	fd_set readfds;
	int max_fd;

	if (millisec >= 0) {
		timeout = &to;
		to.tv_sec = millisec / 1000;
		to.tv_usec = (millisec % 1000) * 1000;
	}
	FD_ZERO(&readfds);
	FD_SET(ConnectionNumber(display), &readfds);
	max_fd = ConnectionNumber(display);

	select(max_fd + 1, &readfds, NULL, NULL, timeout);
#endif
}

const char **restart_args;

static void restart_wmbiff(int sig
#ifdef HAVE___ATTRIBUTE__
						   __attribute__ ((unused))
#endif
	)
{
	if (restart_args) {
		DMA(DEBUG_ERROR, "exec()'ing %s\n", restart_args[0]);
		sleep(1);
		execvp(restart_args[0], (char *const *) restart_args);
		DMA(DEBUG_ERROR, "exec of %s failed: %s\n",
			restart_args[0], strerror(errno));
		exit(EXIT_FAILURE);
	}
	else
		fprintf(stderr, "Unable to restart wmbiff: missing restart arguments (NULL)!\n");
}

extern int x_socket(void)
{
	return ConnectionNumber(display);
}
extern void ProcessPendingEvents(void)
{
	static int but_pressed_region = -1;	/* static so click can be determined */
	int but_released_region = -1;
	/* X Events */
	while (XPending(display)) {
		XEvent Event;
		const char *press_action;

		XNextEvent(display, &Event);

		switch (Event.type) {
		case Expose:
			if (Event.xany.window != win && Event.xany.window != iconwin) {
				msglst_redraw();
			} else {
				RedrawWindow();
			}
			break;
		case DestroyNotify:
			XCloseDisplay(display);
			exit(EXIT_SUCCESS);
			break;
		case ButtonPress:
			but_pressed_region =
				CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
			switch (Event.xbutton.button) {
			case 1:
				press_action = mbox[but_pressed_region].action;
				break;
			case 2:
				press_action = mbox[but_pressed_region].button2;
				break;
			case 3:
				press_action = mbox[but_pressed_region].fetchcmd;
				break;
			default:
				press_action = NULL;
				break;

			}
			if (press_action && strcmp(press_action, "msglst") == 0) {
				msglst_show(&mbox[but_pressed_region],
							Event.xbutton.x_root, Event.xbutton.y_root);
			}
			break;
		case ButtonRelease:
			but_released_region =
				CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
			if (but_released_region == but_pressed_region
				&& but_released_region >= 0) {
				const char *click_action, *extra_click_action = NULL;

				switch (Event.xbutton.button) {
				case 1:		/* Left mouse-click */
					/* C-S-left will restart wmbiff. */
					if ((Event.xbutton.state & ControlMask) &&
						(Event.xbutton.state & ShiftMask)) {
						restart_wmbiff(0);
					}
					/* do we need to run an extra action? */
					if (mbox[but_released_region].UnreadMsgs == -1) {
						extra_click_action =
							mbox[but_released_region].actiondc;
					} else if (mbox[but_released_region].UnreadMsgs > 0) {
						extra_click_action =
							mbox[but_released_region].actionnew;
					} else {
						extra_click_action =
							mbox[but_released_region].actionnonew;
					}
					click_action = mbox[but_released_region].action;
					break;
				case 2:		/* Middle mouse-click */
					click_action = mbox[but_released_region].button2;
					break;
				case 3:		/* Right mouse-click */
					click_action = mbox[but_released_region].fetchcmd;
					break;
				default:
					click_action = NULL;
					break;
				}
				if (extra_click_action != NULL
					&& extra_click_action[0] != 0
					&& strcmp(extra_click_action, "msglst")) {
					DM(&mbox[but_released_region], DEBUG_INFO,
					   "runing: %s", extra_click_action);
					(void) execCommand(extra_click_action);
				}
				if (click_action != NULL
					&& click_action[0] != '\0'
					&& strcmp(click_action, "msglst")) {
					DM(&mbox[but_released_region], DEBUG_INFO,
					   "running: %s", click_action);
					(void) execCommand(click_action);
				}
			}

			/* a button was released, hide the message list if open */
			msglst_hide();

			but_pressed_region = -1;
			/* RedrawWindow(); */
			break;
		case MotionNotify:
			break;
		case KeyPress:{
				XKeyPressedEvent *xkpe = (XKeyPressedEvent *) & Event;
				KeySym ks = XkbKeycodeToKeysym(display, xkpe->keycode, 0, 0);
				if (ks > XK_0 && ks < XK_0 + min(9U, num_mailboxes)) {
					const char *click_action = mbox[ks - XK_1].action;
					if (click_action != NULL
						&& click_action[0] != '\0'
						&& strcmp(click_action, "msglst")) {
						DM(&mbox[but_released_region], DEBUG_INFO,
						   "running: %s", click_action);
						(void) execCommand(click_action);
					}
				}

			}
			break;
		default:
			break;
		}
	}
}

static void do_biff(int argc, const char **argv)
{
	unsigned int i;
	int Sleep_Interval;
	const char **skin_xpm = NULL;
	const char **bkg_xpm = NULL;
	char *skin_file_path = search_path(skin_search_path, skin_filename);
	int wmbiff_mask_height = mbox_y(num_mailboxes) + 4;

	DMA(DEBUG_INFO, "running %u mailboxes w %d h %d\n", num_mailboxes,
		wmbiff_mask_width, wmbiff_mask_height);

	if (skin_file_path != NULL) {
		skin_xpm = (const char **) LoadXPM(skin_file_path);
		free(skin_file_path);
	}
	if (skin_xpm == NULL) {
		DMA(DEBUG_ERROR, "using built-in xpm; %s wasn't found in %s\n",
			skin_filename, skin_search_path);
		skin_xpm = (classic_mode ? wmbiff_classic_master_xpm : wmbiff_master_xpm);
	}

	bkg_xpm = (const char **) CreateBackingXPM(wmbiff_mask_width, wmbiff_mask_height, skin_xpm);

	createXBMfromXPM(wmbiff_mask_bits, (const char**)bkg_xpm,
					 wmbiff_mask_width, wmbiff_mask_height);

	openXwindow(argc, argv, (const char**)bkg_xpm, skin_xpm, wmbiff_mask_bits,
				wmbiff_mask_width, wmbiff_mask_height, notWithdrawn);

	/* now that display is set, we can create the cursors
	   (mouse pointer shapes) */
	busy_cursor = XCreateFontCursor(display, XC_watch);
	ready_cursor = XCreateFontCursor(display, XC_left_ptr);

	if (font != NULL) {
		if (loadFont(font) < 0) {
			DMA(DEBUG_ERROR, "unable to load font. exiting.\n");
			exit(EXIT_FAILURE);
		}
	}

	/* First time setup of button regions and labels */
	for (i = 0; i < num_mailboxes; i++) {
		/* make it easy to recover the mbox index from a mouse click */
		AddMouseRegion(i, x_origin, mbox_y(i), 58, mbox_y(i + 1) - 1);
		if (mbox[i].label[0] != '\0') {
			mbox[i].prevtime = mbox[i].prevfetch_time = 0;
			BlitString(mbox[i].label, x_origin, mbox_y(i), 0);
		}
	}

	do {

		Sleep_Interval = periodic_mail_check();
		ProcessPendingEvents();
		XSleep(Sleep_Interval);
	}
	while (forever);			/* forever is usually true,
								   but not when debugging with -exit */
	if (skin_xpm != NULL && skin_xpm != wmbiff_master_xpm
		&& skin_xpm != wmbiff_classic_master_xpm) {
		free(skin_xpm);			// added 3 jul 02, appeasing valgrind
	}
	if (bkg_xpm != NULL) {
		// Allocated in CreateBackingXPM()
		free((void *)bkg_xpm[0]);
		free((void *)bkg_xpm[2]);
		int mem_block;
		for (mem_block = 6; mem_block < 6 + wmbiff_mask_height; mem_block++)
			free((void *)bkg_xpm[mem_block]);
		free(bkg_xpm);
	}
}

static void sigchld_handler(int sig
#ifdef HAVE___ATTRIBUTE__
							__attribute__ ((unused))
#endif
	)
{
	while (waitpid(0, NULL, WNOHANG) > 0);
	signal(SIGCHLD, sigchld_handler);
}

static void usage(void)
{
	printf("\nwmBiff v%s"
		   " - incoming mail checker\n"
		   "Gennady Belyakov and others (see the README file)\n"
		   "Please report bugs to %s\n"
		   "\n"
		   "usage:\n"
		   "    -bg <color>               background color\n"
		   "    -c <filename>             use specified config file\n"
		   "    -debug                    enable debugging\n"
		   "    -display <display name>   use specified X display\n"
		   "    -fg <color>               foreground color\n"
		   "    -font <font>              font instead of LED\n"
		   "    -geometry +XPOS+YPOS      initial window position\n"
		   "    -h                        this help screen\n"
		   "    -hi <color>               highlight color for new mail\n"
#ifdef USE_GNUTLS
		   "    -skip-certificate-check   using TLS, don't validate the\n"
		   "                              server's certificate\n"
#endif
		   "    -relax                    assume the configuration is \n"
		   "                              correct, parse it without paranoia, \n"
		   "                              and assume hostnames are okay.\n"
		   "    -v                        print the version number\n"
		   "    +w                        not withdrawn: run as a window\n"
		   "\n", PACKAGE_VERSION, PACKAGE_BUGREPORT);
}

static void printversion(void)
{
	printf("wmbiff v%s\n", PACKAGE_VERSION);
}


static void parse_cmd(int argc, const char **argv, char *config_file)
{
	int i;
	int fg = 0, bg = 0, hi = 0;

	config_file[0] = '\0';

	/* Parse Command Line */

	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];

		if (*arg == '-') {
			switch (arg[1]) {
			case 'b':
				if (strcmp(arg + 1, "bg") == 0) {
					if (argc > (i + 1)) {
						background = strdup_ordie(argv[i + 1]);
						bg = 1;
						DMA(DEBUG_INFO, "new background: '%s'\n", foreground);
						i++;
						if (font == NULL)
							font = DEFAULT_FONT;
					}
				}
				break;
			case 'd':
				if (strcmp(arg + 1, "debug") == 0) {
					debug_default = DEBUG_ALL;
				} else if (strcmp(arg + 1, "display") == 0) {
					/* passed to X's command line parser */
				} else {
					usage();
					exit(EXIT_FAILURE);
				}
				break;
			case 'f':
				if (strcmp(arg + 1, "fg") == 0) {
					if (argc > (i + 1)) {
						foreground = strdup_ordie(argv[i + 1]);
						fg = 1;
						DMA(DEBUG_INFO, "new foreground: '%s'\n", foreground);
						i++;
						if (font == NULL)
							font = DEFAULT_FONT;
					}
				} else if (strcmp(arg + 1, "font") == 0) {
					if (argc > (i + 1)) {
						if (strcmp(argv[i + 1], "default") == 0) {
							font = DEFAULT_FONT;
						} else {
							font = strdup_ordie(argv[i + 1]);
						}
						DMA(DEBUG_INFO, "new font: '%s'\n", font);
						i++;
					}
				} else {
					usage();
					exit(EXIT_FAILURE);
				}
				break;
			case 'g':
				if (strcmp(arg + 1, "geometry") != 0) {
					usage();
					exit(EXIT_FAILURE);
				} else {
					i++;		/* gobble the argument */
					if (i >= argc) {	/* fail if there's nothing to gobble */
						usage();
						exit(EXIT_FAILURE);
					}
				}
				break;
			case 'h':
				if (strcmp(arg + 1, "hi") == 0) {
					if (argc > (i + 1)) {
						highlight = strdup_ordie(argv[i + 1]);
						hi = 1;
						DMA(DEBUG_INFO, "new highlight: '%s'\n", highlight);
						i++;
						if (font == NULL)
							font = DEFAULT_FONT;
					}
				} else if (strcmp(arg + 1, "h") == 0) {
					usage();
					exit(EXIT_SUCCESS);
				}
				break;
			case 'v':
				printversion();
				exit(EXIT_SUCCESS);
				break;
			case 's':
				if (strcmp(arg + 1, "skip-certificate-check") == 0) {
					SkipCertificateCheck = 1;
				} else {
					usage();
					exit(EXIT_SUCCESS);
				}

				break;
			case 'r':
				if (strcmp(arg + 1, "relax") == 0) {
					Relax = 1;
				} else {
					usage();
					exit(EXIT_SUCCESS);
				}

				break;
			case 'c':
				if (argc > (i + 1)) {
					strncpy(config_file, argv[i + 1], 255);
					i++;
				}
				break;
			case 'e':			/* undocumented for debugging */
				if (strcmp(arg + 1, "exit") == 0) {
					forever = 0;
				}
				break;
			case 'o':			/* use classic behaviour/theme */
				classic_mode = 1;
				break;
			default:
				usage();
				exit(EXIT_SUCCESS);
				break;
			}
		} else if (*arg == '+') {
			switch (arg[1]) {
			case 'w':
				notWithdrawn = 1;
				break;
			default:
				usage();
				exit(EXIT_SUCCESS);
				break;
			}
		}
	}

	if (classic_mode) {
		/* load classic colors if user did not override them */
		if(!fg)
			foreground = foreground_classic;
		if(!bg)
			background = background_classic;
		if(!hi)
			highlight = highlight_classic;
	}
}

int main(int argc, const char *argv[])
{
	char uconfig_file[256];

	/* hold on to the arguments we were started with; we
	   will need them if we have to restart on sigusr1 */
	restart_args =
		(const char **) malloc((argc + 1) * sizeof(const char *));
	if (restart_args) {
		memcpy(restart_args, argv, (argc) * sizeof(const char *));
		restart_args[argc] = NULL;
	}

	parse_cmd(argc, argv, uconfig_file);

	/* decide what the config file is */
	if (uconfig_file[0] != '\0') {	/* user-specified config file */
		DMA(DEBUG_INFO, "Using user-specified config file '%s'.\n",
			uconfig_file);
	} else {
		const char *home = getenv("HOME");
		if (home == NULL) {
			DMA(DEBUG_ERROR,
				"$HOME undefined. Use the -c option to specify a wmbiffrc\n");
			exit(EXIT_FAILURE);
		}
		sprintf(uconfig_file, "%s/.wmbiffrc", home);
	}

	if (wmbiffrc_permissions_check(uconfig_file) == 0) {
		DMA(DEBUG_ERROR,
			"WARNING: In future versions of WMBiff, .wmbiffrc MUST be\n"
			"owned by the user, and not readable or writable by others.\n\n");
	}
	init_biff(uconfig_file);
	signal(SIGCHLD, sigchld_handler);
	signal(SIGUSR1, restart_wmbiff);
	signal(SIGPIPE, SIG_IGN);	/* write() may fail */

	do_biff(argc, argv);

	// free resources
	if (restart_args)
		free(restart_args);
	if (custom_skin)
		free((void *)skin_filename);

	return 0;
}
