/*
 *   xutils.c - A collection of X-windows utilties for creating WindowMAker
 *		DockApps.
 *
 *     This file contains alot of the lower-level X windows routines. Origins with wmppp
 *     (by  Martijn Pieterse (pieterse@xs4all.nl)), but its been hacked up quite a bit
 *     and passed on from one new DockApp to the next.
 *
 *
 *
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2, or (at your option)
 *      any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program (see the file COPYING); if not, write to the
 *      Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *      Boston, MA 02110-1301 USA
 *
 *
 * $Id: xutils.c,v 1.2 2002/09/15 14:31:41 ico Exp $
 *
 *
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
#include "xutils.h"


Display		*display;
Window          Root;
Window          iconwin, win;
int             screen;
int             DisplayDepth;


/*
 *   X11 Variables
 */
int		x_fd;
XSizeHints	mysizehints;
XWMHints	mywmhints;
Pixel		back_pix, fore_pix;
char		*Geometry = "";
GC		NormalGC;
XpmIcon		wmgen;
Pixmap		pixmask;





/*
 *   flush_expose
 */
static int flush_expose(Window w) {

    XEvent 	dummy;
    int		i=0;

    while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
    i++;

    return i;

}









/*
 *   RedrawWindow
 *   RedrawWindowXY
 */
void RedrawWindow(void) {

    flush_expose(iconwin);
    XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, 0,0, wmgen.attributes.width, wmgen.attributes.height, 0, 0);

    flush_expose(win);
    XCopyArea(display, wmgen.pixmap, win, NormalGC, 0,0, wmgen.attributes.width, wmgen.attributes.height, 0, 0);

}

void RedrawWindowXY(int x, int y) {

    flush_expose(iconwin);
    XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, x,y, wmgen.attributes.width, wmgen.attributes.height, 0, 0);

    flush_expose(win);
    XCopyArea(display, wmgen.pixmap, win, NormalGC, x,y, wmgen.attributes.width, wmgen.attributes.height, 0, 0);

}







/*
 *   copyXPMArea
 *   copyXBMArea
 */
void copyXPMArea(int x, int y, int sx, int sy, int dx, int dy) {
	XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}

