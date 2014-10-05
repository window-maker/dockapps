/* Copyright (C) 2006 Sergei Golubchik

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   as published by the Free Software Foundation

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

/*
  originally based on:
    WMgMon - Window Maker Generic Monitor
    by Nicolas Chauvat <nico@caesium.fr>
  which was based on
    WMMon from Antoine Nulle and Martijn Pieterse.
*/

#ifndef __PANES_H
#define __PANES_H

#include <string.h>
#include <stdio.h>
#include <X11/Xlib.h>

#include "stat_dev.h"

/******************************************************************************/
/* pane structure                                                             */
/******************************************************************************/
#define PTBar                 0
#define PTNumber              1
#define PTPercent             2
#define PTGraph               3

extern char *config;

typedef struct
{
  stat_dev *stat;
  int       type;
  int       flags;
  int       height;
}
pane_part;

#define PANE_PARTS 4
typedef pane_part pane_desc[PANE_PARTS];

#define WNAME_LEN 32
typedef struct
{
  char       name[WNAME_LEN+1];
  pane_desc *panes;
  int        num_panes, cur_pane;
  Window     w[2]; /* normal and iconic */
  Pixmap     pixmap, mask;
}
wind_desc;

/******************************************************************************/
/* pane functions                                                             */
/******************************************************************************/

int read_config_file(pane_desc panes[], int *pane_num, const int max_pane,
                     stat_dev  stats[], int *stat_num, const int stat_max,
                     wind_desc winds[], int *wind_num, const int wind_max);
/*
 * END
 */
#endif
