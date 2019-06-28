/* $Id: socket.c,v 1.14 2004/10/01 21:05:36 bluehal Exp $ */
/* Copyright (C) 1998 Trent Piepho  <xyzzy@u.washington.edu>
 *           (C) 1999 Trent Piepho  <xyzzy@speakeasy.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include "regulo.h"

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif

/* be paranoid about leaking passwords as hostnames, enough
   that we'll avoid attempting lookups on things that aren't
   host names */
extern int Relax;
static int sanity_check_hostname(const char *hostname)
{
	struct in_addr dummy;
	return (Relax
			|| regulo_match("^[A-Za-z][-_A-Za-z0-9.]+$", hostname, NULL)
			|| inet_aton(hostname, &dummy));
}

static int ipv4_sock_connect(struct in_addr *address, uint16_t port)
{
	struct sockaddr_in addr;
	int fd, i;
	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1) {
		perror("Error opening socket");
		printf("socket() failed.\n");
		return (-1);
	};
       if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
               perror("fcntl(FD_CLOEXEC)");

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = *(u_long *) address;
	addr.sin_port = htons(port);
	i = connect(fd, (struct sockaddr *) &addr, sizeof addr);
	if (i == -1) {
		int saved_errno = errno;
		perror("Error connecting");
		printf("connect(%s:%d) failed: %s\n", inet_ntoa(addr.sin_addr),
			   port, strerror(saved_errno));
		close(fd);
		return (-1);
	};
	return (fd);
}

/* nspring/blueHal, 10 Apr 2002; added some extra error
   printing, in line with the debug-messages-to-stdout
   philosophy of the rest of the wmbiff code */
/* 1 June 2002; incorporated IPv6 support by
   Jun-ichiro itojun Hagino <itojun@iijlab.net>, thanks! */

int sock_connect(const char *hostname, uint16_t port)
{
#ifdef HAVE_GETADDRINFO
	struct addrinfo hints, *res, *res0;
	struct sockaddr_in addr;
	int fd;
	char pbuf[NI_MAXSERV];
	int error;

	if (!sanity_check_hostname(hostname)) {
		printf
			("socket/connect to '%s' aborted: it does not appear to be a valid hostname\n",
			 hostname);
		printf("if you really want this, use wmbiff's -relax option.\n");
		return -1;
	}

	/* we were given an IP address, no need to try getaddrinfo on it */
	if (inet_aton(hostname, &addr.sin_addr)) {
		return ipv4_sock_connect(&addr.sin_addr, port);
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	snprintf(pbuf, sizeof(pbuf), "%d", port);
	error = getaddrinfo(hostname, pbuf, &hints, &res0);
	if (error) {
		static int last_error;
		if (last_error != error) {
			/* only report a problem if it's new.  this is an
			   approximation that minimizes kept state. */
			printf("%s: %s\n", hostname, gai_strerror(error));
			last_error = error;
		}
		return -1;
	}

	fd = -1;
	for (res = res0; res; res = res->ai_next) {
		fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (fd < 0)
			continue;
               if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
                       perror("fcntl(FD_CLOEXEC)");
		if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
			close(fd);
			fd = -1;
			continue;
		}
		break;
	}
	freeaddrinfo(res0);
	if (fd < 0) {
		static int last_connecterr;
		if (errno != last_connecterr) {
			/* only report a problem if it's new.
			   EHOSTUNREACH is common when the net is down,
			   for example; again, this is an approximation
			   to minimize kept state. */
			last_connecterr = errno;
			fprintf(stderr, "Error connecting to %s:%d: %s\n",
					hostname, port, strerror(errno));
			printf("socket/connect to %s failed: %s (%d)\n", hostname,
				   strerror(errno), errno);
		}
		return -1;
	}
	return fd;
#else
#warning "This build will not support IPv6"
	struct hostent *host;

	if (!sanity_check_hostname(hostname)) {
		printf
			("socket/connect to '%s' aborted: it does not appear to be a valid hostname\n",
			 hostname);
		printf("if you really want this, use wmbiff's -relax option.\n");
		return -1;
	}

	host = gethostbyname(hostname);
	if (host == NULL) {
		herror("gethostbyname");
		printf("gethostbyname(%s) failed.\n", hostname);
		return (-1);
	};

	return ipv4_sock_connect(host->h_addr_list[0], port);

#endif
}

/* vim:set ts=4: */
/*
 * Local Variables:
 * tab-width: 4
 * c-indent-level: 4
 * c-basic-offset: 4
 * End:
 */
