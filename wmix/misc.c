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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "include/common.h"
#include "include/misc.h"

typedef struct {
    int enable;
    int x;
    int y;
    int width;
    int height;
} MRegion;
MRegion mr[16];

extern Config config;

/* Converts separate left and right channel volumes (each in [0, 1]) to
 * volume and balance values. (Volume is in [0, 1], balance is in [-1, 1])
 */
void lr_to_vb(float left, float right, float *volume, float *balance)
{
    assert((left >= 0.0) && (right >= 0.0));

    *volume = MAX(left, right);

    if (left > right)
	*balance = -1.0 + right / left;
    else if (right > left)
	*balance = 1.0 - left / right;
    else
	*balance = 0.0;
}

/* Performs the reverse calculation of lr_to_vb()
 */
void vb_to_lr(float volume, float balance, float *left, float *right)
{
/*    *left = volume; *right = volume; return; // XXX */

    *left = volume * (1.0 - MAX(0.0, balance));
    *right = volume * (1.0 + MIN(0.0, balance));
}

double get_current_time(void)
{
    struct timeval tv;
    double t;

    gettimeofday(&tv, NULL);

    t = (double)tv.tv_sec;
    t += (double)tv.tv_usec / 1.0e6;

    return t;
}

void add_region(int index, int x, int y, int width, int height)
{
    mr[index].enable = 1;
    mr[index].x = x;
    mr[index].y = y;
    mr[index].width = width;
    mr[index].height = height;
}

int check_region(int x, int y)
{
    register int i;
    bool found = false;

    for (i = 0; i < 16 && !found; i++) {
	if (mr[i].enable && x >= mr[i].x &&
	    x <= mr[i].x + mr[i].width &&
	    y >= mr[i].y && y <= mr[i].y + mr[i].height)
	    found = true;
    }
    if (!found)
	return -1;
    return (i - 1);
}

void config_read(void)
{
    FILE *fp;
    char buf[512];
    char *ptr;

    if (config.file == NULL)
	return;

    fp = fopen(config.file, "r");
    if (!fp)
	return;

    while (fgets(buf, 512, fp)) {
	if ((ptr = strstr(buf, "mousewheel="))) {
	    ptr += 11;
	    config.mousewheel = atoi(ptr);
	}
	if ((ptr = strstr(buf, "scrolltext="))) {
	    ptr += 11;
	    config.scrolltext = atoi(ptr);
	}
	if ((ptr = strstr(buf, "osd="))) {
	    ptr += 4;
	    config.osd = atoi(ptr);
	}
	if ((ptr = strstr(buf, "osdcolor="))) {
	    char *end;
	    ptr += 9;
	    end = strchr(ptr, '\n');
	    ptr[end - ptr] = '\0';
	    if (config.osd_color)
		free(config.osd_color);
	    config.osd_color = strdup(ptr);
	}
	if ((ptr = strstr(buf, "wheelstep="))) {
	    ptr += 10;
	    /* detect old style config */
	    if (atoi(ptr) > 1)
		config.scrollstep = (float)atoi(ptr) / 100.0;
	    else
		config.scrollstep = atof(ptr);
	}
	if ((ptr = strstr(buf, "wheelbtn1="))) {
	    ptr += 10;
	    config.wheel_button_up = atoi(ptr);
	}
	if ((ptr = strstr(buf, "wheelbtn2="))) {
	    ptr += 10;
	    config.wheel_button_down = atoi(ptr);
	}
    }
    fclose(fp);
}
