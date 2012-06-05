/* Copyright (C) 2006 Sergei Golubchik, Nicolas Chauvat

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
    WMMon by Antoine Nulle and Martijn
*/

#define Wmsupermon_VERSION "1.2.2"
#define Wmsupermon_VERSION_DATE "2007/06/23"

#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <regex.h>
#include <assert.h>

#include "dockapp.h"
#include "wmsupermon-master.xpm"
#include "expr.h"
#include "stat_dev.h"
#include "panes.h"

/* define layout constants */
#define ROW_HEIGHT 14
#define LABEL_WIDTH 25

/******************************************************************************/
/* master xpm coordinates                                                     */
/******************************************************************************/

#define X_ALPHA 1
#define Y_ALPHA 1
#define X_NUM   1
#define Y_NUM  11

#define X_LABEL  1
#define Y_LABEL 21
#define W_LABEL 56
#define H_LABEL (ROW_HEIGHT-1)

#define X_NOLABEL  1
#define Y_NOLABEL 77
#define W_NOLABEL W_LABEL
#define H_NOLABEL H_LABEL

#define X_DIAG  1
#define Y_DIAG 34
#define W_DIAG W_LABEL
#define H_SDIAG (ROW_HEIGHT*1)
#define H_MDIAG (ROW_HEIGHT*2)
#define H_DIAG  (ROW_HEIGHT*3)
#define H_BDIAG (ROW_HEIGHT*4)

#define X_BLANK_SBAR (118-2)
#define Y_BLANK_SBAR (127-50)
#define X_SBAR  (118-2)
#define Y_SBAR  (138-50)
#define W_SBAR  30
#define H_SBAR  10
#define X_BLANK_LBAR (62-2)
#define Y_BLANK_LBAR (127-50)
#define X_LBAR  (62-2)
#define Y_LBAR  (138-50)
#define W_LBAR  54
#define H_LBAR  H_SBAR

#define X_DOT      150
#define COLOR_DOT   27
#define BRIGHT_DOT  28
#define BACK_DOT    29
#define LINE_DOT    30

/******************************************************************************/
/* global variables                                                           */
/******************************************************************************/

Pixmap work_pixmap, master_pixmap;
GC gc;

char *displayName="", *config=0;

static DAProgramOption options[]={
    {"-d", "--display", "display to use", DOString, False, {&displayName} },
    {"-c", "--config",  "path to config file", DOString, False, {&config} }
};

#define WIND_MAX 4
wind_desc winds[WIND_MAX];

/* define panes */
#define PANE_MAX (8*WIND_MAX)
pane_desc pane[PANE_MAX];

int rotate_interval=4;
int next_rotate=0;
int current_time=0;
int wind_num=0;

#define STAT_DEV_MAX 20
stat_dev stat_device[STAT_DEV_MAX];

#define APP_WIDTH  56
#define APP_HEIGHT 56

/******************************************************************************/
/* prototypes                                                                 */
/******************************************************************************/

void draw_label(const char *, int, int);
void draw_bar(pane_part *, int, int);
void draw_graph(pane_part *, int, int, int, int);
void draw_current_frame(pane_part *);
void draw_current_pane(pane_part *);

static void click(Window w, int button, int state, int x, int y);

char mask_bits[APP_HEIGHT*APP_WIDTH];

/******************************************************************************/
/* main                                                                       */
/******************************************************************************/

