#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "util.h"

char *
str_dup(const char *src) {
  char *s;
  s = strdup(src);
  if (s == NULL) abort();
  return s;
}

/*
  verifie si la chaine est vide (cad si elle ne contient que des caractères non imprimables 
*/
int
str_is_empty(const char *s) {
  int i;
  if (s == NULL) return 1;
  else if (strlen(s) == 0) return 1;
  else {
    i = 0;
    while (s[i] && s[i] <= ' ') i++;
    return (i == (int)strlen(s));
  }
}


/* recherche la première occurence d'une des chaines 'keys' dans 'src' et renvoie un pointeur vers
   cette occurence, ainsi que le numéro de la 'keys' trouvée

   bien sûr c'est pas optimal du tout, mais ON S'EN FOUT(tm)

   et oui, effectivement, 'str_multi_str' est un nom à la con
*/
char *
str_multi_str(const char *src, const char **keys, int nb_keys, int *key_idx)
{
  int i;
  const char *res;

  assert(key_idx);
  *key_idx = 0;
  res = NULL;
  for (i=0; i < nb_keys; i++) {
    const char *p;
    p = strstr(src, keys[i]);
    if (p && (res==NULL || p < res)) { res = p; *key_idx = i; }
  }
  return (char*)res;
}

/* renvoie une chaine (allouée correctement) contenant la substitution de toutes les occurences de
   'key' dans 'src' par 'substitution' (key et substition sont des tableaux de chaines de
   caractères, car pour faire plusieurs substitutions, mieux vaut les effectuer simultanement que
   les enchainer pour eviter les effets de bords
*/
char *
str_multi_substitute(const char *src, const char **keys, const char **substitutions, int nkeys)
{
  const char *p, *p_key;
  char *dest, *p_dest;
  int dest_sz, p_len,j;

  if (src == NULL) return NULL;

  /* calcul de la longueur de la destination.. */
  p = src;
  dest_sz = strlen(src)+1;

  while ((p_key=str_multi_str(p, keys, nkeys, &j))) {
    dest_sz += (strlen(substitutions[j]) - strlen(keys[j]));
    p = p_key+strlen(keys[j]);
  }

  dest = malloc(dest_sz);

  /* et là PAF ! */
  p = src;
  p_dest = dest;
  while ((p_key=str_multi_str(p, keys, nkeys, &j))) {
    memcpy(p_dest, p, p_key-p);
    p_dest += p_key-p;
    memcpy(p_dest, substitutions[j], strlen(substitutions[j]));
    p_dest += strlen(substitutions[j]);
    p = p_key + strlen(keys[j]);
  }
  p_len = strlen(p);
  if (p_len) {
    memcpy(p_dest, p, p_len); p_dest += p_len;
  }
  *p_dest = 0;
  assert(p_dest - dest == dest_sz-1); /* capote à bugs */
  return dest;
}

char *
str_substitute(const char *src, const char *key, const char *substitution) {
  return str_multi_substitute(src, &key, &substitution, 1);
}

/* quotage pour les commandes externes.. à priori c'est comme pour open_url
   mais bon.. je me refuse à donner la moindre garantie sur la sécurité 

   be aware
*/
char *
shell_quote(const char *src)
{
  char *quote = "&;`'\\\"|*?~<>^()[]{}$ ";
  int i,dest_sz;
  const char *p;
  char *dest;

  if (src == NULL || strlen(src) == 0) return strdup("");

  dest_sz = strlen(src)+1;
  for (p=src; *p; p++) {
    if (strchr(quote, *p)) dest_sz+=1;
  }
  dest = malloc(dest_sz);

  for (p=src, i=0; *p; p++) {
    if (strchr(quote, *p)) {
      dest[i++] = '\\';
    }
    if (*p>=0 && *p < ' ') {
      dest[i++] = ' ';
    } else {
      dest[i++] = *p;
    }
  }
  dest[i] = 0;
  assert(i == dest_sz-1); /* kapeaute à beugue */
  return dest;
}


static unsigned *crc_table = NULL;

void gen_crc_table(void)                /* build the crc table */
{
  unsigned crc, poly;
  int	i, j;
  
  poly = 0xEDB88320;
  for (i = 0; i < 256; i++)
    {
      crc = i;
      for (j = 8; j > 0; j--)
	{
	  if (crc & 1)
	    crc = (crc >> 1) ^ poly;
	  else
                crc >>= 1;
	}
      crc_table[i] = crc;
    }
}


