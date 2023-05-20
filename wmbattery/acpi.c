/*
 * A not-yet-general-purpose ACPI library, by Joey Hess <joey@kitenet.net>
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <fnmatch.h>
#ifdef ACPI_APM
#include "apm.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "acpi.h"

#define SYSFS_PATH "/sys/class/power_supply"
#define ACPI_MAXITEM 8

int acpi_batt_count = 0;
/* Filenames of the battery info files for each system battery. */
char acpi_batt_info[ACPI_MAXITEM][128];
/* Filenames of the battery status files for each system battery. */
char acpi_batt_status[ACPI_MAXITEM][128];
/* Stores battery capacity, or 0 if the battery is absent. */
int acpi_batt_capacity[ACPI_MAXITEM];

int acpi_ac_count = 0;
char acpi_ac_adapter_info[ACPI_MAXITEM][128];
char acpi_ac_adapter_status[ACPI_MAXITEM][128];

char *acpi_labels[] = {
	"uevent",
	"status",
	"Battery",
	"Mains",
	"POWER_SUPPLY_CAPACITY=",
	"POWER_SUPPLY_??????_FULL_DESIGN=", /* CHARGE or ENERGY */
	"POWER_SUPPLY_PRESENT=",
	"POWER_SUPPLY_??????_NOW=",
	"POWER_SUPPLY_CURRENT_NOW=",
	"POWER_SUPPLY_STATUS=",
#if ACPI_THERMAL
	"thermal_zone",
#endif
	"POWER_SUPPLY_ONLINE=",
	"POWER_SUPPLY_??????_FULL=",
	NULL
};

#if ACPI_THERMAL
int acpi_thermal_count = 0;
char acpi_thermal_info[ACPI_MAXITEM][128];
char acpi_thermal_status[ACPI_MAXITEM][128];
#endif

/* Read in an entire ACPI proc file (well, the first 1024 bytes anyway), and
 * return a statically allocated array containing it. */
inline char *get_acpi_file(const char *file)
{
	int fd;
	int end;
	static char buf[1024];
	fd = open(file, O_RDONLY);
	if (fd == -1)
		return NULL;
	end = read(fd, buf, sizeof(buf) - 1);
	buf[end] = '\0';
	close(fd);
	return buf;
}

int strmcmp(const char *s1, const char *s2)
{
	for (; (*s1 == *s2) || (*s2 == '?'); s1++, s2++) {
		if (*s1 == '\0')
			return 0;
	}
	if (*s2 == '\0')
		return 0;
	else
		return 1;
}

/* Given a buffer holding an acpi file, searches for the given key in it,
 * and returns the numeric value. 0 is returned on failure. */
inline int scan_acpi_num(const char *buf, const char *key)
{
	char *ptr;
	int ret = 0;

	do {
		ptr = strchr(buf, '\n');
		if (!strmcmp(buf, key)) {
			ptr = strchr(buf, '=');
			if (ptr) {
				sscanf(ptr + 1, "%d", &ret);
				return ret;
			} else {
				return 0;
			}
		}
		if (ptr)
			ptr++;
		buf = ptr;
	} while (buf != NULL);
	return 0;
}

/* Given a buffer holding an acpi file, searches for the given key in it,
 * and returns its value in a statically allocated string. */
inline char *scan_acpi_value(const char *buf, const char *key)
{
	char *ptr;
	static char ret[256];

	do {
		ptr = strchr(buf, '\n');
		if (!strmcmp(buf, key)) {
			ptr = strchr(buf, '=');
			if (ptr) {
				if (sscanf(ptr + 1, "%255s", ret) == 1)
					return ret;
				else
					return NULL;
			} else {
				return NULL;
			}
		}
		if (ptr)
			ptr++;
		buf = ptr;
	} while (buf != NULL);
	return NULL;
}

/* Read an ACPI proc file, pull out the requested piece of information, and
 * return it (statically allocated string). Returns NULL on error, This is
 * the slow, dumb way, fine for initialization or if only one value is needed
 * from a file, slow if called many times. */
char *get_acpi_value(const char *file, const char *key)
{
	char *buf = get_acpi_file(file);
	if (!buf)
		return NULL;
	return scan_acpi_value(buf, key);
}

/* Returns the last full charge capacity of a battery.
 */
int get_acpi_batt_capacity(int battery)
{
	char *s;

	s = get_acpi_value(acpi_batt_info[battery], acpi_labels[label_last_full_capacity]);
	if (s == NULL)
		return 0;
	else
		return atoi(s);
}

/* Comparison function for qsort. */
int _acpi_compare_strings(const void *a, const void *b)
{
	const char **pa = (const char **)a;
	const char **pb = (const char **)b;
	return strcasecmp((const char *)*pa, (const char *)*pb);
}

