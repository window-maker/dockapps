/*

   YAWMPPP - PPP dock app/helper for WindowMaker
   Copyright (C) 2000:

   Author:  Felipe Bergo     (bergo@seul.org)

   based on the wmppp application by

            Martijn Pieterse (pieterse@xs4all.nl)
            Antoine Nulle    (warp@xs4all.nl)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>

#include <gtk/gtk.h>
#include "thinppp.h"
#include "ycommon.h"
#include "misc.h"
#include "isprc.h"

#include "dataxpm.xpm"
#include "backxpm.xpm"

GdkPixmap *origback, *myback ,*datasrc;
GtkWidget *mainw,*mda;

/* ppp related globals */

/* global variables */

long starttime;

char *ProgName;
char *active_interface = "ppp0";
int TimerDivisor = 60;
int updaterate = 5;
int current_isp = 0;
int num_isps = 0;

int got_sched=0;
int caution=1;

struct YAWMPPP_ISP_INFO IspData[MAX_ISPS];

/* log data */
struct LogStruct logconn;

/* PPP variables */

#define 	PPP_UNIT		0
int ppp_h = -1;
int ppp_open = 0;

#define		PPP_STATS_HIS	23

int pixels_per_byte;
int ppp_history[PPP_STATS_HIS + 1][2];

int pressed_button=-1;
int myaction=-1;

pid_t stop_child = 0;
pid_t start_child = 0;
int status;

int wx,wy,dragx,dragy,xlimit,ylimit,ipx,ipy;

int
main(int argc,char **argv) {

  int i;

  /* Parse Command Line */

  ProgName = argv[0];
  if (strlen (ProgName) >= 5)
    ProgName += (strlen (ProgName) - 5);

  for (i = 1; i < argc; i++) {
    char *arg = argv[i];

    if (*arg == '-') {
      switch (arg[1]) {
      case 'c':
	caution=1;
	break;
      case 'd':
	if (strcmp (arg + 1, "display")) {
	  usage ();
	  exit (1);
	}
	break;
      case 'g':
	if (strcmp (arg + 1, "geometry")) {
	  usage ();
	  exit (1);
	}
	break;
      case 'i':
	if (!argv[i + 1]) {
	  usage ();
	  exit (1);
	}
	if (strncmp (argv[i + 1], "ppp", 3)) {
	  usage ();
	  exit (1);
	}
	active_interface = argv[i + 1];
	i++;
	break;
      case 'p':
	caution=2;
	break;
      case 't':
	TimerDivisor = 1;
	break;
      case 'u':
	i++;
	if (!argv[i]) {
	  usage ();
	  exit (1);
	}
	updaterate = atoi (argv[i]);
	if (updaterate < 1 || updaterate > 10) {
	  usage ();
	  exit (1);
	}
	break;
      case 'v':
	printversion ();
	exit (0);
	break;
      case 'z':
	printf("Caution level: %d\n",caution);
	break;
      default:
	usage ();
	exit (0);
	break;
      }
    }
  }

  gtk_init(&argc,&argv);
  gdk_rgb_init();

  create_thinppp();

  init_ppp();
  gtk_idle_add(thinppp,NULL);
  gtk_main();
  save_initial_position();

  gdk_pixmap_unref(origback);
  gdk_pixmap_unref(datasrc);
  gdk_pixmap_unref(myback);

  return 0;
}

