/*
 * cpu_linux.c - module to get cpu usage, for GNU/Linux
 *
 * Copyright (C) 2001, 2002 Seiichi SATO <ssato@sh.rim.or.jp>
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

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>

#ifdef USE_SMP
#include <linux/threads.h>
#endif

static void set_pidlist_from_namelist(int names, char **name_list);
static int get_cpuusage_bypid(pid_t pid);

static int *pid_list;
static int pids;

void cpu_init(void)
{
    /*  You don't need initialization under GNU/Linux */
    return;
}

/* returns current cpu usage in percent */
int
cpu_get_usage(cpu_options *opts)
{
    static int pre_used, pre_total;
    static int pre_ig_used;
    int usage;
    int cpu, nice, system, idle;
    int used = 0, total = 0;
    int ig_used = 0;
    int i;

    FILE *fp;
    if (!(fp = fopen("/proc/stat", "r"))) {
	perror("can't open /proc/stat");
	exit(1);
    }

    fscanf(fp, "%*s %d %d %d %d", &cpu, &nice, &system, &idle);

#ifdef USE_SMP
    if (opts->cpu_number >= 0) {
	char cpu_name[20];
	if (opts->cpu_number > NR_CPUS - 1) {
	    fprintf (stderr, "MAX CPU number that can be running in SMP is %d\n", NR_CPUS - 1);
	    exit(1);
	}
	
	for (i = 0; i <= opts->cpu_number; i++) {
	    fscanf(fp, "%s %d %d %d %d", cpu_name, &cpu, &nice, &system, &idle);
	    if (strncmp(cpu_name, "cpu", 3)){
		fprintf (stderr, "can't find cpu%d!\n", opts->cpu_number);
		exit (1);
	    }
	}
    }
#endif /* USE_SMP */

    fclose(fp);
    used = cpu + system;
    if (!opts->ignore_nice)
	used += nice;
    total = cpu + nice + system + idle;

    /* get CPU usage of processes which specified by name with '-p' option */
    if (opts->ignore_procs) {
	pids = 0;
	if (!(pid_list = malloc(sizeof(pid_t)))) {
	    perror("malloc");
	    exit(1);
	}
	set_pidlist_from_namelist(opts->ignore_procs, opts->ignore_proc_list);
	for (i = 0; i < pids; i++)
	    ig_used += get_cpuusage_bypid(pid_list[i]);
	free(pid_list);
    }

    /* calc CPU usage */
    if ((pre_total == 0) || !(total - pre_total > 0)) {
	usage = 0;
    } else  if (ig_used - pre_ig_used > 0) {
	usage = (100 * (double)(used - pre_used - ig_used + pre_ig_used)) /
		(double)(total - pre_total);
    } else {
	usage = (100 * (double)(used - pre_used)) / (double)(total - pre_total);
    }

    /* save current values for next calculation */
    pre_ig_used = ig_used;
    pre_used = used;
    pre_total = total;

    return usage;
}

/* set pid list table from command names */
static void
set_pidlist_from_namelist(int names, char **name_list)
{
    DIR *dir;
    struct dirent *de;
    FILE *fp;
    char path[PATH_MAX + 1];
    char comm[COMM_LEN];
    pid_t pid;
    int i;

    if (!(dir = opendir("/proc"))) {
	perror("can't open /proc");
	exit(1);
    }

    /* search specified process from all processes */
    chdir("/proc");
    while ((de = readdir(dir)) != NULL) {
	if ((de->d_name[0] != '.') &&
	    ((de->d_name[0] >= '0') && (de->d_name[0] <= '9'))) {
	    pid = (pid_t) atoi(de->d_name);
	    sprintf(path, "%d/stat", pid);
	    if ((fp = fopen(path, "r")) != NULL) {
		fscanf(fp, "%*d (%[^)]", comm);
		for (i = 0; i < names; i++) {
		    if (strcmp(comm, name_list[i]) == 0) {
			/* add process id to list */
			pids++;
			if (!(pid_list=realloc(pid_list, pids*sizeof(pid_t)))){
			    perror("realloc() failed");
			    exit(1);
			}
			pid_list[pids - 1] = pid;
		    }
		}
		fclose(fp);
	    }
	}
    }
    closedir(dir);
}

static int
get_cpuusage_bypid(pid_t pid)
{
    FILE *fp;
    char path[PATH_MAX];
    int utime = 0, stime = 0;
    int ret = 0;

    sprintf(path, "/proc/%d/stat", pid);
    if ((fp = fopen(path, "r")) != NULL) {
	fscanf(fp, "%*d %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %d %d ",
	       &utime, &stime);
	fclose(fp);
    }

    ret = utime + stime;
    return ret;
}
