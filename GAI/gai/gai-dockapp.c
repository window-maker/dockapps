/*
 * GAI - General Applet Interface Library
 * Copyright (C) 2003-2005 Jonas Aaberg <cja@gmx.net>
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

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <unistd.h>

#ifdef GAI_WITH_GL
#include <GL/gl.h>
#include <GL/glu.h>
#endif


static Window gai_create_base_xwin(int map)
{
    Window main_win;
    XSizeHints sizehints;
    XWMHints wmhints;
    XSetWindowAttributes  windowattributes;
    XClassHint classhints;
    Atom wmprotocols, dock_atom;

    windowattributes.event_mask = ExposureMask | KeyPressMask | 
	KeyReleaseMask | ButtonPressMask | PointerMotionMask |
	ButtonReleaseMask | PropertyChangeMask |
	FocusChangeMask | ButtonMotionMask |
	EnterWindowMask | LeaveWindowMask;

    main_win = XCreateWindow(GDK_DISPLAY(),
			     RootWindow(GDK_DISPLAY(), DefaultScreen(GDK_DISPLAY())),
			     0,0,
			     GAI.width, GAI.height,
			     0,
			     DefaultDepth(GDK_DISPLAY(), DefaultScreen(GDK_DISPLAY())),
			     InputOutput,
			     DefaultVisual(GDK_DISPLAY(), DefaultScreen(GDK_DISPLAY())),
			     CWEventMask,
			     &windowattributes);
    XStoreName(GDK_DISPLAY(), main_win, GAI.applet.name);
    XSetIconName(GDK_DISPLAY(), main_win, GAI.applet.name);


    sizehints.flags = USSize;
    sizehints.width = GAI.default_width;
    sizehints.height = GAI.default_height;
    
    XSetWMNormalHints(GDK_DISPLAY(), main_win, &sizehints);

    
    wmhints.icon_window = main_win;
    wmhints.initial_state = WithdrawnState;
    wmhints.icon_x = 0;
    wmhints.icon_y = 0;
    wmhints.input = True;
    wmhints.window_group = main_win;
    wmhints.flags = 
	StateHint | IconWindowHint | IconPositionHint | WindowGroupHint | InputHint;
    XSetWMHints(GDK_DISPLAY(), main_win, &wmhints);

    classhints.res_name = GAI.applet.name;
    classhints.res_class = GAI.applet.name;
    XSetClassHint(GDK_DISPLAY(), main_win, &classhints);
    
    wmprotocols = XInternAtom(GDK_DISPLAY(), "WM_DELETE_WINDOW, WM_TAKE_FOCUS, _NET_WM_PING", False);
	

    XSetWMProtocols(GDK_DISPLAY(), main_win, &wmprotocols, 1);



    /* Setting up freedesktop settings */

    XChangeProperty(GDK_DISPLAY(), main_win,
		    gdk_x11_get_xatom_by_name_for_display(gdk_display_get_default(), "_NET_WM_NAME"),
		    gdk_x11_get_xatom_by_name_for_display(gdk_display_get_default(), "UTF8_STRING"),
		    8, PropModeReplace, 
		    GAI.applet.name, strlen(GAI.applet.name));
    XChangeProperty(GDK_DISPLAY(), main_win,
		    gdk_x11_get_xatom_by_name_for_display(gdk_display_get_default(), "_NET_WM_ICON_NAME"),
		    gdk_x11_get_xatom_by_name_for_display(gdk_display_get_default(), "UTF8_STRING"),
		    8, PropModeReplace, 
		    GAI.applet.name, strlen(GAI.applet.name));



    dock_atom = gdk_x11_get_xatom_by_name_for_display(gdk_display_get_default(), "_NET_WM_WINDOW_TYPE_DOCK");

    XChangeProperty(GDK_DISPLAY(), main_win,
		    gdk_x11_get_xatom_by_name_for_display(gdk_display_get_default(), "_NET_WM_WINDOW_TYPE"),
		    XA_ATOM, 32, PropModeReplace, 
		    (unsigned char *)&dock_atom, 1);

    if(map)
	XMapWindow(GDK_DISPLAY(), main_win);

    return main_win;
}