void
create_thinppp(void)
{
  GtkStyle *sty;
  GdkBitmap *mask;

  ipx=((gdk_screen_width()-320)/2);
  ipy=0;
  read_initial_position();
  wx=ipx; wy=ipy;

  mainw=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_usize(mainw,320,20);
  gtk_widget_set_uposition(mainw,wx,wy);
  gtk_widget_realize(mainw);
  gdk_window_set_decorations(mainw->window,0);

  xlimit=gdk_screen_width()-320;
  ylimit=gdk_screen_height()-20;

  mda=gtk_drawing_area_new();
  gtk_widget_set_events(mda,GDK_EXPOSURE_MASK|GDK_BUTTON_PRESS_MASK|
			GDK_BUTTON_RELEASE_MASK|GDK_BUTTON_MOTION_MASK);
  gtk_drawing_area_size(GTK_DRAWING_AREA(mda),320,20);
  gtk_container_add(GTK_CONTAINER(mainw),mda);

  sty=gtk_widget_get_style(mainw);

  origback=gdk_pixmap_create_from_xpm_d(mainw->window,
					&mask,
					NULL,
					backxpm_xpm);
  datasrc=gdk_pixmap_create_from_xpm_d(mainw->window,
					&mask,
					NULL,
				        dataxpm_xpm);

  myback=gdk_pixmap_new(mainw->window,320,20,-1);
  gdk_draw_pixmap(myback,
		  mainw->style->fg_gc[GTK_WIDGET_STATE (mainw)],
		  origback,
		  0, 0,
		  0, 0,
		  320, 20);

  gtk_signal_connect(GTK_OBJECT(mda),"expose_event",
		     GTK_SIGNAL_FUNC(exposed),NULL);

  gtk_signal_connect(GTK_OBJECT(mda),"button_press_event",
		     GTK_SIGNAL_FUNC(bpress),NULL);
  gtk_signal_connect(GTK_OBJECT(mda),"button_release_event",
		     GTK_SIGNAL_FUNC(brelease),NULL);
  gtk_signal_connect(GTK_OBJECT(mda),"motion_notify_event",
		     GTK_SIGNAL_FUNC(bmotion),NULL);

  gtk_signal_connect(GTK_OBJECT(mainw),"destroy",
		     GTK_SIGNAL_FUNC(wdestroy),NULL);

  gtk_widget_show(mda);
  gtk_widget_show(mainw);
}

gboolean
exposed(GtkWidget *w,GdkEventExpose *gee,gpointer data)
{
  gdk_draw_pixmap(w->window,
		  w->style->fg_gc[GTK_WIDGET_STATE (w)],
		  myback,
		  gee->area.x, gee->area.y,
		  gee->area.x, gee->area.y,
		  gee->area.width, gee->area.height);
  return FALSE;
}

void
refresh(void)
{
  gdk_draw_pixmap(mainw->window,
		  mainw->style->fg_gc[GTK_WIDGET_STATE (mainw)],
		  myback,
		  0, 0,
		  0, 0,
		  320, 20);
  gdk_flush();
  gtk_widget_queue_resize(mda);
}

void
setled(int index,int type)
{
  int sx,sy,dx,dy,w,h;

  w=LED_SZE_X;
  h=LED_SZE_Y;

  switch(index) {
  case LED_PPP_POWER:
    dx=LED_PWR_X;
    dy=LED_PWR_Y;
    break;
  case LED_PPP_TX:
    dx=LED_SND_X;
    dy=LED_SND_Y;
    break;
  case LED_PPP_RX:
    dx=LED_RCV_X;
    dy=LED_RCV_Y;
    break;
  default:
    return;
  }

  switch(type) {
  case LED_GREEN:
    sx=LED_ON_X;
    sy=LED_ON_Y;
    break;
  case LED_YELLOW:
    sx=LED_WTE_X;
    sy=LED_WTE_Y;
    break;
  case LED_RED:
    sx=LED_ERR_X;
    sy=LED_ERR_Y;
    break;
  case LED_DARK:
    sx=LED_OFF_X;
    sy=LED_OFF_Y;
    break;
  default:
    return;
  }

  paste_xpm(dx,dy,sx,sy,w,h);
  refresh();
}

void
paste_xpm(int dx,int dy,int sx,int sy,int w,int h)
{
  gdk_draw_pixmap(myback,
		  mainw->style->fg_gc[GTK_WIDGET_STATE (mainw)],
		  datasrc,
		  sx, sy,
		  dx, dy,
		  w, h);
}

