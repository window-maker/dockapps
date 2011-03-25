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

Pixmap pixmap;
Pixmap backdrop_on;
Pixmap backdrop_off;
Pixmap parts;
Pixmap pix_chartbuf;
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
static void switch_light(void);
static void draw_digit(int per);
static void parse_arguments(int argc, char **argv);
static void print_help(char *prog);
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
    cpu_opts.ignore_procs = 0;
    parse_arguments(argc, argv);

    /* Initialize Application */
    cpu_init();
    dockapp_open_window(display_name, title == NULL ? PACKAGE : title,
			SIZE, SIZE, argc, argv);
    dockapp_set_eventmask(ButtonPressMask);

    if (light_color) {
	colors[0].pixel = dockapp_getcolor_pixel(light_color);
	colors[1].pixel = dockapp_blendedcolor(light_color, -24, -24, -24, 1.0);
	ncolor = 2;
    }

    /* change raw xpm data to pixmap */
    if (dockapp_stat == WINDOWED_WITH_PANEL) {
	backlight_on_xpm[1] = backlight_off_xpm[1] = WINDOWED_BG;
    }
    dockapp_xpm2pixmap(backlight_on_xpm, &backdrop_on, &mask, colors, ncolor);
    dockapp_xpm2pixmap(backlight_off_xpm, &backdrop_off, NULL, NULL, 0);
    dockapp_xpm2pixmap(parts_xpm, &parts, NULL, colors, ncolor);
    /* shape window */
    if (dockapp_stat == DOCKABLE_ICON || dockapp_stat == WINDOWED) {
	dockapp_setshape(mask, 0, 0);
    }
    if (mask) XFreePixmap(display, mask);
    /* pixmap : draw area */
    pixmap = dockapp_XCreatePixmap(SIZE, SIZE);
    pix_chartbuf = dockapp_XCreatePixmap(SIZE, SIZE);

    /* Initialize pixmap */
    if (backlight == LIGHTON) {
	dockapp_copyarea(backdrop_on, pixmap, 0, 0, SIZE, SIZE, 0, 0);
    } else {
	dockapp_copyarea(backdrop_off, pixmap, 0, 0, SIZE, SIZE, 0, 0);
    }
    dockapp_set_background(pixmap);
    dockapp_show();

    /* Main loop */
    for (;;) {
	if (dockapp_nextevent_or_timeout(&event, update_interval * 1000)) {
	/* Next Event */
	    switch(event.type) {
		case ButtonPress:
		    switch_light();
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

    static light pre_backlight;
    static Bool in_alarm_mode = False;

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
	    return;
	}
    } else {
	if (in_alarm_mode) {
	    in_alarm_mode = False;
	    if (backlight != pre_backlight) {
		switch_light();
		return;
	    }
	}
    }

    /* save current chart */
    dockapp_copyarea(pixmap, pix_chartbuf,  9, 33, 44, 21, 0, 0);

    /* all clear */
    if (backlight == LIGHTON) {
	dockapp_copyarea(backdrop_on, pixmap, 0, 0, 58, 58, 0, 0);
	x = 2;
    } else {
	dockapp_copyarea(backdrop_off, pixmap, 0, 0, 58, 58, 0, 0);
	x = 0;
    }

    /* draw digit */
    draw_digit(usage);

#ifdef USE_SMP
    /* draw cpu number */
    if (cpu_opts.cpu_number != CPUNUM_NONE)
	draw_cpunumber();
#endif

    /* draw chart */
    h = (21 * usage) / 100;
    dockapp_copyarea(pix_chartbuf, pixmap, 0, 0, 44, 21, 6, 33);
    dockapp_copyarea(parts, pixmap,100+x, 21-h, 2, h, 51, 54-h);

    /* show */
    dockapp_copy2window(pixmap);

}

