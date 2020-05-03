#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>

#include "libacpi.h"

extern char *state[];

battery_t batteries[MAXBATT];
int verbosity;

#define PROC_DATA_SOURCE    0
#define SYSFS_DATA_SOURCE   1
static int data_source;

/* local proto */
int acpi_get_design_cap(int batt);

static int
cmpstr (const void *va, const void *vb)
{
    const char *a = *(const char **) va;
    const char *b = *(const char **) vb;

    return strcmp (a, b);
}

static int read_sysfs_file(char *node, char *prop, char *buf, size_t buflen)
{
    char tmp[256];
    FILE *fp;
    int ret;

    ret = snprintf(tmp, sizeof(tmp), "/sys/class/power_supply/%s/%s", node, prop);
    if (ret >= (int)sizeof(tmp)) {
	perr("Path too long for %s/%s\n", node, prop);
	return -1;
    }

    fp = fopen(tmp, "r");
    if (fp == NULL) {
	perr("Could not open %s/%s\n", node, prop);
	return -2;
    }

    ret = fread(buf, 1, buflen - 1, fp);

    fclose(fp);

    if (ret == 0) {
	perr("Could not read %s/%s\n", node, prop);
	return -3;
    }

    buf[ret] = '\0';

    return 0;
}

/* initialise the batteries */
static int sysfs_init_batteries(global_t *globals)
{
    DIR *battdir;
    struct dirent *batt;
    char *name;
    char *names[MAXBATT];
    char ps_type[16];
    int i;

    /* now enumerate batteries */
    globals->battery_count = 0;
    battdir = opendir("/sys/class/power_supply");
    if (battdir == NULL) {
	pfatal("No batteries or ACPI not supported\n");
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

	/* skip ., .. and dotfiles */
	if (name[0] == '.')
	    continue;

	if (read_sysfs_file(name, "type", ps_type, sizeof(ps_type)) < 0)
	    continue;

	if (strncmp("Battery", ps_type, 7) != 0)
	    continue;

	names[globals->battery_count] = strdup(name);
	globals->battery_count++;
    }
    closedir(battdir);

    qsort(names, globals->battery_count, sizeof *names, cmpstr);

    for (i = 0; i < globals->battery_count; i++) {
	snprintf(batteries[i].name, MAX_NAME, "%s", names[i]);
	pdebug("battery detected at /sys/class/power_supply/%s\n", batteries[i].name);
	pinfo("found battery %s\n", names[i]);
	free(names[i]);

	if (read_sysfs_file(batteries[i].name, "energy_now", ps_type, sizeof(ps_type)) == 0)
	    batteries[i].sysfs_capa_mode = SYSFS_CAPA_ENERGY;
	else if (read_sysfs_file(batteries[i].name, "charge_now", ps_type, sizeof(ps_type)) == 0)
	    batteries[i].sysfs_capa_mode = SYSFS_CAPA_CHARGE;
	else if (read_sysfs_file(batteries[i].name, "capacity", ps_type, sizeof(ps_type)) == 0) {
	    batteries[i].sysfs_capa_mode = SYSFS_CAPA_PERCENT;
	    batteries[i].design_cap = 100;
	    batteries[i].last_full_cap = 100;
	} else
	    batteries[i].sysfs_capa_mode = SYSFS_CAPA_ERR;
    }

    /* tell user some info */
    pdebug("%d batteries detected\n", globals->battery_count);
    pinfo("libacpi: found %d batter%s\n", globals->battery_count,
	  (globals->battery_count == 1) ? "y" : "ies");

    return 0;
}

