/* WMix -- a mixer using the OSS mixer API
 * Copyright (C)2014 Christophe CURIS for the WindowMaker Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/* include/config.h: functions related to setting the configuration */

#ifndef WMIX_CONFIG_H
#define WMIX_CONFIG_H

#define EXCLUDE_MAX_COUNT 100

/* Global Configuration */
extern struct _Config {
	char        *file;				/* full path to config file name */
	char        *display_name;		/* X Display to connect to */
	char        *mixer_device;		/* device file to use for controlling Mixer volumes */

	unsigned int api;               /* Sound API (0 = ALSA, 1 = OSS) */
	unsigned int verbose    : 1;	/* be Verbose when starting */
	unsigned int osd        : 1;	/* show OSD? */
	unsigned int mousewheel : 1;	/* mousewheel enabled? */
	unsigned int scrolltext : 1;	/* scroll channel names? */
	unsigned int mmkeys     : 1;	/* grab multimedia keys for volume control */

	unsigned int wheel_button_up;	/* up button */
	unsigned int wheel_button_down;	/* down button */

	float        scrollstep;		/* scroll mouse step adjustment */
	char        *osd_color;			/* osd color */
	char        *osd_monitor_name;		/* monitor name to display osd on */
	int          osd_monitor_number;	/* monitor number to display osd on */

	char        *exclude_channel[EXCLUDE_MAX_COUNT + 1];	/* Devices to exclude from GUI's list */
} config;

/* Default color for OSD */
extern const char default_osd_color[];

/* Current version of WMixer */
#define VERSION "3.2"

/* Sets the default values in the config */
void config_init(void);

/* Release memory associated with configuration (this concern only stuff needed during startup) */
void config_release(void);

/* Sets configuration from command line */
void parse_cli_options(int argc, char **argv);

/* Read configuration from file */
void config_read(void);

#endif	/* WMIX_CONFIG_H */
