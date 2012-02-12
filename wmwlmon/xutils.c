/* $Id: xutils.c,v 1.6 2008/04/30 20:52:42 hacki Exp $ */

/*
 * Copyright (c) 2005, 2006 Marcus Glocker <marcus@nazgul.ch>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Those X functions have been taken from the xutils.c file used by almost
 * every wmdockapp.  It has been slightly modificated for our needs and
 * unused functions have been removed.  The original version was written
 * by Martijn Pieterse <pieterse@xs4all.nl>.
 */

#include <ctype.h>
#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "xutils.h"

/*
 * global variables for this file
 */
int		x_fd;
XSizeHints	mysizehints;
XWMHints	mywmhints;
Pixel		back_pix, fore_pix;
char		*Geometry = "";
GC		NormalGC;
XpmIcon		wmgen;
Pixmap		pixmask;

extern char	TimeColor[30];
extern char	BackgroundColor[30];

/*
 * flush_expose
 */
int
flush_expose(Window w)
{
	XEvent	dummy;
	int	i = 0;

	while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
		i++;

	return (i);
}

/*
 * copxXPMArea
 */
void
copyXPMArea(int x, int y, int sx, int sy, int dx, int dy)
{
	XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy,
	    dx, dy);
}

/*
 * copyXBMArea
 */
void
copyXBMArea(int x, int y, int sx, int sy, int dx, int dy)
{
	XCopyArea(display, wmgen.mask, wmgen.pixmap, NormalGC, x, y, sx, sy,
	    dx, dy);
}

/*
 * RedrawWindow
 */
void
RedrawWindow(void) {
	flush_expose(iconwin);
	XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, 0, 0,
	    wmgen.attributes.width, wmgen.attributes.height, 0, 0);

	flush_expose(win);
	XCopyArea(display, wmgen.pixmap, win, NormalGC, 0, 0,
	    wmgen.attributes.width, wmgen.attributes.height, 0, 0);
}

/*
 * initXwindow
 */
void
initXwindow(char *opt_display)
{
	char	*display_name = NULL;

	if (opt_display != NULL)
		display_name = opt_display;

	if (!(display = XOpenDisplay(display_name)))
		errx(1, "Can't open display: %s", XDisplayName(display_name));

	screen = DefaultScreen(display);
	Root = RootWindow(display, screen);
	DisplayDepth = DefaultDepth(display, screen);
	x_fd = XConnectionNumber(display);
}

/*
 * openXwindow
 */
