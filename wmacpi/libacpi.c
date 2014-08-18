#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include "libacpi.h"

extern char *state[];
extern APMInfo *apminfo;
/* temp buffer */
char buf[512];

/* local proto */
int acpi_get_design_cap(int battery);

/* see if we have ACPI support and check version */
int power_init(void)
{
    FILE *acpi;
    char buf[4096];
    DIR *battdir;
    struct dirent *batt;
    char *name, *tmp1, *tmp2;
    char *names[MAXBATT];
    int i, j;
    int acpi_ver = 0;

    if (!(acpi = fopen("/proc/acpi/info", "r"))) {
	fprintf(stderr, "This system does not support ACPI\n");
	return 1;
    }

    /* okay, now see if we got the right version */
    fread(buf, 4096, 1, acpi);
    acpi_ver = strtol(buf + 25, NULL, 10);
    eprint(0, "ACPI version detected: %d\n", acpi_ver);
    if (acpi_ver < 20020214) {
	fprintf(stderr, "This version requires ACPI subsystem version 20020214\n");
	fclose(acpi);
	return 1;
    }

    /* yep, all good */
    fclose(acpi);

    /* now enumerate batteries */
    batt_count = 0;
    battdir = opendir("/proc/acpi/battery");
    if (battdir == NULL) {
	fprintf(stderr, "No batteries or ACPI not supported\n");
	return 1;
    }
    while ((batt = readdir(battdir))) {
	/* there's a serious problem with this code when there's
	 * more than one battery: the readdir won't return the
	 * entries in sorted order, so battery one won't 
	 * necessarily be the first one returned. So, we need
	 * to sort them ourselves before adding them to the 
	 * batteries array. */
	name = batt->d_name;
	
	/* skip . and .. */
	if (!strncmp(".", name, 1) || !strncmp("..", name, 2))
	    continue;

	names[batt_count] = strdup(name);
	batt_count++;
    }
    closedir(battdir);

    /* A nice quick insertion sort, ala CLR. */
    for (i = 1; i < batt_count; i++) {
	tmp1 = names[i];
	j = i - 1;
	while ((j >= 0) && ((strcmp(tmp1, names[j])) < 0)) {
	    tmp2 = names[j+1];
	    names[j+1] = names[j];
	    names[j] = tmp2;
	}
    }
    
    for (i = 0; i < batt_count; i++) {
	snprintf(batteries[i].name, MAX_NAME, "%s", names[i]);
	snprintf(batteries[i].info_file, MAX_NAME, 
		 "/proc/acpi/battery/%s/info", names[i]);
	snprintf(batteries[i].state_file, MAX_NAME, 
		 "/proc/acpi/battery/%s/state", names[i]);
	eprint(0, "battery detected at %s\n", batteries[i].info_file);
	fprintf(stderr, "found battery %s\n", names[i]);
    }

    /* tell user some info */
    eprint(0, "%d batteries detected\n", batt_count);
    fprintf(stderr, "wmacpi: found %d batter%s\n", batt_count,
	    (batt_count == 1) ? "y" : "ies");
    
    return 0;
}

char *get_value(char *string)
{
    char *retval;
    int i;

    if (string == NULL)
	return NULL;

    i = 0;
    while (string[i] != ':') i++;
    while (!isalnum(string[i])) i++;
    retval = (string + i);

    return retval;
}

power_state_t get_power_status(void)
{
    FILE *file;
    char buf[1024];
    char *val;
    
    if ((file = fopen("/proc/acpi/ac_adapter/AC/state", "r")) == NULL) {
	perror("Could not open /proc/acpi/ac_adapter/AC/state\n");
	return PS_ERR;
    }

    fgets(buf, 1024, file);
    fclose(file);
    val = get_value(buf);
    if ((strncmp(val, "on-line", 7)) == 0)
	return AC;
    else
	return BATT;
}

