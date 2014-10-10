/*
 * Copyright (c) 1999 Alfredo K. Kojima
 * Copyright (c) 2001, 2002 Seiichi SATO <ssato@sh.rim.or.jp>
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

 * This code is based on libdockapp-0.4.0
 * modified by Seiichi SATO <ssato@sh.rim.or.jp>
 */

#include "dockapp.h"

#define WINDOWED_SIZE_W 64
#define WINDOWED_SIZE_H 64

/* global */
Display	*display = NULL;
Bool	dockapp_iswindowed = False;
Bool	dockapp_isbrokenwm = False;

/* private */
static Window	window = None;
static Window	icon_window = None;
static GC	gc = NULL;
static int	depth = 0;
static Atom	delete_win;
static int	width, height;
static int	offset_w, offset_h;

void
dockapp_open_window(char *display_specified, char *appname,
		    unsigned w, unsigned h, int argc, char **argv)
{
    XClassHint	    *classhint;
    XWMHints	    *wmhints;
    XTextProperty   title;
    XSizeHints	    sizehints;
    Window	    root;
    int		    ww, wh;

    /* Open Connection to X Server */
    display = XOpenDisplay(display_specified);
    if (!display) {
	fprintf(stderr, "%s: could not open display %s!\n", argv[0],
		XDisplayName(display_specified));
	exit(1);
    }
    root = DefaultRootWindow(display);

    width = w;
    height = h;

    if (dockapp_iswindowed) {
	offset_w = (WINDOWED_SIZE_W - w) / 2;
	offset_h = (WINDOWED_SIZE_H - h) / 2;
	ww = WINDOWED_SIZE_W;
	wh = WINDOWED_SIZE_H;
    } else {
	offset_w = offset_h = 0;
	ww = w;
	wh = h;
    }

    /* Create Windows */
    icon_window = XCreateSimpleWindow(display, root, 0, 0, ww, wh, 0, 0, 0);
    if (dockapp_isbrokenwm) {
	window = XCreateSimpleWindow(display, root, 0, 0, ww, wh, 0, 0, 0);
    } else {
	window = XCreateSimpleWindow(display, root, 0, 0, 1, 1, 0, 0, 0);
    }

    /* Set ClassHint */
    classhint = XAllocClassHint();
    if (classhint == NULL) {
	fprintf(stderr, "%s: can't allocate memory for wm hints!\n", argv[0]);
	exit(1);
    }
    classhint->res_class = "DockApp";
    classhint->res_name = appname;
    XSetClassHint(display, window, classhint);
    XFree(classhint);

    /* Set WMHints */
    wmhints = XAllocWMHints();
    if (wmhints == NULL) {
	fprintf(stderr, "%s: can't allocate memory for wm hints!\n", argv[0]);
	exit(1);
    }
    wmhints->flags = IconWindowHint | WindowGroupHint;
    if (!dockapp_iswindowed) {
	wmhints->flags |= StateHint;
	wmhints->initial_state = WithdrawnState;
    }
    wmhints->window_group = window;
    wmhints->icon_window = icon_window;
    XSetWMHints(display, window, wmhints);
    XFree(wmhints);

    /* Set WM Protocols */
    delete_win = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols (display, icon_window, &delete_win, 1);

    /* Set Size Hints */
    sizehints.flags = USSize;
    if (!dockapp_iswindowed) {
	sizehints.flags |= USPosition;
	sizehints.x = sizehints.y = 0;
    } else {
	sizehints.flags |= PMinSize | PMaxSize;
	sizehints.min_width = sizehints.max_width = WINDOWED_SIZE_W;
	sizehints.min_height = sizehints.max_height = WINDOWED_SIZE_H;
    }
    sizehints.width = ww;
    sizehints.height = wh;
    XSetWMNormalHints(display, icon_window, &sizehints);

    /* Set WindowTitle for AfterStep Wharf */
    XStringListToTextProperty(&appname, 1, &title);
    XSetWMName(display, window, &title);
    XSetWMName(display, icon_window, &title);

    /* Set Command to start the app so it can be docked properly */
    XSetCommand(display, window, argv, argc);

    depth = DefaultDepth(display, DefaultScreen(display));
    gc = DefaultGC(display, DefaultScreen(display));

    XFlush(display);
}


