/*

   WMPPP - PPP dock app/helper for WindowMaker
   Copyright (C) 1997-2000:

   Authors: Martijn Pieterse (pieterse@xs4all.nl)
            Antoine Nulle    (warp@xs4all.nl)
            Felipe Bergo     (bergo@seul.org)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "wmgeneral.h"

Window Root;
int screen;
int x_fd;
int d_depth;
XSizeHints mysizehints;
XWMHints mywmhints;
Pixel back_pix, fore_pix;
char *Geometry = "";
Window iconwin, win;
GC NormalGC;
XpmIcon wmgen;
Pixmap pixmask;

typedef struct
  {
      int enable;
      int top;
      int bottom;
      int left;
      int right;
  }
MOUSE_REGION;

MOUSE_REGION mouse_region[MAX_MOUSE_REGION];

static void GetXPM (XpmIcon *, char **);
static Pixel GetColor (char *);
void RedrawWindow (void);
void AddMouseRegion (unsigned, int, int, int, int);
int CheckMouseRegion (int, int);

static void
GetXPM (XpmIcon * wmgen, char *pixmap_bytes[])
{

    XWindowAttributes attributes;
    int err;

    /* For the colormap */
    XGetWindowAttributes (display, Root, &attributes);

    wmgen->attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);

    err = XpmCreatePixmapFromData (display, Root, pixmap_bytes, &(wmgen->pixmap),
				   &(wmgen->mask), &(wmgen->attributes));

    if (err != XpmSuccess)
      {
	  fprintf (stderr, "Not enough free colorcells.\n");
	  exit (1);
      }
}

static Pixel
GetColor (char *name)
{

    XColor color;
    XWindowAttributes attributes;

    XGetWindowAttributes (display, Root, &attributes);

    color.pixel = 0;
    if (!XParseColor (display, attributes.colormap, name, &color))
      {
	  fprintf (stderr, "wm.app: can't parse %s.\n", name);
      }
    else if (!XAllocColor (display, attributes.colormap, &color))
      {
	  fprintf (stderr, "wm.app: can't allocate %s.\n", name);
      }
    return color.pixel;
}

static int
flush_expose (Window w)
{

    XEvent dummy;
    int i = 0;

    while (XCheckTypedWindowEvent (display, w, Expose, &dummy))
	i++;

    return i;
}

void
RedrawWindow (void)
{

    flush_expose (iconwin);
    XCopyArea (display, wmgen.pixmap, iconwin, NormalGC,
	       0, 0, wmgen.attributes.width, wmgen.attributes.height, 0, 0);
    flush_expose (win);
    XCopyArea (display, wmgen.pixmap, win, NormalGC,
	       0, 0, wmgen.attributes.width, wmgen.attributes.height, 0, 0);
}

void
RedrawWindowXY (int x, int y)
{

    flush_expose (iconwin);
    XCopyArea (display, wmgen.pixmap, iconwin, NormalGC,
	       x, y, wmgen.attributes.width, wmgen.attributes.height, 0, 0);
    flush_expose (win);
    XCopyArea (display, wmgen.pixmap, win, NormalGC,
	       x, y, wmgen.attributes.width, wmgen.attributes.height, 0, 0);
}

void
AddMouseRegion (unsigned index, int left, int top, int right, int bottom)
{

    if (index < MAX_MOUSE_REGION)
      {
	  mouse_region[index].enable = 1;
	  mouse_region[index].top = top;
	  mouse_region[index].left = left;
	  mouse_region[index].bottom = bottom;
	  mouse_region[index].right = right;
      }
}

int
CheckMouseRegion (int x, int y)
{

    int i;
    int found;

    found = 0;

    for (i = 0; i < MAX_MOUSE_REGION && !found; i++)
      {
	  if (mouse_region[i].enable &&
	      x <= mouse_region[i].right &&
	      x >= mouse_region[i].left &&
	      y <= mouse_region[i].bottom &&
	      y >= mouse_region[i].top)
	      found = 1;
      }
    if (!found)
	return -1;
    return (i - 1);
}

void
createXBMfromXPM (char *xbm, char **xpm, int sx, int sy)
{

    int i, j;
    int width, height, numcol;
    char zero;
    unsigned char bwrite;
    int bcount;


    sscanf (*xpm, "%d %d %d", &width, &height, &numcol);

    zero = xpm[1][0];
    for (i = numcol + 1; i < numcol + sy + 1; i++)
      {
	  bcount = 0;
	  bwrite = 0;
	  for (j = 0; j < sx; j++)
	    {
		bwrite >>= 1;
		if (xpm[i][j] != zero)
		  {
		      bwrite += 128;
		  }
		bcount++;
		if (bcount == 8)
		  {
		      *xbm = bwrite;
		      xbm++;
		      bcount = 0;
		      bwrite = 0;
		  }
	    }
      }
}

