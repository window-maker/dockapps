/*
 * $Id: wmstickynotes.h 10 2009-02-20 23:50:45Z hnc $
 *
 * Copyright (C) 2009 Heath Caldwell <hncaldwell@gmail.com>
 *
 */

#ifndef WMSTICKYNOTES_H
#define WMSTICKYNOTES_H

typedef struct {
	char *name;
	char *top;
	char *background;
} ColorScheme;

typedef struct {
	long int id;
	int x;
	int y;
	int width;
	int height;
	ColorScheme *scheme;
	GtkWidget *window;
	GtkWidget *text_widget;
	GtkWidget *top_bar_box;
	GtkWidget *delete_button_box;
	GtkWidget *resize_button_box;
} Note;

/* The default directory under $HOME in which to store notes */
const char *default_wmstickynotes_dir = ".wmstickynotes";

const num_color_schemes = 8;
ColorScheme color_schemes[] = {
	{"Yellow",	"#ffff00",	"#ffff88"},
	{"Green",	"#66ff00",	"#d0f0c0"},
	{"Orange",	"#ff7f00",	"#ffe5b4"},
	{"Pink",	"#ff007f",	"#ffc0cb"},
	{"Blue",	"#0000ff",	"#ccccff"},
	{"Purple",	"#4b0082",	"#c8a2c8"},
	{"Brown",	"#964b00",	"#f0dc82"},
	{"White",	"#aaaaaa",	"#ffffff"}};

void delete_note(GtkWidget *widget, Note *note);
void save_note(GtkWidget *widget, Note *note);
gboolean note_configure_event(GtkWidget *window, GdkEventConfigure *event, Note *note);
void bar_pressed(GtkWidget *widget, GdkEventButton *event, Note *note);
void resize_button_pressed(GtkWidget *widget, GdkEventButton *event, Note *note);
void delete_button_pressed(GtkWidget *widget, GdkEventButton *event, GtkWidget *window);
void create_note(Note *old_note, ColorScheme *scheme);
void new_note_from_menu(GtkMenuItem *menuitem, ColorScheme *scheme);
void read_old_notes();
void populate_note_popup(GtkTextView *entry, GtkMenu *menu, Note *note);
void set_current_note_color(GtkMenuItem *menuitem, ColorScheme *scheme);
void set_note_color(Note *note, ColorScheme *scheme);
void main_button_pressed(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void usage();

#endif /* WMSTICKYNOTES_H */
