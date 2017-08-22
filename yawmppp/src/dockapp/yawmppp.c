/*

   YAWMPPP - PPP dock app/helper for WindowMaker
   Copyright (C) 2000,2001:

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

   $Id: yawmppp.c,v 1.1.1.1 2001/02/22 07:16:01 bergo Exp $
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

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "wmgeneral.h"
#include "misc.h"
#include "isprc.h"
#include "ycommon.h"

#include "dockxpm.xpm"
#include "yawmppp.h"

/* external vars */

extern char **environ;

/* global variables */

long starttime;

char *ProgName;
char *active_interface = "ppp0";
int TimerDivisor = 60;
int updaterate = 5;

int dock_mask_width = 64;
int dock_mask_height = 64;
char dock_mask_bits[64 * 64];

int current_isp = 0;
int num_isps = 0;

int got_sched=0;
int caution=1;

struct YAWMPPP_ISP_INFO IspData[MAX_ISPS];

/* log data */
struct {
  time_t start;
  time_t end;
  int    status; /* 0=ok 1=error 2=crash */
  char   longname[128];
  char   shortname[16];
  char   phone[32];
  char   user[32];
} logconn;

/* PPP variables */

#define 	PPP_UNIT		0
int ppp_h = -1;
int ppp_open = 0;

#define		PPP_STATS_HIS	54

int pixels_per_byte;

int ppp_history[PPP_STATS_HIS + 1][2];

/* main */

