/*
 *    WMCPULoad - A dockapp to monitor CPU usage
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

#include "libdockapp/dockapp.h"
#include "cpu.h"
#include "backlight_on.xpm"
#include "backlight_off.xpm"
#include "parts.xpm"

#define SIZE	    58
#define WINDOWED_BG "  \tc #AEAAAE"
#define MAX_HISTORY 16
#define CPUNUM_NONE -1

typedef enum { LIGHTON, LIGHTOFF } light;

Pixmap backdrop_on;
Pixmap backdrop_off;
Pixmap parts;
Pixmap mask;
static char	*display_name = "";
static char	*title = NULL;
static char	*light_color = NULL;	/* back-light color */
static unsigned	update_interval = 1;
static light	backlight = LIGHTOFF;
static unsigned	alarm_threshold = 101;
static cpu_options cpu_opts;
static int	history[MAX_HISTORY];	/* history of cpu usage */
static int	hindex = 0;

/* prototypes */
static void update(void);
static void redraw(void);
static void switch_light(void);
static void draw_digit(int per);
static void parse_arguments(int argc, char **argv);
static void print_help(char *prog);
Window dockapp_win(void);
#if USE_SMP
static void draw_cpunumber(void);
#endif

int
main(int argc, char **argv)
{
    XEvent event;
    XpmColorSymbol colors[2] = { {"Back0", NULL, 0}, {"Back1", NULL, 0} };
    int ncolor = 0;

    /* Parse Command-Line */
    cpu_opts.ignore_nice = False;
    cpu_opts.cpu_number = CPUNUM_NONE;
    parse_arguments(argc, argv);

    /* Initialize Application */
    cpu_init();
    dockapp_open_window(display_name, title == NULL ? PACKAGE : title,
			SIZE, SIZE, argc, argv);
    dockapp_set_eventmask(ButtonPressMask | ExposureMask);

    if (light_color) {
	colors[0].pixel = dockapp_getcolor_pixel(light_color);
	colors[1].pixel = dockapp_blendedcolor(light_color, -24, -24, -24, 1.0);
	ncolor = 2;
    }

    /* change raw xpm data to pixmap */
    if (dockapp_stat == WINDOWED_WITH_PANEL)
	backlight_on_xpm[1] = backlight_off_xpm[1] = WINDOWED_BG;
    dockapp_xpm2pixmap(backlight_on_xpm, &backdrop_on, &mask, colors, ncolor);
    dockapp_xpm2pixmap(backlight_off_xpm, &backdrop_off, NULL, NULL, 0);
    dockapp_xpm2pixmap(parts_xpm, &parts, NULL, colors, ncolor);
    /* shape window */
    if (dockapp_stat == DOCKABLE_ICON || dockapp_stat == WINDOWED) {
	dockapp_setshape(mask, 0, 0);
    }
    if (mask) XFreePixmap(display, mask);

    /* Initialize pixmap */
    if (backlight == LIGHTON) {
	dockapp_set_background(backdrop_on);
    } else {
	dockapp_set_background(backdrop_off);
    }
    dockapp_show();

    /* Main loop */
    for (;;) {
	if (dockapp_nextevent_or_timeout(&event, update_interval * 1000)) {
	/* Next Event */
	    switch(event.type) {
		case ButtonPress:
		    switch_light();
		    redraw();
		    break;
		case Expose:
		    redraw();
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

/* called by timer */
static void
update(void)
{
    int usage;
    int x, h;
    Pixmap backdrop;

    static light pre_backlight;
    static Bool in_alarm_mode = False;

    if (backlight == LIGHTON) {
	backdrop = backdrop_on;
	x = 2;
    } else {
	backdrop = backdrop_off;
	x = 0;
    }

    /* get current cpu usage in percent */
    usage = cpu_get_usage(&cpu_opts);
    hindex++;
    if (hindex >= MAX_HISTORY) {
	hindex = 0;
    }
    history[hindex] = usage;

    /* alarm mode */
    if (usage >= alarm_threshold) {
	if (!in_alarm_mode) {
	    in_alarm_mode = True;
	    pre_backlight = backlight;
	}
	if (backlight == LIGHTOFF) {
	    switch_light();
	    redraw();
	    return;
	}
    } else {
	if (in_alarm_mode) {
	    in_alarm_mode = False;
	    if (backlight != pre_backlight) {
		switch_light();
		redraw();
		return;
	    }
	}
    }

    /* slide past chart */
    dockapp_copyarea(dockapp_win(), dockapp_win(), 9, 33, 44, 21, 6, 33);
    dockapp_copy2window(backdrop, 51, 33, 2, 21, 51, 33);

    /* clear digit */
    dockapp_copy2window(backdrop, 5, 7, 34, 20, 5, 7);

    /* draw digit */
    draw_digit(usage);

#ifdef USE_SMP
    /* draw cpu number */
    if (cpu_opts.cpu_number != CPUNUM_NONE)
	draw_cpunumber();
#endif

    /* draw current chart */
    h = (21 * usage) / 100;
    dockapp_copy2window(parts, 100+x, 21-h, 2, h, 51, 54-h);
}

/* called when mouse button pressed */
static void
switch_light(void)
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

static void
redraw(void)
{
    int h, i, j = hindex;
    int x = 0;
    Pixmap backdrop;

    if (backlight == LIGHTON) {
	backdrop = backdrop_on;
	x = 2;
    } else {
	backdrop = backdrop_off;
	x = 0;
    }

    dockapp_copy2window(backdrop, 0, 0, 58, 58, 0, 0);

    /* redraw digit */
    draw_digit(history[hindex]);

#ifdef USE_SMP
    /* draw cpu number */
    if (cpu_opts.cpu_number != CPUNUM_NONE)
	draw_cpunumber();
#endif

    /* redraw chart */
    for (i = 0; i < MAX_HISTORY; i++) {
	h = (21 * history[j]) / 100;
	dockapp_copy2window(parts, 100+x, 21-h, 2, h, 51-3*i, 54-h);
	j--;
	if (j < 0) j = MAX_HISTORY - 1;
    }
}

static void
draw_digit(int per)
{
    int v100, v10, v1;
    int y = 0;

    if (per < 0) per = 0;
    if (per > 100) per = 100;

    v100 = per / 100;
    v10  = (per - v100 * 100) / 10;
    v1   = (per - v100 * 100 - v10 * 10);

    if (backlight == LIGHTON) {
	y = 20;
    }

    /* draw digit */
    dockapp_copy2window(parts, v1 * 10, y, 10, 20, 29, 7);
    if (v10 != 0) {
	dockapp_copy2window(parts, v10 * 10, y, 10, 20, 17, 7);
    }
    if (v100 == 1) {
	dockapp_copy2window(parts, 10, y, 10, 20,  5, 7);
	dockapp_copy2window(parts,  0, y, 10, 20, 17, 7);
	dockapp_copy2window(parts,  0, y, 10, 20, 29, 7);
    }

}


#ifdef USE_SMP
static void
draw_cpunumber(void)
{
    int x, v1 = 0, v2 = 0;

    v2 = cpu_opts.cpu_number / 10;
    v1 = cpu_opts.cpu_number - v2 * 10;

    x = backlight == LIGHTON ? 50 : 0;

    if (v2)
	dockapp_copy2window(parts, x + v2 * 5, 40, 5, 9, 44, 10);

    dockapp_copy2window(parts, x +  v1 * 5, 40, 5, 9, 50, 10);
}
#endif

static void
parse_arguments(int argc, char **argv)
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

	else if (!strcmp(argv[i], "--backlight") || !strcmp(argv[i], "-bl"))
	    backlight = LIGHTON;

	else if (!strcmp(argv[i], "--light-color") || !strcmp(argv[i], "-lc")) {
	    light_color = argv[i + 1];
	    i++;
	}
#ifdef IGNORE_NICE
	else if (!strcmp(argv[i], "--ignore-nice") || !strcmp(argv[i], "-n"))
	    cpu_opts.ignore_nice = True;
#endif

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
	} else if (!strcmp(argv[i], "--alarm") || !strcmp(argv[i], "-a")) {
	    int integer;
	    if (argc == i + 1)
		alarm_threshold = 90;
	    else if (sscanf(argv[i + 1], "%i", &integer) != 1)
		alarm_threshold = 90;
	    else if (integer < 0 || integer > 100)
		fprintf(stderr, "%s: argument %s must be from 0 to 100\n",
			argv[0], argv[i]), exit(1);
	    else
		alarm_threshold = integer, i++;
	} else if (!strcmp(argv[i], "--windowed")
		   || !strcmp(argv[i], "-w"))
	    dockapp_stat = WINDOWED;

	else if (!strcmp(argv[i], "--windowed-withpanel")
		   || !strcmp(argv[i], "-wp"))
	    dockapp_stat = WINDOWED_WITH_PANEL;

	else if (!strcmp(argv[i], "--broken-wm") || !strcmp(argv[i], "-bw"))
	    dockapp_isbrokenwm = True;

	else if (!strcmp(argv[i], "--title") || !strcmp(argv[i], "-t")) {
	    title = argv[i + 1];
	    i++;
	}

#ifdef USE_SMP
	else if (!strcmp(argv[i], "--cpu") || !strcmp(argv[i], "-c")) {
	    int integer;
	    if (argc == i + 1)
		fprintf(stderr, "%s: error parsing argument for option %s\n",
			argv[0], argv[i]), exit(1);
	    if (sscanf(argv[i + 1], "%i", &integer) != 1)
		fprintf(stderr, "%s: error parsing argument for option %s\n",
			argv[0], argv[i]), exit(1);
	    if (integer < 0)
		fprintf(stderr, "%s: argument %s must be >=0\n",
			argv[0], argv[i]), exit(1);
	    cpu_opts.cpu_number = integer;
	    i++;
	}
#endif	/* USE_SMP */

	else {
	    fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0],
		    argv[i]);
	    print_help(argv[0]), exit(1);
	}
    }
}