/* initialise the batteries */
static int procfs_init_batteries(global_t *globals)
{
    DIR *battdir;
    struct dirent *batt;
    char *name;
    char *names[MAXBATT];
    int i;

    /* now enumerate batteries */
    globals->battery_count = 0;
    battdir = opendir("/proc/acpi/battery");
    if (battdir == NULL) {
	pfatal("No batteries or ACPI not supported\n");
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

	names[globals->battery_count] = strdup(name);
	globals->battery_count++;
    }
    closedir(battdir);

    qsort(names, globals->battery_count, sizeof *names, cmpstr);

    for (i = 0; i < globals->battery_count; i++) {
	snprintf(batteries[i].name, MAX_NAME, "%s", names[i]);
	snprintf(batteries[i].info_file, MAX_NAME,
		 "/proc/acpi/battery/%s/info", names[i]);
	snprintf(batteries[i].state_file, MAX_NAME,
		 "/proc/acpi/battery/%s/state", names[i]);
	pdebug("battery detected at %s\n", batteries[i].info_file);
	pinfo("found battery %s\n", names[i]);
	free(names[i]);
    }

    /* tell user some info */
    pdebug("%d batteries detected\n", globals->battery_count);
    pinfo("libacpi: found %d batter%s\n", globals->battery_count,
	    (globals->battery_count == 1) ? "y" : "ies");

    return 0;
}

int init_batteries(global_t *globals)
{
  if (data_source == SYSFS_DATA_SOURCE)
    return sysfs_init_batteries(globals);
  else
    return procfs_init_batteries(globals);
}

/* a stub that just calls the current function */
int reinit_batteries(global_t *globals)
{
    pdebug("reinitialising batteries\n");
    return init_batteries(globals);
}

/* the actual name of the subdirectory under power_supply may
 * be anything, so we need to read the directory and use the
 * name we find there. */
static int sysfs_init_ac_adapters(global_t *globals)
{
    DIR *acdir;
    struct dirent *adapter;
    adapter_t *ap = &globals->adapter;
    char *name;
    char ps_type[16];

    acdir = opendir("/sys/class/power_supply");
    if (acdir == NULL) {
	pfatal("Unable to open /sys/class/power_supply -"
		" are you sure this system supports ACPI?\n");
	return 1;
    }
    name = NULL;
    while ((adapter = readdir(acdir)) != NULL) {

	if (adapter->d_name[0] == '.') {
	  continue;
	}

	if (read_sysfs_file(adapter->d_name, "type", ps_type, sizeof(ps_type)) < 0) {
	  continue;
	}

	if (strncmp("Mains", ps_type, 5) == 0) {
	  pdebug("found adapter %s\n", adapter->d_name);
	  name = strdup(adapter->d_name);
	  break;
	}
    }
    closedir(acdir);

    if (name == NULL) {
      perr("No AC adapter found !\n");
      return 1;
    }

    /* we'll just use the first adapter we find ... */
    ap->name = name;
    pinfo("libacpi: found ac adapter %s\n", ap->name);

    return 0;
}

/* the actual name of the subdirectory under ac_adapter may
 * be anything, so we need to read the directory and use the
 * name we find there. */
static int procfs_init_ac_adapters(global_t *globals)
{
    DIR *acdir;
    struct dirent *adapter;
    adapter_t *ap = &globals->adapter;
    char *name;

    acdir = opendir("/proc/acpi/ac_adapter");
    if (acdir == NULL) {
	pfatal("Unable to open /proc/acpi/ac_adapter -"
		" are you sure this system supports ACPI?\n");
	return 1;
    }
    name = NULL;
    while ((adapter = readdir(acdir)) != NULL) {
	name = adapter->d_name;

	if (!strncmp(".", name, 1) || !strncmp("..", name, 2))
	    continue;
	pdebug("found adapter %s\n", name);
    }
    ap->name = strdup(name);
    closedir(acdir);
    /* we /should/ only see one filename other than . and .. so
     * we'll just use the last value name acquires . . . */
    snprintf(ap->state_file, MAX_NAME, "/proc/acpi/ac_adapter/%s/state",
	     ap->name);
    pinfo("libacpi: found ac adapter %s\n", ap->name);

    return 0;
}

int init_ac_adapters(global_t *globals)
{
  if (data_source == SYSFS_DATA_SOURCE)
    return sysfs_init_ac_adapters(globals);
  else
    return procfs_init_ac_adapters(globals);
}