void
dockapp_set_eventmask(long mask)
{
    XSelectInput(display, icon_window, mask);
    XSelectInput(display, window, mask);
}


static Pixmap
create_bg_pixmap(void)
{
    Pixmap bg;

    bg = XCreatePixmap(display, icon_window, WINDOWED_SIZE_W, WINDOWED_SIZE_H,
		       depth);
    XSetForeground(display, gc, dockapp_getcolor("rgb:ae/aa/ae"));
    XFillRectangle(display, bg, gc, 0, 0, WINDOWED_SIZE_W, WINDOWED_SIZE_H);
    XSetForeground(display, gc, dockapp_getcolor("rgb:ff/ff/ff"));
    XDrawLine(display, bg, gc, 0, 0, 0, 63);
    XDrawLine(display, bg, gc, 1, 0, 1, 62);
    XDrawLine(display, bg, gc, 2, 0, 63, 0);
    XDrawLine(display, bg, gc, 2, 1, 62, 1);
    XSetForeground(display, gc, dockapp_getcolor("rgb:52/55/52"));
    XDrawLine(display, bg, gc, 1, 63, 63, 63);
    XDrawLine(display, bg, gc, 2, 62, 63, 62);
    XDrawLine(display, bg, gc, 63, 1, 63, 61);
    XDrawLine(display, bg, gc, 62, 2, 62, 61);

    return bg;
}


void
dockapp_set_background(Pixmap pixmap)
{
    if (dockapp_iswindowed) {
	Pixmap bg;
	bg = create_bg_pixmap();
	XCopyArea(display, pixmap, bg, gc, 0, 0, width, height,
		  offset_w, offset_w);
	XSetWindowBackgroundPixmap(display, icon_window, bg);
	XSetWindowBackgroundPixmap(display, window, bg);
	XFreePixmap(display, bg);
    } else {
	XSetWindowBackgroundPixmap(display, icon_window, pixmap);
	XSetWindowBackgroundPixmap(display, window, pixmap);
    }
    XClearWindow(display, icon_window);
    XFlush(display);
}


void
dockapp_show(void)
{
    if (!dockapp_iswindowed)
	XMapRaised(display, window);
    else
	XMapRaised(display, icon_window);

    XFlush(display);
}


Bool
dockapp_xpm2pixmap(char **data, Pixmap *pixmap, Pixmap *mask,
		   XpmColorSymbol * colorSymbol, unsigned int nsymbols)
{
    XpmAttributes xpmAttr;
    xpmAttr.valuemask = XpmCloseness;
    xpmAttr.closeness = 40000;

    if (nsymbols) {
	xpmAttr.colorsymbols = colorSymbol;
	xpmAttr.numsymbols = nsymbols;
	xpmAttr.valuemask |= XpmColorSymbols;
    }

    if (XpmCreatePixmapFromData(display, icon_window, data, pixmap, mask, &xpmAttr) != 0)
	return False;

    return True;
}


Pixmap
dockapp_XCreatePixmap(int w, int h)
{
    return (XCreatePixmap(display, icon_window, w, h, depth));
}


void
dockapp_setshape(Pixmap mask, int x_ofs, int y_ofs)
{
    XShapeCombineMask(display, icon_window, ShapeBounding, -x_ofs, -y_ofs,
		      mask, ShapeSet);
    XShapeCombineMask(display, window, ShapeBounding, -x_ofs, -y_ofs,
		      mask, ShapeSet);
    XFlush(display);
}


void
dockapp_copyarea(Pixmap src, Pixmap dist, int x_src, int y_src, int w, int h,
		 int x_dist, int y_dist)
{
    XCopyArea(display, src, dist, gc, x_src, y_src, w, h, x_dist, y_dist);
}