int
main (int argc, char *argv[])
{

    int i;

    /* Parse Command Line */

    ProgName = argv[0];
    if (strlen (ProgName) >= 5)
	ProgName += (strlen (ProgName) - 5);

    for (i = 1; i < argc; i++)
      {
	  char *arg = argv[i];

	  if (*arg == '-')
	    {
		switch (arg[1])
		  {
		  case 'c':
		    caution=1;
		    break;
		  case 'd':
		      if (strcmp (arg + 1, "display"))
			{
			    usage ();
			    exit (1);
			}
		      break;
		  case 'g':
		      if (strcmp (arg + 1, "geometry"))
			{
			    usage ();
			    exit (1);
			}
		      break;
		  case 'i':
		      if (!argv[i + 1])
			{
			    usage ();
			    exit (1);
			}
		      if (strncmp (argv[i + 1], "ppp", 3))
			{
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
		      if (!argv[i])
			{
			    usage ();
			    exit (1);
			}
		      updaterate = atoi (argv[i]);
		      if (updaterate < 1 || updaterate > 10)
			{
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

    for (i = 0; i < MAX_ISPS; i++)
      memset(&IspData[i],0,sizeof(struct YAWMPPP_ISP_INFO));

    make_config_dir();
    signal(SIGUSR1,sigusr_handler);
    signal(SIGHUP,sigusr_handler);
    signal(SIGINT,sigusr_handler);
    signal(SIGTERM,sigusr_handler);

    write_pid_file();
    clean_guards();

    yawmppp_routine (argc, argv);

    return 0;
}

void
yawmppp_routine (int argc, char **argv)
{
    int i, j;

    int but_stat;

    long currenttime;
    long lasttime;
    long waittime;
    long ppptime;
    int hour, minute;
    long timetolog;

    long ppp_send, ppp_sl = -1;
    long ppp_recv, ppp_rl = -1;
    long ppp_sbytes, ppp_rbytes;
    long ppp_osbytes, ppp_orbytes;

    struct stat st;

    pid_t stop_child = 0;
    pid_t start_child = 0;
    int status;

    int isonline = 0;

    XEvent Event;

    int speed_ind = 10;

    /* Initialize some stuff */

    get_statistics (active_interface, &ppp_rl, &ppp_sl,
		    &ppp_orbytes, &ppp_osbytes);
    if (caution>0)
      close_ppp();

    grab_isp_info(1);
 
    /* Open the display */

    createXBMfromXPM (dock_mask_bits, dockxpm_xpm,
		      dock_mask_width, dock_mask_height);

    openXwindow (argc, argv, dockxpm_xpm, dock_mask_bits,
		 dock_mask_width, dock_mask_height);

    /* V Button */
    AddMouseRegion (0, 35, 48, 46, 58);
    /* x Button */
    AddMouseRegion (1, 47, 48, 58, 58);

    /* < Button */
    AddMouseRegion (2, 5, 48, 16, 58);
    /* > Button */
    AddMouseRegion (3, 17, 48, 28, 58);
    /* ISP display */
    AddMouseRegion (4, 5, 6, 59, 43);

    starttime = 0;
    currenttime = time (0);
    ppptime = 0;
    but_stat = -1;
    waittime = 0;
    timetolog=0;

    /* 888k8 on bottom */
    copyXPMArea (ERR_SRC_X+28, ERR_SRC_Y+9, 25, 8, ERR_DEST_X, ERR_DEST_Y);

    DrawISPName ();

    while (1)
      {
	  lasttime = currenttime;
	  currenttime = time (0);

	  /* Check if any child has left the playground */
	  i = waitpid (0, &status, WNOHANG);
	  if (i == stop_child && stop_child != 0)
	    {

		starttime = 0;
		SetOffLED (LED_PPP_POWER);
		SetOffLED (LED_PPP_RX);
		SetOffLED (LED_PPP_TX);
		/* 888k8 on bottom */
		copyXPMArea (ERR_SRC_X+28, ERR_SRC_Y+9, 25, 8,
			     ERR_DEST_X, ERR_DEST_Y);
		RedrawWindow ();

		stop_child = 0;
	    }
	  if (i == start_child && start_child != 0)
	    {
		if (WIFEXITED (status))
		  {
		      if (WEXITSTATUS (status) == 10)
			{

			    starttime = 0;
			    /* 88k8 on bottom */
			    copyXPMArea (ERR_SRC_X+28, ERR_SRC_Y+9, 25, 8,
					 ERR_DEST_X, ERR_DEST_Y);
			    SetOffLED (LED_PPP_POWER);
			    DrawTime (0, 1);
			    RedrawWindow ();
			}
		      start_child = 0;
		  }
	    }

	  /* On-line detectie! 1x per second */

	  if (currenttime != lasttime)
	    {
		i = 0;

		if (stillonline (active_interface))
		  {
		      i = 1;
		      if (!starttime)
			{
			    starttime = currenttime;

			    if (stat (STAMP_FILE, &st) == 0)
				starttime = st.st_mtime;

			    SetOnLED (LED_PPP_POWER);
			    waittime = 0;

			    /* 88k8 on bottom */
			    copyXPMArea (ERR_SRC_X+28, ERR_SRC_Y+9, 25, 8,
					 ERR_DEST_X, ERR_DEST_Y);

			    if (IspData[current_isp].SpeedAction)
				DrawSpeedInd (IspData[current_isp].SpeedAction);

			    speed_ind = currenttime + 10;

			    RedrawWindow ();
			}
		  }
		if (!i && starttime)
		  {
		      starttime = 0;
		      SetErrLED (LED_PPP_POWER);
		      logconn.status=1;

		      /* Error */
		      copyXPMArea (ERR_SRC_X, ERR_SRC_Y+9, 25, 8,
				   ERR_DEST_X, ERR_DEST_Y);

		      if (IspData[current_isp].IfDownAction)
			  execCommand (IspData[current_isp].IfDownAction);

		      RedrawWindow ();
		  }
	    }

	  if (waittime && waittime <= currenttime)
	    {
		SetOffLED (LED_PPP_POWER);
		RedrawWindow ();
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
	  if (starttime)
	    {
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
		    SetOnLED (LED_PPP_TX);
		else
		    SetOffLED (LED_PPP_TX);

		if (ppp_recv != ppp_rl)
		    SetOnLED (LED_PPP_RX);
		else
		    SetOffLED (LED_PPP_RX);

		ppp_sl = ppp_send;
		ppp_rl = ppp_recv;

		/* Every five seconds we check to load on the line */

		if (currenttime - timetolog >= 0) {
		  timetolog=currenttime + 60;
		  make_guards();
		}

		if ((currenttime - ppptime >= 0) || (ppptime == 0))
		  {

		      ppptime = currenttime + updaterate;

		      ppp_history[PPP_STATS_HIS][0] = ppp_rbytes - ppp_orbytes;
		      ppp_history[PPP_STATS_HIS][1] = ppp_sbytes - ppp_osbytes;

		      ppp_orbytes = ppp_rbytes;
		      ppp_osbytes = ppp_sbytes;

		      DrawStats (54, 17, 5, 32);

		      for (j = 1; j < 55; j++)
			{
			    ppp_history[j - 1][0] = ppp_history[j][0];
			    ppp_history[j - 1][1] = ppp_history[j][1];
			}
		      if (currenttime > speed_ind)
			{
			    DrawLoadInd ((ppp_history[54][0] + ppp_history[54][1]) / updaterate);
			}
		  }

		RedrawWindow ();
	    }


	  while (XPending (display))
	    {
		XNextEvent (display, &Event);
		switch (Event.type)
		  {
		  case Expose:
		      RedrawWindow ();
		      break;
		  case DestroyNotify:
		      XCloseDisplay (display);
		      while (start_child | stop_child)
			{
			    i = waitpid (0, &status, WNOHANG);
			    if (i == stop_child)
				stop_child = 0;
			    if (i == start_child)
				start_child = 0;
			    usleep (50000l);
			}
		      exit (0);
		      break;
		  case ButtonPress:
		      i = CheckMouseRegion (Event.xbutton.x, Event.xbutton.y);
		      switch (i)
			{
			case 0:
			    ButtonDown (BUT_V);
			    break;
			case 1:
			    ButtonDown (BUT_X);
			    break;
			case 2:
			    ButtonDown (BUT_REW);
			    break;
			case 3:
			    ButtonDown (BUT_FF);
			    break;
			}
		      but_stat = i;

		      RedrawWindow ();
		      break;
		  case ButtonRelease:
		      i = CheckMouseRegion (Event.xbutton.x, Event.xbutton.y);
		      // Button but_stat omhoogdoen!
		      switch (but_stat)
			{
			case 0:
			    ButtonUp (BUT_V);
			    break;
			case 1:
			    ButtonUp (BUT_X);
			    break;
			case 2:
			    ButtonUp (BUT_REW);
			    break;
			case 3:
			    ButtonUp (BUT_FF);
			    break;
			}

		      if (i == but_stat && but_stat >= 0)
			{
			    switch (i)
			      {
			      case 0:
				  if (!starttime)
				    {
				      /* 888k8 */
				      copyXPMArea (ERR_SRC_X+28, ERR_SRC_Y+9,
						   25, 8,
						   ERR_DEST_X, ERR_DEST_Y);
					DrawTime (0, 1);
					start_child = execCommand (IspData[current_isp].StartAction);
					SetWaitLED (LED_PPP_POWER);
					waittime = ORANGE_LED_TIMEOUT + currenttime;
				    }

				  break;
			      case 1:
				  if (stop_child == 0)
				    {
					stop_child = execCommand (IspData[current_isp].StopAction);
				    }
				  break;
			      case 2:
				  if (!starttime)
				    {
					current_isp--;
					if (current_isp < 0)
					    current_isp = num_isps - 1;
					if (current_isp < 0)
					  current_isp=0;
					DrawISPName ();
				    }
				  break;
			      case 3:
				  if (!starttime)
				    {
					current_isp++;
					if (current_isp == num_isps)
					    current_isp = 0;
					DrawISPName ();
				    }
				  break;
			      case 4:
				if (Event.xbutton.button==Button1)
				  run_pref_app();
				else
				  run_log_app();
				break;
			      }
			}
		      RedrawWindow ();

		      but_stat = -1;
		      break;
		  default:
		      break;
		  }
	    }
	  usleep (50000L);
      }
}

/* DrawTime */

void
DrawTime (int i, int j)
{

    int k = 1000;

    copyXPMArea (TIMER_SZE_X * ((i / k) % 10) + 1, TIMER_SRC_Y,
		 5, 7,
		 TIMER_DES_X + 6 * 0, TIMER_DES_Y);
    k = k / 10;
    copyXPMArea (TIMER_SZE_X * ((i / k) % 10) + 1, TIMER_SRC_Y,
		 5, 7,
		 TIMER_DES_X + 6 * 1, TIMER_DES_Y);
    k = k / 10;

    /* colon */
    if (j)
	copyXPMArea (62, TIMER_SRC_Y,
		     1, 7,
		     TIMER_DES_X + 6 * 2 + 1, TIMER_DES_Y);
    else
	copyXPMArea (63, TIMER_SRC_Y,
		     1, 7,
		     TIMER_DES_X + 6 * 2 + 1, TIMER_DES_Y);

    copyXPMArea (TIMER_SZE_X * ((i / k) % 10) + 1, TIMER_SRC_Y,
		 5, 7,
		 TIMER_DES_X + 6 * 2 + 4, TIMER_DES_Y);
    k = k / 10;
    copyXPMArea (TIMER_SZE_X * ((i / k) % 10) + 1, TIMER_SRC_Y,
		 5, 7,
		 TIMER_DES_X + 6 * 3 + 4, TIMER_DES_Y);
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
      for (j = 0; j < size; j++)
	{	    
	  if (j < (ppp_history[k][0] / pixels_per_byte))
	    copyXPMArea (58 + 2, 92, 1, 1, k + x_left, y_bottom - j);
	  else if (j < (ppp_history[k][0] + ppp_history[k][1]) / pixels_per_byte)
	    copyXPMArea (58 + 1, 92, 1, 1, k + x_left, y_bottom - j);
	  else
	    copyXPMArea (58 + 0, 92, 1, 1, k + x_left, y_bottom - j);
	}
}

void
PrintLittle (int i, int *k)
{

    switch (i)
      {
      case -1:
	  *k -= 5;
	  copyXPMArea (12 * 5, ERR_SRC_Y, 4, 8, *k, ERR_DEST_Y);
	  break;
      case 0:
	  *k -= 5;
	  copyXPMArea (45, ERR_SRC_Y, 5, 8, *k, ERR_DEST_Y);
	  break;
      default:
	  *k -= 5;
	  copyXPMArea (i * 5 - 5, ERR_SRC_Y, 5, 8, *k, ERR_DEST_Y);
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

    if (fp)
      {
	  linespeed = 0;

	  while (fgets (temp, 128, fp))
	      ;

	  pclose (fp);

	  if ((p = strstr (temp, "CONNECT")))
	    {
		linespeed = atoi (p + 8);
	    }

	  k = ERR_DEST_X+25;

	  i = (linespeed % 1000) / 100;
	  linespeed /= 1000;
	  PrintLittle (i, &k);

	  k -= 5;
	  copyXPMArea (ERR_SRC_X+50, ERR_SRC_Y, 5, 8, k, ERR_DEST_Y);

	  do
	    {
		PrintLittle (linespeed % 10, &k);
		linespeed /= 10;
	    }
	  while (linespeed);
      }
}

/* DrawLoadInd */

void
DrawLoadInd (int speed)
{

    int i, k;

    k = ERR_DEST_X+25;
    for (i = 0; i < 5; i++)
	PrintLittle (-1, &k);

    k = ERR_DEST_X+25;

    do
      {
	  PrintLittle (speed % 10, &k);
	  speed /= 10;
      }
    while (speed);
}

/* usage */

void
usage (void)
{
    fprintf (stderr, 
	     "\nyawmppp - Yet Another Window Maker PPP dock applet\n\n");
    fprintf (stderr, 
	     "version %s\n\n",VERSION);
    fprintf (stderr, "usage:\n");
    fprintf (stderr, "-h                   this help screen\n");
    fprintf (stderr, "-i <device>          (ppp0, ppp1, etc)\n");
    fprintf (stderr, "-t                   set the on-line timer to MM:SS instead of HH:MM\n");
    fprintf (stderr, "-u <update rate>     (1..10), default 5 seconds\n");
    fprintf (stderr, "-v                   print the version number\n");
    fprintf (stderr, "-paranoid            be paranoid about open sockets\n");
    fprintf (stderr, "\n");
}

/* printversion */

void
printversion (void)
{
    fprintf (stderr, "%s\n", VERSION);
}

/* SetOnLED */

void
SetOnLED (int led)
{
    switch (led)
      {
      case LED_PPP_POWER:
	  copyXPMArea (LED_ON_X, LED_ON_Y, LED_SZE_X, LED_SZE_Y, LED_PWR_X, LED_PWR_Y);
	  break;
      case LED_PPP_RX:
	  copyXPMArea (ARR_DN_X, ARR_DN_Y+ARR_ACTV, ARR_W, ARR_H, LED_RCV_X, LED_RCV_Y);
	  break;
      case LED_PPP_TX:
	  copyXPMArea (ARR_UP_X, ARR_UP_Y+ARR_ACTV, ARR_W, ARR_H, LED_SND_X, LED_SND_Y);
	  break;
      }
}

/* SetOffLED */

void
SetOffLED (int led)
{

    switch (led)
      {
      case LED_PPP_POWER:
	  copyXPMArea (LED_OFF_X, LED_OFF_Y, LED_SZE_X, LED_SZE_Y, LED_PWR_X, LED_PWR_Y);
	  break;
      case LED_PPP_RX:
	  copyXPMArea (ARR_DN_X, ARR_DN_Y , ARR_W, ARR_H, LED_RCV_X, LED_RCV_Y);
	  break;
      case LED_PPP_TX:
	  copyXPMArea (ARR_UP_X, ARR_UP_Y, ARR_W, ARR_H, LED_SND_X, LED_SND_Y);
	  break;

      }
}

/* SetErrELD */

void
SetErrLED (int led)
{

    switch (led)
      {
      case LED_PPP_POWER:
	  copyXPMArea (LED_ERR_X, LED_ERR_Y, LED_SZE_X, LED_SZE_Y, LED_PWR_X, LED_PWR_Y);
	  break;
      }
}

/* SetWaitLED */

void
SetWaitLED (int led)
{

    switch (led)
      {
      case LED_PPP_POWER:
	  copyXPMArea (LED_WTE_X, LED_WTE_Y, LED_SZE_X, LED_SZE_Y, LED_PWR_X, LED_PWR_Y);
	  break;
      }
}

/* Button Up */

void
ButtonUp (int button)
{

    switch (button)
      {
      case BUT_V:
	  copyXPMArea (BUT_V_SRC_X+BUT_UP_INC, BUT_V_SRC_Y,
		       12, 11,
		       BUT_V_X, BUT_V_Y);
	  break;
      case BUT_X:
	  copyXPMArea (BUT_X_SRC_X+BUT_UP_INC, BUT_X_SRC_Y,
		       12, 11,
		       BUT_X_X, BUT_X_Y);
	  break;
      case BUT_REW:
	  copyXPMArea (BUT_R_SRC_X+BUT_UP_INC, BUT_R_SRC_Y,
		       12, 11,
		       BUT_R_X, BUT_R_Y);
	  break;
      case BUT_FF:
	  copyXPMArea (BUT_F_SRC_X+BUT_UP_INC, BUT_F_SRC_Y,
		       12, 11,
		       BUT_F_X, BUT_F_Y);
	  break;
      }
}

/* Button Down */

void
ButtonDown (int button)
{

    switch (button)
      {
      case BUT_V:
	  copyXPMArea (BUT_V_SRC_X, BUT_V_SRC_Y,
		       12, 11,
		       BUT_V_X, BUT_V_Y);
	  break;
      case BUT_X:
	  copyXPMArea (BUT_X_SRC_X, BUT_X_SRC_Y,
		       12, 11,
		       BUT_X_X, BUT_X_Y);
	  break;
      case BUT_REW:
	  copyXPMArea (BUT_R_SRC_X, BUT_R_SRC_Y,
		       12, 11,
		       BUT_R_X, BUT_R_Y);
	  break;
      case BUT_FF:
	  copyXPMArea (BUT_F_SRC_X, BUT_F_SRC_Y,
		       12, 11,
		       BUT_F_X, BUT_F_Y);
	  break;
      }
}

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

    copyXPMArea (sx, sy, 4, 7, ISP_BASE_X + 5 * pos, ISP_BASE_Y);
}

void
sigusr_handler(int signum)
{
  if (signum==SIGUSR1) {
    if (!starttime) {
      grab_isp_info(0);
      if (current_isp>=num_isps)
	current_isp=0;
      DrawISPName();
      RedrawWindow();
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

void
make_delayed_update(void)
{
  grab_isp_info(0);
  if (current_isp>=num_isps)
    current_isp=0;
  DrawISPName();
  RedrawWindow();
  got_sched=0;
}

