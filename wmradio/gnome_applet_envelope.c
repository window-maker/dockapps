/*
 * Copyright (C) 12 Jun 2003 Tomas Cermak
 * 
 * This file is part of wmradio program. 
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gnome.h>
#include <panel-applet.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>

#include "gnome_applet_envelope.h"
#include "skin.h"
#include "wmradio.h"
#include "rc.h"

GtkWidget *radio;
Window buffer;
GC NormalGC;
unsigned long gcm;
XGCValues gcv;
guint timer;

void logt(char *s)
{
    FILE *f;
    
    f = fopen("/tmp/wmradio.log","a");
    fputs(s,f);
    fputs("\n",f);
    fclose(f);
}

Pixel GetColor(char *ColorName, Display * disp, Window win)
{
    XColor Color;
    XWindowAttributes Attributes;

    XGetWindowAttributes(disp, win, &Attributes);
    Color.pixel = 0;

    if (!XParseColor(disp, Attributes.colormap, ColorName, &Color))
        printf("wmradio: can't parse %s\n", ColorName);
    else if (!XAllocColor(disp, Attributes.colormap, &Color))
        printf("wmradio: can't allocate %s\n", ColorName);

    return Color.pixel;
}


void gtk_drawing_area_copy_from_X11_drawable(GtkDrawingArea *DA,
					     Window X11,
					     GC gc,
					     int srcx,int srcy,
					     unsigned int width, unsigned int height,
					     int destx, int desty)
{
    GdkWindow *gdkwindow;
    
    g_return_if_fail( DA != NULL );
    g_return_if_fail( X11 != 0 );
    
    gdkwindow = GDK_WINDOW(GTK_WIDGET(DA)->window);
    if(!GDK_WINDOW_DESTROYED( gdkwindow ) ) {
	XCopyArea(GDK_DISPLAY(),
		  X11,
		  GDK_WINDOW_XWINDOW(gdkwindow),
		  gc,
		  srcx,srcy,width,height,destx,desty);
    }
}

gint on_expose(GtkWidget *w, GdkEventExpose *e)
{
    RadioEvent re;
    
    re.type = REVENT_EXPOSE;
    wmradio_handle_event(&re);
    return TRUE;
}

gint on_button_press(GtkWidget *w, GdkEventButton *e)
{
    RadioEvent re;
    
    re.type = REVENT_BUTTON_PRESS;
    re.x = e->x;
    re.y = e->y;
    re.control = e->state & GDK_CONTROL_MASK ? 
	CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
    re.shift   = e->state & GDK_SHIFT_MASK ? 
	CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
    wmradio_handle_event(&re);
    if( e->button & 2 ) return FALSE; 
    return TRUE;
}

gint on_button_release(GtkWidget *w, GdkEventButton *e)
{
    RadioEvent re;
    
    re.type = REVENT_BUTTON_RELEASE;
    re.x = e->x;
    re.y = e->y;
    re.control = e->state & GDK_CONTROL_MASK ? 
	CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
    re.shift   = e->state & GDK_SHIFT_MASK ? 
	CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
    wmradio_handle_event(&re);
    if( e->button & 2 ) return FALSE; 
    return TRUE;
}

gint on_scroll(GtkWidget *w, GdkEventScroll *e)
{
    RadioEvent re;
    
    switch(e->direction) {
    case GDK_SCROLL_UP:
	re.type = REVENT_SCROLL_UP;
	break;
    case GDK_SCROLL_DOWN:
	re.type = REVENT_SCROLL_DOWN;
	break;
    default:
	return FALSE;
    }
    re.x = e->x;
    re.y = e->y;
    re.control = e->state & GDK_CONTROL_MASK ? 
	CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
    re.shift   = e->state & GDK_SHIFT_MASK ? 
	CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
    wmradio_handle_event(&re);
    return TRUE;
}

gint on_timer(gpointer data)
{
    RadioEvent re;
    
    re.type = REVENT_TIMER;
    wmradio_handle_event(&re);
    return TRUE;
}

void on_realize(GtkWidget *widget, gpointer not_used)
{
    Pixel foreground,background;
    int screen;

    wmradio_init_radio_info();
    rc_read_config();
    create_skin(rc_get_variable(SECTION_CONFIG,"skin","gdefault.skin"),
		GDK_DISPLAY(),
		GDK_ROOT_WINDOW());
    gtk_drawing_area_size(GTK_DRAWING_AREA(radio), skin_width(),skin_height());
    wmradio_radio_info()->dont_quit_mode = 1;
    rc_set_variable_as_int(SECTION_CONFIG,"start-muted",1);
    wmradio_init();

    background = GetColor("black", GDK_DISPLAY(), GDK_ROOT_WINDOW());
    foreground = GetColor("white", GDK_DISPLAY(), GDK_ROOT_WINDOW());
    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = foreground;
    gcv.background = background;
    gcv.graphics_exposures = 0;
    NormalGC = XCreateGC(GDK_DISPLAY(), GDK_ROOT_WINDOW(), gcm, &gcv);
    screen = DefaultScreen(GDK_DISPLAY());
    buffer = XCreatePixmap(GDK_DISPLAY(),
			   GDK_ROOT_WINDOW(),
			   skin_width(),skin_height(),
			   DefaultDepth(GDK_DISPLAY(),screen)/*16  color_depth */);
    timer = gtk_timeout_add(1000,on_timer,NULL);
}

