/*
    wmget - A background download manager as a Window Maker dock app
    Copyright (c) 2001-2003 Aaron Trickey <aaron@amtrickey.net>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    ********************************************************************
    iq.c - Core functions implementing the server's Unix-domain socket

    To submit jobs to the server's input queue, you connect to a Unix-
    domain socket it opens.  These functions are responsible for
    setting up, connecting to, and accepting connections from that
    socket, and are used by server.c and request.c.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/poll.h>
#include <sys/stat.h>

#include "wmget.h"


/* iqsun == the Unix-domain socket address, aka the pathname, of our
 * socket.  iqsun_len == the number of bytes in iqsun incl. the header
 * and the pathname string, excluding the unused bytes after the
 * string's terminator.
 */
static struct sockaddr_un iqsun;
static int iqsun_len = 0;       /* 0 => not yet initialized */
const char *iqname = ".wmget.iq";

int init_paths (void)
{
    return 0;
}

static int iq_init_address (void)
{
    const char *homedir = home_directory ();

    if (iqsun_len)      /* already initialized */
        return 0;

    /* Our pathname length is constrained by sizeof(sockaddr_un), and
     * must be able to fit the homedir, the /, the iqname, and the \0.
     */
    if (strlen (homedir) > sizeof iqsun.sun_path - 2 - sizeof iqname) {
        error ("Home directory path is too long, can't construct "
               "socket name");
        return 1;
    }

    sprintf (iqsun.sun_path, "%s/%s", homedir, iqname);

    debug ("IQ = '%s'", iqsun.sun_path);

    iqsun.sun_family = AF_UNIX;

    iqsun_len = sizeof iqsun - sizeof iqsun.sun_path
                + strlen (iqsun.sun_path) + 1;

    return 0;
}


FILE *iq_client_connect (void)
{
    int fd;
    FILE *fp;

    if (iq_init_address ())
        return 0;

    if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) {
        error_sys ("Could not create Unix-domain socket to talk to server");
        return 0;
    }

    if (connect (fd, (struct sockaddr *)&iqsun, iqsun_len) < 0) {
        error_sys ("Could not connect to the server");
        close (fd);
        return 0;
    }

    if (!(fp = fdopen (fd, "r+"))) {
        /* this should never fail for any reparable reason */
        error_sys ("fdopen");
        close (fd);
        return 0;
    }

    return fp;
}

/* Server listener socket */
static int iq_listen_fd;


/** was going to use this in atexit(), but the problem is that we fork,
 * and those children inherit the atexits, and well ...
static void iq_server_cleanup (void)
{
    close (iq_listen_fd);
    unlink (iqsun.sun_path);
}
*/


int iq_server_init (void)
{
    int fd, test_fd;

    if (iq_init_address ())
        return 1;


    if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) {
        error_sys ("Could not create Unix-domain socket to receive requests");
        return 1;
    }

    /* We're going to listen on this socket and don't want accept() to
     * block
     */
    if (fcntl (fd, F_SETFL, (long)O_NONBLOCK) < 0) {
        error_sys ("fcntl");
        close (fd);
        return 1;
    }

    /* Before proceeding, try to *connect* to the iq.  If this succeeds,
     * then uh-oh, there's another wmget running out there.
     */
    test_fd = connect (fd, (struct sockaddr *)&iqsun, iqsun_len);
    if (test_fd >= 0) {
        close (test_fd);
        error (
            "There's another wmget dock running.  You can only run "
            "one at a time.");
        return 1;
    }

    /* Good.  Just in case the pathname exists, unlink it. */
    unlink (iqsun.sun_path);

    if (bind (fd, (struct sockaddr *)&iqsun, iqsun_len) < 0) {
        error_sys ("bind");
        close (fd);
        return 1;
    }

    /* Tighten up the new filename's permissions. */
    if (chmod (iqsun.sun_path, S_IRWXU) < 0) {
        error_sys ("chmod");
        close (fd);
        return 1;
    }

    if (listen (fd, 5) < 0) {
        error_sys ("listen");
        close (fd);
        return 1;
    }

    iq_listen_fd = fd;

    return 0;
}


FILE *iq_server_accept (void)
{
    int fd;
    FILE *fp;
    struct sockaddr_un sun;
    int sun_len = sizeof sun;

    if ((fd = accept (iq_listen_fd, (struct sockaddr *)&sun,
                      &sun_len)) < 0) {
        if (errno == EAGAIN) {
            /* Simply no connections waiting. */
            return 0;
        }

        error_sys ("accept");
        return 0;
    }

    debug ("got a connection...");

    if (!(fp = fdopen (fd, "r+"))) {
        error_sys ("fdopen");
        close (fd);
        return 0;
    }

    return fp;
}


int iq_get_listen_fd (void)
{
    return iq_listen_fd;
}




