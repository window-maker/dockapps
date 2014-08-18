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

#include "libacpi.h"
#include "wmacpi.h"

#define WMACPI_VER "1.99"

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
    int blink;			/* should we blink the LED? (critical battery) */
} Dockapp;

/* globals */
Dockapp *dockapp;
global_t *globals;
int count = 0;			/* global timer variable */
/* extern int verbosity; */

/* Time for scroll updates */
#define DEFAULT_UPDATE 150
static int update_timeout = DEFAULT_UPDATE;

/* proto for local stuff */
static void new_window(char *name);
static int open_display(char *display);
static void redraw_window(void);
static void render_text(char *string);
static void scroll_text(int x, int y, int width, int tw, int reset);
static void display_percentage(int percent);
static void display_time(int minutes);

#define copy_xpm_area(x, y, w, h, dx, dy)				\
{									\
    XCopyArea(dockapp->display, dockapp->pixmap, dockapp->pixmap,	\
	    dockapp->gc, x, y, w, h, dx, dy);				\
    dockapp->update = 1;						\
}

/* display AC power symbol */
static void display_power_glyph(void)
{
    copy_xpm_area(67, 38, 12, 7, 6, 17);
}

/* get rid of AC power symbol */
static void kill_power_glyph(void)
{
    copy_xpm_area(67, 48, 12, 7, 6, 17);
}

/* display battery symbol */
static void display_battery_glyph(void)
{
    copy_xpm_area(82, 38, 12, 7, 20, 17);
}

/* get rid of battery symbol */
static void kill_battery_glyph(void)
{
    copy_xpm_area(82, 48, 12, 7, 20, 17);
}

/* clear the time display */
static void clear_time_display(void)
{
    copy_xpm_area(114, 76, 31, 11, 7, 32);
}

/* set time display to -- -- */
static void invalid_time_display(void)
{
    copy_xpm_area(122, 13, 31, 11, 7, 32);
}

