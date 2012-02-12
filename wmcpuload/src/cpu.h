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


typedef struct _cpu_options {
    int ignore_nice;
    int cpu_number;
    char *ignore_proc_list[MAX_PROC];
    int ignore_procs;
} cpu_options;

void cpu_init(void);
int cpu_get_usage(cpu_options *opts);

#endif
