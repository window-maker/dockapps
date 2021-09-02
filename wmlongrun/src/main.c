/*
 *    WMLongRun - A dockapp to monitor LongRun status
 *    Copyright (C) 2001,2002  Seiichi SATO <ssato@sh.rim.or.jp>

 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.

 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.

 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#if defined(HAVE_STRING_H)
#include <string.h>
#elif defined(HAVE_STRINGS_H)
#include <strings.h>
#endif
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include "dockapp.h"
#include "longrun.h"
#include "common.h"
#include "backdrop_on.xpm"
#include "backdrop_off.xpm"
#include "backdrop_led.xpm"
#include "backdrop_on_g.xpm"
#include "backdrop_off_g.xpm"
#include "backdrop_led_g.xpm"
#include "parts.xpm"

#define SIZE	58
#define WINDOWED_BG "  \tc #AEAAAE"

typedef enum { LIGHTON, LIGHTOFF} light;

Pixmap pixmap;
Pixmap backdrop_on;
Pixmap backdrop_off;
Pixmap backdrop_led;
Pixmap backdrop_on_g;
Pixmap backdrop_off_g;
Pixmap backdrop_led_g;
Pixmap parts;
Pixmap mask;
static char	*display_name = "";
static char	*light_color = NULL;
static unsigned	update_interval = 500;
static light	backlight = LIGHTOFF;
static Bool is_led_style = False;

static char *cpuid_dev = NULL;
static char *msr_dev = NULL;

/* prototypes */
static void switch_light(void);
static void update(void);
static void parse_arguments(int argc, char **argv);
static void print_help(char *prog);

int main(int argc, char **argv)
{
    XEvent event;
    XpmColorSymbol colors[2] = { {"Back0", NULL, 0}, {"Back1", NULL, 0} };
    int ncolor = 0;

    /* Parse CommandLine */
    parse_arguments(argc, argv);

    /* Initialize Application */
    dockapp_open_window(display_name, PACKAGE, SIZE, SIZE, argc, argv);
    dockapp_set_eventmask(ButtonPressMask);

    if (light_color) {
	colors[0].pixel = dockapp_getcolor(light_color);
	colors[1].pixel = dockapp_blendedcolor(light_color, -24, -24, -24, 1.0);
	ncolor = 2;
    }

    /* change raw xpm data to pixmap */
    if (dockapp_iswindowed)
	backdrop_on_xpm[1] = backdrop_off_xpm[1] = WINDOWED_BG;
    	backdrop_on_g_xpm[1] = backdrop_off_g_xpm[1] = WINDOWED_BG;
    if (!is_led_style) {
	dockapp_xpm2pixmap(backdrop_on_xpm, &backdrop_on, &mask, colors,ncolor);
	dockapp_xpm2pixmap(backdrop_off_xpm, &backdrop_off, NULL, NULL, 0);
   	dockapp_xpm2pixmap(backdrop_on_g_xpm, &backdrop_on_g, &mask, colors,ncolor);
	dockapp_xpm2pixmap(backdrop_off_g_xpm, &backdrop_off_g, NULL, NULL, 0);
    } else {
	dockapp_xpm2pixmap(backdrop_led_xpm, &backdrop_led, &mask, colors, ncolor);
    	dockapp_xpm2pixmap(backdrop_led_g_xpm, &backdrop_led_g, &mask, colors, ncolor);
    }
    dockapp_xpm2pixmap(parts_xpm, &parts, NULL, colors, ncolor);
    /* shape window */
    if (!dockapp_iswindowed)
	dockapp_setshape(mask, 0, 0);
    if (mask) XFreePixmap(display, mask);
    /* pixmap: draw area */
    pixmap = dockapp_XCreatePixmap(SIZE, SIZE);

    /* Initialize pixmap */
    longrun_init(cpuid_dev, msr_dev);
    update();
    dockapp_set_background(pixmap);
    dockapp_show();

    /* Main loop */
    for (;;) {
	if (dockapp_nextevent_or_timeout(&event, update_interval)) {
	/* Next Event */
	    switch(event.type) {
		case ButtonPress:
		    if (!is_led_style) {  /* led does not have back-light */
			switch_light();
			update();
		    }
		    break;
		default: /* make gcc happy */
		    break;
	    }
	} else {
	/* Time Out */
	    update();
	}
    }

    return 0;
}

