/*
 * ssl.h
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

#ifndef SSL_H
#define SSL_H 1

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#if HAVE_SSL

#include <openssl/ssl.h>
#include <openssl/err.h>

/* Exported variables */
#undef _SCOPE_
#ifdef SSL_M
#  define _SCOPE_ /**/
#else
#  define _SCOPE_ extern
#endif

#define FAIL    -1

typedef struct ssl_infos_t {
  SSL_CTX *ctx;
  SSL *ssl;
} ssl_infos_t;

_SCOPE_ ssl_infos_t ssl_infos;

SSL_CTX *
InitCTX( void );

void
ShowCerts( SSL *ssl );

int
InitSSL( int sock_fd );

#endif /* HAVE_SSL */

#endif /* SSL_H */