void
init_ppp(void)
{
  int i;
  for (i = 0; i < MAX_ISPS; i++)
    memset(&IspData[i],0,sizeof(struct YAWMPPP_ISP_INFO));

  make_config_dir();
  signal(SIGUSR1,sigusr_handler);
  signal(SIGHUP,sigusr_handler);
  signal(SIGINT,sigusr_handler);
  signal(SIGTERM,sigusr_handler);

  write_pid_file();
  clean_guards();
  grab_isp_info(1);
}

gint
thinppp(gpointer data)
{
  static int like_a_virgin=1;
  static int but_stat;
  static long currenttime;
  static long lasttime;
  static long waittime;
  static long ppptime;
  static int hour, minute;
  static long timetolog;
  static long ppp_send, ppp_sl = -1;
  static long ppp_recv, ppp_rl = -1;
  static long ppp_sbytes, ppp_rbytes;
  static long ppp_osbytes, ppp_orbytes;
  static struct stat st;
  static int isonline = 0;
  static int speed_ind = 10;

  int i,j;

  if (like_a_virgin) {
    get_statistics (active_interface, &ppp_rl, &ppp_sl,
		    &ppp_orbytes, &ppp_osbytes);
    starttime = 0;
    currenttime = time (0);
    ppptime = 0;
    but_stat = -1;
    waittime = 0;
    timetolog=0;

    /* 888k8 on bottom */
    paste_xpm(ERR_DEST_X,ERR_DEST_Y,ERR_SRC_X+28,ERR_SRC_Y+9,25,8);
    DrawISPName ();
    refresh();

    like_a_virgin=0;
  }

  lasttime = currenttime;
  currenttime = time (0);

  /* Check if any child has left the playground */
  i = waitpid (0, &status, WNOHANG);
  if (i == stop_child && stop_child != 0) {
    starttime = 0;

    setled(LED_PPP_POWER,LED_DARK);
    setled(LED_PPP_RX,LED_DARK);
    setled(LED_PPP_TX,LED_DARK);
    /* 888k8 on bottom */
    paste_xpm(ERR_DEST_X,ERR_DEST_Y,ERR_SRC_X+28,ERR_SRC_Y+9,25,8);
    refresh();
    stop_child = 0;
  }
  if (i == start_child && start_child != 0) {
    if (WIFEXITED (status)) {
      if (WEXITSTATUS (status) == 10) {
	starttime = 0;
	/* 88k8 on bottom */
	paste_xpm(ERR_DEST_X,ERR_DEST_Y,ERR_SRC_X+28,ERR_SRC_Y+9,25,8);
	setled(LED_PPP_POWER,LED_DARK);
	DrawTime (0, 1);
	refresh();
      }
      start_child = 0;
    }
  }

  /* On-line detectie! 1x per second */

  if (currenttime != lasttime) {
    i = 0;

    if (stillonline (active_interface)) {
	i = 1;
	if (!starttime) {
	  starttime = currenttime;

	  if (stat (STAMP_FILE, &st) == 0)
	    starttime = st.st_mtime;

	  setled(LED_PPP_POWER,LED_GREEN);
	  waittime = 0;

	  /* 88k8 on bottom */
	  paste_xpm(ERR_DEST_X,ERR_DEST_Y,ERR_SRC_X+28,ERR_SRC_Y+9,25,8);

	  if (IspData[current_isp].SpeedAction)
	    DrawSpeedInd (IspData[current_isp].SpeedAction);

	  speed_ind = currenttime + 10;
	  refresh();
	}
    }
    if (!i && starttime) {
	starttime = 0;
	setled(LED_PPP_POWER,LED_RED);
	logconn.status=1;
	/* Error */
	paste_xpm(ERR_DEST_X,ERR_DEST_Y,ERR_SRC_X,ERR_SRC_Y+9,25,8);
	if (IspData[current_isp].IfDownAction)
	  execCommand (IspData[current_isp].IfDownAction);
	refresh();
      }
  }

  if (waittime && waittime <= currenttime) {
    setled(LED_PPP_POWER,LED_RED);
    refresh();
    waittime = 0;
  }

  if ((starttime)&&(!isonline)) {
    isonline=1;

    logconn.start=time(NULL);
    logconn.status=0;
    strcpy(logconn.longname,IspData[current_isp].LongName);
    strcpy(logconn.shortname,IspData[current_isp].ShortName);
    strcpy(logconn.user,IspData[current_isp].User);
    strcpy(logconn.phone,IspData[current_isp].Phone);

    if (!strlen(logconn.shortname))
      strcpy(logconn.shortname,"empty");
    if (!strlen(logconn.longname))
      strcpy(logconn.longname,"empty");
    if (!strlen(logconn.user))
      strcpy(logconn.user,"empty");
    if (!strlen(logconn.phone))
      strcpy(logconn.phone,"empty");

    make_guards();
  }
  if ((!starttime)&&(isonline)) {
    isonline=0;
    logconn.end=time(NULL);
    write_log();
    if (got_sched)
      make_delayed_update();
    if (caution>0)
      close_ppp();
  }

  /* If we are on-line. Print the time we are */
  if (starttime) {
    i = currenttime - starttime;
    i /= TimerDivisor;

    if (TimerDivisor == 1)
      if (i > 59 * 60 + 59)
	i /= 60;

    minute = i % 60;
    hour = (i / 60) % 100;
    i = hour * 100 + minute;

    DrawTime (i, currenttime % 2);
    /* We are online, so we can check for send/recv packets */

    get_statistics (active_interface, &ppp_recv, &ppp_send,
		    &ppp_rbytes, &ppp_sbytes);
    if (caution>1)
      close_ppp();

    if (ppp_send != ppp_sl)
      setled(LED_PPP_TX,LED_GREEN);
    else
      setled(LED_PPP_TX,LED_DARK);

    if (ppp_recv != ppp_rl)
      setled(LED_PPP_RX,LED_GREEN);
    else
      setled(LED_PPP_RX,LED_DARK);

    ppp_sl = ppp_send;
    ppp_rl = ppp_recv;

    /* Every five seconds we check to load on the line */

    if (currenttime - timetolog >= 0) {
      timetolog=currenttime + 60;
      make_guards();
    }

    if ((currenttime - ppptime >= 0) || (ppptime == 0)) {
      ppptime = currenttime + updaterate;

      ppp_history[PPP_STATS_HIS][0] = ppp_rbytes - ppp_orbytes;
      ppp_history[PPP_STATS_HIS][1] = ppp_sbytes - ppp_osbytes;

      ppp_orbytes = ppp_rbytes;
      ppp_osbytes = ppp_sbytes;

      DrawStats (23, 9, 170, 13);

      for (j = 1; j < 24; j++) {
	ppp_history[j - 1][0] = ppp_history[j][0];
	ppp_history[j - 1][1] = ppp_history[j][1];
      }
      if (currenttime > speed_ind) {
	DrawLoadInd ((ppp_history[23][0] + ppp_history[23][1]) / updaterate);
      }
    }

    refresh();
  }

  switch (myaction) {
  case BUT_V:
    if (!starttime) {
      /* 888k8 */
      paste_xpm(ERR_DEST_X,ERR_DEST_Y,ERR_SRC_X+28,ERR_SRC_Y+9,25,8);
      DrawTime (0, 1);
      start_child = execCommand (IspData[current_isp].StartAction);
      setled(LED_PPP_POWER,LED_YELLOW);
      waittime = ORANGE_LED_TIMEOUT + currenttime;
      refresh();
    }
    break;
  case BUT_X:
    if (stop_child == 0)
      stop_child = execCommand (IspData[current_isp].StopAction);
    break;
  case BUT_REW:
    if (!starttime) {
      current_isp--;
      if (current_isp < 0)
	current_isp = num_isps - 1;
      if (current_isp < 0)
	current_isp=0;
      DrawISPName ();
      refresh();
    }
    break;
  case BUT_FF:
    if (!starttime) {
      current_isp++;
      if (current_isp == num_isps)
	current_isp = 0;
      DrawISPName ();
      refresh();
    }
    break;
  case BUT_CONF:
    run_pref_app();
    break;
  case BUT_LOG:
    run_log_app();
    break;
  case BUT_KILL:
    if (!starttime)
      gtk_widget_destroy(mainw);
    break;
  }
  myaction=-1;

  usleep (50000L);
  return TRUE;
}

