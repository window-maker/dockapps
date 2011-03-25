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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

%{

#include <strings.h>
#include <stdlib.h>
#include "expr.h"

int yylex (void);
void yyerror (char const *);

char *yy_err, *yy_str;
Expr *yy_expr;
int yy_nsum, yy_ndiff;

static double e_num(Expr *me, Econtext *ctx) {
  return me->val.num;
}

static double e_var(Expr *me, Econtext *ctx) {
  char c, *s, *e; double v;
  if (ctx->pmatch[me->val.var].rm_so < 0) return 0;
  s=ctx->str+ctx->pmatch[me->val.var].rm_so;
  e=ctx->str+ctx->pmatch[me->val.var].rm_eo;
  c=*e; *e=0; v=strtod(s, 0); *e=c;
  return v;
}

static double e_if(Expr *me, Econtext *ctx) {
  return me->val.arg[0]->eval(me->val.arg[0], ctx) ?
         me->val.arg[1]->eval(me->val.arg[1], ctx) :
         me->val.arg[2]->eval(me->val.arg[2], ctx) ;
}

static double e_lt(Expr *me, Econtext *ctx) {
  return me->val.arg[0]->eval(me->val.arg[0], ctx) < me->val.arg[1]->eval(me->val.arg[1], ctx);
}

static double e_gt(Expr *me, Econtext *ctx) {
  return me->val.arg[0]->eval(me->val.arg[0], ctx) > me->val.arg[1]->eval(me->val.arg[1], ctx);
}

static double e_add(Expr *me, Econtext *ctx) {
  return me->val.arg[0]->eval(me->val.arg[0], ctx) + me->val.arg[1]->eval(me->val.arg[1], ctx);
}

static double e_sub(Expr *me, Econtext *ctx) {
  return me->val.arg[0]->eval(me->val.arg[0], ctx) - me->val.arg[1]->eval(me->val.arg[1], ctx);
}

static double e_mul(Expr *me, Econtext *ctx) {
  return me->val.arg[0]->eval(me->val.arg[0], ctx) * me->val.arg[1]->eval(me->val.arg[1], ctx);
}

static double e_div(Expr *me, Econtext *ctx) {
  return me->val.arg[0]->eval(me->val.arg[0], ctx) / me->val.arg[1]->eval(me->val.arg[1], ctx);
}

static double e_sum(Expr *me, Econtext *ctx) {
  int slot=(long)me->val.arg[1];
  return ctx->sum_acc[slot]+=me->val.arg[0]->eval(me->val.arg[0], ctx);
}

static double e_diff(Expr *me, Econtext *ctx) {
  double e, v;
  int slot=(long)me->val.arg[1];
  ctx->diff_new[slot]=e=me->val.arg[0]->eval(me->val.arg[0], ctx);
  v=e-ctx->diff_old[slot];
  return v;
}

static Expr *new_num(double val) {
  Expr *e=malloc(sizeof(Expr));
  e->eval=e_num;
  e->val.num=val;
  return e;
}

static Expr *new_var(int var) {
  Expr *e=malloc(sizeof(Expr));
  e->eval=e_var;
  e->val.var=var;
  return e;
}

static Expr *new_op(Expr *e1, Expr *e2, double (*eval)(Expr *, Econtext *))
{
  Expr *e=calloc(1, sizeof(Expr));
  e->eval=eval;
  e->val.arg[0]=e1;
  e->val.arg[1]=e2;
  return e;
}

%}

%error-verbose

%union {
  int      var;
  double   val;
  Expr   *expr;
}

/* Bison declarations.  */
%token<val> NUM
%token<var> VAR
%token SUM DIFF
%type <expr> expr

%left '?' ':'
%left '<' '>'
%left '-' '+'
%left '*' '/'
%left NEG

%% /* The grammar follows.  */

input: expr { yy_expr=$1; }

expr:     NUM                 { $$=new_num($1); }
        | VAR                 {
                                if ($1 >= MAX_VAR) {
                                  yyerror("too large register number");
                                  YYERROR;
                                }
                                $$=new_var($1);
                              }
        | SUM '(' expr ')'    { $$=new_op($3, (Expr *)(long)yy_nsum++, e_sum);}
        | DIFF '(' expr ')'   { $$=new_op($3, (Expr *)(long)yy_ndiff++, e_diff);}
        | expr '?' expr ':' expr { $$=new_op($1, $3, e_if); $$->val.arg[2]=$5; }
        | expr '<' expr       { $$=new_op($1, $3, e_lt); }
        | expr '>' expr       { $$=new_op($1, $3, e_gt); }
        | expr '+' expr       { $$=new_op($1, $3, e_add); }
        | expr '-' expr       { $$=new_op($1, $3, e_sub); }
        | expr '*' expr       { $$=new_op($1, $3, e_mul); }
        | expr '/' expr       { $$=new_op($1, $3, e_div); }
        | '-' expr  %prec NEG { $$=new_op(new_num(0) ,$2, e_sub); }
        | '+' expr  %prec NEG { $$=$2; }
        | '(' expr ')'        { $$=$2; }
        ;
%%

#include <ctype.h>
#include <string.h>

#define prefix(S,CONST)    !strncasecmp((S),CONST,sizeof(CONST)-1)

int yylex(void)
{
  while (isspace(*yy_str)) yy_str++;
  if (isdigit(*yy_str)) {
    yylval.val=strtod(yy_str, &yy_str);
    return NUM;
  }
  if (*yy_str == '\\') {
    if (!isdigit(*++yy_str)) return '\\';
    yylval.var=strtol(yy_str, &yy_str, 10);
    return VAR;
  }
  if (prefix(yy_str, "sum")) {
    yy_str +=3;
    return SUM;
  }
  if (prefix(yy_str, "diff")) {
    yy_str +=4;
    return DIFF;
  }
  return *yy_str++;
}

/* Called by yyparse on error.  */
void
yyerror (char const *s)
{
  strcpy(yy_err, s);
}

