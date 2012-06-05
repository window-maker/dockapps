/***************************************************************************
                          libacpi.c  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003 by Noberasco Michele
    e-mail               : 2001s098@educ.disi.unige.it
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.              *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
        Originally written by Costantino Pistagna for his wmacpimon
***************************************************************************/


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>

#include "libacpi.h"
#include "power_management.h"
#include "lib_utils.h"


/* here we put temp stuff read from proc files */
char buf[512];

static char batteries[MAXBATT][128];
static char  battinfo[MAXBATT][128];
static char      fans[MAXFANS][128];


void sort(char names[MAXBATT][128], int count)
{
	int i, j;
	char temp[128];

	for (i=0; i<(count-1); i++)
		for (j=i+1; j<count; j++)
			if (strcmp(names[j], names[i]) < 0)
			{
				strcpy(temp, names[j]);
				strcpy(names[j], names[i]);
				strcpy(names[i], temp);
			}
}

/* see if we have ACPI support */
int check_acpi(void)
{
  DIR *battdir;
  struct dirent *batt;
  char *name;

	/* do proc entries for acpi exist? */
	if (access("/proc/acpi/info", R_OK) != 0) return 0;

  /* now enumerate batteries */
  batt_count = 0;
  battdir = opendir ("/proc/acpi/battery");
  if (!battdir) return 0;

  while ((batt = readdir (battdir)))
  {
    name = batt->d_name;

    /* skip . and .. */
    if (!strcmp (".", name) || !strcmp ("..", name)) continue;

    if (!access("/proc/acpi/battery/1/status", R_OK))
	    sprintf (batteries[batt_count], "/proc/acpi/battery/%s/status", name);
    else
	    sprintf (batteries[batt_count], "/proc/acpi/battery/%s/state", name);
    sprintf (battinfo[batt_count], "/proc/acpi/battery/%s/info", name);

    batt_count++;
  }
  closedir (battdir);

	/* order battery names as readdir doesn't handle that */
	if (batt_count > 1)
	{
		sort(batteries, batt_count);
		sort(battinfo,  batt_count);
	}

	fprintf(stderr, "libacpi: found %d batter%s\n", batt_count, (batt_count == 1) ? "y" : "ies");

  return 1;
}


char *find_acad_proc_file(void)
{
  DIR *dir;
  char *basedir = "/proc/acpi/ac_adapter/";
  struct dirent *entry;

  dir= opendir(basedir);	
  if (!dir) return NULL;

  while ((entry= readdir(dir)))
  {
		char *result = NULL;
		char *temp1, *temp2, *temp3;
    if (!strcmp(entry->d_name, "." )) continue;
    if (!strcmp(entry->d_name, "..")) continue;    
    temp1 = StrApp((char**)NULL, basedir, entry->d_name, "/state",  (char*)NULL);
		temp2 = StrApp((char**)NULL, basedir, entry->d_name, "/status", (char*)NULL);
		temp3 = StrApp((char**)NULL, basedir, entry->d_name, "/stats",  (char*)NULL);
		
		if      (!access(temp1, R_OK)) {result = temp1; free(temp2); free(temp3);}
		else if (!access(temp2, R_OK)) {result = temp2; free(temp1); free(temp3);}
		else if (!access(temp3, R_OK)) {result = temp3; free(temp1); free(temp2);}

		if (result)
		{
			closedir(dir);
			return result;
		}
		free(temp1); free(temp2); free(temp3);
  }
  closedir(dir);

	return NULL;
}

void read_acad_state (ACADstate *acadstate)
{
	static int   searched = 0;
	static char *file     = NULL;	
	static char *where    = NULL;
	FILE *fp;

	if (!searched)
	{
		file = find_acad_proc_file();
		searched = 1;
	}
	if (!file) return;
	if (!(fp = fopen(file, "r"))) return;

	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);

	if (!where)
	{
		if (!strncmp(buf, "state:",  6)) where = buf + 26;
		if (!strncmp(buf, "Status:", 7)) where = buf + 26;
	}
	if (!where) return;
	
	if (where[0] == 'n') acadstate->state = 1;
	if (where[0] == 'f') acadstate->state = 0;	
}