static void 
gai_dockapp_window(void)
{
    XSizeHints sizehints;
    XWMHints wmhints;
    Window xwin;

    GAI_ENTER;

    GAI.about = NULL;

    GAI.width = gai_scale(GAI.default_width);
    GAI.height = gai_scale(GAI.default_height);

    if(GAI.parent_window == -1 && !GAI.broken_wm && !GAI.panel)
	GAI.parent_window = (int)gai_create_base_xwin(TRUE);
    			    

    GAI.widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    
    gtk_window_set_has_frame(GTK_WINDOW(GAI.widget), FALSE);
    
    gtk_window_set_wmclass(GTK_WINDOW(GAI.widget),
			   GAI.applet.name, GAI.applet.name);
    

    gtk_widget_set_events(GAI.widget,GAI.mask);
    
    gtk_window_set_default_size(GTK_WINDOW(GAI.widget),
				GAI.width, GAI.height);

    if(GAI.panel){
	gtk_window_stick(GTK_WINDOW(GAI.widget));
	gtk_window_set_decorated(GTK_WINDOW(GAI.widget), FALSE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(GAI.widget), TRUE);
    }
    

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


    gtk_widget_realize(GAI.widget);

    if(GAI.panel)
	gdk_window_set_type_hint(GAI.widget->window, GDK_WINDOW_TYPE_HINT_TOOLBAR);

    if(GAI.parent_window != -1){
	GAI.icon_window = gdk_window_foreign_new((Window)GAI.parent_window);
	gdk_window_reparent(GAI.widget->window, GAI.icon_window,0,0);
	gdk_window_show(GAI.icon_window);
    }


    if(GAI.gc != NULL)
	g_object_unref(GAI.gc);
    GAI.gc = gdk_gc_new(GAI.window);


    
    if(GAI.use_default_background)
	gai_load_background(); /* Just to be sure */
    else
	gai_draw_update_bg();


    if(!GAI.broken_wm && !GAI.panel){
	/* Does not work with GTK 2.0. */
	/* gdk_window_set_type_hint(GAI.widget->window, GDK_WINDOW_TYPE_HINT_DOCK); */

	xwin = GDK_WINDOW_XWINDOW(GAI.widget->window);
	wmhints.icon_window = xwin;

	sizehints.flags = USSize;
	sizehints.width = GAI.width;
	sizehints.height = GAI.height;

	XSetWMNormalHints(GDK_DISPLAY(), xwin, &sizehints);

	wmhints.initial_state = WithdrawnState;
	wmhints.icon_x = 0;
	wmhints.icon_y = 0;
	wmhints.window_group = xwin;
	wmhints.input = True;
	wmhints.flags = 
	    StateHint | IconWindowHint | IconPositionHint | WindowGroupHint | InputHint;
	XSetWMHints(GDK_WINDOW_XDISPLAY(GAI.widget->window), xwin, &wmhints);

    }
    GAI_LEAVE;

}

static void
gai_dockapp_window_windowmaker(void)
{
    XSizeHints sizehints;
    XWMHints wmhints;
    Window iconwin, main_win;
    GdkGeometry geo;

    GAI_ENTER;

    GAI.about = NULL;
    
    GAI.width = gai_scale(GAI.default_width);
    GAI.height = gai_scale(GAI.default_height);
					
    main_win = gai_create_base_xwin(FALSE);

    GAI.widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events(GAI.widget, GAI.mask);
    gtk_widget_realize(GAI.widget);
    gdk_window_reparent(GAI.widget->window,gdk_window_foreign_new(main_win), 0, 0); 

    iconwin = GDK_WINDOW_XWINDOW(GAI.widget->window);


    sizehints.flags = USSize;
    sizehints.width = GAI.width;
    sizehints.height = GAI.height;

    XSetWMNormalHints(GDK_DISPLAY(), main_win, &sizehints);
    
    GAI.drawingarea = gtk_drawing_area_new();


#ifdef GAI_WITH_GL
    if(GAI.open_gl)
	gai_display_error_quit(_("You're trying to run an applet that uses OpenGL.\n"
			       "gtkglext doesn't work together with WindowMaker\n"
			       "special way of docking applets.\n"
			       "I'm sorry.\n"));
    /*
    if(GAI.open_gl){
	gtk_widget_set_gl_capability(GAI.drawingarea,
				     GAI.glconfig,
				     NULL, TRUE,
				     GDK_GL_RGBA_TYPE);
    }
    */

#endif

    wmhints.icon_window = iconwin;
    wmhints.initial_state = WithdrawnState;
    wmhints.icon_x = 0;
    wmhints.icon_y = 0;
    wmhints.window_group = main_win;
    wmhints.flags = 
	StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
    XSetWMHints(GDK_DISPLAY(), main_win, &wmhints);

    XMapWindow(GDK_DISPLAY(), main_win);

    gdk_window_set_title(GAI.widget->window, GAI.applet.name);
    geo.min_width = geo.max_width = GAI.width;
    geo.min_height = geo.max_height = GAI.height;

    gdk_window_set_geometry_hints(GAI.widget->window, &geo,
				  GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);

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

    gdk_window_set_type_hint(GAI.widget->window, GDK_WINDOW_TYPE_HINT_DOCK);

    GAI_LEAVE;
}


