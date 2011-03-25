/***************************************************************************
                          cpufreq.c  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003,2004,2005 by Noberasco Michele
    e-mail               : s4t4n@gentoo.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.              *
 *                                                                         *
 ***************************************************************************/


#define SYSFS_CPU_BASE_DIR      "/sys/devices/system/cpu"
#define SYSFS_CPUFREQ_TEST      "affected_cpus"
#define SYSFS_CPUFREQ_AVAIL_GOV "scaling_available_governors"
#define SYSFS_CPUFREQ_SET_GOV   "scaling_governor"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include "power_management.h"
#include "lib_utils.h"


/* This will contain a list of CPU that we will manage through cpufreq */
struct cpufreq_available_cpus
{
	int cpu_number;
	char *dir_name;
	struct cpufreq_available_cpus *next;
} *available_cpus = NULL;


/* Add one element to the list of CPUs available for frequency scaling */
void add_to_cpus_list(int cpu_number, char *path)
{
	struct cpufreq_available_cpus *new_cpu;

	if (!path) return;

	new_cpu = (struct cpufreq_available_cpus *) calloc(1, sizeof(struct cpufreq_available_cpus));
	if (!new_cpu)
	{
		fprintf(stderr, "allocation failure!\n");
		abort();
	}
	new_cpu->cpu_number = cpu_number;
	new_cpu->dir_name   = path;
	new_cpu->next       = NULL;

	if (available_cpus)
	{ /* we navigate to the bottom of the list... */
		struct cpufreq_available_cpus *curr_cpu = available_cpus;
		for (; curr_cpu->next != NULL; curr_cpu=curr_cpu->next);
		curr_cpu->next = new_cpu;
		return;
	}

	available_cpus = new_cpu;
}

/* check CPUfreq support via the sysfs file system */
int check_cpufreq_2_6(void)
{
	DIR *dir = opendir(SYSFS_CPU_BASE_DIR);
	struct dirent *entry;

	/* How many CPUs do we have? */
	if (!dir) return 0;
	while ((entry= readdir(dir)))
	{
		size_t  len  = 0;
		char   *line = NULL;
		char   *test;
		char   *filename;
		FILE   *fp;

		if (!strcmp(entry->d_name, "." )) continue;
		if (!strcmp(entry->d_name, "..")) continue;
		if (strncmp(entry->d_name, "cpu", 3)) continue;
			
		/* let's see wether this particular CPU has CPUfreq support... */
		filename = StrApp((char**)NULL, SYSFS_CPU_BASE_DIR, "/", entry->d_name, "/cpufreq/", SYSFS_CPUFREQ_TEST, (char*)NULL);
		fp = fopen(filename, "r");
		if (!fp)
		{ /* no good... */
			free(filename);
			continue;
		}
		if (getline(&line, &len, fp) == -1)
		{ /* no good... */
			free(filename);
			fclose(fp);
			continue;
		}
		fclose(fp);
		test = (char *) calloc(strlen(line)+1, sizeof(char));
		if (!test)
		{
			fprintf(stderr, "allocation failure!\n");
			abort();
		}
		while (sscanf(line, "%s", test) != EOF)
			/* let's see wether we can find a cpu number equal to our one in test file...  */
			if (!strcmp((entry->d_name)+3, test))
			{ /* Got it! */
				int cpu_number = atoi(entry->d_name + 3) + 1;
				add_to_cpus_list(cpu_number, StrApp((char**)NULL, SYSFS_CPU_BASE_DIR, "/", entry->d_name, "/cpufreq/", (char*)NULL));
				break;
			}
		/* no good.. */
		free(filename);
		free(test);
		free(line);
	}
	closedir(dir);
		
	/* did we get results? */
	if (available_cpus) return 1;

	/* What a shame, all these CPU cycles for nothing! */
	return 0;
}

/* Checks wether this machine supports CPU frequency scaling,
 * and builds a list of CPUs available for that...
 */
int check_cpufreq(void)
{
	if (kernel_version == IS_2_6)
		return check_cpufreq_2_6();

	/* 2.4 kernels are not supported at the moment... */

	return 0;
}

