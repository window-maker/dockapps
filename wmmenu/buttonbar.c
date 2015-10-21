#include <assert.h>

#include <libdockapp/dockapp.h>

#include "buttonbar.h"
#include "xobjects.h"
#include "pixmaps.h"
#include "options.h"
#include "menu.h"

/* item number for last known highlight position */
static int OldPos = -1 ;
/* Graphic context used to draw highlight */
static GC HighlightGC = 0 ;
/* position where bar will be shown next time */
static int BarX=0, BarY=0 ;

extern void ButtonBar_Build (void)
{
    int i, n ;
    int h, j, w ;
    int width, height ;
    GC imgOp ;
    GC maskOp ;
    XGCValues gc ;
    Pixmap image, mask ;
    int x, y ;
    int x0, y0 ;
    XSetWindowAttributes wa ;

    n = Menu_GetNbEntries () ;
    assert (n > 0) ;
    h = Menu_GetNbRows () ;
    w = Menu_GetNbColumns () ;
    width = TileXSize * w ;
    height = TileYSize * h ;


    ButtonBarWindow = XCreateSimpleWindow (
        DADisplay, DefaultRootWindow (DADisplay), 0, 0, width, height, 0,
        BlackPixel (DADisplay, DefaultScreen (DADisplay)),
        BlackPixel (DADisplay, DefaultScreen (DADisplay))) ;
    ButtonBarImage = XCreatePixmap (DADisplay, ButtonBarWindow,
        width, height, DADepth) ;
    imgOp = XCreateGC (DADisplay, ButtonBarWindow, 0, & gc) ;

    HighlightBehindMask = XCreatePixmap (DADisplay, ButtonBarWindow,
	width, height, 1) ;
    maskOp = XCreateGC (DADisplay, HighlightBehindMask, 0, & gc) ;

    /* first apply tile to whole bar */
    for (i=0; i<w; i++)
        for (j=0; j<h; j++)
            XCopyArea (DADisplay, TileImage, ButtonBarImage, imgOp, 0, 0,
                TileXSize, TileYSize, TileXSize*i, TileYSize*j) ;

    /* initialize "behind" mask to all 1's */
    XSetFunction (DADisplay, maskOp, GXset) ;
    XFillRectangle (DADisplay, HighlightBehindMask, maskOp,
	0, 0, width, height) ;
    /* and copy behind mask cells from highlight mask */
    if (HighlightMask != 0)
    {
	XSetFunction (DADisplay, maskOp, GXcopy) ;
	for (i=0; i<w; i++)
	    for (j=0; j<h; j++)
		XCopyArea (DADisplay, HighlightMask, HighlightBehindMask,
		    maskOp,
		    0, 0, TileXSize, TileYSize, TileXSize*i, TileYSize*j) ;
    }

    /* then apply each pixmap of menu entry */
    /* and build a global invert mask for highlight "behind" */
    for (i=0; i<n; i++)
    {
        Pixmaps_FindLoad (Menu_GetEntryPixmap (i),
            & image, & mask, & width, & height) ;

	/* center pixmap within its cell */
        x = (width >= TileXSize ? 0 : (TileXSize-width)/2) ;
        y = (height >= TileYSize ? 0 : (TileYSize-height)/2) ;

	/* use GC to draw with pixmap's mask in the right cell */
	x0 = TileXSize*(i/h) ;
	y0 = TileYSize*(i%h) ;
        gc.clip_x_origin = x0 + x ;
        gc.clip_y_origin = y0 + y ;
        gc.clip_mask = mask ; /* may be None */
        XChangeGC (DADisplay, imgOp,
            GCClipXOrigin | GCClipYOrigin | GCClipMask, & gc) ;
        XCopyArea (DADisplay, image, ButtonBarImage, imgOp, 0, 0,
            width, height, gc.clip_x_origin, gc.clip_y_origin) ;

	/* update highlight behind mask: cell &= ~iconmask */
	if (HighlightBehind && mask != None)
	{
	    /* use "andInverted" (dst &= ~src) */
	    XSetFunction (DADisplay, maskOp, GXandInverted) ;
	    XCopyArea (DADisplay, mask, HighlightBehindMask, maskOp,
		0, 0, width, height, gc.clip_x_origin, gc.clip_y_origin) ;
	}
	/* or highlight behind cell &= 0 if no mask */
	else
	if (HighlightBehind && mask == None)
	{
	    XSetFunction (DADisplay, maskOp, GXclear) ;
	    XFillRectangle (DADisplay, HighlightBehindMask, maskOp,
		gc.clip_x_origin, gc.clip_y_origin, width, height) ;
	}

        XFreePixmap (DADisplay, image) ;
        if (mask != 0) XFreePixmap (DADisplay, mask) ;
    }

    XFreeGC (DADisplay, imgOp) ;
    XFreeGC (DADisplay, maskOp) ;

    wa.background_pixmap = ButtonBarImage ;
    wa.event_mask = ButtonPressMask | ButtonReleaseMask ;
    if (! ClickOnly) wa.event_mask |= EnterWindowMask | LeaveWindowMask ;
    if (HighlightImage != 0) wa.event_mask |= PointerMotionMask ;
    wa.override_redirect = True ;
    XChangeWindowAttributes (DADisplay, ButtonBarWindow,
        CWBackPixmap | CWEventMask | CWOverrideRedirect, & wa) ;
}