int gai_timer_joystick(gpointer d)
{
#ifdef GAI_WITH_JOYSTICK
	if(GAI.on_joystick_callback && GAI.applet_focused){
	    GAI.on_joystick_callback(GAI.jflags, GAI.on_joystick_userdata);
	    GAI.jflags = 0;
	}
#endif
    return TRUE;
}

int 
gai_timer(gpointer d)
{
#ifdef GAI_WITH_GL
    GdkGLContext *glcontext;
    GdkGLDrawable *gldrawable=NULL;
#endif

    GAI_ENTER;
    if(!GAI.timer_started)
	gai_display_queued_errors();

    GAI.timer_started = 1;
    if(! GAI.restarting)
    {
#ifdef GAI_WITH_GL

	if(GAI.open_gl){
	    glcontext = gtk_widget_get_gl_context(GAI.drawingarea);
	    gldrawable = gtk_widget_get_gl_drawable(GAI.drawingarea);

	    if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return TRUE;
	}
#endif
	if(GAI.on_update_callback)
	    GAI.on_update_callback(GAI.on_update_userdata);

#ifdef GAI_WITH_GL
	if(GAI.open_gl){
	    if(gdk_gl_drawable_is_double_buffered(gldrawable))
		gdk_gl_drawable_swap_buffers(gldrawable);
	    else
		glFlush();

	    gdk_gl_drawable_gl_end(gldrawable);
	}
#endif


    }

    GAI_LEAVE;
    return TRUE;
}

static void gai_create_menu(void)
{
    GtkWidget *menu_line=NULL;
    GtkAccelGroup *accel_group;
    GdkPixbuf *pixbuf;
    MenuHashItem *hash_item;
    gint i;

    GAI_ENTER;

    if(!GAI.menu_changed)
	return;

    GAI.menu_changed = FALSE;

    accel_group = gtk_accel_group_new();

    if(GAI.menu != NULL){
	gtk_widget_destroy(GAI.menu);
	//g_object_unref(GAI.menu);
    }

    GAI.menu = gtk_menu_new();

    for(i=0;i<g_slist_length(GAI.menu_list);i++){

	hash_item = (MenuHashItem *)g_hash_table_lookup(GAI.menu_hash, g_slist_nth_data(GAI.menu_list, i));

	if(hash_item->type == GAI_MENU_STOCK || hash_item->type == GAI_MENU_NONE || hash_item->type == GAI_MENU_FILE){

	    menu_line = gtk_image_menu_item_new_with_mnemonic(hash_item->name);


	    if(hash_item->icon != NULL){

		if(hash_item->type == GAI_MENU_STOCK){

		    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_line), 
						   gtk_image_new_from_stock(hash_item->icon,
									     GTK_ICON_SIZE_MENU));
		}

		if(hash_item->type == GAI_MENU_FILE) {
		    pixbuf = gai_load_image(hash_item->icon);

		    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menu_line), 
						   gtk_image_new_from_pixbuf(pixbuf));
		    
		    g_object_unref(pixbuf);
		}
	    }


	    if(hash_item->name != NULL)
		g_signal_connect(G_OBJECT(menu_line), "activate",
				 G_CALLBACK(hash_item->func),
				 hash_item->ptr);

	}
	if(hash_item->type == GAI_MENU_SEPARATOR){
	    menu_line = gtk_menu_item_new();
	    gtk_widget_set_sensitive(menu_line, FALSE);
	}

	gtk_container_add(GTK_CONTAINER(GAI.menu), menu_line);
    }
    
    gtk_menu_set_accel_group(GTK_MENU(GAI.menu), accel_group);

    gtk_widget_show_all(GAI.menu);

    GAI_LEAVE;
}


