/*
 * Copyright (c) 2007 Daniel Borca  All rights reserved.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "dockapp.h"
#include "sensors.h"
#include "system.h"
#include "util.h"

#include "font.h"
#include "frame0.xpm"
#include "frame1.xpm"
#include "frame2.xpm"
#include "frame3.xpm"
#include "frame4.xpm"
#include "frame5.xpm"


#define DockApp App.d

#define NELEM(tab) (const int)(sizeof(tab) / sizeof((tab)[0]))

#define INSIDE(x, y, xmin, ymin, xmax, ymax) ((x) >= (xmin) && (x) <= (xmax) && (y) >= (ymin) && (y) <= (ymax))

#define SWITCH_FRAME(a, n)			\
    do {					\
	if ((a)->frames[n].v) {			\
	    (a)->page = n;			\
	    (a)->millis = (a)->frames[n].u;	\
	    (a)->handle = handle_frame##n;	\
	    (a)->update = update_frame##n;	\
	    (a)->frames[n].p = 0;		\
	    (a)->update(a, 1);			\
	}					\
    } while (0)

#define SCROLL_START(start, num_items, max_items)	\
    do {						\
	if ((start) > (num_items) - (max_items)) {	\
	    (start) = (num_items) - (max_items);	\
	}						\
	if ((start) < 0) {				\
	    (start) = 0;				\
	}						\
    } while (0)

#define MAX_DISPLAY_CPU  2
#define MAX_DISPLAY_NET  2
#define MAX_DISPLAY_TEMP 5


typedef struct {
	Pixmap f;
	Pixmap m;
	int v;
	int u;
	int p;
} FRAME;

typedef struct {
    int row;
    int height;
    const int *width;
    const int *offset;
} FONT;

typedef struct APP {
    DOCKAPP d;
    FONT fontx9;
    FONT fontx8;
    FONT fontx7;
    FONT fontx6;
    FONT meter2;
    Pixmap font;
    SENSOR *list;
    FRAME *frames;
    void (*handle) (struct APP *a, XEvent *event);
    void (*update) (const struct APP *a, int full);
    int page;
    int millis;
} APP;


static int
draw_text (const APP *a, const FONT *font, const char *str, int dx, int dy)
{
    const DOCKAPP *d = (DOCKAPP *)a;

    int ch;

    while ((ch = *(unsigned char *)str++)) {
	int w = font->width[ch];
	dockapp_copy_area(d, a->font, font->offset[ch], font->row, w, font->height, dx, dy);
	dx += w;
    }

    /*printf("dx = %d\n", dx);*/
    return dx;
}


static void
draw_meter (const APP *a, const FONT *meter, int num, int den, int dx, int dy)
{
    const DOCKAPP *d = (DOCKAPP *)a;

    const int w = meter->width[0];
    const int h = meter->height;
    int col = 1 * w;

    if (num) {
	int ratio = w * num / den;

	XCopyArea(d->display, a->font, a->font, d->gc,
		1 * w, meter->row,
		w,     h,
		2 * w, meter->row);
	XCopyArea(d->display, a->font, a->font, d->gc,
		0 * w, meter->row,
		ratio, h,
		2 * w, meter->row);
	col = 2 * w;
    }
    dockapp_copy_area(d, a->font, col, meter->row, w, h, dx, dy);
}


static void handle_frame0 (APP *a, XEvent *event);
#define handle_frame1 handle_frameN
#define handle_frame2 handle_frameN
#define handle_frame3 handle_frameN
#define handle_frame4 handle_frameN
#define handle_frame5 handle_frameN
static void handle_frameN (APP *a, XEvent *event);
static void update_frame0 (const APP *a, int full);
static void update_frame1 (const APP *a, int full);
static void update_frame2 (const APP *a, int full);
static void update_frame3 (const APP *a, int full);
static void update_frame4 (const APP *a, int full);
static void update_frame5 (const APP *a, int full);


