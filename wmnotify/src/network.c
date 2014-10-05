/*
 * network.c -- common routines for POP3 and IMAP protocols
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
#define NETWORK_M 1

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

#include "common.h"
#include "wmnotify.h"
#if HAVE_SSL
#  include "ssl.h"
#endif
#include "network.h"


#define SEND_FLAGS 0
#define RECV_FLAGS 0


/* Common buffers for IMAP4 and POP3. */
char tx_buffer[WMNOTIFY_BUFSIZE + 1];
char rx_buffer[WMNOTIFY_BUFSIZE + 1];


int
SocketOpen( char *server_name, int port )
{
  int status;
  int sock_fd = -1;
  struct hostent *hostinfo;
  struct sockaddr_in serv_addr;

  hostinfo = gethostbyname(server_name);
  if( hostinfo == NULL ) {
    herror( PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    goto error;
  }

  /* Open socket for Stream (TCP) */
  sock_fd = socket( PF_INET, SOCK_STREAM, 0 );
  if( sock_fd < 0 ) {
    perror( PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    goto error;
  }

  /*---Initialize server address/port struct---*/
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr = *((struct in_addr *) hostinfo->h_addr );
  memset( &( serv_addr.sin_zero ), '\0', 8 ); /* Clear the rest of the structure. */

  if( wmnotify_infos.debug ) {
    printf( "  Server IP   = %s\n", inet_ntoa( serv_addr.sin_addr ) );
    printf( "  Server port = %d\n", ntohs(serv_addr.sin_port) );
  }

  /* Establishing connection. */
  status = connect( sock_fd, (struct sockaddr *) &(serv_addr), sizeof(serv_addr) );
  if( status < 0 ) {
    perror( PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    goto error;
  }

 end:
  return sock_fd;

 error:
  if( sock_fd >= 0 ) {
    status = close( sock_fd );
    if( status < 0 ) {
      perror( PACKAGE );
      ErrorLocation( __FILE__, __LINE__ );
    }
  }

  sock_fd = -1;
  goto end;
}


int
ConnectionEstablish( char *server_name, int port )
{
  int len;
  char rx_buffer[1024]; /* Temporary... */

  wmnotify_infos.sock_fd = SocketOpen( wmnotify_infos.server_name, wmnotify_infos.port );
  if( wmnotify_infos.sock_fd < 0 ) {
    goto error;
  }

#if HAVE_SSL
  if( wmnotify_infos.use_ssl == true ) {
    int status;
    status = InitSSL( wmnotify_infos.sock_fd );
    if( status != EXIT_SUCCESS ) {
      goto error;
    }
  }
#endif

  /* Testing connection. */
  len = WmnotifyGetResponse( rx_buffer, 1024 );
  if( len < 0 ) {
    goto error;
  }

  if( wmnotify_infos.debug ) {
    rx_buffer[len] = 0;
    printf(" Connect response:\n%s\n", rx_buffer );
  }

  return EXIT_SUCCESS;

 error:
  return EXIT_FAILURE;
}


int
ConnectionTerminate( void )
{
#if HAVE_SSL
  if( wmnotify_infos.use_ssl == true ) {
    SSL_free( ssl_infos.ssl ); /* release connection state */
  }
#endif

  close( wmnotify_infos.sock_fd ); /* close socket */

#if HAVE_SSL
  if( wmnotify_infos.use_ssl == true ) {
    SSL_CTX_free( ssl_infos.ctx ); /* release context */
  }
#endif

  return EXIT_SUCCESS;
}


int
WmnotifySendData( char *buffer, int size )
{
  int len;

#if HAVE_SSL
  if( wmnotify_infos.use_ssl == true ) {
    len = SSL_write( ssl_infos.ssl, buffer, size ); /* Encrypt & send message */
    if( len <= 0 ) {
      SSL_get_error( ssl_infos.ssl, len );
      len = -1;
    }

    return len;
  }
#endif /* HAVE_SSL */

  /* if errno = EINTR, it means the operation was interrupted by a signal before any data was
   * sent. We must retry the operation in this case. */
  do {
    len = send( wmnotify_infos.sock_fd, buffer, size, SEND_FLAGS );
  }
  while( ( len < 0 ) && ( errno == EINTR ) );

  if( len < 0 ) {
    perror( PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
  }

  return len;
}


int
WmnotifyGetResponse( char *buffer, int max_size )
{
  int len;

#if HAVE_SSL
  if( wmnotify_infos.use_ssl == true ) {
    len = SSL_read( ssl_infos.ssl, buffer, max_size ); /* Get reply & decrypt. */
    switch( SSL_get_error( ssl_infos.ssl, len ) ) {
    case SSL_ERROR_NONE:
      /* Success. */
      break;
    case SSL_ERROR_ZERO_RETURN:
      fprintf( stderr, "%s: SSL_read() connection closed.\n", PACKAGE );
      break;
    case SSL_ERROR_SYSCALL:
      fprintf( stderr, "%s: SSL_read() I/O error.\n", PACKAGE );
      goto ssl_error;
    case SSL_ERROR_SSL:
      fprintf( stderr, "%s: SSL_read() protocol error.\n", PACKAGE );
      goto ssl_error;
    default:
      fprintf( stderr, "%s: SSL_read() error.\n", PACKAGE );
      goto ssl_error;
    }

    return len;

  ssl_error:
    return -1;
  }
#endif /* HAVE_SSL */

  /* if errno = EINTR, it means the operation was interrupted by a signal before any data was
   * read. We must retry the operation in this case. */
  do {
    len = recv( wmnotify_infos.sock_fd, buffer, max_size, RECV_FLAGS );
  }
  while( ( len < 0 ) && ( errno == EINTR ) );

  if( len < 0 ) {
    perror( PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
  }

  return len;
}
