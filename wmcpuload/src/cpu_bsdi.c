/* $Id: cpu_bsdi.c,v 1.1.1.1 2002-10-14 09:31:17 sch Exp $ */

/*
 * cpu_bsdi - module to get cpu usage, for BSDi
 *
 * Copyright (C) 2001, 2002 Seiichi SATO <ssato@sh.rim.or.jp>
 * Copyright (C)       2002 Nicolas Belan <belan@matranet.com>
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
    /* You don't need initialization under BSDi */
    return;
}

/* Returns the current CPU usage in percent */
int
cpu_get_usage(struct cpu_options *opts)
{
    int total, used, result;
    static int pre_total, pre_used;
	struct cpustats cpustat;
	int mib[] = { CTL_KERN, KERN_CPUSTATS };
    size_t size = sizeof(struct cpustats);

    /* get cpu time*/
    if (sysctl(mib, 2, &cpustat, &size, NULL, 0) < 0)
	return 0;

    /* calc usage */
    used = cpustat.cp_time[CP_USER] + cpustat.cp_time[CP_SYS];
    if (!opts->ignore_nice)
	used += cpustat.cp_time[CP_NICE];
    total = used + cpustat.cp_time[CP_IDLE];

    if (pre_total == 0)
	result = 0;
    else if ((total - pre_total) > 0)
	result = 100 * (double)(used - pre_used) / (double)(total - pre_total);
    else
	result = 0;

    /* save used/total for next calculation */
    pre_used = used;
    pre_total = total;

    return result;
}
