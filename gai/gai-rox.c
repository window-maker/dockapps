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
 *             Dedicated to Evelyn Reimann. - Min ss sv gp af!!
 */
#include "../config.h"
#include "gai.h"
#include "gai-private.h"
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>

#define GAI_ROX_MAGIC 15

static int rox_expose(GtkWidget *widget, gpointer d)
{
    int h, w, l;

    if(GAI.lock)
	return TRUE;


    GAI_D("Expose! %d %d\n",
	  GAI.widget->allocation.width,
	  GAI.widget->allocation.height);



    if(GAI.widget->allocation.width > GAI.widget->allocation.height)
	l = GAI.widget->allocation.height;
    else
	l = GAI.widget->allocation.width;

    if(GAI.orient == GAI_VERTICAL){
	printf("Vertical\n");
	w = GAI.widget->allocation.width;
	h = GAI.default_height*w/GAI.default_width;

    } else {
	h = GAI.widget->allocation.height;

	if(GAI.max_size >0 && h >GAI.max_size)
	    h = GAI.max_size;

	w = GAI.default_width*h/GAI.default_height;
	printf("Horizontal\n");
    }

    

    printf("GAI.max_size: %d, w=%d, h=%d, wo=%d, ho=%d\n", GAI.max_size,w,h,
	   GAI.widget->allocation.width,
	   GAI.widget->allocation.height);



    /* Calculate the size of the applet, depending on rotate rules, panel size and orientation */
    gai_size_change(0, w, h, FALSE, 0);
    

    return TRUE;
}

void gai_rox_window(void)
{
    char *data, *tmp;
    int real_format, i;
    unsigned long items_read, items_left;

    Display *display;
    Atom rox_panel_menu_pos, real_type;

    GAI_ENTER;

    display = XOpenDisplay(NULL);
    rox_panel_menu_pos = XInternAtom(display, "_ROX_PANEL_MENU_POS", False);
    

    if (XGetWindowProperty(display, 
			   (Window)GAI.parent_window,
			   rox_panel_menu_pos,
			   0L, 100L, False, /* Request so much data so all will be delivered once */
			   XA_STRING,
			   &real_type, 
			   &real_format, 
			   &items_read,
			   &items_left, 
			   (unsigned char **) &data) 
	!= Success || items_left != 0)
	GAI_D("XGet failed (%d)!\n", (int)items_left);


    
    for(i=0; data[i] != '\0'; i++)
	if(data[i]==',') break;

    /*
    if(data[i] != '\0')
	size = atoi(data+i+1);
    else
	size = 32;


    GAI_D("%s %d, size:%d\n", data, i, size);
    */


    /* ROX panel delivers: "Bottom,32", "Left,8", "Right,8", or "Top,8" */

    if(!strncmp(data,"Bottom",6) || !strncmp(data,"Top",3))
	GAI.orient = GAI_HORIZONTAL;
    else
	GAI.orient = GAI_VERTICAL;

    free(data);



    GAI.widget = gtk_plug_new(GAI.parent_window);

    //    gtk_widget_set_size_request(GAI.widget, GAI.width, GAI.height);


    gtk_widget_set_events(GAI.widget, GAI.mask);

    GAI.drawingarea = gtk_drawing_area_new();

#ifdef GAI_WITH_GL
    if(GAI.open_gl){
	gtk_widget_set_gl_capability(GAI.drawingarea,
				     GAI.glconfig,
				     NULL, TRUE,
				     GDK_GL_RGBA_TYPE);
    }
#endif

    gtk_container_add(GTK_CONTAINER(GAI.widget), GAI.drawingarea);

    gtk_window_set_wmclass(GTK_WINDOW(GAI.widget),
			   GAI.applet.name, GAI.applet.name);

    gtk_widget_set_events(GAI.drawingarea, GAI.mask);

    gtk_widget_realize(GAI.drawingarea);

    gtk_widget_realize(GAI.widget);

    GAI.window = GAI.drawingarea->window;


    /*    g_signal_connect(G_OBJECT(GAI.widget), "expose_event",
		      G_CALLBACK(rox_expose), NULL);
    */

    if(GAI.gc != NULL)
	g_object_unref(GAI.gc);
    GAI.gc = gdk_gc_new(GAI.window);


    tmp = g_strdup_printf("%s/rox_panel_size", GAI.applet.name);
    gai_size_change(gai_load_int_with_default(tmp, 64), 0, 0, TRUE, 0);
    g_free(tmp);


    if(GAI.use_default_background)
	gai_load_background(); /* Just to be sure */
    else
	gai_draw_update_bg();



    GAI_LEAVE;


}
