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

#include <dockapp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/xpm.h>

#include "libacpi.h"
#include "wmacpi.h"

#define WMACPI_VER "2.2rc1"

/* main pixmap */
#ifdef LOW_COLOR
#include "master_low.xpm"
static char **master_xpm = master_low_xpm;
#else
#include "master.xpm"
#endif

struct dockapp {
    Display *display;		/* display */
    Window win;			/* main window */
    Pixmap pixmap;		/* main pixmap */
    Pixmap mask;		/* mask pixmap */
    Pixmap text;		/* pixmap for text scroller */
    unsigned short width;	/* width of pixmap */
    unsigned short height;	/* height of pixmap */
    int screen;			/* current screen */
    int tw;			/* text width inside text pixmap */
    int update;			/* need to redraw? */
    int blink;			/* should we blink the LED? (critical battery) */
    int bell;			/* bell on critical low, or not? */
    int scroll;			/* scroll message text? */
    int scroll_reset;		/* reset the scrolling text */
};

/* globals */
struct dockapp *dockapp;
/* global_t *globals; */

/* this gives us a variable scroll rate, depending on the importance of the
 * message . . . */
#define DEFAULT_SCROLL_RESET 150;
int scroll_reset = DEFAULT_SCROLL_RESET;

/* copy a chunk of pixmap around the app */
static void copy_xpm_area(int x, int y, int w, int h, int dx, int dy)					
{										
    XCopyArea(DADisplay, dockapp->pixmap, dockapp->pixmap,	
	      DAGC, x, y, w, h, dx, dy);			
    dockapp->update = 1;							
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

static void reset_scroll(void) {
    dockapp->scroll_reset = 1;
}

static void clear_text_area(void) {
    copy_xpm_area(66, 9, 52, 7, 6, 50);
}

static void redraw_window(void)
{
    if (dockapp->update) {
	XCopyArea(dockapp->display, dockapp->pixmap, dockapp->win,
		  DAGC, 0, 0, 64, 64, 0, 0);
	dockapp->update = 0;
    }
}

static void new_window(char *display, char *name, int argc, char **argv)
{
    XSizeHints *hints;

    /* Initialise the dockapp window and appicon */
    DAOpenDisplay(display, argc, argv);
    DACreateIcon(name, 64, 64, argc, argv);
    dockapp->display = DADisplay;
    dockapp->win = DAWindow;

    XSelectInput(dockapp->display, dockapp->win,
                 ExposureMask | ButtonPressMask | ButtonReleaseMask |
                 StructureNotifyMask);

    /* create the main pixmap . . . */
    DAMakePixmapFromData(master_xpm, &dockapp->pixmap, &dockapp->mask, 
			 &dockapp->width, &dockapp->height);
    DASetPixmap(dockapp->pixmap);
    DASetShape(dockapp->mask);

    /* text area is 318x7, or 53 characters long */
    dockapp->text = XCreatePixmap(dockapp->display, dockapp->win, 318, 7,
				  DefaultDepth(dockapp->display,
					       dockapp->screen));
    if (!dockapp->text) {
	pfatal("FATAL: Cannot create text scroll pixmap!\n");
	exit(1);
    }

    /* force the window to stay this size - otherwise the user could
     * resize us and see our panties^Wmaster pixmap . . . */
    hints = XAllocSizeHints();
    if(hints) {
	hints->flags |= PMinSize | PMaxSize;
	hints->min_width = 64;
	hints->max_width = 64;
	hints->min_height = 64;
	hints->max_height = 64;
	XSetWMNormalHints(dockapp->display, dockapp->win, hints);
	XFree(hints);
    } 

    DAShow();
}

static void copy_to_text_buffer(int sx, int sy, int w, int h, int dx, int dy)
{
    XCopyArea(dockapp->display, dockapp->pixmap, dockapp->text,
	      DAGC, sx, sy, w, h, dx, dy);
}

static void copy_to_text_area(int sx, int sy, int w, int h, int dx, int dy)
{
    XCopyArea(dockapp->display, dockapp->text, dockapp->pixmap,
	      DAGC, sx, sy, w, h, dx, dy);
}

static void scroll_text(void)
{
    static int start, end, stop;
    int x = 6;			/* x coord of the start of the text area */
    int y = 50;			/* y coord */
    int width = 52;		/* width of the text area */
    int height = 7;		/* height of the text area */
    int tw = dockapp->tw;	/* width of the rendered text */
    int sx, dx, w;

    if (!dockapp->scroll) 
	return;

    /* 
     * Conceptually this is viewing the text through a scrolling
     * window - the window starts out with the end immediately before
     * the text, and stops when the start of the window is immediately
     * after the end of the text.
     *
     * We begin with the start of the window at pixel (0 - width) and
     * as we scroll we render only the portion of the window above
     * pixel 0. The destination of the copy during this period starts
     * out at the end of the text area and works left as more of the
     * text is being copied, until a full window is being copied.
     *
     * As the end of the window moves out past the end of the text, we
     * want to keep the destination at the beginning of the text area, 
     * but copy a smaller and smaller chunk of the text. Eventually the
     * start of the window will scroll past the end of the text, at 
     * which point we stop doing any work and wait to be reset.
     */

    if (dockapp->scroll_reset) {
	start = 0 - width;
	end = 0;
	stop = 0;
	clear_text_area();
	dockapp->scroll_reset = 0;
    }

    if (stop)
	return;

    w = 52;
    if (end < 52)
	w = end;
    else if (end > tw)
	w = 52 - (end - tw);
	
    dx = x + 52 - w;
    if (end > tw)
	dx = x;

    sx = start;
    if (start < 0)
	sx = 0;

    if (start > tw)
	stop = 1;

    clear_text_area();
    copy_to_text_area(sx, 0, w, height, dx, y);
    start += 2;
    end += 2;

    dockapp->update = 1;
}

static void render_text(char *string)
{
    int i, c, k;

    /* drop out immediately if scrolling is disabled - we don't render
     * any text at all, since there's not much else we could do
     * sensibly without scrolling. */
    if (!dockapp->scroll)
	return;

    if (strlen(string) > 53)
	return;

    /* prepare the text area by clearing it */
    for (i = 0; i < 54; i++) {
	copy_to_text_buffer(133, 57, 6, 8, i * 6, 0);
    }
    k = 0;

    for (i = 0; string[i]; i++) {
	c = toupper(string[i]);
	if (c >= 'A' && c <= 'Z') {	/* letter */
	    c = c - 'A';
	    copy_to_text_buffer(c * 6, 67, 6, 7, k, 0);
	} else if (c >= '0' && c <= '9') {	/* number */
	    c = c - '0';
	    copy_to_text_buffer(c * 6 + 66, 58, 6, 7, k, 0);
	} else if (c == '.') {
	    copy_to_text_buffer(140, 58, 6, 7, k, 0);
	} else if (c == '-') {
	    copy_to_text_buffer(126, 58, 6, 7, k, 0);
	}
	k += 6;
    }
    dockapp->tw = k;		/* length of text segment */
    /* re-scroll the message */
    reset_scroll();
    scroll_text();
}

static void display_percentage(int percent)
{
    static int op = -1;
    static unsigned int obar;
    unsigned int bar;
    int width = 54;		/* width of the bar */
    float ratio = 100.0/width;	/* ratio between the current percentage
				 * and the number of pixels in the bar */

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

    bar = (int)((float)percent / ratio);

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

static void set_power_panel(global_t *globals)
{
    static enum panel_states power = PS_NULL;
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

    if ((binfo->state == CRIT) && (ap->power == BATT))
	blink_battery_glyph();

    if (binfo->state == HARD_CRIT) {
	really_blink_battery_glyph();
	/* we only do this here because it'd be obnoxious to 
	 * do it anywhere else. */
	if (dockapp->bell) {
	    XBell(dockapp->display, 100);
	}
    }
}

void scroll_faster(double factor) {
    scroll_reset = scroll_reset * factor;
}

void scroll_slower(double factor) {
    scroll_reset = scroll_reset * factor;
}

void reset_scroll_speed(void) {
    scroll_reset = DEFAULT_SCROLL_RESET;
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

static void set_message(global_t *globals)
{
    static enum messages state = M_NULL;
    battery_t *binfo = globals->binfo;
    adapter_t *ap = &globals->adapter;
    
    /* battery not present case */
    if (!binfo->present) {
	if (state != M_NP) {
	    state = M_NP;
	    reset_scroll_speed();
	    render_text("not present");
	}
    } else if (ap->power == AC) {
	if (binfo->charge_state == CHARGE) {
	    if (state != M_CH) {
		state = M_CH;
		reset_scroll_speed();
		render_text("battery charging");
	    }
	} else {
	    if (state != M_AC) {
		state = M_AC;
		reset_scroll_speed();
		render_text("on ac power");
	    }
	}
    } else {
	if (binfo->state == CRIT) {
	    if (state != M_CB) {
		state = M_CB;
		scroll_faster(0.75);
		render_text("critical low battery");
	    }
	} else if (binfo->state == HARD_CRIT) {
	    if (state != M_HCB) {
		state = M_HCB;
		scroll_faster(0.5);
		render_text("hard critical low battery");
	    }
	} else if (binfo->state == LOW) {
	    if (state != M_LB) {
		state = M_LB;
		scroll_faster(0.85);
		render_text("low battery");
	    }
	} else {
	    if (state != M_BATT) {
		state = M_BATT;
		reset_scroll_speed();
		render_text("on battery");
	    }
	}
    }    
}

void set_time_display(global_t *globals)
{
    if (globals->binfo->charge_state == CHARGE)
	display_time(globals->binfo->charge_time);
    else if (globals->binfo->charge_state == DISCHARGE)
	display_time(globals->rtime);
    else
	invalid_time_display();
}

void set_batt_id_area(int bno)
{
    int w = 7;			/* Width of the number */
    int h = 11;			/* Height of the number */
    int dx = 50;		/* x coord of the target area */
    int dy = 31;		/* y coord of the target area */
    int sx = (bno + 1) * 7;	/* source x coord */
    int sy = 76;		/* source y coord */
    
    copy_xpm_area(sx, sy, w, h, dx, dy);
}

#define VERSION "wmacpi version " WMACPI_VER "\nUsing libacpi version " LIBACPI_VER

void cli_wmacpi(global_t *globals, int samples)
{
    int i, j, sleep_time = 0;
    battery_t *binfo;
    adapter_t *ap;
    
    pdebug("samples: %d\n", samples);
    if(samples > 1)
    	sleep_time = 1000000/samples;

    /* we want to acquire samples over some period of time, so . . . */
    for(i = 0; i < samples + 2; i++) {
	for(j = 0; j < globals->battery_count; j++)
	    acquire_batt_info(globals, j);
	acquire_global_info(globals);
	usleep(sleep_time);
    }
    
    ap = &globals->adapter;
    if(ap->power == AC) {
	printf("On AC Power");
	for(i = 0; i < globals->battery_count; i++) {
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
	for(i = 0; i < globals->battery_count; i++) {
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
    int sample_count = 0;
    int batt_reinit, ac_reinit;
    int batt_count = 0;
    int ac_count = 0;
    int cli = 0, samples = 1, critical = 10;
    int samplerate = 20;
    int sleep_rate = 10;
    int sleep_time = 1000000/sleep_rate;
    int scroll_count = 0;
    enum rtime_mode rt_mode = RT_RATE;
    int rt_forced = 0;
    battery_t *binfo;
    global_t *globals;

    DAProgramOption options[] = {
     {"-r", "--no-scroll", "disable scrolling message", DONone, False, {NULL}},
     {"-n", "--no-blink", "disable blinking of various UI elements", DONone, False, {NULL}},
     {"-x", "--cmdline", "run in command line mode",  DONone, False, {NULL}}, 
     {"-f", "--force-capacity-mode", "force the use of capacity mode for calculating time remaining", DONone, False, {NULL}},
     {"-d", "--display", "display or remote display", DOString, False, {&display}},
     {"-c", "--critical", "set critical low alarm at <number> percent\n                               (default: 10 percent)", DONatural, False, {&critical}},
     {"-m", "--battery", "battery number to monitor", DONatural, False, {&battery_no}},
     {"-s", "--sample-rate", "number of times per minute to sample battery information\n                               default 20 (once every three seconds)", DONatural, False, {&samplerate}},
     {"-V", "--verbosity", "Set verbosity", DONatural, False, {&verbosity}},
     {"-a", "--samples", "number of samples to average over (cli mode only)",  DONatural, False, {&samples}}, 
    };

    dockapp = calloc(1, sizeof(struct dockapp));
    globals = calloc(1, sizeof(global_t));

    dockapp->blink = 1;
    dockapp->bell = 0;
    dockapp->scroll = 1;
    dockapp->scroll_reset = 0;
    globals->crit_level = 10;
    battery_no = 1;

    /* after this many samples, we reinit the battery and AC adapter 
     * information. 
     * XXX: make these configurable . . . */
    batt_reinit = 100;
    ac_reinit = 1000;

    /* this needs to be up here because we need to know what batteries
     * are available /before/ we can decide if the battery we want to
     * monitor is available. */
    /* parse command-line options */
    DAParseArguments(argc, argv, options, 10, 
      "A battery monitor dockapp for ACPI based systems", 
      VERSION);
		
    if (options[0].used)
        dockapp->scroll = 0;
    if (options[1].used)
        dockapp->blink = 0;
    if (options[2].used)
        cli = 1;
    if (options[3].used) {
        rt_mode = RT_CAP;
        rt_forced = 1;
    }
        
    if (samplerate == 0) samplerate = 1;
    if (samplerate > 600) samplerate = 600;

    if (critical > 100) {
        fprintf(stderr, "Please use values between 0 and 100%%\n");
        fprintf(stderr, "Using default value of 10%%\n");
        critical = 10;
    }
    globals->crit_level = critical;

    if (battery_no >= MAXBATT) {
        fprintf(stderr, "Please specify a battery number below %d\n", MAXBATT);
        return 1;
    }
    pinfo("Monitoring battery %d\n", battery_no);

    if (power_init(globals))
	/* power_init functions handle printing error messages */
	exit(1);

    globals->rt_mode = rt_mode;
    globals->rt_forced = rt_forced;

    if (battery_no > globals->battery_count) {
	pfatal("Battery %d not available for monitoring.\n", battery_no);
	exit(1);
    }

    /* check for cli mode */
    if (cli) {
	cli_wmacpi(globals, samples);
	exit(0);
    }
    /* check to see if we've got a valid DISPLAY env variable, as a simple check to see if
     * we're running under X */
    if (!getenv("DISPLAY")) {
	pdebug("Not running under X - using cli mode\n");
	cli_wmacpi(globals, samples);
	exit(0);
    }

    battery_no--;

    /* make new dockapp window */
    /* Don't even /think/ of asking me why, but if I set the window name to 
     * "acpi", the app refuses to dock properly - it's just plain /weird/.
     * So, wmacpi it is . . . */
    new_window(display, "wmacpi", argc, argv);

    /* get initial statistics */
    acquire_all_info(globals);
    binfo = &batteries[battery_no];
    globals->binfo = binfo;
    pinfo("monitoring battery %s\n", binfo->name);
    clear_time_display();
    set_power_panel(globals);
    set_message(globals);
    set_batt_id_area(battery_no);

    /* main loop */
    while (1) {
	Atom atom;
	Atom wmdelwin;
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
		battery_no = battery_no % globals->battery_count;
		globals->binfo = &batteries[battery_no];
		binfo = globals->binfo;
		pinfo("changing to monitor battery %s\n", binfo->name);
		set_batt_id_area(battery_no);
		dockapp->update = 1;
		break;
	    case ClientMessage:
		/* what /is/ this crap?
		 * Turns out that libdockapp adds the WM_DELETE_WINDOW atom to
		 * the WM_PROTOCOLS property for the window, which means that
		 * rather than get a simple DestroyNotify message, we get a 
		 * nice little message from the WM saying "hey, can you delete
		 * yourself, pretty please?". So, when running as a window 
		 * rather than an icon, we're impossible to kill in a friendly
		 * manner, because we're expecting to die from a DestroyNotify
		 * and thus blithely ignoring the WM knocking on our window
		 * border . . .
		 *
		 * This simply checks for that scenario - it may fail oddly if
		 * something else comes to us via a WM_PROTOCOLS ClientMessage
		 * event, but I suspect it's not going to be an issue. */
		wmdelwin = XInternAtom(dockapp->display, "WM_DELETE_WINDOW", 1);
		atom = event.xclient.data.l[0];
		if (atom == wmdelwin) {
		    XCloseDisplay(dockapp->display);
		    exit(0);
		}
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
	/* Okay, this needs /fixing/ - it's ridiculous. We should be giving
	 * the user the option of saying how many times per minute the 
	 * battery should be sampled, defaulting to 20 times. 
	 * 
	 * We sleep for one tenth of a second at a time, so 60 seconds
	 * translates to 600 sleeps. So, we change the default sample
	 * rate to 20, and the calculation below becomes . . .*/
	if (sample_count++ == ((sleep_rate*60)/samplerate)) {
	    acquire_all_info(globals);

	    /* we need to be able to reinitialise batteries and adapters, because
	     * they change - you can hotplug batteries on most laptops these days
	     * and who knows what kind of shit will be happening soon . . . */
	    if (batt_count++ >= batt_reinit) {
		    if(reinit_batteries(globals)) 
			    pfatal("Oh my god, the batteries are gone!\n");
		    batt_count = 0;
	    }

	    if (ac_count++ >= ac_reinit) {
		    if(reinit_ac_adapters(globals)) 
			    pfatal("What happened to our AC adapters?!?\n");
		    ac_count = 0;
	    }
	    sample_count = 0;
	}

	if (scroll_count++ >= scroll_reset) {
	    reset_scroll();
	    scroll_count = 0;
	}

	/* The old code had some kind of weird crap with timers and the like. 
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
	set_time_display(globals);
	set_power_panel(globals);
	set_message(globals);
	display_percentage(binfo->percentage);
	scroll_text();

	/* redraw_window, if anything changed - determined inside 
	 * redraw_window. */
	redraw_window();

	usleep(sleep_time);
    }
    return 0;
}