int get_battery_info(int batt_no)
{
    FILE *file;
    battery *info = &batteries[batt_no];
    char buf[1024];
    char *entry;
    int buflen;
    char *val;

    if ((file = fopen(info->info_file, "r")) == NULL) {
	/* this is cheating, but string concatenation should work . . . */
	fprintf(stderr, "Could not open %s:", info->info_file );
	perror(NULL);
	return 0;
    }
    
    /* grab the contents of the file */
    buflen = fread(buf, sizeof(buf), 1, file);
    fclose(file);

    /* check to see if battery is present */
    entry = strstr(buf, "present:");
    val = get_value(entry);
    if ((strncmp(val, "yes", 3)) == 0) {
	info->present = 1;
    } else {
	eprint(0, "Battery %s not present\n", info->name);
	info->present = 0;
	return 0;
    }
    
    /* get design capacity
     * note that all these integer values can also contain the
     * string 'unknown', so we need to check for this. */
    entry = strstr(buf, "design capacity:");
    val = get_value(entry);
    if (val[0] == 'u') 
	info->design_cap = -1;
    else
	info->design_cap = strtoul(val, NULL, 10);

    /* get last full capacity */
    entry = strstr(buf, "last full capacity:");
    val = get_value(entry);
    if (val[0] == 'u')
	info->last_full_cap = -1;
    else
	info->last_full_cap = strtoul(val, NULL, 10);

    /* get design voltage */
    entry = strstr(buf, "design voltage:");
    val = get_value(entry);
    if (val[0] == 'u')
	info->design_voltage = -1;
    else
	info->design_voltage = strtoul(val, NULL, 10);

    
    if ((file = fopen(info->state_file, "r")) == NULL) {
	fprintf(stderr, "Could not open %s:", info->state_file );
	perror(NULL);
	return 0;
    }
    
    /* grab the file contents */
    buflen = fread(buf, sizeof(buf), 1, file);
    fclose(file);

    /* check to see if battery is present */
    entry = strstr(buf, "present:");
    val = get_value(entry);
    if ((strncmp(val, "yes", 3)) == 0) {
	info->present = 1;
    } else {
	info->present = 0;
	eprint(1, "Battery %s no longer present\n", info->name);
	return 0;
    }

    /* get capacity state 
     * note that this has only two values (at least, in the 2.4.21-rc2
     * source code) - ok and critical. */
    entry = strstr(buf, "capacity state:");
    val = get_value(entry);
    if ((strncmp(val, "ok", 2)) == 0)
	info->capacity_state = OK;
    else
	info->capacity_state = CRITICAL;

    /* get charging state */
    entry = strstr(buf, "charging state:");
    val = get_value(entry);
    if ((strncmp(val, "discharging", 10)) == 0)
	info->charging = 0;
    else
	info->charging = 1;

    /* get current rate of burn 
     * note that if it's on AC, this will report 0 */
    entry = strstr(buf, "present rate:");
    val = get_value(entry);
    if (val[0] == 'u') {
	info->present_rate = -1;
    } else {
	int rate;
	rate = strtoul(val, NULL, 10);
	if (rate != 0)
	    info->present_rate = rate;
    }

    /* get remaining capacity */
    entry = strstr(buf, "remaining capacity:");
    val = get_value(entry);
    if (val[0] == 'u')
	info->remaining_cap = -1;
    else
	info->remaining_cap = strtoul(val, NULL, 10);

    /* get current voltage */
    entry = strstr(buf, "present voltage:");
    val = get_value(entry);
    if (val[0] == 'u')
	info->present_voltage = -1;
    else
	info->present_voltage = strtoul(val, NULL, 10);

    return 1;
}

/*
 * 2003-7-1.
 * In order to make this code more convenient for things other than
 * just plain old wmacpi-ng I'm breaking the basic functionality
 * up into several chunks: collecting and collating info for a 
 * single battery, calculating the global info (such as rtime), and 
 * some stuff to provide a similar interface to now.
 */

