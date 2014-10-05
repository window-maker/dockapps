#include <apm.h>

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

