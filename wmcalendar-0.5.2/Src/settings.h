#define WMCALENDAR_VERSION "0.5.1"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>



char    rcfile[250];       /* location of settings file */
char    icsfile[250];      /* location of icalendar file */
char    application[250];  /* command for starting external application */
int     start_of_week; /* defines the first day of week */
char*   daystr[8];
int     appicon;       /* sets the application icon evol(0) moz(1) other(2)*/
int     lang;          /* defines the language 0:english(default) 1:farsi */
int     debug;         /* debug mode off(0) or on(1) */


void destroy (GtkWidget * widget, gpointer data);
void getSettings();
void writeSettings();
void openSettings();
void changeFilename();
void setFirstDay(GtkWidget *widget, GtkWidget *combo );
static void file_ok_sel( GtkWidget        *w,
                         GtkFileSelection *fs );
void set_lang(int language);
void setAppicon(int app);
void enter_callback( GtkWidget *widget, GtkWidget *entry );
int getAppicon();
int get_start_of_week();
int get_lang();
int get_debug();
void set_debug(int deb);
char* getVersion();
const char* get_application();
const char* get_icsfile();
