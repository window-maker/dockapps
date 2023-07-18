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

#ifndef __GAI_H__
#define __GAI_H__

/*
#include "gaiconf.h"
#include "gaidefs.h"
*/

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdkkeysyms.h>



#define GAI_GNOME1   1
#define GAI_DOCKAPP  2
#define GAI_GNOME2   3
#define GAI_ROX      4
#define GAI_KDE      5
#define GAI_XFCE4    6


#define GAI_HORIZONTAL 1
#define GAI_VERTICAL   2


#define GAI_TEXT_NORMAL 0
#define GAI_TEXT_ITALIC 1
#define GAI_TEXT_BOLD   2
#define GAI_TEXT_SMOOTH 4

#define GAI_MOUSE_BUTTON_1 1
#define GAI_MOUSE_BUTTON_2 2


#define GAI_BACKGROUND_MAX_SIZE_NONE  -1
#define GAI_BACKGROUND_MAX_SIZE_IMAGE -2

#define GAI_MENU_STOCK     1
#define GAI_MENU_SEPARATOR 2
#define GAI_MENU_NONE 3
#define GAI_MENU_FILE 4


#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    GAI_JOYSTICK_NOTHING =  0,
    GAI_JOYSTICK_LEFT =     (1 << 0),
    GAI_JOYSTICK_RIGHT =    (1 << 1),
    GAI_JOYSTICK_UP =       (1 << 2),
    GAI_JOYSTICK_DOWN =     (1 << 3),
    GAI_JOYSTICK_BUTTON_A = (1 << 4),
    GAI_JOYSTICK_BUTTON_B = (1 << 5)
} GaiFlagsJoystick;

typedef enum
{
    GAI_FLAGS_MOUSE_PTR_HIDE = 1 << 0,
    GAI_FLAGS_MOUSE_PTR_SHOW = 1 << 1,
    GAI_FLAGS_ALLOW_ROTATE   = 1 << 2,
    GAI_FLAGS_NEVER_ROTATE   = 1 << 3,
    GAI_FLAGS_OPEN_GL_WINDOW = 1 << 4,
    GAI_FLAGS_FREEZE_UPDATES = 1 << 5,
    GAI_FLAGS_THAW_UPDATES   = 1 << 6,
    GAI_FLAGS_TRANSPARENT    = 1 << 7,
    GAI_FLAGS_PANEL	     = 1 << 8
} GaiFlagsType;



typedef gboolean GaiCallback0(gpointer);
typedef gboolean GaiCallback1(int, gpointer);
typedef gboolean GaiCallback2(int,int, gpointer);
typedef gboolean GaiCallback3(int,int,int, gpointer);



/* This is for the deprecated preference window v1 */
typedef struct {
    char *name;
    void *ptr;
} GaiNoteBook;

typedef struct {
    char *name;
    int  *default_val;
    int  *result;
} GaiCheckButton;

typedef struct {
    int type;
    void *ptr;
} GaiBox;


typedef struct {
    char *name;
    char **default_val;
    char **result;
} GaiTextEntry;

typedef struct {
    char *name;
    char **default_val;
    char **result;
} GaiPasswordEntry;

typedef struct{
    char *name;
} GaiText;

typedef struct{
    char *name1;
    char *name2;
    void *ptr1;
    void *ptr2;
} GaiFrame;

typedef struct{
    char *name1;
    char *name2;
    char *name3;
    void *ptr1;
    void *ptr2;
    void *ptr3;
} GaiFrame3;


typedef struct{
    char *name;
    int  *default_val;
    int  *result;
    int   group_number;
} GaiRadioButton;

typedef struct{
    char *name;
    int *default_val;
    int *min;
    int *max;
    int *step;
    int *result;
} GaiSpinButton;


typedef struct{
    char *name;
    float *default_val;
    float *min;
    float *max;
    float *step;
    float *result;
    int *decimals;
} GaiSpinButtonFloat;


typedef struct
{
    char *name;
    unsigned char *r;
    unsigned char *g;
    unsigned char *b;
    unsigned char *alpha;
    int number;
} GaiColourSelector;

typedef struct
{
    char *name;
    int *val;
    GList **list;
} GaiCombo;

typedef struct
{
    char *name;
    char **default_val;
    char **result;
    int number;
} GaiFileSelector;

typedef struct
{
    char *name;
    char **option_list;
    int *default_val;
    int *result;
    int number;
} GaiOptionMenu;


/* end of pref */


/* Second generation of preference window generator */

