/*
 *  options.h
 *
 *  Copyright (C) 2003 Draghicioiu Mihai <misuceldestept@go.ro>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <stdio.h>

/* The version number */
#ifndef OPT_VERSION
#define OPT_VERSION "0.8"
#endif /* OPT_VERSION */

/* The X11 display to connect to */
#ifndef OPT_DISPLAY
#define OPT_DISPLAY NULL
#endif /* OPT_DISPLAY */

/* Number of miliseconds between updates */
#ifndef OPT_MILISECS
#define OPT_MILISECS 100
#endif /* OPT_MILISECS */

/* Include buffers in physical memory display */
#ifndef OPT_BUFFERS
#define OPT_BUFFERS 0
#endif /* OPT_BUFFERS */

/* Include cache in physical memory display */
#ifndef OPT_CACHE
#define OPT_CACHE 0
#endif /* OPT_CACHE */

/* Run in a window */
#ifndef OPT_WINDOW
#define OPT_WINDOW 0
#endif /* OPT_WINDOW */

/* Use the XShape extension */
#ifndef OPT_SHAPE
#define OPT_SHAPE 1
#endif /* OPT_WINDOW */

/* Window class hint name */
#ifndef OPT_CLASS_NAME
#define OPT_CLASS_NAME "wmmemfree"
#endif /* OPT_CLASS_NAME */

/* Window class hint class */
#ifndef OPT_CLASS_CLASS
#define OPT_CLASS_CLASS "WMMemFree"
#endif /* OPT_CLASS_CLASS */

/* The window title */
#ifndef OPT_WINDOW_NAME
#define OPT_WINDOW_NAME "WMMemFree"
#endif /* OPT_WINDOW_NAME */

extern char *opt_display;
extern int   opt_buffers;
extern int   opt_cache;
extern int   opt_window;
extern int   opt_shape;
extern int   opt_milisecs;

void parse_args(void);

#endif /* __OPTIONS_H__ */
