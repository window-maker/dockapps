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
*/

#include <stdlib.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "expr.h"
#include "stat_dev.h"

/******************************************************************************/
/* stat_dev structure and related functions                                   */
/******************************************************************************/

void stat_dev_init(stat_dev * st)
{
   bzero(st, sizeof(*st));
   st->update_interval=1;
   st->hist_update_interval=5;
}

void stat_dev_initstat (stat_dev * st)
{
  if (st->flags & F_USED)
    update_stat(st); /* to get first value for DIFF */
  else
    st->next_update=~0; /* that is never */
}

void stat_dev_update_history (stat_dev *st)
{
  int i;
  for (i=0; i <= P_HIST; i++) {
    int j;
    history *hist=st->hist[i];
    if (!hist) continue;

    /* history is updated more often than the value is sampled. Use previous
       value to avoid a gap in the history */
    if (hist->count == 0)
      hist->data[HIST_LAST]=hist->data[HIST_LAST-1];

    /* we probably sampled several times since last time we updated,
       let's normalize value by dividing by the number of times we sampled */
    if (hist->count > 1)
       hist->data[HIST_LAST] /= hist->count;

    if (i & P_LOG && hist->count)
      hist->data[HIST_LAST]=hist->data[HIST_LAST]*3 <= 1 ?
        0 : log(hist->data[HIST_LAST]*3);

    /* shift values to the left (discard his[0]) */
    for (j=1; j < HIST_SIZE; j++)
       hist->data[j-1]=hist->data[j];

     /* reset sample */
     hist->data[HIST_LAST]=0;
     hist->count=0;
  }
}

/******************************************************************************/
/* update stat functions                                                      */
/******************************************************************************/

/* returns 1 on a match */
static int do_regex(stat_dev *st, char *str) {
  regmatch_t pmatch[MAX_VAR];
  Econtext ctx={str, pmatch, st->diff_old, st->diff_new, st->sum_acc};

  if (regexec(&st->regex, str, MAX_VAR, pmatch, 0))
    return 0;

  st->value[0]=st->expr->eval(st->expr, &ctx)/st->scale;
  return 1;
}

static void stat_perror(stat_dev *st, char *op)
{
  fprintf(stderr, "[%s]: %s() failed: %s\n", st->name, op, strerror(errno));
}

void update_stat(stat_dev *st)
{
  char *buf;
  double *tmp;
  FILE *f;
  int matched=0;

  f= (st->source[0] == '!') ?
      popen(st->source+1, "r") : fopen(st->source, "r");
  
  if (!f) {
    stat_perror(st, "open");
    return;
  }

  if (!st->expr) {
    double value;
    if (fscanf(f, "%lf", &value) == 1)
      st->value[0]=value/st->scale;
    goto done;
  }

  /* ok, doing regexp */
  if (st->flags & F_SINGLE_LINE) {
    struct stat stat;
    if (fstat(fileno(f), &stat) < 0) { stat_perror(st, "fstat"); goto ret; }
    if (stat.st_size < MIN_FILE_LEN)
      stat.st_size=MIN_FILE_LEN; /* because /proc files may have zero length */
    buf=malloc(stat.st_size); /* and it'd better be small! */
    if (!buf) { stat_perror(st, "malloc"); goto ret; }
    fread(buf, stat.st_size,  1, f);
    matched=do_regex(st, buf);
    free(buf);
    goto done;
  }

  buf=alloca(TEMP_SIZE);
  if (st->nsum) bzero(st->sum_acc, st->nsum*sizeof(double));
  while (fgets(buf, TEMP_SIZE, f)) {
    if ((matched|=do_regex(st, buf)) && !st->nsum)
      break;
  }

done:
  if (st->flags & F_DEBUG) {
    if (matched)
      printf("%-4s: %f\n", st->name, st->value[0]);
    else
      printf("%-4s: match failed\n", st->name);
  }

  tmp=st->diff_old; st->diff_old=st->diff_new; st->diff_new=tmp;

ret:
  (st->source[0] == '!') ?  pclose(f) : fclose(f);
}

