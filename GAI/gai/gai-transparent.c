/*
 * GAI - General Applet Interface Library
 * Copyright (C) 2003-2004 Jonas Aaberg <cja@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 *             Dedicated to Evelyn Reimann - Min ss sv gp af!!
 */

#include "../config.h"
#include "gai.h"
#include "gai-private.h"

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <stdlib.h>

static GdkPixbuf *behind_window(int x, int y, int w, int h)
{
    GdkPixmap *pixmap;
    Pixmap *prop_data = NULL;
    GdkPixbuf *pixbuf;
    GdkAtom prop_type;

    gdk_property_get(GAI.root_window,
		     gdk_atom_intern("_XROOTPMAP_ID", FALSE),
		     0, 0, 10, FALSE, &prop_type, NULL, NULL,
		     (unsigned char **) &prop_data);

    if ((prop_type == GDK_TARGET_PIXMAP) && prop_data && prop_data [0])
	pixmap = gdk_pixmap_foreign_new (prop_data [0]);
    else 
	return NULL;

    g_free(prop_data);

    pixbuf = gdk_pixbuf_get_from_drawable(NULL, pixmap, gdk_colormap_get_system(),
					  x, y, 0, 0, w, h);
    g_object_unref(pixmap);

    return pixbuf;

}

static void update_bg(int x, int y, int w, int h, int sx, int sy)
{
    //    GdkPixmap *pixmap = NULL;
    //    GdkBitmap *mask = NULL;
    GdkPixbuf *pixbuf;

    pixbuf = behind_window(x, y, w, h); 

    if(gdk_pixbuf_get_height(GAI.behind_applet) != GAI.height || 
       gdk_pixbuf_get_width(GAI.behind_applet) != GAI.width){
	g_object_unref(GAI.behind_applet);
	GAI.behind_applet = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, GAI.width, GAI.height);
    }

    gdk_pixbuf_copy_area(pixbuf,0,0,w,h, GAI.behind_applet, sx, sy);
    gai_draw_update_bg();

    /*
    gdk_pixbuf_render_pixmap_and_mask(background, &pixmap, &mask, 0x80);

    gtk_widget_shape_combine_mask(widget, mask, 0, 0);

    gdk_window_set_back_pixmap(widget->window, pixmap, FALSE);
    gdk_flush();

    gtk_widget_queue_draw_area(widget, sx, sy, w, h);

    gdk_window_process_all_updates();

    g_object_unref(pixmap);
    if(mask !=NULL)
    g_object_unref(mask);*/

    g_object_unref(pixbuf);
}


gboolean gai_root_window_config(GtkWidget *widget, GdkEventConfigure *e, gpointer d)
{
    static int old_x = -1, old_y = -1, old_w = -1, old_h = -1;
    int scr_w, scr_h, x, y, w, h, sx = 0, sy = 0;
    /* sx and sy is used to point out where in the window updating bg starts */

    w = e->width;
    h = e->height;
    x = e->x;
    y = e->y;

    /* Already updated, quit */
    if(old_x == x && old_y == y && old_w == w && old_h == h && d == NULL)
	return TRUE;
    old_x = x;
    old_y = y;
    old_w = w;
    old_h = h;

    scr_w = gdk_screen_width();
    scr_h = gdk_screen_height();


    /* Check if window beyond our reach to the left, and up */
    if(x+w <= 0 || y+h <= 0)
	return TRUE;

    /* Check if window beyond our reach to the right and bottom */
    if(x >= scr_w || y >= scr_h)
	return TRUE;

    /* Check where the window starts - left */
    if(x < 0){
	w += x; /* plus since it is already negative */
	sx = abs(x);
    }

    /* right */
    if(x+w > scr_w)
	w -= (x+w) - scr_w;

    /* upper */
    if(y < 0){
	h += y;
	sy = abs(y);
    }

    /* lower */
    if(y+h > scr_h)
	h -= (y+h) - scr_h;

    /* Now we have correct w and h. and sx and sy */
    
    x += sx;
    y += sy;
    

    printf("x=%d y=%d w=%d h=%d sx=%d sy=%d\n", x, y, w, h, sx, sy); 

    update_bg(x, y, w, h, sx, sy);
    return TRUE;
}

/* This piece of code was taken from Multi-gnome-terminal-1.6.1 - Thanks GPL :-) */
GdkFilterReturn gai_root_window_event(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer d)
{
    XEvent *xevent = (XEvent *) gdk_xevent;
    GdkEventConfigure e;

    if(xevent->type == PropertyNotify && 
       xevent->xproperty.atom == gdk_x11_atom_to_xatom(gdk_atom_intern("_XROOTPMAP_ID", TRUE))){
	gdk_window_get_geometry(GAI.widget->window, NULL, NULL, &e.width, &e.height, NULL);
	gdk_window_get_position(GAI.widget->window, &e.x, &e.y);
	/*	printf("root_win, x=%d y=%d w=%d h=%d\n", e.x, e.y, e.width, e.height); */
	gai_root_window_config(NULL, &e, (gpointer) -1);
    }

    return GDK_FILTER_CONTINUE;
}