char *cpufreq_get_governor_2_6(int cpu)
{
	struct cpufreq_available_cpus *curr_cpu = available_cpus;
	char   *filename;
	FILE   *fp;
	size_t  len  = 0;
	char   *line = NULL;
	char   *test;

	for (; curr_cpu!=NULL; curr_cpu=curr_cpu->next)
		if (curr_cpu->cpu_number == cpu)
			break;

	if (curr_cpu->cpu_number != cpu) return NULL;

	filename = StrApp((char**)NULL, curr_cpu->dir_name, SYSFS_CPUFREQ_SET_GOV, (char*)NULL);
	fp       = fopen(filename, "r");

	if (!fp)
	{
		free(filename);
		return NULL;
	}

	if (getline(&line, &len, fp) == -1)
	{
		fprintf(stderr, "could not read from file %s!\n", filename);
		free(filename);
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	free(filename);

	test = (char*) calloc(strlen(line)+1,sizeof(char));
	if (!test)
	{
		fprintf(stderr, "failure allocating memory!\n");
		abort();
	}
	if (sscanf(line, "%s", test) != 1)
	{
		free(test);
		free(line);
		return NULL;
	}
	free(line);

	return test;
}

/* Get cpufreq governor of CPU #n, where 1<=n<=MAX, where MAX is
 * the number of CPUs you have in your system
 */
char *cpufreq_get_governor(int cpu)
{
	if (cpu < 1) return NULL;

	if (kernel_version == IS_2_6)
		return cpufreq_get_governor_2_6(cpu);

	/* 2.4 kernels are not supported at the moment... */

	return NULL;
}

/* set CPUfreq governor via the sysfs file system */
int set_cpufreq_governor_2_6(char *governor)
{
	struct cpufreq_available_cpus *curr_cpu=available_cpus;
	int retval = 1;

	/* Set desired governor for all available CPUs */
	for (; curr_cpu!=NULL; curr_cpu=curr_cpu->next)
	{
		FILE *fp;
		char *filename;
		int supported = 0;

		fprintf(stderr, "Setting CPU scaling governor \"%s\" for CPU %d...\n", governor, curr_cpu->cpu_number);

		/* check wether this CPU supports the specified governor */
		filename = StrApp((char**)NULL, curr_cpu->dir_name, SYSFS_CPUFREQ_AVAIL_GOV, (char*)NULL);
		fp = fopen(filename, "r");
		if (!fp)
		{
			fprintf(stderr, "could not open file %s!\n", filename);
			free(filename);
			retval = 0;
			continue;
		}
		else
		{
			size_t  len  = 0;
			char   *line = NULL;
			char   *test;
			int     cont = 0;

			if (getline(&line, &len, fp) == -1)
			{
				fprintf(stderr, "could not read from file %s!\n", filename);
				free(filename);
				fclose(fp);
				retval = 0;
				continue;					
			}
			fclose(fp);

			test = (char *) calloc(strlen(line)+1, sizeof(char));
			if (!test)
			{
				fprintf(stderr, "failure allocating memory!\n");
				abort();
			}

			while (sscanf(line+cont, "%s", test) != EOF)
			{
				if (!strcmp(test, governor))
				{
					supported = 1;
					break;
				}
				cont = strstr(line, test) - line + strlen(test);
			}
			free(test);
			free(line);
		}
		free(filename);
		if (!supported)
		{
			fprintf(stderr, "sorry, this CPU does not support the speficied CPUfreq governor...\n");
			retval = 0;
			continue;
		}

		/* OK, let's set the specified governor for this CPU */
		filename = StrApp((char**)NULL, curr_cpu->dir_name, SYSFS_CPUFREQ_SET_GOV, (char*)NULL);

		fp = fopen(filename, "w");
		if (!fp)
		{
			fprintf(stderr, "could not open file %s for writing!\n", filename);
			free(filename);
			retval = 0;
			continue;
		}
		if (fprintf(fp, "%s", governor) <= 0)
		{
			fprintf(stderr, "failed writing to file %s!\n", filename);
			free(filename);
			fclose(fp);
			retval = 0;
			continue;
		}
		fclose(fp);
		/* OK, we supposedly set our governor on this CPU, let's see wether it actually went through... */
		fp = fopen(filename, "r");
		if (!fp)
		{
			fprintf(stderr, "could not open file %s for reading!\n", filename);
			free(filename);
			retval = 0;
			continue;
		}
		else
		{
			size_t  len  = 0;
			char   *line = NULL;
			char   *test;
				
			if (getline(&line, &len, fp) == -1)
			{
				fprintf(stderr, "could not read from file %s!\n", filename);
				free(filename);
				fclose(fp);
				retval = 0;
				continue;					
			}
			fclose(fp);
			test = (char *) calloc(strlen(line)+1,sizeof(char));
			if (!test)
			{
				fprintf(stderr, "failure allocating memory!\n");
				abort();
			}
			if (sscanf(line, "%s", test) != 1)
			{
				fprintf(stderr, "could not read from file %s!\n", filename);
				free(filename);
				free(test);
				free(line);
				retval = 0;
				continue;
			}
			free(line);
			if (strcmp(test, governor))
			{
				fprintf(stderr, "failed setting specified CPUfreq governor!\n");
				retval = 0;
			}
			free(test);
		}
	}
	return retval;
}


/* Set cpufreq governor */
int cpufreq_set_governor(char *governor)
{
	if (!governor)         return 0;
	if (!available_cpus)   return 0;
	if (!strlen(governor)) return 0;

	if (kernel_version == IS_2_6)
		return set_cpufreq_governor_2_6(governor);


	/* 2.4 kernels are not supported at the moment */

	return 0;
}