int main (int argc, char *argv[])
{
  unsigned w, h;
  int i, device_num=0, pane_num=0;
  DACallbacks callbacks={ NULL, &click, NULL, NULL, NULL, NULL, NULL };

  DAParseArguments(argc, argv, options,
                   sizeof(options)/sizeof(DAProgramOption),
                   "wmsupermon", Wmsupermon_VERSION);

  /* init structures */
  for (i=0; i<STAT_DEV_MAX; i++)
    stat_dev_init(&stat_device[i]);

  /* read config file and populate structures */
  i=read_config_file(pane, &pane_num, PANE_MAX,
                     stat_device, &device_num, STAT_DEV_MAX,
                     winds, &wind_num, WIND_MAX);
  if (i < 0) /* error in config file */
    exit(1);
  if (i > 0) /* no config file */
    exit(2); /* TODO: setup default panes */

  /* init timing stuff */
  next_rotate=time(0) + rotate_interval;

  for (i=0; i<device_num; i++)
    stat_dev_initstat(&stat_device[i]);

  
  for (i=0; i < wind_num; i++) {
    DAInitialize(displayName, winds[i].name, APP_WIDTH, APP_HEIGHT, argc, argv, winds[i].w);
    DASetCallbacks(winds[i].w, &callbacks);

    winds[i].pixmap=work_pixmap=DAMakePixmap(winds[i].w);
    DAMakePixmapFromData(winds[i].w, wmsupermon_master_xpm, &master_pixmap, 0, &w, &h);

    DAShow(winds[i].w);
    winds[i].cur_pane=0;
  }

  gc=DefaultGC(DADisplay, DefaultScreen(DADisplay));

  XSetForeground(DADisplay, gc, BlackPixel(DADisplay, DefaultScreen(DADisplay)));

  for (i=0; i < wind_num; i++) {
    XFillRectangle(DADisplay, work_pixmap=winds[i].pixmap, gc, 0, 0, APP_WIDTH, APP_HEIGHT);
    draw_current_frame(winds[i].panes[winds[i].cur_pane]);
    winds[i].mask=XCreateBitmapFromData(DADisplay, winds[i].w[0], mask_bits, APP_WIDTH, APP_HEIGHT);
    DASetShape(winds[i].w, winds[i].mask);
  }

  /* mainloop */
  while (1) {
    XEvent ev;

    current_time=time(0);

    /* change pane ? */
    if (current_time > next_rotate) {
      next_rotate=current_time+rotate_interval;
      for (i=0; i < wind_num; i++) {
        if (winds[i].num_panes > 1) {
          winds[i].cur_pane++;
          if (winds[i].cur_pane >= winds[i].num_panes)
            winds[i].cur_pane=0;
          /* redraw frame */
          work_pixmap=winds[i].pixmap;
          draw_current_frame(winds[i].panes[winds[i].cur_pane]);
          XFreePixmap(DADisplay, winds[i].mask);
          winds[i].mask=XCreateBitmapFromData(DADisplay, winds[i].w[0], mask_bits, APP_WIDTH, APP_HEIGHT);
          DASetShape(winds[i].w, winds[i].mask);
        }
      }
    }

    for (i=0; i < device_num; i++) {
      stat_dev *st=&stat_device[i];
      /* if time has come, update stats */
      if  (current_time >= st->next_update) {
        int j;
        history **h=st->hist;
        update_stat(st);
        if (st->smooth) {
          double v=st->smooth[0];
          for (j=0; j < SMOOTH_SIZE-2; j++)
            v+=(st->smooth[j]=st->smooth[j+1]);
          v+=(st->smooth[j]=st->value[0]);
          st->value[1]=v/SMOOTH_SIZE;
        }
        for (j=0; j <= P_HIST; j++, h++) {
          if (!*h) continue;
          (*h)->data[HIST_LAST]+= st->value[j & P_SMOOTH];
          (*h)->count++;
        }
        st->next_update=current_time+st->update_interval;
      }
      /* if time has come, update history */
      if  (current_time > st->hist_next_update) {
        stat_dev_update_history(st);
        if (st->hist_next_update < current_time)
          st->hist_next_update=current_time;
        st->hist_next_update+=st->hist_update_interval;
      }
    }

    /* update view */
    for (i=0; i < wind_num; i++) {
      work_pixmap=winds[i].pixmap;
      draw_current_pane(winds[i].panes[winds[i].cur_pane]);
      XCopyArea(DADisplay, work_pixmap, winds[i].w[0], gc, 0, 0,
          APP_WIDTH, APP_HEIGHT, 0, 0);
    }

    /* handle all pending X events */
    while (XPending(DADisplay)) {
      XNextEvent(DADisplay, &ev);
      for (i=0; i < wind_num; i++)
        DAProcessEvent(winds[i].w, &ev);
    }

    /* short sleep */
    usleep(250000L);

  }

  return 0;
}

/******************************************************************************/
/* mouse events / callbacks                                                   */
/******************************************************************************/

static void click(Window w, int button, int state, int x, int y)
{
  pane_part *pane;
  int p=y, i;
  for (i=0; i < wind_num; i++) {
    if (winds[i].w[0] != w) continue;
    pane=winds[i].panes[winds[i].cur_pane];

    for (p=0; pane[p].stat && p < PANE_PARTS; p++)
      if ((y-= ROW_HEIGHT*pane[p].height) < 0) break;
    if (y < 0 && pane[p].stat->action)
      if (!fork()) {
        system(pane[p].stat->action);
        exit(0);
      }
    break;
  }
}

