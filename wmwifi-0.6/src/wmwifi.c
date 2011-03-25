/*
 *    wmupmon - A dockapp to monitor system uptime
 *    Copyright (C) 2003  Justin Spadea <jzs@mail.rit.edu>
 *
 *    Based on work by Seiichi SATO <ssato@sh.rim.or.jp>
 *    Copyright (C) 2001,2002  Seiichi SATO <ssato@sh.rim.or.jp>
 *    AND by Mark Staggs <me@markstaggs.net>
 *    Copyright (C) 2002  Mark Staggs <me@markstaggs.net>

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

#include "dockapp.h"
#include <dockapp.h>
#include "display_link.xpm"
#include "display_stats.xpm"
#include "parts.xpm"
#include "font.xpm"
#include "wmwifi.h"

#define SIZE	    58
#define WINDOWED_BG "  \tc #AEAAAE"
#define MAX_HISTORY 16
#define CPUNUM_NONE -1

typedef enum { LIGHTON, LIGHTOFF } light;
typedef enum { STANDARD, EXTENDED } interface;
Pixmap pixmap;
Pixmap fonts;
Pixmap backdrop_on;
Pixmap display_link;
Pixmap display_stats;
Pixmap backdrop_on_stats;
Pixmap display_error;
Pixmap parts;
Pixmap mask;
static char *display_name = "";
static char *light_color = NULL;	/* back-light color */
static unsigned update_interval = 1;
static light backlight = LIGHTOFF;
static interface gui = STANDARD;
static interface lgui = STANDARD;
Bool wmwifi_learn;
Bool scroll = True;
int count;
struct theme {
    char *bg;
    char *fg;
    char *hi;
    char *mt;
};
struct theme tcur;
struct theme blgt;
struct wifi lwfi;

/* prototypes */
static void update(struct wifi *wfi);
static void switch_interface(struct wifi *wfi);
static void do_theme(struct theme thm);
static void draw_junk(struct wifi *wfi);
static void draw_ext(struct wifi *wfi);
static void draw_text(char *text, int dx, int dy, Bool digit);
static void parse_arguments(int argc, char **argv);
static void print_help(char *prog);
static void copy_wifi(struct wifi *lwfi, struct wifi *wfi);
static void clear_wifi(struct wifi *wfi);
static void refresh(struct theme thm);

int main(int argc, char **argv)
{
    XEvent event;
    XButtonPressedEvent *bevent;
    int i;
    struct wifi wfi;

    wfi.ifnum = 0;

    /* Parse CommandLine */
    parse_arguments(argc, argv);

    /* Initialize Application */
    dockapp_open_window(display_name, PACKAGE, SIZE, SIZE, argc, argv);
    dockapp_set_eventmask(ButtonPressMask);

    if (backlight == LIGHTON) {
	do_theme(blgt);
    } else {
	do_theme(tcur);
    }

    /* shape window */
    if (!dockapp_iswindowed)
	dockapp_setshape(mask, 0, 0);
    if (mask)
	XFreePixmap(display, mask);

    /* pixmap : draw area */
    pixmap = dockapp_XCreatePixmap(SIZE, SIZE);

    /* Initialize pixmap */

    if (gui == STANDARD)
	dockapp_copyarea(display_link, pixmap, 0, 0, SIZE, SIZE, 0, 0);
    else
	dockapp_copyarea(display_stats, pixmap, 0, 0, SIZE, SIZE, 0, 0);

    dockapp_set_background(pixmap);
    dockapp_show();
    draw_text("link", 32, 47, False);

    /* Main loop */
    for (i = 0;; i++) {
	/* CHANGED */
	if (dockapp_nextevent_or_timeout(&event, update_interval * 100)) {
	    /* Next Event */
	    switch (event.type) {
	    case ButtonPress:
		bevent = (XButtonPressedEvent *) & event;
		switch (bevent->button & 0xff) {
		case Button1:
		    scroll = (scroll) ? False : True;
		    count = 0;
		    break;
		case Button2:
		    next_if(&wfi);
		    break;
		case Button3:
		    switch_interface(&wfi);
		    break;
		}
		break;
	    default:		/* make gcc happy */
		break;
	    }
	} else {
	    /* Time Out */
	    update(&wfi);
	}
    }

    return 0;
}