/* stub that does nothing but call the normal init function */
int reinit_ac_adapters(global_t *globals)
{
    pdebug("reinitialising ac adapters\n");
    return init_ac_adapters(globals);
}

/* see if we have ACPI support and check version */
int power_init(global_t *globals)
{
    FILE *acpi;
    char buf[4096];
    int acpi_ver = 0;
    int retval;
    size_t buflen;
    unsigned int version_offset = 0;

    if (!(acpi = fopen("/sys/module/acpi/parameters/acpica_version", "r"))) {
	if (!(acpi = fopen("/proc/acpi/info", "r"))) {
	    pfatal("This system does not support ACPI\n");
	    return 1;
	} else {
	    version_offset = 25;
	}
    }

    /* okay, now see if we got the right version */
    buflen = fread(buf, 4096, 1, acpi);
    if (buflen != 4096 && ferror(acpi)) {
	    pfatal("Could not read file\n");
	    return 1;
    }
    acpi_ver = strtol(buf + version_offset, NULL, 10);
    pinfo("ACPI version detected: %d\n", acpi_ver);
    if (acpi_ver < 20020214) {
	pfatal("This version requires ACPI subsystem version 20020214\n");
	fclose(acpi);
	return 1;
    }
    /* yep, all good */
    fclose(acpi);

    /* determine data source */
    if (access("/sys/class/power_supply", R_OK | X_OK) == 0) {
	data_source = SYSFS_DATA_SOURCE;
	pinfo("Selecting sysfs as the data source\n");
    } else {
	data_source = PROC_DATA_SOURCE;
	pinfo("Selecting procfs as the data source\n");
    }

    if (!(retval = init_batteries(globals)))
	retval = init_ac_adapters(globals);

    return retval;
}

/* reinitialise everything, to deal with changing batteries or ac adapters */
int power_reinit(global_t *globals)
{
    FILE *acpi;
    int retval;

    if (!(acpi = fopen("/sys/module/acpi/parameters/acpica_version", "r"))) {
	if (!(acpi = fopen("/proc/acpi/info", "r"))) {
	    pfatal("Could not reopen ACPI info file - does this system support ACPI?\n");
	    return 1;
	}
    }

    if (!(retval = reinit_batteries(globals)))
	retval = reinit_ac_adapters(globals);

    fclose (acpi);
    return retval;
}

static char *get_value(char *string)
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

static int check_error(char *buf)
{
    if(strstr(buf, "ERROR") != NULL)
	return 1;
    return 0;
}

static power_state_t sysfs_get_power_status(global_t *globals)
{
    char online[2];
    adapter_t *ap = &globals->adapter;

    if (read_sysfs_file(ap->name, "online", online, sizeof(online)) < 0)
      return PS_ERR;

    if (*online == '1')
	return AC;
    else
	return BATT;
}

static power_state_t procfs_get_power_status(global_t *globals)
{
    FILE *file;
    char buf[1024];
    char *val;
    adapter_t *ap = &globals->adapter;

    if ((file = fopen(ap->state_file, "r")) == NULL) {
	snprintf(buf, 1024, "Could not open state file %s", ap->state_file);
	perror(buf);
	return PS_ERR;
    }

    if (!fgets(buf, 1024, file)) {
	    pfatal("Could not read file\n");
	    return PS_ERR;
    }
    fclose(file);
    val = get_value(buf);
    if ((strncmp(val, "on-line", 7)) == 0)
	return AC;
    else
	return BATT;
}

power_state_t get_power_status(global_t *globals)
{
    if (data_source == SYSFS_DATA_SOURCE)
	return sysfs_get_power_status(globals);
    else
	return procfs_get_power_status(globals);
}