/* called when mouse button pressed */
static void
switch_light(void)
{
    int h, i, j = hindex;
    int x = 0;

    switch (backlight) {
	case LIGHTOFF:
	    backlight = LIGHTON;
	    dockapp_copyarea(backdrop_on, pixmap, 0, 0, 58, 58, 0, 0);
	    x = 2;
	    break;
	case LIGHTON:
	    backlight = LIGHTOFF;
	    dockapp_copyarea(backdrop_off, pixmap, 0, 0, 58, 58, 0, 0);
	    x = 0;
	    break;
    }

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
	dockapp_copyarea(parts, pixmap, 100+x, 21-h, 2, h, 51-3*i, 54-h);
	j--;
	if (j < 0) j = MAX_HISTORY - 1;
    }

    /* show */
    dockapp_copy2window(pixmap);
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
    dockapp_copyarea(parts, pixmap, v1 * 10, y, 10, 20, 29, 7);
    if (v10 != 0) {
	dockapp_copyarea(parts, pixmap, v10 * 10, y, 10, 20, 17, 7);
    }
    if (v100 == 1) {
	dockapp_copyarea(parts, pixmap, 10, y, 10, 20,  5, 7);
	dockapp_copyarea(parts, pixmap,  0, y, 10, 20, 17, 7);
	dockapp_copyarea(parts, pixmap,  0, y, 10, 20, 29, 7);
    }
}


#ifdef USE_SMP
static void
draw_cpunumber(void)
{
    int x_offset = 0;
    int v10 = 0, v1 = 0;

    v10 = cpu_opts.cpu_number / 10;
    v1 = cpu_opts.cpu_number - v10 * 10;

    if (backlight == LIGHTON) {
	x_offset = 50;
    }

    if (v10) {
	dockapp_copyarea(parts, pixmap, x_offset + v10 * 5, 40, 5, 9, 44, 10);
    }
    dockapp_copyarea(parts, pixmap, x_offset +  v1 * 5, 40, 5, 9, 50, 10);
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

#ifdef IGNORE_PROC
	else if (!strcmp(argv[i], "--ignore-proc") || !strcmp(argv[i], "-p")) {
	    if (argc == i + 1)
		fprintf(stderr, "%s: error parsing argument for option %s\n",
			argv[0], argv[i]), exit(1);
	    if (argv[i + 1][0] == '-')
		fprintf(stderr, "%s: error parsing argument for option %s\n",
			argv[0], argv[i]), exit(1);
	    while (i + 1 < argc) {
		if (!(argv[i + 1][0] == '-')) {
		    if (strlen(argv[i + 1]) >= COMM_LEN)
			fprintf(stderr, "%s: command name %s is longer than 15 characters\n",
				argv[0], argv[i + 1]), exit(1);
		    if (cpu_opts.ignore_procs == MAX_PROC)
			fprintf(stderr, "%s: maximum number of command names is %d\n",
				argv[0], MAX_PROC), exit(1);
		    cpu_opts.ignore_proc_list[cpu_opts.ignore_procs] = argv[i + 1];
		    cpu_opts.ignore_procs++;
		} else {
		    break;
		}
		i++;
	    }

	}
#endif	/* IGNORE_PROC */

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

#if defined(USE_SMP) && defined(IGNORE_PROC)
    if (cpu_opts.cpu_number != CPUNUM_NONE && cpu_opts.ignore_procs) {
	fprintf(stderr, "You can't use '-c, --cpu' option with '-p, --ignore_procs' option");
	exit (1);
    }
#endif
}

static void
print_help(char *prog)
{
    printf("Usage : %s [OPTIONS]\n", prog);
    printf("WMCPULoad - A dockapp to monitor CPU usage\n");
    printf("  -d,  --display <string>       display to use\n");
    printf("  -t,  --title <string>         application title name\n");
    printf("  -bl, --backlight              turn on back-light\n");
    printf("  -lc, --light-color <string>   back-light color(rgb:6E/C6/3B is default)\n");
    printf("  -i,  --interval <number>      number of secs between updates (1 is default)\n");
#ifdef USE_SMP
    printf("  -c,  --cpu <number>           CPU number (0, 1, ... )\n");
#endif
#ifdef IGNORE_NICE
    printf("  -n,  --ignore-nice            ignore a nice value\n");
#endif
#ifdef IGNORE_PROC
    printf("  -p,  --ignore-proc <name> ..  ignore all processes specified by command name\n");
#endif
    printf("  -h,  --help                   show this help text and exit\n");
    printf("  -v,  --version                show program version and exit\n");
    printf("  -w,  --windowed               run the application in windowed mode\n");
    printf("  -wp, --windowed-withpanel     run the application in windowed mode\n");
    printf("                                with background panel\n");
    printf("  -bw, --broken-wm              activate broken window manager fix\n");
    printf("  -a,  --alarm <percentage>     activate alarm mode. <percentage> is threshold\n");
    printf("                                of percentage from 0 to 100.(90 is default)\n");
}
