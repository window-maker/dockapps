#ifndef	_WMCLIPHIST_H_
#define	_WMCLIPHIST_H_


#define _GNU_SOURCE

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include <fcntl.h>
#include <regex.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <foodock.h>

#include <debug.h>


#define	VERSION			0x0003
#define	DEF_LOCKED_COLOR	"red"
#define	DEF_MENUKEY		"Control+Alt+V"
#define	DEF_PREV_ITEM_KEY	"Control+Alt+C"
#define	DEF_EXEC_ITEM_KEY	"Control+Alt+E"
#define	MAX_ITEM_LENGTH		40


/* ==========================================================================
 *                                                        CLIPBOARD FUNCTIONS
 */

/* history item */
typedef struct {
	GtkWidget	*menu_item;
	gint		locked;
	gchar		*content;
	GtkWidget	*menu;
}	HISTORY_ITEM;


/* number of items to keep (may be overriden from command line) */
extern gint		num_items_to_keep;

/* when true, clipboard will be automatically taken up by wmcliphist */
extern gint		auto_take_up;

/* number of items kept */
extern gint		num_items;

/* list of clipboard history items */
extern GList		*history_items;

/* selected item */
extern HISTORY_ITEM	*selected;

/* current number of locked items */
extern gint		locked_count;



#ifdef DEBUG
#define	dump_history_list(header)	dump_history_list_fn(header)
#else
#define	dump_history_list(header)	
#endif


/*
 * get clipboard content - partialy inspired by Downloader for X
 */
gboolean
my_get_xselection(GtkWidget *window, GdkEvent *event);

/*
 * clipboard conversion - inspired by Downloader for X too :)
 */
gboolean
time_conv_select();

/*
 * handles request for selection from other apps
 */
gint
selection_handle(GtkWidget *widget, 
		GtkSelectionData *selection_data,
		guint info,
		guint time_stamp,
		gpointer data);


/* ==========================================================================
 *                                                                  RC CONFIG
 */

/* action record */
typedef struct {
	regex_t			expression;
	enum {
		ACT_EXEC,
		ACT_SUBMENU,
		ACT_IGNORE
	}			action;
	char			*command;
	GtkWidget		*menu_item;
	GtkWidget		*submenu;
} ACTION;


extern GList	*action_list;


/*
 * returns config/data file name in user's home
 */
char *
rcconfig_get_name(char *append);

/*
 * read and parse rcconfig
 */
int
rcconfig_get(char *fname);

/*
 * free rcconfig data
 */
void
rcconfig_free();



/* ==========================================================================
 *                                                                        GUI
 */

/* error codes */
#define	E_BASE		10000
#define	E_OPEN		(E_BASE | 1)
#define	E_INVALID	(E_BASE | 2)
#define	E_REMOVE	(E_BASE | 3)
#define	E_TOO_MUCH	(E_BASE | 4)
#define	E_WRITE		(E_BASE | 5)
#define	E_RENAME	(E_BASE | 6)


/*
 * process new history item
 */
void
process_item(char *content, gint locked, gboolean exec);



/* ==========================================================================
 *                                                          HISTORY FUNCTIONS
 */

/*
 * autosave period
 */
extern int	autosave_period;

/*
 * confirm actions?
 */
extern int	confirm_exec;

/*
 *  Exec immediately when item is captured?
 */
extern int	exec_immediately;

/*
 * move supplied item to begin
 */
void
move_item_to_begin(HISTORY_ITEM *item);

/*
 * Execute an item.
 */
void
exec_item(char *content, ACTION *action);

/*
 * loads history from file
 */
int
history_load(gboolean dump_only);

/*
 * store history to file
 */
int
history_save();

/*
 * free history data
 */
void
history_free();

/*
 * autosave timer function
 */
gboolean
history_autosave();


/* ==========================================================================
 *                                                                    HOTKEYS
 */

/* hotkeys */
extern gchar		menukey_str[32];
extern guint		menukey;
extern gchar		prev_item_key_str[32];
extern gchar		exec_item_key_str[32];

/*
 *  Exec on hotkey?
 */
extern int	exec_hotkey;

/*
 * initialize hotkeys
 */
void
hotkeys_init();

/*
 * disable hotkeys
 */
void
hotkeys_done();



/* ==========================================================================
 *                                                                        GUI
 */

/* color of locked item */
extern gchar		locked_color_str[32];
extern GdkColor		locked_color;
extern GtkStyle		*style_locked,
			*style_normal;
extern gint		submenu_count;

/*
 *  Exec on middle click?
 */
extern int	exec_middleclick;

/* main window widget */
extern GtkWidget	*main_window;

/* dock icon widget */
extern GtkWidget	*dock_app;

/* clipboard history menu */
extern GtkWidget	*menu_hist;
extern GtkWidget	*menu_title;

/* application menu */
extern GtkWidget	*menu_app;
extern GtkWidget	*menu_app_clip_ignore;
extern GtkWidget	*menu_app_clip_lock;
extern GtkWidget	*menu_app_exit;
extern GtkWidget	*menu_app_save;

/* button */
extern GtkWidget	*button;

/* pixmap */
extern GtkWidget	*pixmap;
extern GdkPixmap	*icon;
extern GdkBitmap	*icon_mask;
extern GdkBitmap	*mask;


/*
 * dock button click response
 */
gboolean
button_press(GtkWidget *widget, GdkEvent *event, gpointer data);


/*
 * checks, if there is already such item in menu,
 * in which case it moves it to the begining
 */
HISTORY_ITEM *
menu_item_exists(gchar *content, GtkWidget *submenu);

/*
 * add new item to menu
 */
HISTORY_ITEM *
menu_item_add(gchar *content, gint locked, GtkWidget *target_menu);


/*
 * application main menu handler
 */
gboolean
menu_app_item_click(GtkWidget *menuitem, gpointer data);


/*
 * open dialog with specified message andbuttons
 * and return number of button pressed
 */
gint
show_message(gchar *message, char *title,
		char *b1_text, char *b2_text, char *b3_text);



/* ==========================================================================
 *                                                                  UTILITIES
 */
gchar *
from_utf8(gchar *string);

#endif
