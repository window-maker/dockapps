/*
 * cpu_freebsd.c - module to get cpu usage, for FreeBSD
 *
 * Copyright (c) 2001, 2002, 2004 Seiichi SATO <ssato@sh.rim.or.jp>
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

#include <kvm.h>
#include <fcntl.h>

#include <sys/param.h>

#if __FreeBSD_version < 500101
#   include <sys/dkstat.h>
#else
#   include <sys/resource.h>
#endif /* __FreeBSD_version < 500101 */

static kvm_t *kd = NULL;
static struct nlist nlst[] = { {"_cp_time"}, {0} };

void
cpu_init(void)
{

    kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open");

    if (kd == NULL) {
	fprintf(stderr, "can't open kernel virtual memory");
	exit(1);
    }

    kvm_nlist(kd, nlst);

    if (nlst[0].n_type == 0) {
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

/* returns current CPU usage in percent */
int
cpu_get_usage(cpu_options *opts)
{
    static int pre_used, pre_total;
    int used, total, result;
    unsigned long int cpu_time[CPUSTATES];

    if (kvm_read(kd, nlst[0].n_value, &cpu_time, sizeof(cpu_time)) !=
	sizeof(cpu_time))
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
