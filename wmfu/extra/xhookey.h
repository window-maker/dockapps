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


#ifndef XHOOKEY_H_included
#define XHOOKEY_H_included

#define XHK_XERROR	1	/* use custom X error handler */
#define XHK_SIGFORK 	1	/* use single fork + SIGCHLD reaping */

typedef struct {
    KeyCode key;
    unsigned int mod;	/* ShiftMask, ControlMask, Mod1Mask */
    const char *command;
} KTUPLE;

int xhk_parse (int argc, char **argv, KTUPLE **out_keys);
int xhk_grab (Display *dpy, int num, KTUPLE keys[]);
void xhk_ungrab (Display *dpy);

#if XHK_SIGFORK
void xhk_sig_handler (int sig);
#endif
#if XHK_XERROR
int xhk_eks_handler (Display *dpy, XErrorEvent *evt);
#endif

int xhk_run (Display *dpy, XEvent *evt, int num, KTUPLE keys[]);

#endif
