/* $Id: maildirClient.c,v 1.15 2004/03/28 00:28:58 bluehal Exp $ */
/* Author : Yong-iL Joh ( tolkien@mizi.com )
   Modified : Jorge García ( Jorge.Garcia@uv.es )
   Modified : Dwayne C. Litzenberger ( dlitz@dlitz.net )
 *
 * Maildir checker.
 *
 * Last Updated : $Date: 2004/03/28 00:28:58 $
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Client.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <utime.h>
#include <unistd.h>
#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif


#define PCM	(pc->u).maildir

static int count_msgs(char *path)
{
	DIR *D;
	struct dirent *de;
	int count = 0;

	D = opendir(path);
	if (D == NULL) {
		DMA(DEBUG_ERROR,
			"Error opening directory '%s': %s\n", path, strerror(errno));
		return -1;
	}

	while ((de = readdir(D)) != NULL) {
		if ((strcmp(de->d_name, ".") & strcmp(de->d_name, "..")) != 0) {
			count++;
		}
	}
	closedir(D);

	return count;
}

int maildirCheckHistory(Pop3 pc)
{
	struct stat st_new;
	struct stat st_cur;
	struct utimbuf ut;
	char path_new[BUF_BIG * 2], path_cur[BUF_BIG * 2];

	int count_new = 0, count_cur = 0;

	DM(pc, DEBUG_INFO, ">Maildir: '%s'\n", pc->path);

	strcpy(path_new, pc->path);
	strcat(path_new, "/new/");
	strcpy(path_cur, pc->path);
	strcat(path_cur, "/cur/");

	if (pc->u.maildir.dircache_flush) {
		/* hack to clear directory cache for network-mounted maildirs */
		int fd;
		char path_newtmp[BUF_BIG * 2 + 32];
		strcpy(path_newtmp, path_new);
		strcat(path_newtmp, ".wmbiff.dircache_flush.XXXXXX");
		if ((fd = mkstemp(path_newtmp)) >= 0) {
			close(fd);
			unlink(path_newtmp);
		} else {
			DM(pc, DEBUG_ERROR,
			   "Can't create dircache flush file '%s': %s\n",
			   path_newtmp, strerror(errno));
		}
	}

	/* maildir */
	if (stat(path_new, &st_new)) {
		DM(pc, DEBUG_ERROR, "Can't stat mailbox '%s': %s\n",
		   path_new, strerror(errno));
		return -1;				/* Error stating mailbox */
	}
	if (stat(path_cur, &st_cur)) {
		DM(pc, DEBUG_ERROR, "Can't stat mailbox '%s': %s\n",
		   path_cur, strerror(errno));
		return -1;				/* Error stating mailbox */
	}


	/* file was changed OR initially read */
	if (st_new.st_mtime != PCM.mtime_new
		|| st_new.st_size != PCM.size_new
		|| st_cur.st_mtime != PCM.mtime_cur
		|| st_cur.st_size != PCM.size_cur || pc->OldMsgs < 0) {
		DM(pc, DEBUG_INFO, "  was changed,\n"
		   " TIME(new): old %lu, new %lu"
		   " SIZE(new): old %lu, new %lu\n"
		   " TIME(cur): old %lu, new %lu"
		   " SIZE(cur): old %lu, new %lu\n",
		   PCM.mtime_new, (unsigned long) st_new.st_mtime,
		   (unsigned long) PCM.size_new, (unsigned long) st_new.st_size,
		   PCM.mtime_cur, (unsigned long) st_cur.st_mtime,
		   (unsigned long) PCM.size_cur, (unsigned long) st_cur.st_size);

		count_new = count_msgs(path_new);
		count_cur = count_msgs(path_cur);
		if ((count_new | count_cur) == -1) {	/* errors occurred */
			return -1;
		}

		pc->TotalMsgs = count_cur + count_new;
		pc->UnreadMsgs = count_new;

		/* Reset atime for MUTT and something others work correctly */
		ut.actime = st_new.st_atime;
		ut.modtime = st_new.st_mtime;
		utime(path_new, &ut);
		ut.actime = st_cur.st_atime;
		ut.modtime = st_cur.st_mtime;
		utime(path_cur, &ut);

		/* Store new values */
		PCM.mtime_new = st_new.st_mtime;	/* Store new mtime_new */
		PCM.size_new = st_new.st_size;	/* Store new size_new */
		PCM.mtime_cur = st_cur.st_mtime;	/* Store new mtime_cur */
		PCM.size_cur = st_cur.st_size;	/* Store new size_cur */
	}

	return 0;
}

int maildirCreate(Pop3 pc, const char *str)
{
	int i;
	char c;
	/* Maildir format: maildir:fullpathname */

	pc->TotalMsgs = 0;
	pc->UnreadMsgs = 0;
	pc->OldMsgs = -1;
	pc->OldUnreadMsgs = -1;
	pc->checkMail = maildirCheckHistory;
	pc->u.maildir.dircache_flush = 0;

	/* special flags */
	if (*(str + 8) == ':') {	/* path is of the format maildir::flags:path */
		c = ' ';
		for (i = 1; c != ':' && c != '\0'; i++) {
			c = *(str + 8 + i);
			switch (c) {
			case 'F':
				pc->u.maildir.dircache_flush = 1;
				DM(pc, DEBUG_INFO, "maildir: dircache_flush enabled\n");
			}
		}
	} else {
		i = 0;
	}
	if (strlen(str + 8 + i) + 1 > BUF_BIG) {
		DM(pc, DEBUG_ERROR, "maildir '%s' is too long.\n", str + 8 + i);
		memset(pc->path, 0, BUF_BIG);
	} else {
		strncpy(pc->path, str + 8 + i, BUF_BIG - 1);	/* cut off ``maildir:'' */
	}

	DM(pc, DEBUG_INFO, "maildir: str = '%s'\n", str);
	DM(pc, DEBUG_INFO, "maildir: path= '%s'\n", pc->path);

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
