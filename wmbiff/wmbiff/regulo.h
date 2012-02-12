/* $Id: regulo.h,v 1.2 2003/03/02 02:17:14 bluehal Exp $ */
/* regulo, (pronounced as if the name of a super-hero) added
   by Neil Spring to provide a portable interface to regular
   expressions that doesn't suck. */

void regulo_atoi(void *dest_int, const char *source);
void regulo_strcpy(void *dest, const char *source);
void regulo_strcpy_tolower(void *dest, const char *source);
void regulo_strcpy_skip1(void *dest, const char *source);
struct regulo {
	int match_index;
	void *destination;
	void (*match_handler) (void *dest, const char *source);
};

int regulo_match(const char *regex,
				 const char *string, const struct regulo *instructions);
