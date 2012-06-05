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
  originally based on
    WMgMon - Window Maker Generic Monitor
    by Nicolas Chauvat <nico@caesium.fr>
  which was based on
    WMMon from Antoine Nulle and Martijn Pieterse.
*/

#ifndef __STAT_DEV_H
#define __STAT_DEV_H

#include <string.h>
#include <stdio.h>

#define INIT 0
#define NORMAL 1
#define CLOSE 2

#define TEMP_SIZE     128
#define STRARG_LEN    255
#define NAME_LEN        3
#define MIN_FILE_LEN 8192
/******************************************************************************/
/* stat_dev structure and related functions                                   */
/******************************************************************************/

#define HIST_SIZE   55
#define HIST_LAST   54
#define SMOOTH_SIZE  8

/* note that P_SMOOTH and P_LOG *must* be 0 and 1 respectively */
#define P_SMOOTH  (1L << 0)
#define P_LOG     (1L << 1)
#define P_LABEL   (1L << 2)
#define P_FLOAT   (1L << 3)
#define P_SMALL   (1L << 4)
#define P_MEDIUM  (1L << 5)
#define P_BIG     (1L << 6)
#define P_SCALEDOWN (1L << 7)

#define P_SIZE    (P_SMALL | P_MEDIUM | P_BIG)
/* the following is an index in stat_dev::hist[] array */
#define P_HIST    (P_SMOOTH | P_LOG)

#define F_SINGLE_LINE 1 /* regex flag */
#define F_DEBUG       2
#define F_USED        4

typedef struct {
  double data[HIST_SIZE];
  double max;
  unsigned count;
} history;

typedef struct {
  char  name[NAME_LEN+1]; /* cpu, mem, swap, i/o, etc. */
  double scale;
  double min,max;         /* range for the bar/graph */
  unsigned flags;         /* F_xxx from above */
  char *action;
  char *source;

  regex_t regex;
  Expr *expr;
  double *diff_old, *diff_new, *sum_acc;
  unsigned nsum;          /* number of elements in sum_acc */

  double value[P_SMOOTH+1];     /* "real-time" stat */
  double *smooth;               /* 0 if unused */
  history *hist[P_HIST+1];      /* 0 if unused */

  unsigned update_interval;
  unsigned next_update;
  unsigned hist_update_interval;
  unsigned hist_next_update;
} stat_dev;

void stat_dev_init(stat_dev * st);
void stat_dev_initstat(stat_dev * st);
void stat_dev_update_history (stat_dev * st);
void update_stat(stat_dev *);

/*
 * END
 */
#endif
