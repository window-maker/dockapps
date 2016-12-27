/*
 * this file is part of the "wmtrash" project
 * Copyright (C) 2004 by Jean Philippe GUILLEMIN <jp.guillemin@free.fr>
 * license: This software is under GPL license
 * rev: 0.2
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sysexits.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "wmtrash.h"


// Start main *******************************************************************

int main( int argc, char *argv[] ) {

// Here we parse command line args and configfile *******************************************************************
	GtkWidget *dockapp;
	static char *defaultcf;
	static char *configfile;
	static char *homecf;
	static char *homedir;
	homecf = malloc(MEDIUM_STRING);
	configfile = malloc(MEDIUM_STRING);
	
	defaultcf = malloc(MEDIUM_STRING);
	strncpy(defaultcf, __CONFPATH, MEDIUM_STRING);	
	strcat(defaultcf, "/");	
	strcat(defaultcf, __CONFFILE);	
	
	homedir = malloc(SHORT_STRING);
	homedir = getenv("HOME");
	
	strncpy(homecf, homedir, SHORT_STRING);
	strcat(homecf, "/");	
	strcat(homecf, ".wmtrash.cf");

	int test1, test2;
	if ((test1 = access(homecf, F_OK)) == -1){
		if ((test2 = fcopy(defaultcf, homecf)) == EXIT_FAILURE){
			fprintf (stderr,"Error creating config file %s !\n",homecf);
		}
	}
	
	int i;
	
	if (argc < 2){
		configfile = homecf;
	}else{
		while ((i = getopt(argc, argv, "hc:")) != EOF){
			switch (i){
				case 'c': /* config file */
					strncpy(configfile, optarg, MEDIUM_STRING);
					break;
				case 'h': usage(homecf, defaultcf); exit (EXIT_SUCCESS);
			}
		}
	}
	
	
	gtk_init(&argc, &argv);
	dockapp = (GtkWidget *) build_dockapp(configfile);
	gtk_widget_show_all (dockapp);
	gtk_main ();
	return(0);
} // end main


GtkWidget * build_dockapp(char *configfile) {
	static GtkWidget *mainwin;
	static GtkWidget *mainbox;
	static GtkWidget *box;
	static GtkWidget *pixmap;
	static GdkBitmap *mask;
	static GtkWidget *pixmap_widget;
	
	static struct wmtrash *wmtrash;
	wmtrash = malloc(sizeof(struct wmtrash));
	memset(wmtrash, 0, sizeof(struct wmtrash));


	char *image_path_file;
	
	strncpy (wmtrash->param_img, "trashicon", MEDIUM_STRING);
	strncpy (wmtrash->param_fm, "filemanager", MEDIUM_STRING);
	strncpy (wmtrash->param_path, "trashpath", MEDIUM_STRING);
	strncpy (wmtrash->configfile, configfile, MEDIUM_STRING);
	
	image_path_file = malloc(LONG_STRING);

	parse_from_config(wmtrash->configfile, wmtrash->param_img, wmtrash->imagefile);
	sprintf(image_path_file, "%s/%s", __ICONPATH, wmtrash->imagefile);	
	parse_from_config(wmtrash->configfile, wmtrash->param_fm, wmtrash->fm);
	parse_from_config(wmtrash->configfile, wmtrash->param_path, wmtrash->path);
		
	// GTK stuff *******************************************************************
	gtk_widget_destroy(mainwin);
	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize(mainwin);
	mainbox = create_main_dockapp_icon_window(mainwin, 52);
	
	box = gtk_event_box_new();
	pixmap = (gpointer) gdk_pixmap_create_from_xpm (mainwin->window, &mask,
               	NULL, image_path_file);
        pixmap_widget = gtk_pixmap_new((gpointer) pixmap, mask);
	gtk_container_add(GTK_CONTAINER(box), pixmap_widget);

	
	gtk_container_add (GTK_CONTAINER (mainbox), box);
	
	gtk_signal_connect (GTK_OBJECT(box), "button_press_event", GTK_SIGNAL_FUNC(launchonclick), wmtrash);
	
	free(image_path_file);
	return mainwin;
}


