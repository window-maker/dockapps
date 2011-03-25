/*
 * Copyright (c) 2007 Daniel Borca  All rights reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include "xhookey.h"


static void
test_keys (Display *dpy, int num, KTUPLE keys[])
{
    int quit = 0;

#if XHK_SIGFORK
    void (*old_sig_handler) (int)  = signal(SIGCHLD, xhk_sig_handler);
#endif
#if XHK_XERROR
    XErrorHandler old_eks_handler = XSetErrorHandler(xhk_eks_handler);
#endif
    while (!quit) {
	XEvent evt;
        XNextEvent(dpy, &evt);
	xhk_run(dpy, &evt, num, keys);
    }
#if XHK_XERROR
    XSetErrorHandler(old_eks_handler);
#endif
#if XHK_SIGFORK
    signal(SIGCHLD, old_sig_handler);
#endif
}


int
main (int argc, char **argv)
{
    int rv;

    int n;
    KTUPLE *keys;

    Display *display;

    display = XOpenDisplay(NULL);
    if (display == NULL) {
	fprintf(stderr, "cannot open display\n");
	goto err_0;
    }

    n = xhk_parse(argc, argv, &keys);
    if (n <= 0) {
	fprintf(stderr, "no keys\n");
	goto err_1;
    }

    rv = xhk_grab(display, n, keys);
    if (rv < 0) {
	printf("cannot grab keys\n");
	goto err_2;
    }

    test_keys(display, n, keys);

    xhk_ungrab(display);

    free(keys);

    XCloseDisplay(display);

    return 0;

  err_2:
    free(keys);
  err_1:
    XCloseDisplay(display);
  err_0:
    return -1;
}