static void
handle_frame0 (APP *a, XEvent *event)
{
    XButtonPressedEvent *bevent;

    switch (event->type) {
	case ButtonPress:
	    bevent = (XButtonPressedEvent *)event;
	    switch (bevent->button & 0xFF) {
		case Button1:
		    /*printf("handle0: b1\n");*/
		    if (INSIDE(bevent->x, bevent->y, 4, 4, 10, 10)) {
			/*printf("next: %d %d\n", bevent->x, bevent->y);*/
			break;
		    }
		    if (INSIDE(bevent->x, bevent->y, 50, 4, 56, 10)) {
			/*printf("quit: %d %d\n", bevent->x, bevent->y);*/
			((DOCKAPP *)a)->quit = True;
			break;
		    }
		    if (INSIDE(bevent->x, bevent->y, 1, 1, 59, 13)) {
			/*printf("time: %d %d\n", bevent->x, bevent->y);*/
			SWITCH_FRAME(a, 1);
			break;
		    }
		    if (INSIDE(bevent->x, bevent->y, 1, 16, 59, 27)) {
			/*printf("cpu : %d %d\n", bevent->x, bevent->y);*/
			SWITCH_FRAME(a, 2);
			break;
		    }
		    if (INSIDE(bevent->x, bevent->y, 1, 30, 29, 41)) {
			/*printf("wifi: %d %d\n", bevent->x, bevent->y);*/
			SWITCH_FRAME(a, 3);
			break;
		    }
		    if (INSIDE(bevent->x, bevent->y, 32, 30, 59, 41)) {
			/*printf("temp: %d %d\n", bevent->x, bevent->y);*/
			SWITCH_FRAME(a, 4);
			break;
		    }
		    if (INSIDE(bevent->x, bevent->y, 1, 44, 59, 59)) {
			/*printf("batt: %d %d\n", bevent->x, bevent->y);*/
			SWITCH_FRAME(a, 5);
			break;
		    }
		    break;
		case Button2:
		    /*printf("handle0: b2\n");*/
		    break;
		case Button3:
		    /*printf("handle0: b3\n");*/
		    break;
	    }
	    break;
    }
}


static void
handle_frameN (APP *a, XEvent *event)
{
    XButtonPressedEvent *bevent;

    switch (event->type) {
	case ButtonPress:
	    bevent = (XButtonPressedEvent *)event;
	    switch (bevent->button & 0xFF) {
		case Button1:
		    /*printf("handleN: b1\n");*/
		    SWITCH_FRAME(a, 0);
		    break;
		case Button2:
		    /*printf("handleN: b2\n");*/
		    break;
		case Button3:
		    /*printf("handleN: b3\n");*/
		    break;
		case Button4:
		    /*printf("handle0: b4\n");*/
		    a->frames[a->page].p--;
		    a->update(a, 0);
		    break;
		case Button5:
		    /*printf("handle0: b5\n");*/
		    a->frames[a->page].p++;
		    a->update(a, 0);
		    break;
	    }
	    break;
    }
}


static void
update_frame0 (const APP *a, int full)
{
    const DOCKAPP *d = (DOCKAPP *)a;
    const FRAME *frame = &a->frames[0];

    char buf[16];

    time_t at;
    struct tm *bt;

    static CPUSTAT avg;

    int wifi;

    int temp;
    int crit;

    int on_line;
    int battery;
    BATSTAT total;

    if (full) {
	if (!d->iswindowed) {
	    dockapp_set_shape(d, frame->m);
	}

	dockapp_copy_area(d, frame->f, 0, 0, d->width, d->height, 0, 0);
    }

    at = time(NULL);
    bt = localtime(&at);
    sprintf(buf, "%2d:%02d", bt->tm_hour, bt->tm_min);
    draw_text(a, &a->fontx9, buf, 15, 2);

    if (system_get_cpu_load(0, &avg, NULL) > 0) {
	sprintf(buf, "__CPU__%3d%%", avg.used);
    } else {
	strcpy(buf, "      _____");
    }
    draw_text(a, &a->fontx8, buf, 1, 17);

    if (system_get_best_wifi(&wifi)) {
	sprintf(buf, "%3d\xF7", wifi);
    } else {
	strcpy(buf, "   \xF7");
    }
    draw_text(a, &a->fontx8, buf, 0, 31);

    if (system_get_max_temp(a->list, &temp, &crit)) {
	sprintf(buf, "%3d\xF8", temp);
    } else {
	strcpy(buf, "   \xF8");
    }
    draw_text(a, &a->fontx8, buf, 31, 31);

    battery = system_get_battery(a->list, 0, &total, NULL);
    on_line = system_get_ac_adapter(a->list);
    if (battery) {
	int ratio = 100 * total.curr_cap / total.full_cap;
	int ch = 0xB0;
	if (ratio >= 33) {
	    ch++;
	}
	if (ratio >= 66) {
	    ch++;
	}
	if (total.rate) {
	    if (on_line) {
		/* charging */
		int estimate = 60 * (total.full_cap - total.curr_cap) / total.rate;
		int hours = estimate / 60;
		if (hours > 99) {
		    hours = 99;
		}
		sprintf(buf, "%c__%2d:%02d\xAE\x98", ch, hours, estimate % 60);
	    } else {
		/* discharging */
		int estimate = 60 * total.curr_cap / total.rate;
		int hours = estimate / 60;
		if (hours > 99) {
		    hours = 99;
		}
		sprintf(buf, "%c__%2d:%02d__ ", ch, hours, estimate % 60);
	    }
	} else {
	    if (on_line) {
		/* bat + AC */
		sprintf(buf, "%c__%3d%%___\x98", ch, ratio);
	    } else {
		/* bat */
		sprintf(buf, "%c__%3d%%___ ", ch, ratio);
	    }
	}
	draw_meter(a, &a->meter2, total.curr_cap, total.full_cap, 3, 55);
    } else {
	if (on_line) {
	    /* only AC */
	    strcpy(buf, "     _____\x98");
	} else {
	    /* nada */
	    strcpy(buf, "     _____ ");
	}
	draw_meter(a, &a->meter2, 0, 0, 3, 55);
    }
    draw_text(a, &a->fontx9, buf, 2, 44);

    dockapp_update(d);
}


