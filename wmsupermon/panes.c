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
*/

#include <stdlib.h>
#include <ctype.h>
#include "expr.h"
#include "panes.h"

#define _(S) S,sizeof(S)-1
struct {
  char *name;
  int name_length;
  int default_height;
  int possible_flags;
} pane_defs[]={
  { _("bar"),     1, P_LABEL | P_SMOOTH },
  { _("number"),  1, P_LABEL | P_FLOAT | P_SMOOTH },
  { _("percent"), 1, P_LABEL | P_SMOOTH },
  { _("graph"),   3, P_LABEL | P_SIZE | P_SMOOTH | P_SCALEDOWN | P_LOG },
  { 0, 0, 0, 0 }
};

struct {
  char *name;
  int   name_length;
  int   flag;
} options[]= {
  { _("-big"),           P_BIG },
  { _("-float"),         P_FLOAT },
  { _("-label"),         P_LABEL },
  { _("-log"),           P_LOG },
  { _("-medium"),        P_MEDIUM },
  { _("-scaledown"),     P_SCALEDOWN },
  { _("-small"),         P_SMALL },
  { _("-smooth"),        P_SMOOTH },
  { 0, 0, 0   }
};

#undef _

static const char *parse_regex(regex_t *preg, char *re, unsigned *flags);

/******************************************************************************/
/* panes functions                                                            */
/******************************************************************************/

