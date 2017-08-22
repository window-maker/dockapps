
#ifndef LOGLIST_H
#define LOGLIST_H

#include <gtk/gtk.h>

void create_loglist(void);
void load_log(void);
void prepend_log_entry(char *);
void free_log(void);
void update_list(void);
void make_resume(void);
void add_hourly(long s,long e);
void add_weekly(long s,long e);

gint applet_kill (GtkWidget * widget, GdkEvent * event, gpointer data);
void applet_destroy (GtkWidget * widget, gpointer data);

void time_menu(GtkMenuItem *gmi,gpointer data);
void user_menu(GtkMenuItem *gmi,gpointer data);
void isp_menu(GtkMenuItem *gmi,gpointer data);
int already_exists(char *s,GList *pt);

gint hgra_expose(GtkWidget *,GdkEventExpose *,gpointer);
gint hgra_configure(GtkWidget *,GdkEventConfigure *,gpointer);
gint wgra_expose(GtkWidget *,GdkEventExpose *,gpointer);
gint wgra_configure(GtkWidget *,GdkEventConfigure *,gpointer);

struct logentry {
  time_t start;
  time_t end;
  int    status; /* 0=ok 1=error 2=crash */
  char   longname[128];
  char   shortname[16];
  char   phone[32];
  char   user[32];
};

#endif
