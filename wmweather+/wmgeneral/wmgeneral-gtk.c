#include "../config.h"

/*
    Best viewed with vim5, using ts=4

    wmgeneral was taken from wmppp.

    It has a lot of routines which most of the wm* programs use.

    ------------------------------------------------------------

    Author: Brad Jorsch, anomie@users.sourceforge.net

    ---
    CHANGES:
    ---
    15/08/2002 (Brad Jorsch, anomie@users.sourceforge.net)
        * Pulled createXBMfromXPM into its own file, because it's the same in
          both -gtk and -x11.

    11/08/2002 (Brad Jorsch, anomie@users.sourceforge.net)
        * This is based on wmgeneral-x11.c (formerly wmgeneral.c), it
          implements a subset of the interface using Gtk+ 2.0

*/

#include <stdio.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "wmgeneral-gtk.h"

  /******************/
 /* Gtk+ Variables */
/******************/

static GtkWidget *dockwin, *iconwin;
static GdkPixmap *pixmap, *mask;
static GdkGC *pixmap_gc, *mask_gc;
static void (*click_func)(GdkEventButton *ev);

/******************************************************************************\
|* RedrawWindow                                                               *|
\******************************************************************************/

void RedrawWindow(void) {
    gdk_draw_drawable(dockwin->window, pixmap_gc, pixmap, 0, 0, 0, 0, 64, 64);
    gdk_draw_drawable(iconwin->window, pixmap_gc, pixmap, 0, 0, 0, 0, 64, 64);
}

static gint redraw_dock(gpointer d){
    RedrawWindow();
    return 0;
}

/******************************************************************************\
|* RedrawWindowXY                                                             *|
\******************************************************************************/

void RedrawWindowXY(int x, int y) {
    gdk_draw_drawable(dockwin->window, pixmap_gc, pixmap, x, y, 0, 0, 64, 64);
    gdk_draw_drawable(iconwin->window, pixmap_gc, pixmap, x, y, 0, 0, 64, 64);
}

/******************************************************************************\
|* copyXPMArea                                                                *|
\******************************************************************************/

void copyPixmapArea(int sx, int sy, int w, int h, int dx, int dy){
    gdk_draw_drawable(pixmap, pixmap_gc, pixmap, sx, sy, dx, dy, w, h);
}

/******************************************************************************\
|* copyXBMArea                                                                *|
\******************************************************************************/

void copyMaskArea(int sx, int sy, int w, int h, int dx, int dy){
    gdk_draw_drawable(mask, mask_gc, mask, sx, sy, dx, dy, w, h);
}


/******************************************************************************\
|* setMaskXY                                                                  *|
\******************************************************************************/

void setMaskXY(int x, int y) {
     gtk_widget_shape_combine_mask(dockwin, mask, x, y);
     gtk_widget_shape_combine_mask(iconwin, mask, x, y);
}

/******************************************************************************\
|* setClickCallback                                                           *|
\******************************************************************************/
void setClickCallback(void (*func)(GdkEventButton *ev)){
    click_func=func;
}

/******************************************************************************\
|* openXwindow                                                                *|
\******************************************************************************/

static GdkWindow *get_gdk_leader(GdkWindow *win){
    GdkAtom atom, type;
    gint len;
    guchar *data;
    GdkWindow *leader=NULL;

    atom=gdk_atom_intern("WM_CLIENT_LEADER", TRUE);
    type=gdk_atom_intern("WINDOW", TRUE);
    if(atom==GDK_NONE || type==GDK_NONE) return NULL;
    if(!gdk_property_get(win, atom, type, 0, 4, FALSE, NULL, NULL, &len, &data)) return NULL;                                                                       if(len==4) leader=gdk_window_foreign_new(*(GdkNativeWindow *)data);
    g_free(data);
    return leader;
}

static GdkFilterReturn button_filter(XEvent *x, GdkEvent *ev, gpointer data){
    /* Bleh, Gdk insists on trying to translate buttons 4-7 into Scroll events.
     * Which would be ok, except for the part where it just _throws_ _away_ the
     * releases! Damnit... So, we cheat and change any buttons >3 into
     * button+4, and change it back in the click handler. */
    if((x->type==ButtonPress || x->type==ButtonRelease) && x->xbutton.button>3){
        x->xbutton.button+=4;
    }
    return GDK_FILTER_CONTINUE;
}

