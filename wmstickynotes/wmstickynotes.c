/*
 * $Id: wmstickynotes.c 11 2009-02-21 04:11:47Z hnc $
 *
 * Copyright (C) 2009 Heath Caldwell <hncaldwell@gmail.com>
 *
 */

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <getopt.h>

#include "wmstickynotes.h"
#include "wmstickynotes.xpm"
#include "delete_button.xpm"
#include "resize_button.xpm"
#include "config.h"

#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

GdkColormap *colormap;

/* The highest note id used so far (this is used when making a new note so
 * that no ids are clobbered */
long int highest_note_id = 0;

/* The current note that the popup menu was shown for */
Note *current_note;



void usage()
{
	printf("Usage: wmstickynotes [options]\n");
	printf("\toptions:\n");
	printf("\t-d [dir], --directory=[dir]\tSet directory in which to store notes\n");
	printf("\t\t\t\t\tDefaults to $HOME/%s\n", default_wmstickynotes_dir);
	printf("\t-v, --version\tPrint version information\n");
	printf("\t-h, --help\tPrint usage\n");
}

int main(int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *box;
	GdkColor color;
	XWMHints mywmhints;
	GtkWidget *main_button;
	GdkPixmap *main_button_pixmap;
	GdkBitmap *main_button_mask;
	GtkWidget *main_button_box;
	GtkWidget *color_menu;
	GtkWidget *item;
	GtkWidget *label;
	GtkWidget *color_box;
	GtkWidget *hbox;
	GdkColor gcolor;
	char *wmstickynotes_dir = NULL;
	gboolean use_default_dir = TRUE;
	int option_index = 0;
	int i = 0;

	struct option long_options[] = {
		{"directory", required_argument, 0, 'd'},
		{"version", no_argument, 0, 'v'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}};

	for(
		i = getopt_long(argc, argv, "d:vh", long_options, &option_index);
		i >= 0;
		i = getopt_long(argc, argv, "d:vh", long_options, &option_index)
	) {
		switch(i) {
			case 'd':
				wmstickynotes_dir = optarg;
				use_default_dir = FALSE;
				break;
			case 'v':
				printf("%s\n", PACKAGE_STRING);
				printf("Copyright (C) 2009  %s\n", PACKAGE_BUGREPORT);
				return 0;
			case 'h':
				usage();
				return 0;
			default:
				usage();
				return 1;
		}
	}

	umask(077);

	if(use_default_dir) {
		wmstickynotes_dir = calloc(
			strlen(default_wmstickynotes_dir) +
			strlen(getenv("HOME")) + 2, sizeof(char));
		strcpy(wmstickynotes_dir, getenv("HOME"));
		strcat(wmstickynotes_dir, "/");
		strcat(wmstickynotes_dir, default_wmstickynotes_dir);
	}

	if(chdir(wmstickynotes_dir)) {
		if(errno == ENOENT) {
			if(mkdir(wmstickynotes_dir, 0777)) {
				fprintf(stderr, "Couldn't make directory: %s\n", wmstickynotes_dir);
				exit(1);
			}
			if(chdir(wmstickynotes_dir)) {
				fprintf(stderr, "Couldn't change to directory: %s\n", wmstickynotes_dir);
				exit(1);
			}
		} else {
			fprintf(stderr, "Couldn't change to directory: %s\n", wmstickynotes_dir);
			exit(1);
		}
	}

	if(use_default_dir) free(wmstickynotes_dir);

	gtk_init(&argc, &argv);

	colormap = gdk_colormap_new(gdk_visual_get_system(), TRUE);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 64, 64);

	box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER (window), box);

	gdk_color_parse ("#fafafa", &color);
	gtk_widget_modify_bg(box, GTK_STATE_NORMAL, &color);

	main_button_pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &main_button_mask, NULL, wmstickynotes_xpm);
	main_button = gtk_image_new_from_pixmap(main_button_pixmap, main_button_mask);
	main_button_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(main_button_box), main_button);
	gtk_container_add(GTK_CONTAINER(box), main_button_box);

	color_menu = gtk_menu_new();

	for(i=0; i < num_color_schemes; i++) {
		item = gtk_menu_item_new();
		label = gtk_label_new(color_schemes[i].name);
		color_box = gtk_event_box_new();
		gtk_widget_set_size_request(color_box, 15, -1);
		hbox = gtk_hbox_new(FALSE, 4);

		gdk_color_parse(color_schemes[i].top, &gcolor);
		gtk_widget_modify_bg(color_box, GTK_STATE_NORMAL, &gcolor);
		gtk_widget_modify_bg(color_box, GTK_STATE_PRELIGHT, &gcolor);

		gtk_container_add(GTK_CONTAINER(item), hbox);
		gtk_box_pack_start(GTK_BOX(hbox), color_box, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

		gtk_menu_shell_append(GTK_MENU_SHELL(color_menu), item);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(new_note_from_menu), &color_schemes[i]);
	}

	gtk_widget_show_all(GTK_WIDGET(color_menu));
	gtk_widget_show_all(window);

	mywmhints.initial_state = WithdrawnState;
	mywmhints.icon_window = GDK_WINDOW_XWINDOW(box->window);
	mywmhints.icon_x = 0;
	mywmhints.icon_y = 0;
	mywmhints.window_group = GDK_WINDOW_XWINDOW(window->window);
	mywmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;

	XSetWMHints(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(window->window), &mywmhints);

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(main_button_box), "button-press-event", G_CALLBACK(main_button_pressed), color_menu);

	read_old_notes();
	gtk_main();

	return 0;
}