void acquire_batt_info(int batt)
{
    float rcap, lfcap;
    battery *binfo;
    
    get_battery_info(batt);
    
    binfo = &batteries[batt];
    
    if (!binfo->present) {
	binfo->percentage = 0;
	binfo->valid = 0;
	binfo->charge_time = 0;
	apminfo->rtime = 0;
	return;
    }

    /* calculate the percentage remaining, using the values of 
     * remaining capacity and last full capacity, as outlined in
     * the ACPI spec v2.0a, section 3.9.3. */
    {
	rcap = (float)binfo->remaining_cap;
	lfcap = (float)binfo->last_full_cap;
	if (rcap <= 0)
	    rcap = 0;
	if (lfcap <= 0)
	    lfcap = 1;
	binfo->percentage = (int)((rcap/lfcap) * 100.0);
	eprint(0, "percent: %d\n", binfo->percentage);
    }

    /* set the battery's capacity state, based (at present) on some 
     * guesstimated values: more than 75% == HIGH, 25% to 75% MED, and
     * less than 25% is LOW. Less than apminfo->crit_level is CRIT. */
    if (binfo->percentage > 75) 
	binfo->state = HIGH;
    else if (binfo->percentage > 25)
	binfo->state = MED;
    else 
	/* we only go to critical state if the battery is reporting
	 * critical itself . . . */
	binfo->state = LOW;

    /* we need to /know/ that we've got a valid state for the 
     * apminfo->power value . . . .*/
    apminfo->power = get_power_status();

    if ((apminfo->power != AC) && !binfo->charging) {
	/* we're not on power, and not charging. So we might as well 
	 * check if we're at a critical battery level, and calculate
	 * other interesting stuff . . . */
	if (binfo->capacity_state == CRITICAL) {
	    eprint(1, "Received critical battery status");
	    apminfo->power = CRIT;
	}
    }
    
    if (binfo->charging) {
	/* calculate remaining time until the battery is charged.
	 * when charging, the battery state file reports the 
	 * current being used to charge the battery. We can use 
	 * this and the remaining capacity to work out how long
	 * until it reaches the last full capacity of the battery.
	 * XXX: make sure this is actually portable . . . */
	lfcap = (float)binfo->last_full_cap;
	rcap = (float)binfo->remaining_cap;
	
	binfo->charge_time = (int)(((lfcap - rcap)/binfo->present_rate) * 60.0);
    } else
	if (binfo->charge_time)
	    binfo->charge_time = 0;

    /* and finally, we tell anyone who wants to use this information
     * that it's now valid . . .*/
    binfo->valid = 1;
}
	
void acquire_all_batt_info(void)
{
    int i;
    
    for(i = 0; i < batt_count; i++)
	acquire_batt_info(i);
}

void acquire_global_info(void)
{
    int i;
    int rtime;
    float rcap = 0;
    float rate = 0;
    static float rate_samples[SAMPLES];
    static int j = 0;
    static int sample_count = 0;
    static int n = 0;

    /* calculate the time remaining, using the battery's remaining 
     * capacity and the reported burn rate (3.9.3). 
     * For added accuracy, we average the value over the last 
     * SAMPLES number of calls, or for anything less than this we
     * simply report the raw number. */
    {
	
	for (i = 0; i < batt_count; i++) {
	    if (batteries[i].present && batteries[i].valid) {
		rcap += (float)batteries[i].remaining_cap;
		rate += (float)batteries[i].present_rate;
	    }
	}
	rate_samples[j] = rate;
	j++, sample_count++;
	j = j % SAMPLES;
	
	/* for the first SAMPLES number of calls we calculate the
	 * average based on sample_count, then we use SAMPLES to
	 * calculate the rolling average. */

	/* when this fails, n should be equal to SAMPLES. */
	if (sample_count < SAMPLES)
	    n++;
	for (i = 0, rate = 0; i < n; i++)
	    rate += rate_samples[i];
	rate = rate/n;

	if ((rcap < 1) || (rate < 1)) {
	    rtime = 0;
	    goto out;
	}
	if (rate <= 0) 
	    rate = 1;
	/* time remaining in minutes */
	rtime = (int)((rcap/rate) * 60.0);
	if(rtime <= 0) 
	    rtime = 0;
    out:
	eprint(0, "time rem: %d\n", rtime);
	apminfo->rtime = rtime;
    }

    /* get the power status.
     * note that this is actually reported seperately from the
     * battery info, under /proc/acpi/ac_adapter/AC/state */
    apminfo->power = get_power_status();
}

void acquire_all_info(void)
{
    acquire_all_batt_info();
    acquire_global_info();
}
