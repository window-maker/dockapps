
/* 
   This is a very simple Panel.
   Its purpose is to show that it is quite easy to use GAI applets in any panel
   and maybe influence someone to write a panel of his own.

   Written by Jonas Aaberg <cja@gmx.net> 2004.

   Released under GNU GPL.

*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

static int applet_size = 64, num_applets = 0;
static char **applets;

/* This function interprets the command line options and converts the
   to something that create_panel understands */
void init_panel(int argc, char **argv)
{
    int i=1,j;

    if(argc == 1){
	printf("A very simple panel - Just to demo GAI applets\n\n"
	       "Syntax: %s [--size X] applets..\n\n",argv[0]);
	exit(0);
    }

    /* The default size is 64 */
    if(!strcmp(argv[1],"--size")){
	applet_size = atoi(argv[2]);
	i+=2;
    }

    /* Count the number of applets that will be on the panel */
    num_applets = argc-i;

    /* allocate pointers to the applets */
    applets = g_malloc0(sizeof(char *)*num_applets);

    for(j=0;j<num_applets;j++)
	applets[j] = argv[i+j];
    

}

void create_panel(void)
{
    GtkWidget *window, *hbox, **drawingarea;
    int i;

    drawingarea = g_malloc0(sizeof(GtkWidget *)*num_applets);

    /* Create a window that acts like a panel */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 5);
    gtk_window_stick(GTK_WINDOW(window));
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);

    gtk_window_set_default_size(GTK_WINDOW(window), (applet_size+10)*num_applets, applet_size+10);
    hbox = gtk_hbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(window), hbox);


    for(i=0;i<num_applets;i++){
	drawingarea[i] = gtk_drawing_area_new();
	gtk_box_pack_start_defaults(GTK_BOX(hbox), drawingarea[i]);
    }

    gtk_widget_realize(window);
    gdk_window_set_type_hint(window->window, GDK_WINDOW_TYPE_HINT_TOOLBAR);

    /* Load all the applet into the panel */
    for(i=0;i<num_applets;i++){

	/* The widget has to be realized before we can get an XWINDOW id of the drawing area */
	gtk_widget_realize(drawingarea[i]);

	/* Pass Window and size to each applet via enviromental variables */
	putenv(g_strdup_printf("GAI_APPLET_SIZE=%d\n",applet_size));
	putenv(g_strdup_printf("GAI_APPLET_XWINDOW=%d\n",
			       (int)GDK_WINDOW_XWINDOW(drawingarea[i]->window)));

	/* Execute the given applets, and just quit on errors */
	if(!fork()){
	    execlp(applets[i],applets[i],NULL);
	    exit(0);
	}
    }

    /* Showing all parts */
    gtk_widget_show_all(window);

    /* With Enlightenment atleast, the window must be shown before you can move it. */

    /* This places the panel in the bottom right corner */
    gdk_window_move(window->window,
		    gdk_screen_get_width(gdk_screen_get_default()) - (applet_size+10)*num_applets,
		    gdk_screen_get_height(gdk_screen_get_default()) - (applet_size+10));

    g_free(drawingarea);

}

int main(int argc, char **argv)
{

    gtk_init(&argc, &argv);
    init_panel(argc, argv);
    create_panel();

    gtk_main();
    return 0;
}