static int sysfs_get_battery_info(global_t *globals, int batt_no)
{
    battery_t *info = &batteries[batt_no];
    char buf[32];
    int ret;

    /* check to see if battery is present */
    ret = read_sysfs_file(info->name, "present", buf, sizeof(buf));
    if (ret < 0) {
	/* interestingly, when the battery is not present, the whole
	 * /sys/class/power_supply/BATn directory does not exist.
	 * Yes, this is broken.
	 */
	if (ret == -2)
	    info->present = 0;

	/* reinit batteries, this one went away and it's very
	   possible there just isn't any other one */
	reinit_batteries(globals);

	return 0;
    }

    info->present = (*buf == '1');
    if (!info->present) {
	pinfo("Battery %s not present\n", info->name);
	return 0;
    }

    /* get design capacity
     * note that all these integer values can also contain the
     * string 'unknown', so we need to check for this. */
    if (info->sysfs_capa_mode == SYSFS_CAPA_ENERGY) {
	if (read_sysfs_file(info->name, "energy_full_design", buf, sizeof(buf)) < 0)
	    info->design_cap = -1;
	else
	    info->design_cap = strtoul(buf, NULL, 10) / 1000;

	/* get last full capacity */
	if (read_sysfs_file(info->name, "energy_full", buf, sizeof(buf)) < 0)
	    info->last_full_cap = -1;
	else
	    info->last_full_cap = strtoul(buf, NULL, 10) / 1000;
    } else if (info->sysfs_capa_mode == SYSFS_CAPA_CHARGE) {
	/* get design capacity */
	if (read_sysfs_file(info->name, "charge_full_design", buf, sizeof(buf)) < 0)
	    info->design_cap = -1;
	else
	    info->design_cap = strtoul(buf, NULL, 10) / 1000;

	/* get last full capacity */
	if (read_sysfs_file(info->name, "charge_full", buf, sizeof(buf)) < 0)
	    info->last_full_cap = -1;
	else
	    info->last_full_cap = strtoul(buf, NULL, 10) / 1000;
    } else if (info->sysfs_capa_mode != SYSFS_CAPA_PERCENT) {
	info->design_cap = -1;
	info->last_full_cap = -1;
    }


    /* get design voltage */
    if (read_sysfs_file(info->name, "voltage_min_design", buf, sizeof(buf)) < 0)
	info->design_voltage = -1;
    else
	info->design_voltage = strtoul(buf, NULL, 10) / 1000;

    /* get charging state */
    if (read_sysfs_file(info->name, "status", buf, sizeof(buf)) < 0) {
	info->charge_state = CH_ERR;
    } else {
	if (strncmp(buf, "Unknown", 7) == 0)
	    info->charge_state = CH_ERR;
	else if (strncmp(buf, "Discharging", 11) == 0)
	    info->charge_state = DISCHARGE;
	else if (strncmp(buf, "Charging", 8) == 0)
	    info->charge_state = CHARGE;
	else if (strncmp(buf, "Not charging", 12) == 0)
	    info->charge_state = NO_CHARGE;
	else if (strncmp(buf, "Full", 4) == 0)
	    info->charge_state = FULL; /* DISCHARGE ? as per old comment ... */
    }

    /* get current rate of burn
     * note that if it's on AC, this will report 0 */
    if (read_sysfs_file(info->name, "current_now", buf, sizeof(buf)) < 0)
	info->present_rate = -1;
    else {
	int rate;
	rate = strtoul(buf, NULL, 10) / 1000;
	info->present_rate = (rate != 0) ? rate : info->present_rate;
    }

    if (info->sysfs_capa_mode == SYSFS_CAPA_ENERGY) {
	/* get remaining capacity */
	if (read_sysfs_file(info->name, "energy_now", buf, sizeof(buf)) < 0)
	    info->remaining_cap = -1;
	else
	    info->remaining_cap = strtoul(buf, NULL, 10) / 1000;

    } else if (info->sysfs_capa_mode == SYSFS_CAPA_CHARGE) {
	/* get remaining capacity */
	if (read_sysfs_file(info->name, "charge_now", buf, sizeof(buf)) < 0)
	    info->remaining_cap = -1;
	else
	    info->remaining_cap = strtoul(buf, NULL, 10) / 1000;
    } else if (info->sysfs_capa_mode == SYSFS_CAPA_PERCENT) {
	/* get remaining capacity */
	if (read_sysfs_file(info->name, "capacity", buf, sizeof(buf)) < 0)
	    info->remaining_cap = -1;
	else
	    info->remaining_cap = strtoul(buf, NULL, 10) / 1000;
    } else {
	info->remaining_cap = -1;
    }

    /* get current voltage */
    if (read_sysfs_file(info->name, "voltage_now", buf, sizeof(buf)) < 0)
	info->present_voltage = -1;
    else
	info->present_voltage = strtoul(buf, NULL, 10) / 1000;

    return 1;
}

