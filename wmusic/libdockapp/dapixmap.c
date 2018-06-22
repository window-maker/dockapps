/*
 * Copyright (c) 1999 Alfredo K. Kojima
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

 * $Id: dapixmap.c,v 1.1.1.1 2004/02/27 02:53:10 john Exp $
 */

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>
#include "dockapp.h"

extern int	d_width, d_height;

void
DASetShapeWithOffset(Pixmap shapeMask, int x_ofs, int y_ofs)
{
    XShapeCombineMask(DADisplay, DAWindow, ShapeBounding, -x_ofs, -y_ofs, shapeMask,
	    ShapeSet);
    XFlush(DADisplay);
}

void
DASetPixmap(Pixmap pixmap)
{
    XSetWindowBackgroundPixmap(DADisplay, DAWindow, pixmap);
    XClearWindow(DADisplay, DAWindow);
    XFlush(DADisplay);
}

Pixmap
DAMakePixmap(void)
{
    return (XCreatePixmap(DADisplay, DAWindow, d_width, d_height, DADepth));
}

Bool
DAMakePixmapFromData(char **data, Pixmap *pixmap, Pixmap *mask,
	unsigned *width, unsigned *height)
{
    XpmAttributes	xpmAttr;
    
    xpmAttr.valuemask = XpmCloseness;
    xpmAttr.closeness = 40000;
    
    if (XpmCreatePixmapFromData(DADisplay, DAWindow, data,
		pixmap, mask, &xpmAttr) != 0)
	return False;
    
    *width = xpmAttr.width;
    *height = xpmAttr.height;
    
    return True;
}

