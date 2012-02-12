/*
 * imap.c -- Routines for communication with an IMAP server
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
#define IMAP_M 1

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
#include <arpa/inet.h>
#include <ctype.h> /* for isdigit() */

#include "common.h"
#include "wmnotify.h"
#include "network.h"
#include "imap.h"


#define IMAP4_ENDL "\r\n" /* CRLF */

#define IMAP4_CMD_CAPABILITY    "CAPABILITY"
#define IMAP4_CMD_LOGIN         "LOGIN"
#define IMAP4_CMD_SELECT        "SELECT"
#define IMAP4_CMD_EXAMINE       "EXAMINE"
#define IMAP4_CMD_LOGOUT        "LOGOUT"
#define IMAP4_CMD_SEARCH_UNSEEN "SEARCH UNSEEN"

/* Responses from IMAP4 server. */
#define IMAP4_RSP_SUCCESS      "OK"
#define IMAP4_RSP_FAILURE      "NO"
#define IMAP4_RSP_PROTOCOL_ERR "BAD"
#define IMAP4_RSP_SEARCH_UNSEEN "* SEARCH " /* This is the line that will be returned by
					     * the IMAP4 server after receiving the
					     * "SEARCH UNSEEN" command, followed by the
					     * messages ID of the unseen messages. */


static int tlabel = 0;
static int tlabel_len;
static int unseen_string_found;

/* Defined in network.c */
extern char tx_buffer[WMNOTIFY_BUFSIZE + 1];
extern char rx_buffer[WMNOTIFY_BUFSIZE + 1];


static int
IMAP4_ReceiveResponse( void )
{
  int len;
  char *token;
  char *stringp;
  
  /* All interactions transmitted by client and server are in the form of
     lines, that is, strings that end with a CRLF.  The protocol receiver
     of an IMAP4rev1 client or server is either reading a line, or is
     reading a sequence of octets with a known count followed by a line. */
  
 get_packet:
  len = WmnotifyGetResponse( rx_buffer, WMNOTIFY_BUFSIZE );
  if( len < 0 ) {
    /* An error occured. WmnotifyGetResponse() should have printed an error message. */
    goto error;
  }
  else if( len == 0 ) {
    /* The return value will be 0 when the peer has performed an orderly shutdown. */
    if( wmnotify_infos.debug ) {
      fprintf( stderr, "IMAP server has closed connection.\n" );
    }
    goto error;
  }
  else if( len == WMNOTIFY_BUFSIZE ) {
    if( wmnotify_infos.debug ) {
      ErrorLocation( __FILE__, __LINE__ );
      fprintf( stderr, "Response too big (%d bytes) to fit in receive buffer.\n", len );
    }
    goto error;
  }

  /* We suppose that, if a partial response packet was sent, it is not broken in the middle
     of a line (to confirm). Normally, each string is terminated by CRLF. */
  if( STREQ_LEN( &rx_buffer[ len - 2 ], IMAP4_ENDL, 2 ) == false ) {
    /* No CRLF found at the end of the buffer --> not handled by wmnotify. */
    ErrorLocation( __FILE__, __LINE__ );
    fprintf( stderr, "Response buffer doesn't contain CRLF at the end.\n" );
    goto error;
  }

  if( wmnotify_infos.debug ) {
    printf( "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n" );
    printf( "IMAP4 Server Response (size %d bytes):\n", len );
    printf( "%s", rx_buffer );
    printf( "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n" );
  }
      
  /* Converting the last CRLF into a LF followed by a NULL termination character. */
  rx_buffer[ len - 2 ] = '\n';
  rx_buffer[ len - 1 ] = '\0';
  
  /* Check the Server Completion Response returned by the IMAP4 server. There are currently
   * three Server Completion Responses codes: success ("OK"), failure ("NO") and protocol error
   * ("BAD"). */
  stringp = rx_buffer;
  
  while( ( token = strsep( &stringp, "\n" ) ) != NULL ) {

    /* In case no delimiter was found, the token is  taken  to
       be the entire string *stringp, and *stringp is made NULL. */
    if( stringp == NULL ) {
      if( token[0] == '\0' ) {
	/* This means we finished parsing the last line of the buffer, but we need to
	   get more data to continue process the next part of the IMAP4 response. */
	goto get_packet;
      }
      else {
	/* This should never happen. */
	ErrorLocation( __FILE__, __LINE__ );
	fprintf( stderr, "  Delimiter not found in strsep() call.\n" );
	goto error;
      }
    }
    
    if( token == NULL ) {
      /* This should never happen. */
      ErrorLocation( __FILE__, __LINE__ );
      fprintf( stderr, "  NULL token returned by strsep().\n" );
      goto error;
    }
    
    if( token[0] == '*' ) {
      /* Untagged response. If there is a space after the SEARCH response, it means
       * at least 1 message is unseen. */
      if( STREQ_LEN( token, IMAP4_RSP_SEARCH_UNSEEN, strlen(IMAP4_RSP_SEARCH_UNSEEN) ) == true ) {
	unseen_string_found = true;
      }
    }
    else {
      /* Must be the status... */

      /* We check for the correct transaction label plus a space. */
      if( STREQ_LEN( token, tx_buffer, tlabel_len + 1 ) == true ) {
	token += tlabel_len + 1;
	if( STREQ_LEN( token, IMAP4_RSP_SUCCESS, strlen(IMAP4_RSP_SUCCESS) ) == true ) {
	  goto end; /* OK, no errors. */
	}
	else if( STREQ_LEN( token, IMAP4_RSP_PROTOCOL_ERR, strlen(IMAP4_RSP_PROTOCOL_ERR) ) == true ) {
	  fprintf( stderr, "%s: Protocol error (%s).\n", PACKAGE, token );
	  goto error;
	}
	else if( STREQ_LEN( token, IMAP4_RSP_FAILURE, strlen(IMAP4_RSP_FAILURE) ) == true ) {
	  fprintf( stderr, "%s: Failure (%s).\n", PACKAGE, token );
	  goto error;
	}
	else {
	  fprintf( stderr, "%s: Unknown error code (%s).\n", PACKAGE, token );
	  goto error;
	}
      }
      else {
	fprintf( stderr, "%s: Error, transaction label mismatch.\n", PACKAGE );
	goto error;
      }
    }
  } /* while( token ) */
  
  /* Get next part of IMAP4 response. */
  goto get_packet;
  
 end:
  /* No error. */
  return len;
  
 error:
  return -1;
}


