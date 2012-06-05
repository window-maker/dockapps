/* WMix 3.0 -- a mixer using the OSS mixer API.
 * Copyright (C) 2000, 2001
 *	Daniel Richard G. <skunk@mit.edu>,
 *	timecop <timecop@japan.co.jp>
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

typedef unsigned int bool;

#define false 0
#define true (!false)

#define NULL_CURSOR 1
#define NORMAL_CURSOR 2
#define HAND_CURSOR 3
#define BAR_CURSOR 4

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#define MAX_DOUBLE_CLICK_TIME 0.5
#define BUTTON_WHEEL_UP 4
#define BUTTON_WHEEL_DOWN 5

typedef struct _Config Config;

struct _Config {
    char 	*file;			/* full path to config file name */
    unsigned int osd        : 1;	/* show OSD? */
    unsigned int mousewheel : 1;	/* mousewheel enabled? */
    unsigned int scrolltext : 1;	/* scroll channel names? */
    unsigned int wheel_button_up;	/* up button */
    unsigned int wheel_button_down;	/* down button */
    float	 scrollstep;		/* scroll mouse step adjustment */
    char	*osd_color;		/* osd color */
};