typedef enum {
    GAI_END              = 0x00,		/* */
    GAI_CHECKBUTTON      = 0x01,		/* Name, default, result */
    GAI_TEXTENTRY        = 0x02,		/* Name, default, result */
    GAI_TEXT             = 0x03,		/* Name, */
    GAI_FRAME		 = 0x04,		/* Name */
    GAI_RADIOBUTTON      = 0x05,		/* Name, default, result, list */
    GAI_SPINBUTTON       = 0x06,         	/* Name, default, result, {min,max,step} */
    GAI_COLORSELECTOR    = 0x07,		/* Name, default(int of RGBA), result, */
    GAI_HLINE            = 0x08,         	/* Just a line */
    GAI_FILESELECTOR     = 0x09,         	/* Name, default, result*/
    GAI_NOTEBOOK 	 = 0x0A,		/* Name, */
    GAI_COMBOBOX         = 0x0B,		/* Name, default, result, list - Same as outdated GAI_OPTIONMENU*/
    GAI_OPTIONMENU       = 0x0B,		/* Name, default, result, list */
    GAI_PASSWORDENTRY    = 0x0C,		/* Name, default, result, sign */
    GAI_SPINBUTTON_FLOAT = 0x0D,		/* Name, default, result, {min, max,step} */
    GAI_COMBO            = 0x0E,		/* Name, default, result, list */
    GAI_COMBOENTRY       = 0x0E,		/* Name, default, result, list - Same as outdated GAI_COMBO*/
    GAI_ALL_LEFT         = 0x0F,		/* Text Alignment left */
    GAI_ALL_CENTER       = 0x10,		/* Text Alignment center */
    GAI_ALL_RIGHT        = 0x11,		/* Text Alignment right */
    GAI_FRAME_R		 = 0x12,		/* Name */
    GAI_FRAME_E		 = 0x13,		/* - */
    GAI_NOTEBOOK_E	 = 0x14,		/* - */
    GAI_BUTTON_TEXT      = 0x15,   	      	/* Name, function */
    GAI_BUTTON_STOCK     = 0x16,       		/* Name, stock image, func */
    GAI_BUTTON_IMAGE     = 0x17,         	/* Name, image, func */
    GAI_LISTSTORE        = 0x18,                /* Names[], defaults[], results[], types[] */
    GAI_EDITLISTSTORE    = 0x19,                /* Names[], defaults[], results[], types[] */
    GAI_HSCROLL_BOX      = 0x1A,                /* - Horizontal scrollbar, if needed */
    GAI_HSCROLL_BOX_E    = 0x1B,                /* - */
    GAI_VSCROLL_BOX      = 0x1C,                /* - Vertical scrollbar, if needed */
    GAI_VSCROLL_BOX_E    = 0x1D,                /* - */
    GAI_SCROLL_BOX       = 0x1E,                /* - Scrollbars on both sides, if needed */
    GAI_SCROLL_BOX_E     = 0x1F,                /* - */


    /* 0x0 - 0x3f for types */

    /* Alignments and frames */
    GAI_LEFT            = 0x40<<0,
    GAI_RIGHT		= 0x40<<1,
    GAI_CENTER		= 0x40<<2, 	/* Comibination of left and right */
    GAI_NO_TEXT_MARKUP	= 0x40<<3,	/* Do use the pango markup text style */

} GaiPrefTypes;

/* Backward & uk-english compability */
#define GAI_COLOURSELECTOR   GAI_COLORSELECTOR
#define GAI_FRAME3           GAI_NOTEBOOK
#define GAI_SPINBUTTONFLOAT  GAI_SPINBUTTON_FLOAT


typedef struct {
    GaiPrefTypes type;
    const void *name;
    void *default_val;
    void *result_val;
    void *extra;
} GaiPI;

/* Gai Spinbutton Settings */
typedef struct {
    int min;
    int max;
    int step;
} GaiSS;



/* Gai Spinbutton Settings - Float*/
typedef struct {
    float min;
    float max;
    float step;
    int decimals;
} GaiSSF;


typedef struct {
    unsigned char r, g, b, alpha;
} GaiColor;

/* End second gen pref window */



typedef struct
{
    char *name;
    char *version;
    char *nice_name;
    char *author;
    char *license;
    char *description;
    char *icon;
    char *image_path;
} GaiApplet;

int gai_init2(GaiApplet *appstruct, int *argc_p, char ***argv_p);

int gai_init(const char * name, const char * version, const char * image_path,
	     int* argc_p, char *** argv_p);

void gai_background_from_xpm(const char **xpm_image, int max_size);
void gai_background_from_gdkpixbuf(GdkPixbuf *pixbuf, int max_size);
void gai_background_from_file(const char *file, int max_size);
void gai_background_set(int width, int height, int max_size, int border);
			


void gai_menu_add_help_text(const char *text);
int gai_menu_add(const char *name, const char *icon, int type, void *func, void *ptr);
gboolean gai_menu_change(int id, const char *name, const char *icon, int type, void *func, void *ptr);
int gai_menu_insert(int num, const char *name, const char *icon, int type, void *func, void *ptr);
void gai_menu_remove(int id);


void gai_preferences(const char *window_name, GaiNoteBook *nbstruct, const char *help,
		     GaiCallback1 func, gpointer userdata);