// Function create_main_dockapp_icon_window (c) 2000, Alexey Vyskubov <alexey@pepper.spb.ru>

GtkWidget *create_main_dockapp_icon_window
		(GtkWidget *mw,
		unsigned int s) 
{
	GtkWidget *dockappbox; // This will become icon box 
	Window xmw;
	XWMHints *wm_hints;
	
	xmw = GDK_WINDOW_XWINDOW(mw->window);
	
	dockappbox = gtk_event_box_new();
	gtk_widget_set_usize(dockappbox, s, s);
	gtk_container_add (GTK_CONTAINER (mw), dockappbox);
	gtk_widget_realize(dockappbox);
	
	// Time for game with Xlib 
	wm_hints = XAllocWMHints();
	wm_hints->window_group = xmw;
	wm_hints->icon_window = GDK_WINDOW_XWINDOW(dockappbox->window);
	wm_hints->icon_x = 0;
	wm_hints->icon_y = 0; 
	wm_hints->initial_state = WithdrawnState;
	wm_hints->flags = StateHint |
		IconPositionHint |
		WindowGroupHint |
		IconWindowHint;
	
	XSetWMHints(GDK_DISPLAY(), xmw, wm_hints);
	
	XFree(wm_hints);
	
	return dockappbox;
} // end create_main_dockapp_icon_window


/* the function to execute command when left button is clicked *************************************
	or change the command and pixmap icon when right button is 2-clicked */

void launchonclick
		(GtkWidget *event_box, 
		GdkEventButton *event,
		struct wmtrash *data)
{
		char *cmd; 
		cmd=malloc(BIG_STRING); 
		memset(cmd, 0, BIG_STRING);

		/* left button of mouse is clicked */
        if(event->button == 1){
			strcat (cmd, data->fm);
			strcat (cmd, " ");
			strcat (cmd, data->path);
			strcat (cmd, "&");
			//fprintf(stderr, cmd);  //debug
			int output  = system(cmd);
			if (output){
				fprintf(stderr, "error launching command in function \"launchonclick\"\n");
			}
			return;
	}
	if((event->button == 3)&&(event->type == GDK_2BUTTON_PRESS)) {					
			strcat (cmd, "rm -rf");
			strcat (cmd, " ");
			strcat (cmd, data->path);
			strcat (cmd, "/* ");
			strcat (cmd, data->path);
			strcat (cmd, "/.*");
			//fprintf(stderr, cmd);  //debug
			int output  = system(cmd);
			if (output){
				fprintf(stderr, "error launching command in function \"launchonclick\"\n");
			}
			return;
       }
	   
	   /* right button of mouse is 2-clicked */
        if((event->button == 2)&&(event->type == GDK_2BUTTON_PRESS)){
			
				/* create a new text entry window for entering the new command */
				GtkWidget *dialog, *table, *vbox;
				GtkWidget *entry1, *entry2, *button;
				// window parameters
				dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
				gtk_window_set_title (GTK_WINDOW (dialog), "Change command & pixmap");
			    	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
				
				
				table = gtk_table_new(2,2, TRUE);
				gtk_table_set_col_spacings(GTK_TABLE (table), 5);
				gtk_table_set_row_spacings(GTK_TABLE (table), 5);
				
				vbox = gtk_vbox_new (FALSE, 0);
				
				// entry box parameters
				entry1 = gtk_entry_new ();
				entry2 = gtk_entry_new ();
				gtk_entry_set_max_length (GTK_ENTRY (entry1), 120);
				gtk_entry_set_max_length (GTK_ENTRY (entry2), 120);
				
				gtk_signal_connect (GTK_OBJECT (entry1), "changed", GTK_SIGNAL_FUNC (enter_callback1), data);
				gtk_signal_connect (GTK_OBJECT (entry2), "changed", GTK_SIGNAL_FUNC (enter_callback2), data);
				
				gtk_entry_set_text (GTK_ENTRY (entry1), data->fm);
				gtk_entry_set_text (GTK_ENTRY (entry2), data->path);
				gtk_editable_set_editable (GTK_EDITABLE (entry1), TRUE);
				gtk_entry_set_visibility (GTK_ENTRY (entry1), TRUE);
				gtk_editable_set_editable (GTK_EDITABLE (entry2), TRUE);
				gtk_entry_set_visibility (GTK_ENTRY (entry2), TRUE);
				
				// button parameters
				button = gtk_button_new_with_label("OK");
				//label = gtk_label_new ("OK");
				gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (close_callback), data);
				