gboolean
wdestroy(GtkWidget *w,GdkEvent *ev,gpointer data)
{
  int i;
  while (start_child | stop_child) {
    i = waitpid (0, &status, WNOHANG);
    if (i == stop_child)
      stop_child = 0;
    if (i == start_child)
      start_child = 0;
    usleep (50000l);
  }
  gtk_main_quit();
  return FALSE;
}

gboolean
bpress(GtkWidget *w,GdkEventButton *geb,gpointer data)
{
  int x,y;

  if (geb==NULL)
    return FALSE;

  x=(int)geb->x;
  y=(int)geb->y;

  dragx=(int)geb->x_root;
  dragy=(int)geb->y_root;

  pressed_button=-1;

  if (x<50) {
    pressed_button=BUT_DRAG;
    grab_me();
  }

  if (inbox(x,y,BUT_V_X,BUT_V_Y,12,11)) {
    pressed_button=BUT_V;
    paste_xpm(BUT_V_X,BUT_V_Y,BUT_V_SRC_X,BUT_V_SRC_Y,12,11);
    refresh();
    grab_me();
    return FALSE;
  }

  if (inbox(x,y,BUT_X_X,BUT_X_Y,12,11)) {
    pressed_button=BUT_X;
    paste_xpm(BUT_X_X,BUT_X_Y,BUT_X_SRC_X,BUT_X_SRC_Y,12,11);
    refresh();
    grab_me();
    return FALSE;
  }

  if (inbox(x,y,BUT_R_X,BUT_R_Y,12,11)) {
    pressed_button=BUT_REW;
    paste_xpm(BUT_R_X,BUT_R_Y,BUT_R_SRC_X,BUT_R_SRC_Y,12,11);
    refresh();
    grab_me();
    return FALSE;
  }

  if (inbox(x,y,BUT_F_X,BUT_F_Y,12,11)) {
    pressed_button=BUT_FF;
    paste_xpm(BUT_F_X,BUT_F_Y,BUT_F_SRC_X,BUT_F_SRC_Y,12,11);
    refresh();
    grab_me();
    return FALSE;
  }

  if (inbox(x,y,BUT_C_X,BUT_C_Y,12,11)) {
    pressed_button=BUT_CONF;
    paste_xpm(BUT_C_X,BUT_C_Y,BUT_C_SRC_X,BUT_C_SRC_Y,12,11);
    refresh();
    grab_me();
    return FALSE;
  }

  if (inbox(x,y,BUT_L_X,BUT_L_Y,12,11)) {
    pressed_button=BUT_LOG;
    paste_xpm(BUT_L_X,BUT_L_Y,BUT_L_SRC_X,BUT_L_SRC_Y,12,11);
    refresh();
    grab_me();
    return FALSE;
  }

  if (inbox(x,y,BUT_K_X,BUT_K_Y,17,18)) {
    pressed_button=BUT_KILL;
    paste_xpm(BUT_K_X,BUT_K_Y,BUT_K_SRC_X,BUT_K_SRC_Y,17,18);
    refresh();
    grab_me();
    return FALSE;
  }

  return FALSE;
}

