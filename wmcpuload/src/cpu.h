/* $Id: cpu.h,v 1.3 2005-02-10 01:12:49 sch Exp $ */

/*
 * cpu.h - header file of the module to get cpu usage
 *
 * Copyright (c) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */

#ifndef __CPU_H
#define __CPU_H

#ifdef IGNORE_PROC
#define COMM_LEN 16
#endif
#define MAX_PROC 5
#define MAX_CPU 99

typedef struct _cpu_options {
    int ignore_nice;
    int cpu_number;
} cpu_options;

void cpu_init(void);
int cpu_get_usage(cpu_options *opts);

#endif
