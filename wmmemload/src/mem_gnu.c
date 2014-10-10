/*
 * mem_gnu.c - module to get memory/swap usages, for GNU/Hurd
 *
 * Copyright(C) 2014       Svante Signell <svante.signell@gmail.com>
 * Copyright(C) 2001,2002  Seiichi SATO <ssato@sh.rim.or.jp>
 * Copyright(C) 2001       John McCutchan <ttb@tentacle.dhs.org>
 *
 * licensed under the GPL
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#if defined(HAVE_STRING_H)
#include <string.h>
#elif defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/utsname.h>
#include "mem.h"

#ifdef DEBUG
#  define INLINE_STATIC static
#else
#  define INLINE_STATIC static inline
#endif

/* initialize function */
void mem_init(void)
{
	struct utsname un;
	int version, patchlevel;

	/* get kernel version */
	if (uname(&un) == -1)
		perror("uname()");
	sscanf(un.release, "%d.%d", &version, &patchlevel);
}


INLINE_STATIC char *skip_line(const char *p)
{
	while (*p != '\n')
		p++;
	return (char *) ++p;
}

INLINE_STATIC char *skip_token(const char *p)
{
	while (isspace(*p))
		p++;
	while (*p && !isspace(*p))
		p++;
	return (char *)p;
}

INLINE_STATIC char *skip_multiple_token(const char *p, int count)
{
	int i;
	for (i = 0; i < count; i++)
		p = skip_token(p);
	return (char *)p;
}

/* return mem/swap usage in percent 0 to 100 */
void mem_getusage(int *per_mem, int *per_swap, const struct mem_options *opts)
{
	char buffer[BUFSIZ], *p;
	int fd, len, i;
	u_int64_t mtotal, mused, mfree, mbuffer, mcached;
	u_int64_t stotal, sused, sfree, scached = 0;

	/* read /proc/meminfo */
	fd = open("/proc/meminfo", O_RDONLY);
	if (fd < 0) {
		perror("can't open /proc/meminfo");
		exit(1);
	}
	len = read(fd, buffer, BUFSIZ - 1);
	if (len < 0) {
		perror("can't read /proc/meminfo");
		exit(1);
	}
	close(fd);

	buffer[len] = '\0';
	p = buffer;

	p = skip_token(p);
	/* examine each line of file */
	mtotal  = strtoul(p, &p, 0); p = skip_multiple_token(p, 2);
	mfree   = strtoul(p, &p, 0); p = skip_multiple_token(p, 2);
	mbuffer = strtoul(p, &p, 0); p = skip_multiple_token(p, 2);
	mcached = strtoul(p, &p, 0); p = skip_multiple_token(p, 2);
	scached = strtoul(p, &p, 0);

	/* skip N lines and examine info about swap */
	while (isprint(p[0])) {
		p = skip_line(p);
		if (strncmp(p, "SwapTotal", 9) == 0)
			break;
	}

	p = skip_token(p);
	stotal = strtoul(p, &p, 0); p = skip_multiple_token(p, 2);
	sfree  = strtoul(p, &p, 0);

	/* calculate memory usage in percent */
	mused = mtotal - mfree;
	if (opts->ignore_buffers)
		mused -= mbuffer;
	if (opts->ignore_cached)
		mused -= mcached;
	*per_mem = 100 * (double) mused / (double) mtotal;

	/* calculate swap usage in percent */
	sused = stotal - sfree;
	if (opts->ignore_cached)
		sused -= scached;
	if (!stotal)
		*per_swap = 0;
	else
		*per_swap = 100 * (double) sused / (double) stotal;

#if DEBUG
	printf("-----------------------\n");
	printf("MemTotal:  %12ld\n", (unsigned long)mtotal);
	printf("MemFree:   %12ld\n", (unsigned long)mfree);
	printf("Buffers:   %12ld\n", (unsigned long)mbuffer);
	printf("Cached:    %12ld\n", (unsigned long)mcached);
	printf("SwapTotal: %12ld\n", (unsigned long)stotal);
	printf("SwapFree:  %12ld\n", (unsigned long)sfree);
	printf("SwapCached:%12ld\n", (unsigned long)scached);
	printf("-----------------------\n\n");
#endif

}