				gtk_container_add (GTK_CONTAINER (vbox), entry1);
				gtk_container_add (GTK_CONTAINER (vbox), entry2);
				
				gtk_table_attach_defaults(GTK_TABLE(table), vbox, 0,1, 0,2);
				gtk_table_attach_defaults(GTK_TABLE(table), button, 1,2, 0,2);

				gtk_container_add (GTK_CONTAINER (dialog), table);
				gtk_widget_show_all(dialog);

				return;
       	}
	free(cmd);
} // end launchonclick

/* some callback to handle change of values in entry boxes************************* */
void enter_callback1( 
			GtkWidget *entry,
			struct wmtrash *data){
  	char *entry_text;
	entry_text = malloc(BIG_STRING); 
	memset(entry_text, 0, BIG_STRING);
  	strncpy (entry_text, gtk_entry_get_text (GTK_ENTRY (entry)), BIG_STRING);
	memset(data->fm, 0, BIG_STRING);
	strncpy (data->fm, entry_text, BIG_STRING);
	
}
void enter_callback2( 
			GtkWidget *entry,
			struct wmtrash *data){
  	char *entry_text;
	entry_text = malloc(BIG_STRING); 
	memset(entry_text, 0, BIG_STRING);
  	strncpy (entry_text, gtk_entry_get_text (GTK_ENTRY (entry)), BIG_STRING);
	memset(data->path, 0, BIG_STRING);
	strncpy (data->path, entry_text, BIG_STRING);	
}

void close_callback( 
			GtkWidget *entry,
			struct wmtrash *data){
	
	GtkWidget *parent;
	GtkWidget *dockapp; 
  	replace_value_in_config(data->configfile, data->param_fm, data->fm);
	replace_value_in_config(data->configfile, data->param_path, data->path);
	parent = gtk_widget_get_ancestor(entry, GTK_TYPE_WINDOW); 
	gtk_widget_destroy(parent);	// close the dialog box
	dockapp = (GtkWidget *) build_dockapp(data->configfile);	// will redraw icons
	gtk_widget_show_all (dockapp);
}


// the function to parse the config file for parameters *********************************************
int parse_from_config
		(char *filename, 
		char *param, 
		char *value)
{
	int c = 0;
	FILE *file;
	char 	*sep, 
		*buffer, 
		*line, 
		*val;
	line = malloc(LONG_STRING); 
	buffer = malloc(MEDIUM_STRING); 
	sep = malloc(SHORT_STRING); 
	val = malloc(MEDIUM_STRING);
	
	if((file=fopen(filename, "rb")) == 0) {
		fprintf (stderr,"Error opening file %s !\n",filename);
		exit(EXIT_FAILURE);
	}
	
	while (fgets (line, LONG_STRING, file) !=NULL) {
		if (strstr (line, "[") !=0) continue;
		if (strstr (line, "#") !=0) continue;
		memset(val, 0, MEDIUM_STRING);
		c = sscanf (line, "%s %s %s", buffer, sep, val); 

		if (strncmp (param, buffer, MEDIUM_STRING) == 0) {
			memset(value, 0, BIG_STRING);

			strncpy(value,line + strlen(buffer) + strlen(sep) +2, BIG_STRING);
			//fprintf(stderr,"%s\n", value);
			char *p = strchr (value, '\n');
   			if (p){
				*p = 0;
			}
			fclose (file);
			return(EXIT_SUCCESS);
		}
	memset(line, 0, LONG_STRING);	
	}
	fclose (file);
	free(sep); 
	free(buffer); 
	free(line); 
	free(val);
	return(EXIT_FAILURE);
} // end parse_from_config


