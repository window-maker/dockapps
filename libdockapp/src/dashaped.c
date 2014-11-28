/*
 * Copyright (c) 2002-2005 Alban G. Hertroys
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * $Id: dashaped.c,v 1.11 2008/05/01 09:49:06 dalroi Exp $
 */

#include <assert.h>
#include <string.h>

#include "dockapp.h"
#include "daargs.h"

/*
 * DAShapedPixmap functions
 */

/* Local typedef */
typedef enum {
    daShapeSourceData,
    daShapeSourceFile
} daShapeSource;

/* local functions */
void setGCs(DAShapedPixmap *dasp);
DAShapedPixmap* _daMakeShapedPixmap(daShapeSource source, char **data);

extern struct DAContext *_daContext;

/* Create a new shaped pixmap with width & height of dockapp window */
DAShapedPixmap*
DAMakeShapedPixmap()
{
    DAShapedPixmap *dasp = malloc(sizeof(DAShapedPixmap));

    if (dasp == NULL)
	return NULL;

    memset(dasp, 0, sizeof(DAShapedPixmap));
    dasp->pixmap = DAMakePixmap();
    dasp->shape	 = DAMakeShape();
    dasp->geometry.width  = _daContext->width;
    dasp->geometry.height = _daContext->height;

    setGCs(dasp);
    DASPClear(dasp);

    return dasp;
}


/* Create a new shaped pixmap from XPM-data */
DAShapedPixmap*
DAMakeShapedPixmapFromData(char **data)
{
    return _daMakeShapedPixmap(daShapeSourceData, data);
}


/* Create a new shaped pixmap from XPM-data */
DAShapedPixmap*
DAMakeShapedPixmapFromFile(char *filename)
{
    return _daMakeShapedPixmap(daShapeSourceFile, (char**)filename);
}


/* Free memory reserved for a shaped pixmap */
void
DAFreeShapedPixmap(DAShapedPixmap *dasp)
{
    assert(dasp);

    XFreePixmap(DADisplay, dasp->pixmap);
    XFreePixmap(DADisplay, dasp->shape);
    XFreeGC(DADisplay, dasp->shapeGC);

    free(dasp);
}

/* Copy shape-mask and pixmap-data from an area in one shaped pixmap
 * into another shaped pixmap */
void
DASPCopyArea(DAShapedPixmap *src, DAShapedPixmap *dst, int x1, int y1, int w, int h, int x2, int y2)
{
    assert(src != NULL && dst != NULL);

    XCopyPlane(DADisplay, src->shape, dst->shape, src->shapeGC, x1, y1, w, h, x2, y2, 1);
    XCopyArea(DADisplay, src->pixmap, dst->pixmap, src->drawGC, x1, y1, w, h, x2, y2);
}


/* Clear a shaped pixmap */
void
DASPClear(DAShapedPixmap *dasp)
{
    XGCValues gcv;

    assert(dasp != NULL);

    gcv.foreground = 0;
    XChangeGC(DADisplay, dasp->shapeGC, GCForeground, &gcv);

    /* Clear pixmaps */
    XFillRectangle(DADisplay, dasp->pixmap,
	    DAClearGC, 0, 0, dasp->geometry.width, dasp->geometry.height);
    XFillRectangle(DADisplay, dasp->shape,
	    dasp->shapeGC, 0, 0, dasp->geometry.width, dasp->geometry.height);

    gcv.foreground = 1;
    XChangeGC(DADisplay, dasp->shapeGC, GCForeground, &gcv);
}


/* Show the pixmap in the dockapp-window */
void
DASPSetPixmap(DAShapedPixmap *dasp)
{
    DASPSetPixmapForWindow(DAWindow, dasp);
}

void
DASPSetPixmapForWindow(Window window, DAShapedPixmap *dasp)
{
    assert(dasp != NULL);

    DASetShapeForWindow(window, dasp->shape);
    DASetPixmapForWindow(window, dasp->pixmap);
}


void
setGCs(DAShapedPixmap *dasp)
{
    XGCValues		gcv;

    dasp->drawGC  = DAGC;
    dasp->clearGC = DAClearGC;

    /* create GC for bit-plane operations in shapes */
    gcv.graphics_exposures = False;
    gcv.foreground = 1;
    gcv.background = 0;
    gcv.plane_mask = 1;

    dasp->shapeGC = XCreateGC(
	    DADisplay,
	    dasp->shape,
	    GCGraphicsExposures|GCForeground|GCBackground|GCPlaneMask,
	    &gcv);

}


/* Create a new shaped pixmap using specified method */
DAShapedPixmap*
_daMakeShapedPixmap(daShapeSource source, char **data)
{
    Bool success;
    DAShapedPixmap *dasp = malloc(sizeof(DAShapedPixmap));

    if (dasp == NULL)
	return NULL;

    memset(dasp, 0, sizeof(DAShapedPixmap));

    if (source == daShapeSourceData)
	success = DAMakePixmapFromData(data, &dasp->pixmap, &dasp->shape,
	    &dasp->geometry.width, &dasp->geometry.height);
    else
	success = DAMakePixmapFromFile((char*)data, &dasp->pixmap, &dasp->shape,
	    &dasp->geometry.width, &dasp->geometry.height);

    if (!success)
	return NULL;

    setGCs(dasp);

    return dasp;
}

