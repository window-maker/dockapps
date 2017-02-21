/* csnlock v1.02
 * Copyright (C) 2002 Simon Hunter (lists@sprig.dyn.dhs.org)
 *
 * cnslock is a dock application that displays the current state of the
 * three lock keys (caps, num, and scroll)
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
 * You should have receive a copy of the GNU General Public License along with
 * this program; if you still want it, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Hacked from various wm applets.
 *
 */

/* general includes */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <ctype.h>

/* gdk includes */
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

/* x11 includes */
#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

/* header includes */
#include "include/defines.h"
#include "include/applet.h"
#include "include/cnslock.h"
#include "include/kleds.h"

/* graphic includes */
#include "graphics/none.h"
#include "graphics/caps.h"
#include "graphics/num.h"
#include "graphics/scroll.h"
#include "graphics/caps_num.h"
#include "graphics/caps_scroll.h"
#include "graphics/num_scroll.h"
#include "graphics/caps_num_scroll.h"

/* stuff */
void cnslock_update(void);

/* misc support functions */
void prepare_backbuffer(int solid);
void parse_options(int argc, char **argv);
void do_help(void);
void do_version(void);
void make_rgb_buffer256(unsigned char *target, unsigned char *source,int width, int height, int frames, char *cmap);

/* globals */
AppletData ad;

/* keeps track of mouse focus */
int proximity;

/* background style (n/a) */
int selected_background=1;

/* initial positional data (n/a) */
int posx=-1, posy=-1;

/* window manager style (0=wm, 1=other) */
int manager_style=0;

/* the main routine */
int main(int argc, char **argv)
{
    GdkEvent *event;

    srand(time(NULL));

    /* initialize GDK */
    if (!gdk_init_check(&argc, &argv))
	{
		fprintf(stderr, "GDK init failed, bye bye.  Check \"DISPLAY\" variable.\n");
		exit(-1);
    }
    gdk_rgb_init();

    /* parse command line options */
    parse_options(argc, argv);

    /* zero main data structure */
    memset(&ad, 0, sizeof(ad));

	/* intialise keyboard leds */
    init_kleds();

    /* create dockapp window. creates windows, allocates memory, etc */
    make_new_cnslock_dockapp(manager_style);

    /* draw initial background */
    prepare_backbuffer(selected_background);

    while(1)
	{
		while(gdk_events_pending())
		{
	    	event = gdk_event_get();
		    if(event)
			{
				switch (event->type)
				{
					case GDK_DESTROY:
					    gdk_exit(0);
					    exit(0);
					    break;
					case GDK_BUTTON_PRESS:
					    /* printf("button press\n"); */
					    break;
					case GDK_ENTER_NOTIFY:
					    proximity = 1;
					    break;
					case GDK_LEAVE_NOTIFY:
					    proximity = 0;
					    break;
					default:
					    break;
				}
		    }
		}

		usleep(20000);
		cnslock_update();

		/* actually draw the rgb buffer to screen */

		if (manager_style==0)
			gdk_draw_rgb_image(ad.iconwin, ad.gc, XMIN, YMIN, XMAX, YMAX, GDK_RGB_DITHER_NONE, ad.rgb, XMAX * 3);
		else
			gdk_draw_rgb_image(ad.win, ad.gc, XMIN, YMIN, XMAX, YMAX, GDK_RGB_DITHER_NONE, ad.rgb, XMAX * 3);
	}
    return 0;
}				/* main */

/* update caps, num, scroll lock */
void cnslock_update(void)
{
    int status;

    memcpy(&ad.rgb, &ad.bgr, RGBSIZE);

	status = check_kleds();
	if ((status & 0) == 0)
		make_rgb_buffer256(ad.rgb,none_data,none_width,none_height,1, *none_cmap);
	if ((status & 1) == 1)
		make_rgb_buffer256(ad.rgb,c_data,c_width,c_height,1, *c_cmap);
    if ((status & 2) == 2)
		make_rgb_buffer256(ad.rgb,n_data,n_width,n_height,1, *n_cmap);
    if ((status & 3) == 3)
		make_rgb_buffer256(ad.rgb,cn_data,cn_width,cn_height,1, *cn_cmap);
    if ((status & 4) == 4)
		make_rgb_buffer256(ad.rgb,s_data,s_width,s_height,1, *s_cmap);
    if ((status & 5) == 5)
		make_rgb_buffer256(ad.rgb,cs_data,cs_width,cs_height,1, *cs_cmap);
    if ((status & 6) == 6)
		make_rgb_buffer256(ad.rgb,ns_data,ns_width,ns_height,1, *ns_cmap);
    if ((status & 7) == 7)
		make_rgb_buffer256(ad.rgb,cns_data,cns_width,cns_height,1, *cns_cmap);

	memcpy(&ad.bgr, &ad.rgb, RGBSIZE);
}

void prepare_backbuffer(int solid)
{
	make_rgb_buffer256(ad.rgb,none_data,none_width,none_height,1, *none_cmap);

	/* copy it to the "frequent use" buffer */
	memcpy(&ad.bgr, &ad.rgb, RGBSIZE);

}				/* prepare_backbuffer */

void make_rgb_buffer256(unsigned char *target, unsigned char *source, int width, int height, int frames, char *cmap)
{
    int i, j = 0;

    for (i = 0; i < (width * height * frames); i++)
	{
    	target[j] = cmap[3*source[i]];
		target[j+1] = cmap[3*source[i]+1];
    	target[j+2] = cmap[3*source[i]+2];
    	j += 3;
    }
}

void parse_options(int argc, char *argv[])
{
    int c = 0;

    struct option long_options[] = {
      {"help",         no_argument,       NULL,          1},
      {"h",            no_argument,       NULL,          1},
      {"version",      no_argument,       NULL,          2},
      {"v",            no_argument,       NULL,          2},
	  {"window",       no_argument,       NULL,          3},
	  {"w",            no_argument,       NULL,          3},

      {0,              0,                 0,             0}
    };

    while ((c = getopt_long_only(argc, argv, "", long_options, NULL)) != -1)
	{
		switch(c)
	  	{
      		case 1:
				do_help();
				exit(0);
				break;
			case 2:
				do_version();
				exit(0);
				break;
			case 3:
				manager_style = 1;
				break;
		}
    }
}

void do_version(void)
{
    printf("\ncnslock applet v%.2f\n\n", VERSION);
}

/*
-h,--help		Display help
-v,--version	Display version
-w,--window		Alternative window system
*/

void do_help(void)
{
    do_version();
    printf("This is an applet that displays the various states of the CAPS, NUM and SCROLL LOCK keys.\n"
		"\nUsage: cnslock [options]\n\n"
		" -h\t--help\t\tShow this message and exit.\n"
		" -v\t--version\tShow version and exit.\n"
		" -w\t--window\tUse alternative windowing system.\n\n"
    );
}

