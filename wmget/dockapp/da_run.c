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
    dockapp/da_run.c - Main dockapp loop; fd polling; periodic callback

    This file implements dockapp_run(), which provides the main loop of
    the dockapp application.  In addition, it implements
    dockapp_add_pollfd(), which adds polling of arbitrary file
    descriptors to the main loop, and dockapp_set_periodic_callback(),
    which sets up a periodic app callback.
*/

#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/poll.h>

#include "dockapp.h"


/* we keep two parallel structures... one, the list of pollfd's for
 * poll(2)'s benefit, and the other, the collection of callback
 * information
 */
typedef struct {
    dockapp_rv_t (*cb) (void *cbdata, short revents);
    void *cbdata;
    short revents;
} da_pollcb;


enum {
    da_max_pollfds = 50
};

static struct pollfd    da_pollfds[da_max_pollfds];
static da_pollcb        da_pollcbs[da_max_pollfds];
static int              da_num_pollfds = 0;


dockapp_rv_t dockapp_add_pollfd (
        int fd,
        short pollevents,
        dockapp_rv_t (*cb) (void *, short pollstatus),
        void *cbdata)
{
    struct pollfd *pfd;
    da_pollcb *pcb;

    assert (da_num_pollfds < da_max_pollfds);

    /*
    fprintf (stderr, "adding pollfd #%d, fd = %d, events = %hu\n",
             da_num_pollfds, fd, pollevents);
     */

    pfd = da_pollfds + da_num_pollfds;
    pcb = da_pollcbs + da_num_pollfds;

    pfd->fd = fd;
    pfd->events = pollevents;
    pcb->cb = cb;
    pcb->cbdata = cbdata;

    ++da_num_pollfds;

    return dockapp_ok;
}


dockapp_rv_t dockapp_remove_pollfd (
        int fd)
{
    int i = da_num_pollfds;
    struct pollfd *pfd = da_pollfds;
    da_pollcb *pcb = da_pollcbs;

    for ( ; i; --i, ++pfd, ++pcb) {
        if (pfd->fd == fd) {
            memmove (pfd, pfd + 1, (i - 1) * sizeof (struct pollfd));
            memmove (pcb, pcb + 1, (i - 1) * sizeof (da_pollcb));
            return dockapp_ok;
        }
    }

    /* not found... */
    return dockapp_invalid_arg;
}


/* periodic callback info... if da_timer_cb is null, then no callback is
 * set.
 */
static dockapp_rv_t (*da_timer_cb) (void *) = 0;
static void *da_timer_cbdata;
static struct timeval da_timer_next_timeout;
static long da_timer_interval_msec;


static void da_reset_timer (void)
{
    int rv;

    rv = gettimeofday (&da_timer_next_timeout, 0);
    assert (rv == 0);

    /*
    fprintf (stderr,
             "da_reset_timer: RIGHT NOW is %ld.%ld, msec = %ld\n",
             da_timer_next_timeout.tv_sec,
             da_timer_next_timeout.tv_usec,
             da_timer_interval_msec);
     */

    da_timer_next_timeout.tv_usec
        += (da_timer_interval_msec % 1000L) * 1000L;

    da_timer_next_timeout.tv_sec
        += da_timer_interval_msec / 1000L
         + da_timer_next_timeout.tv_usec / 1000000L;

    da_timer_next_timeout.tv_usec %= 1000000L;

    /*
    fprintf (stderr,
             "da_reset_timer: NEXT TIMEOUT is %ld.%ld\n",
             da_timer_next_timeout.tv_sec,
             da_timer_next_timeout.tv_usec);
     */
}


static long da_timer_msec_remaining (void)
{
    struct timeval right_now;
    int rv;

    rv = gettimeofday (&right_now, 0);

    return
        (da_timer_next_timeout.tv_sec - right_now.tv_sec) * 1000L
        + (da_timer_next_timeout.tv_usec - right_now.tv_usec) / 1000L;
}


void dockapp_set_periodic_callback (
        long msec,
        dockapp_rv_t (*cb) (void *),
        void *cbdata)
{
    da_timer_cb = cb;
    da_timer_cbdata = cbdata;

    da_timer_interval_msec = msec;

    da_reset_timer ();
}


void dockapp_remove_periodic_callback (void)
{
    da_timer_cb = 0;
    da_timer_cbdata = 0;
    da_timer_interval_msec = 0;
}


/* this is the main loop for the dockapp...
 */
dockapp_rv_t dockapp_run (void)
{
    for ( ; ; ) {
        int rv;
        int poll_timeout = -1;  /* infinite unless periodic callback */

        if (da_timer_cb) {
            if (da_timer_msec_remaining () <= 0) {
                rv = da_timer_cb (da_timer_cbdata);
                da_reset_timer ();
            }

            poll_timeout = da_timer_msec_remaining ();
        }

        /*
        fprintf (stderr, "about to poll(%d fd, %d timeout)\n",
                da_num_pollfds, poll_timeout);
        */

        rv = poll (da_pollfds, da_num_pollfds, poll_timeout);

        /*
        fprintf (stderr, "poll returned %d\n", rv);
        */

        if (rv == 0) {
            /* poll timed out; let's loop back up and let the logic
             * prior to the poll() invoke the user-defined periodic
             * callback
             */
            continue;
        } else if (rv < 0) {
            /* Disregard EINTR; just means that a signal was caught.
             * We'll retry later.
             */
            if (errno != EINTR)
                perror ("poll()");

        } else {
            /* poll returned with some nonzero number of events...
             * collect the callback structures first, then invoke the
             * callbacks, since the callback functions are allowed to
             * create and destroy callbacks as well
             */
            int i;
            da_pollcb callbacks[da_max_pollfds];
            da_pollcb *c;
            int ncallbacks = 0;

            for (i = 0; i < da_num_pollfds && ncallbacks < rv; ++i) {
                if (da_pollfds[i].revents) {
                    callbacks[ncallbacks] = da_pollcbs[i];
                    callbacks[ncallbacks].revents
                        = da_pollfds[i].revents;
                    ++ncallbacks;
                }
            }

            for (c = callbacks; ncallbacks; --ncallbacks, ++c) {
                if (c->cb (c->cbdata, c->revents) == dockapp_exit) {
                    fprintf (stderr, "exiting dockapp_run()\n");
                    return dockapp_ok;
                }
            }
        }
    }

}