#define streq(A,B) !strcasecmp((A),(B))
#define error(format, ...)                                      \
  do {                                                          \
    fprintf(stderr, "Line %d: " format "\n", line, ## __VA_ARGS__);  \
    return -1;                                                  \
  } while (0)
#define dup_keyword error("Duplicate keyword '%s'", keyword)

static char *next_token(char *s)
{
  char *p;
  while (*s && !isspace(*s)) s++; p=s;
  while (*s &&  isspace(*s)) s++; *p=0;
  return s;
}

int read_config_file(pane_desc panes[], int *pane_num, const int max_pane,
                     stat_dev  stats[], int *stat_num, const int stat_max,
                     wind_desc winds[], int *wind_num, const int wind_max)
{
  int i, cur_wind=-1, cur_stat=-1, cur_pane=0, cur_part=-1, line=0;
  char *home, buf[4096];
  FILE *cfg;

  if (!config) {
    config=buf;
    if (!(home=getenv("HOME"))) return 1;
    snprintf(buf, sizeof(buf), "%s/.wmsupermonrc", home);
  }
  cfg=fopen(config, "r");
  if(!cfg) return 1;

  while (fgets(buf, sizeof(buf), cfg)) {
    line++;
    if (buf[0] == '#') {
      /* comment */
    } else if (buf[0] == '[' && buf[1] == '[') {
      char *s=buf+2;
      while (s[0] && s[0] != '\n' && !(s[0] == ']' && s[1] == ']')) s++;
      if (s[0] != ']' || s[1] != ']') error("Syntax error");
      if (s-buf > WNAME_LEN+2)
        error("Too long name %s", buf);
      *s=0; s=buf+2;
      if (++cur_wind >= wind_max)
        error("Too many window definition, max is %i", wind_max);
      strncpy(winds[cur_wind].name, *s ? s : "wmsupermon", WNAME_LEN);
      winds[cur_wind].panes=panes+cur_pane;
      cur_part=0;
    } else if (buf[0] == '[') {
      cur_part=-1;
      if (cur_stat != -1) {
        if (!stats[cur_stat].source)
          error("Label [%s] has no source", stats[cur_stat].name);
      }
      if (++cur_stat >= stat_max)
        error("Too many stat definition, max is %i", stat_max);
      stats[cur_stat].scale=1;
      for (i=1; buf[i] !=']' && i <= NAME_LEN && buf[i] && buf[i] != '\n'; i++)
        stats[cur_stat].name[i-1]=buf[i];
      if (buf[i] != ']')
        error("Too long label %s", buf);
    } else if (cur_stat >= 0 || cur_part >=0) {
      char *s=buf, *keyword, *value;
      if (*s == '\n') s=keyword=value=0;
      else {
        while (isspace(*s)) s++;
        keyword=s;
        while (isalnum(*s) || *s == '/' || *s == '-' || *s == '%') s++;
        value=s;
        while (isspace(*s)) s++;
        if (*s++ != '=') error("Syntax error");
        while (isspace(*s)) s++;
        *value=0;
        value=s;
        s+=strlen(value)-1;
        while (isspace(*s)) s--;
        s[1]=0;
      }

      if (cur_part >=0) {
        if (!s) {
          if (panes[cur_pane][0].stat) { cur_pane++; winds[cur_wind].num_panes++; }
          cur_part=0;
        } else {
          pane_part *widget=&panes[cur_pane][cur_part];
          int possible_flags;

          if (cur_pane >= max_pane)
            error("Too many pane definition, max is %i", max_pane);

          /* label */
          for (i=0; i <= cur_stat; i++) {
            if (streq(keyword, stats[i].name))
              break;
          }
          if (i > cur_stat)
            error("unknown label %s", keyword);
          widget->stat=&stats[i];
          stats[i].flags|=F_USED;

          /* widget name */
          s=next_token(value);
          for (i=0; pane_defs[i].name; i++)
            if (streq(value, pane_defs[i].name))
              break;
          if (!pane_defs[i].name)
            error("unknown widget %s", value);
          widget->type=i;

          /* options */
          possible_flags=pane_defs[widget->type].possible_flags;
          for ( s=next_token(value=s); *value; s=next_token(value=s)) {
            for (i=0; options[i].name; i++)
              if (streq(value, options[i].name))
                break;
            if (!options[i].name)
              error("Unknown option %s", value);
            if (!(possible_flags & options[i].flag))
              error("Widget %s does not support the option %s",
                  pane_defs[widget->type].name, value);
            if (options[i].flag & P_SIZE && widget->flags & P_SIZE)
              error("Duplicate size option (-small, -medium, -big)");
            widget->flags|=options[i].flag;
          }

          if (widget->flags & P_SMALL)
            widget->height=1;
          else if (widget->flags & P_MEDIUM)
            widget->height=2;
          else if (widget->flags & P_BIG)
            widget->height=4;
          else
            widget->height=pane_defs[widget->type].default_height;

          if (widget->flags & P_SMOOTH && !widget->stat->smooth)
            widget->stat->smooth=calloc(SMOOTH_SIZE-1, sizeof(double));
          if (widget->type == PTGraph) {
            history **x=&widget->stat->hist[widget->flags & P_HIST];
            if (!*x) {
              *x=calloc(1, sizeof(history));
              (*x)->max=1;
            }
          }

          cur_part++;
        }
      } else if (!s) continue;
      else if (streq(keyword, "source")) {
        if (stats[cur_stat].source) dup_keyword;
        stats[cur_stat].source=strdup(value);
      } else if (streq(keyword, "regex")) { const char *err;
        if (stats[cur_stat].expr) dup_keyword;
        err=parse_regex(&stats[cur_stat].regex, value, &stats[cur_stat].flags);
        if (err)
          error("Regex: %s\n", err);
        stats[cur_stat].expr=yy_expr;
        stats[cur_stat].diff_old=calloc(yy_ndiff, sizeof(double));
        stats[cur_stat].diff_new=calloc(yy_ndiff, sizeof(double));
        stats[cur_stat].sum_acc =calloc(yy_nsum,  sizeof(double));
        stats[cur_stat].nsum=yy_nsum;
      } else if (streq(keyword, "action")) {
        if (stats[cur_stat].action) dup_keyword;
        stats[cur_stat].action=strdup(value);
      } else if (streq(keyword, "interval")) {
        stats[cur_stat].hist_update_interval=
        stats[cur_stat].update_interval=atoi(value);
      } else if (streq(keyword, "scale")) {
        stats[cur_stat].scale=atof(value);
      } else if (streq(keyword, "range")) {
        if (sscanf(value, " %lf .. %lf", &stats[cur_stat].min, &stats[cur_stat].max) != 2)
          error("Syntax error.\n");
        if (!(stats[cur_stat].max-=stats[cur_stat].min))
          error("Invalid range.\n");
      } else error("Syntax error.\n");
    } else if (*buf != '\n')
      error("Syntax error.\n");
  }

  if (cur_wind < 0)
      error("No window definitions.\n");
  if (cur_pane < 0)
      error("No pane definitions.\n");
  if (cur_stat < 0)
      error("No stat definitions.\n");

  *pane_num=cur_pane;
  *stat_num=cur_stat+1;
  *wind_num=cur_wind+1;
  return 0;
}

static char bracket(char c)
{
  return c == '<' ? '>' :
         c == '{' ? '}' :
         c == '[' ? ']' :
         c == '(' ? ')' : c;
}

static const char *parse_regex(regex_t *preg, char *re, unsigned *flags)
{
  char bra, ket, *s, *subs;
  int cflags=REG_EXTENDED, err;
  static char error_buf[256];

  bra=*re++;
  ket=bracket(bra);
  for (s=re; *s && *s != ket; s++);
  if (*s != ket) return "Not terminated regex";
  *s++=0;
  if (bra != ket) {
    bra=*s++;
    if (!bra) return "Missing substitution";
    ket=bracket(bra);
  }
  for (subs=s; *s && *s != ket; s++);
  if (*s != ket) return s == subs ? "Missing substitution" : "Not terminated substitution";
  for (*s++=0; *s; s++)
    if (*s == 'i') cflags|=REG_ICASE;    else
    if (*s == 's') *flags|=F_SINGLE_LINE; else
    if (*s == 'd') *flags|=F_DEBUG; else
    return "Trailing characters";
  err=regcomp(preg, re, cflags);
  if (err) {
    regerror(err, preg, error_buf, sizeof(error_buf));
    regfree(preg);
    return error_buf;
  }

  yy_str=subs; yy_nsum=yy_ndiff=0;
  strcpy(error_buf, "Expression: ");
  yy_err=error_buf+12;
  if (yyparse()) return error_buf;
  return 0;
}

