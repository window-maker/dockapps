/*
 * mem_solaric.c - module to get memory/swap usages in percent, for Solaris
 *
 * Copyright (c) 2001 Jonathan Lang <lang@synopsys.com>
 * Copyright (c) 2002 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mem.h"

#include <sys/stat.h>
#include <sys/swap.h>

/* initialize function */
void mem_init(void)
{
	/* solaris doesn't need initialization */
	return;
}

/* return memory/swap usage in percent 0 to 100 */
void mem_getusage(int *per_mem, int *per_swap, const struct mem_options *opts)
{
	static int mem_total;
	static int mem_free;
	static int swap_total;
	static int swap_free;
	struct anoninfo anon;

	/* get mem usage */
	mem_total = sysconf(_SC_PHYS_PAGES);
	mem_free = sysconf(_SC_AVPHYS_PAGES);

	/* get swap usage */
	if (swapctl(SC_AINFO, &anon) == -1)
		exit(1);
	swap_total = anon.ani_max;
	swap_free = anon.ani_max - anon.ani_resv;

	/* calc mem/swap usage in percent */
	*per_mem = 100 * (1 - ((double) mem_free / (double) mem_total));
	*per_swap = 100 * (1 - ((double) swap_free / (double) swap_total));
}
