/*
 * cpu_solaric.c - module to get cpu usage, for Solaris
 *
 * Copyright (C) 2001 Jonathan Lang <lang@synopsys.com>
 * Copyright (C) 2002 Seiichi SATO <ssato@sh.rim.or.jp>
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
#include "cpu.h"

#include <kstat.h>
#include <sys/cpuvar.h>
#include <sys/sysinfo.h>

kstat_t **cpu_ks;
cpu_stat_t *cpu_stat;

void
cpu_init(void)
{
    /* You don't need initialization under solaris */
    return;
}

/* returns current CPU usage in percent */
int
cpu_get_usage(cpu_options *opts)
{
    static long oldload, oldtotal;
    long cpuload, cputotal;
    int result, ncpus, i;
    kstat_ctl_t *kc;
    kstat_named_t *kn;
    kstat_t *ks;

    kc = kstat_open();
    if (!kc) {
	perror("kstat_open");
	exit(1);
    }

    ks = kstat_lookup(kc, "unix", 0, "system_misc");
    if (kstat_read(kc, ks, NULL) == -1) {
	perror("kstat_read");
	exit(1);
    }

    /*
     *  Find out how many CPUs the machine has
     */
    kn = kstat_data_lookup(ks, "ncpus");
    ncpus = kn->value.ul;

    /*
     *  Get CPU usage stats.
     */
    cpu_ks = (kstat_t **) realloc(cpu_ks, ncpus * sizeof(kstat_t *));
    cpu_stat =
	(cpu_stat_t *) realloc(cpu_stat, ncpus * sizeof(cpu_stat_t));

    for (i = 0, ks = kc->kc_chain; ks; ks = ks->ks_next) {
	if (strncmp(ks->ks_name, "cpu_stat", 8) == 0)
	    cpu_ks[i++] = ks;
    }

    for (i = 0; i < ncpus; ++i)
	(void) kstat_read(kc, cpu_ks[i], &cpu_stat[i]);

    /*
     *  Sum the times for the various non-idle CPU_STATES, for Solaris the
     *  CPU_STATES are:
     *
     *    CPU_IDLE        0
     *    CPU_USER        1
     *    CPU_KERNEL      2
     *    CPU_WAIT        3
     */
    for (cpuload = 0, i = 0; i < ncpus; ++i) {
	cpuload += (long) cpu_stat[i].cpu_sysinfo.cpu[CPU_USER];
	cpuload += (long) cpu_stat[i].cpu_sysinfo.cpu[CPU_KERNEL];
	cpuload += (long) cpu_stat[i].cpu_sysinfo.cpu[CPU_WAIT];
    }

    /*
     *  Total Time
     */
    for (cputotal = cpuload, i = 0; i < ncpus; ++i)
	cputotal += (long) cpu_stat[i].cpu_sysinfo.cpu[CPU_IDLE];

    kstat_close(kc);

    if (oldtotal == 0) {
	result = 0;
    } else if ((cputotal - oldtotal) > 0) {
	result = (100 * (double) (cpuload - oldload)) /
			(double) (cputotal - oldtotal);
    } else if (cputotal == oldtotal) {
	result = 0;
    } else
	result = 0;

    oldload = cpuload;
    oldtotal = cputotal;

    return result;
}
