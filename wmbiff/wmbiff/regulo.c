#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <regex.h>
#include <ctype.h>

#include "regulo.h"
#include "charutil.h"

#define min(a,b) ((a)<(b) ? (a) : (b))

/* callbacks specified by the calling function to extract
   substrings to values. */
void regulo_atoi(void *dest, const char *source)
{
	int *dest_int = dest;

	/* skip any leading non-digit */
	while (*source != '\0' && !isdigit(*source))
		source++;

	*dest_int = atoi(source);
}

void regulo_strcpy(void *dest, const char *source)
{
	strcpy((char *) dest, source);
}

void regulo_strcpy_tolower(void *dest, const char *source)
{
	char *dest_str = dest;
	size_t i;

	for (i = 0; i < strlen(source); i++) {
		dest_str[i] = tolower(source[i]);
	}
	dest_str[i] = '\0';
}

void regulo_strcpy_skip1(void *dest, const char *source)
{
	strcpy(dest, source + 1);
}

#ifdef USE_GNU_REGEX
/* deprecated as unportable */

int
regulo_match(const char *regex, const char *string,
	     struct regulo *instructions)
{
	struct re_registers regs;
	int ret;
	int matchedchars;
	int i;
	memset(&regs, 0, sizeof(struct re_registers));
	matchedchars = compile_and_match_regex(regex, string, &regs);
	if (matchedchars <= 0)
		return 0;
	if (instructions == NULL)
		return 1;
	for (i = 0; instructions[i].match_handler != NULL; i++) {
		char buf[255];
		int j = instructions[i].match_index;
		int len = min(254, regs.end[j] - regs.start[j]);
		if (regs.start[j] >= 0) {
			strncpy(buf, string + regs.start[j], len);
			buf[len] = '\0';
			instructions[i].match_handler(instructions[i].destination,
										  buf);
		}
	}
	ret = regs.end[0];
	free(regs.end);				// added 3 jul 02, appeasing valgrind
	free(regs.start);			// added 3 jul 02, appeasing valgrind
	return ret;
}

#else
/* favored */

int compile_and_match_regex_posix(const char *regex, const char *str,	/*@out@ */
								  regmatch_t * regs, size_t regs_len)
{
	regex_t reg;
	int errcode;
	if ((errcode = regcomp(&reg, regex, REG_EXTENDED)) != 0) {
		char errbuf[256];
		regerror(errcode, &reg, errbuf, 256);
		fprintf(stderr, "error in compiling regular expression: %s\n",
				errbuf);
		return -1;
	}

	errcode = regexec(&reg, str, regs_len, regs, 0);
	regfree(&reg);
	if (errcode == 0)
		return 1;
	else
		return 0;
}


int
regulo_match(const char *regex, const char *string,
	     struct regulo *instructions)
{
	regmatch_t regs[20];
	int ret;
	int matchedchars;
	int i;
	matchedchars = compile_and_match_regex_posix(regex, string, regs, 20);
	if (matchedchars <= 0)
		return 0;
	if (instructions == NULL)
		return 1;
	for (i = 0; instructions[i].match_handler != NULL; i++) {
		char buf[255];
		int j = instructions[i].match_index;
		int len = min(254, regs[j].rm_eo - regs[j].rm_so);
		if (regs[j].rm_so >= 0) {
			strncpy(buf, string + regs[j].rm_so, len);
			buf[len] = '\0';
			instructions[i].match_handler(instructions[i].destination,
										  buf);
		}
	}
	ret = regs[0].rm_eo;
	return ret;
}

#endif
