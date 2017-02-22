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

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include <stdio.h>
#include <stdlib.h>

extern Display *display;
extern Bool dockapp_iswindowed;
extern Bool dockapp_isbrokenwm;


void dockapp_open_window(char *display_specified, char *appname,
			 unsigned w, unsigned h, int argc, char **argv);
void dockapp_set_eventmask();
void dockapp_set_background(Pixmap pixmap);
void dockapp_show(void);
Bool dockapp_xpm2pixmap(char **data, Pixmap * pixmap, Pixmap * mask,
			XpmColorSymbol * colorSymbol,
			unsigned int nsymbols);
Pixmap dockapp_XCreatePixmap(int w, int h);
void dockapp_setshape(Pixmap mask, int x_ofs, int y_ofs);
void dockapp_copyarea(Pixmap src, Pixmap dist, int x_src, int y_src,
		      int w, int h, int x_dist, int y_dist);
void dockapp_copy2window(Pixmap src);
Bool dockapp_nextevent_or_timeout(XEvent * event, unsigned long miliseconds);
unsigned long dockapp_getcolor(char *color);
unsigned long dockapp_blendedcolor(char *color, int r, int g, int b, float fac);