gboolean
brelease(GtkWidget *w,GdkEventButton *geb,gpointer data)
{
  int x,y,c;
  if (geb==NULL)
    return FALSE;

  if (pressed_button<0)
    return FALSE;

  x=(int)geb->x;
  y=(int)geb->y;
  c=-1;

  if (inbox(x,y,BUT_V_X,BUT_V_Y,12,11))
    c=BUT_V;
  if (inbox(x,y,BUT_X_X,BUT_X_Y,12,11))
    c=BUT_X;
  if (inbox(x,y,BUT_R_X,BUT_R_Y,12,11))
    c=BUT_REW;
  if (inbox(x,y,BUT_F_X,BUT_F_Y,12,11))
    c=BUT_FF;
  if (inbox(x,y,BUT_C_X,BUT_C_Y,12,11))
    c=BUT_CONF;
  if (inbox(x,y,BUT_L_X,BUT_L_Y,12,11))
    c=BUT_LOG;
  if (inbox(x,y,BUT_K_X,BUT_K_Y,17,18))
    c=BUT_KILL;

  ungrab_me();
  if (c!=pressed_button) {
    pressed_button=-1;
    paste_xpm(BUT_V_X,BUT_V_Y,BUT_V_SRC_X+24,BUT_V_SRC_Y,12,11);
    paste_xpm(BUT_X_X,BUT_X_Y,BUT_X_SRC_X+24,BUT_X_SRC_Y,12,11);
    paste_xpm(BUT_R_X,BUT_R_Y,BUT_R_SRC_X+24,BUT_R_SRC_Y,12,11);
    paste_xpm(BUT_F_X,BUT_F_Y,BUT_F_SRC_X+24,BUT_F_SRC_Y,12,11);
    paste_xpm(BUT_C_X,BUT_C_Y,BUT_C_SRC_X+24,BUT_C_SRC_Y,12,11);
    paste_xpm(BUT_L_X,BUT_L_Y,BUT_L_SRC_X+24,BUT_L_SRC_Y,12,11);
    paste_xpm(BUT_K_X,BUT_K_Y,BUT_K_SRC_X+17,BUT_K_SRC_Y,17,18);
    refresh();
    return FALSE;
  }

  myaction=pressed_button;

  paste_xpm(BUT_V_X,BUT_V_Y,BUT_V_SRC_X+24,BUT_V_SRC_Y,12,11);
  paste_xpm(BUT_X_X,BUT_X_Y,BUT_X_SRC_X+24,BUT_X_SRC_Y,12,11);
  paste_xpm(BUT_R_X,BUT_R_Y,BUT_R_SRC_X+24,BUT_R_SRC_Y,12,11);
  paste_xpm(BUT_F_X,BUT_F_Y,BUT_F_SRC_X+24,BUT_F_SRC_Y,12,11);
  paste_xpm(BUT_C_X,BUT_C_Y,BUT_C_SRC_X+24,BUT_C_SRC_Y,12,11);
  paste_xpm(BUT_L_X,BUT_L_Y,BUT_L_SRC_X+24,BUT_L_SRC_Y,12,11);
  paste_xpm(BUT_K_X,BUT_K_Y,BUT_K_SRC_X+17,BUT_K_SRC_Y,17,18);
  refresh();
  pressed_button=-1;
  return FALSE;
}

