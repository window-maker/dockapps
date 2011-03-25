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


#ifndef DOCKAPP_H_included
#define DOCKAPP_H_included

typedef struct {
    Display *display;
    Bool iswindowed;
    Pixmap pixmap;
    Bool quit;

    Window window;
    Window icon_window;
    GC gc;
    int depth;
    Atom delete_win;
    int width, height;
    int x_offset, y_offset;
} DOCKAPP;

int dockapp_open_window(DOCKAPP *d,
			const char *display_specified,
			char *appname, int iswindowed,
			int argc, char **argv);

void dockapp_set_eventmask(const DOCKAPP *d, long mask);

void dockapp_set_shape(const DOCKAPP *d, Pixmap mask);

void dockapp_copy_area(const DOCKAPP *d,
		      Pixmap src,
		      int x_src, int y_src, int w, int h, int x_dst, int y_dst);

void dockapp_update(const DOCKAPP *d);

Bool dockapp_xpm2pixmap(const DOCKAPP *d,
			char **data, Pixmap *pixmap, Pixmap *mask,
			XpmColorSymbol *colorSymbol, unsigned int nsymbols);

Pixmap dockapp_createpixmap(const DOCKAPP *d, int width, int height);

Bool dockapp_nextevent_or_timeout(DOCKAPP *d,
				  XEvent *event, unsigned long millis);

unsigned long dockapp_get_color(const DOCKAPP *d, const char *color);

#endif