void gai_preferences2(const char *window_name, GaiPI *pref_instr, const char *help_text, 
		      GaiCallback1 func, gpointer userdata);



/* Argument is function with open GL calls */
void gai_gl_init_func(GaiCallback0 function);


/* About field settings */

void gai_about_from(const char *msg);

/* Applet - gai settings */
void gai_flags_set(GaiFlagsType gf);
GaiFlagsType gai_flags_get(void);


/* Error handeling */

void gai_display_error_quit(const char *str);
void gai_display_error_continue(const char *str);



/* Starts it all */
void gai_start(void);


/* Low-level information */
GdkWindow *gai_get_window(void);
GtkWidget *gai_get_drawingarea(void);
GdkGC *gai_get_gc(void);
int gai_get_size(void);
int gai_scale(int s);
int gai_get_orient(void);
int gai_applet_mode(void);


/* Signal functions */
void gai_signal_on_exit(GaiCallback0 func, gpointer userdata);
void gai_signal_on_update(GaiCallback0 func, int delay, gpointer userdata);
void gai_signal_on_update_interval_change(int delay);

void gai_signal_on_enter(GaiCallback0 func, gpointer userdata);
void gai_signal_on_leave(GaiCallback0 func, gpointer userdata);
void gai_signal_on_keypress(GaiCallback1 func, gpointer userdata);
void gai_signal_on_mouse_button_click(GaiCallback2 func, int button, gpointer userdata);
void gai_signal_on_mouse_button_release(GaiCallback2 func, int button, gpointer userdata);
void gai_signal_on_mouse_move(GaiCallback2 func, gpointer userdata);
void gai_signal_on_scroll_buttons(GaiCallback1 func, gpointer userdata);
void gai_signal_on_change(GaiCallback3 func, gpointer userdata);
void gai_signal_on_preferences(GaiCallback1 func, gpointer userdata);
void gai_signal_on_joystick(GaiCallback1 func, gpointer userdata);




/* Image loading functions */
GdkPixbuf *gai_load_image(const char *);
GdkPixbuf *gai_load_image_at_size(const char *, int, int);



/* Drawing functions */
void gai_draw_update_bg(void);
void gai_draw_bg(GdkPixbuf *src, int sx, int sy, int sw, int sh, int dx, int dy);
void gai_draw_raw_bg(unsigned char *img, int x, int y, int w, int h, int rs);
void gai_draw_raw_alpha_bg(unsigned char *im, int x, int y, int w, int h, int rs);

void gai_draw_update(void);
void gai_draw(GdkPixbuf *src, int sx, int sy, int sw, int sh, int dx, int dy);
void gai_draw_raw(unsigned char *img, int x, int y, int w, int h, int rs);
void gai_draw_raw_alpha(unsigned char *img, int x, int y, int w, int h, int rs);



/* Tool tip set and remove */
void gai_tooltip_set(const char *msg);
void gai_tooltip_remove(void);

/* Make pixbuf out of a textstring */
GdkPixbuf *gai_text_create(const char *text, const char *font, 
			   int font_size, int font_features, 
			   char r, char g, char b);
/* Make a pixbuf out of a textstring and uses the default font */
GdkPixbuf *gai_text_create_simple(const char *text, char r, char g, char b);

/* Execute a program in the background */
void gai_exec(const char *prg);

/* For saving and loading settings */

void gai_save_int(const char *name, int data);
void gai_save_bool(const char *name, int data);
void gai_save_float(const char *name, float data);
void gai_save_string(const char *name, const char *data);
void gai_save_raw_data(const char *name, unsigned const char *data, int size);
void gai_save_glist(const char *name, GList *data);
void gai_save_gaicolor(const char *name, GaiColor data);

char *gai_load_string_with_default(const char *name, const char *valdefault);
int   gai_load_int_with_default(const char *name, int valdefault);
int   gai_load_bool_with_default(const char *name, int valdefault);
float gai_load_float_with_default(const char *name, float valdefault);
unsigned char *gai_load_raw_data(const char *name, int *size);
GList *gai_load_glist_with_default(const char *name, GList *default_list);
GaiColor gai_load_gaicolor_with_default(const char *name, GaiColor valdefault);


/************************************************************
 * native language suport
 ************************************************************/

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(String) gettext(String)

#ifdef gettext_noop
#define N_(String) gettext_noop(String)
#else
#define N_(String) (String)
#endif

#else /* NLS is disabled */

#define _(String) (String)
#define N_(String) (String)
#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,String) (String)
#define dcgettext(Domain,String,Type) (String)
#define bindtextdomain(Domain,Directory) (Domain) 
#define bind_textdomain_codeset(Domain,Codeset) (Codeset) 

#endif /* ENABLE_NLS */

/************************************************************/


#ifdef __cplusplus
}
#endif

#endif
