/*
 * Copyright (c) 2007 Daniel Borca  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../list.h"
#include "../sensors.h"


static int
spawn_sync_read (const char *filename, const char *const argv[], int max, char *out)
{
#define READ  0
#define WRITE 1
    pid_t pid;
    int fd[2];
    int status;

    int size;
    int eof;

    if (pipe(fd) != 0) {
	return -1;
    }

    pid = fork();
    if (pid == -1) {
	return -1;
    }
    if (pid == 0) {
	close(fd[READ]);
	close(STDERR_FILENO);
	dup2(fd[WRITE], STDOUT_FILENO);

	exit(execvp(filename, (char *const *)argv));
    }

    close(fd[WRITE]);

    for (--max, size = 0, eof = 0; !eof;) {
	char buf[512];
	int chunk = read(fd[READ], buf, sizeof(buf));
	if (chunk < 0) {
	    break;
	}
	eof = (chunk < (int)sizeof(buf));
	if (size + chunk > max) {
	    chunk = max - size;
	}
	memcpy(&out[size], buf, chunk);
	size += chunk;
    }
    out[size] = '\0';

    close(fd[READ]);

    wait(&status);

    return size;
#undef WRITE
#undef READ
}


int
sensors_nvidia (const char *setting, int *val)
{
    const char *argv[] = { "nvidia-settings", "-q" , NULL, NULL };
    char out[BUFSIZ];
    int dummy;

    argv[2] = (char *)setting;

    if (val == NULL) {
	val = &dummy;
    }

    if (spawn_sync_read(argv[0], argv, sizeof(out), out) < 0) {
	return -1;
    }

    if (sscanf(out, " Attribute %*s %*s %d", val) != 1) {
	return -1;
    }

    return 0;
}


int
sensors_read_line (const char *filename, int max, char *out)
{
    char *p;
    FILE *f;
    f = fopen(filename, "rt");
    if (f == NULL) {
	return -1;
    }
    if (fgets(out, max, f) == NULL) {
	fclose(f);
	return -1;
    }
    fclose(f);
    p = strchr(out, '\n');
    if (p != NULL) {
	*p = '\0';
	return p - out;
    }
    return strlen(out);
}


static int
add_sensor (SENSOR *list, const char *filename, const char *name, SENSOR_TYPE type)
{
    SENSOR *s;
    char *p, buf[256];

    s = malloc(sizeof(SENSOR));
    if (s == NULL) {
	return -1;
    }
    s->type = type;
    s->idata = 0;

    switch (type) {
	case S_ACPI_THERMAL_ZONE:
	    p = malloc(strlen(filename) + 32);
	    if (p == NULL) {
		break;
	    }
	    strcat(strcpy(p, filename), "/trip_points");
	    s->idata = 100;
	    if (sensors_read_line(p, sizeof(buf), buf) > 0) {
		sscanf(buf, "%*s %*s %d", &s->idata);
	    }
	    s->filename = strcat(strcpy(p, filename), "/temperature");
	    s->name = strdup(name);
	    if (s->name == NULL) {
		free(p);
		break;
	    }
	    list_append(list, s);
	    return 0;
	case S_ACPI_AC_ADAPTER:
	    p = malloc(strlen(filename) + 32);
	    if (p == NULL) {
		break;
	    }
	    s->filename = strcat(strcpy(p, filename), "/state");
	    s->name = strdup(name);
	    if (s->name == NULL) {
		free(p);
		break;
	    }
	    list_append(list, s);
	    return 0;
	case S_ACPI_BATTERY:
	    p = malloc(strlen(filename) + 32);
	    if (p == NULL) {
		break;
	    }
	    s->filename = strcat(strcpy(p, filename), "/");
	    s->name = strdup(name);
	    if (s->name == NULL) {
		free(p);
		break;
	    }
	    list_append(list, s);
	    return 0;
	case S_HWMON_CORETEMP:
	    p = malloc(strlen(filename) + 32);
	    if (p == NULL) {
		break;
	    }
	    strcat(strcpy(p, filename), "/device/name");
	    if (sensors_read_line(p, sizeof(buf), buf) <= 0) {
		free(p);
		break;
	    }
	    if (strcmp(buf, "coretemp")) {
		free(p);
		break;
	    }
	    strcat(strcpy(p, filename), "/device/temp1_crit");
	    s->idata = 100;
	    if (sensors_read_line(p, sizeof(buf), buf) > 0) {
		sscanf(buf, "%d", &s->idata);
	    }
	    strcat(strcpy(p, filename), "/device/temp1_label");
	    if (sensors_read_line(p, sizeof(buf), buf) > 0) {
		s->name = strdup(buf);
	    } else {
		s->name = strdup(name);
	    }
	    s->filename = strcat(strcpy(p, filename), "/device/temp1_input");
	    if (s->name == NULL) {
		free(s->filename);
		break;
	    }
	    list_append(list, s);
	    return 0;
	case S_NVIDIA_SETTINGS_GPUCORETEMP:
	    if (sensors_nvidia(name, NULL) != 0) {
		break;
	    }
	    p = strdup(filename);
	    if (p == NULL) {
		break;
	    }
	    s->filename = p;
	    s->name = strdup(name);
	    if (s->name == NULL) {
		free(p);
		break;
	    }
	    list_append(list, s);
	    return 0;
    }

    free(s);
    return -1;
}


static void
scan_dirs (const char *dirname, SENSOR *list, SENSOR_TYPE type)
{
    DIR *dir;
    int len = strlen(dirname);

    dir = opendir(dirname);
    if (dir != NULL) {
	struct dirent *ent;
	while ((ent = readdir(dir))) {
	    if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
		char *fullname = malloc(len + 1 + strlen(ent->d_name) + 1);
		if (fullname != NULL) {
		    struct stat buf;
		    int l = len;
		    strcpy(fullname, dirname);
		    if (fullname[l - 1] != '/') {
			fullname[l++] = '/';
		    }
		    strcpy(&fullname[l], ent->d_name);
		    /* don't use d_type */
		    if (stat(fullname, &buf) == 0 && S_ISDIR(buf.st_mode)) {
			add_sensor(list, fullname, ent->d_name, type);
		    }
		    free(fullname);
		}
	    }
	}
	closedir(dir);
    }
}


SENSOR *
sensors_init (void)
{
    SENSOR *list;

    list = malloc(sizeof(SENSOR));
    if (list == NULL) {
	return NULL;
    }
    list->type = -1;

    list_create(list);

    scan_dirs("/proc/acpi/thermal_zone", list, S_ACPI_THERMAL_ZONE);
    scan_dirs("/proc/acpi/ac_adapter", list, S_ACPI_AC_ADAPTER);
    scan_dirs("/proc/acpi/battery", list, S_ACPI_BATTERY);
    scan_dirs("/sys/class/hwmon", list, S_HWMON_CORETEMP);
    add_sensor(list, "nvidia-settings", "GPUCoreTemp", S_NVIDIA_SETTINGS_GPUCORETEMP);

    return list;
}


void
sensors_free (SENSOR *list)
{
    SENSOR *s, *tmp;

    list_foreach_s (s, tmp, list) {
	list_remove(s);
	free(s->filename);
	free((char *)s->name);
	free(s);
    }

    free(list);
}
