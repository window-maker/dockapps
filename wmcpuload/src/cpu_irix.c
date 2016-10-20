/*
 * cpu_irix.c - module to get cpu usage, for IRIX 6.5 and IRIX64 6.5
 *
 * Copyright (C) 2002 Jonathan C. Patschke <jp@celestrion.net>
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

#include <sys/types.h>
#include <sys/sysmp.h>
#include <sys/sysinfo.h>
#include <sys/sysget.h>

int cpuCount;

void
cpu_init(void)
{
    cpuCount = (int)sysmp(MP_NPROCS);
    return;
}

/* returns current CPU usage in percent */
int
cpu_get_usage(cpu_options *opts)
{
    struct sgt_cookie cookie;
    struct sysinfo info;
    long cpuload, cputotal;
    static long ocpuload, ocputotal;
    int result, i;

    if (opts->cpu_number >= cpuCount) return 0;
    SGT_COOKIE_INIT(&cookie);
    if (opts->cpu_number < 1) {
      /* Get stats for all CPUs */
      cpuload  = 0;
      cputotal = 0;
      for (i = 0 ; i < cpuCount ; i++) {
        SGT_COOKIE_SET_CPU(&cookie, i);
        memset(((void *)&info), 0x00, sizeof(info));
        sysget(SGT_SINFO_CPU, ((char *)&info), sizeof(info),
               SGT_READ, &cookie);
        cpuload  += info.cpu[CPU_USER] + info.cpu[CPU_KERNEL] +
                    info.cpu[CPU_WAIT] + info.cpu[CPU_SXBRK]  +
                    info.cpu[CPU_INTR];
        cputotal += cpuload + info.cpu[CPU_IDLE];
      };
    } else {
      SGT_COOKIE_SET_CPU(&cookie, opts->cpu_number);
      memset(((void *)&info), 0x00, sizeof(info));
      sysget(SGT_SINFO_CPU, ((char *)&info), sizeof(info),
             SGT_READ, &cookie);
      cpuload  = info.cpu[CPU_USER] + info.cpu[CPU_KERNEL] +
                 info.cpu[CPU_WAIT] + info.cpu[CPU_SXBRK]  +
                 info.cpu[CPU_INTR];
      cputotal = cpuload + info.cpu[CPU_IDLE];
    }
#ifdef DEBUG
    fprintf(stderr, "!!!%d/%d: %d, %d, %d, %d, %d, %d\n", opts->cpu_number,
            cpuCount, info.cpu[CPU_USER], info.cpu[CPU_KERNEL], info.cpu[CPU_WAIT],
            info.cpu[CPU_SXBRK], info.cpu[CPU_INTR], info.cpu[CPU_IDLE]);
#endif

    result    = ((cpuload - ocpuload) * 100) / (cputotal - ocputotal);
    ocpuload  = cpuload;
    ocputotal = cputotal;

    return result;
}