/* called when mouse button pressed */
static void switch_light(void)
{
    switch (backlight) {
    case LIGHTOFF:
	backlight = LIGHTON;
	break;
    case LIGHTON:
	backlight = LIGHTOFF;
	break;
    }
}


static void update(void)
{
    static int percent;		/* LongRun performance level */
    static int flags;		/* LongRun flags */
    static int mhz;		/* LongRun frequency */
    static int voltz;		/* LongRun voltage */

    int digit1 = 0, digit10 = 0, digit100 = 0, digit1000=0;

    longrun_get_stat(&percent, &flags, &mhz, &voltz);

    digit1000 = mhz / 1000;
    digit100 = (mhz - digit1000 * 1000) / 100;
    digit10 = (mhz - digit1000 * 1000 - digit100 * 100) / 10;
    digit1 = mhz - digit1000 * 1000 - digit100 * 100 - digit10 * 10;

    /* LCD interface */
    if (!is_led_style) {
	int y_lrmode = 0, y_gauge = 0, y_digit = 0;

	/* clear */
	switch (backlight) {
	    case LIGHTON:
		if (digit1000==0) {
			dockapp_copyarea(backdrop_on, pixmap, 0, 0, 58, 58, 0, 0);
		}
		else {
			dockapp_copyarea(backdrop_on_g, pixmap, 0, 0, 58, 58, 0, 0);
		}
		y_lrmode = 11;
		y_gauge = 14;
		y_digit = 13;
		break;
	    case LIGHTOFF:
		if (digit1000==0) {
			dockapp_copyarea(backdrop_off, pixmap, 0, 0, 58, 58, 0, 0);
		}
		else {
			dockapp_copyarea(backdrop_off_g, pixmap, 0, 0, 58, 58, 0, 0);
		}
		break;
	}

	/* longrun flags (performance or economy) */
	switch (flags) {
	    case LONGRUN_FLAGS_PEFORMANCE:
		dockapp_copyarea(parts, pixmap, 24, y_lrmode, 24, 11, 31, 5);
		break;
	    case LONGRUN_FLAGS_ECONOMY:
		dockapp_copyarea(parts, pixmap,  0, y_lrmode, 24, 11, 5, 5);
		break;
	    default:
		break;		/* make gcc happy */
	}

	/* draw digit (frequency) */
	if (digit1000==0) {
		dockapp_copyarea(parts, pixmap, digit100*7,y_digit+33,  7,13,   6,22);
		dockapp_copyarea(parts, pixmap,  digit10*7,y_digit+33,  7,13,  15,22);
		dockapp_copyarea(parts, pixmap,   digit1*7,y_digit+33,  7,13,  24,22);
	}
	else {
		dockapp_copyarea(parts, pixmap, digit1000*7,y_digit+33,  7,13,   6,22);
                dockapp_copyarea(parts, pixmap,  digit100*7,y_digit+33,  7,13,  15,22);
                dockapp_copyarea(parts, pixmap,   digit10*7,y_digit+33,  7,13,  24,22);
	}
	/* draw level gauge */
	dockapp_copyarea(parts, pixmap, 0,y_gauge+72, 49*percent/100,14,5,40);
    }

    /* LED interface */
    else {
	if (!digit1000) {
		dockapp_copyarea(backdrop_led, pixmap, 0, 0, 58, 58, 0, 0);
	}
	else {
		dockapp_copyarea(backdrop_led_g, pixmap, 0, 0, 58, 58, 0, 0);
	}
		/* longrun flags (performance or economy) */
	switch (flags) {
	case LONGRUN_FLAGS_PEFORMANCE:
	    dockapp_copyarea(parts, pixmap, 24, 22, 24, 11, 30, 3);
	    break;
	case LONGRUN_FLAGS_ECONOMY:
	    dockapp_copyarea(parts, pixmap, 0, 22, 24, 11, 3, 3);
	    break;
	default:
	    break;		/* make gcc happy */
	}

	/* draw digit (frequency) */
	if (digit1000==0) {
		dockapp_copyarea(parts, pixmap, digit100*7,59,  7,13,   4,22);
		dockapp_copyarea(parts, pixmap,  digit10*7,59,  7,13,  13,22);
		dockapp_copyarea(parts, pixmap,   digit1*7,59,  7,13,  22,22);
	}
	else {
		dockapp_copyarea(parts, pixmap, digit1000*7,59,  7,13,   4,22);
	        dockapp_copyarea(parts, pixmap,  digit100*7,59,  7,13,  13,22);
	        dockapp_copyarea(parts, pixmap,   digit10*7,59,  7,13,  22,22);
	}
	/* draw level gauge */
	dockapp_copyarea(parts, pixmap, 0, 100, 56 * percent / 100, 16, 1, 41);
    }

    /* show */
    dockapp_copy2window(pixmap);
}