void copyXBMArea(int x, int y, int sx, int sy, int dx, int dy) {

	XCopyArea(display, wmgen.mask, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}



/*
 *   initXwindow
 */
void initXwindow(int argc, char *argv[]){

    int		 i;
    char	*display_name = NULL;

    for (i=1; argv[i]; ++i) {
        if (!strcmp(argv[i], "-display")) display_name = argv[i+1];
    }


    if (!(display = XOpenDisplay(display_name))) {
        fprintf(stderr, "%s: can't open display %s\n",
                argv[0], XDisplayName(display_name));
        exit(1);
    }


    screen  	 = DefaultScreen(display);
    Root    	 = RootWindow(display, screen);
    DisplayDepth = DefaultDepth(display, screen);
    x_fd    	 = XConnectionNumber(display);

}





/*
 *   openXwindow
 */
void openXwindow(int argc, char *argv[], char *pixmap_bytes[], char *pixmask_bits,
    int pixmask_width, int pixmask_height, char *BackColor, char *LabelColor,
    char *WindGustColor, char *DataColor, char *StationTimeColor) {

    unsigned int	borderwidth = 1;
    XClassHint		classHint;
    char		*wname = argv[0];
    XTextProperty	name;
    XGCValues		gcv;
    unsigned long	gcm;
    int			dummy=0;
    XpmColorSymbol 	cols[5]={	{"BackColor", NULL, 0},
					{"LabelColor", NULL, 0},
    					{"DataColor", NULL, 0},
					{"WindGustColor", NULL, 0},
					{"StationTimeColor", NULL, 0}   };




    /*
     *  Create Pixmap
     */
    cols[0].pixel		  = getColor(BackColor, 1.0);
    cols[1].pixel		  = getColor(LabelColor, 1.0);
    cols[2].pixel		  = getColor(DataColor, 1.0);
    cols[3].pixel		  = getColor(WindGustColor, 1.0);
    cols[4].pixel		  = getColor(StationTimeColor, 1.0);
    wmgen.attributes.numsymbols   = 5;
    wmgen.attributes.colorsymbols = cols;
    wmgen.attributes.exactColors  = False;
    wmgen.attributes.closeness    = 40000;
    wmgen.attributes.valuemask    = XpmReturnPixels | XpmReturnExtensions | XpmColorSymbols
							| XpmExactColors | XpmCloseness | XpmSize;
    if (XpmCreatePixmapFromData(display, Root, pixmap_bytes,
      &(wmgen.pixmap), &(wmgen.mask), &(wmgen.attributes)) != XpmSuccess){
	fprintf(stderr, "Not enough free colorcells.\n");
	exit(1);
    }




    /*
     *  Create a window
     */
    mysizehints.flags = USSize | USPosition;
    mysizehints.x = 0;
    mysizehints.y = 0;

    back_pix = getColor("white", 1.0);
    fore_pix = getColor("black", 1.0);

    XWMGeometry(display, screen, Geometry, NULL, borderwidth, &mysizehints,
				&mysizehints.x, &mysizehints.y,&mysizehints.width,&mysizehints.height, &dummy);

    mysizehints.width = 64;
    mysizehints.height = 64;



    win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,
				mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

    iconwin = XCreateSimpleWindow(display, win, mysizehints.x, mysizehints.y,
				mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);



    /*
     *  Activate hints
     */
    XSetWMNormalHints(display, win, &mysizehints);
    classHint.res_name = wname;
    classHint.res_class = wname;
    XSetClassHint(display, win, &classHint);



    /*
     *  Set up the xevents that you want the relevent windows to inherit
     *  Currently, its seems that setting KeyPress events here has no
     *  effect. I.e. for some you will need to Grab the focus and then return
     *  it after you are done...
     */
    XSelectInput(display, win, ButtonPressMask | ExposureMask | ButtonReleaseMask
		| PointerMotionMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask
						| KeyPressMask | KeyReleaseMask);
    XSelectInput(display, iconwin, ButtonPressMask | ExposureMask | ButtonReleaseMask
		| PointerMotionMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask
						| KeyPressMask | KeyReleaseMask);


    if (XStringListToTextProperty(&wname, 1, &name) == 0) {
        fprintf(stderr, "%s: can't allocate window name\n", wname);
        exit(1);
    }


    XSetWMName(display, win, &name);

    /*
     *   Create Graphics Context (GC) for drawing
     */
    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = fore_pix;
    gcv.background = back_pix;
    gcv.graphics_exposures = 0;
    NormalGC = XCreateGC(display, Root, gcm, &gcv);



    pixmask = XCreateBitmapFromData(display, win, pixmask_bits, pixmask_width, pixmask_height);
    XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
    XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);


    mywmhints.initial_state = WithdrawnState;
    mywmhints.icon_window = iconwin;
    mywmhints.icon_x = mysizehints.x;
    mywmhints.icon_y = mysizehints.y;
    mywmhints.window_group = win;
    mywmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;


    XSetWMHints(display, win, &mywmhints);


    XSetCommand(display, win, argv, argc);
    XMapWindow(display, win);

}

unsigned long getColor(char *ColorName, float fac) {

     XColor 		Color;
     XWindowAttributes 	Attributes;

     XGetWindowAttributes(display, Root, &Attributes);
     Color.pixel = 0;

     XParseColor(display, Attributes.colormap, ColorName, &Color);
     Color.red 	 = (unsigned short)(Color.red/fac);
     Color.blue  = (unsigned short)(Color.blue/fac);
     Color.green = (unsigned short)(Color.green/fac);
     Color.flags = DoRed | DoGreen | DoBlue;
     XAllocColor(display, Attributes.colormap, &Color);

     return Color.pixel;

}