static void dock_click(GtkWidget *w, GdkEventButton *ev, gpointer d){
    if(ev->button>7) ev->button-=4;
    if(click_func!=NULL) click_func(ev);
}

#define die(args...) { fprintf(stderr, args); exit(1); }

void openDockWindow(int argc, char *argv[], char *pixmap_bytes[], char *pixmask_bits, int pixmask_width, int pixmask_height){
    GdkColormap *cmap;
    GdkColor white, black;
    GdkWindow *leader;
    XWMHints hints;

    click_func=NULL;
    if((dockwin=gtk_window_new(GTK_WINDOW_TOPLEVEL))==NULL) die("Couldn't create window");
    if((iconwin=gtk_window_new(GTK_WINDOW_TOPLEVEL))==NULL) die("Couldn't create window");
    gtk_widget_set_size_request(dockwin, 64, 64);
    gtk_widget_set_size_request(iconwin, 64, 64);
    gtk_widget_set_app_paintable(dockwin, TRUE);
    gtk_widget_set_app_paintable(iconwin, TRUE);
    gtk_widget_add_events(dockwin, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_EXPOSURE_MASK | GDK_SCROLL_MASK);
    gtk_widget_add_events(iconwin, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_EXPOSURE_MASK | GDK_SCROLL_MASK);
    g_signal_connect(G_OBJECT(dockwin), "expose-event", G_CALLBACK(redraw_dock), NULL);
    g_signal_connect(G_OBJECT(iconwin), "expose-event", G_CALLBACK(redraw_dock), NULL);
    g_signal_connect(G_OBJECT(dockwin), "button-press-event", G_CALLBACK(dock_click), NULL);
    g_signal_connect(G_OBJECT(iconwin), "button-press-event", G_CALLBACK(dock_click), NULL);
    g_signal_connect(G_OBJECT(dockwin), "button-release-event", G_CALLBACK(dock_click), NULL);
    g_signal_connect(G_OBJECT(iconwin), "button-release-event", G_CALLBACK(dock_click), NULL);
    g_signal_connect(G_OBJECT(dockwin), "destroy", G_CALLBACK(exit), NULL);
    g_signal_connect(G_OBJECT(iconwin), "destroy", G_CALLBACK(exit), NULL);
    gtk_widget_realize(dockwin);
    gtk_widget_realize(iconwin);
    gdk_window_add_filter(dockwin->window, (GdkFilterFunc)button_filter, NULL);
    gdk_window_add_filter(iconwin->window, (GdkFilterFunc)button_filter, NULL);
    if((leader=get_gdk_leader(dockwin->window))==NULL) die("Couldn't obtain Gdk leader window");
    gdk_window_set_icon(leader, iconwin->window, NULL, NULL);
    gdk_window_reparent(iconwin->window, leader, 0, 0);
    gdk_window_unref(leader);

    hints.initial_state = WithdrawnState;
    hints.flags = StateHint;
    XSetWMHints(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(dockwin->window), &hints);

    cmap=gdk_colormap_get_system();
    white.red=65535;
    white.green=65535;
    white.blue=65535;
    black.red=0;
    black.green=0;
    black.blue=0;
    gdk_color_alloc(cmap, &white);
    gdk_color_alloc(cmap, &black);
    mask=gdk_pixmap_create_from_data(NULL, pixmask_bits, pixmask_width, pixmask_height, 1, &white, &black);
    pixmap=gdk_pixmap_colormap_create_from_xpm_d(NULL, cmap, NULL, NULL, pixmap_bytes);

    pixmap_gc=gdk_gc_new(iconwin->window);
    mask_gc=gdk_gc_new(mask);

    setMaskXY(0, 0);
    RedrawWindow();

    gtk_widget_show(iconwin);
    gtk_widget_show(dockwin);
    gdk_window_withdraw(dockwin->window);
}
