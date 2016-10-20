/*
 * wmcpuload
 * GNU/Linux specific part
 *
 * Copyright (C) 2001, 2002, 2005 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* $Id: cpu_linux.c,v 1.4 2006-01-28 10:40:09 sch Exp $ */

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

static int is_linux26;

static void
skip_line(FILE *fp)
{
	int c;

	while ((c = fgetc(fp)) != '\n')
		if (c == EOF) break;
}

void
cpu_init(void)
{
	unsigned long long softirq;
	FILE *fp;

	if (!(fp = fopen("/proc/stat", "r"))) {
		perror("fopen");
		exit(1);
	}

	is_linux26 = fscanf(fp, "%*s  %*u %*u %*u %*u %*u %*u %llu",
			    &softirq);

	fclose(fp);

	return;
}

/* returns current cpu usage in percent */
int
cpu_get_usage(cpu_options * opts)
{
	unsigned long long user, nice, system, idle, iowait, irq, softirq;
	unsigned long long used, total;
	static unsigned long long pre_used = 0, pre_total = 0;
	int result;

	FILE *fp;
	if (!(fp = fopen("/proc/stat", "r"))) {
		perror("fopen");
		exit(1);
	}


	if (opts->cpu_number == -1) {
		if (is_linux26)
			fscanf(fp, "%*s  %llu %llu %llu %llu %llu %llu %llu",
			       &user, &nice, &system, &idle, &iowait,
			       &irq, &softirq);
		else
			fscanf(fp, "%*s  %llu %llu %llu %llu",
			       &user, &nice, &system, &idle);
	} else {
		char cpu_name[20];
		int i;

		for (i = 0; i <= opts->cpu_number; i++)
			skip_line(fp);

		if (is_linux26)
			fscanf(fp, "%s  %llu %llu %llu %llu %llu %llu %llu",
			       cpu_name, &user, &nice, &system, &idle, &iowait,
			       &irq, &softirq);
		else
			fscanf(fp, "%s  %llu %llu %llu %llu",
			       cpu_name, &user, &nice, &system,
			       &idle);

		if (cpu_name[3] != '0' + opts->cpu_number) {
			fprintf(stderr, "Could not find cpu%d.\n",
				opts->cpu_number);
			exit(1);
		}
	}

	fclose(fp);

	used = user + system;
	if (!opts->ignore_nice)
		used += nice;
	total = user + nice + system + idle;
	if (is_linux26)
		total += iowait + irq + softirq;

	result = (100 * (double)(used - pre_used)) / (double)(total - pre_total);

	pre_total = total;
	pre_used = used;

	return result;
}

