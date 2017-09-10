/*
 * File: opts.h
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 *
 */

#ifndef OPTS_H
#define OPTS_H

/* X Window resource management */
#include <X11/Xlib.h>
#include <X11/Xresource.h>
extern XrmDatabase cmdlineDB;	/* Database for resources from command 
				   line */

extern XrmDatabase finalDB;	/* Database for resources from other
				   sources -- app-defaults and X
				   Window resources */

void ParseOptions(int *argc, register char *argv[]);	/* Parse
							   command
							   line
							   options */

void MoreOptions(Display *dpy);		/* Parse
				   app-defaults
				   and X
				   resources
				   database */

#endif				/* OPTS_H */
