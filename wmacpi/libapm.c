#include <stdio.h>
#include "wmacpi.h"

#ifdef APM
#ifdef PRO
extern char *state[];
#endif
extern APMInfo *apminfo;
extern int crit_level;

int power_init(void)
{
    FILE *apm;

    if (!(apm = fopen("/proc/apm", "r"))) {
	fprintf(stderr, "This system does not support APM\n");
	return 1;
    }
    fclose(apm);

    return 0;
}

void acquire_info(void)
{
    FILE *apm;
    char buf[256];
    char min[10];

    int ac_line, batt, percent, rtime;

#ifdef PRO
    /* testing */
    if (!(apm = fopen("apm", "r")))
	return;
#else
    if (!(apm = fopen("/proc/apm", "r")))
	return;
#endif

    fgets(buf, 255, apm);
    sscanf(buf, "%*s %*s %*s %x %x %*s %d%% %d %s",
	   &ac_line, &batt, &percent, &rtime, min);

    eprint(0, "%02x %02x, %03d%%, %d", ac_line, batt, percent, rtime);
    apminfo->percentage = percent;
    apminfo->rtime = rtime;

    switch (ac_line) {
    case 0:	/* on battery.  calculate status. handle charging under AC */
	switch (batt) {
	case 0:
	    apminfo->power = HIGH;
	    break;
	case 1:
	    apminfo->power = LOW;
	    break;
	case 2:
	    apminfo->power = CRIT;
	    break;
	}

	/* check user-defined critical alarm */
	if (apminfo->percentage <= apminfo->crit_level)
	    apminfo->power = CRIT;

	break;
    case 1:	/* on AC power.  Check if battery is being charged */
#ifdef RETARDED_APM
	/* this is incase your battery is "charging" all the fucking time,
	 * even when it's actually done charging */
	if ((batt == 3) && (percent != 100))
#else
	if (batt == 3)
#endif
	    apminfo->power = CHARGING;
	else
	    apminfo->power = POWER;
	break;
#ifdef STUPID_APM
	/* treatment for GAY apm bioses that show wrong time
	 * remaining when AC is plugged in */
	apminfo->rtime = 0;
#endif
    }
    fclose(apm);
    process_plugin_timer();

    eprint(1, "current state: %s (%d)", state[apminfo->power], apminfo->power);
}
#endif /* APM */
