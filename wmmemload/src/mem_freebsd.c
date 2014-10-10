/*
 * mem_freebsd.c - module to get memory/swap usages in percent, for FreeBSD
 *
 * Copyright(c) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

#include <kvm.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/sysctl.h>
#include <time.h>

static kvm_t *kvm_data;

/* initialize function */
void mem_init(void)
{
	kvm_data = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open");

	if (kvm_data == NULL) {
		fprintf(stderr, "can't open kernel virtual memory");
		exit(1);
	}

	/* drop setgid & setuid (the latter should not be there really) */
	seteuid(getuid());
	setegid(getgid());

	if (geteuid() != getuid() || getegid() != getgid()) {
		fprintf(stderr, "unable to drop privileges");
		exit(1);
	}

	return;
}

#define GETSYSCTL(name, var) getsysctl(name, &(var), sizeof(var))

static void getsysctl(const char *name, void *ptr, size_t len)
{
	size_t nlen = len;

	if (sysctlbyname(name, ptr, &nlen, NULL, 0) == -1) {
		fprintf(stderr, "top: sysctl(%s...) failed: %s\n", name,
			strerror(errno));
		exit(1);
	}
	if (nlen != len) {
		fprintf(stderr, "top: sysctl(%s...) expected %lu, got %lu\n",
			name, (unsigned long)len, (unsigned long)nlen);
		exit(1);
	}
}

/* return mem/swap usage in percent 0 to 100 */
void mem_getusage(int *per_mem, int *per_swap, const struct mem_options *opts)
{
	u_int mtotal, mwired, mcached, mfree, mused;
#ifdef DEBUG
	u_int pagesize;
#endif
	u_int new_swappgsin, new_swappgsout;
	static int swap_firsttime = 1;
	static int swappgsin = -1;
	static int swappgsout = -1;
	static int swapmax, swapused;
	time_t cur_time;
	static time_t last_time_swap;


	/* get mem usage */
	GETSYSCTL("vm.stats.vm.v_page_count", mtotal);
	GETSYSCTL("vm.stats.vm.v_wire_count", mwired);
	GETSYSCTL("vm.stats.vm.v_cache_count", mcached);
	GETSYSCTL("vm.stats.vm.v_free_count", mfree);
#ifdef DEBUG
	GETSYSCTL("hw.pagesize", pagesize);
#endif

	/* get swap usage */
	/* only calculate when first time or when changes took place         */
	/* do not call it more than 1 time per 2 seconds                     */
	/* otherwise it can eat up to 50% of CPU time on heavy swap activity */
	cur_time = time(NULL);

	GETSYSCTL("vm.stats.vm.v_swappgsin", new_swappgsin);
	GETSYSCTL("vm.stats.vm.v_swappgsout", new_swappgsout);

	if (swap_firsttime ||
	    (((new_swappgsin > swappgsin) || (new_swappgsout > swappgsout))
	     && cur_time > last_time_swap + 1)) {

		struct kvm_swap swap;
		int n;

		swapmax = 0;
		swapused = 0;

		n = kvm_getswapinfo(kvm_data, &swap, 1, 0);
		if (n >= 0 && swap.ksw_total != 0) {
			swapmax = swap.ksw_total;
			swapused = swap.ksw_used;
		}

		swap_firsttime = 0;
		last_time_swap = cur_time;
	}

	swappgsin = new_swappgsin;
	swappgsout = new_swappgsout;

#ifdef DEBUG
	printf("-------------------\n");
	printf("total:%10d\n", mtotal * pagesize);
	printf("free :%10d\n", mfree * pagesize);
	printf("wired:%10d\n", mwired * pagesize);
	printf("cache:%10d\n", mcached * pagesize);
	printf("-------------------\n");
#endif

	/* calc mem/swap usage in percent */
	mused = mtotal - mfree;
	if (opts->ignore_wired)
		mused -= mwired;
	if (opts->ignore_cached)
		mused -= mcached;
	*per_mem = 100 * (double) mused / (double) mtotal;
	*per_swap = 100 * (double) swapused / (double) swapmax;

	if (*per_mem > 97)
		*per_mem = 100;
}
