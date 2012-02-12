/*
 * cpu_openbsd - module to get cpu usage, for OpenBSD
 *
 * Copyright (C) 2001, 2002 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * Licensed under the GPL
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/dkstat.h>

void
cpu_init(void)
{
    /* You don't need initialization under OpenBSD */
    return;
}

/* Returns the current CPU usage in percent */
int
cpu_get_usage(cpu_options *opts)
{
    int total, used, result;
    static int pre_total, pre_used;

    int mib[] = { CTL_KERN, KERN_CPTIME };
    unsigned long int cpu_time[CPUSTATES];
    size_t size = sizeof(cpu_time);

    /* get cpu time*/
    if (sysctl(mib, 2, &cpu_time, &size, NULL, 0) < 0)
	return 0;

    /* calc usage */
    used = cpu_time[CP_USER] + cpu_time[CP_SYS];
    if (!opts->ignore_nice) {
	used += cpu_time[CP_NICE];
    }
    total = used + cpu_time[CP_IDLE];

    if (pre_total == 0) {
	result = 0;
    } else if ((total - pre_total) > 0) {
	result = 100 * (double)(used - pre_used) / (double)(total - pre_total);
    } else {
	result = 0;
    }

    /* save used/total for next calculation */
    pre_used = used;
    pre_total = total;

    return result;
}
