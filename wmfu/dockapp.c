/*
 * Copyright (c) 1999 Alfredo K. Kojima
 * Copyright (c) 2001, 2002 Seiichi SATO
 * Copyright (c) 2007 Daniel Borca
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "dockapp.h"


#define WINDOWED_SIZE_W 64
#define WINDOWED_SIZE_H 64
#define DOCKED_SIZE_W   (WINDOWED_SIZE_W - 4)
#define DOCKED_SIZE_H   (WINDOWED_SIZE_H - 4)


int
dockapp_open_window(DOCKAPP *d,
		    const char *display_specified,
		    char *appname, int iswindowed,
		    int argc, char **argv)
{
    XSizeHints sizehints;
    XClassHint classhint;
    XWMHints wmhints;
    XTextProperty title;
    Window root;
    int ww, wh;

    /* Open Connection to X Server */
    d->display = XOpenDisplay(display_specified);
    if (d->display == NULL) {
	return -1;
    }

    d->x_offset = d->y_offset = 0;
    ww = DOCKED_SIZE_W;
    wh = DOCKED_SIZE_H;
    if (iswindowed) {
	d->x_offset = (WINDOWED_SIZE_W - DOCKED_SIZE_W) / 2;
	d->y_offset = (WINDOWED_SIZE_H - DOCKED_SIZE_H) / 2;
	ww = WINDOWED_SIZE_W;
	wh = WINDOWED_SIZE_H;
    }

    /* Create Windows */
    root = DefaultRootWindow(d->display);
    d->icon_window = XCreateSimpleWindow(d->display, root, 0, 0, ww, wh, 0, 0, 0);
    d->window = XCreateSimpleWindow(d->display, root, 0, 0, 1, 1, 0, 0, 0);

    /* Set ClassHint */
    classhint.res_class = "DockApp";
    classhint.res_name = appname;
    XSetClassHint(d->display, d->window, &classhint);

    /* Set WMHints */
    wmhints.flags = IconWindowHint | WindowGroupHint;
    if (!iswindowed) {
	wmhints.flags |= StateHint;
	wmhints.initial_state = WithdrawnState;
    }
    wmhints.window_group = d->window;
    wmhints.icon_window = d->icon_window;
    XSetWMHints(d->display, d->window, &wmhints);

    /* Set Size Hints */
    sizehints.flags = USSize;
    if (!iswindowed) {
	sizehints.flags |= USPosition;
	sizehints.x = sizehints.y = 0;
    } else {
	sizehints.flags |= PMinSize | PMaxSize;
	sizehints.min_width = sizehints.max_width = WINDOWED_SIZE_W;
	sizehints.min_height = sizehints.max_height = WINDOWED_SIZE_H;
    }
    sizehints.width = ww;
    sizehints.height = wh;
    XSetWMNormalHints(d->display, d->icon_window, &sizehints);

    /* Set WindowTitle for AfterStep Wharf */
    XStringListToTextProperty(&appname, 1, &title);
    XSetWMName(d->display, d->window, &title);
    XSetWMName(d->display, d->icon_window, &title);
    XFree(title.value);

    /* Set WM Protocols */
    d->delete_win = XInternAtom(d->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(d->display, d->icon_window, &d->delete_win, 1);

    /* Set Command to start the app so it can be docked properly */
    XSetCommand(d->display, d->window, argv, argc);

    d->width = DOCKED_SIZE_W;
    d->height = DOCKED_SIZE_H;
    d->iswindowed = iswindowed;
    d->depth = DefaultDepth(d->display, DefaultScreen(d->display));
    d->gc = DefaultGC(d->display, DefaultScreen(d->display));

    d->quit = False;
    d->pixmap = dockapp_createpixmap(d, ww, wh);
    XSetForeground(d->display, d->gc, dockapp_get_color(d, "rgb:ae/aa/ae"));
    XFillRectangle(d->display, d->pixmap, d->gc, 0, 0, ww, wh);
    XSetWindowBackgroundPixmap(d->display, d->icon_window, d->pixmap);
    XSetWindowBackgroundPixmap(d->display, d->window, d->pixmap);
    XClearWindow(d->display, d->icon_window);

    if (!d->iswindowed) {
	XMapRaised(d->display, d->window);
    } else {
	XMapRaised(d->display, d->icon_window);
    }

    XFlush(d->display);

    return 0;
}


