/* Not particularly good interface to hal, for programs that used to use
 * apm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <upower.h>
#include "apm.h"

static UpClient * up;

struct context {
	int current;
	int needed;
	guint state;
	int percentage;
	gboolean ac;
	int time;
};

static void get_devinfo(gpointer device, gpointer result)
{
	gboolean online;
	gdouble percentage;
	guint state;
	guint kind;
	gint64 time_to_empty;
	gint64 time_to_full;
	struct context *ctx = result;

	g_object_get(G_OBJECT(device), "percentage", &percentage,
		"online", &online,
		"state", &state,
		"kind", &kind,
		"time-to-empty", &time_to_empty,
		"time-to-full", &time_to_full,
		NULL);
	if (kind == UP_DEVICE_KIND_BATTERY) {
		if (ctx->current == ctx->needed) {
			ctx->percentage = (int)percentage;
			ctx->state = state;
			if (time_to_empty)
				ctx->time = time_to_empty;
			else
				ctx->time = time_to_full;
		}
		ctx->current++;
	} else if (kind == UP_DEVICE_KIND_LINE_POWER) {
		ctx->ac |= online;
	}
}

int upower_supported(void)
{
	up = up_client_new();

	if (!up) {
		return 0;
	} else {
		GPtrArray *devices = up_client_get_devices(up);

		if (!devices) {
			return 0;
		} else {
			g_ptr_array_unref(devices);
			return 1;
		}
	}
}

/* Fill the passed apm_info struct. */
int upower_read(int battery, apm_info *info)
{
	GPtrArray *devices = NULL;

	up = up_client_new();

	if (!up)
		return -1;

	#if !UP_CHECK_VERSION(0, 9, 99)
	/* Allow a battery that was not present before to appear. */
	up_client_enumerate_devices_sync(up, NULL, NULL);
	#endif

	devices = up_client_get_devices(up);

	if (!devices)
		return -1;

	info->battery_flags = 0;
	info->using_minutes = 0;

	struct context ctx = {
		.current = 0,
		.needed = battery - 1,
		.state = UP_DEVICE_STATE_UNKNOWN,
		.percentage = -1,
		.ac = FALSE,
		.time = -1
	};

	g_ptr_array_foreach(devices, &get_devinfo, &ctx);

	info->ac_line_status = ctx.ac;

	/* remaining_time and charge_level.percentage are not a mandatory
	 * keys, so if not present, -1 will be returned */
	info->battery_time = ctx.time;
	info->battery_percentage = ctx.percentage;
	if (ctx.state == UP_DEVICE_STATE_DISCHARGING) {
		info->battery_status = BATTERY_STATUS_CHARGING;
		/* charge_level.warning and charge_level.low are not
		 * required to be available; this is good enough */
		if (info->battery_percentage < 1)
			info->battery_status = BATTERY_STATUS_CRITICAL;
		else if (info->battery_percentage < 10)
			info->battery_status = BATTERY_STATUS_LOW;
	} else if (info->ac_line_status && ctx.state == UP_DEVICE_STATE_CHARGING) {
		info->battery_status = BATTERY_STATUS_CHARGING;
		info->battery_flags = info->battery_flags | BATTERY_FLAGS_CHARGING;
	} else if (info->ac_line_status) {
		/* Must be fully charged. */
		info->battery_status = BATTERY_STATUS_HIGH;
	} else {
		fprintf(stderr, "unknown battery state\n");
	}

	if (ctx.percentage < 0) {
		info->battery_percentage = 0;
		info->battery_time = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
	}

	g_ptr_array_free(devices, TRUE);
	return 0;
}