gboolean
bmotion(GtkWidget *w,GdkEventMotion *geb,gpointer data)
{
  if (pressed_button!=BUT_DRAG)
    return TRUE;

  if (geb==NULL)
    return TRUE;

  wx=wx+((int)geb->x_root-dragx);
  wy=wy+((int)geb->y_root-dragy);

  if (wx<0) wx=0;
  if (wx>xlimit) wx=xlimit;
  if (wy<0) wy=0;
  if (wy>ylimit) wy=ylimit;

  dragx=(int)geb->x_root;
  dragy=(int)geb->y_root;

  gdk_window_move(mainw->window,wx,wy);
  gdk_flush();
  return FALSE;
}

void
grab_me(void)
{
  gdk_pointer_grab(mda->window,FALSE,
		   GDK_BUTTON_RELEASE_MASK,
		   NULL,
		   NULL,
		   time(NULL));
}

void
ungrab_me(void)
{
  gdk_pointer_ungrab(time(NULL));
}

gboolean
inbox(int x,int y,int bx,int by,int bw,int bh)
{
  if ((x>=bx)&&(y>=by)&&(x<=(bx+bw))&&(y<=(by+bh)))
    return TRUE;
  else
    return FALSE;
}

void
sigusr_handler(int signum)
{
  save_initial_position();
  if (signum==SIGUSR1) {
    if (!starttime) {
      grab_isp_info(0);
      if (current_isp>=num_isps)
	current_isp=0;
      DrawISPName();
      refresh();
    } else {
      got_sched=1;
      warn_pref();
    }
  }
  else {
    remove_pid_file();
    exit(0);
  }
}

