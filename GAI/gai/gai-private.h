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

#ifndef __GAI_PRIVATE_H__
#define __GAI_PRIVATE_H__



#include "config.h"


#define GAI_DEFAULT_H 64
#define GAI_DEFAULT_W 64
#define GAI_DEFAULT_UPDATE_INTERVAL 200

/* 10 times a second is the joystick function called */
#define GAI_JOYSTICK_UPDATE 100   

/* 10 times a second is the sample rate */
#define GAI_JOYSTICK_DELAY 100000

/* Gnome makes a ugly border of a pixel around the applet */
#define GNOME_STEALS 2

/* This is somewhat the default dpi resoltion for screens. Might not fit for everyone.. */
#define GAI_DPI_X 100
#define GAI_DPI_Y 100


#define GAI_ENV_APPLET_SIZE "GAI_APPLET_SIZE"
#define GAI_ENV_APPLET_XWINDOW "GAI_APPLET_XWINDOW"

#include <gtk/gtk.h>

/* Pango and Pango-FreeType2 are needed for text rendering */
#include <pango/pango.h>
#include <pango/pangoft2.h>


#ifdef GAI_WITH_GL
#include <gtk/gtkgl.h>
#endif

#ifdef GAI_WITH_GNOME
#include <panel-applet.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#endif


#include <stdio.h>
#include <string.h>

typedef enum {
    GAI_PREF_NONE = 1<<0,
    GAI_PREF_OWN  = 1<<1,
    GAI_PREF_GEN  = 1<<2,
    GAI_PREF_GEN2 = 1<<3
} GaiPrefType;

typedef struct
{
    char *name, *icon, *key;
    int type;
    void *ptr, *func;
} MenuHashItem;


typedef struct
{
    GaiApplet applet;

    int  applet_type;
    
    int  default_width;
    int  default_height;

    int  width;
    int  height;

    int  auto_scale;
    float scale;

    int use_default_background;
    int background_has_border;

    int update_interval;
    int broken_wm;

    int mask;

    /* Used for knowing when to start activate background change in gnome */
    int timer_started;

    /* Used for knowing when to clear the altered background buffer */
    int draw_bg_update_done;
    int hide_mouse_ptr;

    /* This is the currecnt background, scaled and fixed */
    GdkPixbuf *background;
    /* This is the original background, generated or loaded from file.
       Transparency is fixed. */
    GdkPixbuf *orig_background;

    /* Transparency is not fixed in this file. Used for regenerations of orig_background */
    GdkPixbuf *file_background;

    /* Forground drawings are done here */
    GdkPixbuf *foreground;

    /* This is what is behind the applet for nice transparency effects */
    GdkPixbuf *behind_applet;
    gboolean transparent_bg;

    /* This is the root window */
    GdkWindow *root_window;

    /* Basic widgets */
    GtkWidget *widget, *drawingarea, *about;

    /* Used in gai_text_create_simple() */
    PangoFT2FontMap *simple_fontmap;
    PangoContext *simple_context;

    /* Run in Panel mode */
    gboolean panel;


#ifdef GAI_WITH_GNOME
    int gnome_started;
#endif

    int timer;
    int orient;
    int rotate;

    GaiPrefType has_preferences;

    /* Window id, if we're going to dock a known window */
    int parent_window;

    /* Size in gnome mode */
    int applet_size;

    GtkTooltips *tooltips;
    char *tooltips_msg;
    int restarting;

    GdkWindow *icon_window;
    GdkWindow *window;
    GdkGC *gc;

    int lock;				/* Turn it on when crucial functions shall not be called */

    GaiNoteBook *gn;
    char *pref_name;
    GaiPI *pref_instr;

    /* Keeping track of what pref2 allocated */
    GHashTable *pref_mem_usage;
    	 
    char *help_text;

    int max_size;
    int debug;
    int window_maker;

    int use_help;
    int init_done;


    /* Controls freeze/thaw */
    gboolean freeze;

    /* This toggles if internal size change is going on */
    char size_changing;

    /* This is for handling the background of the gnome panel */
    int bg_type;
    GdkColor bg_colour;
    GdkPixbuf *bg_pixbuf;

#ifdef GAI_WITH_GL
    int	open_gl;
    GaiCallback0* gl_init_func;
    GdkGLConfig *glconfig;
    int *argc;
    char ***argv;
#endif

    int did_exit_function;

    int foreground_alpha;

    char* binfile;


    /* 1 = if mouse ptr is over the applet, else 0 */
    char applet_focused;

    /* For the menu */
    GHashTable *menu_hash;
    GSList *menu_list;
    int menu_entries;
    GtkWidget *menu;
    char *menu_help_text;
    gboolean menu_changed;
#ifdef GAI_WITH_GNOME
    BonoboUIVerb *menu_verb;
    char *menu_xml;
#endif

#ifdef ENABLE_NLS
    gchar *locale;
#endif


    GaiCallback0* init_function;

    GaiCallback0* on_exit_callback;
    gpointer      on_exit_userdata;
    GaiCallback0* on_update_callback;
    gpointer      on_update_userdata;
    GaiCallback0* on_enter_callback;
    gpointer      on_enter_userdata;
    GaiCallback0* on_leave_callback;
    gpointer      on_leave_userdata;
    GaiCallback1* on_keypress_callback;
    gpointer      on_keypress_userdata;
    GaiCallback2* on_mouse_click1_callback;
    gpointer      on_mouse_click1_userdata;
    GaiCallback2* on_mouse_move_callback;
    gpointer	  on_mouse_move_userdata;
    GaiCallback2* on_mouse_click2_callback;
    gpointer      on_mouse_click2_userdata;
    GaiCallback2* on_mouse_release1_callback;
    gpointer      on_mouse_release1_userdata;
    GaiCallback2* on_mouse_release2_callback;
    gpointer      on_mouse_release2_userdata;
    GaiCallback1* on_scroll_buttons_callback;
    gpointer      on_scroll_buttons_userdata;
    GaiCallback3* on_change_callback;
    gpointer      on_change_userdata;
    GaiCallback1* on_preferences_callback;
    gpointer      on_preferences_userdata;


#ifdef GAI_WITH_JOYSTICK
    /* This is for joystick handeling */
    GaiFlagsJoystick jflags;
    GaiCallback1* on_joystick_callback;
    gpointer      on_joystick_userdata;
    int timer_joystick;
#endif

    FILE *debug_output;
    int debug_depth;

} GAI_struct;