static void parse_arguments(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
	    print_help(argv[0]), exit(0);

	else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v"))
	    printf("%s version %s\n", PACKAGE, VERSION), exit(0);

	else if (!strcmp(argv[i], "--display") || !strcmp(argv[i], "-d")) {
	    display_name = argv[i + 1];
	    i++;
	}

	else if (!strcmp(argv[i], "--backlight")
		 || !strcmp(argv[i], "-bl"))
	    backlight = LIGHTON;

	else if (!strcmp(argv[i], "--light-color")
		 || !strcmp(argv[i], "-lc")) {
	    if (argc == i + 1)
		fprintf(stderr,
			"%s: error parsing argument for option %s\n",
			argv[0], argv[i]), exit(1);
	    light_color = argv[i + 1];
	    i++;
	}

	else if (!strcmp(argv[i], "--interval") || !strcmp(argv[i], "-i")) {
	    int integer;
	    if (argc == i + 1)
		fprintf(stderr,
			"%s: error parsing argument for option %s\n",
			argv[0], argv[i]), exit(1);
	    if (sscanf(argv[i + 1], "%i", &integer) != 1)
		fprintf(stderr,
			"%s: error parsing argument for option %s\n",
			argv[0], argv[i]), exit(1);
	    if (integer < 1)
		fprintf(stderr, "%s: argument %s must be >=1\n",
			argv[0], argv[i]), exit(1);
	    update_interval = integer;
	    i++;
	}

	else if (!strcmp(argv[i], "--led") || !strcmp(argv[i], "-l"))
	    is_led_style = True;

	else if (!strcmp(argv[i], "--windowed")
		   || !strcmp(argv[i], "-w"))
	    dockapp_iswindowed = True;

	else if (!strcmp(argv[i], "--broken-wm")
		 || !strcmp(argv[i], "-bw"))
	    dockapp_isbrokenwm = True;

#ifdef LINUX
	else if (!strcmp(argv[i], "--cpuid-device")
		 || !strcmp(argv[i], "-cd")) {
	    cpuid_dev = argv[i + 1];
	    i++;
	}
	else if (!strcmp(argv[i], "--msr-device")
		 || !strcmp(argv[i], "-md")) {
	    msr_dev = argv[i + 1];
	    i++;
	}
#endif

	else {
	    fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0],
		    argv[i]);
	    print_help(argv[0]), exit(1);
	}
    }
}

static void print_help(char *prog)
{
    printf("Usage : %s [OPTIONS]\n", prog);
    printf
	("WMLongRun - A dockapp to monitor the LongRun(tm) status on Crusoe(tm) processors.\n");
    printf("  -d,  --display <string>       display to use\n");
    printf("  -bl, --backlight              turn on back-light\n");
    printf("  -lc, --light-color <string>   back-light color(rgb:6E/C6/3B is default)\n");
    printf("  -i,  --interval <number>      number of milliseconds between updates\n");
    printf("                                (500 is default)\n");
#ifdef LINUX
    printf("  -cd, --cpuid-device <device>  CPUID device ('/dev/cpu/0/cpuid' is default)\n");
    printf("  -md, --msr-device <device>    MSR device ('/dev/cpu/0/msr' is default)\n");
#endif
    printf("  -l,  --led                    run the application with LED interface\n");
    printf("  -h,  --help                   show this help text and exit\n");
    printf("  -v,  --version                show program version and exit\n");
    printf("  -w,  --windowed               run the application in windowed mode\n");
    printf("  -bw, --broken-wm              activate broken window manager fix\n");
}
