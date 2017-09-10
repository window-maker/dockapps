/*
 * params.h
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 *
 */

#ifndef PARAMS_H
#define PARAMS_H


#ifdef WMAKER
#define DEFAULTS_FILE "~/GNUstep/Defaults/FOOkb"
#else
#define DEFAULTS_FILE "~/.fookb"
#endif				/* WMAKER */

char *read_param(char *);

#endif				/* PARAMS_H */
