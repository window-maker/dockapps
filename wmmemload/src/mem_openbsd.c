/*
 * mem_openbsd.c - module to get memory/swap usages in percent, for OpenBSD
 *
 * Copyright (c) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
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

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/swap.h>

/* initialize function */
void
mem_init(void)
{
    return;
}

/* get swap usage */
static int
get_swap_usage(void)
{
    struct swapent *swap_dev;
    int num_swap;
    int stotal, sused, i;

    stotal = sused = 0;

    if ((num_swap = swapctl(SWAP_NSWAP, 0, 0)) == 0)
	return 0;

    if ((swap_dev = malloc(num_swap * sizeof(*swap_dev))) == NULL) 
	return 0;

    if (swapctl(SWAP_STATS, swap_dev, num_swap) == -1)
	return 0;

    for (i = 0; i < num_swap; i++) {
	if (swap_dev[i].se_flags & SWF_ENABLE) {
	    stotal += swap_dev[i].se_nblks;
	    sused += swap_dev[i].se_inuse;
	}
    }

    free(swap_dev);

    if (sused == 0)
	return 0;

    return (100 * (double) sused / (double) stotal);
}


/* return mem/swap usage in percent 0 to 100 */
void
mem_getusage(int *per_mem, int *per_swap, const struct mem_options *opts)
{

    struct vmtotal vm;
    size_t size = sizeof(vm);
    static int mib[] = { CTL_VM, VM_METER };

    /* get mem usage */
    if (sysctl(mib, 2, &vm, &size, NULL, 0) < 0)
	bzero(&vm, sizeof(vm));

    /* calc mem usage in percent */
    if (vm.t_rm > 0)
	*per_mem = 100 * (double) vm.t_rm / (double) (vm.t_rm + vm.t_free);

    if (*per_mem > 97) *per_mem = 100;

    /* swap */
    *per_swap = get_swap_usage();

#ifdef DEBUG
    printf("t_rm      total real memory in use     %6d\n", vm.t_rm);
    printf("t_arm     active real memory           %6d\n", vm.t_arm);
    printf("t_rmshr   shared real memory           %6d\n", vm.t_rmshr);
    printf("t_armshr  active shared real memory    %6d\n", vm.t_armshr);
    printf("t_free    free memory pages            %6d\n", vm.t_free);
#endif

    return;
}