static void
update_frame1 (const APP *a, int full)
{
    const DOCKAPP *d = (DOCKAPP *)a;
    const FRAME *frame = &a->frames[1];

    char buf[16];

    time_t at;
    struct tm *bt;

    int days, hours, mins;

    static const char *dow[] = {
	"___SUNDAY___",
	"___MONDAY___",
	"__TUESDAY__",
	"WEDNESDAY",
	"_THURSDAY_",
	"___FRIDAY___",
	"_SATURDAY_"
    };
    static const char *mon[] = {
	"JAN", "FEB", "MAR", "APR",
	"MAY", "JUN", "JUL", "AUG",
	"SEP", "OCT", "NOV", "DEC"
    };

    if (full) {
	if (!d->iswindowed) {
	    dockapp_set_shape(d, frame->m);
	}

	dockapp_copy_area(d, frame->f, 0, 0, d->width, d->height, 0, 0);
    }

    at = time(NULL);
    bt = localtime(&at);
    draw_text(a, &a->fontx9, dow[bt->tm_wday], 3, 2);

    sprintf(buf, "%-2d %s", bt->tm_mday, mon[bt->tm_mon]);
    draw_text(a, &a->fontx9, buf, 10, 13);

    sprintf(buf, "%d", 1900 + bt->tm_year);
    draw_text(a, &a->fontx9, buf, 16, 24);

    draw_text(a, &a->fontx7, "UPTIME", 15, 39);
    strcpy(buf, "   _  _  ");
    if (system_get_uptime(&days, &hours, &mins)) {
	if (days > 999) {
	    days = 999;
	}
	sprintf(buf, "%03d_%02d:%02d", days, hours, mins);
    }
    draw_text(a, &a->fontx8, buf, 2, 48);

    dockapp_update(d);
}