void
openXwindow(int argc, char *argv[], char *pixmap_bytes[],
    unsigned char *pixmask_bits, int pixmask_width, int pixmask_height)
{
	int		dummy = 0, red, grn, blu;
	char		*wname = argv[0];
	unsigned int	borderwidth = 1;
	unsigned long	gcm;
	XGCValues	gcv;
	XClassHint	classHint;
	XTextProperty	name;
	XpmColorSymbol	cols[10] = {
			    { "Back", NULL, 0 },
			    { "Color1", NULL, 0 },
			    { "Color2", NULL, 0 },
			    { "Color3", NULL, 0 },
			    { "Color4", NULL, 0 },
			    { "Color5", NULL, 0 },
			    { "Color6", NULL, 0 },
			    { "Color7", NULL, 0 },
			    { "Color8", NULL, 0 },
			    { "Color9", NULL, 0 }
			};

	/* 
	 *  create pixmap
	 */
	cols[0].pixel = getColor(BackgroundColor, 1.0000, &red, &grn, &blu);
	cols[1].pixel = getBlendedColor(TimeColor, 0.1522, red, grn, blu);
	cols[2].pixel = getBlendedColor(TimeColor, 0.2602, red, grn, blu);
	cols[3].pixel = getBlendedColor(TimeColor, 0.3761, red, grn, blu);
	cols[4].pixel = getBlendedColor(TimeColor, 0.4841, red, grn, blu);
 	cols[5].pixel = getBlendedColor(TimeColor, 0.5922, red, grn, blu);
	cols[6].pixel = getBlendedColor(TimeColor, 0.6980, red, grn, blu);
 	cols[7].pixel = getBlendedColor(TimeColor, 0.7961, red, grn, blu);
	cols[8].pixel = getBlendedColor(TimeColor, 0.8941, red, grn, blu);
 	cols[9].pixel = getBlendedColor(TimeColor, 1.0000, red, grn, blu);

	wmgen.attributes.numsymbols = 10;
	wmgen.attributes.colorsymbols = cols;
	wmgen.attributes.exactColors = False;
	wmgen.attributes.closeness = 40000;
	wmgen.attributes.valuemask = XpmReturnPixels | XpmReturnExtensions |
	    XpmColorSymbols | XpmExactColors | XpmCloseness | XpmSize;

	if (XpmCreatePixmapFromData(display, Root, pixmap_bytes,
	    &(wmgen.pixmap), &(wmgen.mask), &(wmgen.attributes)) != XpmSuccess)
		errx(1, "Not enough free colocells\n");

	/*
	 * create a window
	 */
	mysizehints.flags = USSize | USPosition;
	mysizehints.x = 0;
	mysizehints.y = 0;

	back_pix = getColor("white", 1.0, &red, &grn, &blu);
	fore_pix = getColor("black", 1.0, &red, &grn, &blu);

	XWMGeometry(display, screen, Geometry, NULL, borderwidth, &mysizehints,
	    &mysizehints.x, &mysizehints.y, &mysizehints.width,
	    &mysizehints.height, &dummy);

	mysizehints.width = 64;
	mysizehints.height = 64;

	win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,
	    mysizehints.width, mysizehints.height, borderwidth, fore_pix,
	    back_pix);

	iconwin = XCreateSimpleWindow(display, win, mysizehints.x,
	    mysizehints.y, mysizehints.width, mysizehints.height, borderwidth,
	    fore_pix, back_pix);

	/* 
	 * activate hints 
	 */
	XSetWMNormalHints(display, win, &mysizehints);
	classHint.res_name = wname;
	classHint.res_class = wname;
	XSetClassHint(display, win, &classHint);

	/*
 	 * set up the xevents
 	 */
	XSelectInput(display, win, ButtonPressMask | ExposureMask |
	    ButtonReleaseMask  | PointerMotionMask | StructureNotifyMask |
	    EnterWindowMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask);
	XSelectInput(display, win, ButtonPressMask | ExposureMask |
	    ButtonReleaseMask  | PointerMotionMask | StructureNotifyMask |
	    EnterWindowMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask);

	if (XStringListToTextProperty(&wname, 1, &name) == 0)
		errx(1, "Can't allocate window name: %s\n", wname);

	XSetWMName(display, win, &name);

	/* 
	 * create graphics context (gc) for drawing 
	 */
	gcm = GCForeground | GCBackground | GCGraphicsExposures;
	gcv.foreground = fore_pix;
	gcv.background = back_pix;
	gcv.graphics_exposures = 0;
	NormalGC = XCreateGC(display, Root, gcm, &gcv);

	pixmask = XCreateBitmapFromData(display, win, (char *)pixmask_bits,
	    pixmask_width, pixmask_height);
	XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask,
	    ShapeSet);
	XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask,
	    ShapeSet);

	mywmhints.initial_state = WithdrawnState;
	mywmhints.icon_window = iconwin;
	mywmhints.icon_x = mysizehints.x;
	mywmhints.icon_y = mysizehints.y;
	mywmhints.window_group = win;
	mywmhints.flags = StateHint | IconWindowHint | IconPositionHint |
	    WindowGroupHint;

	XSetWMHints(display, win, &mywmhints);

	XSetCommand(display, win, argv, argc);
	XMapWindow(display, win);
}

/*
 * getColor
 */
unsigned long
getColor(char *ColorName, float fac, int *red, int *grn, int *blu)
{
	XColor			Color;
	XWindowAttributes	Attributes;

	XGetWindowAttributes(display, Root, &Attributes);
	Color.pixel = 0;

	XParseColor(display, Attributes.colormap, ColorName, &Color);
	Color.red   = (unsigned short)(fac*(Color.red-24) + 24);
	Color.blue  = (unsigned short)(fac*(Color.blue-24) + 24);
	Color.green = (unsigned short)(fac*(Color.green-24) + 24);
	Color.flags = DoRed | DoGreen | DoBlue;
	XAllocColor(display, Attributes.colormap, &Color);
    
	*red = Color.red;
	*grn = Color.green;
	*blu = Color.blue;

	return (Color.pixel);
}

/*
 * getBlendedColor
 */
unsigned long
getBlendedColor(char *ColorName, float fac, int red, int grn, int blu)
{
	XColor			Color;
	XWindowAttributes	Attributes;

	XGetWindowAttributes(display, Root, &Attributes);
	Color.pixel = 0;

	XParseColor(display, Attributes.colormap, ColorName, &Color);
	Color.red   = (unsigned short)(fac*(Color.red-red) + red);
	Color.blue  = (unsigned short)(fac*(Color.blue-grn) + grn);
	Color.green = (unsigned short)(fac*(Color.green-blu) + blu);
	Color.flags = DoRed | DoGreen | DoBlue;
	XAllocColor(display, Attributes.colormap, &Color);

	return (Color.pixel);
}