static int procfs_get_battery_info(global_t *globals, int batt_no)
{
    FILE *file;
    battery_t *info = &batteries[batt_no];
    char buf[1024];
    char *entry;
    int buflen;
    char *val;

    globals = globals; /* silencing a warning */

    if ((file = fopen(info->info_file, "r")) == NULL) {
	/* this is cheating, but string concatenation should work . . . */
	pfatal("Could not open %s:", info->info_file );
	perror(NULL);
	return 0;
    }

    /* grab the contents of the file */
    buflen = fread(buf, sizeof(buf), 1, file);
    fclose(file);

    /* check to see if there were any errors reported in the file */
    if(check_error(buf)) {
	pinfo("Error reported in file %s - discarding data\n",
	      info->info_file);
	return 0;
    }

    /* check to see if battery is present */
    entry = strstr(buf, "present:");
    val = get_value(entry);
    if ((strncmp(val, "yes", 3)) == 0) {
	info->present = 1;
    } else {
	pinfo("Battery %s not present\n", info->name);
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
	perr("Could not open %s:", info->state_file );
	perror(NULL);
	return 0;
    }

    /* grab the file contents */
    memset(buf, 0, sizeof(buf));
    buflen = fread(buf, sizeof(buf), 1, file);
    if (buflen != sizeof(buf) && ferror(file)) {
	    pfatal("Could not read file\n");
	    return 1;
    }
    fclose(file);

    /* check to see if there were any errors reported in the file */
    if(check_error(buf)) {
	pinfo("Error reported in file %s - discarding data\n",
	      info->state_file);
	return 0;
    }
    /* check to see if battery is present */
    entry = strstr(buf, "present:");
    val = get_value(entry);
    if ((strncmp(val, "yes", 3)) == 0) {
	info->present = 1;
    } else {
	info->present = 0;
	perr("Battery %s no longer present\n", info->name);
	return 0;
    }

    /* get charging state */
    entry = strstr(buf, "charging state:");
    val = get_value(entry);
    if (val[0] == 'u')
	info->charge_state = CH_ERR;
    else if ((strncmp(val, "discharging", 10)) == 0)
	info->charge_state = DISCHARGE;
    else if ((strncmp(val, "charged", 7)) == 0)
	/* this is a workaround for machines that report
	 * their charge state as 'charged', rather than
	 * what my laptop does, which is go straight to
	 * 'discharging'. dunno which matches the standard */
	info->charge_state = DISCHARGE;
    else
	info->charge_state = CHARGE;

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

int get_battery_info(global_t *globals, int batt_no)
{
  if (data_source == SYSFS_DATA_SOURCE)
      return sysfs_get_battery_info(globals, batt_no);
  else
      return procfs_get_battery_info(globals, batt_no);
}

/*
 * 2003-7-1.
 * In order to make this code more convenient for things other than
 * just plain old wmacpi-ng I'm breaking the basic functionality
 * up into several chunks: collecting and collating info for a
 * single battery, calculating the global info (such as rtime), and
 * some stuff to provide a similar interface to now.
 */

/* calculate the percentage remaining, using the values of
 * remaining capacity and last full capacity, as outlined in
 * the ACPI spec v2.0a, section 3.9.3. */
static int calc_remaining_percentage(int batt)
{
    float rcap, lfcap;
    battery_t *binfo;
    int retval;

    binfo = &batteries[batt];

    rcap = (float)binfo->remaining_cap;
    lfcap = (float)binfo->last_full_cap;

    /* we use -1 to indicate that the value is unknown . . . */
    if (rcap < 0) {
	perr("unknown percentage value\n");
	retval = -1;
    } else {
	if (lfcap <= 0)
	    lfcap = 1;
	retval = (int)((rcap/lfcap) * 100.0);
	pdebug("percent: %d\n", retval);
    }
    return retval;
}

/* check to see if we've been getting bad data from the batteries - if
 * we get more than some limit we switch to using the remaining capacity
 * for the calculations. */
static enum rtime_mode check_rt_mode(global_t *globals)
{
    int i;
    int bad_limit = 5;
    battery_t *binfo;

    /* if we were told what to do, we should keep doing it */
    if(globals->rt_forced)
	return globals->rt_mode;

    for(i = 0; i < MAXBATT; i++) {
	binfo = &batteries[i];
	if(binfo->present && globals->adapter.power == BATT) {
	    if(binfo->present_rate <= 0) {
		pdebug("Bad report from %s\n", binfo->name);
		binfo->bad_count++;
	    }
	}
    }
    for(i = 0; i < MAXBATT; i++) {
	binfo = &batteries[i];
	if(binfo->bad_count > bad_limit) {
	    if(globals->rt_mode != RT_CAP)
		pinfo("More than %d bad reports from %s; "
		      "Switching to remaining capacity mode\n",
		      bad_limit, binfo->name);
	    return RT_CAP;
	}
    }
    return RT_RATE;
}

/* calculate remaining time until the battery is charged.
 * when charging, the battery state file reports the
 * current being used to charge the battery. We can use
 * this and the remaining capacity to work out how long
 * until it reaches the last full capacity of the battery.
 * XXX: make sure this is actually portable . . . */
static int calc_charge_time_rate(int batt)
{
    float rcap, lfcap;
    battery_t *binfo;
    int charge_time = 0;

    binfo = &batteries[batt];

    if (binfo->charge_state == CHARGE) {
	if (binfo->present_rate == -1) {
	    perr("unknown present rate\n");
	    charge_time = -1;
	} else {
	    lfcap = (float)binfo->last_full_cap;
	    rcap = (float)binfo->remaining_cap;

	    charge_time = (int)(((lfcap - rcap)/binfo->present_rate) * 60.0);
	}
    } else
	if (binfo->charge_time)
	    charge_time = 0;
    return charge_time;
}

/* we need to calculate the present rate the same way we do in rt_cap
 * mode, and then use that to estimate charge time. This will
 * necessarily be even less accurate than it is for remaining time, but
 * it's just as neessary . . . */
static int calc_charge_time_cap(int batt)
{
    static float cap_samples[CAP_SAMPLES];
    static int time_samples[CAP_SAMPLES];
    static int sample_count = 0;
    static int current = 0;
    static int old = 1;
    int rtime;
    int tdiff;
    float cdiff;
    float current_rate;
    battery_t *binfo = &batteries[batt];

    cap_samples[current] = (float) binfo->remaining_cap;
    time_samples[current] = time(NULL);

    if (sample_count == 0) {
	/* we can't do much if we don't have any data . . . */
	current_rate = 0;
    } else if (sample_count < CAP_SAMPLES) {
	/* if we have less than SAMPLES samples so far, we use the first
	 * sample and the current one */
	cdiff = cap_samples[current] - cap_samples[0];
	tdiff = time_samples[current] - time_samples[0];
	current_rate = cdiff/tdiff;
    } else {
	/* if we have more than SAMPLES samples, we use the oldest
	 * current one, which at this point is current + 1. This will
	 * wrap the same way that current will wrap, but one cycle
	 * ahead */
	cdiff = cap_samples[current] - cap_samples[old];
	tdiff = time_samples[current] - time_samples[old];
	current_rate = cdiff/(float)tdiff;
    }
    if (current_rate == 0)
	rtime = 0;
    else {
	float cap_left = (float)(binfo->last_full_cap - binfo->remaining_cap);
	rtime = (int)(cap_left/(current_rate * 60.0));
    }
    sample_count++, current++, old++;
    if (current >= CAP_SAMPLES)
	current = 0;
    if (old >= CAP_SAMPLES)
	old = 0;

    pdebug("cap charge time rem: %d\n", rtime);
    return rtime;
}

static int calc_charge_time(global_t *globals, int batt)
{
    int ctime = 0;

    globals->rt_mode = check_rt_mode(globals);

    switch(globals->rt_mode) {
    case RT_RATE:
	ctime = calc_charge_time_rate(batt);
	break;
    case RT_CAP:
	ctime = calc_charge_time_cap(batt);
	break;
    }
    return ctime;
}

void acquire_batt_info(global_t *globals, int batt)
{
    battery_t *binfo;
    adapter_t *ap = &globals->adapter;

    get_battery_info(globals, batt);

    binfo = &batteries[batt];

    if (!binfo->present) {
	binfo->percentage = 0;
	binfo->valid = 0;
	binfo->charge_time = 0;
	globals->rtime = 0;
	return;
    }

    binfo->percentage = calc_remaining_percentage(batt);

    /* set the battery's capacity state, based (at present) on some
     * guesstimated values: more than 75% == HIGH, 25% to 75% MED, and
     * less than 25% is LOW. Less than globals->crit_level is CRIT. */
    if (binfo->percentage == -1)
	binfo->state = BS_ERR;
    if (binfo->percentage < globals->crit_level)
	binfo->state = CRIT;
    else if (binfo->percentage > 75)
	binfo->state = HIGH;
    else if (binfo->percentage > 25)
	binfo->state = MED;
    else
	binfo->state = LOW;

    /* we need to /know/ that we've got a valid state for the
     * globals->power value . . . .*/
    ap->power = get_power_status(globals);

    binfo->charge_time = calc_charge_time(globals, batt);

    /* and finally, we tell anyone who wants to use this information
     * that it's now valid . . .*/
    binfo->valid = 1;
}

void acquire_all_batt_info(global_t *globals)
{
    int i;

    for(i = 0; i < globals->battery_count; i++)
	acquire_batt_info(globals, i);
}

/*
 * One of the feature requests I've had is for some way to deal with
 * batteries that are too dumb or too b0rken to report a present rate
 * value. The way to do this, obviously, is to record the time that
 * samples were taken and use that information to calculate the rate
 * at which the battery is draining/charging. This still won't help
 * systems where the battery doesn't even report the remaining
 * capacity, but without the present rate or the remaining capacity, I
 * don't think there's /anything/ we can do to work around it.
 *
 * So, what we need to do is provide a way to use a different method
 * to calculate the time remaining. What seems most sensible is to
 * split out the code to calculate it into a seperate function, and
 * then provide multiple implementations . . .
 */

/*
 * the default implementation - if present rate and remaining capacity
 * are both reported correctly, we use them.
 */
int calc_time_remaining_rate(global_t *globals)
{
    int i;
    int rtime;
    float rcap = 0;
    float rate = 0;
    battery_t *binfo;
    static float rate_samples[SAMPLES];
    static int sample_count = 0;
    static int j = 0;
    static int n = 0;

    /* calculate the time remaining, using the battery's remaining
     * capacity and the reported burn rate (3.9.3).
     * For added accuracy, we average the value over the last
     * SAMPLES number of calls, or for anything less than this we
     * simply report the raw number. */
    /* XXX: this needs to correctly handle the case where
     * any of the values used is unknown (which we flag using
     * -1). */
    for (i = 0; i < globals->battery_count; i++) {
	binfo = &batteries[i];
	if (binfo->present && binfo->valid) {
	    rcap += (float)binfo->remaining_cap;
	    rate += (float)binfo->present_rate;
	}
    }
    rate_samples[j] = rate;
    j++, sample_count++;
    if (j >= SAMPLES)
	j = 0;

    /* for the first SAMPLES number of calls we calculate the
     * average based on sample_count, then we use SAMPLES to
     * calculate the rolling average. */

    /* when this fails, n should be equal to SAMPLES. */
    if (sample_count < SAMPLES)
	n++;
    for (i = 0, rate = 0; i < n; i++) {
	/* if any of our samples are invalid, we drop
	 * straight out, and flag our unknown values. */
	if (rate_samples[i] < 0) {
	    rate = -1;
	    rtime = -1;
	    goto out;
	}
	rate += rate_samples[i];
    }
    rate = rate/(float)n;

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
    pdebug("discharge time rem: %d\n", rtime);
    return rtime;
}

/*
 * the alternative implementation - record the time at which each
 * sample was taken, and then use the difference between the latest
 * sample and the one SAMPLES ago to calculate the difference over
 * that time, and from there the rate of change of capacity.
 *
 * XXX: this code sucks, but largely because batteries aren't exactly
 * precision instruments - mine only report with about 70mAH
 * resolution, so they don't report any changes until the difference
 * is 70mAH. This means that calculating the current rate from the
 * remaining capacity is very choppy . . .
 *
 * To fix this, we should calculate an average over some number of
 * samples at the old end of the set - this would smooth out the
 * transitions.
 */
int calc_time_remaining_cap(global_t *globals)
{
    static float cap_samples[CAP_SAMPLES];
    static int time_samples[CAP_SAMPLES];
    static int sample_count = 0;
    static int current = 0;
    static int old = 1;
    battery_t *binfo;
    int i;
    int rtime;
    int tdiff;
    float cdiff;
    float cap = 0;
    float current_rate;

    for (i = 0; i < globals->battery_count; i++) {
	binfo = &batteries[i];
	if (binfo->present && binfo->valid)
	    cap += binfo->remaining_cap;
    }
    cap_samples[current] = cap;
    time_samples[current] = time(NULL);

    if (sample_count == 0) {
	/* we can't do much if we don't have any data . . . */
	current_rate = 0;
    } else if (sample_count < CAP_SAMPLES) {
	/* if we have less than SAMPLES samples so far, we use the first
	 * sample and the current one */
	cdiff = cap_samples[0] - cap_samples[current];
	tdiff = time_samples[current] - time_samples[0];
	current_rate = cdiff/tdiff;
    } else {
	/* if we have more than SAMPLES samples, we use the oldest
	 * current one, which at this point is current + 1. This will
	 * wrap the same way that current will wrap, but one cycle
	 * ahead */
	cdiff = cap_samples[old] - cap_samples[current];
	tdiff = time_samples[current] - time_samples[old];
	current_rate = cdiff/tdiff;
    }
    if (current_rate == 0)
	rtime = 0;
    else
	rtime = (int)(cap_samples[current]/(current_rate * 60.0));

    sample_count++, current++, old++;
    if (current >= CAP_SAMPLES)
	current = 0;
    if (old >= CAP_SAMPLES)
	old = 0;

    pdebug("cap discharge time rem: %d\n", rtime);
    return rtime;
}

void acquire_global_info(global_t *globals)
{
    adapter_t *ap = &globals->adapter;

    globals->rt_mode = check_rt_mode(globals);

    switch(globals->rt_mode) {
    case RT_RATE:
	globals->rtime = calc_time_remaining_rate(globals);
	break;
    case RT_CAP:
	globals->rtime = calc_time_remaining_cap(globals);
	break;
    }

    /* get the power status.
     * note that this is actually reported seperately from the
     * battery info, under /proc/acpi/ac_adapter/AC/state */
    ap->power = get_power_status(globals);
}

void acquire_all_info(global_t *globals)
{
    acquire_all_batt_info(globals);
    acquire_global_info(globals);
}