static int 
on_mouse_click_callback(GtkWidget *widget, GdkEventButton *e, gpointer d)
{
    int x, y;
    GAI_ENTER;

    if(e->button == 3 && e->type == GDK_BUTTON_PRESS)
    {
	if(GAI.applet_type == GAI_DOCKAPP || GAI.applet_type == GAI_ROX)
	{
	    gai_create_menu();
	    gtk_menu_popup(GTK_MENU(GAI.menu), NULL, NULL, NULL, NULL, 
			    e->button, e->time);
	    return TRUE;
	}
	GAI_LEAVE;
	return FALSE;
    }


    x=(int)(e->x/GAI.scale);
    y=(int)(e->y/GAI.scale);


    GAI_D("click:(%d, %d) scaled to(%d, %d)\n",(int)e->x, (int)e->y, x,y);

    if(e->button==1 && e->type == GDK_BUTTON_PRESS)
    {  
	if(GAI.on_mouse_click1_callback)
	    GAI.on_mouse_click1_callback(x,y, 
					 GAI.on_mouse_click1_userdata);
	GAI_LEAVE;  
	return TRUE;
    }

    if (e->button==2 && e->type == GDK_BUTTON_PRESS)
    {  
	if (GAI.on_mouse_click2_callback)
	    GAI.on_mouse_click2_callback(x,y, 
					 GAI.on_mouse_click2_userdata);
    }
    
    GAI_LEAVE;  
    /* TRUE = Not propagate */
    return FALSE;
}

static gboolean 
on_scroll_buttons_callback(GtkWidget *widget, GdkEventScroll *event, 
			    gpointer d)
{
    GAI_ENTER;
    if(GAI.on_scroll_buttons_callback)
		GAI.on_scroll_buttons_callback(event->direction,
					       GAI.on_scroll_buttons_userdata);
    GAI_LEAVE;
    return FALSE;
}

static gboolean 
on_enter_callback(GtkWidget *widget, gpointer d)
{
    GAI_ENTER;

    XSetInputFocus(GDK_WINDOW_XDISPLAY(GAI.window),
		   PointerRoot,
		   RevertToPointerRoot,
		   CurrentTime);

    GAI.applet_focused = 1;
    if(GAI.on_enter_callback)
	GAI.on_enter_callback(GAI.on_enter_userdata);

    GAI_LEAVE;
    return FALSE;
}

static gboolean 
on_leave_callback(GtkWidget *widget, gpointer d)
{
    GAI_ENTER;
    GAI.applet_focused = 0;

    if(GAI.on_leave_callback)
	GAI.on_leave_callback(GAI.on_leave_userdata);

    GAI_LEAVE;
    return FALSE;
}

static gboolean 
on_keypress_callback(GtkWidget *widget, GdkEventKey *ev, gpointer d)
{
    GAI_ENTER;
    if(GAI.on_keypress_callback)
	GAI.on_keypress_callback(ev->keyval, GAI.on_keypress_userdata);
    GAI_LEAVE;
    return FALSE;
}

static gboolean 
on_mouse_release_callback(GtkWidget *widget, GdkEventButton *e, gpointer d)
{
    int x,y;
    GAI_ENTER;

    if(e->button==3)
	return FALSE;

    /*
    if(GAI.rotate && GAI.orient == GAI_VERTICAL)
    {
	x=gai_scale((int)e->y);
	y=gai_scale((int)e->x);
    }else{
	x=gai_scale((int)e->x);
	y=gai_scale((int)e->y);
    }
*/
    x=(int)(e->x/GAI.scale);
    y=(int)(e->y/GAI.scale);


    if(e->button == 1)
    {  
	if(GAI.on_mouse_release1_callback)
	    GAI.on_mouse_release1_callback(x,y, 
					 GAI.on_mouse_release1_userdata);
	GAI_LEAVE;
	return FALSE;

    }

    if(e->button == 2)
    {  
	if(GAI.on_mouse_release2_callback)
	    GAI.on_mouse_release2_callback(x,y, 
					 GAI.on_mouse_release2_userdata);

    }
    GAI_LEAVE;
    return FALSE;

}

