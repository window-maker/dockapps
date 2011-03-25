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

 * This code is based on libdockapp-0.4.0
 * modified by Seiichi SATO <ssato@sh.rim.or.jp>
 */

#include "dockapp.h"

#define PANEL_SIZE_W 64
#define PANEL_SIZE_H 64

/* global */
Display		*display = NULL;
Bool		dockapp_isbrokenwm = False;
dockapp_status	dockapp_stat;
Atom		delete_win;

/* private */
static Window	window = None;
static Window	icon_window = None;
static GC	gc = NULL;
static int	depth = 0;
static int	width, height;
static int	offset_w, offset_h;

void
dockapp_open_window(char *display_specified, char *appname,
		    unsigned w, unsigned h, int argc, char **argv)
{
    XClassHint	    *classhint;
    XWMHints	    *wmhints;
    Status	    stat;
    XTextProperty   title;
    XSizeHints	    sizehints;
    Window	    root;
    int		    ww, wh;

    /* Open Connection to X Server */
    display = XOpenDisplay(display_specified);
    if (!display) {
	fprintf(stderr, "%s: can't open display %s!\n",
		argv[0],
		XDisplayName(display_specified));
	exit(1);
    }
    root = DefaultRootWindow(display);

    /*
     * ww, wh: window size
     * width, height: drawble area size
     */
    width = w;
    height = h;
    if (dockapp_stat == WINDOWED_WITH_PANEL) {
	ww = PANEL_SIZE_W;
	wh = PANEL_SIZE_H;
    } else {
	ww = w;
	wh = h;
    }
    offset_w = (ww - w) / 2;
    offset_h = (wh - h) / 2;

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
    XSetClassHint(display, icon_window, classhint);
    XFree(classhint);

    /* Set WMHints */
    wmhints = XAllocWMHints();
    if (wmhints == NULL) {
	fprintf(stderr, "%s: can't allocate memory for wm hints!\n", argv[0]);
	exit(1);
    }
    wmhints->flags = IconWindowHint | WindowGroupHint;
    if (dockapp_stat == DOCKABLE_ICON) {
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
    switch(dockapp_stat) {
	case DOCKABLE_ICON:
	    sizehints.flags |= USPosition;
	    sizehints.x = sizehints.y = 0;
	    break;
	case WINDOWED:
	case WINDOWED_WITH_PANEL:
	    sizehints.flags |= PMinSize | PMaxSize;
	    sizehints.min_width = sizehints.max_width = ww;
	    sizehints.min_height = sizehints.max_height = wh;
	    break;
    }
    sizehints.width = ww;
    sizehints.height = wh;
    XSetWMNormalHints(display, icon_window, &sizehints);

    /* Set WindowTitle for AfterStep Wharf */
    stat = XStringListToTextProperty(&appname, 1, &title);
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
create_bgpanel_pixmap(void)
{
    Pixmap bg;

    bg = XCreatePixmap(display, icon_window, PANEL_SIZE_W, PANEL_SIZE_H,
		       depth);
    XSetForeground(display, gc, dockapp_getcolor_pixel("rgb:ae/aa/ae"));
    XFillRectangle(display, bg, gc, 0, 0, PANEL_SIZE_W, PANEL_SIZE_H);
    XSetForeground(display, gc, dockapp_getcolor_pixel("rgb:ff/ff/ff"));
    XDrawLine(display, bg, gc, 0, 0, 0, 63);
    XDrawLine(display, bg, gc, 1, 0, 1, 62);
    XDrawLine(display, bg, gc, 2, 0, 63, 0);
    XDrawLine(display, bg, gc, 2, 1, 62, 1);
    XSetForeground(display, gc, dockapp_getcolor_pixel("rgb:52/55/52"));
    XDrawLine(display, bg, gc, 1, 63, 63, 63);
    XDrawLine(display, bg, gc, 2, 62, 63, 62);
    XDrawLine(display, bg, gc, 63, 1, 63, 61);
    XDrawLine(display, bg, gc, 62, 2, 62, 61);

    return bg;
}


void
dockapp_set_background(Pixmap pixmap)
{
    if (dockapp_stat == WINDOWED_WITH_PANEL) {
	Pixmap bg;
	bg = create_bgpanel_pixmap();
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
    if (dockapp_stat == DOCKABLE_ICON)
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

    if (XpmCreatePixmapFromData(display, icon_window, data, pixmap, mask,
				&xpmAttr) != 0) {
	return False;
    }

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
		XDestroyWindow(display, event->xclient.window);
		XCloseDisplay(display);
		exit(0);
	    }
	}
	event->xbutton.x -= offset_w;
	event->xbutton.y -= offset_h;
	return True;
    }

    return False;
}