extern void ButtonBar_SetPositionFromDockApp (int dockx, int docky,
    int dockw, int dockh)
{
    int xMid, scrWidth, h, scrHeight ;
    int x, y ;

    (void) dockh;

    /* compute y */
    scrHeight = DisplayHeight (DADisplay, DefaultScreen (DADisplay)) ;
    y = docky ;
    h = TileYSize * Menu_GetNbRows () ;
    if (y + h >= scrHeight)
    {
        y = scrHeight - h ;
    }

    /* compute x */
    scrWidth = DisplayWidth (DADisplay, DefaultScreen (DADisplay)) ;
    xMid = dockx + dockw/2 ;
    if (xMid*2 < scrWidth)
    {
        /* we are rather on left, expand to right */
        x = dockx + dockw ;
    }
    else
    {
        /* we are rather on right, expand to left */
        x = dockx - TileXSize * Menu_GetNbColumns () ;
    }

    BarX = x ;
    BarY = y ;
}

extern void ButtonBar_Show (void)
{
    XMoveWindow (DADisplay, ButtonBarWindow, BarX, BarY) ;
    XMapRaised (DADisplay, ButtonBarWindow) ;
}

static void BuildHighlightGC (void)
{
    XGCValues gc ;
    gc.clip_mask = HighlightBehindMask ;
    HighlightGC = XCreateGC (DADisplay, DAWindow,
	GCClipMask, & gc) ;
}

extern void ButtonBar_Highlight (int col, int row)
{
    int h ;
    int newPos ;

    if (HighlightImage == 0) return ;

    h = Menu_GetNbRows () ;
    newPos = col*h + row ;

    if (newPos != OldPos)
    {
	int x, y ;
	XGCValues gc ;

	/* first clear old highlight position */
	ButtonBar_Unhighlight () ;

	/* don't draw highlight on empty slots */
	if (newPos >= Menu_GetNbEntries ()) return ;

	/* compute new draw position */
	x = col * TileXSize ;
	y = row * TileYSize ;
	/* and draw it */
	if (HighlightGC == 0) BuildHighlightGC () ;
	gc.clip_x_origin = HighlightBehind ? 0 : x ;
	gc.clip_y_origin = HighlightBehind ? 0 : y ;
        XChangeGC (DADisplay, HighlightGC,
            GCClipXOrigin | GCClipYOrigin, & gc) ;
        XCopyArea (DADisplay, HighlightImage, ButtonBarWindow,
	    HighlightGC, 0, 0, TileXSize, TileYSize, x, y) ;

	OldPos = newPos ;
    }
}

extern void ButtonBar_Unhighlight (void)
{
    int x, y, h ;

    if (HighlightImage == 0 || OldPos < 0) return ;

    h = Menu_GetNbRows () ;
    x = (OldPos / h) * TileXSize ;
    y = (OldPos % h) * TileYSize ;
    XClearArea (DADisplay, ButtonBarWindow,
		x, y, TileXSize, TileYSize, False) ;
    OldPos = -1 ;
}

extern void ButtonBar_Hide (void)
{
    XUnmapWindow (DADisplay, ButtonBarWindow) ;
    OldPos = -1 ;
}

extern void ButtonBar_Rebuild (void)
{
    XDestroyWindow (DADisplay, ButtonBarWindow) ;
    if (HighlightGC != 0)
    {
	XFreeGC (DADisplay, HighlightGC) ;
	HighlightGC = 0 ;
    }
    OldPos = -1 ;
    ButtonBar_Build () ;
}
