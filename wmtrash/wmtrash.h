/*
 * this file is part of the "wmtrash" project
 * Copyright (C) 2004 by Jean Philippe GUILLEMIN <jp.guillemin@free.fr>
 * license: This software is under GPL license
 * rev: 0.2
 */



#ifndef __CONFPATH
#define __CONFPATH "/etc/wmtrash/"
#endif

#ifndef __CONFFILE
#define __CONFFILE "default.cf"
#endif

#ifndef __ICONPATH
#define __ICONPATH "/usr/share/wmtrash/"
#endif

#define SHORT_STRING 64
#define MEDIUM_STRING 128
#define BIG_STRING 256
#define LONG_STRING 1024
#define EXTRALONG_STRING 4096

// Prototypes ************************************************************

struct wmtrash {
	char configfile[MEDIUM_STRING];
	char param_fm[MEDIUM_STRING];
	char fm[BIG_STRING];
	char param_img[MEDIUM_STRING];
	char imagefile[BIG_STRING];
	char param_path[MEDIUM_STRING];
	char path[BIG_STRING];
};

GtkWidget * build_dockapp(char *configfile);

GtkWidget *create_main_dockapp_icon_window
		(GtkWidget *main_window,
		unsigned int size);

void launchonclick
		(GtkWidget *event_box,
		GdkEventButton *event,
		struct wmtrash *data);

int parse_from_config
		(char *filename,
		char *param,
		char *value);

int replace_value_in_config
		(char *filename,
		char *param,
		char *new_value);

void enter_callback1(
		GtkWidget *entry1,
		struct wmtrash *data);

void enter_callback2(
		GtkWidget *entry1,
		struct wmtrash *data);

void close_callback(
		GtkWidget *entry,
		struct wmtrash *data);

int usage(char *homecf, char *defaultcf);

int fcopy(	char *sfile,
	char *dfile);