static void do_theme(struct theme thm)
{
    XpmColorSymbol colors[4] =
	{ {"Back0", NULL, 0}, {"Fore0", NULL, 0}, {"High0", NULL, 0},
    {"Mid0", NULL, 0}
    };
    int ncolor = 0;
    int i;

    colors[0].pixel = dockapp_getcolor(thm.bg);
    colors[1].pixel = dockapp_getcolor(thm.fg);
    colors[2].pixel = dockapp_getcolor(thm.hi);
    colors[3].pixel = dockapp_getcolor(thm.mt);
    ncolor = 4;
    /* change raw xpm data to pixmap */
    if (dockapp_iswindowed)
	display_link_xpm[1] = WINDOWED_BG;

    if (!dockapp_xpm2pixmap
	(display_link_xpm, &display_link, &mask, colors, ncolor)) {
	fprintf(stderr, "Error initializing background image.\n");
	exit(1);
    }
    if (!dockapp_xpm2pixmap
	(display_stats_xpm, &display_stats, &mask, colors, ncolor)) {
	fprintf(stderr, "Error initializing background image.\n");
	exit(1);
    }
    if (!dockapp_xpm2pixmap(parts_xpm, &parts, NULL, colors, ncolor)) {
	fprintf(stderr, "Error initializing parts image.\n");
	exit(1);
    }
    if (!dockapp_xpm2pixmap(font_xpm, &fonts, NULL, colors, ncolor)) {
	fprintf(stderr, "Error initializing fonts image.\n");
	exit(1);
    }
}

/* called by timer */
static void update(struct wifi *wfi)
{
    int sw, percent;
    extern int count;
    //char *str = calloc(1, sizeof(wfi->ifname) + sizeof(wfi->essid) + 3);
    char str[512];

    /* get current link level from /proc/net/wireless */
    copy_wifi(&lwfi, wfi);
    get_wifi_info(wfi);

    if (wfi->max_qual == -1) {
	tcur.bg = "rgb:ff/0/0";
	tcur.fg = "rgb:0/0/0";
	tcur.mt = "rgb:00/00/00";
	do_theme(tcur);
	dockapp_copyarea(display_link, pixmap, 0, 0, 58, 58, 0, 0);
	draw_text("ERROR", (56 / 2) - 5 * 2.5, 4, False);
    } else {
	/* all clear */

	/* If percent == 0, show backlight */
	percent = wfi->link / (wfi->max_qual / 100);
	if (percent > 0) {
	    if (backlight == LIGHTON) {
		refresh(tcur);
	    }
	    backlight = LIGHTOFF;
	} else {
	    if (backlight == LIGHTOFF) {
		refresh(blgt);
	    }
	    backlight = LIGHTON;
	}

	if (gui == STANDARD) {
	    if (gui != lgui) {
		if (backlight == LIGHTOFF)
		    refresh(tcur);
		else
		    refresh(blgt);
		lgui = gui;
	    }
	    /* draw junk */
	    draw_junk(wfi);
	} else {
	    if (gui != lgui) {
		if (backlight == LIGHTOFF)
		    refresh(tcur);
		else
		    refresh(blgt);
		lgui = gui;
	    }
	    draw_ext(wfi);
	}
	count++;
	if (scroll) {
	    dockapp_copyarea(display_link, pixmap, 0, 0, 58, 12, 0, 0);
	    snprintf(str, strlen(wfi->ifname) + strlen(wfi->essid) + 4,
		     "%s - %s", wfi->ifname, wfi->essid);
	    draw_text(str, 52 - (count * 3), 4, False);
	    dockapp_copyarea(display_link, pixmap, 0, 0, 4, 12, 0, 0);
	    dockapp_copyarea(display_link, pixmap, 55, 0, 3, 12, 55, 0);
	    if (count > (strlen(str) * 3))
		count = 1;
	} else {
	    if (count < 50)
		sw = 1;
	    if (count >= 50)
		sw = 2;
	    if (count == 100)
		count = -1;
	    switch (sw) {
	    case 1:
		dockapp_copyarea(display_link, pixmap, 0, 0, 58, 12, 0, 0);
		draw_text(wfi->ifname,
			  ((56 / 2) - (strlen(wfi->ifname) * 2)), 4,
			  False);
		dockapp_copyarea(display_link, pixmap, 0, 0, 4, 12, 0, 0);
		dockapp_copyarea(display_link, pixmap, 55, 0, 3, 12, 55,
				 0);
		break;
	    case 2:
		dockapp_copyarea(display_link, pixmap, 0, 0, 58, 12, 0, 0);
		draw_text(wfi->essid,
			  ((56 / 2) - (strlen(wfi->essid) * 2)), 4, False);
		dockapp_copyarea(display_link, pixmap, 0, 0, 4, 12, 0, 0);
		dockapp_copyarea(display_link, pixmap, 55, 0, 3, 12, 55,
				 0);
		break;
	    }
	}
	/* show */
    }
    dockapp_copy2window(pixmap);
}
static void refresh(struct theme thm)
{
    do_theme(thm);
    switch (gui) {
    case STANDARD:
	dockapp_copyarea(display_link, pixmap, 0, 0, 58, 58, 0, 0);
	clear_wifi(&lwfi);
	draw_text("link", 32, 47, False);
	break;
    case EXTENDED:
	dockapp_copyarea(display_stats, pixmap, 0, 0, 58, 58, 0, 0);
	clear_wifi(&lwfi);
	draw_text("lnk", 37, 45, False);
	draw_text("lvl", 37, 36, False);
	draw_text("nse", 37, 27, False);
	draw_text("rte", 37, 18, False);
	break;
    }

}
static void switch_interface(struct wifi *wfi)
{
    switch (gui) {
    case STANDARD:
	lgui = gui;
	gui = EXTENDED;
	dockapp_copyarea(display_link, pixmap, 0, 0, 58, 58, 0, 0);
	draw_text("lnk", 37, 45, False);
	draw_text("lvl", 37, 36, False);
	draw_text("nse", 37, 27, False);
	draw_text("rte", 37, 18, False);
	break;
    case EXTENDED:
	lgui = gui;
	gui = STANDARD;
	dockapp_copyarea(display_stats, pixmap, 0, 0, 58, 58, 0, 0);
	draw_text("link", 32, 47, False);
	break;
    }
    dockapp_copy2window(pixmap);
}