void
dockapp_copy2window (Pixmap src)
{
    if (dockapp_isbrokenwm) {
	XCopyArea(display, src, window, gc, 0, 0, width, height, offset_w,
		  offset_h);
    } else {
	XCopyArea(display, src, icon_window, gc, 0, 0, width, height, offset_w,
		  offset_h);
    }
}


Bool
dockapp_nextevent_or_timeout(XEvent *event, unsigned long miliseconds)
{
    struct timeval timeout;
    fd_set rset;

    XSync(display, False);
    if (XPending(display)) {
	XNextEvent(display, event);
	return True;
    }

    timeout.tv_sec = miliseconds / 1000;
    timeout.tv_usec = (miliseconds % 1000) * 1000;

    FD_ZERO(&rset);
    FD_SET(ConnectionNumber(display), &rset);
    if (select(ConnectionNumber(display)+1, &rset, NULL, NULL, &timeout) > 0) {
	XNextEvent(display, event);
	if (event->type == ClientMessage) {
	    if (event->xclient.data.l[0] == delete_win) {
		XDestroyWindow(display,event->xclient.window);
		XCloseDisplay(display);
		exit(0);
	    }
	}
	if (dockapp_iswindowed) {
		event->xbutton.x -= offset_w;
		event->xbutton.y -= offset_h;
	}
	return True;
    }

    return False;
}


unsigned long
dockapp_getcolor(char *color_name)
{
    XColor color;

    if (!XParseColor(display, DefaultColormap(display, DefaultScreen(display)),
		     color_name, &color))
	fprintf(stderr, "can't parse color %s\n", color_name), exit(1);

    if (!XAllocColor(display, DefaultColormap(display, DefaultScreen(display)),
		     &color)) {
	fprintf(stderr, "can't allocate color %s. Using black\n", color_name);
	return BlackPixel(display, DefaultScreen(display));
    }

    return color.pixel;
}


unsigned long
dockapp_blendedcolor(char *color_name, int r, int g, int b, float fac)
{
    XColor color;

    if ((r < -255 || r > 255)||(g < -255 || g > 255)||(b < -255 || b > 255)){
	fprintf(stderr, "r:%d,g:%d,b:%d (r,g,b must be 0 to 255)", r, g, b);
	exit(1);
    }

    r *= 255;
    g *= 255;
    b *= 255;

    if (!XParseColor(display, DefaultColormap(display, DefaultScreen(display)),
		     color_name, &color))
	fprintf(stderr, "can't parse color %s\n", color_name), exit(1);

    if (!XAllocColor(display, DefaultColormap(display, DefaultScreen(display)),
		     &color)) {
	fprintf(stderr, "can't allocate color %s. Using black\n", color_name);
	return BlackPixel(display, DefaultScreen(display));
    }

    if (DefaultDepth(display, DefaultScreen(display)) < 16)
	return color.pixel;

    /* red */
    if (color.red + r > 0xffff) {
	color.red = 0xffff;
    } else if (color.red + r < 0) {
	color.red = 0;
    } else {
	color.red = (unsigned short)(fac * color.red + r);
    }

    /* green */
    if (color.green + g > 0xffff) {
	color.green = 0xffff;
    } else if (color.green + g < 0) {
	color.green = 0;
    } else {
	color.green = (unsigned short)(fac * color.green + g);
    }

    /* blue */
    if (color.blue + b > 0xffff) {
	color.blue = 0xffff;
    } else if (color.blue + b < 0) {
	color.blue = 0;
    } else {
	color.blue = (unsigned short)(fac * color.blue + b);
    }

    color.flags = DoRed | DoGreen | DoBlue;

    if (!XAllocColor(display, DefaultColormap(display, DefaultScreen(display)),
		     &color)) {
	fprintf(stderr, "can't allocate color %s. Using black\n", color_name);
	return BlackPixel(display, DefaultScreen(display));
    }

    return color.pixel;
}
