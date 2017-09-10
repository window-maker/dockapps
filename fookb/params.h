/*
 * params.h
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 *
 */

#ifndef PARAMS_H
#define PARAMS_H

#ifdef HAVE_WINGS_WUTIL_H 
#include <WINGs/WUtil.h> 

#ifdef WMAKER
#define DEFAULTS_FILE "~/GNUstep/Defaults/FOOkb"
#else
#define DEFAULTS_FILE "~/.fookb"
#endif				/* WMAKER */

#endif				/* HAVE_WINGS_WUTIL_H */

#include <X11/Xlib.h>		/* X Window standard header */
#include <X11/Xresource.h>	/* X resource manager stuff */

char *read_param(char *);

#endif				/* PARAMS_H */
