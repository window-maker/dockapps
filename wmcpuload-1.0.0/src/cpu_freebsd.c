/*
 * cpu_freebsd.c - module to get cpu usage, for FreeBSD
 *
 * Copyright (c) 2001, 2002 Seiichi SATO <ssato@sh.rim.or.jp>
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
#include <sys/dkstat.h>

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

    used = cpu_time[CP_USER] + cpu_time[CP_SYS];
    if (!opts->ignore_nice)
	used += cpu_time[CP_NICE];
    total = used + cpu_time[CP_IDLE];

    if (pre_total == 0) {
	result = 0;
    } else if ((total - pre_total) > 0) {
	result = (100 * (double) (used - pre_used)) / (double) (total -
		 pre_total);
    } else {
	result = 0;
    }

    pre_used = used;
    pre_total = total;

    return result;
}
