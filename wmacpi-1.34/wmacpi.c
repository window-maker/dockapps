/* apm/acpi dockapp - phear it 1.34
 * Copyright (C) 2000, 2001, 2002 timecop@japan.co.jp
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

/* #define RETARDED_APM */
/* #define STUPID_APM */
/* see README if you need to #define these or not. No user serviceable
 * parts below */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/xpm.h>

#include "wmacpi.h"

#if defined(ACPI) && defined(APM)
# error Cannot compile with ACPI and APM compiled in.  Please select only one.
#endif

/* main pixmap */
#ifdef LOW_COLOR
#include "master_low.xpm"
#else
#include "master.xpm"
#endif

typedef struct {
    Display *display;		/* X11 display struct */
    int screen;			/* current screen */
    Window root;		/* root window */
    Window win;			/* one window */
    Window iconwin;		/* another one */
    Pixmap pixmap;		/* UI pixmap, window pixmap */
    Pixmap mask;		/* mask pixmap for shape */
    GC gc;			/* main drawing GC */
    Pixmap text;		/* pixmap for text scroller */
    int tw;			/* text width inside text pixmap */
    int update;			/* need to redraw? */
    int pressed;		/* is the button pressed? */
    DspMode dspmode;		/* time remaining or battery timer */
    Mode blink;			/* should we blink the LED? (critical battery) */
} Dockapp;

/* for debug printing */
#ifdef PRO
char *state[] = { "AC", "Charging", "High", "Low", "Crit" };
#endif

/* globals */
Dockapp *dockapp;
APMInfo *apminfo;
int count = 0;			/* global timer variable */
int noisy_critical = 0;		/* ring xbell annoyingly if critical? */

/* proto for local stuff */
static void new_window(char *name);
static int open_display(char *display);
static void redraw_window(void);
static void render_text(char *string);
static void scroll_text(int x, int y, int width, int tw, int reset);
static void display_percentage(int percent);
static void display_state(void);
static void display_time(int minutes);
static void blink_button(Mode mode);

#define copy_xpm_area(x, y, w, h, dx, dy)				\
{									\
    XCopyArea(dockapp->display, dockapp->pixmap, dockapp->pixmap,	\
	    dockapp->gc, x, y, w, h, dx, dy);				\
    dockapp->update = 1;						\
}

static void redraw_window(void)
{
    if (dockapp->update) {
	eprint(1, "redrawing window");
	XCopyArea(dockapp->display, dockapp->pixmap, dockapp->iconwin,
		  dockapp->gc, 0, 0, 64, 64, 0, 0);
	XCopyArea(dockapp->display, dockapp->pixmap, dockapp->win,
		  dockapp->gc, 0, 0, 64, 64, 0, 0);
	dockapp->update = 0;
    }
}

static void new_window(char *name)
{
    XpmAttributes attr;
    Pixel fg, bg;
    XGCValues gcval;
    XSizeHints sizehints;
    XClassHint classhint;
    XWMHints wmhints;

    dockapp->screen = DefaultScreen(dockapp->display);
    dockapp->root = DefaultRootWindow(dockapp->display);

    sizehints.flags = USSize;
    sizehints.width = 64;
    sizehints.height = 64;

    fg = BlackPixel(dockapp->display, dockapp->screen);
    bg = WhitePixel(dockapp->display, dockapp->screen);

    dockapp->win = XCreateSimpleWindow(dockapp->display, dockapp->root,
				       0, 0, sizehints.width,
				       sizehints.height, 1, fg, bg);
    dockapp->iconwin =
	XCreateSimpleWindow(dockapp->display, dockapp->win, 0, 0,
			    sizehints.width, sizehints.height, 1, fg, bg);

    XSetWMNormalHints(dockapp->display, dockapp->win, &sizehints);
    classhint.res_name = name;
    classhint.res_class = name;
    XSetClassHint(dockapp->display, dockapp->win, &classhint);

    XSelectInput(dockapp->display, dockapp->win,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask |
		 StructureNotifyMask);
    XSelectInput(dockapp->display, dockapp->iconwin,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask |
		 StructureNotifyMask);

    XStoreName(dockapp->display, dockapp->win, name);
    XSetIconName(dockapp->display, dockapp->win, name);

    gcval.foreground = fg;
    gcval.background = bg;
    gcval.graphics_exposures = False;

    dockapp->gc =
	XCreateGC(dockapp->display, dockapp->win,
		  GCForeground | GCBackground | GCGraphicsExposures,
		  &gcval);

    attr.exactColors = 0;
    attr.alloc_close_colors = 1;
    attr.closeness = 1L << 15;
    attr.valuemask = XpmExactColors | XpmAllocCloseColors | XpmCloseness;
    if (XpmCreatePixmapFromData(dockapp->display, dockapp->win,
				master_xpm, &dockapp->pixmap,
				&dockapp->mask, &attr) != XpmSuccess) {
	fprintf(stderr, "FATAL: Not enough colors for main pixmap!\n");
	exit(1);
    }

    /* text area is 318x7, or 53 characters long */
    dockapp->text = XCreatePixmap(dockapp->display, dockapp->win, 318, 7,
				  DefaultDepth(dockapp->display,
					       dockapp->screen));
    if (!dockapp->text) {
	fprintf(stderr, "FATAL: Cannot create text scroll pixmap!\n");
	exit(1);
    }

    XShapeCombineMask(dockapp->display, dockapp->win, ShapeBounding, 0, 0,
		      dockapp->mask, ShapeSet);
    XShapeCombineMask(dockapp->display, dockapp->iconwin, ShapeBounding, 0,
		      0, dockapp->mask, ShapeSet);

    wmhints.initial_state = WithdrawnState;
    wmhints.flags = StateHint;
    wmhints.icon_window = dockapp->iconwin;
    wmhints.icon_x = sizehints.x;
    wmhints.icon_y = sizehints.y;
    wmhints.window_group = dockapp->win;
    wmhints.flags =
	StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
    XSetWMHints(dockapp->display, dockapp->win, &wmhints);

    XMapWindow(dockapp->display, dockapp->win);
}

