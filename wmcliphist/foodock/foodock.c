/*
 * File: foodock.c
 *
 * Created: Fri Jan 14 01:15:24 2000
 * 
 * (c) 2000, Alexey Vyskubov <alexey@pepper.spb.ru>
 *
 * LGPL, see file LICENSE
 */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "foodock.h"

/*
 * Function foo_create_main_icon_window returns pointer to gtk event
 * box. This event box is created into main window and can be used as
 * a dockable Windowmaker applet. Main window should be realized
 * before calling foo_create_main_icon_window. Returned event box
 * will be realized by foo_create_main_icon_window. You should to show
 * icon window as well as main window before gtk_main().
 *
 * Call foo_set_wmhints() after both windows are shown (gtk_widget_show()).
 * 
 * Input:
 *	mw		Pointer to main window
 *	s		icon window size (56 is recommended)
 *	margc, margv	argc and argv of main program for XSetCommand
 */

GtkWidget *foo_create_main_icon_window(GtkWidget *mw,
				       unsigned int s,
				       int margc,
				       char *margv[]) {

    GtkWidget *foobox; /* This will become icon box */

    foobox = gtk_event_box_new();
    gtk_widget_set_usize(foobox, s, s);
    gtk_container_add (GTK_CONTAINER (mw), foobox);
    gtk_widget_realize(foobox);

    return foobox;
}

/*
 * Set WMHints on the dockapp (icon) window. Needs to be called after
 * the main window is shown, due to changes in GTK+ 2.4.
 * 
 * Input:
 *	mw		Pointer to main window
 *	dw		Pointer to icon (dockapp) window
 *	margc, margv	argc and argv of main program for XSetCommand
 */
void foo_set_wmhints(GtkWidget *mw,
		     GtkWidget *dw,
		     int margc,
		     char *margv[]) {
    Window xmw;
    XWMHints *wm_hints;

    xmw = GDK_WINDOW_XWINDOW(mw->window);

    
/* Time for game with Xlib */
    wm_hints = XAllocWMHints();
    wm_hints->window_group = xmw;
    wm_hints->icon_window = GDK_WINDOW_XWINDOW(dw->window);
    wm_hints->icon_x = 0;
    wm_hints->icon_y = 0; 
    wm_hints->initial_state = WithdrawnState;
    wm_hints->flags = StateHint |
	              IconPositionHint |
		      WindowGroupHint |
		      IconWindowHint |
		      InputHint;

    XSetWMHints(GDK_DISPLAY(), xmw, wm_hints);

    XSetCommand(GDK_DISPLAY(), xmw, margv, margc);
    
    XFree(wm_hints);
}
