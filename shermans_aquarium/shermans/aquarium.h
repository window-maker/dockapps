
/* Main header file for Sherman's aquarium */
#ifndef AQUARIUM_H
#define AQUARIUM_H


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "defines.h"

#ifdef GNOME
#include <libgnomeui/libgnomeui.h>
#include <libgnome/libgnome.h>
#endif


typedef struct {
    unsigned char *image;
    unsigned char *rev;
    int width;
    int height;
    int frames;
    int full_height;
    int rowstride;
    GdkPixbuf *pixbuf;
} SA_Image;

/* main dockapp info structure.  windows, buffers, etc */
typedef struct {

    int xmax;
    int ymax;
    int xmin;
    int ymin;
    /*    int windowsize_x;
	  int windowsize_y;*/
    int virtual_aquarium_x;
    int virtual_aquarium_y;
    int viewpoint_start_x;
    int viewpoint_start_y;

    int posx;
    int posy;
    int proximity;


    GdkWindow *win;		/* main window */
    GdkWindow *iconwin;


    GtkWidget *about_box;
    GtkWidget *applet;
    GtkWidget *drawingarea;
    int oldwidth;
    int oldheight;
    int oldbg;

    GdkGC *gc;			/* drawing GC */

    GRand *rnd;                 /* Random number instance */

    /* main image buffer */
    unsigned char *rgb;

    /* back buffer - stores things we dont want to redraw all the time */
    unsigned char *bgr;
    int special_action;
    int dont_update;

} AquariumData;


#define MOUSE_OFF 0
#define MOUSE_RUN 1

typedef struct {
    int ratio_width;
    int ratio_height;

    int mouse_left, mouse_middle, mouse_down, mouse_up;
    char *mouse_left_option, *mouse_middle_option, *mouse_up_option, *mouse_down_option;

} General_settings;

/* Declare functions */

void aquarium_update(gpointer);
void aquarium_change(int, int, int, gpointer);
void prepare_graphics(void);

void aquarium_reload_all(void);
void aquarium_free_all(void);
void aquarium_draw_image(int, int, int, int, SA_Image *);
void aquarium_clean_image(int, int, int, int);
void aquarium_draw_pic_alpha(SA_Image *, int, int, int, int, int, int);
unsigned char *aquarium_install_path(void);

/* Returns a pointer to the static struct in aquarium.c */
AquariumData *aquarium_get_settings_ptr(void);
General_settings *general_get_settings_ptr(void);

void load_image(char *, SA_Image *,int);
void load_image_n_scale(char *, SA_Image *,int, int);

unsigned int checksum(unsigned char *, int);

#endif
