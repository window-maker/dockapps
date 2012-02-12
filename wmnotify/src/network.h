/*
 * network.h
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

#ifndef NETWORK_H
#define NETWORK_H 1

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* POP3 responses may be up to 512 characters long, including the terminating
   CRLF. IMAP4 responses can be more than 1024 characters. */
#define WMNOTIFY_BUFSIZE 10240

int
SocketOpen( char *server_name, int port );

int
ConnectionEstablish( char *server_name, int port );

int
ConnectionTerminate( void );

int
WmnotifySendData( char *buffer, int size );

int
WmnotifyGetResponse( char *buffer, int max_size );

#endif /* NETWORK_H */