/* lower level drawing */

void
DrawISPName (void)
{
    int i, s;

    s = strlen (IspData[current_isp].ShortName);
    for (i = 0; i < 5; i++)
      {
	  if (s >= (i + 1))
	      draw_isp_char (i, IspData[current_isp].ShortName[i]);
	  else
	      draw_isp_char (i, ' ');
      }
}

void
draw_isp_char (int pos, char letter)
{
    int sx = 0, sy = 0, ac = 0;

    if ((!ac) && (letter >= 'A') && (letter <= 'Z'))
      {
	  sx = UPPER_ABC_BASE_X;
	  sy = UPPER_ABC_BASE_Y;
	  sy += 8 * ((letter - 'A') / 12);
	  sx += 5 * ((letter - 'A') % 12);
	  ac = 1;
      }
    if ((!ac) && (letter >= 'a') && (letter <= 'z'))
      {
	  sx = LOWER_ABC_BASE_X;
	  sy = LOWER_ABC_BASE_Y;
	  sy += 8 * ((letter - 'a') / 12);
	  sx += 5 * ((letter - 'a') % 12);
	  ac = 1;
      }
    if ((!ac) && (letter >= '0') && (letter <= '9'))
      {
	  sx = DIGIT_BASE_X;
	  sy = DIGIT_BASE_Y;
	  sx += 5 * (letter - '0');
	  ac = 1;
      }
    if (!ac)
      {
	  sx = SPACE_BASE_X;
	  sy = SPACE_BASE_Y;
      }

    paste_xpm(ISP_BASE_X + 5 * pos, ISP_BASE_Y, sx, sy, 4, 7);
}

void
DrawTime (int i, int j)
{
  int k = 1000;

  paste_xpm(TIMER_DES_X + 6 * 0, TIMER_DES_Y,
	    TIMER_SZE_X * ((i/k)%10)+1, TIMER_SRC_Y,5,7);
  k=k/10;

  paste_xpm(TIMER_DES_X + 6 * 1, TIMER_DES_Y,
	    TIMER_SZE_X * ((i/k)%10)+1, TIMER_SRC_Y,5,7);
  k=k/10;

  /* colon */
  if (j)
    paste_xpm(TIMER_DES_X + 6 * 2 + 1, TIMER_DES_Y,
	      62, TIMER_SRC_Y,1,7);
  else
    paste_xpm(TIMER_DES_X + 6 * 2 + 1, TIMER_DES_Y,
	      63, TIMER_SRC_Y,1,7);

  paste_xpm(TIMER_DES_X + 6 * 2 + 4, TIMER_DES_Y,
	    TIMER_SZE_X * ((i/k)%10)+1, TIMER_SRC_Y,5,7);
  k=k/10;

  paste_xpm(TIMER_DES_X + 6 * 3 + 4, TIMER_DES_Y,
	    TIMER_SZE_X * ((i/k)%10)+1, TIMER_SRC_Y,5,7);
}

void
DrawStats (int num, int size, int x_left, int y_bottom)
{
  int pixels_per_byte;
  int j, k;

  pixels_per_byte = size;
  for (j = 0; j < num; j++)
    if ((ppp_history[j][0]+ppp_history[j][1]) > pixels_per_byte)
      pixels_per_byte = ppp_history[j][0] + ppp_history[j][1];

  pixels_per_byte /= size;

  for (k = 0; k < num; k++)
    for (j = 0; j < size; j++) {
      if (j < (ppp_history[k][0] / pixels_per_byte))
	paste_xpm(k+x_left, y_bottom-j,HIST_SRC_X+2,HIST_SRC_Y,1,1);
      else if (j < (ppp_history[k][0] + ppp_history[k][1]) / pixels_per_byte)
	paste_xpm(k+x_left,y_bottom-j,HIST_SRC_X+1,HIST_SRC_Y,1,1);
      else
	paste_xpm(k+x_left,y_bottom-j,HIST_SRC_X,HIST_SRC_Y,1,1);
    }
}