static void render_text(char *string)
{
    int i, c, k;

    if (strlen(string) > 53)
	return;

    eprint(1, "rendering: %s", string);

    /* prepare the text area by clearing it */
    for (i = 0; i < 54; i++) {
	XCopyArea(dockapp->display, dockapp->pixmap, dockapp->text,
		  dockapp->gc, 133, 57, 6, 8, i * 6, 0);
    }
    k = 0;

    for (i = 0; string[i]; i++) {
	c = toupper(string[i]);
	if (c >= 'A' && c <= 'Z') {	/* letter */
	    c = c - 'A';
	    XCopyArea(dockapp->display, dockapp->pixmap, dockapp->text,
		      dockapp->gc, c * 6, 67, 6, 7, k, 0);
	} else if (c >= '0' && c <= '9') {	/* number */
	    c = c - '0';
	    XCopyArea(dockapp->display, dockapp->pixmap, dockapp->text,
		      dockapp->gc, c * 6 + 66, 58, 6, 7, k, 0);
	} else if (c == '.') {
	    XCopyArea(dockapp->display, dockapp->pixmap, dockapp->text,
		      dockapp->gc, 140, 58, 6, 7, k, 0);
	} else if (c == '-') {
	    XCopyArea(dockapp->display, dockapp->pixmap, dockapp->text,
		      dockapp->gc, 126, 58, 6, 7, k, 0);
	}
	k += 6;
    }
    dockapp->tw = k;		/* length of text segment */
    /* re-scroll the message */
    scroll_text(6, 50, 52, dockapp->tw, 1);
    /* reset the scroll repeat counter */
    count = 0;
}

static int open_display(char *display)
{
    dockapp->display = XOpenDisplay(display);
    if (!dockapp->display) {
	fprintf(stderr, "Unable to open display '%s'\n", display);
	return 1;
    }
    return 0;
}

static void scroll_text(int x, int y, int width, int tw, int reset)
{
    static int pos, first, stop;

    if (reset) {
	pos = 0;
	first = 0;
	stop = 0;
	XCopyArea(dockapp->display, dockapp->pixmap, dockapp->text,
		  dockapp->gc, 0, 0, width, 7, x, y);
    }

    if (stop) {
	return;
    }

    if ((first == 0) && pos == 0) {
	pos = width;
	first = 1;
    }

    if (pos == (0 - tw - 2)) {
	first = 1;
	pos = width;
	stop = 1;
	return;
    }
    pos -= 2;

    eprint(0, "scrolling");

    if (pos > 0) {
	copy_xpm_area(66, 9, pos, 7, x, y);	/* clear */
	XCopyArea(dockapp->display, dockapp->text, dockapp->pixmap,
		  dockapp->gc, 0, 0, width - pos, 7, x + pos, y);
    } else {			/* don't need to clear, already in text */
	XCopyArea(dockapp->display, dockapp->text, dockapp->pixmap,
		  dockapp->gc, abs(pos), 0, width, 7, x, y);
    }
    dockapp->update = 1;
}

static void display_percentage(int percent)
{
    static int op = -1, obar;
    unsigned int bar;

    eprint(1, "received: %d\n", percent);

    if (op == percent)
	return;

    if (percent < 0)
	percent = 0;
    if (percent > 100)
	percent = 100;

    if (percent < 100) {	/* 0 - 99 */
	copy_xpm_area(95, 48, 8, 7, 37, 17);
	if (percent >= 10)
	    copy_xpm_area((percent / 10) * 6 + 67, 28, 5, 7, 40, 17);
	copy_xpm_area((percent % 10) * 6 + 67, 28, 5, 7, 46, 17);
    } else
	copy_xpm_area(95, 37, 21, 9, 37, 16);	/* 100% */
    op = percent;

    bar = percent / 1.8518;

    if (bar == obar)
	return;

    copy_xpm_area(66, 0, bar, 8, 5, 5);
    if (bar < 54)
	copy_xpm_area(66 + bar, 18, 54 - bar, 8, bar + 5, 5);
    obar = bar;
}

