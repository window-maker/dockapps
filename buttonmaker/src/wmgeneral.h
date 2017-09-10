/*
 * wmgeneral.h
 * Copyright (C) Renan Vedovato Traba 2012 <rvt10@inf.ufpr.br>
 *
ButtonMaker is free software: you can redistribute it and/or modify it
	under the terms of the GNU General Public License as published by the
	Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	ButtonMaker is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED
/* Defines */
#define MAX_MOUSE_REGION (8)

/* Typedefs */
typedef struct _rckeys rckeys;

struct _rckeys {
	const char	*label;
	char		**var;
};

typedef struct {
	Pixmap		pixmap;
	Pixmap		mask;
	XpmAttributes	attributes;
} XpmIcon;

/* Global variable */
Display	*display;
Window          Root;
GC              NormalGC;
XpmIcon         wmgen;

/* Function Prototypes */
void AddMouseRegion(int idx, int left, int top, int right, int bottom);
int CheckMouseRegion(int x, int y);
void openXwindow(int argc, char *argv[], char **, char *, int, int);
void RedrawWindow(void);
void RedrawWindowXY(int x, int y);
void copyXPMArea(int, int, int, int, int, int);
void copyXBMArea(int, int, int, int, int, int);
void setMaskXY(int, int);
void parse_rcfile(const char *, rckeys *);

#endif