static int
IMAP4_SendCommand( int argc, char *argv[] )
{
  int len;
  int i;
  
  /* Adding Transaction Label. */
  tlabel++;
  tx_buffer[0] = 'A';
  len = 1;
  len += sprintf( tx_buffer + len, "%d", tlabel );
  tlabel_len = len;

  /* Adding command and it's arguments. */
  for( i = 0; i < argc; i++ ) {
    len += sprintf( tx_buffer + len, " %s", argv[i] );
  }

  if( wmnotify_infos.debug ) {
    tx_buffer[len] = '\0';
    printf( ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n" );
    printf( "IMAP4 Client Command (size %d bytes):\n%s\n", len, tx_buffer );
    printf( ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n" );
  }

  /* Adding termination characters. */
  len += sprintf( tx_buffer + len, IMAP4_ENDL );

  len = WmnotifySendData( tx_buffer, len );
  if( len < 0 ) {
    return EXIT_FAILURE;
  }
  
  len = IMAP4_ReceiveResponse();
  if( len < 0 ) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


int
IMAP4_CheckForNewMail( void )
{
  char *argv[10];
  int new_messages = 0;
  int status;

  status = ConnectionEstablish( wmnotify_infos.server_name, wmnotify_infos.port );
  if( status != EXIT_SUCCESS ) {
    new_messages = -1;
    goto end;
  }

  argv[0] = IMAP4_CMD_LOGIN;
  argv[1] = wmnotify_infos.username;
  argv[2] = wmnotify_infos.password;
  status = IMAP4_SendCommand( 3, argv );
  if( status != EXIT_SUCCESS ) {
    new_messages = -1;
    goto imap4_logout;
  }

  /* Selecting the mailbox first. */
  argv[0] = IMAP4_CMD_EXAMINE;
  argv[1] = wmnotify_infos.imap_folder;
  status = IMAP4_SendCommand( 2, argv );
  if( status != EXIT_SUCCESS ) {
    new_messages = -1;
    goto imap4_logout;
  }

  /* Searching in selected mailbox for new messages. We must use the UNSEEN search criteria
   * instead of NEW (combination of RECENT and UNSEEN). If there is a new message, RECENT
   * and UNSEEN will have entries. But if we recheck again later, RECENT will report zero.
   * RECENT, when set, simply means that there are new messages since our last visit.
     But, on the other hand, when using EXAMINE, no messages should lose their RECENT flag. */
  unseen_string_found = false;
  argv[0] = IMAP4_CMD_SEARCH_UNSEEN;
  argv[1] = "";
  status = IMAP4_SendCommand( 1, argv );
  if( status != EXIT_SUCCESS ) {
    new_messages = -1;
    goto imap4_logout;
  }
  
  if( unseen_string_found == true ) {
    new_messages = 1;
  }

 imap4_logout:
  argv[0] = IMAP4_CMD_LOGOUT;
  status = IMAP4_SendCommand( 1, argv );
  if( status != EXIT_SUCCESS ) {
    new_messages = -1;
  }

  status = ConnectionTerminate();
  if( status != EXIT_SUCCESS ) {
    new_messages = -1;
  }
  
 end:
  return new_messages;
}