gboolean on_mouse_motion_callback(GtkWidget *w, GdkEventMotion *e, gpointer d)
{
    GAI_ENTER;
    if(GAI.on_mouse_move_callback)
	GAI.on_mouse_move_callback((int)(e->x/GAI.scale), 
				   (int)(e->y/GAI.scale), 
				   GAI.on_mouse_move_userdata);
    GAI_LEAVE;
    return FALSE;
}


int gai_dies(gpointer d)
{
    GAI_ENTER;

    if(GAI.timer)
	gtk_timeout_remove(GAI.timer);
#ifdef GAI_WITH_JOYSTICK
    if(GAI.timer_joystick)
	gtk_timeout_remove(GAI.timer_joystick);
#endif
    
    gtk_main_quit();
    GAI_LEAVE;

    return TRUE;
}

/* This little function looks for WindowMaker */
int gai_detect_window_maker(void)
{
    if(XInternAtom(GDK_DISPLAY(),
		   "_WINDOWMAKER_WM_PROTOCOLS", True) != None)
	return 1;
    else
	return 0;
}

/* Get called when the theme changes */
/* This function doesn't really work! - It seems like it get 
   called BEFORE the actual theme change is done. */
void gai_style_change(GtkWidget *a, GtkStyle *s, gpointer d)
{
    gai_draw_update_bg();
}

void gai_hook(void)
{
    XWindowAttributes attr;
    GAI_ENTER;

    g_signal_connect(G_OBJECT(GAI.widget), "button-press-event", 
		      G_CALLBACK(on_mouse_click_callback), NULL);

    g_signal_connect(G_OBJECT(GAI.widget), "delete-event",
		     G_CALLBACK(gai_dies), NULL);

    g_signal_connect(G_OBJECT(GAI.widget), "destroy",
		     G_CALLBACK(gai_dies), NULL);

    /* This signal gets called when the GTK theme get changed.*/
    g_signal_connect(G_OBJECT(GAI.widget),  "style-set",
		     G_CALLBACK(gai_style_change), NULL);

    if(GAI.on_mouse_release1_callback || 
	GAI.on_mouse_release2_callback)
	g_signal_connect(G_OBJECT(GAI.widget), "button-release-event", 
			  G_CALLBACK(on_mouse_release_callback), NULL);

    if(GAI.on_scroll_buttons_callback)
	g_signal_connect(G_OBJECT(GAI.widget), "scroll-event",
			  G_CALLBACK(on_scroll_buttons_callback), NULL);

    if(GAI.on_keypress_callback)
	g_signal_connect(G_OBJECT(GAI.widget), "key-press-event",
			  G_CALLBACK(on_keypress_callback), NULL);


    //    if(GAI.on_keypress_callback || GAI.on_leave_callback || GAI.on_enter_callback || GAI.on_joystick_callback){
	g_signal_connect(G_OBJECT(GAI.widget), "enter-notify-event",
			 G_CALLBACK(on_enter_callback), NULL);

	/* Shall "move_focus_out_of_applet" be used instead of this ?? */

	g_signal_connect(G_OBJECT(GAI.widget), "leave-notify-event",
			 G_CALLBACK(on_leave_callback), NULL);

	//}

    /* This is for transparent backgrounds */
    
    if((GAI.applet_type != GAI_GNOME1 && GAI.applet_type != GAI_GNOME2) && GAI.transparent_bg){

	GAI.root_window = gdk_screen_get_root_window(gdk_screen_get_default());
	XGetWindowAttributes(GDK_DISPLAY(), GDK_ROOT_WINDOW(), &attr);
	XSelectInput(GDK_DISPLAY(), GDK_ROOT_WINDOW(), attr.your_event_mask | PropertyChangeMask);

	gdk_window_add_filter(GDK_ROOT_PARENT(), gai_root_window_event, NULL);
	GAI.behind_applet = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, GAI.width, GAI.height);

	g_signal_connect(G_OBJECT(GAI.widget), "configure-event", G_CALLBACK(gai_root_window_config), NULL);
    }

    if(GAI.on_mouse_move_callback){
	g_signal_connect(G_OBJECT(GAI.widget), "motion-notify-event",
			  G_CALLBACK(on_mouse_motion_callback), NULL);
    }


    if(GAI.on_update_callback)
	GAI.timer = gtk_timeout_add(GAI.update_interval, gai_timer, NULL);
    else
	GAI.timer = 0;
