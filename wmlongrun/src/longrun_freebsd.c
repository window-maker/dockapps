/*
 * longrun_freebsd.c - module to get LongRun(TM) status, for FreeBSD
 *
 * Copyright(C) 2001,2002 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#if defined(HAVE_STRING_H)
#include <string.h>
#elif defined(HAVE_STRINGS_H)
#include <strings.h>
#endif
#include <unistd.h>
#include <err.h>
#include <sys/sysctl.h>
#include <sys/file.h>
#include "longrun.h"
#include "common.h"

#define LONGRUN_MODE_MINFREQUENCY       0x00
#define LONGRUN_MODE_ECONOMY            0x01
#define LONGRUN_MODE_PERFORMANCE        0x02
#define LONGRUN_MODE_MAXFREQUENCY       0x03

void
longrun_init(char *dummy, char *dummy2)
{
    /* You don't need initialization under FreeBSD */
}

/*
 * percent:  performance level (0 to 100)
 * flags:    performance/economy/unknown
 * mhz:      frequency
 * volts:    voltage
 */
void
longrun_get_stat(int *percent, int *flags, int *mhz, int *voltz)
{
    u_int ret_val;
    size_t len;

    *percent = *mhz = *voltz = *flags = 0;

    /* level */
    len = sizeof(ret_val);
    if (sysctlbyname("hw.crusoe.percentage", &ret_val, &len, NULL, 0) == 0)
	*percent = (int) ret_val;

    /* frequency */
    if (sysctlbyname("hw.crusoe.frequency", &ret_val, &len, NULL, 0) == 0)
	*mhz = (int) ret_val;

    /* voltage */
    if (sysctlbyname("hw.crusoe.voltage", &ret_val, &len, NULL, 0) == 0)
	*voltz = (int) ret_val;

    /* flags */
    if (sysctlbyname("hw.crusoe.longrun", &ret_val, &len, NULL, 0) == 0) {
	switch (ret_val) {
	case LONGRUN_MODE_MINFREQUENCY:
	case LONGRUN_MODE_ECONOMY:
	    *flags = LONGRUN_FLAGS_ECONOMY;
	    break;
	case LONGRUN_MODE_PERFORMANCE:
	case LONGRUN_MODE_MAXFREQUENCY:
	    *flags = LONGRUN_FLAGS_PEFORMANCE;
	    break;
	default:
	    *flags = LONGRUN_FLAGS_UNKNOWN;
	}
    }
}
