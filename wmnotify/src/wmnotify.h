/*
 * wmnotify.h
 *
 * Copyright (C) 2003 Hugo Villeneuve <hugo@hugovil.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef WMNOTIFY_H
#define WMNOTIFY_H 1

#define POP3_PROTOCOL  0
#define IMAP4_PROTOCOL 1

/* New messages animation duration, in microseconds. */
#define NEW_MAIL_ANIMATION_DURATION 900000

/* Flag used in the new messages animation to identify which image is currently
   displayed. */
#define MAILBOX_CLOSED 0
#define MAILBOX_FULL   1

/* Source coordinates in global pixmap for the closed mailbox image. */
#define MAILBOX_CLOSED_SRC_X 64
#define MAILBOX_CLOSED_SRC_Y 4

/* Source coordinates in global pixmap for the opened and empty mailbox image. */
#define MAILBOX_OPENED_EMPTY_SRC_X 64
#define MAILBOX_OPENED_EMPTY_SRC_Y 64

/* Source coordinates in global pixmap for the opened and full mailbox image. */
#define MAILBOX_OPENED_FULL_SRC_X 4
#define MAILBOX_OPENED_FULL_SRC_Y 64

/* Source coordinates in global pixmap for the opened and full mailbox image. */
#define EXEC_CMD_IMG_SRC_X 124
#define EXEC_CMD_IMG_SRC_Y 4

/* Size of all mailbox images. */
#define MAILBOX_SIZE_X 56
#define MAILBOX_SIZE_Y 56

/* Destination coordinates when copying a mailbox image. */
#define MAILBOX_DEST_X 4
#define MAILBOX_DEST_Y 4

#define ARGV_LIMIT 64

#define MAX_STR_LEN 256

typedef struct wmnotify_t
{
  bool debug;
  char *display_arg;
  char *geometry_arg;
  char *optional_config_file;
  char mail_client_command[512];
  char *mail_client_argv[ARGV_LIMIT];
  unsigned int mail_check_interval; /* In seconds. */
  bool audible_notification;
  char audiofile[512];
  int volume;
  int protocol;
  char imap_folder[MAX_STR_LEN];
  bool use_ssl;
  char server_name[MAX_STR_LEN];
  int port;
  char username[MAX_STR_LEN];
  char password[MAX_STR_LEN];
  int sock_fd;
} wmnotify_t;

/* Exported variables */
#undef _SCOPE_
#ifdef WMNOTIFY_M
#  define _SCOPE_ /**/
#else
#  define _SCOPE_ extern
#endif

_SCOPE_ wmnotify_t wmnotify_infos;

#endif /* WMNOTIFY_H */
