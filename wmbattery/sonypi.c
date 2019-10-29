#include <stdio.h>
#ifdef __sun
#include <sys/filio.h>
#include <sys/sockio.h>
#endif
#include <sys/ioctl.h>
#include "apm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

#include "sonypi.h"

signed int spicfd = -1;

int sonypi_supported(void)
{
	spicfd = open("/dev/sonypi", O_RDWR);
	if (spicfd == -1)
		return 0;
	else
		return 1;
}

inline int sonypi_ioctl(int ioctlno, void *param)
{
	if (ioctl(spicfd, ioctlno, param) < 0)
		return 0;
	else
		return 1;
}

/* Read battery info from sonypi device and shove it into an apm_info
 * struct. */
int sonypi_read(apm_info *info)
{
	uint8_t batflags;
	uint16_t cap, rem;
	int havebatt = 0;

	info->using_minutes = info->battery_flags = 0;

	if (!sonypi_ioctl(SONYPI_IOCGBATFLAGS, &batflags))
		return 1;

	info->ac_line_status = (batflags & SONYPI_BFLAGS_AC) != 0;
	if (batflags & SONYPI_BFLAGS_B1) {
		if (!sonypi_ioctl(SONYPI_IOCGBAT1CAP, &cap))
			return 1;
		if (!sonypi_ioctl(SONYPI_IOCGBAT1REM, &rem))
			return 1;
		havebatt = 1;
	} else if (batflags & SONYPI_BFLAGS_B2) {
		/* Not quite right, if there is a second battery I should
		 * probably merge the two somehow.. */
		if (!sonypi_ioctl(SONYPI_IOCGBAT2CAP, &cap))
			return 1;
		if (!sonypi_ioctl(SONYPI_IOCGBAT2REM, &rem))
			return 1;
		havebatt = 1;
	} else {
		info->battery_percentage = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
	}

	if (havebatt) {
		info->battery_percentage = 100 * rem / cap;
		/* Guess at whether the battery is charging. */
		if (info->battery_percentage < 99 && info->ac_line_status == 1) {
			info->battery_flags = info->battery_flags | BATTERY_FLAGS_CHARGING;
			info->battery_status = BATTERY_STATUS_CHARGING;
		}
	}

	/* Sadly, there is no way to estimate this. */
	info->battery_time = 0;

	return 0;
}
