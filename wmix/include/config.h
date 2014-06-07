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

/* Global Configuration */
extern struct _Config {
	char        *file;				/* full path to config file name */

	unsigned int osd        : 1;	/* show OSD? */
	unsigned int mousewheel : 1;	/* mousewheel enabled? */
	unsigned int scrolltext : 1;	/* scroll channel names? */

	unsigned int wheel_button_up;	/* up button */
	unsigned int wheel_button_down;	/* down button */

	float        scrollstep;		/* scroll mouse step adjustment */
	char        *osd_color;			/* osd color */
} config;

/* Read configuration from file */
void config_read(void);

#endif	/* WMIX_CONFIG_H */