void
dockapp_set_eventmask(const DOCKAPP *d, long mask)
{
    XSelectInput(d->display, d->icon_window, mask);
    XSelectInput(d->display, d->window, mask);
}


void
dockapp_set_shape(const DOCKAPP *d, Pixmap mask)
{
    XShapeCombineMask(d->display, d->icon_window, ShapeBounding,
		      d->x_offset, d->y_offset, mask, ShapeSet);
    XShapeCombineMask(d->display, d->window, ShapeBounding,
		      d->x_offset, d->y_offset, mask, ShapeSet);
    XFlush(d->display);
}


void
dockapp_copy_area(const DOCKAPP *d,
		  Pixmap src,
		  int x_src, int y_src, int w, int h, int x_dst, int y_dst)
{
    XCopyArea(d->display, src, d->pixmap, d->gc, x_src, y_src, w, h, x_dst + d->x_offset, y_dst + d->y_offset);
}


void
dockapp_update(const DOCKAPP *d)
{
    XClearWindow(d->display, d->icon_window);
}


Bool
dockapp_xpm2pixmap(const DOCKAPP *d,
		   char **data, Pixmap *pixmap, Pixmap *mask,
		   XpmColorSymbol *colorSymbol, unsigned int nsymbols)
{
    XpmAttributes xpmAttr;
    xpmAttr.valuemask = XpmCloseness;
    xpmAttr.closeness = 1 << 15;

    if (nsymbols) {
	xpmAttr.colorsymbols = colorSymbol;
	xpmAttr.numsymbols = nsymbols;
	xpmAttr.valuemask |= XpmColorSymbols;
    }

    return (XpmCreatePixmapFromData(d->display, d->icon_window, data, pixmap, mask, &xpmAttr) == 0);
}


Pixmap
dockapp_createpixmap(const DOCKAPP *d, int width, int height)
{
    return XCreatePixmap(d->display, d->icon_window, width, height, d->depth);
}


Bool
dockapp_nextevent_or_timeout(DOCKAPP *d,
			     XEvent *event, unsigned long millis)
{
    struct timeval timeout;
    fd_set rset;

    XSync(d->display, False);
    if (XPending(d->display)) {
	XNextEvent(d->display, event);
	return True;
    }

    timeout.tv_sec = millis / 1000;
    timeout.tv_usec = (millis % 1000) * 1000;

    FD_ZERO(&rset);
    FD_SET(ConnectionNumber(d->display), &rset);
    if (select(ConnectionNumber(d->display) + 1, &rset, NULL, NULL, &timeout) > 0) {
	XNextEvent(d->display, event);
	if (event->type == ClientMessage) {
	    if ((Atom)event->xclient.data.l[0] == d->delete_win) {
		d->quit = True;
		return False;
	    }
	}
	if (d->iswindowed) {
	    event->xbutton.x -= d->x_offset;
	    event->xbutton.y -= d->y_offset;
	}
	return True;
    }

    return False;
}


unsigned long
dockapp_get_color(const DOCKAPP *d, const char *color_name)
{
    XColor color;

    if (!XParseColor(d->display, DefaultColormap(d->display, DefaultScreen(d->display)), color_name, &color)) {
	return BlackPixel(d->display, DefaultScreen(d->display));
    }

    if (!XAllocColor(d->display, DefaultColormap(d->display, DefaultScreen(d->display)), &color)) {
	return BlackPixel(d->display, DefaultScreen(d->display));
    }

    return color.pixel;
}