GtkWidget *video_init(void)
{
    radio = gtk_drawing_area_new();
    gtk_signal_connect(GTK_OBJECT(radio),"expose-event",
		       GTK_SIGNAL_FUNC(on_expose),NULL);
    gtk_signal_connect(GTK_OBJECT(radio),"button-press-event",
		       GTK_SIGNAL_FUNC(on_button_press),NULL);
    gtk_signal_connect(GTK_OBJECT(radio),"button-release-event",
		       GTK_SIGNAL_FUNC(on_button_release),NULL);
    gtk_signal_connect(GTK_OBJECT(radio),"realize",
		       GTK_SIGNAL_FUNC(on_realize),NULL);
    gtk_signal_connect(GTK_OBJECT(radio),"scroll-event",
		       GTK_SIGNAL_FUNC(on_scroll),NULL);
    gtk_widget_set_events(radio,
			  GDK_EXPOSURE_MASK
			  | GDK_BUTTON_PRESS_MASK
			  | GDK_POINTER_MOTION_MASK
			  | GDK_BUTTON_RELEASE_MASK
			  | GDK_SCROLL_MASK
			  );
    return radio;
}

void video_mainloop(void)
{
    /*applet_widget_gtk_main();*/
    gtk_timeout_remove(timer);
}

void video_close(void)
{
    /*applet_widget_remove(APPLET_WIDGET(applet));*/
    /*applet_widget_gtk_main_quit();*/
}

void video_draw(float freq,int stereo)
{
    skin_to_window(GDK_DISPLAY(),buffer, NormalGC,freq,stereo);
    gtk_drawing_area_copy_from_X11_drawable(
					    GTK_DRAWING_AREA(radio),
					    buffer,
					    NormalGC,
					    0,0,skin_width(),skin_height(),0,0);
}

static gboolean wmradio_applet_fill(PanelApplet *applet,
				    const gchar *iid,
				    gpointer data)
{
    if (strcmp (iid, "OAFIID:WMRadioApplet") != 0) return FALSE;
    
    video_init();
    gtk_container_add (GTK_CONTAINER (applet), radio);
    gtk_widget_show_all (GTK_WIDGET (applet));
    return TRUE;
}


PANEL_APPLET_BONOBO_FACTORY ("OAFIID:WMRadioApplet_Factory",
                             PANEL_TYPE_APPLET,
                             "The WMRadio Applet",
                             "0",
                             wmradio_applet_fill,
                             NULL);

/* int main(int argc, char *argv[]) */
/* { */
/*     GtkWidget *win; */
    
/*     gtk_set_locale(); */
/*     gtk_init(&argc,&argv); */
    
/*     win = gtk_window_new(GTK_WINDOW_TOPLEVEL); */
/*     radio = video_init(); */
/*     gtk_container_add (GTK_CONTAINER (win), radio); */
/*     gtk_widget_show_all(win); */
/*     gtk_main(); */
/*     return 0; */
/* } */