/* Find something (batteries, ac adpaters, etc), and set up a string array
 * to hold the paths to info and status files of the things found.
 * Returns the number of items found. */
int find_items(char *itemname, char infoarray[ACPI_MAXITEM][128],
		char statusarray[ACPI_MAXITEM][128])
{
	DIR *dir;
	struct dirent *ent;
	int num_devices = 0;
	int i;
	char **devices = malloc(ACPI_MAXITEM * sizeof(char *));

	dir = opendir(SYSFS_PATH);
	if (dir == NULL) {
		free(devices);
		return 0;
	}
	while ((ent = readdir(dir))) {
		char filename[sizeof(SYSFS_PATH) + 1 + sizeof(ent->d_name) + 6];
		char buf[1024];

		if (!strcmp(".", ent->d_name) ||
		    !strcmp("..", ent->d_name))
			continue;

		snprintf(filename, sizeof(filename), SYSFS_PATH "/%s/type", ent->d_name);
		int fd = open(filename, O_RDONLY);
		if (fd != -1) {
			int end = read(fd, buf, sizeof(buf));
			buf[end-1] = '\0';
			close(fd);
			if (strstr(buf, itemname) != buf)
				continue;
		}

		devices[num_devices] = strdup(ent->d_name);
		num_devices++;
		if (num_devices >= ACPI_MAXITEM)
			break;
	}
	closedir(dir);

	/* Sort, since readdir can return in any order. /sys/ does
	 * sometimes list BAT1 before BAT0. */
	qsort(devices, num_devices, sizeof(char *), _acpi_compare_strings);

	for (i = 0; i < num_devices; i++) {
		snprintf(infoarray[i], sizeof(infoarray[i]), SYSFS_PATH "/%s/%s", devices[i],
			acpi_labels[label_info]);
		snprintf(statusarray[i], sizeof(statusarray[i]), SYSFS_PATH "/%s/%s", devices[i],
			acpi_labels[label_status]);
		free(devices[i]);
	}

	free(devices);
	return num_devices;
}

/* Find batteries, return the number, and set acpi_batt_count to it as well. */
int find_batteries(void)
{
	int i;
	acpi_batt_count = find_items(acpi_labels[label_battery], acpi_batt_info, acpi_batt_status);
	for (i = 0; i < acpi_batt_count; i++)
		acpi_batt_capacity[i] = get_acpi_batt_capacity(i);
	return acpi_batt_count;
}

/* Find AC power adapters, return the number found, and set acpi_ac_count to it
 * as well. */
int find_ac_adapters(void)
{
	acpi_ac_count = find_items(acpi_labels[label_ac_adapter], acpi_ac_adapter_info, acpi_ac_adapter_status);
	return acpi_ac_count;
}

#if ACPI_THERMAL
/* Find thermal information sources, return the number found, and set
 * thermal_count to it as well. */
int find_thermal(void)
{
	acpi_thermal_count = find_items(acpi_labels[label_thermal], acpi_thermal_info, acpi_thermal_status);
	return acpi_thermal_count;
}
#endif

/* Returns true if the system is on ac power. Call find_ac_adapters first. */
int on_ac_power(void)
{
	int i;
	for (i = 0; i < acpi_ac_count; i++) {
		char *online = get_acpi_value(acpi_ac_adapter_info[i], acpi_labels[label_ac_state]);
		if (online && atoi(online))
			return 1;
		else
			return 0;
	}
	return 0;
}

/* See if we have ACPI support and check version. Also find batteries and
 * ac power adapters. */
int acpi_supported(void)
{
	char *version;
	DIR *dir;
	int num;

	dir = opendir(SYSFS_PATH);
	if (!dir)
		return 0;
	closedir(dir);

	/* If kernel is 2.6.21 or newer, version is in
	   /sys/module/acpi/parameters/acpica_version */

	version = get_acpi_file("/sys/module/acpi/parameters/acpica_version");
	if (version == NULL)
		return 0;
	num = atoi(version);
	if (num < ACPI_VERSION) {
		fprintf(stderr, "ACPI subsystem %s too is old, consider upgrading to %i.\n",
				version, ACPI_VERSION);
		return 0;
	}

	find_batteries();
	find_ac_adapters();
#if ACPI_THERMAL
	find_thermal();
#endif

	return 1;
}

#ifdef ACPI_APM
/* Read ACPI info on a given power adapter and battery, and fill the passed
 * apm_info struct. */
