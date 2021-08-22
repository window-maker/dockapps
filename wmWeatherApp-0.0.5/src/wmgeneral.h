/*
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
