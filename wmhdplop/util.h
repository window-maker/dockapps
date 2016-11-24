#ifndef UTIL_H
#define UTIL_H
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef SQR
#  define SQR(a) ((a)*(a))
#endif
#ifndef ABS
#  define ABS(a) ((a)>0?(a):-(a))
#endif
#define ALLOC_OBJ(p) { p = calloc(1, sizeof(*p)); assert(p);}
#define FREE_OBJ(p) free(p);
#define ALLOC_VEC(p, n) { p = calloc(n, sizeof(*(p))); assert(p); }
#define SET_VEC(p, v, i0, i1) { int _i; \
                  for (_i = (i0); _i < (i1); _i++) p[_i] = v; }
#define FREE_VEC(p) free(p);

#define ALLOC_ARR(v,nl,nc)  \
  { int __i; v = calloc(nl,sizeof(*(v))); assert(v); \
    v[0]=calloc((nc)*(nl), sizeof(**(v))); assert((v)[0]); \
    for (__i=1; __i < nl; ++__i) (v)[__i] = (v)[__i-1] + nc; }
#define FREE_ARR(p) { free(p[0]); free(p); }
#define BASE_ARR(p) (p[0])

#ifndef NO_BLAHBLAH
# define BLAHBLAH(n,x) if (Prefs.verbosity >= n) { x; fflush(stdout); }
#else
# define BLAHBLAH(n,x)
#endif

#define ONLY_ONCE(x) { static int __cnt = 0; if (__cnt++==0) { x;} }
#define ONLY_NTIMES(n,x) { static int __cnt = 0; if (__cnt++<n) { x;} }



/* macro for str_hache */
#define CVINT(a,b,c,d) (a + (b<<8) + (c<<16) + (d<<24))

#define IS_INSIDE(x, y, xmin, ymin, xmax, ymax) ((x) >= (xmin) && (x) <= (xmax) && (y) >= (ymin) && (y) <= (ymax))

char *str_dup(const char *s);
int str_is_empty(const char *s);
char *str_multi_str(const char *src, const char **keys, int nb_keys, int *key_idx);
char *str_multi_substitute(const char *src, const char **keys, const char **substitutions, int nkeys);
char *str_substitute(const char *src, const char *key, const char *substitution);
char *shell_quote(const char *src);
unsigned str_hash(const unsigned char *s, int max_len);
unsigned char chr_noaccent_tolower(unsigned char c);
void str_noaccent_tolower(unsigned char *s);
unsigned char *str_noaccent_casestr(const unsigned char *meule, const unsigned char *aiguille);
char *str_printf(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
char *str_fget_line(FILE *f);
void str_trim(unsigned char *s);

typedef struct strlist {
  char *s;
  struct strlist *next;
} strlist;
strlist *strlist_ins(strlist *head, const char *s);
#endif