void read_acpi_info (ACPIinfo *acpiinfo, int battery)
{
	FILE *fp;
	char *ptr    = buf;
	int   offset = 25;

  if (battery > MAXBATT) return;
  if (!(fp = fopen (battinfo[battery], "r"))) return;

	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);

	while (ptr)
	{
		static int count = 0;

		if (!strncmp(ptr+1, "resent:", 7))
		{
			if (ptr[offset] != 'y')
			{
				acpiinfo->present = 0;
				acpiinfo->design_capacity = 0;
				acpiinfo->last_full_capacity = 0;
				acpiinfo->battery_technology = 0;
				acpiinfo->design_voltage = 0;
				acpiinfo->design_capacity_warning = 0;
				acpiinfo->design_capacity_low = 0;
				return;
			}
			acpiinfo->present = 1;
			ptr = jump_next_line(ptr);
			if (!ptr) break;
		}
		if (!strncmp(ptr, "design capacity:", 16) || !strncmp(ptr, "Design Capacity:", 16))
    {
      sscanf (ptr+offset, "%d", &(acpiinfo->design_capacity));
			ptr = jump_next_line(ptr);
			if (!ptr) break;
    }
		if (!strncmp(ptr, "last full capacity:", 19) || !strncmp(ptr, "Last Full Capacity:", 19))
    {
      sscanf (ptr+offset, "%d", &(acpiinfo->last_full_capacity));
			ptr = jump_next_line(ptr);
			if (!ptr) break;
    }
		if (!strncmp(ptr, "battery technology:", 19) || !strncmp(ptr, "Battery Technology:", 19))
    {
      switch (ptr[offset])
	    {
	      case 'n':
	        acpiinfo->battery_technology = 1;
	        break;
	      case 'r':
	        acpiinfo->battery_technology = 0;
	        break;
	    }
			ptr = jump_next_line(ptr);
			if (!ptr) break;
    }
		if (!strncmp(ptr, "design voltage:", 15) || !strncmp(ptr, "Design Voltage:", 15))
    {
      sscanf (ptr+offset, "%d", &(acpiinfo->design_voltage ));
			ptr = jump_next_line(ptr);
			if (!ptr) break;
    }
		if (!strncmp(ptr, "design capacity warning:", 24) || !strncmp(ptr, "Design Capacity Warning:", 24))
		{
      sscanf (ptr+offset, "%d", &(acpiinfo->design_capacity_warning));
			ptr = jump_next_line(ptr);
			if (!ptr) break;
		}
		if (!strncmp(ptr, "design capacity low:", 20) || !strncmp(ptr, "Design Capacity Low:", 20))
		{
			sscanf (ptr+offset, "%d", &(acpiinfo->design_capacity_low));
			if (!count) return; /* we did read all stuff in just one passage! */
			ptr = jump_next_line(ptr);
			if (!ptr) break;
		}

		ptr = jump_next_line(ptr);
		count++;
	}
}


void read_acpi_state (ACPIstate *acpistate, ACPIinfo *acpiinfo, int battery)
{
	FILE *fp;
	char *ptr = buf;
	int offset = 25;

  if (battery > MAXBATT) return;
	if (!(fp = fopen (batteries[battery], "r"))) return;

	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);

	while (ptr)
	{
		static int count = 0;

		if (!strncmp(ptr+1, "resent:", 7))
		{
			if (ptr[offset] != 'y')
			{
				acpistate->present    = 0;
				acpistate->state      = UNKNOW;
				acpistate->prate      = 0;
				acpistate->rcapacity  = 0;
				acpistate->pvoltage   = 0;
				acpistate->rtime      = 0;
				acpistate->percentage = 0;

				return;
			}
			acpistate->present = 1;
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "capacity state:", 15))
		{
			/* nothing to do here... */
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "charging state:", 15) || !strncmp(ptr, "State:", 6))
		{
			switch (ptr[offset])
			{
				case 'd':
					acpistate->state = 1;
					break;
				case 'c':
				{
					if (*(ptr + 33) == '/') acpistate->state = 0;
					if (!strncmp(ptr+offset, "charged", 7))
						acpistate->state = 1;
					else
						acpistate->state = 2;
					break;
				}
				case 'u':
					acpistate->state = 3;
					break;
			}
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "present rate:", 13) || !strncmp(ptr, "Present Rate:", 13))
		{
			sscanf (ptr+offset, "%d", &(acpistate->prate));
			/* if something wrong */
			if (acpistate->prate <= 0) acpistate->prate = 0;
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "remaining capacity:", 19) || !strncmp(ptr, "Remaining Capacity:", 19))
		{
			sscanf (ptr+offset, "%d", &(acpistate->rcapacity));
			acpistate->percentage =	(float) ((float) acpistate->rcapacity / (float) acpiinfo->last_full_capacity) * 100;
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "present voltage:", 16) || !strncmp(ptr, "Battery Voltage:", 16))
		{
			sscanf (ptr+offset, "%d", &(acpistate->pvoltage));
			if (!count) break; /* we did read all stuff in just one passage! */
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}

		ptr = jump_next_line(ptr);
		count++;
	}

	/* time remaining in minutes */
	if (!acpistate->prate) return;
	if (acpistate->state == 2) /* charging */
		acpistate->rtime = ((float) ((float) (acpiinfo->last_full_capacity - acpistate->rcapacity) / (float) acpistate->prate)) * 60;
	else /* discharging */
		acpistate->rtime = ((float) ((float) acpistate->rcapacity / (float) acpistate->prate)) * 60;
	if (acpistate->rtime <= 0) acpistate->rtime = 0;
}

