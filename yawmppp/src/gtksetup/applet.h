
#ifndef YAWMPPP_APPLET_H
#define YAWMPPP_APPLET_H

#include <gtk/gtk.h>
#include "isprc.h"

/* not less than the dock applet */
#define MAX_ISPS 40

#define ERR_MAX_ISPS   "You can't have more than 40 entries."

#define ERR_CANT_MKDIR "Unable to create directory ~/.yawmppp\n"\
                       "Current configuration could not be\n"\
		       "written."

#define ERR_CANT_WRITE "Unable to create configuration files\n"\
                       "in directory ~/.yawmppp. Current configuration\n"\
		       "may not have been only partially written."

#define ERR_DIDNT_WRITE "There were errors when saving the configuration.\n"\
		        "Do you still wish to close wmppp.pref and [maybe] lose\n"\
		        "current data ?"

#define INFO_CANT_APPLY_NOW "The dock applet will update itself only when\n"\
		            "the current connection is over."

#define ERR_CANT_SU "Script didn't run properly. Bad password ?"

#define MAX_EXPECT_PAIRS 16

struct PREF_ISP_INFO {
  char LongName[128];
  char ShortName[16];
  char Device[64];
  char Username[32];
  char Password[32];
  char Phone[32];
  char ModemInit1[256];
  char ModemInit2[256];
  char UserString[32];
  char PassString[32];
  char PulseDial[8];
  char ModemSpeed[16];

  /* appended in 1.1.0 */
  int  nExpectPairs;
  char s_expect[MAX_EXPECT_PAIRS][32];
  char s_send[MAX_EXPECT_PAIRS][32];

  struct ISP_PPP ppp; /* defined in ../isprc.h */

  /* appended in 1.1.2 */
  int nologin;
};

struct PREF_PPP_OPT {
  int defaultroute;
  int lock;
  int passive;
  int debug;
  int kdebug;
  int noauth;
  int noipdefault;
  int linectl;        /* 0=modem 1=local */
  int flowctl;        /* 0=crtscts 1=xonxoff 2=nocrtscts 3=DC */
  int mtu;
  int mru;
  int lcp[5];         /* in the same order as in the dialog */

  int chap;
  int pap;

  int usepeerdns;
};

void create_preferences_panel(void);
void clear_and_disable_right_pane(void);
void load_rc_entries(void);
void update_right_pane(void);
void commit_back_to_db(void);
void fill_list(void);
int  write_and_apply_data(void);
void check_client(void);
int file_exists(char *s);
void find_out_paths(void);
void test_set_path(char *,char *);

void xlate_ppp_to_pref(struct YAWMPPP_ISP_INFO *wii,
		       struct PREF_ISP_INFO  *pii);

GtkWidget *hlabel_new(char *);

GtkWidget *make_ppp_pane(void);
void get_ppp_pane(void);
void set_ppp_pane(void);
char *mk_ppp_string(struct PREF_PPP_OPT *modi,char *sep);
void read_ppp_options_from_rc(char *p);

char *wrapped_strtok(char *a,char *b);

gint applet_kill (GtkWidget * widget, GdkEvent * event, gpointer data);
void applet_destroy (GtkWidget * widget, gpointer data);
void applet_apply (GtkWidget * widget, gpointer data);
void applet_save_and_quit (GtkWidget * widget, gpointer data);

void list_select(GtkCList *cl,gint row,gint column,
		 GdkEventButton *geb,gpointer data);
void list_unselect(GtkCList *cl,gint row,gint column,
		   GdkEventButton *geb,gpointer data);

void list_moveup(GtkWidget *gw,gpointer data);
void list_movedown(GtkWidget *gw,gpointer data);
void list_remove(GtkWidget *gw,gpointer data);
void list_duplicate(GtkWidget *gw,gpointer data);
void list_add(GtkWidget *gw,gpointer data);

/* more settings dialog */

void pop_advanced(GtkWidget *gw,gpointer data);
void adv_ok (GtkWidget * widget, gpointer data);
void adv_cancel (GtkWidget * widget, gpointer data);
void adv_destroy (GtkWidget * widget, gpointer data);

void enable_local_ppp(gboolean e);
void ppp_override_toggle(GtkToggleButton *gtb,gpointer data);

/* more settings dialog */

void isp_rename(GtkEditable *ge,gpointer data);

void make_ppp_default(GtkWidget *w,gpointer data);

/* expect/send */

void pop_expect(GtkWidget *w,gpointer data);
void exp_ok (GtkWidget * widget, gpointer data);
void exp_cancel (GtkWidget * widget, gpointer data);
void exp_destroy (GtkWidget * widget, gpointer data);
void exp_update(void);
void exp_up(GtkWidget *widget,gpointer data);
void exp_down(GtkWidget *widget,gpointer data);
void exp_disable(void);
void exp_enable(void);
void exp_select(GtkCList *cl,gint row,gint column,
		GdkEventButton *geb,gpointer data);
void exp_unselect(GtkCList *cl,gint row,gint column,
		  GdkEventButton *geb,gpointer data);
void exp_add_delay(void);
void exp_remove(void);
void exp_add(void);
void expa_destroy (GtkWidget * widget, gpointer data);
void expa_ok (GtkWidget * widget, gpointer data);
void expa_cancel (GtkWidget * widget, gpointer data);

/* expect/send */

void extract_delimited_string(char *dest,char *src,int count,char delim,int max);

/* help */
void pop_help (GtkWidget * widget, gpointer data);
void help_dead (GtkWidget * widget, gpointer data);
void help_die (GtkWidget * widget, gpointer data);

void unman(char *s);
void add_man(char *manpage,GtkWidget *text,int index);
void add_docfile(char *source,GtkWidget *text,int index);


/* misc */
void run_as_root(char *what);
gint pwd_kill (GtkWidget * widget, GdkEvent * event, gpointer data);

void pwd_cancel(GtkWidget *gw,gpointer data);
void pwd_ok(GtkWidget *gw,gpointer data);
void pwd_view(GtkWidget *gw,gpointer data);

void spwd_destroy (GtkWidget * widget, gpointer data);
void spwd_close(GtkWidget *gw,gpointer data);
int try_run_script(char *cmd);

#endif