static void display_time(int minutes)
{
    static int ohour = -1, omin = -1;
    static int counter;
    int hour, min, tmp;

    if (minutes == -1) {	/* error - blink 00:00 */
	counter++;
	if (counter == 5) {
	    copy_xpm_area(80, 76, 31, 11, 7, 32);
	} else if (counter == 10) {
	    copy_xpm_area(114, 76, 31, 11, 7, 32);
	}
	if (counter > 10)
	    counter = 0;
	ohour = omin = -1;
	return;
    }

    /* render time on the display */
    hour = minutes / 60;
    min = minutes % 60;

    if (hour == ohour && min == omin)
	return;

    eprint(0, "redrawing time");
    tmp = hour / 10;
    copy_xpm_area(tmp * 7 + 1, 76, 6, 11, 7, 32);
    tmp = hour % 10;
    copy_xpm_area(tmp * 7 + 1, 76, 6, 11, 14, 32);
    tmp = min / 10;
    copy_xpm_area(tmp * 7 + 1, 76, 6, 11, 25, 32);
    tmp = min % 10;
    copy_xpm_area(tmp * 7 + 1, 76, 6, 11, 32, 32);
    copy_xpm_area(71, 76, 3, 11, 21, 32);
    ohour = hour;
    omin = min;
}

static void display_state(void)
{
    static int dopower;
    static int docharging;
    static int dobattery;
    static int docritical;
    static int counter;
    
    switch (apminfo->power) {
    case POWER:
	eprint(0, "selected ac power case");
	if (!dopower) {
	    dopower = 1;
	    docharging = 0;
	    dobattery = 0;
	    dockapp->blink = OFF;
	    copy_xpm_area(67, 38, 12, 7, 6, 17);
	    copy_xpm_area(82, 48, 11, 7, 20, 17);
	    render_text("On AC power");
	}
	break;
    case CHARGING:
	eprint(0, "selected charging case");
	counter++;
	if (counter == 10) {
	    copy_xpm_area(67, 38, 12, 7, 6, 17);
	} else if (counter == 20) {
	    copy_xpm_area(67, 48, 12, 7, 6, 17);
	}
	if (counter > 20)
	    counter = 0;
	if (!docharging) {
	    render_text("Battery is charging");
	    /* get rid of battery symbol */
	    copy_xpm_area(82, 48, 12, 7, 20, 17);
	    /* housekeeping */
	    dockapp->blink = OFF;
	    docharging = 1;
	    dopower = 0;
	    dobattery = 0;
	}
	break;
    case HIGH:
    case LOW:
    case CRIT:
	eprint(0, "selected battery case");
	if (!dobattery) {
	    render_text("On Battery");
	    /* display battery symbol */
	    copy_xpm_area(82, 38, 12, 7, 20, 17);
	    /* get rid of AC power symbol */
	    copy_xpm_area(67, 48, 12, 7, 6, 17);
	    dobattery = 1;
	    dopower = 0;
	    docharging = 0;
	}
	if (apminfo->power == CRIT) {
	    dockapp->blink = BLINK;
	    if (!docritical) {
		render_text("Battery Critical Low");
		docritical = 1;
	    }
	} else {
	    if (docritical) {
		render_text("On Battery");
		docritical = 0;
	    }
	    dockapp->blink = OFF;
	}
	break;
    }
}

static void blink_button(Mode mode)
{
    static int counter;
    static int clear;

    if ((mode == OFF) && !clear) {
	eprint(0, "we are off");
	copy_xpm_area(136, 38, 3, 3, 44, 30);
	clear = 1;
	return;
    }
    if (mode != BLINK)
	return;

    counter++;

    if (counter == 5) {
	copy_xpm_area(137, 33, 3, 3, 44, 30);
	clear = 0;
    } else if (counter == 10) {
	copy_xpm_area(136, 38, 3, 3, 44, 30);
	clear = 0;
	/* make some noise */
	if (noisy_critical)
	    XBell(dockapp->display, 100);
    }
    if (counter > 10)
	counter = 0;
}