static void
update_frame2 (const APP *a, int full)
{
    const DOCKAPP *d = (DOCKAPP *)a;
    FRAME *frame = &a->frames[2];

    char buf[16];

    int speed[8];
    static CPUSTAT load[8];
    int mem_free, mem_total, swp_free, swp_total;
    int i, j, n, k;

    if (full) {
	if (!d->iswindowed) {
	    dockapp_set_shape(d, frame->m);
	}

	dockapp_copy_area(d, frame->f, 0, 0, d->width, d->height, 0, 0);
    }

    n = system_get_cpu_speed(NELEM(speed), speed);
    system_get_cpu_load(NELEM(load), NULL, load);

    SCROLL_START(frame->p, n, MAX_DISPLAY_CPU);
    j = frame->p + MAX_DISPLAY_CPU;
    if (n < MAX_DISPLAY_CPU) {
	while (j > n) {
	    j--;
	    draw_text(a, &a->fontx7, "           ", 2, 2 + j * 20);
	    draw_text(a, &a->fontx7, "           ", 2, 10 + j * 20);
	}
    }

    for (k = 0, i = 0; k < j; i++, k++) {
	int r;
	char *gov;

	if (speed[i] < 0) {
	    k--;
	    continue;
	}

	r = k - frame->p;

	if (r < 0) {
	    continue;
	}

	sprintf(buf, "CPU%d %4d M", i, speed[i]);
	draw_text(a, &a->fontx7, buf, 2, 2 + r * 20);

	strcpy(buf, "    ");
	if (load[i].used >= 0) {
	    sprintf(buf, "%3d%%", load[i].used);
	}
	draw_text(a, &a->fontx7, buf, 2, 10 + r * 20);

	gov = "      ";
	if (system_get_cpu_gov(i, sizeof(buf), buf)) {
	    if (!strcmp(buf, "performance")) {
		gov = "PERFRM";
	    } else if (!strcmp(buf, "powersave")) {
		gov = "PWRSAV";
	    } else if (!strcmp(buf, "userspace")) {
		gov = "USRSPC";
	    } else if (!strcmp(buf, "ondemand")) {
		gov = "ONDMND";
	    } else if (!strcmp(buf, "conservative")) {
		gov = "CONSRV";
	    }
	}
	draw_text(a, &a->fontx7, gov, 27, 10 + r * 20);
    }

    if (system_get_mem_stat(&mem_free, &mem_total, &swp_free, &swp_total)) {
	strcpy(buf, "        ");
	if (mem_total) {
	    sprintf(buf, "MEM %3d%%", 100 * (mem_total - mem_free) / mem_total);
	}
	draw_text(a, &a->fontx7, buf, 7, 42);
	strcpy(buf, "        ");
	if (swp_total) {
	    sprintf(buf, "SWP %3d%%", 100 * (swp_total - swp_free) / swp_total);
	}
	draw_text(a, &a->fontx7, buf, 7, 50);
    }

    dockapp_update(d);
}