void delete_note(GtkWidget *widget, Note *note)
{
	char *filename;
	asprintf(&filename, "%d", note->id);
	unlink(filename);
	free(note);
}

void save_note(GtkWidget *widget, Note *note)
{
	FILE *file;
	char *filename;
	GtkTextBuffer *text_buffer;
	GtkTextIter start;
	GtkTextIter end;
	gchar *text;

	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->text_widget));
	gtk_text_buffer_get_start_iter(text_buffer, &start);
	gtk_text_buffer_get_end_iter(text_buffer, &end);

	text = gtk_text_buffer_get_text(text_buffer, &start, &end, FALSE);

	asprintf(&filename, "%d", note->id);
	file = fopen(filename, "w");
	free(filename);

	fprintf(
		file, "%d,%d,%d,%d,%d,%d,%s\n%s",
		note->x, note->y, note->width, note->height, 0, 0, note->scheme->name, text);
	fclose(file);

	g_free(text);
}

gboolean note_configure_event(GtkWidget *window, GdkEventConfigure *event, Note *note)
{
	note->x = event->x;
	note->y = event->y;
	note->width = event->width;
	note->height = event->height;
	save_note(window, note);
	return FALSE;
}

void bar_pressed(GtkWidget *widget, GdkEventButton *event, Note *note)
{
	gtk_window_begin_move_drag(GTK_WINDOW(note->window), event->button, event->x_root, event->y_root, event->time);
}

void resize_button_pressed(GtkWidget *widget, GdkEventButton *event, Note *note)
{
	gtk_window_begin_resize_drag(GTK_WINDOW(note->window), GDK_WINDOW_EDGE_SOUTH_EAST, event->button, event->x_root, event->y_root, event->time);
}

void delete_button_pressed(GtkWidget *widget, GdkEventButton *event, GtkWidget *window)
{
	if(event->button != 1) return;

	gtk_widget_destroy(window);
}

void main_button_pressed(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	if(event->button == 1) {
		create_note(NULL, &color_schemes[0]);
	} else if(event->button == 3) {
		gtk_menu_popup(GTK_MENU(user_data), NULL, NULL, NULL, NULL, event->button, event->time);
	}
}