/******************************************************************************/
/* draw functions                                                             */
/******************************************************************************/

void copy_area(int src_x, int src_y, int width, int height, int d_x, int d_y)
{
  XCopyArea(DADisplay, master_pixmap, work_pixmap, gc,
    src_x, src_y, width, height, d_x, d_y);
}

void draw_current_frame(pane_part *pane)
{
  int i, j;
  int row=0;

  XSetForeground(DADisplay, gc, BlackPixel(DADisplay, DefaultScreen(DADisplay)));
  XFillRectangle(DADisplay, work_pixmap, gc, 0, 0, APP_WIDTH, APP_HEIGHT);

  for (j=0; j < sizeof(mask_bits); j++) mask_bits[j]=0xff;
  for (i=0; (row<PANE_PARTS) && (i<PANE_MAX) && pane[i].stat; i++) {
    switch(pane[i].height) {
    case 1:
       if (pane[i].flags & P_LABEL) {
         copy_area (X_LABEL, Y_LABEL, W_LABEL, H_LABEL, 0, row*ROW_HEIGHT);
         draw_label(pane[i].stat->name, 1, 2+row*ROW_HEIGHT);
         for (j=0; j < ROW_HEIGHT; j++)
           mask_bits[(row*ROW_HEIGHT+j)*W_LABEL/8+(LABEL_WIDTH-3)/8] &=
             ~(1 << (LABEL_WIDTH-3) % 8);
       } else
         copy_area (X_NOLABEL, Y_NOLABEL, W_NOLABEL, H_NOLABEL, 0, row*ROW_HEIGHT);
       break;
    case 2:
      copy_area (X_DIAG, Y_DIAG, W_DIAG, H_MDIAG-5, 0, row*ROW_HEIGHT);
      copy_area (X_DIAG, Y_DIAG+H_DIAG-H_MDIAG+5, W_DIAG, H_MDIAG-5, 0, row*ROW_HEIGHT+5);
      break;
    case 3:
      copy_area (X_DIAG, Y_DIAG, W_DIAG, H_DIAG, 0, row*ROW_HEIGHT);
      break;
    case 4:
      copy_area (X_DIAG, Y_DIAG, W_DIAG, H_DIAG-5, 0, row*ROW_HEIGHT);
      copy_area (X_DIAG, Y_DIAG+5, W_DIAG, H_DIAG-5, 0, row*ROW_HEIGHT+H_BDIAG-H_DIAG+5);
      break;
    default:
      assert(0);
    }
    row+=pane[i].height;
    for (j=0; j < W_LABEL/8; j++) mask_bits[(row*ROW_HEIGHT-1)*W_LABEL/8+j]=0;
  }
}

static struct { int w,h; } graph_box[]=
{
  {0,0},
  {W_DIAG-2, H_SDIAG-2},
  {W_DIAG-2, H_MDIAG-2},
  {W_DIAG-2, H_DIAG-2},
  {W_DIAG-2, H_BDIAG-2}
};
    
void draw_current_pane(pane_part *pane)
{
  int i;
  int row=0, label_width;
  char str[6]={0};

  for (i=0; (row<PANE_PARTS) && pane->stat; pane++, i++) {
    label_width=pane->flags & P_LABEL && pane->height == 1 ?
                LABEL_WIDTH-1 : 0;
    switch(pane->type) {
    case PTBar:
      draw_bar(pane, label_width+1, 1+row*ROW_HEIGHT);
      break;
    case PTPercent:
      sprintf(str, " %*.*f%%", label_width ? 2 : 5, label_width ? 0 : 2,
          100*pane->stat->value[pane->flags & P_SMOOTH]);
      draw_label(str, label_width+4, 2+row*ROW_HEIGHT);
      break;
    case PTNumber:
      sprintf(str,
          pane->flags & P_FLOAT ? "%2$*1$.*1$g" : "%2$*1$.0f",
          label_width ? 4 : 6, pane->stat->value[pane->flags & P_SMOOTH]);
      draw_label(str, label_width+4, 2+row*ROW_HEIGHT);
      break;
    case PTGraph:
      draw_graph(pane, label_width+1, 1+row*ROW_HEIGHT,
          graph_box[pane->height].w-label_width,
          graph_box[pane->height].h);
      if (pane->flags & P_LABEL && pane->height > 1)
        draw_label(pane->stat->name, 1, 1+row*ROW_HEIGHT);
      break;
    default:
      assert(0);
      break;
    }
    row+=pane->height;
  }
}

