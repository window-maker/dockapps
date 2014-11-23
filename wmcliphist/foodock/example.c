/*
 * File: example.c
 *
 * This file is a part of foodock library
 *
 * (c) 2000 Alexey Vyskubov <alexey@pepper.spb.ru>
 */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include "foodock.h"

int main( int argc, char *argv[] ) {

    GtkWidget *gtkiw;
    GtkWidget *box;
    GtkWidget *button;
    
    gtk_init(&argc, &argv);

    gtkiw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_realize(gtkiw);
    
    box = foo_create_main_icon_window(gtkiw, 56, argc, argv);

    button = gtk_button_new_with_label ("FOO");

    gtk_container_add (GTK_CONTAINER (box), button);

    gtk_widget_show(button);
    gtk_widget_show(box);
    gtk_widget_show(gtkiw);

    gtk_main ();
     
    return(0);
}
