#include <stdio.h>
#include <stdlib.h>

#include "lib_utils.h"
#include "power_management.h"

#define PROC_FILE "/proc/i8k"

/* we check for /proc/i18k */
int machine_is_dell(void)
{
	FILE *fp = fopen(PROC_FILE, "r");

	if (fp)
	{
		fclose(fp);
		return 1;
	}

	return 0;
}

int dell_get_fan_status(void)
{
	FILE *fp = fopen(PROC_FILE, "r");
	int fan_status   = 0;
	int fan_2_status = 0;

	if (!fp) return PM_Error;

	if (fscanf(fp, "%*s%*s%*s%*s%d%d", &fan_status, &fan_2_status) == 2)
	{
		fclose(fp);
		return (fan_status + fan_2_status);
	}

	fclose(fp);
	return PM_Error;
}

int dell_get_temperature(void)
{
	FILE *fp = fopen(PROC_FILE, "r");
	int result;	

	if (!fp) return PM_Error;

	if (fscanf(fp, "%*s%*s%*s%d", &result) == 1)
	{
		fclose(fp);
		return result;
	}
	fclose(fp);

	return PM_Error;
}