#ifdef GAI_WITH_JOYSTICK
    if(GAI.on_joystick_callback)
	GAI.timer_joystick = gtk_timeout_add(GAI_JOYSTICK_UPDATE, gai_timer_joystick, NULL);
    else
	GAI.timer_joystick = 0;

#endif

    GAI_LEAVE;

}

static void 
gai_simple_about_ok(GtkWidget *widget, gpointer d)
{
    GAI_ENTER;
    gtk_widget_destroy(GAI.about);
    GAI.about = NULL;
    GAI_LEAVE;

} 

static void gai_simple_about(void)
{

    GtkWidget *vbox;
    GtkWidget *label, *separator, *about_ok_button;

    char *tmp;
    GdkPixbuf *pixbuf=NULL;

    GAI_ENTER;

    if(GAI.about != NULL){
	gtk_window_present(GTK_WINDOW(GAI.about));
	GAI_LEAVE;
	return;
    }

    if(GAI.applet.icon != NULL)
        pixbuf = gdk_pixbuf_new_from_file(GAI.applet.icon, NULL);


    GAI.about = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(GAI.about), 5);
    gtk_window_set_resizable(GTK_WINDOW(GAI.about), FALSE);

    tmp = g_strdup_printf("About - %s", GAI.applet.nice_name);
    gtk_window_set_title(GTK_WINDOW(GAI.about), tmp);
    g_free(tmp);


    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(GAI.about), vbox);

    if(pixbuf!=NULL){
	gtk_box_pack_start(GTK_BOX(vbox), gtk_image_new_from_pixbuf(pixbuf) , TRUE, TRUE, 5);
	g_object_unref(pixbuf);
    }

    


    label = gtk_label_new(NULL);
    tmp = g_strdup_printf("<span size=\"x-large\" font_desc=\"Arial\"><b>" 
		        "%s %s</b></span>",GAI.applet.nice_name, GAI.applet.version);
    gtk_label_set_markup(GTK_LABEL(label), tmp);
    g_free(tmp);
    gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_misc_set_padding(GTK_MISC(label), 5, 0);

    label = gtk_label_new(GAI.applet.description);
    gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_misc_set_padding(GTK_MISC(label), 5, 0);

    tmp = g_strdup_printf("Written by %s",GAI.applet.author);
    label = gtk_label_new(tmp);
    gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
    g_free(tmp);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_misc_set_padding(GTK_MISC(label), 5, 0);


    label = gtk_label_new(NULL);
    tmp = g_strdup_printf("<tt>%s</tt>",GAI.applet.license);
    gtk_label_set_markup(GTK_LABEL(label),tmp);
    gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
    g_free(tmp);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_misc_set_padding(GTK_MISC(label), 5, 0);

    separator = gtk_hseparator_new();
    gtk_widget_show(separator);
    gtk_box_pack_start(GTK_BOX(vbox), separator, TRUE, TRUE, 0);

    about_ok_button = gtk_button_new_from_stock("gtk-ok");

    gtk_box_pack_start(GTK_BOX(vbox), about_ok_button, FALSE, FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(about_ok_button), 5);
    
    g_signal_connect((gpointer)about_ok_button, "clicked",
		      G_CALLBACK(gai_simple_about_ok), NULL);

    g_signal_connect((gpointer)about_ok_button, "destroy",
		      G_CALLBACK(gai_simple_about_ok), NULL);

    gtk_widget_show_all(GAI.about);
    GAI_LEAVE;

}

