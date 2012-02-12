/*
 * pop3.h
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

#ifndef POP3_H
#define POP3_H 1

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Exported variables */
#undef _SCOPE_
#ifdef POP3_M
#  define _SCOPE_ /**/
#else
#  define _SCOPE_ extern
#endif

#define POP3_ENDL "\r\n" /* CRLF */

#define POP3_CMD_USERNAME "USER"
#define POP3_CMD_PASSWORD "PASS"
#define POP3_CMD_STAT     "STAT"
#define POP3_CMD_QUIT     "QUIT"

#define POP3_RSP_SUCCESS "+OK"
#define POP3_RSP_FAILURE "-ERR"

int
POP3_CheckForNewMail( void );

#endif /* POP3_H */