static void
print_help(char *prog)
{
    printf("Usage : %s [OPTIONS]\n", prog);
    printf("WMCPULoad - A dockapp to monitor CPU usage\n");
    printf("  -d,  --display <string>       display to use\n");
    printf("  -t,  --title <string>         application title name\n");
    printf("  -bl, --backlight              turn on back-light\n");
    printf("  -lc, --light-color <string>   "
	   "back-light color(rgb:6E/C6/3B is default)\n");
    printf("  -i,  --interval <number>      "
	   "number of secs between updates (1 is default)\n");
#ifdef USE_SMP
    printf("  -c,  --cpu <number>           "
	   "which CPU is monitored (0, 1, ... )\n");
#endif
#ifdef IGNORE_NICE
    printf("  -n,  --ignore-nice            ignore a nice value\n");
#endif
    printf("  -h,  --help                   show this help text and exit\n");
    printf("  -v,  --version                show program version and exit\n");
    printf("  -w,  --windowed               "
	   "run the application in windowed mode\n");
    printf("  -wp, --windowed-withpanel     "
	   "run the application in windowed mode\n");
    printf("                                with background panel\n");
    printf("  -bw, --broken-wm              "
	   "activate broken window manager fix\n");
    printf("  -a,  --alarm <percentage>     "
	   "activate alarm mode. <percentage> is threshold\n");
    printf("                                "
	   "of percentage from 0 to 100.(90 is default)\n");
}
/* ex:set sw=4 softtabstop=4: */
