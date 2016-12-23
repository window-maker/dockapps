/*********************************************************************
 *
 * Filename:      obex_io.c
 * Version:       0.3
 * Description:   Some useful disk-IO functions
 * Status:        Experimental.
 * Author:        Pontus Fuchs <pontus.fuchs@tactel.se>
 * Created at:    Mon Aug 07 19:32:14 2000
 * Modified at:   Mon Aug 07 19:32:14 2000
 * Modified by:   Pontus Fuchs <pontus.fuchs@tactel.se>
 * Modified at:   Sun Oct 06 10:00:00 2001
 * Modified by:   Ben Moore <ben@netjunki.org>
 *
 *     Copyright (c) 1999 Dag Brattli, All Rights Reserved.
 *
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of
 *     the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *     MA 02111-1307 USA
 *
 ********************************************************************/

#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif /*_WIN32 */

#include <fcntl.h>
#include <string.h>

#include <glib.h>
#include <openobex/obex.h>

#include "obex_io.h"

extern obex_t *handle;
int obex_protocol_type = OBEX_PROTOCOL_GENERIC;

//
// Get the filesize in a "portable" way
//
gint get_filesize(const char *filename)
{
#ifdef _WIN32
	HANDLE fh;
	gint size;
	fh = CreateFile(filename, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if(fh == INVALID_HANDLE_VALUE) {
		g_print("Cannot open %s\n", filename);
		return -1;
	}
	size = GetFileSize(fh, NULL);
	g_print("fize size was %d\n", size);
	CloseHandle(fh);
	return size;

#else
	struct stat stats;
	/*  Need to know the file length */
	stat(filename, &stats);
	return (gint) stats.st_size;
#endif
}


//
// Read a file and alloc a buffer for it
//
guint8* easy_readfile(const char *filename, int *file_size)
{
	int actual;
	int fd;
	guint8 *buf;

	*file_size = get_filesize(filename);
	g_print("name=%s, size=%d\n", filename, *file_size);

#ifdef _WIN32
	fd = open(filename, O_RDONLY | O_BINARY, 0);
#else
	fd = open(filename, O_RDONLY, 0);
#endif

	if (fd == -1) {
		return NULL;
	}

	if(! (buf = g_malloc(*file_size)) )	{
		return NULL;
	}

	actual = read(fd, buf, *file_size);
	close(fd);

#ifdef _WIN32
	if(actual != *file_size) {
		g_free(buf);
		buf = NULL;
	}
#else
	*file_size = actual;
#endif
	return buf;
}


//
//
//
obex_object_t *build_object_from_file(obex_t *handle, const char *filename)
{
	obex_headerdata_t hdd;
	guint8 unicode_buf[200];
	gint namebuf_len;
 	obex_object_t *object;
	guint32 creator_id;
	int file_size;
	char *name = NULL;
	guint8 *buf;


	buf = easy_readfile(filename, &file_size);
	if(buf == NULL)
		return NULL;

	/* Set Memopad as the default creator ID */
	creator_id = MEMO_PAD;

	/* Find the . in the filename */
	name = strchr(filename, '.');
	if (name) {
		name++;
		if (strcmp(name, "vcf") == 0) {
			printf( "This is a Address Book file\n");
			creator_id = ADDRESS_BOOK;
		} else if (strcmp(name, "vcs") == 0) {
			printf( "This is a Date Book file\n");
			creator_id = DATE_BOOK;
		} else if (strcmp(name, "txt") == 0) {
			printf("This is a Memo pad file\n");
			creator_id = MEMO_PAD;
		} else if (strcmp(name, "prc") == 0) {
			printf("This is a Pilot resource file\n");
			creator_id = PILOT_RESOURCE;
		}
	}
	/* Build object */
	object = OBEX_ObjectNew(handle, OBEX_CMD_PUT);

	namebuf_len = OBEX_CharToUnicode(unicode_buf, filename, sizeof(unicode_buf));

	hdd.bs = unicode_buf;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_NAME,
				hdd, namebuf_len, 0);

	hdd.bq4 = file_size;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_LENGTH,
				hdd, sizeof(guint32), 0);

#if 0
	/* Optional header for win95 irxfer, allows date to be set on file */
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_TIME2,
				(obex_headerdata_t) (guint32) stats.st_mtime,
				sizeof(guint32), 0);
#endif
	if (obex_protocol_type != 1) {
		/* Optional header for Palm Pilot */
		/* win95 irxfer does not seem to like this one */
		hdd.bq4 = creator_id;
		OBEX_ObjectAddHeader(handle, object, HEADER_CREATOR_ID,
					hdd, sizeof(guint32), 0);
	}

	hdd.bs = buf;
	OBEX_ObjectAddHeader(handle, object, OBEX_HDR_BODY,
				hdd, file_size, 0);

	g_free(buf);
	return object;
}


/*
 * Function safe_save_file ()
 *
 *    First remove path and add "/tmp/". Then save.
 *
 */
gint safe_save_file(gchar *name, const guint8 *buf, gint len, gchar *savedir)
{
	gchar *s = NULL;
	gchar filename[255] = {0,};
	gint fd;
	gint actual;


	g_print("Filename = %s\n", name);

#ifndef _WIN32
	sprintf( filename, "%s", savedir);
#endif
	s = strrchr(name, '/');
	if (s == NULL)
		s = name;
	else
		s++;

	strncat(filename, s, 250);
#ifdef _WIN32
	fd = open(filename, O_RDWR | O_CREAT, 0);
#else
	fd = open(filename, O_RDWR | O_CREAT, DEFFILEMODE);
#endif

	if ( fd < 0) {
		perror( filename);
		return -1;
	}

	actual = write(fd, buf, len);
	close(fd);

	g_print( "Wrote %s (%d bytes)\n", filename, actual);

	return actual;
}