extern GAI_struct* gai_instance;
#define GAI (*gai_instance)

/* gai-debug.c */
void gai_log_debug_init (void);
void gai_log_event(const char *);
void gai_display_queued_errors(void);

/* gai.c */
void gai_is_init (void);
void gai_hide_mouse_ptr(void);
void gai_show_mouse_ptr(void);

/* gai-draw.c */
void gai_load_background (void);

/* gai-dockapp.c */
int gai_detect_window_maker(void);
int  gai_timer(gpointer);
void gai_hook (void);
void gai_on_remove_activate (GtkMenuItem *, gpointer);
void gai_on_about_activate(GtkMenuItem *, gpointer);
void gai_on_help_activate(GtkMenuItem *, gpointer);
void gai_on_preferences_activate (GtkMenuItem *, gpointer);
int gai_dies(gpointer);
void gai_dockapp_main(void);


/* gai-pref.c */
void gai_make_preference_window(const char *, GaiNoteBook *);

/* gai-pref2.c */
void gai_make_preference_window2(const char *, GaiPI *);


/* gai-gnome.c */
#ifdef GAI_WITH_GNOME
void gai_gnome_create_menu();

void gai_gnome_main(void);
GdkPixbuf *gai_rotate(GdkPixbuf *);
void gai_gnome_tooltip_set(char *);
void gai_gnome_tooltip_remove(void);
void  gai_gnome_server_info(FILE* out);
void gai_gnome_about_show(void);
int gai_gnome_change_size(GtkWidget *, int, gpointer);
void gai_gnome_init(GaiApplet *, GaiCallback0, int *, char ***);
#endif
void gai_size_change(int, int, int, int, int);
int  gai_gnome_detect_applet_type (int argc, char** argv);

/* gai-transparent.c */
gboolean gai_root_window_config(GtkWidget *, GdkEventConfigure *, gpointer);
GdkFilterReturn gai_root_window_event(GdkXEvent *, GdkEvent *, gpointer);


/* gai-rox.c */
#ifdef GAI_WITH_ROX
void gai_rox_window(void);
#endif

/* FIXME: Just for KDE testing */
#define GAI_WITH_KDE

#ifdef GAI_WITH_KDE
void gai_kde_window(void);
#endif



extern const char GAI_spaces[];

#define GAI_D(...) if(GAI.debug && GAI.debug_output != NULL) { \
                         if(strlen(GAI_spaces) > GAI.debug_depth) \
                             fwrite(GAI_spaces,1,GAI.debug_depth,GAI.debug_output); \
                         fprintf(GAI.debug_output, "%s: ", __FUNCTION__); \
                         fprintf(GAI.debug_output,__VA_ARGS__); \
                         fflush(GAI.debug_output); \
                   }

#define GAI_ENTER   GAI_D(" -- entering\n"); \
                    GAI.debug_depth++;

#define GAI_LEAVE   GAI_D(" -- leaving\n"); \
                    GAI.debug_depth--;
#define GAI_CHECK   GAI_D(" * checkpoint *\n")


#define GAI_NOTE(X) GAI_D("%s\n",X)
#define GAI_INT(X)  GAI_D("%d\n",X)
#define GAI_UNIT(X) GAI_INT(X)

#ifdef ENABLE_NLS
#define T(X) dgettext(GAI.applet.name, X)
#else
#define T(X)
#endif

#endif
