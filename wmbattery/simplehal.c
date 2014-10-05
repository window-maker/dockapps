/* Not particularly good interface to hal, for programs that used to use
 * apm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libhal.h>
#include "apm.h"

static DBusConnection *dbus_ctx = NULL;
static LibHalContext *hal_ctx = NULL;

int num_ac_adapters = 0;
int num_batteries = 0;
char **ac_adapters = NULL;
char **batteries = NULL;

int connect_hal (void) {
	DBusError error;

	dbus_error_init(&error);
	dbus_ctx = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);
	if (dbus_ctx == NULL) {
		fprintf(stderr, "error: dbus_bus_get: %s: %s\n",
			 error.name, error.message);
		LIBHAL_FREE_DBUS_ERROR(&error);
		return 0;
	}
	if ((hal_ctx = libhal_ctx_new()) == NULL) {
		fprintf(stderr, "error: libhal_ctx_new\n");
		LIBHAL_FREE_DBUS_ERROR(&error);
		return 0;
	}
	if (!libhal_ctx_set_dbus_connection(hal_ctx, dbus_ctx)) {
		fprintf(stderr, "error: libhal_ctx_set_dbus_connection: %s: %s\n",
			 error.name, error.message);
		LIBHAL_FREE_DBUS_ERROR(&error);
		return 0;
	}
	if (!libhal_ctx_init(hal_ctx, &error)) {
		if (dbus_error_is_set(&error)) {
			fprintf(stderr, "error: libhal_ctx_init: %s: %s\n", error.name, error.message);
			LIBHAL_FREE_DBUS_ERROR(&error);
		}
		fprintf(stderr, "Could not initialise connection to hald.\n"
				 "Normally this means the HAL daemon (hald) is not running or not ready.\n");
		return 0;
	}

	return 1;
}

int hal_ready (void) {
	if (hal_ctx && dbus_connection_get_is_connected(dbus_ctx)) {
		return 1;
	}
	else {
		/* The messy business of reconnecting.
		 * dbus's design is crap when it comes to reconnecting.
		 * If dbus is down, can't actually close the connection to hal,
		 * since libhal wants to use dbus to do it. */
		if (dbus_ctx) {
			dbus_connection_close(dbus_ctx);
			dbus_connection_unref(dbus_ctx);
		}
		dbus_ctx = NULL;
		hal_ctx = NULL;

		return connect_hal();
	}
}

signed int get_hal_int (const char *udi, const char *key, int optional) {
	int ret;
	DBusError error;

	if (! hal_ready()) {
		return -1;
	}

	dbus_error_init(&error);

	ret = libhal_device_get_property_int (hal_ctx, udi, key, &error);

	if (! dbus_error_is_set (&error)) {
		return ret;
	}
	else {
		if (! optional) {
			fprintf(stderr, "error: libhal_device_get_property_int: %s: %s\n",
				 error.name, error.message);
		}
		dbus_error_free (&error);
		return -1;
	}
}

signed int get_hal_bool (const char *udi, const char *key, int optional) {
	int ret;
	DBusError error;

	if (! hal_ready()) {
		return -1;
	}

	dbus_error_init(&error);

	ret = libhal_device_get_property_bool (hal_ctx, udi, key, &error);

	if (! dbus_error_is_set (&error)) {
		return ret;
	}
	else {
		if (! optional) {
			fprintf(stderr, "error: libhal_device_get_property_bool: %s: %s\n",
				 error.name, error.message);
		}
		dbus_error_free (&error);
		return -1;
	}
}

void find_devices (void) {
	DBusError error;

	dbus_error_init(&error);

	if (ac_adapters)
		libhal_free_string_array(ac_adapters);
	ac_adapters = libhal_find_device_by_capability(hal_ctx, "ac_adapter",
		&num_ac_adapters, &error);
	if (dbus_error_is_set (&error)) {
		fprintf (stderr, "error: %s: %s\n", error.name, error.message);
		LIBHAL_FREE_DBUS_ERROR (&error);
	}

	if (batteries)
		libhal_free_string_array(batteries);
	batteries = libhal_find_device_by_capability(hal_ctx, "battery",
		&num_batteries, &error);
	if (dbus_error_is_set (&error)) {
		fprintf (stderr, "error: %s: %s\n", error.name, error.message);
		LIBHAL_FREE_DBUS_ERROR (&error);
	}
}

int simplehal_supported (void) {
	if (! connect_hal()) {
		return 0;
	}
	else {
		find_devices();
		return 1;
	}
}

/* Fill the passed apm_info struct. */
int simplehal_read (int battery, apm_info *info) {
	char *device;
	int i;

	/* Allow a battery that was not present before to appear. */
	if (battery > num_batteries) {
		find_devices();
	}

	info->battery_flags = 0;
	info->using_minutes = 0;

	info->ac_line_status=0;
	for (i = 0 ; i < num_ac_adapters && ! info->ac_line_status ; i++) {
		info->ac_line_status = (get_hal_bool(ac_adapters[i], "ac_adapter.present", 0) == 1);
	}

	if (battery > num_batteries) {
		info->battery_percentage = 0;
		info->battery_time = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
		return 0;
	}
	else {
		device=batteries[battery-1];
	}

	if (get_hal_bool(device, "battery.present", 0) != 1) {
		info->battery_percentage = 0;
		info->battery_time = 0;
		info->battery_status = BATTERY_STATUS_ABSENT;
		return 0;
	}

	/* remaining_time and charge_level.percentage are not a mandatory
	 * keys, so if not present, -1 will be returned */
	info->battery_time = get_hal_int(device, "battery.remaining_time", 1);
	info->battery_percentage = get_hal_int(device, "battery.charge_level.percentage", 1);
	if (get_hal_bool(device, "battery.rechargeable.is_discharging", 0) == 1) {
		info->battery_status = BATTERY_STATUS_CHARGING;
		/* charge_level.warning and charge_level.low are not
		 * required to be available; this is good enough */
		if (info->battery_percentage < 1) {
			info->battery_status = BATTERY_STATUS_CRITICAL;
		}
		else if (info->battery_percentage < 10) {
			info->battery_status = BATTERY_STATUS_LOW;
		}
	}
	else if (info->ac_line_status &&
	         get_hal_bool(device, "battery.rechargeable.is_charging", 0) == 1) {
		info->battery_status = BATTERY_STATUS_CHARGING;
		info->battery_flags = info->battery_flags | BATTERY_FLAGS_CHARGING;
	}
	else if (info->ac_line_status) {
		/* Must be fully charged. */
		info->battery_status = BATTERY_STATUS_HIGH;
	}
	else {
		fprintf(stderr, "unknown battery state\n");
	}

	return 0;
}
