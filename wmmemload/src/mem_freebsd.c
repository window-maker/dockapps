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
#include "mem.h"

#include <kvm.h>
#include <fcntl.h>
#include <sys/vmmeter.h>
#include <time.h>

static kvm_t *kvm_data = NULL;
static int pageshift;
static struct nlist nlst[] = { {"_cp_time"}, {"_cnt"}, {0} };

/* initialize function */
void mem_init(void)
{
    int pagesize = getpagesize();
    pageshift = 0;

    while (pagesize > 1) {
	pageshift++;
	pagesize >>= 1;
    }

    kvm_data = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open");

    if (kvm_data == NULL) {
	fprintf(stderr, "can't open kernel virtual memory");
	exit(1);
    }
    kvm_nlist(kvm_data, nlst);

    if (nlst[0].n_type == 0 || nlst[1].n_type == 0) {
	fprintf(stderr, "error extracting symbols");
	exit(1);
    }
    /* drop setgid & setuid (the latter should not be there really) */
    seteuid(getuid());
    setegid(getgid());

    if (geteuid() != getuid() || getegid() != getgid()) {
	fprintf(stderr, "unable to drop privileges");
	exit(1);
    }
}


/* return mem/swap usage in percent 0 to 100 */
void mem_getusage(int *per_mem, int *per_swap, const struct mem_options *opts)
{
    struct vmmeter vm;
    int bufspace;
    static int swap_firsttime = 1;
    static int swappgsin = -1;
    static int swappgsout = -1;
    static int swapmax = 0, swapused = 0;
    time_t cur_time;
    static time_t last_time_swap = 0;
    u_int mused;

    /* get mem usage */
    if (kvm_read(kvm_data, nlst[0].n_value, &bufspace, sizeof(bufspace)) !=
	sizeof(bufspace))
	exit(1);
    if (kvm_read(kvm_data, nlst[1].n_value, &vm, sizeof(vm)) != sizeof(vm))
	exit(1);

    /* get swap usage */
    /* only calculate when first time or when changes took place         */
    /* do not call it more than 1 time per 2 seconds                     */
    /* otherwise it can eat up to 50% of CPU time on heavy swap activity */
    cur_time = time(NULL);
    if (swap_firsttime ||
	(((vm.v_swappgsin > swappgsin) || (vm.v_swappgsout > swappgsout))
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
    swappgsin = vm.v_swappgsin;
    swappgsout = vm.v_swappgsout;

#ifdef DEBUG
    printf ("-------------------\n");
    printf ("total:%10d\n", vm.v_page_count * vm.v_page_size);
    printf ("free :%10d\n", vm.v_free_count * vm.v_page_size);
    printf ("act  :%10d\n", vm.v_active_count * vm.v_page_size);
    printf ("inact:%10d\n", vm.v_inactive_count * vm.v_page_size);
    printf ("wired:%10d\n", vm.v_wire_count * vm.v_page_size);
    printf ("cache:%10d\n", vm.v_cache_count * vm.v_page_size);
    printf ("-------------------\n");
#endif

    /* calc mem/swap usage in percent */
    mused = vm.v_page_count - vm.v_free_count;
    if (opts->ignore_wired) mused -= vm.v_wire_count;
    if (opts->ignore_cached) mused -= vm.v_cache_count;

    *per_mem = 100 * (double) mused / (double) vm.v_page_count;
    *per_swap = 100 * (double) swapused / (double) swapmax;

    if (*per_mem > 97) *per_mem = 100;
}