static void
update_frame3 (const APP *a, int full)
{
    const DOCKAPP *d = (DOCKAPP *)a;
    FRAME *frame = &a->frames[3];

    char buf[16];

    IFSTAT ifs[8];
    int i, j, n;

    if (full) {
	if (!d->iswindowed) {
	    dockapp_set_shape(d, frame->m);
	}

	dockapp_copy_area(d, frame->f, 0, 0, d->width, d->height, 0, 0);
    }

    n = system_get_netif(NELEM(ifs), ifs);

    SCROLL_START(frame->p, n, MAX_DISPLAY_NET);
    j = frame->p + MAX_DISPLAY_NET;
    if (n < MAX_DISPLAY_NET) {
	while (j > n) {
	    j--;
	    draw_text(a, &a->fontx7, "           ", 2, 2 + j * 30);
	    draw_text(a, &a->fontx7, "           ", 2, 2 + j * 30 + 9);
	    draw_text(a, &a->fontx6, "   _   _   _   ", 2, 2 + j * 30 + 18);
	}
    }

    for (i = frame->p; i < j; i++) {
	int r = i - frame->p;

	strupr(ifs[i].name, ifs[i].name);
	ifs[i].name[6] = '\0';
	if (ifs[i].wlink >= 0) {
	    sprintf(buf, "%-6s_%3d\xF7", ifs[i].name, ifs[i].wlink);
	    draw_text(a, &a->fontx7, buf, 2, 2 + r * 30);
	    strupr(buf, ifs[i].essid);
	    buf[11] = '\0';
	} else {
	    sprintf(buf, "%-6s_    _", ifs[i].name);
	    draw_text(a, &a->fontx7, buf, 2, 2 + r * 30);
	    strcpy(buf, "           ");
	}
	draw_text(a, &a->fontx7, buf, 2, 2 + r * 30 + 9);
	if (ifs[i].ipv4 != -1) {
	    int addr = ifs[i].ipv4;
	    sprintf(buf, "%3d.%3d.%3d.%3d", (addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF);
	} else {
	    strcpy(buf, "   _   _   _   ");
	}
	draw_text(a, &a->fontx6, buf, 2, 2 + r * 30 + 18);
    }

    dockapp_update(d);
}


static void
update_frame4 (const APP *a, int full)
{
    const DOCKAPP *d = (DOCKAPP *)a;
    FRAME *frame = &a->frames[4];

    char buf[16];

    TEMPSTAT temp[16];
    int i, j, n;

    if (full) {
	if (!d->iswindowed) {
	    dockapp_set_shape(d, frame->m);
	}

	dockapp_copy_area(d, frame->f, 0, 0, d->width, d->height, 0, 0);
    }

    n = system_get_temperature(a->list, NELEM(temp), temp);

    SCROLL_START(frame->p, n, MAX_DISPLAY_TEMP);
    j = frame->p + MAX_DISPLAY_TEMP;
    if (n < MAX_DISPLAY_TEMP) {
	while (j > n) {
	    j--;
	    draw_text(a, &a->fontx7, "    _       ", 1, 1 + j * 12);
	}
    }

    for (i = frame->p; i < j; i++) {
	int r = i - frame->p;

	if (!strncmp(temp[i].name, "Core ", 5) && isdigit(temp[i].name[5]) && !temp[i].name[6]) {
	    temp[i].name[3] = temp[i].name[5];
	}
	temp[i].name[4] = '\0';
	strupr(temp[i].name, temp[i].name);
	sprintf(buf, "%-4s:%3d\xA7%3d", temp[i].name, temp[i].temp, temp[i].max);
	if (!temp[i].max) {
	    int l = strlen(buf);
	    buf[--l] = ' ';
	    buf[--l] = ' ';
	    buf[--l] = ' ';
	}
	draw_text(a, &a->fontx7, buf, 1, 1 + r * 12);
    }

    dockapp_update(d);
}


static void
update_frame5 (const APP *a, int full)
{
    const DOCKAPP *d = (DOCKAPP *)a;
    const FRAME *frame = &a->frames[5];

    char buf[16];

    int on_line;
    int battery;
    BATSTAT total, batt[2];

    int i;

    if (full) {
	if (!d->iswindowed) {
	    dockapp_set_shape(d, frame->m);
	}

	dockapp_copy_area(d, frame->f, 0, 0, d->width, d->height, 0, 0);
    }

    on_line = system_get_ac_adapter(a->list);
    battery = system_get_battery(a->list, NELEM(batt), &total, batt);

    for (i = 0; i < battery; i++) {
	char *state;
	int ratio = 100 * batt[i].curr_cap / batt[i].full_cap;
	if (batt[i].rate) {
	    int estimate;
	    int hours;
	    if (on_line) {
		estimate = 60 * (batt[i].full_cap - batt[i].curr_cap) / batt[i].rate;
		state = "CHARGING   ";
	    } else {
		estimate = 60 * batt[i].curr_cap / batt[i].rate;
		state = "DISCHARGING";
	    }
	    hours = estimate / 60;
	    if (hours > 99) {
		hours = 99;
	    }
	    sprintf(buf, "%3d%% %2d:%02d", ratio, hours, estimate % 60);
	} else {
	    sprintf(buf, "%3d%%   _  ", ratio);
	    state = "STEADY     ";
	}
	draw_text(a, &a->fontx7, batt[i].name, 2, 1 + i * 30);
	draw_text(a, &a->fontx7, state, 2, 9 + i * 30);
	draw_text(a, &a->fontx7, buf, 2, 17 + i * 30);
	draw_meter(a, &a->meter2, batt[i].curr_cap, batt[i].full_cap, 3, 25 + i * 30);
    }
    for (; i < 2; i++) {
	draw_text(a, &a->fontx7, "           ", 2, 1 + i * 30);
	draw_text(a, &a->fontx7, "           ", 2, 9 + i * 30);
	draw_text(a, &a->fontx7, "       _  ", 2, 17 + i * 30);
	draw_text(a, &a->fontx7, "           ", 2, 20 + i * 30);
    }

    dockapp_update(d);
}


static int
update_text (SENSOR *list)
{
    (void)list;
    /* XXX cool stuff here */
}


static int
run_text (SENSOR *list, unsigned long millis)
{
    struct timeval timeout;
    fd_set rset;

    struct stat buf;
    int redir = fstat(STDOUT_FILENO, &buf) == 0 &&
		(S_ISREG(buf.st_mode) || S_ISFIFO(buf.st_mode));

    for (;;) {
	update_text(list);

	if (redir) {
	    break;
	}

	printf("not yet implemented -- press Enter to quit\n");

	timeout.tv_sec = millis / 1000;
	timeout.tv_usec = (millis % 1000) * 1000;

	FD_ZERO(&rset);
	FD_SET(0, &rset);
	if (select(0 + 1, &rset, NULL, NULL, &timeout) > 0) {
	    break;
	}
    }

    return 0;
}


static void
args (int argc, char **argv, int *iswindowed, const char **display_name, int *textmode)
{
    const char *myself = argv[0];

    *iswindowed = 0;
    *display_name = NULL;
    *textmode = 0;

    while (--argc) {
	const char *p = *++argv;
	if (!strcmp(p, "-h") || !strcmp(p, "--help")) {
	    printf("wmfu v1.0 (c) 2007 Daniel Borca\n");
	    printf("wmfu is distributed under GNU GPL v2\n\n");
	    printf("usage: %s [OPTIONS]\n", myself);
	    printf("    -display <name>    use specified display\n");
	    printf("    --windowed         run in regular window\n");
	    printf("    --textmode         run in text mode\n");
	    exit(0);
	}
	if (!strcmp(p, "-display")) {
	    if (argc < 2) {
		fprintf(stderr, "%s: argument to `%s' is missing\n", myself, p);
		exit(1);
	    }
	    --argc;
	    *display_name = *++argv;
	}
	if (!strcmp(p, "--windowed")) {
	    *iswindowed = 1;
	}
	if (!strcmp(p, "--textmode")) {
	    *textmode = 1;
	}
    }
}


int
main (int argc, char **argv)
{
    int rv;

    APP App;
    FRAME frames[6];

    int iswindowed;
    const char *display_name;
    int textmode;

    args(argc, argv, &iswindowed, &display_name, &textmode);

    App.list = sensors_init();
    if (App.list == NULL) {
	fprintf(stderr, "Error allocating sensor structure\n");
	return -1;
    }

    if (textmode) {
	int rv = run_text(App.list, 1000);
	sensors_free(App.list);
	return rv;
    }

    rv = dockapp_open_window(&DockApp, display_name, "wmfu", iswindowed, argc, argv);
    if (rv != 0) {
	fprintf(stderr, "Could not open display `%s'\n", display_name);
	sensors_free(App.list);
	return rv;
    }

    frames[0].v = dockapp_xpm2pixmap(&DockApp, frame0_xpm, &frames[0].f, &frames[0].m, NULL, 0);
    if (!frames[0].v) {
	fprintf(stderr, "Error initializing frame\n");
	sensors_free(App.list);
	return -1;
    }

    if (!dockapp_xpm2pixmap(&DockApp, font_xpm, &App.font, NULL, NULL, 0)) {
	fprintf(stderr, "Error initializing font\n");
	sensors_free(App.list);
	return -1;
    }

    App.fontx9.row = FONTX9_ROW;
    App.fontx9.height = 9;
    App.fontx9.width = fontx9_width;
    App.fontx9.offset = fontx9_offset;

    App.fontx8.row = FONTX8_ROW;
    App.fontx8.height = 8;
    App.fontx8.width = fontx8_width;
    App.fontx8.offset = fontx8_offset;

    App.fontx7.row = FONTX7_ROW;
    App.fontx7.height = 7;
    App.fontx7.width = fontx7_width;
    App.fontx7.offset = fontx7_offset;

    App.fontx6.row = FONTX6_ROW;
    App.fontx6.height = 6;
    App.fontx6.width = fontx6_width;
    App.fontx6.offset = fontx6_offset;

    App.meter2.row = METER2_ROW;
    App.meter2.height = 2;
    App.meter2.width = meter2_width;
    App.meter2.offset = NULL;

    frames[1].v = dockapp_xpm2pixmap(&DockApp, frame1_xpm, &frames[1].f, &frames[1].m, NULL, 0);
    frames[2].v = dockapp_xpm2pixmap(&DockApp, frame2_xpm, &frames[2].f, &frames[2].m, NULL, 0);
    frames[3].v = dockapp_xpm2pixmap(&DockApp, frame3_xpm, &frames[3].f, &frames[3].m, NULL, 0);
    frames[4].v = dockapp_xpm2pixmap(&DockApp, frame4_xpm, &frames[4].f, &frames[4].m, NULL, 0);
    frames[5].v = dockapp_xpm2pixmap(&DockApp, frame5_xpm, &frames[5].f, &frames[5].m, NULL, 0);

    frames[0].u = 2000;
    frames[1].u = 10000;
    frames[2].u = 5000;
    frames[3].u = 5000;
    frames[4].u = 5000;
    frames[5].u = 10000;

    App.frames = frames;
    SWITCH_FRAME(&App, 0);

    dockapp_set_eventmask(&DockApp, ButtonPressMask);

#if 1
    while (!DockApp.quit) {
	XEvent event;
	if (dockapp_nextevent_or_timeout(&DockApp, &event, App.millis)) {
	    App.handle(&App, &event);
	} else {
	    App.update(&App, 0);
	}
    }
#else
{
    int i;
    for (i = 0; i < 1000; i++) {
	App.update(&App, 0);
    }
}
#endif

    XCloseDisplay(DockApp.display);

    sensors_free(App.list);
    return 0;
}