unsigned str_hash(const unsigned char *s, int max_len)    /* calculate the crc value */
{
  unsigned crc;
  int i;
  
  if (crc_table == NULL) {
    crc_table = calloc(256, sizeof(unsigned));
    gen_crc_table();
  }
  crc = 0xFFFFFFFF;
  for (i=0; i < max_len && s[i]; i++) {
    crc = ((crc>>8) & 0x00FFFFFF) ^ crc_table[ (crc^s[i]) & 0xFF ];
  }
  return( crc^0xFFFFFFFF );
}


unsigned char char_trans[256];
static int char_trans_init = 0;

static void 
init_char_trans()
{
  const unsigned char *trans_accents  = 
    (const unsigned char*) "éèëêÊËÉÈàâáäÀÂÁÄûüùÙçÇîïíìÏÎÍÌôóòõÔÓÒÕñ";
  const unsigned char *trans_accents2 = 
    (const unsigned char*) "eeeeeeeeaaaaaaaauuuucciiiiiiiioooooooon";
  int i;

  for (i=0; i < 256; i++) {
    unsigned char *p;
    if ((p=(unsigned char*)strchr((char*)trans_accents, i))) {
      char_trans[i] = trans_accents2[(p - trans_accents)];
      } else if (i < (unsigned char)'A' || i > (unsigned char)'Z') {
	char_trans[i] = i;
      } else {
	char_trans[i] = i + 'a' - 'A';
      }
  }
  char_trans_init = 1;
}

unsigned char
chr_noaccent_tolower(unsigned char c)
{
  if (char_trans_init == 0) init_char_trans();
  return char_trans[c];
}

void
str_noaccent_tolower(unsigned char *s)
{
  int i;
  if (s == NULL) return;
  if (char_trans_init == 0) init_char_trans();
  i = 0; while(s[i]) {
    s[i] = char_trans[s[i]]; i++;
  }
}

unsigned char *
str_noaccent_casestr(const unsigned char *meule, const unsigned char *aiguille)
{
  unsigned char *res;
  char *m = strdup((char*)meule);
  char *a = strdup((char*)aiguille);

  str_noaccent_tolower((unsigned char*)m);
  str_noaccent_tolower((unsigned char*)a);
  res = (unsigned char*)strstr(m, a);
  free(a); free(m);
  return res;
}

/* un printf pas très fin, mais avec allocation dynamique..
   c'est pratique ces ptites choses */
char *
str_printf(const char *fmt, ...)
{
  va_list ap;
  char *s;
  int s_sz;

  s_sz = 10; /* a reaugmenter des que la fonction est validee : */
  s = malloc(s_sz); assert(s);
  while (1) {
    int ret;
    va_start(ap, fmt);
    ret = vsnprintf(s, s_sz, fmt, ap);
    va_end(ap);
    if (ret == -1 || ret >= s_sz-1) {
      s_sz *= 2;
      assert(s_sz < 100000);
      s = realloc(s, s_sz); assert(s);
    } else 
      break;
  }
  s = realloc(s, strlen(s)+1); assert(s);
  return s;
}

/* read a line in a file and return the result in a dynamically
   allocated buffer */
char *
str_fget_line(FILE *f)
{
  int i,c;
  char *s;
  int s_sz;

  s_sz = 100; s = malloc(s_sz); assert(s);
  i = 0;
  while ((c = fgetc(f)) > 0) {
    if (c >= ' ' || c == '\t') {
      s[i++] = c;
      if (i == s_sz) { 
	s_sz *= 2; assert(s_sz < 100000);
	s = realloc(s, s_sz); assert(s);
      }
    }
    if (c == '\n') break;
  }
  s[i] = 0; assert(i < s_sz);
  s = realloc(s, strlen(s)+1); assert(s);
  return s;
}

/* remove spaces at the beginning and at the end */
void
str_trim(unsigned char *s) {
  int i,j;

  if (s == NULL) return;
  j = strlen((char*)s)-1;
  while (j>=0 && s[j] <= ' ') s[j--] = 0;

  i = 0; 
  while (s[i] && s[i] <= ' ') i++;
  if (i<=j) {
    memmove(s, s+i, j+2-i);
  }
}

/* insertion into a string list */
strlist *strlist_ins(strlist *head, const char *s) {
  strlist *p; ALLOC_OBJ(p);
  p->s = strdup(s); p->next = head; return p;
}
