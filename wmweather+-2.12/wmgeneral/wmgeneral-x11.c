#include "../config.h"

/*
    Best viewed with vim5, using ts=4

    wmgeneral was taken from wmppp.

    It has a lot of routines which most of the wm* programs use.

    ------------------------------------------------------------

    Author: Martijn Pieterse (pieterse@xs4all.nl)

    ---
    CHANGES:
    ---
    15/08/2002 (Brad Jorsch, anomie@users.sourceforge.net)
        * Pulled createXBMfromXPM into its own file, because it's the same in
          both -gtk and -x11.

    11/08/2002 (Brad Jorsch, anomie@users.sourceforge.net)
        * Removed the rc-file and mouse region stuff to their own files.
        * Renamed this file to "wmgeneral-x11.c"
        * Renamed a few of the functions

    28/08/2001 (Brad Jorsch, anomie@users.sourceforge.net)
        * Added EnableMouseRegion and DisableMouseRegion
        * Got annoyed with the 81-character lines. Fixed it. If you don't like
          it, find a different copy of wmgeneral.c ;)
        * GraphicsExpose events are enabled here.
        * GetXPM is exported. It optionally takes an XpmColorSymbol array.
        * GetColor is exported.

    30/09/2000 (Brad Jorsch, anomie@users.sourceforge.net)
    * You know, wmgen.mask sounds like a much nicer place to store the
      mask... why don't we do that?

    21/09/1999 (Brad Jorsch, anomie@users.sourceforge.net)
        * Changed openXwindow to use only the filename, sans path,
          as the name and class properties of the app.

    14/09/1998 (Dave Clark, clarkd@skyia.com)
        * Updated createXBMfromXPM routine
        * Now supports >256 colors
    11/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Removed a bug from parse_rcfile. You could
          not use "start" in a command if a label was
          also start.
        * Changed the needed geometry string.
          We don't use window size, and don't support
          negative positions.
    03/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Added parse_rcfile2
    02/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Added -geometry support (untested)
    28/08/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Added createXBMfromXPM routine
        * Saves a lot of work with changing xpm's.
    02/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * changed the read_rc_file to parse_rcfile, as suggested by Marcelo E. Magallon
        * debugged the parse_rc file.
    30/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Ripped similar code from all the wm* programs,
          and put them in a single file.

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

#include "wmgeneral-x11.h"

  /*****************/
 /* X11 Variables */
/*****************/

Window      Root;
int         screen;
int         x_fd;
int         d_depth;
XSizeHints  mysizehints;
XWMHints    mywmhints;
Pixel       back_pix, fore_pix;
char        *Geometry = "";
Window      iconwin, win;
GC          NormalGC;
GC          RedrawGC;
XpmIcon     wmgen;

  /***********************/
 /* Function Prototypes */
/***********************/

void RedrawWindow(void);

/******************************************************************************\
|* GetXPM                                                                     *|
\******************************************************************************/

void GetXPM(XpmIcon *wmgen, char *pixmap_bytes[]) {

    XWindowAttributes   attributes;
    int                 err;

    /* For the colormap */
    XGetWindowAttributes(display, Root, &attributes);

    wmgen->attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);
    err = XpmCreatePixmapFromData(display, Root, pixmap_bytes, &(wmgen->pixmap),
                    &(wmgen->mask), &(wmgen->attributes));

    if (err != XpmSuccess) {
        fprintf(stderr, "Not enough free colorcells.\n");
        exit(1);
    }
}

/******************************************************************************\
|* GetColor                                                                   *|
\******************************************************************************/

Pixel GetColor(char *name) {

    XColor              color;
    XWindowAttributes   attributes;

    XGetWindowAttributes(display, Root, &attributes);

    color.pixel = 0;
    if (!XParseColor(display, attributes.colormap, name, &color)) {
        fprintf(stderr, "wm.app: can't parse %s.\n", name);
    } else if (!XAllocColor(display, attributes.colormap, &color)) {
        fprintf(stderr, "wm.app: can't allocate %s.\n", name);
    }
    return color.pixel;
}

/******************************************************************************\
|* flush_expose                                                               *|
\******************************************************************************/

static int flush_expose(Window w) {

    XEvent      dummy;
    int         i=0;

    while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
        i++;

    return i;
}

/******************************************************************************\
|* RedrawWindow                                                               *|
\******************************************************************************/