int acpi_read(int battery, apm_info *info)
{
	char *buf, *state;

	if (acpi_batt_count == 0) {
		info->battery_percentage = 0;
		info->battery_time = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
		acpi_batt_capacity[battery] = 0;
		/* Where else would the power come from, eh? ;-) */
		info->ac_line_status = 1;
		return 0;
	}

	/* Internally it's zero indexed. */
	battery--;

	buf = get_acpi_file(acpi_batt_info[battery]);
	if (buf == NULL) {
		fprintf(stderr, "unable to read %s\n", acpi_batt_info[battery]);
		perror("read");
		exit(1);
	}

	info->ac_line_status = 0;
	info->battery_flags = 0;
	info->using_minutes = 1;

	/* Work out if the battery is present, and what percentage of full
	 * it is and how much time is left. */
	if (strcmp(scan_acpi_value(buf, acpi_labels[label_present]), "1") == 0) {
		int pcap = scan_acpi_num(buf, acpi_labels[label_remaining_capacity]);
		int rate = scan_acpi_num(buf, acpi_labels[label_present_rate]);
		if (rate) {
			/* time remaining = (current_capacity / discharge rate) */
			info->battery_time = (float) pcap / (float) rate * 60;
		} else {
			char *rate_s = scan_acpi_value(buf, acpi_labels[label_present_rate]);
			if (!rate_s) {
				/* Time remaining unknown. */
				info->battery_time = 0;
			} else {
				/* a zero or unknown in the file; time
				 * unknown so use a negative one to
				 * indicate this */
				info->battery_time = -1;
			}
		}

		state = scan_acpi_value(buf, acpi_labels[label_charging_state]);
		if (state) {
			if (state[0] == 'D') { /* discharging */
				info->battery_status = BATTERY_STATUS_CHARGING;
				/* Expensive ac power check used here
				 * because AC power might be on even if a
				 * battery is discharging in some cases. */
				info->ac_line_status = on_ac_power();
			} else if (state[0] == 'C' && state[1] == 'h') { /* charging */
				info->battery_status = BATTERY_STATUS_CHARGING;
				info->ac_line_status = 1;
				info->battery_flags = info->battery_flags | BATTERY_FLAGS_CHARGING;
				if (rate)
					info->battery_time = -1 * (float) (acpi_batt_capacity[battery] - pcap) /
						(float) rate * 60;
				else
					info->battery_time = 0;
				if (abs(info->battery_time) < 0.5)
					info->battery_time = 0;
			} else if (state[0] == 'F') { /* full */
				/* charged, on ac power */
				info->battery_status = BATTERY_STATUS_HIGH;
				info->ac_line_status = 1;
			} else if (state[0] == 'C') { /* not charging, so must be critical */
				info->battery_status = BATTERY_STATUS_CRITICAL;
				/* Expensive ac power check used here
				 * because AC power might be on even if a
				 * battery is critical in some cases. */
				info->ac_line_status = on_ac_power();
			} else if (state[0] == 'U') { /* unknown */
				info->ac_line_status = on_ac_power();
				int current = scan_acpi_num(buf, acpi_labels[label_present_rate]);
				if (info->ac_line_status) {
					if (current == 0)
						info->battery_status = BATTERY_STATUS_HIGH;
					else
						info->battery_status = BATTERY_STATUS_CHARGING;
				} else {
					info->battery_status = BATTERY_STATUS_CHARGING;
				}
			} else {
				fprintf(stderr, "unknown battery state: %s\n", state);
			}
		} else {
			/* Battery state unknown. */
			info->battery_status = BATTERY_STATUS_ABSENT;
		}

		if (acpi_batt_capacity[battery] == 0) {
			/* The battery was absent, and now is present.
			 * Well, it might be a different battery. So
			 * re-probe the battery. */
			/* NOTE that this invalidates buf. No accesses of
			 * buf below this point! */
			acpi_batt_capacity[battery] = get_acpi_batt_capacity(battery);
		} else if (pcap > acpi_batt_capacity[battery]) {
			/* Battery is somehow charged to greater than max
			 * capacity. Rescan for a new max capacity. */
			find_batteries();
		}

		if (pcap && acpi_batt_capacity[battery]) {
			info->battery_percentage = 100 * pcap / acpi_batt_capacity[battery];
			if (info->battery_percentage > 100)
				info->battery_percentage = 100;
		} else {
			info->battery_percentage = -1;
		}

	} else {
		info->battery_percentage = 0;
		info->battery_time = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
		acpi_batt_capacity[battery] = 0;
		if (acpi_batt_count == 0) {
			/* Where else would the power come from, eh? ;-) */
			info->ac_line_status = 1;
		} else {
			/* Expensive ac power check. */
			info->ac_line_status = on_ac_power();
		}
	}

	return 0;
}
#endif
