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

#include <regex.h>

#define MAX_VAR 10

typedef struct {
  char *str;
  regmatch_t *pmatch;
  double *diff_old, *diff_new, *sum_acc;
} Econtext;

typedef struct st_expr Expr;
struct st_expr {
  double (*eval)(Expr *, Econtext *);
  union { int var; double num; Expr *arg[3]; } val;
};

extern char *yy_err, *yy_str;
extern Expr *yy_expr;
extern int yy_nsum, yy_ndiff;

int yyparse(void);