char *find_temperature_proc_file(void)
{
  DIR *dir;
  char *basedir = "/proc/acpi/thermal_zone/";
  struct dirent *entry;

  dir= opendir(basedir);	
  if (!dir) return NULL;

  while ((entry= readdir(dir)))
  {
		char *temp;
    if (!strcmp(entry->d_name, "." )) continue;
    if (!strcmp(entry->d_name, "..")) continue;    
    temp = StrApp((char**)NULL, basedir, entry->d_name, "/temperature", (char*)NULL);
		if (!access(temp, R_OK))
		{
			closedir(dir);
			return temp;
		}
		free(temp);
  }
  closedir(dir);

	return NULL;
}

void acpi_get_temperature(int *temperature, int *temp_is_celsius)
{
	static int   temp;
	static char  unit[2];
	static int   searched = 0;
	static char *file     = NULL;
	FILE *fp;

	(*temperature)     = PM_Error;
	(*temp_is_celsius) = PM_Error;

	if (!searched)
	{
		file = find_temperature_proc_file();
		searched = 1;
	}
	if (!file) return;
	if (!(fp=fopen(file, "r"))) return;
	if (!fgets(buf, 512, fp))
	{
		fclose(fp);
		return;
	}
	fclose(fp);

	if (sscanf(buf, "%*s%d%1s", &temp, unit) != 2) return;

	(*temperature) = temp;
	if (*unit == 'C') (*temp_is_celsius) = 1;
	else (*temp_is_celsius) = 0;
}

int get_fan_info(void)
{
	struct dirent *entry;
	char *basedir = "/proc/acpi/fan";
	DIR  *dir;
	int   n_fans = 0;

	dir = opendir(basedir);
	if (!dir) return 0;
	while ((entry= readdir(dir)))
  {
		char *temp;
    if (!strcmp(entry->d_name, "." )) continue;
    if (!strcmp(entry->d_name, "..")) continue;    
    temp = StrApp((char**)NULL, basedir, "/", entry->d_name, "/state", (char*)NULL);
		if (!access(temp, R_OK))
		{
			if (n_fans == MAXFANS)
			{
				fprintf(stderr, "acpi_lib: found more fans, but wmpower can handle only %d.\n", MAXFANS);
				free(temp);
				break;
			}
			strncpy(fans[n_fans], temp, 127);
			fans[n_fans][127] = '\0';
			n_fans++;
			fprintf(stderr, "acpi_lib: found fan info at '%s'\n", temp);
		}
		else fprintf(stderr, "acpi_lib: cannot access fan info at '%s'\n", temp);
		free(temp);
	}
	closedir(dir);
	fprintf(stderr, "acpi_lib: %d fan(s) found...\n", n_fans);

	return n_fans;
}

/* return number of fans running */
int acpi_get_fan_status(void)
{
	static int n_fans = -1;
	int running_fans  =  0;
	int i;

	if (n_fans == -1) n_fans = get_fan_info();
	if (!n_fans) return PM_Error;

	for (i=0; i<n_fans; i++)
	{
		char *ptr  = buf;
		FILE *fp   = fopen(fans[i], "r");
		int offset = 26;

		if (!fp) return PM_Error;
		if (!fgets(buf, 512, fp)) ptr = NULL;
		fclose(fp);
		if (!ptr) return PM_Error;
		if (ptr[offset] == 'n') running_fans++;
	}

	return running_fans;
}