void 
gai_menu_show_help_text(GtkWidget *widget, gpointer d)
{
    GtkWidget *w;

    GAI_ENTER;

    if(GAI.menu_help_text==NULL)
	return;

    w = gtk_message_dialog_new(
	NULL, 0, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, GAI.menu_help_text);
    g_signal_connect_swapped(G_OBJECT(w), "response",
			      G_CALLBACK(gtk_widget_destroy),
			      G_OBJECT(w));
    gtk_widget_show(w);
    GAI_LEAVE;
}


void 
gai_on_preferences_activate(GtkMenuItem *menuitem, gpointer d)
{
    GAI_ENTER;

    if(GAI.has_preferences == GAI_PREF_OWN)
    {
	if(GAI.on_preferences_callback)
	    GAI.on_preferences_callback(FALSE, GAI.on_preferences_userdata);
    }
    else if(GAI.has_preferences == GAI_PREF_GEN)
    {
	gai_make_preference_window(GAI.pref_name, GAI.gn);
    }
    else if(GAI.has_preferences == GAI_PREF_GEN2)
    {
	gai_make_preference_window2(GAI.pref_name, GAI.pref_instr);
    }
    GAI_ENTER;

}



void 
gai_on_help_activate(GtkMenuItem *menuitem, gpointer d)
{
    GAI_ENTER;
    gai_menu_show_help_text(NULL, NULL);
    GAI_LEAVE;
}


void 
gai_on_about_activate(GtkMenuItem *menuitem, gpointer d)
{
    GAI_ENTER;

#ifdef GAI_WITH_GNOME
    if(GAI.applet_type == GAI_GNOME1 ||
	GAI.applet_type == GAI_GNOME2)
	gai_gnome_about_show();
    else
#endif
	gai_simple_about();
    GAI_LEAVE;
}

void 
gai_on_remove_activate(GtkMenuItem *menuitem, gpointer d)
{
    GAI_ENTER;
    GAI.did_exit_function = 1;
    if(GAI.timer)
	gtk_timeout_remove(GAI.timer);
#ifdef GAI_WITH_JOYSTICK
    if(GAI.timer_joystick)
	gtk_timeout_remove(GAI.timer_joystick);
#endif

    if(GAI.on_exit_callback)
	GAI.on_exit_callback(GAI.on_exit_userdata);

    gtk_main_quit();
    GAI_LEAVE;
}





void gai_dockapp_main(void)
{
#ifdef GAI_WITH_GL
    GdkGLContext *glcontext;
    GdkGLDrawable *gldrawable;
#endif

    GAI_ENTER;

    switch(GAI.applet_type){
#ifdef GAI_WITH_ROX
    case GAI_ROX:
	gai_rox_window();
	break;
#endif
#ifdef GAI_WITH_KDE
    case GAI_KDE:
	gai_kde_window();
	break;
#endif
    default:
	if(GAI.window_maker && !GAI.panel)
	    gai_dockapp_window_windowmaker();
	else
	    gai_dockapp_window();
	break;
    }

    gai_hook();
    //    gai_create_menu();


#ifdef GAI_WITH_GL
    if(GAI.open_gl){

	glcontext = gtk_widget_get_gl_context(GAI.drawingarea);
	gldrawable = gtk_widget_get_gl_drawable(GAI.drawingarea);

	while(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)){
	    usleep(10000);
	}

	if(GAI.gl_init_func)
	    GAI.gl_init_func(NULL);

	gdk_gl_drawable_gl_end(gldrawable);

    }
#endif 

    /* Freeze updates, if requested */
    if(GAI.freeze){
	gdk_window_freeze_updates(GAI.widget->window);
	gdk_window_freeze_updates(GAI.drawingarea->window);
    }

    gtk_widget_show_all(GAI.widget);

    if(GAI.hide_mouse_ptr)
	gai_hide_mouse_ptr();
    else
	gai_show_mouse_ptr();


    if(GAI.tooltips_msg != NULL){
	gai_tooltip_set(GAI.tooltips_msg);
	g_free(GAI.tooltips_msg);
	GAI.tooltips_msg = NULL;
    }

    /* Start with updating */
    if(GAI.on_update_callback)
	GAI.on_update_callback(GAI.on_update_userdata);
    

    gtk_main();

    GAI_LEAVE;

}