static long
get_closest_color_pixel(unsigned long red, unsigned long green, unsigned long blue)
{
    XColor *all_colors;
    XColor closest_color;

    int closest_index = -1;
    unsigned long min_diff = 0xffffffff;
    unsigned long diff;
    unsigned long diff_r = 0, diff_g = 0, diff_b = 0;

    int ncells = DisplayCells(display, DefaultScreen(display));
    int i;

    if ((all_colors = malloc(ncells * sizeof(XColor))) == NULL) {
	perror("malloc");
	return(1);
    }

    /* get all colors from default colormap */
    for (i = 0; i < ncells; i++) {
	all_colors[i].pixel = i;
    }
    XQueryColors( display , DefaultColormap(display, DefaultScreen(display)),
		  all_colors, ncells) ;

    /* find the closest color */
    for (i = 0; i < ncells; i++) {
	diff_r = (red - all_colors[i].red) >> 8 ;
	diff_g = (green - all_colors[i].green) >> 8 ;
	diff_r = (blue - all_colors[i].blue) >> 8 ;

	diff = diff_r * diff_r + diff_g * diff_g + diff_b * diff_b ;

	if ( diff < min_diff) /* closest ? */
	{
	    min_diff = diff ;
	    closest_index = i ;
	}
    }

    if (closest_index == -1) { /* unable to find closest color */
	fprintf(stderr, "can't allocate color #%lu/%lu/%lu, Using black\n",
		red, green, blue);
	free(all_colors);
	return BlackPixel(display, DefaultScreen(display));
    }

    closest_color.red = all_colors[closest_index].red;
    closest_color.green = all_colors[closest_index].green;
    closest_color.blue = all_colors[closest_index].blue;
    closest_color.flags = DoRed | DoGreen | DoBlue;

    free(all_colors);

    if (!XAllocColor(display, DefaultColormap(display, DefaultScreen(display)),
		     &closest_color)) {
	return BlackPixel(display, DefaultScreen(display));
    }

    return closest_color.pixel;
}

unsigned long
dockapp_getcolor_pixel(char *color_name)
{
    Visual *visual = DefaultVisual( display , DefaultScreen(display)) ;
    XColor screen_color;
    XColor exact_color;

    if (!XAllocNamedColor(display,
			  DefaultColormap(display, DefaultScreen(display)),
			  color_name, &screen_color, &exact_color)) {
	if (visual->class == PseudoColor || visual->class == GrayScale) {
	    return get_closest_color_pixel(screen_color.red,
					   screen_color.green,
					   screen_color.blue);
	} else {
	    fprintf(stderr, "can't allocate color %s. Using black\n",
		    color_name);
	    return BlackPixel(display, DefaultScreen(display));
	}
    }

    return exact_color.pixel;
}


unsigned long
dockapp_blendedcolor(char *color_name, int r, int g, int b, float fac)
{
    Visual *visual = DefaultVisual( display , DefaultScreen(display)) ;
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

    if (visual->class == PseudoColor || visual->class == GrayScale) {
	return get_closest_color_pixel(color.red, color.green, color.blue);
    }

    if (!XAllocColor(display, DefaultColormap(display, DefaultScreen(display)),
		     &color)) {
	return BlackPixel(display, DefaultScreen(display));
    }

    return color.pixel;
}
