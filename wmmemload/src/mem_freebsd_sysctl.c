/*
 * mem_freebsd.c - module to get memory/swap usages in percent, for FreeBSD
 *
 * Copyright (c) 2001, 2002 Seiichi SATO <ssato@sh.rim.or.jp>
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

#include <sys/types.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>
#include <sys/vmmeter.h>

/* initialize function */
void mem_init(void)
{
    return;
}

/* return mem/swap usage in percent 0 to 100 */
void mem_getusage(int *per_mem, int *per_swap, const struct mem_options *opts)
{

    struct vmtotal vm;
    size_t size = sizeof(vm);
    static int mib[] = { CTL_VM, VM_METER };

    /* get mem usage */
    if (sysctl(mib, 2, &vm, &size, NULL, 0) < 0)
	bzero(&vm, sizeof(vm));

    /* calc mem usage in percent */
    /* FIXME: the total usage(t_rm and t_free) is incorrect?? */
    if (vm.t_rm > 0)
	*per_mem = 100 * (double) vm.t_rm / (double) (vm.t_rm + vm.t_free);
    if (*per_mem > 95)
	*per_mem = 100;

#ifdef DEBUG
    printf("t_vm      total virtual memory         %6lu\n", vm.t_vm);
    printf("t_rm      total real memory in use     %6lu\n", vm.t_rm);
    printf("t_arm     active real memory           %6lu\n", vm.t_arm);
    printf("t_rmshr   shared real memory           %6lu\n", vm.t_rmshr);
    printf("t_armshr  active shared real memory    %6lu\n", vm.t_armshr);
    printf("t_free    free memory pages            %6lu\n", vm.t_free);
    printf("--------------------------------------------\n");
#endif

    /* get swap usage */
    /* not written yet.. can i get swap usage via sysctl? */

    *per_swap = 0;
    return;
}