void
PrintLittle (int i, int *k)
{
  switch (i) {
  case -1:
    *k -= 5;
    paste_xpm(*k,ERR_DEST_Y,12*5,ERR_SRC_Y,4,8);
    break;
  case 0:
    *k -= 5;
    paste_xpm(*k,ERR_DEST_Y,45,ERR_SRC_Y,5,8);
    break;
  default:
    *k -= 5;
    paste_xpm(*k,ERR_DEST_Y,i*5 - 5,ERR_SRC_Y,5,8);
    break;
  }
}

void
DrawSpeedInd (char *speed_action)
{
  int linespeed, i, k;
  FILE *fp;
  char *p;
  char temp[128];

  fp = popen (speed_action, "r");

  if (fp) {
    linespeed = 0;

    while (fgets (temp, 128, fp))
      ;

    pclose (fp);

    if ((p = strstr (temp, "CONNECT")))
      linespeed = atoi (p + 8);

    k = ERR_DEST_X+25;

    i = (linespeed % 1000) / 100;
    linespeed /= 1000;
    PrintLittle (i, &k);

    k -= 5;
    paste_xpm(k,ERR_DEST_Y,ERR_SRC_X+50,ERR_SRC_Y,5,8);

    do {
      PrintLittle (linespeed % 10, &k);
      linespeed /= 10;
    } while (linespeed);
  }
}

void
DrawLoadInd (int speed)
{
  int i, k;

  k = ERR_DEST_X+25;
  for (i = 0; i < 5; i++)
    PrintLittle (-1, &k);

  k = ERR_DEST_X+25;

  do {
    PrintLittle (speed % 10, &k);
    speed /= 10;
  } while (speed);
}


void
make_delayed_update(void)
{
  grab_isp_info(0);
  if (current_isp>=num_isps)
    current_isp=0;
  DrawISPName();
  refresh();
  got_sched=0;
}

void
usage (void) {
  fprintf (stderr,
	   "\nyawmppp.thin\nYet Another Window Maker PPP dock applet,\nfor non-Window Maker window managers\n\n");
  fprintf (stderr,
	   "version %s\n\n",VERSION);
  fprintf (stderr, "usage:\n");
  fprintf (stderr, "-h                     this help screen\n");
  fprintf (stderr, "-i <device>            (ppp0, ppp1, etc)\n");
  fprintf (stderr, "-t                     set the on-line timer to MM:SS instead of HH:MM\n");
  fprintf (stderr, "-u <update rate>       (1..10), default 5 seconds\n");
  fprintf (stderr, "-v                     print the version number\n");
  fprintf (stderr, "-paranoid              be paranoid about open sockets\n");
  fprintf (stderr, "\n");
}

void
printversion (void)
{
  fprintf (stderr, "%s\n", VERSION);
}

void
read_initial_position(void)
{
  FILE *f;
  char *p,z[256];
  p=getenv("HOME");
  sprintf(z,"%s/.yawmppp/thin.position",p);
  f=fopen(z,"r");
  if (!f)
    return;
  fgets(z,255,f);
  ipx=atoi(strtok(z," \t\n"));
  ipy=atoi(strtok(NULL," \t\n"));
  fclose(f);
}

void
save_initial_position(void)
{
  FILE *f;
  char *p,z[256];
  p=getenv("HOME");
  sprintf(z,"%s/.yawmppp/thin.position",p);
  f=fopen(z,"w");
  if (!f)
    return;
  ipx=wx;
  ipy=wy;
  fprintf(f,"%d %d\n",ipx,ipy);
  fclose(f);
}