void
copyXPMArea (int x, int y, int sx, int sy, int dx, int dy)
{

    XCopyArea (display, wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);

}

void
copyXBMArea (int x, int y, int sx, int sy, int dx, int dy)
{

    XCopyArea (display, wmgen.mask, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}

void
setMaskXY (int x, int y)
{

    XShapeCombineMask (display, win, ShapeBounding, x, y, pixmask, ShapeSet);
    XShapeCombineMask (display, iconwin, ShapeBounding, x, y, pixmask, ShapeSet);
}

void
openXwindow (int argc, char *argv[], char *pixmap_bytes[], char *pixmask_bits, int pixmask_width, int pixmask_height)
{

    unsigned int borderwidth = 1;
    XClassHint classHint;
    char *display_name = NULL;
    char *wname = argv[0];
    XTextProperty name;

    XGCValues gcv;
    unsigned long gcm;

    char *geometry = NULL;

    int dummy = 0;
    int i, wx, wy;

    for (i = 1; argv[i]; i++)
      {
	  if (!strcmp (argv[i], "-display"))
	    {
		display_name = argv[i + 1];
		i++;
	    }
	  if (!strcmp (argv[i], "-geometry"))
	    {
		geometry = argv[i + 1];
		i++;
	    }
      }

    if (!(display = XOpenDisplay (display_name)))
      {
	  fprintf (stderr, "%s: can't open display %s\n",
		   wname, XDisplayName (display_name));
	  exit (1);
      }
    screen = DefaultScreen (display);
    Root = RootWindow (display, screen);
    d_depth = DefaultDepth (display, screen);
    x_fd = XConnectionNumber (display);

    /* Convert XPM to XImage */
    GetXPM (&wmgen, pixmap_bytes);

    /* Create a window to hold the stuff */
    mysizehints.flags = USSize | USPosition;
    mysizehints.x = 0;
    mysizehints.y = 0;

    back_pix = GetColor ("white");
    fore_pix = GetColor ("black");

    XWMGeometry (display, screen, Geometry, NULL, borderwidth, &mysizehints,
		 &mysizehints.x, &mysizehints.y, &mysizehints.width, &mysizehints.height, &dummy);

    mysizehints.width = 64;
    mysizehints.height = 64;

    win = XCreateSimpleWindow (display, Root, mysizehints.x, mysizehints.y,
    mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

    iconwin = XCreateSimpleWindow (display, win, mysizehints.x, mysizehints.y,
    mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

    /* Activate hints */
    XSetWMNormalHints (display, win, &mysizehints);
    classHint.res_name = wname;
    classHint.res_class = wname;
    XSetClassHint (display, win, &classHint);

    XSelectInput (display, win, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
    XSelectInput (display, iconwin, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

    if (XStringListToTextProperty (&wname, 1, &name) == 0)
      {
	  fprintf (stderr, "%s: can't allocate window name\n", wname);
	  exit (1);
      }

    XSetWMName (display, win, &name);

    /* Create GC for drawing */

    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = fore_pix;
    gcv.background = back_pix;
    gcv.graphics_exposures = 0;
    NormalGC = XCreateGC (display, Root, gcm, &gcv);

    /* ONLYSHAPE ON */

    pixmask = XCreateBitmapFromData (display, win, pixmask_bits, pixmask_width, pixmask_height);

    XShapeCombineMask (display, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
    XShapeCombineMask (display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);

    /* ONLYSHAPE OFF */

    mywmhints.initial_state = WithdrawnState;
    mywmhints.icon_window = iconwin;
    mywmhints.icon_x = mysizehints.x;
    mywmhints.icon_y = mysizehints.y;
    mywmhints.window_group = win;
    mywmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;

    XSetWMHints (display, win, &mywmhints);

    XSetCommand (display, win, argv, argc);
    XMapWindow (display, win);

    if (geometry)
      {
	  if (sscanf (geometry, "+%d+%d", &wx, &wy) != 2)
	    {
		fprintf (stderr, "Bad geometry string.\n");
		exit (1);
	    }
	  XMoveWindow (display, win, wx, wy);
      }
}

