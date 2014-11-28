/* $Id: basic.c,v 1.10 2003/02/10 12:35:03 dalroi Exp $
 *
 * Copyright (c) 2002 Alban G. Hertroys
 *
 * Basic example of libDockapp usage
 *
 * This dockapp will draw a rectangle with a
 * bouncing ball in it.
 */

/* also includes Xlib, Xresources, XPM, stdlib and stdio */
#include <dockapp.h>

/* include the pixmap to use */
#include "ball_red.xpm"

#define SPEED 20
#define MAX_MOVE 4

/*
 * Prototypes for local functions
 */
void drawRelief();
void moveBall();
void destroy();

/*
 * Global variables
 */
Window ballWin;
DAShapedPixmap *ballPix;

/*
 * M A I N
 */

int
main(int argc, char **argv)
{
    unsigned int x=1, y=1;
    Pixmap back;
    DACallbacks eventCallbacks = {
	destroy, /* destroy */
	NULL, /* buttonPress */
	NULL, /* buttonRelease */
	NULL, /* motion (mouse) */
	NULL, /* mouse enters window */
	NULL, /* mouse leaves window */
	moveBall /* timeout */
    };

    /* provide standard command-line options */
    DAParseArguments(
	    argc, argv,	/* Where the options come from */
	    NULL, 0,	/* Our list with options - none as you can see */
	    "This is the help text for the basic example of how to "
	    "use libDockapp.\n",
	    "Basic example version 1.1");

    /* Tell libdockapp what version we expect it to be (a date from the
     * ChangeLog should do).
     */
    DASetExpectedVersion(20020126);

    DAOpenDisplay(
	    NULL	/* default display */,
	    argc, argv	/* needed by libdockapp */
    );
    DACreateIcon(
	    "daBasicExample"	/* WM_CLASS hint; don't use chars in [.?*: ] */,
	    48, 48		/* geometry of dockapp internals */,
	    argc, argv		/* needed by libdockapp */
    );

    /* The pixmap that makes up the background of the dockapp */
    back = DAMakePixmap();
    drawRelief(back);
    DASetPixmap(back);
    XFreePixmap(DADisplay, back);

    /* A window(!) for the ball pixmap.
     * Moving a window is a lot easier then erasing/copying the pixmap all
     * the time.
     *
     * I use a DAShapedPixmap here, which contains all the information
     * related to the pixmap: pixmap, mask and geometry.
     */
    ballPix = DAMakeShapedPixmapFromData(ball_red_xpm);
    ballWin = XCreateSimpleWindow(DADisplay, DAWindow,
	    x, y,
	    /* Use the geometry of the shaped pixmap */
	    ballPix->geometry.width, ballPix->geometry.height,
	    0, 0, 0);
    DASPSetPixmapForWindow(ballWin, ballPix);

    /* Respond to destroy and timeout events (the ones not NULL in the
     * eventCallbacks variable.
     */
    DASetCallbacks(&eventCallbacks);

    /* Set the time for timeout events (in msec) */
    DASetTimeout(SPEED);

    /* Randomize movement variation.
     * Run multiple versions of the dockapp simultaneously to see the effect
     * best.
     * (which function to use is set from the Imakefile)
     */
#ifdef HAS_RANDOMDEV
    srandomdev();
#else
    srandom(time(NULL));
#endif

    DAShow();    /* Show the dockapp window. */

    /* Process events and keep the dockapp running */
    DAEventLoop();

    /* not reached */
    exit(EXIT_SUCCESS);
}


void
drawRelief(Pixmap pixmap)
{
    XGCValues gcv;
    GC lightGrayGC, darkGrayGC;

    /* GC's */
    gcv.foreground = DAGetColor("Navy");
    XChangeGC(DADisplay, DAClearGC, GCForeground, &gcv);

    gcv.foreground = DAGetColor("lightGray");
    gcv.graphics_exposures = False;

    lightGrayGC	= XCreateGC(DADisplay, DAWindow,
	    GCForeground|GCGraphicsExposures, &gcv);

    gcv.foreground = DAGetColor("#222222");
    darkGrayGC	=  XCreateGC(DADisplay, DAWindow,
	    GCForeground|GCGraphicsExposures, &gcv);

    /* Drawing */
    XFillRectangle(DADisplay, pixmap, DAClearGC, 1, 1, 46, 46);

    XDrawLine(DADisplay, pixmap, darkGrayGC, 0, 0, 0, 46);
    XDrawLine(DADisplay, pixmap, darkGrayGC, 1, 0, 47, 0);

    XDrawLine(DADisplay, pixmap, lightGrayGC, 0, 47, 47, 47);
    XDrawLine(DADisplay, pixmap, lightGrayGC, 47, 1, 47, 46);

    /* Free the GC's, we don't use them anymore */
    XFreeGC(DADisplay, lightGrayGC);
    XFreeGC(DADisplay, darkGrayGC);
}


void
moveBall()
{
    static int x = 1;
    static int y = 1;
    static int dx = 0;
    static int dy = 0;
    signed int var = random()%3 -1;

    if (dx == 0) dx = var;
    if (dy == 0) dy = var;
    if (dx > MAX_MOVE) dx = MAX_MOVE;
    if (dy > MAX_MOVE) dy = MAX_MOVE;

    /* calculate new position */
    x += dx;
    y += dy;

    if (x < 1) {
	x = 1;
	dx = -dx + var;
    }

    if (y < 1) {
	y = 1;
	dy = -dy + var;
    }

    if (x + ballPix->geometry.width > 46) {
	x = 46 - ballPix->geometry.width;
	dx = -dx + var;
    }

    if (y + ballPix->geometry.height > 46) {
	y = 46 - ballPix->geometry.height;
	dy = -dy + var;
    }

    XMoveWindow(DADisplay, ballWin, x, y);
}


void
destroy(void)
{
    XFreePixmap(DADisplay, ballPix->pixmap);
    XFreePixmap(DADisplay, ballPix->shape);

fprintf(stderr, "Destroyed!\n");
    /* exit is done by libdockapp */
}
