#include "config.h"

#ifdef HAVE_MACHINE_APM_BIOS_H /* for FreeBSD */
#include <machine/apm_bios.h>
#endif

#ifdef HAVE_I386_APMVAR_H /* for NetBSD and OpenBSD */
#include <i386/apmvar.h>
#endif

#ifdef HAVE_APM_H
#include <apm.h>
#endif

/* Symbolic constants for apm may be in system apm.h, or may not. */
#ifndef AC_LINE_STATUS_ON
#define AC_LINE_STATUS_OFF      (0)
#define AC_LINE_STATUS_ON       (1)
#define AC_LINE_STATUS_BACKUP   (2)
#define AC_LINE_STATUS_UNKNOWN  (0xff)

#define BATTERY_STATUS_HIGH     (0)
#define BATTERY_STATUS_LOW      (1)
#define BATTERY_STATUS_CRITICAL (2)
#define BATTERY_STATUS_CHARGING (3)
#define BATTERY_STATUS_ABSENT   (4)
#define BATTERY_STATUS_UNKNOWN  (0xff)

#define BATTERY_FLAGS_HIGH      (0x1)
#define BATTERY_FLAGS_LOW       (0x2)
#define BATTERY_FLAGS_CRITICAL  (0x4)
#define BATTERY_FLAGS_CHARGING  (0x8)
#define BATTERY_FLAGS_ABSENT    (0x80)

#define BATTERY_PERCENTAGE_UNKNOWN  (-1)

#define BATTERY_TIME_UNKNOWN        (-1)
#endif /* AC_LINE_STATUS_ON */

#if !defined(HAVE_APM_H)
typedef struct {
	const char driver_version[10];
	int apm_version_major;
	int apm_version_minor;
	int apm_flags;
	int ac_line_status;
	int battery_status;
	int battery_flags;
	int battery_percentage;
	int battery_time;
	int using_minutes;
} apm_info;

int apm_read(apm_info *i);
int apm_exists(void);
#endif