void create_note(Note *old_note, ColorScheme *scheme)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *top_hbox;
	GtkWidget *mid_hbox;
	GtkWidget *bottom_bar;
	GtkWidget *bottom_hbox;
	GtkWidget *top_bar;
	GtkWidget *delete_button;
	GdkPixmap *delete_button_pixmap;
	GdkBitmap *delete_button_mask;
	GtkWidget *resize_button;
	GdkPixmap *resize_button_pixmap;
	GdkBitmap *resize_button_mask;
	GtkTextBuffer *text_buffer;

	Note *note;

	note = old_note ? old_note : malloc(sizeof(Note));

	if(!old_note) {
		highest_note_id++;
		note->id = highest_note_id;
		note->scheme = scheme;
	}

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
	gtk_window_set_default_size(GTK_WINDOW(window), 150, 150);

	if(!old_note) {
		note->text_widget = gtk_text_view_new_with_buffer(NULL);
	}
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(note->text_widget));

	note->window = window;

	vbox = gtk_vbox_new(FALSE, 0);
	top_hbox = gtk_hbox_new(FALSE, 0);
	mid_hbox = gtk_hbox_new(FALSE, 0);
	bottom_hbox = gtk_hbox_new(FALSE, 0);
	top_bar = gtk_label_new("");
	note->top_bar_box = gtk_event_box_new();
	gtk_widget_set_size_request(top_bar, -1, 10);
	bottom_bar = gtk_label_new("");
	gtk_widget_set_size_request(bottom_bar, -1, 8);

	delete_button_pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &delete_button_mask, NULL, delete_button_xpm);
	delete_button = gtk_image_new_from_pixmap(delete_button_pixmap, delete_button_mask);
	note->delete_button_box = gtk_event_box_new();
	gtk_widget_set_size_request(note->delete_button_box, 10, 10);

	resize_button_pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &resize_button_mask, NULL, resize_button_xpm);
	resize_button = gtk_image_new_from_pixmap(resize_button_pixmap, resize_button_mask);
	note->resize_button_box = gtk_event_box_new();

	set_note_color(note, note->scheme);

	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_container_add(GTK_CONTAINER(note->top_bar_box), top_bar);
	gtk_container_add(GTK_CONTAINER(note->delete_button_box), delete_button);
	gtk_container_add(GTK_CONTAINER(note->resize_button_box), resize_button);
	gtk_box_pack_start(GTK_BOX(top_hbox), note->top_bar_box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(top_hbox), note->delete_button_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mid_hbox), note->text_widget, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(bottom_hbox), bottom_bar, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(bottom_hbox), note->resize_button_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), top_hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), mid_hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), bottom_hbox, FALSE, FALSE, 0);

	gtk_widget_show_all(window);

	if(old_note) {
		gtk_window_resize(GTK_WINDOW(window), old_note->width, old_note->height);
		gtk_window_move(GTK_WINDOW(window), old_note->x, old_note->y);
	} else {
		gtk_window_get_position(GTK_WINDOW(window), &(note->x), &(note->y));
		gtk_window_get_size(GTK_WINDOW(window), &(note->width), &(note->height));
	}

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(delete_note), note);
	g_signal_connect(G_OBJECT(window), "configure-event", G_CALLBACK(note_configure_event), note);
	g_signal_connect(G_OBJECT(note->delete_button_box), "button-press-event", G_CALLBACK(delete_button_pressed), window);
	g_signal_connect(G_OBJECT(note->resize_button_box), "button-press-event", G_CALLBACK(resize_button_pressed), note);
	g_signal_connect(G_OBJECT(text_buffer), "changed", G_CALLBACK(save_note), note);
	g_signal_connect(G_OBJECT(note->top_bar_box), "button-press-event", G_CALLBACK(bar_pressed), note);
	g_signal_connect(G_OBJECT(note->text_widget), "populate-popup", G_CALLBACK(populate_note_popup), note);
}

