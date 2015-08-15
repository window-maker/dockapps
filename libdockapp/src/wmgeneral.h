/*
	wmgeneral was taken from wmppp.

	It has a lot of routines which most of the wm* programs use.

	------------------------------------------------------------

	Copyright (C) 1998 Martijn Pieterse (pieterse@xs4all.nl)

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
	02110-1301, USA.

*/

#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED

#include <X11/X.h>                      /* for Pixmap */
#include <X11/Xlib.h>                   /* for Display */
#include <X11/xpm.h>                    /* for XpmAttributes */

  /***********/
 /* Defines */
/***********/

#define MAX_MOUSE_REGION (16)

  /************/
 /* Typedefs */
/************/

struct _rckeys {
	const char	*label;
	char		**var;
};

typedef struct _rckeys rckeys;

struct _rckeys2 {
	const char	*family;
	const char	*label;
	char		**var;
};

typedef struct _rckeys2 rckeys2;

typedef struct {
	Pixmap			pixmap;
	Pixmap			mask;
	XpmAttributes	attributes;
} XpmIcon;

  /*******************/
 /* Global variable */
/*******************/

Display		*display;

  /***********************/
 /* Function Prototypes */
/***********************/

void AddMouseRegion(int index, int left, int top, int right, int bottom);
int CheckMouseRegion(int x, int y);

void openXwindow(int argc, char *argv[], char **, char *, int, int);
void RedrawWindow(void);
void RedrawWindowXY(int x, int y);

void createXBMfromXPM(char *, char **, int, int);
void copyXPMArea(int, int, int, int, int, int);
void copyXBMArea(int, int, int, int, int, int);
void setMaskXY(int, int);

void parse_rcfile(const char *, rckeys *);

#endif
