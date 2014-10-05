/*
 * pop3.c -- Routines for communication with a pop3 server
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

/* Define filename_M */
#define POP3_M 1

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "common.h"
#include "wmnotify.h"
#include "network.h"
#include "pop3.h"


/* Defined in network.c */
extern char tx_buffer[WMNOTIFY_BUFSIZE + 1];
extern char rx_buffer[WMNOTIFY_BUFSIZE + 1];


static int
POP3_ReceiveResponse( void )
{
  int len;

  len = WmnotifyGetResponse( rx_buffer, WMNOTIFY_BUFSIZE );
  if( len < 0 ) {
    perror( PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    return len;
  }

  rx_buffer[ len - 2 ] = '\0';

  if( wmnotify_infos.debug ) {
    printf( "Response: \"%s\"\n", rx_buffer );
  }

  /* No error in recv at this point. Now we parse response from POP3 server. */

  /* Check the status indicator returned by the POP3 server.
     There are currently two status indicators: positive ("+OK") and negative
     ("-ERR"). Servers MUST send the status indicators in upper case. */
  if( STREQ_LEN( rx_buffer, POP3_RSP_SUCCESS, strlen(POP3_RSP_SUCCESS) ) == false ) {
    fprintf( stderr, "%s: Error, POP3 server responded:\n  \"%s\"\n", PACKAGE, rx_buffer );
    len = -1;
  }

  return len;
}


static int
POP3_SendCommand( int argc, char *argv[] )
{
  int len;
  int i;

  /* Adding command and it's arguments. */
  for( i = 0, len = 0; i < argc; i++ ) {
    len += sprintf( tx_buffer + len, "%s", argv[i] );
    if( i != ( argc - 1 ) ) {
      len += sprintf( tx_buffer + len, " " );
    }
  }

  if( wmnotify_infos.debug ) {
    tx_buffer[len] = '\0';
    printf( "Command: \"%s\"\n", tx_buffer );
  }

  /* Adding termination characters. */
  len += sprintf( tx_buffer + len, POP3_ENDL );

  len = WmnotifySendData( tx_buffer, len );
  if( len < 0 ) {
    return EXIT_FAILURE;
  }

  len = POP3_ReceiveResponse();
  if( len < 0 ) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


/* Return the number of new messages on success, -1 on error. */
static int
POP3_ParseStatCommand( void )
{
  int new_messages;
  char *token;

  /* STAT command:
   * The positive response consists of "+OK" followed by a single space, the number of messages
   * in the maildrop, a single space, and the size of the maildrop in octets. */
  token = strtok( rx_buffer, " " );
  token = strtok( NULL, " " );
  if( token != NULL ) {
    /* Do more checks for digits... */
    new_messages = atoi( token );
  }
  else {
    fprintf( stderr, "%s: Error parsing \"STAT\" response", PACKAGE );
    new_messages = -1;
  }

  return new_messages;
}


int
POP3_CheckForNewMail( void )
{
  int status;
  int new_messages = -1;
  char *argv[10];

  status = ConnectionEstablish( wmnotify_infos.server_name, wmnotify_infos.port );
  if( status != EXIT_SUCCESS ) {
    return -1;
  }

  /* Sending username. */
  argv[0] = POP3_CMD_USERNAME;
  argv[1] = wmnotify_infos.username;
  status = POP3_SendCommand( 2, argv );
  if( status != EXIT_SUCCESS ) {
    goto pop3_close_connection;
  }

  /* Sending password. */
  argv[0] = POP3_CMD_PASSWORD;
  argv[1] = wmnotify_infos.password;
  status = POP3_SendCommand( 2, argv );
  if( status != EXIT_SUCCESS ) {
    goto pop3_close_connection;
  }

  /* Sending STAT command to inquiry about new messages. */
  argv[0] = POP3_CMD_STAT;
  status = POP3_SendCommand( 1, argv );
  if( status != EXIT_SUCCESS ) {
    goto pop3_close_connection;
  }

  /* Parsing STAT command. */
  new_messages = POP3_ParseStatCommand();
  if( new_messages < 0 ) {
    goto pop3_close_connection;
  }

  /* Sending QUIT command. */
  argv[0] = POP3_CMD_QUIT;
  status = POP3_SendCommand( 1, argv );
  if( status != EXIT_SUCCESS ) {
    new_messages = -1;
    goto pop3_close_connection;
  }

 pop3_close_connection:
  status = ConnectionTerminate();
  if( status != EXIT_SUCCESS ) {
    new_messages = -1;
  }

  return new_messages;
}