int main(int argc, char **argv)
{
    char *display = NULL;
    char ch;
    int update = 0;

    dockapp = calloc(1, sizeof(Dockapp));
    apminfo = calloc(1, sizeof(APMInfo));

    dockapp->blink = OFF;
    apminfo->crit_level = 10;

    /* see if whatever we want to use is supported */
    if (power_init()) {
	/* power_init functions handle printing error messages */
	exit(1);
    }

    /* parse command-line options */
    while ((ch = getopt(argc, argv, "bd:c:h")) != EOF) {
	switch (ch) {
	case 'b':
	    noisy_critical = 1;
	    break;
	case 'c':
	    if (optarg) {
		apminfo->crit_level = atoi(optarg);
		if ((apminfo->crit_level < 0) || (apminfo->crit_level > 100)) {
		    fprintf(stderr, "Please use values between 0 and 100%%\n");
		    apminfo->crit_level = 10;
		    fprintf(stderr, "Using default value of 10%%\n");
		}
	    }
	    break;
	case 'd':
	    if (optarg)
		display = strdup(optarg);
	    break;
	case 'h':
	    printf("wmacpi - help\t\t[timecop@japan.co.jp]\n\n"
		   "-d display\t\tdisplay on remote display <display>\n"
		   "-b\t\t\tmake noise when battery is critical low (beep)\n"
		   "-c value\t\tset critical low alarm at <value> percent\n"
		   "\t\t\t(default: 10 percent)\n"
		   "-h\t\t\tdisplay this help\n");
	    return 0;
	    break;
	}
	
    }

    /* open local or command-line specified display */
    if (open_display(display))
	exit(1);

    /* make new dockapp window */
    new_window("apm");

    /* get initial statistics */
    acquire_info();

    dockapp->dspmode = REMAIN;

    /* main loop */
    while (1) {
	XEvent event;
	while (XPending(dockapp->display)) {
	    eprint(0, "X11 activity");
	    XNextEvent(dockapp->display, &event);
	    switch (event.type) {
	    case Expose:
		/* update */
		dockapp->update = 1;
		while (XCheckTypedEvent(dockapp->display, Expose, &event));
		redraw_window();
		break;
	    case DestroyNotify:
		XCloseDisplay(dockapp->display);
		exit(0);
		break;
	    case ButtonPress:
		/* press event */
		if (event.xbutton.x >= 44 && event.xbutton.x <= 57 &&
		    event.xbutton.y >= 30 && event.xbutton.y <= 43) {
		    eprint(0, "inside button!");
		    dockapp->pressed = 1;
		    copy_xpm_area(118, 38, 15, 15, 44, 30);
		}
		break;
	    case ButtonRelease:
		/* release event */
		if (event.xbutton.x >= 44 && event.xbutton.x <= 57 &&
		    event.xbutton.y >= 30 && event.xbutton.y <= 43 &&
		    dockapp->pressed) {
		    /* handle button press */
		    eprint(0, "release still inside button!");
		    dockapp->pressed = 0;
		    copy_xpm_area(136, 38, 15, 15, 44, 30);
		    if ((apminfo->power != POWER) && (apminfo->power != CHARGING)) {
			dockapp->dspmode = !dockapp->dspmode;
			eprint(1, "Mode: %d", dockapp->dspmode);
		    }
		    /* end button press handler */
		}
		if (dockapp->pressed) {
		    copy_xpm_area(136, 38, 15, 15, 44, 30);
		    dockapp->pressed = 0;
		}
		break;
	    }
	}

	if (update++ == 30) {
	    eprint(1, "polling apm");
	    acquire_info();
	    update = 0;
	}

	if (count++ == 256) {
	    scroll_text(6, 50, 52, dockapp->tw, 1);
	    count = 0;
	}

	/* it's okay to test here because display_time will not draw anything
	 * unless there is a change.  Also if we switched power states from
	 * battery to charging/etc, we need to exit from "timer" mode */
	if (dockapp->dspmode == REMAIN || apminfo->power == POWER || apminfo->power == CHARGING) {
	    display_time(apminfo->rtime);
	} else {
	    display_time((time(NULL) - apminfo->timer) / 60);
	}

	display_state();
	blink_button(dockapp->blink);
	display_percentage(apminfo->percentage);
	scroll_text(6, 50, 52, dockapp->tw, 0);

	/* redraw_window, if anything changed - determined inside 
	 * redraw_window. */
	redraw_window();
	usleep(100000);
    }
    return 0;
}

/* this handles enabling "on-battery" timer.  It only needs to happen once
 * for each unplug event.  Functions from libapm and libacpi call this to
 * start the timer */
void process_plugin_timer(void)
{
    static int timer;

    if ((apminfo->power != POWER) && (apminfo->power != CHARGING) && !timer) {
	eprint(1, "not AC and not charging, and timer is not started");
	eprint(1, "starting battery timer");
	apminfo->timer = time(NULL);
	timer = 1;
    }
    if (((apminfo->power == POWER) || (apminfo->power == CHARGING)) && timer) {
	eprint(1, "disabling battery timer");
	timer = 0;
    }

}