/* called when mouse button pressed */
static void draw_ext(struct wifi *wfi)
{
    char buffer[25];
    double rate = wfi->bitrate.value;

    if (lwfi.link != wfi->link) {
	sprintf(buffer, "%.f", wfi->link);
	draw_text(buffer, 15, 45, True);
    }

    if (lwfi.level != wfi->level) {
	sprintf(buffer, "%d", wfi->level - 0x100);
	draw_text(buffer, 15, 36, True);
    }

    if ((lwfi.noise - 0x100) != (wfi->noise - 0x100)) {
	sprintf(buffer, "%d", wfi->noise - 0x100);
	draw_text(buffer, 15, 27, True);
    }

    if (lwfi.bitrate.value != wfi->bitrate.value) {
	if (rate >= GIGA) {
	    sprintf(buffer, "%.fgb", rate / GIGA);
	    draw_text(buffer, 9, 18, False);
	} else {
	    if (rate >= MEGA) {
		sprintf(buffer, "%.fmb", (rate / MEGA));
		draw_text(buffer, 9, 18, False);
	    } else {
		sprintf(buffer, "%.fkb", rate / KILO);
		draw_text(buffer, 3, 18, False);
	    }
	}
    }

}
static void draw_text(char *text, int dx, int dy, Bool digit)
{
    int ax, ay = 1, bx, len, i;
    char tmptext[255] = "";

    len = strlen(text);
    bx = 4;

    /*
       for (i = 0; i < len; i++) {
       digit = (!isalpha(text[i])) ? True : False;
       }
     */

    if (digit) {
	if (len == 4)
	    dx -= 6;
	strcat(tmptext, text);
	if (len == 3)
	    strcat(tmptext, text);
	if (len == 2) {
	    tmptext[0] = 0x20;
	    tmptext[1] = text[0];
	    tmptext[2] = text[1];
	    len++;
	}
	if (len == 1) {
	    tmptext[0] = ' ';
	    tmptext[1] = ' ';
	    tmptext[2] = text[0];
	    len += 2;
	}
    } else {
	strcpy(tmptext, text);
    }

    for (i = 0; i < len; i++) {
	if (isalpha(tmptext[i])) {
	    ax = ((tolower(tmptext[i]) - 97) * 6) + 1;
	    ay = 1;
	} else {
	    ax = ((tmptext[i] - 33) * 6) + 1;
	    ay = 10;
	}
	/* Space */
	if (tmptext[i] == 0x20)
	    ax = 79;
	/* Draw Text */
	dockapp_copyarea(fonts, pixmap, ax, ay, 6, 8, dx, dy);
	dx += 6;
    }

}

