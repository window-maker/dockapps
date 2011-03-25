/*
 * ssl.c
 *
 * Copyright (C) 2003 Hugo Villeneuve <hugo@hugovil.com>
 * Based on ssl_client.c (Sean Walton and Macmillan Publishers).
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

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#if HAVE_SSL

/* Define filename_M */
#define SSL_M 1

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "common.h"
#include "wmnotify.h"
#include "ssl.h"


/* InitCTX - initialize the SSL engine. */
SSL_CTX *
InitCTX( void )
{
  SSL_METHOD *method;
  SSL_CTX *ctx;
  
  SSL_library_init();             /* Load cryptos, et.al. */
  SSL_load_error_strings();       /* Bring in and register error messages */
  method = SSLv23_client_method(); /* Indicate we support SSLv2, SSLv3 and TLSv1 methods. */
  ctx = SSL_CTX_new(method);      /* Create new context */
  if( ctx == NULL ) {
    ERR_print_errors_fp(stderr);
    abort();
  }
  return ctx;
}


/* ShowCerts - print out the certificates. */
void
ShowCerts( SSL *ssl )
{
  X509 *cert;
  char *line;
  
  cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
  if ( cert != NULL ) {
    printf("Server certificates:\n");
    line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    printf("Subject: %s\n", line);
    free(line); /* free the malloc'ed string */
    line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    printf("Issuer: %s\n", line);
    free(line); /* free the malloc'ed string */
    X509_free(cert); /* free the malloc'ed certificate copy */
  }
  else {
    printf("No certificates.\n");
  }
}


int
InitSSL( int sock_fd )
{
  ssl_infos.ctx = InitCTX();  
  ssl_infos.ssl = SSL_new( ssl_infos.ctx ); /* create new SSL connection state */
  if( ssl_infos.ssl == NULL ) {
    printf( "%s: Error in SSL_new()\n", PACKAGE );
    return EXIT_FAILURE;
  }

  SSL_set_fd( ssl_infos.ssl, sock_fd ); /* attach the socket descriptor */
  if( SSL_connect( ssl_infos.ssl ) == FAIL ) { /* perform the connection */
    ERR_print_errors_fp(stderr);
    return EXIT_FAILURE;
  }

  if( wmnotify_infos.debug ) {
    printf("Connected with %s encryption\n", SSL_get_cipher( ssl_infos.ssl ));
    ShowCerts( ssl_infos.ssl ); /* get any certs */
  }

  return EXIT_SUCCESS;
}


#endif /* HAVE_SSL */
