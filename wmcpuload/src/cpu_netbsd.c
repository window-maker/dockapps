/* $Id: cpu_netbsd.c,v 1.3 2003-10-13 04:34:02 sch Exp $ */

/*
 * cpu_netbsd - module to get cpu usage, for NetBSD 
 *
 * This code is based on cpu_openbsd.c
 *
 * Copyright (c) 2001, 2002 Seiichi SATO <ssato@sh.rim.or.jp>
 * Copyright (c) 2002 Thomas Runge <coto@core.de>
 * Copyright (C) 2003 Nedko Arnaudov <nedko@users.sourceforge.net>
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/sched.h>

void
cpu_init(void)
{
    /* You don't need initialization under NetBSD */
    return;
}

/* Returns the current CPU usage in percent */
int
cpu_get_usage(cpu_options *opts)
{
    int total, used, result, idle;
    static int pre_total, pre_used, pre_idle;

    int mib[] = { CTL_KERN, KERN_CP_TIME };
    u_int64_t cpu_time[CPUSTATES];
    size_t size = sizeof(cpu_time);

    /* get cpu time via sysctl */
    if (sysctl(mib, 2, &cpu_time, &size, NULL, 0) < 0)
	return 0;

    /* calculate usage */
    total = cpu_time[CP_USER] + cpu_time[CP_SYS] + cpu_time[CP_INTR] +
	    cpu_time[CP_NICE] + cpu_time[CP_IDLE];
    used = cpu_time[CP_USER] + cpu_time[CP_SYS] + cpu_time[CP_INTR] +
	   (opts->ignore_nice ? 0 : cpu_time[CP_NICE]);
    if ((pre_total == 0) || !(total - pre_total > 0)) {
	result = 0;
    } else {
	result = 100 * (double)(used - pre_used) / (double)(total - pre_total);
    }

    /* save used/total for next calculation */
    pre_used = used;
    pre_total = total;

    return result;
}
