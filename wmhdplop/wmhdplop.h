#ifndef WMHDPLOP_H
#define WMHDPLOP_H
#ifdef GKRELLM
#include <gkrellm2/gkrellm.h>
#endif
#include "config.h"
#include "dockapp_imlib2.h"
#include "procstat.h"
#include "devnames.h"


typedef struct {
  int nrow, ncol;
  int w;
  unsigned char **pre_cnt;
  char **intensity; /* > 0 for swap in, < 0 for swap out */
} SwapMatrix;

typedef struct IO_op_lst {
  enum { OP_READ, OP_WRITE } op;
  int n; /* "magnitude" of operation (log2 of kb/s) */
  int i,j; /* location of the IOMatrix */
  struct IO_op_lst *next;
} IO_op_lst;

#define CMAPSZ 256
typedef struct colormap {
  DATA32 p[CMAPSZ];
} cmap;

typedef struct {
  int w,h;
  int * __restrict * __restrict v; /* dim w+2, h+4 */
  cmap cm;
  IO_op_lst *ops;
} IOMatrix;

typedef struct {
  DockImlib2 *dock;
  Imlib_Font bigfont,smallfont;

  char *current_bigfont_name, *current_smallfont_name;
  unsigned update_display_delay_ms;
  unsigned update_stats_mult;
  
  unsigned char swap_matrix_luminosity, swap_matrix_lighting;

  SwapMatrix sm;
  IOMatrix iom;

  int nb_hd, nb_dev;
  enum {HD_ACTIVE, HD_STANDBY, HD_SLEEP, HD_UNKNOWN} *disk_power_mode;
  int *disk_temperature;
  
  int filter_hd, filter_part;
  int displayed_hd_changed;
  int reshape_cnt;
} App;

void reshape(int w, int h);
#ifdef GKRELLM
int hdplop_main(int w, int h, GdkDrawable *gkdrawable);
void gkrellm_hdplop_update(int update_options);
#endif
void change_displayed_hd(int dir);
void next_displayed_hd();
void prev_displayed_hd();
void init_fonts(App*);
DECL_GLOB_INIT(App *app, NULL);
#endif