static void redraw_window(void)
{
    if (dockapp->update) {
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
	pfatal("FATAL: Not enough colors for main pixmap!\n");
	exit(1);
    }

    /* text area is 318x7, or 53 characters long */
    dockapp->text = XCreatePixmap(dockapp->display, dockapp->win, 318, 7,
				  DefaultDepth(dockapp->display,
					       dockapp->screen));
    if (!dockapp->text) {
	pfatal("FATAL: Cannot create text scroll pixmap!\n");
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
	perr("Unable to open display '%s'\n", display);
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
    static int op = -1;
    static unsigned int obar;
    unsigned int bar;

    if (percent == -1)
	percent = 0;

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
    int hour, min, tmp;

    if (minutes <= 0) {	/* error - clear the display */
	invalid_time_display();
	ohour = omin = -1;
	return;
    }

    /* render time on the display */
    hour = minutes / 60;
    /* our display area only fits %2d:%2d, so we need to make sure
     * what we're displaying will fit in those constraints. I don't
     * think we're likely to see any batteries that do more than 
     * 100 hours any time soon, so it's fairly safe. */
    if (hour >= 100) {
	hour = 99;
	min = 59;
    } else
	min = minutes % 60;

    if (hour == ohour && min == omin)
	return;

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

/* 
 * The reworked state handling stuff.
 */

/* set the current state of the power panel */
enum panel_states {
    PS_AC,
    PS_BATT,
    PS_NULL,
};

static void really_blink_power_glyph(void)
{
    static int counter = 0;

    if (counter == 10) 
	display_power_glyph();
    else if (counter == 20) 
	kill_power_glyph();
    else if (counter > 30)
	counter = 0;
    counter++;
}

static void blink_power_glyph(void)
{
    if (dockapp->blink)
	really_blink_power_glyph();
}

static void really_blink_battery_glyph(void)
{
    static int counter = 0;

    if (counter == 10)
	display_battery_glyph();
    else if (counter == 20)
	kill_battery_glyph();
    else if (counter > 30)
	counter = 0;
    counter++;
}    

static void blink_battery_glyph(void)
{
    if (dockapp->blink)
	really_blink_battery_glyph();
}

static void set_power_panel(void)
{
    enum panel_states power = PS_NULL;
    battery_t *binfo = globals->binfo;
    adapter_t *ap = &globals->adapter;

    if (ap->power == AC) {
	if (power != PS_AC) {
	    power = PS_AC;
	    kill_battery_glyph();
	    display_power_glyph();
	}
    } else if (ap->power == BATT) {
	if (power != PS_BATT) {
	    power = PS_BATT;
	    kill_power_glyph();
	    display_battery_glyph();
	}
    }

    if (binfo->charge_state == CHARGE)
	blink_power_glyph();

    if (binfo->state == CRIT)
	blink_battery_glyph();

    if (binfo->state == HARD_CRIT)
	really_blink_battery_glyph();
}

/* 
 * The message that needs to be displayed needs to be decided
 * according to a heirarchy: a message like not present needs to take
 * precedence over a global thing like the current power status, and
 * something like a low battery warning should take precedence over
 * the "on battery" message. Likewise, a battery charging message
 * needs to take precedence over the on ac power message. The other
 * question is how much of a precedence local messages should take
 * over global ones . . . 
 *
 * So, there are three possible sets of messages: not present, on-line
 * and off-line messages. We need to decide which of those sets is
 * appropriate right now, and then decide within them. 
 */
enum messages {
    M_NP,	/* not present */
    M_AC,	/* on ac power */
    M_CH,	/* battery charging */
    M_BATT,	/* on battery */
    M_LB,	/* low battery */
    M_CB,	/* critical low battery */
    M_HCB,	/* battery reported critical capacity state */
    M_NULL,	/* empty starting state */
};

static void set_message(void)
{
    static enum messages state = M_NULL;
    battery_t *binfo = globals->binfo;
    adapter_t *ap = &globals->adapter;
    
    /* battery not present case */
    if (!binfo->present) {
	if (state != M_NP) {
	    state = M_NP;
	    render_text("not present");
	}
    } else if (ap->power == AC) {
	if (binfo->charge_state == CHARGE) {
	    if (state != M_CH) {
		state = M_CH;
		update_timeout = DEFAULT_UPDATE;
		render_text("battery charging");
	    }
	} else {
	    if (state != M_AC) {
		state = M_AC;
		update_timeout = DEFAULT_UPDATE;
		render_text("on ac power");
	    }
	}
    } else {
	if (binfo->state == CRIT) {
	    if (state != M_CB) {
		state = M_CB;
		update_timeout = 80;
		render_text("critical low battery");
	    }
	} else if (binfo->state == HARD_CRIT) {
	    if (state != M_HCB) {
		state = M_HCB;
		update_timeout = 60;
		render_text("hard critical low battery");
	    }
	} else if (binfo->state == LOW) {
	    if (state != M_LB) {
		state = M_LB;
		update_timeout = 100;
		render_text("low battery");
	    }
	} else {
	    if (state != M_BATT) {
		state = M_BATT;
		update_timeout = DEFAULT_UPDATE;
		render_text("on battery");
	    }
	}
    }    
}

void set_time_display(void)
{
    battery_t *binfo = &batteries[battery_no];
    
    if (binfo->charge_state == CHARGE)
	display_time(binfo->charge_time);
    else if (binfo->charge_state == DISCHARGE)
	display_time(globals->rtime);
    else
	invalid_time_display();
}

/*
 * This should really be fixed so that it can handle more than two batteries.
 */

void set_id_1(void)
{
    copy_xpm_area(118, 38, 15, 15, 44, 30);
}    

void set_id_2(void)
{
    copy_xpm_area(136, 38, 15, 15, 44, 30);
}

void set_batt_id_area(int bno)
{
    switch(bno) {
    case 0:
	set_id_1();
	break;
    case 1:
	set_id_2();
	break;
    }
}

void usage(char *name)
{
    printf("%s - help\t\t[simon@dreamcraft.com.au]\n\n"
	   "-d display\t\tdisplay on remote display <display>\n"
	   "-b\t\t\tmake noise when battery is critical low (beep)\n"
	   "-c value\t\tset critical low alarm at <value> percent\n"
	   "\t\t\t(default: 10 percent)\n"
	   "-m <battery number>\tbattery number to monitor\n"
	   "-s <sample rate>\trate at which to sample battery status\n"
	   "\t\t\tdefault 100 (once every three seconds)\n"
	   "-n\t\t\tdo not blink\n"
	   "-w\t\t\trun in command line mode\n"
	   "-a <samples>\t\tsamples to average over (cli mode only)\n"
	   "-v\t\t\tincrease verbosity\n"
	   "\t\t\tcan be used multiple times to increase verbosity further\n"
	   "-h\t\t\tdisplay this help\n",
	   name);
}

void print_version(void)
{
    printf("wmacpi version %s\n", WMACPI_VER);
    printf(" Using libacpi version %s\n", LIBACPI_VER);
}

void cli_wmacpi(int samples)
{
    int i, j, sleep_time;
    battery_t *binfo;
    adapter_t *ap;
    
    sleep_time = 1000000/samples;

    /* we want to acquire samples over some period of time, so . . . */
    for(i = 0; i < samples + 2; i++) {
	for(j = 0; j < batt_count; j++)
	    acquire_batt_info(j);
	acquire_global_info();
	usleep(sleep_time);
    }
    
    ap = &globals->adapter;
    if(ap->power == AC) {
	printf("On AC Power");
	for(i = 0; i < batt_count; i++) {
	    binfo = &batteries[i];
	    if(binfo->present && (binfo->charge_state == CHARGE)) {
		printf("; Battery %s charging", binfo->name);
		printf(", currently at %2d%%", binfo->percentage);
		if(binfo->charge_time >= 0) 
		    printf(", %2d:%02d remaining", 
			   binfo->charge_time/60,
			   binfo->charge_time%60);
	    }
	}
	printf("\n");
    } else if(ap->power == BATT) {
	printf("On Battery");
	for(i = 0; i < batt_count; i++) {
	    binfo = &batteries[i];
	    if(binfo->present && (binfo->percentage >= 0))
		printf(", Battery %s at %d%%", binfo->name,
		       binfo->percentage);
	}
	if(globals->rtime >= 0)
	    printf("; %d:%02d remaining", globals->rtime/60, 
		   globals->rtime%60);
	printf("\n");
    }
    return;
}

int main(int argc, char **argv)
{
    char *display = NULL;
    char ch;
    int update = 0;
    int cli = 0, samples = 1;
    int samplerate = 100;
    battery_t *binfo;

    dockapp = calloc(1, sizeof(Dockapp));
    globals = calloc(1, sizeof(global_t));

    dockapp->blink = 1;
    globals->crit_level = 10;
    battery_no = 1;

    /* parse command-line options */
    while ((ch = getopt(argc, argv, "d:c:m:s:a:hnwvV")) != EOF) {
	switch (ch) {
	case 'c':
	    if (optarg) {
		globals->crit_level = atoi(optarg);
		if ((globals->crit_level < 0) || (globals->crit_level > 100)) {
		    fprintf(stderr, "Please use values between 0 and 100%%\n");
		    globals->crit_level = 10;
		    fprintf(stderr, "Using default value of 10%%\n");
		}
	    }
	    break;
	case 'd':
	    if (optarg)
		display = strdup(optarg);
	    break;
	case 'm':
	    if (optarg) {
		battery_no = atoi(optarg);
		if (battery_no >= MAXBATT) {
		    fprintf(stderr, "Please specify a battery number below %d\n",
			    MAXBATT);
		    return 1;
		}
		if (battery_no > batt_count) {
		    fprintf(stderr, "Battery %d does not appear to be installed\n",
			    battery_no);
		    return 1;
		}
		fprintf(stderr, "Monitoring battery %d\n", battery_no);
	    } 
	    break;
	case 's':
	    if (optarg) {
		samplerate = atoi(optarg);
		if (samplerate == 0) samplerate = 1;
		if (samplerate > 3000) samplerate = 3000;
	    } else {
		usage(argv[0]);
		exit(1);
	    }
	    break;
	case 'h':
	    usage(argv[0]);
	    return 0;
	case 'v':
	    verbosity++;
	    break;
	case 'V':
	    print_version();
	    return 0;
	case 'n':
	    dockapp->blink = 0;
	    break;
	case 'w':
	    cli = 1;
	    break;
	case 'a':
	    if(optarg != NULL) {
		samples = atoi(optarg);
		if(samples > 1000 || samples <= 0) {
		    fprintf(stderr, "Please specify a reasonable number of samples\n");
		    exit(1);
		}
	    }
	    break;
	default:
	    usage(argv[0]);
	    return 1;
	}
	
    }
    
    /* see if whatever we want to use is supported */
    if (power_init())
	/* power_init functions handle printing error messages */
	exit(1);

    /* check for cli mode */
    if (cli) {
	cli_wmacpi(samples);
	exit(0);
    }

    battery_no--;

    /* open local or command-line specified display */
    if (open_display(display))
	exit(1);

    /* make new dockapp window */
    /* Don't even /think/ of asking me why, but if I set the window name to 
     * "acpi", the app refuses to dock properly - it's just plain /weird/ */
/*    new_window("acpi"); */
    new_window("apm");

    /* get initial statistics */
    acquire_all_info();
    binfo = &batteries[battery_no];
    globals->binfo = binfo;
    pinfo("monitoring battery %s\n", binfo->name);
    clear_time_display();
    set_power_panel();
    set_message();
    set_batt_id_area(battery_no);

    /* main loop */
    while (1) {
	XEvent event;
	while (XPending(dockapp->display)) {
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
		break;
	    case ButtonRelease:
		/* cycle through the known batteries. */
		battery_no++;
		battery_no = battery_no % batt_count;
		globals->binfo = &batteries[battery_no];
		binfo = globals->binfo;
		pinfo("changing to monitor battery %d\n", battery_no + 1);
		set_batt_id_area(battery_no);
		break;
	    }
	}

	/* XXX: some laptops have problems with sampling the battery
	 * regularly - apparently, the BIOS disables interrupts while
	 * reading from the battery, which is generally on a slow bus 
	 * and is a slow device, so you get significant periods without
	 * interrupts. This causes interactivity to suffer . . . 
	 * 
	 * My proposed workaround is to allow the user to set the sample
	 * rate - it defaults to ten, but can be set lower (or higher).
	 *
	 * The only problem with this is that we need to sample less 
	 * frequently, while still allowing the app to update normally. 
	 * That means calling redraw_window() and all the set_*() functions
	 * normally, but only calling acquire_all_info() every so often. 
	 * As it stands, we only call acquire_all_info() once every three
	 * seconds (once every thirty updates) . . . I'm not entirely sure
	 * /how/ this could cause interactivity problems, but hey . . . 
	 *
	 * So, given the base rate of once every three seconds, we want to
	 * change this test to . . . */
	if (update++ == (3000/samplerate)) {
	    acquire_all_info();
	    update = 0;
	}

	if (count++ == update_timeout) {
	    scroll_text(6, 50, 52, dockapp->tw, 1);
	    count = 0;
	}

	/* the old code had some kind of weird crap with timers and the like. 
	 * As far as I can tell, it's meaningless - the time we want to display
	 * is the time calculated from the remaining capacity, as per the 
	 * ACPI spec. The only thing I'd change is the handling of a charging
	 * state: my best guess, based on the behaviour I'm seeing with my 
	 * Lifebook, is that the present rate value when charging is the rate
	 * at which the batteries are being charged, which would mean I'd just
	 * need to reverse the rtime calculation to be able to work out how 
	 * much time remained until the batteries were fully charged . . . 
	 * That would be rather useful, though given it would vary rather a lot
	 * it seems likely that it'd be little more than a rough guesstimate. */
	set_time_display();
	set_power_panel();
	set_message();
	display_percentage(binfo->percentage);
	scroll_text(6, 50, 52, dockapp->tw, 0);

	/* redraw_window, if anything changed - determined inside 
	 * redraw_window. */
	redraw_window();
	usleep(100000);
    }
    return 0;
}
