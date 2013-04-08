/* $Id: mboxClient.c,v 1.17 2004/03/28 00:28:58 bluehal Exp $ */
/* Author:		Yong-iL Joh <tolkien@mizi.com>
   Modified:	Jorge García <Jorge.Garcia@uv.es>
   			 	Rob Funk <rfunk@funknet.net>
   			 	Neil Spring <nspring@cs.washington.edu>
 * 
 * MBOX checker.
 *
 * Last Updated : $Date: 2004/03/28 00:28:58 $
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Client.h"
#include <sys/stat.h>
#include <errno.h>
#include <utime.h>
#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

#define PCM	(pc->u).mbox
#define FROM_STR   "From "
#define STATUS_STR "Status: "

FILE *openMailbox(Pop3 pc, const char *mbox_filename)
{
	FILE *mailbox;

	if ((mailbox = fopen(mbox_filename, "r")) == NULL) {
		DM(pc, DEBUG_ERROR, "Error opening mailbox '%s': %s\n",
		   mbox_filename, strerror(errno));
		pc->TotalMsgs = -1;
		pc->UnreadMsgs = -1;
	}
	return (mailbox);
}

/* count the messages in a mailbox */
static void countMessages(Pop3 pc, const char *mbox_filename)
{
	FILE *F;
	char buf[BUF_SIZE];
	int is_header = 0;
	int next_from_is_start_of_header = 1;
	int count_from = 0, count_status = 0;
	int len_from = strlen(FROM_STR), len_status = strlen(STATUS_STR);
	int pseudo_mail = 0;

	F = openMailbox(pc, mbox_filename);
	if (F == NULL)
		return;

	/* count message */
	while (fgets(buf, BUF_SIZE, F)) {
		// The first message usually is automatically created by POP3/IMAP
		// clients for internal record keeping and is ignored
		// (not displayed) by most email clients.
		if (is_header && !strncmp(buf, "X-IMAP: ", 8))
		{
			pseudo_mail = 1;
		}
		if (buf[0] == '\n') {
			/* a newline by itself terminates the header */
			if (is_header)
				is_header = 0;
			else
				next_from_is_start_of_header = 1;
		} else if (!strncmp(buf, FROM_STR, len_from)) {
			/* A line starting with "From" is the beginning of a new header.
			   "From" in the text of the mail should get escaped by the MDA.
			   If your MDA doesn't do that, it is broken.
			 */
			if (next_from_is_start_of_header)
				is_header = 1;
			if (is_header)
				count_from++;
		} else {
			next_from_is_start_of_header = 0;
			if (is_header && !strncmp(buf, STATUS_STR, len_status)
				&& strrchr(buf, 'R')) {
				count_status++;
			}
		}
	}

	if (count_from && pseudo_mail) {
		count_from--;
		if (count_status)
			count_status--;
	}

	DM(pc, DEBUG_INFO, "from: %d status: %d\n", count_from, count_status);
	pc->TotalMsgs = count_from;
	pc->UnreadMsgs = count_from - count_status;
	fclose(F);
}

/* check file status; hold on to file information used 
   to restore access time */
int
fileHasChanged(const char *mbox_filename, time_t * atime,
			   time_t * mtime, off_t * size)
{
	struct stat st;

	/* mbox file */
	if (stat(mbox_filename, &st)) {
		DMA(DEBUG_ERROR, "Can't stat '%s': %s\n",
			mbox_filename, strerror(errno));
	} else if (st.st_mtime != *mtime || st.st_size != *size) {
		/* file was changed OR initially read */
		DMA(DEBUG_INFO, " %s was changed,"
			" mTIME: %lu -> %lu; SIZE: %lu -> %lu\n",
			mbox_filename, *mtime, st.st_mtime,
			(unsigned long) *size, (unsigned long) st.st_size);

		*atime = st.st_atime;
		*mtime = st.st_mtime;
		*size = st.st_size;
		return 1;
	}
	return 0;
}

int mboxCheckHistory(Pop3 pc)
{
	char *mbox_filename = backtickExpand(pc, pc->path);
	struct utimbuf ut;

	DM(pc, DEBUG_INFO, ">Mailbox: '%s'\n", mbox_filename);

	if (fileHasChanged(mbox_filename, &ut.actime, &PCM.mtime, &PCM.size)
		|| pc->OldMsgs < 0) {

		countMessages(pc, mbox_filename);

		/* Reset atime for (at least) MUTT to work */
		/* ut.actime is set above */
		ut.modtime = PCM.mtime;
		utime(mbox_filename, &ut);
	}
	free(mbox_filename);
	return 0;
}

int mboxCreate(Pop3 pc, const char *str)
{
	/* MBOX format: mbox:fullpathname */

	pc->TotalMsgs = 0;
	pc->UnreadMsgs = 0;
	pc->OldMsgs = -1;
	pc->OldUnreadMsgs = -1;
	pc->checkMail = mboxCheckHistory;

	/* default boxes are mbox... cut mbox: if it exists */
	if (!strncasecmp(pc->path, "mbox:", 5)) {
		if (strlen(str + 5) + 1 > BUF_BIG) {
			DM(pc, DEBUG_ERROR, "mbox '%s' is too long.\n", str + 5);
			memset(pc->path, 0, BUF_BIG);
		} else {
			strncpy(pc->path, str + 5, BUF_BIG - 1);	/* cut off ``mbox:'' */
		}
	}

	DM(pc, DEBUG_INFO, "mbox: str = '%s'\n", str);
	DM(pc, DEBUG_INFO, "mbox: path= '%s'\n", pc->path);

	return 0;
}

/* vim:set ts=4: */
/*
 * Local Variables:
 * tab-width: 4
 * c-indent-level: 4
 * c-basic-offset: 4
 * End:
 */
