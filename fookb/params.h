/*
 * params.h
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 *
 */

#ifndef PARAMS_H
#define PARAMS_H

#define DEFAULTS_FILE		"~/.fookb"

#define DEFAULT_ICON1		"/usr/local/share/fookb/lat.xpm"
#define DEFAULT_ICON2		"/usr/local/share/fookb/rus.xpm"
#define DEFAULT_ICON3		"/usr/local/share/fookb/3.xpm"
#define DEFAULT_ICON4		"/usr/local/share/fookb/4.xpm"
#define DEFAULT_ICON_BOOM	"/usr/local/share/fookb/boom.xpm"
#define DEFAULT_SOUND		"Yes"
#define DEFAULT_COMMAND		"/usr/bin/play /usr/local/share/fookb/beep_spring.au"

char *read_param(char *);

#endif				/* PARAMS_H */
