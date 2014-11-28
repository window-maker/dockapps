/*
 * Copyright (c) 1999-2005 Alfredo K. Kojima, Alban Hertroys
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

 */

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>

#include "dockapp.h"
#include "daargs.h"

extern struct DAContext *_daContext;

/* Local typedef */
typedef enum {
    daXpmSourceData,
    daXpmSourceFile
} daXpmSource;

/* Function prototype */
Bool _daMakePixmap(daXpmSource source,
	char **data, Pixmap *pixmap, Pixmap *mask,
	unsigned short *width, unsigned short *height);



void
DASetShapeWithOffset(Pixmap shapeMask, int x_ofs, int y_ofs)
{
    DASetShapeWithOffsetForWindow(DAWindow, shapeMask, x_ofs, y_ofs);
}

void
DASetShapeWithOffsetForWindow(Window window, Pixmap shapeMask,
	int x_ofs, int y_ofs)
{
    XShapeCombineMask(DADisplay, window, ShapeBounding, -x_ofs, -y_ofs,
	    shapeMask, ShapeSet);
    XFlush(DADisplay);
}

void
DASetPixmap(Pixmap pixmap)
{
    DASetPixmapForWindow(DAWindow, pixmap);
}

void
DASetPixmapForWindow(Window window, Pixmap pixmap)
{
    XSetWindowBackgroundPixmap(DADisplay, window, pixmap);
    XClearWindow(DADisplay, window);
    XFlush(DADisplay);
}

Pixmap
DAMakePixmap(void)
{
    return (XCreatePixmap(DADisplay, DAWindow,
		_daContext->width, _daContext->height,
		DADepth));
}

Pixmap
DAMakeShape(void)
{
    return (XCreatePixmap(DADisplay, DAWindow,
		_daContext->width, _daContext->height,
		1));
}

Bool
DAMakePixmapFromData(char **data, Pixmap *pixmap, Pixmap *mask,
	unsigned short *width, unsigned short *height)
{
    return _daMakePixmap(daXpmSourceData, data,
	    pixmap, mask,
	    width, height);
}

Bool
DAMakePixmapFromFile(char *filename, Pixmap *pixmap, Pixmap *mask,
	unsigned short *width, unsigned short *height)
{
    if (access(filename, R_OK) < 0)
	return False;

    return _daMakePixmap(daXpmSourceFile, (char**)filename,
	    pixmap, mask,
	    width, height);
}



Bool
_daMakePixmap(daXpmSource source,
	char **data, Pixmap *pixmap, Pixmap *mask,
	unsigned short *width, unsigned short *height)
{
    XpmAttributes	xpmAttr;

    xpmAttr.valuemask = XpmCloseness;
    xpmAttr.closeness = 40000;


    if (source == daXpmSourceData
	    && (XpmCreatePixmapFromData(
		    DADisplay, DAWindow, data, pixmap, mask, &xpmAttr) != 0))
	return False;

    else if (source == daXpmSourceFile
    	    && (XpmReadFileToPixmap(
		    DADisplay, DAWindow, (char*)data, pixmap, mask, &xpmAttr) != 0))
	return False;

    *width = xpmAttr.width;
    *height = xpmAttr.height;

    return True;
}