static void draw_junk(struct wifi *wfi)
{
    int num = wfi->link;
    int percent;
    int y1 = 5;
    char tmp[255];

    if (lwfi.link != wfi->link) {

	if (wfi->max_qual > -1) {
	    if (num < 0)
		num = 0;

	    /* Calculate Link percentage */
	    percent = num / (wfi->max_qual / 100);
	    num = percent;

	    /* draw digits */

	    sprintf(tmp, "%d", num);
	    draw_text(tmp, 6, 47, True);

	    dockapp_copyarea(display_link, pixmap, 10, 21, 15, 24, 10, 21);
	    dockapp_copyarea(display_link, pixmap, 37, 16, 18, 30, 37, 16);

	    if (num > 0) {
		/* Level bar 1 ( > 1 - 20%) */
		dockapp_copyarea(parts, pixmap, 70, 27, 2, 2, 37, 44);
	    }
	    if (num > 20) {
		/* Level bar 2 (18.4 - 40%) */
		dockapp_copyarea(parts, pixmap, 70, 27, 2, 2, 41, 44);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 41, 37);
		/* Antenna */
		dockapp_copyarea(parts, pixmap, 2, y1, 15, 24, 10, 21);
	    }
	    if (num > 40) {
		/* Level bar 3 (36.8 - 60%) */
		dockapp_copyarea(parts, pixmap, 70, 27, 2, 2, 45, 44);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 45, 37);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 45, 30);
		/* Antenna */
		dockapp_copyarea(parts, pixmap, 18, y1, 15, 24, 10, 21);
	    }
	    if (num > 60) {
		dockapp_copyarea(parts, pixmap, 70, 27, 2, 2, 49, 44);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 49, 37);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 49, 30);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 49, 23);
		/* Antenna */
		dockapp_copyarea(parts, pixmap, 34, y1, 15, 24, 10, 21);
	    }
	    if (num > 80) {
		/* Level bar 5 (73.6 - 100%) */
		dockapp_copyarea(parts, pixmap, 70, 27, 2, 2, 53, 44);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 53, 37);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 53, 30);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 53, 23);
		dockapp_copyarea(parts, pixmap, 74, 22, 2, 7, 53, 16);
		/* Antenna */
		dockapp_copyarea(parts, pixmap, 50, y1, 15, 24, 10, 21);
	    }
	}
    }

}
static void copy_wifi(struct wifi *lwfi, struct wifi *wfi)
{
    memcpy(lwfi->ifname, wfi->ifname, 255);
    memcpy(lwfi->essid, wfi->essid, IW_ESSID_MAX_SIZE + 1);
    lwfi->ifnum = wfi->ifnum;
    lwfi->link = wfi->link;
    lwfi->level = wfi->level;
    lwfi->noise = wfi->noise;
    lwfi->bitrate.value = wfi->bitrate.value;
}
static void clear_wifi(struct wifi *wfi)
{
    //memcpy(wfi->ifname, 0, 255);
    //memcpy(wfi->essid, 0, IW_ESSID_MAX_SIZE + 1);
    wfi->ifnum = 0;
    wfi->link = (float) 0;
    wfi->level = 0;
    wfi->noise = 0 - 0x100;
    wfi->bitrate.value = 0;
}
static void parse_arguments(int argc, char **argv)
{
    int i;
    int integer;
    //dockapp_isbrokenwm = True;

    /* Default to Classic Theme */
    tcur.bg = "rgb:8e/96/8a";
    tcur.hi = "rgb:76/7c/6f";
    tcur.mt = "rgb:44/44/44";
    tcur.fg = "rgb:00/00/00";
    blgt.bg = "rgb:6E/C6/3B";
    blgt.fg = "rgb:00/00/00";
    blgt.mt = "rgb:00/00/00";
    blgt.hi = "rgb:6c/b2/37";


    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
	    print_help(argv[0]), exit(0);
	else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v"))
	    printf("%s version %s\n", PACKAGE, VERSION), exit(0);
	else if (!strcmp(argv[i], "--display") || !strcmp(argv[i], "-d")) {
	    display_name = argv[i + 1];
	    i++;
	} else if (!strcmp(argv[i], "--backlight")
		   || !strcmp(argv[i], "-bl")) {
	    backlight = LIGHTON;
	} else if (!strcmp(argv[i], "--light-color")
		   || !strcmp(argv[i], "-lc")) {
	    light_color = argv[i + 1];
	    blgt.bg = argv[i + 1];
	    i++;
	} else if (!strcmp(argv[i], "--interval")
		   || !strcmp(argv[i], "-i")) {
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
	} else if (!strcmp(argv[i], "--learn")
		   || !strcmp(argv[i], "-l")) {
	    wmwifi_learn = True;
	} else if (!strcmp(argv[i], "--scroll-on")
		   || !strcmp(argv[i], "+s")) {
	    scroll = True;
	} else if (!strcmp(argv[i], "--scroll-off")
		   || !strcmp(argv[i], "-s")) {
	    scroll = False;
	} else if (!strcmp(argv[i], "--windowed")
		   || !strcmp(argv[i], "-w")) {
	    dockapp_iswindowed = True;
	} else if (!strcmp(argv[i], "--theme-classic")
		   || !strcmp(argv[i], "-tc")) {
	    tcur.bg = "rgb:8e/96/8a";
	    tcur.hi = "rgb:76/7c/6f";
	    tcur.mt = "rgb:44/44/44";
	    tcur.fg = "rgb:00/00/00";
	} else if (!strcmp(argv[i], "--theme-snow")
		   || !strcmp(argv[i], "-ts")) {
	    tcur.hi = "rgb:cc/cc/cc";
	    tcur.mt = "rgb:aa/aa/aa";
	    tcur.bg = "rgb:ff/ff/ff";
	    tcur.fg = "rgb:00/00/00";
	} else if (!strcmp(argv[i], "--theme-new")
		   || !strcmp(argv[i], "-tn")) {
	    tcur.hi = "rgb:00/49/41";
	    tcur.mt = "rgb:02/7e/72";
	    tcur.bg = "rgb:20/20/20";
	    tcur.fg = "rgb:20/b2/ae";
	} else if (!strcmp(argv[i], "--hilight-color")
		   || !strcmp(argv[i], "-hi")) {
	    tcur.hi = argv[i + 1];
	    i++;
	} else if (!strcmp(argv[i], "--midtone-color")
		   || !strcmp(argv[i], "-mt")) {
	    tcur.mt = argv[i + 1];
	    i++;
	} else if (!strcmp(argv[i], "--background-color")
		   || !strcmp(argv[i], "-bg")) {
	    tcur.bg = argv[i + 1];
	    i++;
	} else if (!strcmp(argv[i], "--foreground-color")
		   || !strcmp(argv[i], "-fg")) {
	    tcur.fg = argv[i + 1];
	    i++;
	} else if (!strcmp(argv[i], "--broken-wm")
		   || !strcmp(argv[i], "-bw"))
	    dockapp_isbrokenwm = True;
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
    printf("wmwifi - Wireless signal strength monitor dockapp\n");
    printf("  -d,  --display <string>        display to use\n");
    printf("  -bl, --backlight               turn on back-light\n");
    printf
	("  -lc, --light-color <string>    back-light color(rgb:6E/C6/3B is default)\n");
    printf
	("  -fg, --foreground-color <string>    foreground color(rgb:00/00/00 is default)\n");
    printf
	("  -bg, --background-color <string>    background color(rgb:8e/96/8a is default)\n");
    printf
	("  -hc, --hilight-color <string>    background highlight color(rgb:76/7c/6f is default)\n");
    printf
	("  -mt, --midtone-color <string>    midtone highlight color(rgb:44/44/44 is default)\n");
    printf
	("  -i,  --interval <number>       number of secs between updates (1 is default)\n");
    printf
	("  -h,  --help                    show this help text and exit\n");
    printf
	("  -v,  --version                 show program version and exit\n");
    printf
	("  -w,  --windowed                run the application in windowed mode\n");
    printf
	("  -bw, --broken-wm               activate broken window manager fix\n");
    printf
	("  -l, --learn                    enter learning mode (may break quality)\n");
    printf
	("  +s, --scroll-on                scroll interface name and essid\n");
    printf
	("  -s, --scroll-off               use <= 0.5 style interface and essid\n");
    printf
	("  -tc, --theme-classic           Theme: use <= 0.5 style interface theme\n");
    printf("  -ts, --theme-snow              Theme: Snow\n");
    printf("  -tn, --theme-new               Theme: New\n");
}
