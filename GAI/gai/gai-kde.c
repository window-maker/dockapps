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

/* Hardly completed - Just testing at the moment. */

void gai_kde_window(void)
{
    int x,y,w,h;
    GAI_ENTER;

    GAI.widget = gtk_plug_new(GAI.parent_window);

    /* At this point, we have no idea how large the panel is */
    gtk_widget_set_size_request(GAI.widget, 42, 42); //GAI.width, GAI.height);

    /* FIXME: Vert & horz */

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

    gtk_widget_realize(GAI.drawingarea);

    GAI.window = GAI.drawingarea->window;


    if(GAI.gc != NULL)
	g_object_unref(GAI.gc);
    GAI.gc = gdk_gc_new(GAI.window);

    if(GAI.use_default_background)
	gai_load_background(); /* Just to be sure */
    else
	gai_draw_update_bg();

    gdk_window_get_geometry(GAI.widget->window, &x, &y, &w, &h,NULL);
    GAI_D("curr size: %d %d, location %d %d (%d %d)\n",
	  GAI.widget->allocation.width, GAI.widget->allocation.height,
	  x,y,w,h
	  );

    GAI_LEAVE;

}