void read_old_notes()
{
	Note *note;
	GtkTextBuffer *text_buffer;
	GtkTextIter iter;
	DIR *dir = opendir(".");
	FILE *file;
	struct dirent *entry;
	int reserved1;
	int reserved2;
	int i;
	char buffer[256];

	rewinddir(dir);
	while((entry = readdir(dir)) != NULL) {
		/* Check if it is a valid note name */
		for(i=0; entry->d_name[i]; i++) {
			if(entry->d_name[i] < '0' || entry->d_name[i] > '9') break;
		}
		if(i < strlen(entry->d_name)) continue;

		file = fopen(entry->d_name, "r");
		note = malloc(sizeof(Note));

		note->id = atoi(entry->d_name);
		if(note->id > highest_note_id) highest_note_id = note->id;

		fscanf(file, "%d,%d,%d,%d,%d,%d,",
			&(note->x), &(note->y), &(note->width), &(note->height),
			&reserved1, &reserved2);

		/* Get color name */
		fgets(buffer, 256, file);
		/* Replace the newline with a null char */
		buffer[strlen(buffer) - 1] = '\0';

		for(i=num_color_schemes; i > 0; i--) {
			if(!strcmp(color_schemes[i].name, buffer)) break;
		}
		note->scheme = &color_schemes[i];

		text_buffer = gtk_text_buffer_new(NULL);
		while(fgets(buffer, 256, file)) {
			gtk_text_buffer_get_end_iter(text_buffer, &iter);
			gtk_text_buffer_insert(text_buffer, &iter, buffer, -1);
		}

		note->text_widget = gtk_text_view_new_with_buffer(text_buffer);

		create_note(note, note->scheme);

		fclose(file);
	}

	closedir(dir);
}

void populate_note_popup(GtkTextView *entry, GtkMenu *menu, Note *note)
{
	GtkWidget *color_menu;
	GtkWidget *color_item;
	GtkWidget *item;
	GtkWidget *label;
	GtkWidget *color_box;
	GtkWidget *hbox;
	GdkColor gcolor;
	int i;

	color_menu = gtk_menu_new();
	color_item = gtk_menu_item_new_with_label("Color");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(color_item), color_menu);
	gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), color_item);

	current_note = note;
	for(i=0; i < num_color_schemes; i++) {
		item = gtk_menu_item_new();
		label = gtk_label_new(color_schemes[i].name);
		color_box = gtk_event_box_new();
		gtk_widget_set_size_request(color_box, 15, -1);
		hbox = gtk_hbox_new(FALSE, 4);

		gdk_color_parse(color_schemes[i].top, &gcolor);
		gtk_widget_modify_bg(color_box, GTK_STATE_NORMAL, &gcolor);
		gtk_widget_modify_bg(color_box, GTK_STATE_PRELIGHT, &gcolor);

		gtk_container_add(GTK_CONTAINER(item), hbox);
		gtk_box_pack_start(GTK_BOX(hbox), color_box, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

		gtk_menu_shell_append(GTK_MENU_SHELL(color_menu), item);
		g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(set_current_note_color), &color_schemes[i]);
	}

	gtk_widget_show_all(GTK_WIDGET(menu));
}

void set_current_note_color(GtkMenuItem *menuitem, ColorScheme *scheme)
{
	set_note_color(current_note, scheme);
	save_note(NULL, current_note);
}

void new_note_from_menu(GtkMenuItem *menuitem, ColorScheme *scheme)
{
	create_note(NULL, scheme);
}

void set_note_color(Note *note, ColorScheme *scheme)
{
	GdkColor gcolor;

	note->scheme = scheme;

	gdk_color_parse(scheme->top, &gcolor);
	gtk_widget_modify_bg(note->top_bar_box, GTK_STATE_NORMAL, &gcolor);
	gtk_widget_modify_bg(note->delete_button_box, GTK_STATE_NORMAL, &gcolor);

	gdk_color_parse(scheme->background, &gcolor);
	gtk_widget_modify_base(note->text_widget, GTK_STATE_NORMAL, &gcolor);
	gtk_widget_modify_bg(note->window, GTK_STATE_NORMAL, &gcolor);
	gtk_widget_modify_bg(note->resize_button_box, GTK_STATE_NORMAL, &gcolor);
}