void RedrawWindow(void) {

    flush_expose(iconwin);
    XCopyArea(display, wmgen.pixmap, iconwin, RedrawGC,
                0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
    flush_expose(win);
    XCopyArea(display, wmgen.pixmap, win, RedrawGC,
                0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}

/******************************************************************************\
|* RedrawWindowXY                                                             *|
\******************************************************************************/

void RedrawWindowXY(int x, int y) {

    flush_expose(iconwin);
    XCopyArea(display, wmgen.pixmap, iconwin, RedrawGC,
                x,y, wmgen.attributes.width, wmgen.attributes.height, 0,0);
    flush_expose(win);
    XCopyArea(display, wmgen.pixmap, win, RedrawGC,
                x,y, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}

/******************************************************************************\
|* copyXPMArea                                                                *|
\******************************************************************************/

void copyPixmapArea(int x, int y, int sx, int sy, int dx, int dy) {

    XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);

}

/******************************************************************************\
|* copyXBMArea                                                                *|
\******************************************************************************/

void copyMaskArea(int x, int y, int sx, int sy, int dx, int dy) {

    XCopyArea(display, wmgen.mask, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}


/******************************************************************************\
|* setMaskXY                                                                  *|
\******************************************************************************/

void setMaskXY(int x, int y) {

     XShapeCombineMask(display, win, ShapeBounding, x, y, wmgen.mask, ShapeSet);
     XShapeCombineMask(display, iconwin, ShapeBounding, x, y, wmgen.mask, ShapeSet);
}

/******************************************************************************\
|* openXwindow                                                                *|
\******************************************************************************/
void openDockWindow(int argc, char *argv[], char *pixmap_bytes[], char *pixmask_bits, int pixmask_width, int pixmask_height) {

    unsigned int    borderwidth = 1;
    XClassHint      classHint;
    char            *display_name = NULL;
    char            *wname;
    XTextProperty   name;

    XGCValues       gcv;
    unsigned long   gcm;

    char            *geometry = NULL;

    int             dummy=0;
    int             i, wx, wy;

    wname=strrchr(argv[0], '/');
    if(wname==NULL) wname=argv[0];
    else wname++;

    for (i=1; argv[i]; i++) {
        if (!strcmp(argv[i], "-display")) {
            display_name = argv[i+1];
            i++;
        }
        if (!strcmp(argv[i], "-geometry")) {
            geometry = argv[i+1];
            i++;
        }
    }

    if (!(display = XOpenDisplay(display_name))) {
        fprintf(stderr, "%s: can't open display %s\n",
                        wname, XDisplayName(display_name));
        exit(1);
    }
    screen  = DefaultScreen(display);
    Root    = RootWindow(display, screen);
    d_depth = DefaultDepth(display, screen);
    x_fd    = XConnectionNumber(display);

    /* Convert XPM to XImage */
    GetXPM(&wmgen, pixmap_bytes);

    /* Create a window to hold the stuff */
    mysizehints.flags = USSize | USPosition;
    mysizehints.x = 0;
    mysizehints.y = 0;

    back_pix = GetColor("white");
    fore_pix = GetColor("black");

    XWMGeometry(display, screen, Geometry, NULL, borderwidth, &mysizehints,
                &mysizehints.x, &mysizehints.y,&mysizehints.width,&mysizehints.height, &dummy);

    mysizehints.width = 64;
    mysizehints.height = 64;

    win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,
                mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

    iconwin = XCreateSimpleWindow(display, win, mysizehints.x, mysizehints.y,
                mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

    /* Activate hints */
    XSetWMNormalHints(display, win, &mysizehints);
    classHint.res_name = wname;
    classHint.res_class = wname;
    XSetClassHint(display, win, &classHint);

    XSelectInput(display, win, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
    XSelectInput(display, iconwin, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

    if (XStringListToTextProperty(&wname, 1, &name) == 0) {
        fprintf(stderr, "%s: can't allocate window name\n", wname);
        exit(1);
    }

    XSetWMName(display, win, &name);

    /* Create GC for drawing */

    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = fore_pix;
    gcv.background = back_pix;
    gcv.graphics_exposures = True;
    NormalGC = XCreateGC(display, Root, gcm, &gcv);
    gcv.graphics_exposures = False;
    RedrawGC = XCreateGC(display, Root, gcm, &gcv);

    /* ONLYSHAPE ON */

    if(pixmask_bits!=NULL){
        XFreePixmap(display, wmgen.mask);
        wmgen.mask = XCreateBitmapFromData(display, win, pixmask_bits, pixmask_width, pixmask_height);
    }

    XShapeCombineMask(display, win, ShapeBounding, 0, 0, wmgen.mask, ShapeSet);
    XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, wmgen.mask, ShapeSet);

    /* ONLYSHAPE OFF */

    mywmhints.initial_state = WithdrawnState;
    mywmhints.icon_window = iconwin;
    mywmhints.icon_x = mysizehints.x;
    mywmhints.icon_y = mysizehints.y;
    mywmhints.window_group = win;
    mywmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;

    XSetWMHints(display, win, &mywmhints);

    XSetCommand(display, win, argv, argc);
    XMapWindow(display, win);

    if (geometry) {
        if (sscanf(geometry, "+%d+%d", &wx, &wy) != 2) {
            fprintf(stderr, "Bad geometry string.\n");
            exit(1);
        }
        XMoveWindow(display, win, wx, wy);
    }
}
