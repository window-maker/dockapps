#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include "wmacpi.h"

#define MAXBATT 8

#ifdef ACPI
#ifdef PRO
extern char *state[];
#endif
extern APMInfo *apminfo;
static char batteries[MAXBATT][128];
static char battinfo[MAXBATT][128];
int batt_count;
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
    char *name;
    int acpi_ver = 0;

    if (!(acpi = fopen("/proc/acpi/info", "r"))) {
	fprintf(stderr, "This system does not support ACPI\n");
	return 1;
    }

    /* okay, now see if we got the right version */
    fread(buf, 4096, 1, acpi);
    acpi_ver = strtol(buf + 25, NULL, 10);
    eprint(1, "ACPI version detected: %d\n", acpi_ver);
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
	name = batt->d_name;
	
	/* skip . and .. */
	if (!strncmp(".", name, 1) || !strncmp("..", name, 2))
	    continue;

	sprintf(batteries[batt_count], "/proc/acpi/battery/%s/state", name);
	sprintf(battinfo[batt_count], "/proc/acpi/battery/%s/info", name);
	eprint(1, "battery detected at %s\n", batteries[batt_count]);
	batt_count++;
    }
    closedir(battdir);

    /* tell user some info */
    eprint(1, "%d batteries detected\n", batt_count);
    fprintf(stderr, "wmacpi: found %d batter%s\n", batt_count,
	    (batt_count == 1) ? "y" : "ies");
    
    return 0;
}

int acpi_get_design_cap(int battery)
{
    FILE *acpi;
    char *ptr;
    int design_cap;

    if (battery > MAXBATT)
	return -1;

    if (!(acpi = fopen(battinfo[battery], "r")))
	return -1;
    
    fread(buf, 512, 1, acpi);
    fclose(acpi);

    if ((ptr = strstr(buf, "last full capacity"))) {
	ptr += 25;
	sscanf(ptr, "%d", &design_cap);
	eprint(1, "last full capacity: %d\n", design_cap);
    } else {
	/* hack.  if there isnt any info on last capacity, we are
	 * screwed, but let's not come back here again */
	design_cap = -1;
	eprint(1, "Cannot retrieve design capacity!");
    }
    
    return design_cap;
}

void acquire_info(void)
{
    FILE *acpi;
    char *ptr;
    char stat;

    static int dcap = 0xdeadbeef;

    int percent = 100;		/* battery percentage */
    int ptemp, rate, rtime = 0;

    if (dcap == 0xdeadbeef) {
	/* get from first battery for now */
	dcap = acpi_get_design_cap(0);
    }

    if (!(acpi = fopen(batteries[0], "r")))
	return;

    eprint(1, "opened acpi file successfully");
    fread(buf, 512, 1, acpi);
    fclose(acpi);

    /* This section of the code will calculate "percentage remaining"
     * using battery capacity, and the following formula (acpi spec 3.9.2):
     * percentage = (current_capacity / last_full_capacity) * 100; */
    if ((ptr = strstr(buf, "remaining capacity"))) {
	ptr += 25;
	sscanf(ptr, "%d", &ptemp);
	eprint(1, "capacity: %d\n", ptemp);
	percent = (float)((float)ptemp / (float)dcap) * 100;
	eprint(1, "percent: %d\n", percent);
    }
    apminfo->percentage = percent;

    /* this section of code will calculate "time remaining"
     * using battery remaining capacity, and battery "rate" (3.9.3) */
    if ((ptr = strstr(buf, "present rate"))) {
	ptr += 25;
	sscanf(ptr, "%d", &rate);
	eprint(1, "rate: %d\n", rate);
	if (rate <= 0)
	    rate = 0;
	/* time remaining in minutes */
	rtime = ((float)((float)ptemp / (float)rate)) * 60;
	if (rtime <= 0)
	    rtime = 0;
	eprint(1, "time rem: %d\n", rtime);
    }
    apminfo->rtime = rtime;

    if ((ptr = strstr(buf, "charging state"))) {
	/* found battery discharging.  This is used to determine if
	 * we are on AC power or not. Notice check for "ch" later on */
	stat = *(ptr + 25);
	if (stat == 'o' || stat == 'u')	/* "ok" | "unknown" : charged, on ac power */		
	    apminfo->power = POWER;
	else
	    /* we set this, and later on use percentage
	     * value to determine high/med/low */
	    apminfo->power = HIGH;

	/* but if we are on power, we might be charging too.  Check. */
	if ((ptr = strstr(buf, "charging state"))) {
	    /* found battery charging line.  We will change power state
	     * if we are on power, and charging. */
	    stat = *(ptr + 25);
	    /* this is seriously stupid - but we catch "critical" */
	    if (stat == 'c' && (*(ptr + 26) == 'h'))
		apminfo->power = CHARGING;
	}
    }

    /* we are not on power, and not charging.  So, it would make sense
     * to check if battery is "critical low", and calculate interesting
     * things like battery HIGH/LOW, and maybe battery usage LOAD
     * This will be replaced with some code to allow setting user-specified
     * low / critical alarms */
    if ((apminfo->power != POWER) && (apminfo->power != CHARGING)) {
	eprint(1, "entering battery status check");
	if ((ptr = strstr(buf, "capacity state"))) {
	    stat = *(ptr + 25);
	    /* only check "c" here because we already caught "CHarging" earlier
	     * and also look into crit_level */
	    if (stat == 'c' || (apminfo->percentage <= apminfo->crit_level)) {
		/* nothing else to do here - critical battery. get out */
		eprint(1, "Received critical battery status");
		apminfo->power = CRIT;
	    }
	}
    }
    process_plugin_timer();

    eprint(1, "current state: %s (%d)", state[apminfo->power], apminfo->power);
}
#endif /* ACPI */
