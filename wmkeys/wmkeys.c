/* $Id$ -*- C -*-
 *
 * wmkeys.c
 * Version: 0.1
 * A Window Maker/AfterStep dock application for switching X key sets
 * on the fly.
 *
 * Copyright (C) 1999 Eric Crampton <EricCrampton@worldnet.att.net>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * Reference source: wmtime dock app
 *
 */

/*
 * Standard C includes
 */

#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/param.h>
#include <sys/types.h>

/*
 * X11 includes
 */

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

/*
 * Window Maker includes
 */

#include "libdockapp/wmgeneral.h"
#include "libdockapp/misc.h"

/*
 * Pixmap, bitmap includes
 */

#include "wmkeys-master.xpm"
#include "wmkeys-mask.xbm"

/*
 * Global variables
 */

char *ProgName;

typedef struct {
  char* name;
  char* filename;
} keysym_config;

keysym_config configs[10];
int num_configs;
int current_config;

/*
 * Function prototypes
 */

void wmkeys_routine(int, char **);
void read_config();
void draw_string(char* s);
void enable_configuration(int n);

/*
 * Main
 */

int main(int argc, char *argv[])
{
  num_configs = 0;
  current_config = 0;

  ProgName = argv[0];
  if (strlen(ProgName) >= 6)
    ProgName += (strlen(ProgName) - 6);

  read_config();
  wmkeys_routine(argc, argv);

  return 0;
}

/*
 * wmkeys_routine
 */

void wmkeys_routine(int argc, char **argv)
{
  int i;
  XEvent Event;
  int but_stat = -1;

  openXwindow(argc, argv, wmkeys_master_xpm, wmkeys_mask_bits, 64, 64);
  enable_configuration(0);

  /* add mouse region */
  AddMouseRegion(0, 5, 5, 58, 122);

  while (1) {
    waitpid(0, NULL, WNOHANG);
    RedrawWindow();

    while (XPending(display)) {
      XNextEvent(display, &Event);
      switch (Event.type) {
      case Expose:
	RedrawWindow();
	break;
      case DestroyNotify:
	XCloseDisplay(display);
	exit(0);
	break;
      case ButtonPress:
	but_stat = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
	break;
      case ButtonRelease:
	i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
	if (but_stat == i && but_stat >= 0) {
	  switch (but_stat) {
	  case 0:
	    current_config++;
	    if(current_config == num_configs)
	      current_config = 0;
	    enable_configuration(current_config);
	    break;
	  }
	}
	break;
      }
    }

    /* Sleep 0.3 seconds */
    usleep(300000L);
  }
}

/*
 * draw_string()
 *
 *   Draws string s in the LCD display portion of the window.
 */

void draw_string(char* s)
{
  int i;
  for(i=0; i<strlen(s) && i<8; i++) {
    copyXPMArea((toupper(s[i]) - 'A')*6, 74, 6, 9, 5+(i*6), 49);
  }
}

/*
 * getline()
 */

int getline_wmkeys(FILE* pfile, char* s, int lim)
{
  int c = 0, i;
  for(i=0; i<lim-1 && (c=fgetc(pfile)) != EOF && c!='\n'; ++i) {
    s[i] = c;
  }
  if(c == '\n') {
    s[i] = c;
    ++i;
  }
  s[i] = '\0';
  return i;
}

/*
 * read_config()
 *
 *   Reads the appropriate configuration file from ${HOME}/.wmkeysrc
 *   or from /etc/wmkeysrc, in that order.
 */

void read_config()
{
  char* rcfilename;
  char* home_var;
  FILE* pfile;
  char key[256], value[256];

  home_var = getenv("HOME");

  rcfilename = malloc(sizeof(char) * strlen(home_var) + 11 /* / .wmkeysrc + NULL*/);
  strcpy(rcfilename, home_var);
  strcat(rcfilename, "/.wmkeysrc");

  pfile = fopen(rcfilename, "r");
  if(pfile == NULL) {
    /* try to open system-wide configuration */
    strcpy(rcfilename, "/etc/wmkeysrc");
    pfile = fopen(rcfilename, "r");

    if(!pfile) {
      fprintf(stderr, "Error: cannot open ${HOME}/.wmkeysrc or /etc/wmkeysrc\n");
      exit(1);
    }
  }

  while(!feof(pfile)) {
    getline_wmkeys(pfile, key, 256);

    if(!feof(pfile)) {
      getline_wmkeys(pfile, value, 256);

      configs[num_configs].name = malloc(sizeof(char)*strlen(key)+1);
      strcpy(configs[num_configs].name, key);
      configs[num_configs].filename = malloc(sizeof(char)*strlen(value)+1);
      strcpy(configs[num_configs].filename, value);
      num_configs++;
    }
  }
  if(num_configs == 0) {
    fprintf(stderr, "Error: no configurations, exiting.\n");
    exit(1);
  }
}

/*
 * enable_configuration()
 *
 *   Enables configuration number n.
 */

void enable_configuration(int n)
{
  char syscmd[256];
  draw_string(configs[n].name);
  strcpy(syscmd, "xmodmap ");
  strcat(syscmd, configs[n].filename);
  system(syscmd);
  RedrawWindow();
}