void draw_label(const char *str, int x, int y)
{
  int i, c;

  for (i=0; str[i] != '\0'; i++) {
    if (isalnum(str[i])) {
      c=toupper(str[i]);
      if (c >= 'A' && c <= 'Z') {
        c -= 'A';
        copy_area(X_ALPHA+c*6, Y_ALPHA, 6, 8, x+i*6, y);
      }
      else {
        c -= '0';
        copy_area(X_NUM+c*6, Y_NUM, 6, 8, x+i*6, y);
      }
    }
    else {
      switch(str[i]) {
      case '/' :
        copy_area(72-2, 61-50, 6, 8, x+i*6, y);
        break;
      case '.' :
        copy_area(84-2, 61-50, 6, 8, x+i*6, y);
        break;
      case '%' :
        copy_area(66-2, 61-50, 6, 8, x+i*6, y);
        break;
      case '-' :
        copy_area(89-2, 61-50, 6, 8, x+i*6, y);
        break;
      default:
        copy_area(78-2, 61-50, 6, 8, x+i*6, y);
      }
    }
  }
}

static const struct {
  int x_blank, y_blank, x, y, h, w;
} bar_coords[]={
  {X_BLANK_SBAR, Y_BLANK_SBAR, X_SBAR, Y_SBAR, H_SBAR, W_SBAR},
  {X_BLANK_LBAR, Y_BLANK_LBAR, X_LBAR, Y_LBAR, H_LBAR, W_LBAR}
};

void draw_bar (pane_part *widget, int x, int y)
{
  stat_dev *st=widget->stat;
  int value;
  int label= x==1;

   /* copy blank bar */
   copy_area(bar_coords[label].x_blank, bar_coords[label].y_blank,
             bar_coords[label].w, bar_coords[label].h, x, y);

   /* compute value */
   if (st->max)
     value=(st->value[widget->flags & P_SMOOTH]-st->min)/st->max *
           bar_coords[label].w;
   else
     value=st->value[widget->flags & P_SMOOTH] * bar_coords[label].w;
   if (value > bar_coords[label].w)
      value=bar_coords[label].w;
   if (value < 0)
      value=0;

   /* copy part of color bar */
   copy_area(bar_coords[label].x, bar_coords[label].y,
       value, bar_coords[label].h, x, y);
}

/* Below: (0) - to disable, (st->max) to enable */
#define USER_RANGE (st->max)
void draw_graph (pane_part *widget, int x, int y, int width, int height)
{
   stat_dev *st=widget->stat;
   int row, column, top, x0, y0;
   history *hist=st->hist[widget->flags & P_HIST];

   x0=width - HIST_LAST + x;
   y0=y + height - 1;

   if (!USER_RANGE) {
     /* compute scale */
     if (widget->flags & P_SCALEDOWN) {
        double hist_max=0;
        for (column=HIST_LAST-width; column < HIST_LAST; column++) {
           if (hist->data[column] > hist_max)
              hist_max=hist->data[column];
        }
        if (hist_max > 0)
          hist->max=hist_max;
     }
     else if (hist->data[HIST_LAST-1] > hist->max)
       hist->max=hist->data[HIST_LAST-1];
   }
   /* draw bars (top is lighter) */
   for (column=HIST_LAST-width; column < HIST_LAST; column++) {
     if (USER_RANGE)
       top=(hist->data[column] - st->min) * height / st->max;
     else
       top=hist->data[column] * height / hist->max;

       for (row=0; row < height; row++) {
         if (row < top - 3)
           copy_area (X_DOT, COLOR_DOT, 1, 1, x0+column, y0-row);
         else if (row < top)
           copy_area (X_DOT, BRIGHT_DOT, 1, 1, x0+column, y0-row);
         else
           copy_area (X_DOT, BACK_DOT, 1, 1, x0+column, y0-row);
      }
   }
}
#undef USER_RANGE