// the function to parse the config file to replace the value of a given parameter ****************************
int replace_value_in_config
		(char *filename, 
		char *param, 
		char *new_value)
{
	int c = 0;
	FILE *file;
	char 	*bigbuffer, 
		*sep, 
		*buffer, 
		*line, 
		*old_value;
	bigbuffer = malloc(EXTRALONG_STRING); 
	line = malloc(LONG_STRING); 
	buffer = malloc(MEDIUM_STRING); 
	sep = malloc(MEDIUM_STRING); 
	old_value = malloc(BIG_STRING);
	
	if((file=fopen(filename, "rb")) == 0) {
		fprintf (stderr,"Error opening file %s !\n",filename);
		exit(EXIT_FAILURE);
	}
	
	memset(bigbuffer, 0, EXTRALONG_STRING);
	while (fgets (line, LONG_STRING, file) !=NULL) {
		memset(buffer, 0, MEDIUM_STRING);
		c = sscanf (line, "%s %s %s\n", buffer, sep, old_value); 

		if (strncmp (param, buffer, MEDIUM_STRING) == 0) {
			memset(line, 0, LONG_STRING);
			sprintf(line, "%s %s %s\n", buffer, sep, new_value);

		}
		strncat (bigbuffer, line, LONG_STRING); 
		memset(line, 0, LONG_STRING);
	}
	fclose (file);

	if((file=fopen(filename, "w+")) == 0) {
		fprintf (stderr,"Error opening file %s !\n",filename);
		exit(EXIT_FAILURE);
	}
	fprintf(file,bigbuffer);
	fclose (file);
	memset(bigbuffer, 0, EXTRALONG_STRING);
	
	free(bigbuffer); 
	free(sep); 
	free(buffer); 
	free(line); 
	free(old_value);
return(EXIT_SUCCESS);
} // end replace_value_in_config *************************************************************************






int usage(char *homecf, char *defaultcf) {
		fprintf(stdout,"\nwmtrash v0.2 : Desktop trash dockapp\n");
	fprintf(stdout, "Usage : \n");
	fprintf(stdout,"- single LEFT click : browse trash\n");
	fprintf(stdout,"- double MIDDLE click : change location & filemanager\n");
	fprintf(stdout,"- double RIGHT click : empty trash\n");
	fprintf(stdout, "---------------\n");
	fprintf(stdout, "How to start it : \n"
					"\t wmtrash [-c path_to_configfile] use a custom config\n"
					"\t wmtrash [-h] display this help\n");
	fprintf(stdout, "---------------\n");
	fprintf(stdout,"home config file is %s\n", homecf);
	fprintf(stdout,"default config file is %s\n",defaultcf);
	return(EXIT_SUCCESS);
} // end usage
//*************************************************************************



// this one just copy sfile into dfile :)
int fcopy(	char *sfile, 
	char *dfile){
int c;			/* Character read/written between files */
FILE *IPFile;			/* Pointer to the I/P file. FILE is a 
				   structure  defined in <stdio.h>	*/
FILE *OPFile;

				/* Open the file - */
if ((IPFile = fopen(sfile,"r")) == 0) {
	fprintf (stderr,"Error opening file %s !\n",sfile);
	return (EXIT_FAILURE);
}
if ((OPFile = fopen(dfile,"w")) == 0) {
	fprintf (stderr,"Error opening file %s !\n",dfile);
	return (EXIT_FAILURE);
}
				/* Read one character at a time, checking 
				   for the End of File. EOF is defined 
				   in <stdio.h>  as -1 			*/
while ((c = fgetc(IPFile)) != EOF){
   fputc(c, OPFile);		/* O/P the character 			*/
   }

fclose(IPFile);		/* Close the files.			*/
fclose(OPFile);		/* Close the files.			*/
return (EXIT_SUCCESS);
} // end fcopy

