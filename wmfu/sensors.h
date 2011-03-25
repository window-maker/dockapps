/*
 * Copyright (c) 2007 Daniel Borca  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef SENSORS_H_included
#define SENSORS_H_included

typedef enum {
    S_ACPI_THERMAL_ZONE,
    S_ACPI_AC_ADAPTER,
    S_ACPI_BATTERY,
    S_HWMON_CORETEMP,
    S_NVIDIA_SETTINGS_GPUCORETEMP
} SENSOR_TYPE;

typedef struct SENSOR {
    struct SENSOR *next, *prev;
    SENSOR_TYPE type;
    const char *name;
    char *filename;
    int idata;
} SENSOR;

SENSOR *sensors_init (void);
int sensors_nvidia (const char *setting, int *val);
int sensors_read_line (const char *filename, int max, char *out);
void sensors_free (SENSOR *list);

#endif
