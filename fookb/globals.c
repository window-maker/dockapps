/*
 * File: globals.c
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 *
 */

#include <X11/Xlib.h>
#include <X11/Xresource.h>

char mydispname[256];		/* X display name */

XrmDatabase cmdlineDB;		/* X resource database generated from
				   command line */
XrmDatabase finalDB;		/* X resource database generated from
				   app-defaults and X resource
				   database */
