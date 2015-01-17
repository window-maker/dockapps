/* wmbattery - display laptop battery info, dockable in WindowMaker
 * Copyright (C) 1998-2014 Joey Hess <joey@kitenet.net>
 * Copyright (C) 2014 Window Maker Developers Team
 *                      <wmaker-dev@lists.windowmaker.org>
 *
 * Portions of code derived from:
 *   wmapm - Copyright (C) 1998-1999 Chris D. Faulhaber <jedgar@fxp.org>
 *   wmmon - Copyright (C) 1998 Martijn Pieterse <pieterse@xs4all.nl>
 *           Copyright (C) 1998 Antoine Nulle <warp@xs4all.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wmbattery.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "mask.xbm"
#include "sonypi.h"
#include "acpi.h"
#ifdef HAL
#include "simplehal.h"
#endif
#ifdef UPOWER
#include "upower.h"
#endif

Pixmap images[NUM_IMAGES];
Window root, iconwin, win;
int screen;
XpmIcon icon;
Display *display;
GC NormalGC;
int pos[2] = {0, 0};

#ifdef HAVE__DEV_APM
#define APM_STATUS_FILE "/dev/apm"
#else
#define APM_STATUS_FILE "/proc/apm"
#endif

char *apm_status_file = APM_STATUS_FILE;

char *crit_audio_fn = NULL;
char *crit_audio;
int crit_audio_size;
char *crit_command = NULL;

int battnum = 1;
#ifdef HAL
int use_simplehal = 0;
#endif
#ifdef UPOWER
int use_upower = 0;
#endif
int use_sonypi = 0;
int use_acpi = 0;
int delay = 0;
int always_estimate_remaining = 0;
int granularity_estimate_remaining = 1;
int initial_state = WithdrawnState;

signed int low_pct = -1;
signed int critical_pct = -1;

void error(const char *fmt, ...)
{
	va_list arglist;

	va_start(arglist, fmt);
	fprintf(stderr, "Error: ");
	vfprintf(stderr, fmt, arglist);
	fprintf(stderr, "\n");
	va_end(arglist);

	exit(1);
}

#if defined (HAVE_MACHINE_APM_BIOS_H) || defined (HAVE_I386_APMVAR_H) /* BSD */
int apm_read(apm_info *i)
{
	int fd;
#ifdef HAVE_MACHINE_APM_BIOS_H /* FreeBSD */
	unsigned long request = APMIO_GETINFO;
	struct apm_info info;
#else /* NetBSD or OpenBSD */
	unsigned long request= APM_IOC_GETPOWER;
	struct apm_power_info info;
#endif

	if ((fd = open(apm_status_file, O_RDONLY)) == -1) {
		return 0;
	}
	if (ioctl(fd, request, &info) == -1) {
		return 0;
	}
	close(fd);

#ifdef HAVE_MACHINE_APM_BIOS_H /* FreeBSD */
	i->ac_line_status = info.ai_acline;
	i->battery_status = info.ai_batt_stat;
	i->battery_flags = (info.ai_batt_stat == 3) ? 8: 0;
	i->battery_percentage = info.ai_batt_life;
	i->battery_time = info.ai_batt_time;
	i->using_minutes = 0;
#else /* NetBSD or OpenBSD */
	i->ac_line_status = info.ac_state;
	i->battery_status = info.battery_state;
	i->battery_flags = (info.battery_state == 3) ? 8: 0;
	i->battery_percentage = info.battery_life;
	i->battery_time = info.minutes_left;
	i->using_minutes = 1;
#endif

	return 1;
}

int apm_exists(void)
{
	apm_info i;

	if (access(apm_status_file, R_OK))
		return 0;
	return apm_read(&i);
}
#endif
#if !defined(HAVE_APM_H)
int apm_read(apm_info *i)
{
	return -1;
}
int apm_exists(void)
{
	return -1;
}
#endif

int apm_change(apm_info *cur)
{
	static int ac_line_status = 0, battery_status = 0, battery_flags = 0,
		battery_percentage = 0, battery_time = 0, using_minutes = 0;

	int i = cur->ac_line_status     == ac_line_status     &&
		cur->battery_status     == battery_status     &&
		cur->battery_flags      == battery_flags      &&
		cur->battery_percentage == battery_percentage &&
		cur->battery_time       == battery_time       &&
		cur->using_minutes      == using_minutes;

	ac_line_status = cur->ac_line_status;
	battery_status = cur->battery_status;
	battery_flags = cur->battery_flags;
	battery_percentage = cur->battery_percentage;
	battery_time = cur->battery_time;
	using_minutes = cur->using_minutes;

	return i;
}

/* Calculate battery estimate */
void estimate_timeleft(apm_info *cur_info)
{
	/* Time of the last estimate */
	static time_t estimate_time;
	/* Estimated time left */
	static time_t estimate;
	/* Time when we last noticed a battery level change */
	static time_t battery_change_time;
	/* The previous estimation we had before the battery level changed */
	static time_t prev_estimate;
	/* Percentage at the last estimate */
	static short percent;
	/* Where we charging or discharging the last time we were called? */
	static short was_charging = 1;
	/* Have we made a guess lately? */
	static short guessed_lately;

	time_t t;
	int interval;
	short is_charging = cur_info->battery_flags & BATTERY_FLAGS_CHARGING;

	errno = 0;
	if (time(&t) == ((time_t)-1) && errno != 0)
		goto estim_values;

	if ((
		    /* AC is on and battery is not charging anymore or ... */
		    (cur_info->ac_line_status == AC_LINE_STATUS_ON) && !is_charging
		    ) ||
	    (
		    /* ... the charging state has changed */
		    is_charging ^ was_charging
		    )) {
		/* Reset counters */
		battery_change_time = t;
		estimate = -1;
		guessed_lately = 0;
		estimate_time = t;
		prev_estimate = 0;
		goto estim_values;
	}

	/* No change: decrease estimate */
	if ((percent - cur_info->battery_percentage)
	    / granularity_estimate_remaining == 0) {
		estimate -= t - estimate_time;
		estimate_time = t;
		if (guessed_lately && estimate < 0)
			estimate = 0;
		goto estim_values;
	}

	/* The battery level changed: calculate estimate based
	 * on change speed and previous estimate */
	guessed_lately = 1;
	estimate_time = t;
	interval = estimate_time - battery_change_time;
	prev_estimate = estimate;
	battery_change_time = estimate_time;
	estimate = (is_charging
		    ? (cur_info->battery_percentage - 100)
		    : cur_info->battery_percentage)
		* interval / (percent - cur_info->battery_percentage);
	if (prev_estimate > 0)
		estimate = (estimate * 2 + prev_estimate) / 3;

estim_values:
	percent = cur_info->battery_percentage;
	was_charging = is_charging;
	cur_info->battery_time = estimate;
	if (estimate < 0)
		estimate = 0;
	cur_info->using_minutes = 0;
}

/* Load up the images this program uses. */
void load_images(void)
{
	int x;
	char fn[128]; /* enough? */

	for (x = 0; x < NUM_IMAGES; x++) {
		sprintf(fn, "%s/%s.xpm", ICONDIR, image_info[x].filename);
		if (XpmReadFileToPixmap(display, root, fn, &images[x], NULL, NULL)) {
			/* Check in current direcotry for fallback. */
			sprintf(fn, "%s.xpm", image_info[x].filename);
			if (XpmReadFileToPixmap(display, root, fn, &images[x], NULL, NULL))
				error("Failed to load %s\n", fn);
		}
	}
}

void load_audio(void)
{
	int fd;
	struct stat s;

	crit_audio = NULL;
	if (crit_audio_fn == NULL)
		return;
	fd = open(crit_audio_fn, 0);
	if (fd == -1)
		error("unable to open audio file");
	if (fstat(fd, &s) == 0) {
		crit_audio_size = s.st_size;
		crit_audio = malloc(crit_audio_size);
		/* XXX: make this more robust? (loop?) */
		if (read(fd, crit_audio, crit_audio_size) != crit_audio_size) {
			free(crit_audio);
			crit_audio = NULL;
			error("unable to read audio file");
		}
	}
	close(fd);
}

/* string replacement function by Laird Shaw, in public domain
 * http://creativeandcritical.net/str-replace-c */
char *replace_str(const char *str, const char *old, const char *new)
{
	char *ret, *r;
	const char *p, *q;
	size_t oldlen = strlen(old);
	size_t count, retlen, newlen = strlen(new);

	if (oldlen != newlen) {
		for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
			count++;
		/* this is undefined if p - str > PTRDIFF_MAX */
		retlen = p - str + strlen(p) + count * (newlen - oldlen);
	} else
		retlen = strlen(str);

	ret = malloc(retlen + 1);
	if (!ret)
		return NULL;

	for (r = ret, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen) {
		/* this is undefined if q - p > PTRDIFF_MAX */
		ptrdiff_t l = q - p;
		memcpy(r, p, l);
		r += l;
		memcpy(r, new, newlen);
		r += newlen;
	}
	strcpy(r, p);

	return ret;
}

void cmd_crit(const char *cmd, int percentage, int time)
{
	char prc_str[255] = "";
	char min_str[255] = "";
	char sec_str[255] = "";
	char *tmp_a = NULL;
	char *tmp_b = NULL;
	char *command = NULL;
	int ret;

	if (!cmd)
		return;
	if (percentage > 100 || percentage < 0)
		return;
	if (time > 65535 || time < 0)
		return;

	sprintf(prc_str, "%i", percentage);
	sprintf(min_str, "%i", time / 60);
	sprintf(sec_str, "%i", time % 60);

	tmp_a = replace_str(cmd, STR_SUB_PERCENT, prc_str);
	if (!tmp_a)
		return;
	tmp_b = replace_str(tmp_a, STR_SUB_MINUTES, min_str);
	if (!tmp_b)
		return;
	command = replace_str(tmp_b, STR_SUB_SECONDS, sec_str);
	if (!command)
		return;

	ret = system(command);
	if (ret == -1)
		error("unable to run command: %s", command);

	free(tmp_a);
	free(tmp_b);
	free(command);
}

/* Returns the display to run on (or NULL for default). */
char *parse_commandline(int argc, char *argv[])
{
	int c = 0;
	char *ret = NULL;
	char *s;

	while (c != -1) {
		c = getopt(argc, argv, "hd:g:if:b:w:c:l:es:a:x:v");
		switch (c) {
		case 'h':
			printf("Usage: wmbattery [options]\n");
			printf("\t-d <display>\tselects target display\n");
			printf("\t-h\t\tdisplay this help\n");
			printf("\t-g +x+y\t\tposition of the window\n");
			printf("\t-i start\n");
			printf("\t-b num\t\tnumber of battery to display\n");
			printf("\t-w secs\t\tseconds between updates\n");
			printf("\t-l percent\tlow percentage\n");
			printf("\t-c percent\tcritical percentage\n");
			printf("\t-e\t\tuse own time estimates\n");
			printf("\t-s granularity\tignore fluctuations less than granularity%% (implies -e)\n");
			printf("\t-a file\t\twhen critical send file to /dev/audio\n");
			printf("\t-x command\twhen critical execute this command\n");
			printf("\t-v\t\tdisplay version number\n");
			exit(0);
			break;
		case 'd':
			ret = strdup(optarg);
			break;
		case 'g':
			s = strtok(optarg, "+");
			if (s) {
				pos[0] = atoi(s);
				s = strtok(NULL, "+");
				if (s)
					pos[1] = atoi(s);
				else
					pos[0] = 0;
			}
			break;
		case 'i':
			initial_state = IconicState;
			break;
		case 'b':
			battnum = atoi(optarg);
			break;
		case 'w':
			delay = atoi(optarg);
			break;
		case 'l':
			low_pct = atoi(optarg);
			break;
		case 'c':
			critical_pct = atoi(optarg);
			break;
		case 'e':
			always_estimate_remaining = 1;
			break;
		case 's':
			always_estimate_remaining = 1;
			granularity_estimate_remaining = atoi(optarg);
			break;
		case 'a':
			crit_audio_fn = strdup(optarg);
			break;
		case 'x':
			crit_command = strdup(optarg);
			break;
		case 'v':
			printf("wmbattery "PACKAGE_VERSION"\n");
			exit(0);
			break;
		}
	}

	return ret;
}

/* Sets up the window and icon and all the nasty X stuff. */
void make_window(char *display_name, int argc, char *argv[])
{
	XClassHint classhint;
	char *wname = argv[0];
	XTextProperty name;
	XGCValues gcv;
	int dummy = 0, borderwidth = 1;
	XSizeHints sizehints;
	XWMHints wmhints;
	Pixel back_pix, fore_pix;
	Pixmap pixmask;

	display = XOpenDisplay(display_name);
	if (!display)
		error("can't open display %s", XDisplayName(display_name));

	screen = DefaultScreen(display);
	root = RootWindow(display, screen);

	/* Create window. */
	sizehints.flags = USSize | USPosition;
	sizehints.x = 0;
	sizehints.y = 0;
	XWMGeometry(display, screen, "", NULL, borderwidth,
		    &sizehints, &sizehints.x, &sizehints.y,
		    &sizehints.width, &sizehints.height, &dummy);

	sizehints.width = 64;
	sizehints.height = 64;
	sizehints.x = pos[0];
	sizehints.y = pos[1];
	back_pix = WhitePixel(display, screen);
	fore_pix = BlackPixel(display, screen);
	win = XCreateSimpleWindow(display, root, sizehints.x, sizehints.y,
				  sizehints.width, sizehints.height,
				  borderwidth, fore_pix, back_pix);
	iconwin = XCreateSimpleWindow(display, win, sizehints.x,
				      sizehints.y, sizehints.width,
				      sizehints.height, borderwidth,
				      fore_pix, back_pix);

	/* Activate hints */
	XSetWMNormalHints(display, win, &sizehints);
	classhint.res_name = wname;
	classhint.res_class = wname;
	XSetClassHint(display, win, &classhint);

	if (!XStringListToTextProperty(&wname, 1, &name))
		error("Can't allocate window name.");

	XSetWMName(display, win, &name);

	/* Create GC for drawing */
	gcv.foreground = fore_pix;
	gcv.background = back_pix;
	gcv.graphics_exposures = 0;
	NormalGC = XCreateGC(display, root,
			     GCForeground | GCBackground | GCGraphicsExposures,
			     &gcv);

	pixmask = XCreateBitmapFromData(display, win, mask_bits,
					mask_width, mask_height);
	XShapeCombineMask(display, win, ShapeBounding, 0, 0,
			  pixmask, ShapeSet);
	XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0,
			  pixmask, ShapeSet);

	wmhints.initial_state = initial_state;
	wmhints.icon_window = iconwin;
	wmhints.icon_x = sizehints.x;
	wmhints.icon_y = sizehints.y;
	wmhints.window_group = win;
	wmhints.flags = StateHint | IconWindowHint |
		IconPositionHint | WindowGroupHint;

	XSetWMHints(display, win, &wmhints);
	XSetCommand(display, win, argv, argc);

	XSelectInput(display, iconwin, ExposureMask);
	XSelectInput(display, win, ExposureMask);

	XMapWindow(display, win);
}

void flush_expose(Window w)
{
	XEvent dummy;

	while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
		;
}

void redraw_window(void)
{
	XCopyArea(display, images[FACE], iconwin, NormalGC, 0, 0,
		  image_info[FACE].width, image_info[FACE].height, 0, 0);
	flush_expose(iconwin);
	XCopyArea(display, images[FACE], win, NormalGC, 0, 0,
		  image_info[FACE].width, image_info[FACE].height, 0, 0);
	flush_expose(win);
}

/*
 * Display an image, using XCopyArea. Can display only part of an image,
 * located anywhere.
 */
void copy_image(int image, int xoffset, int yoffset,
		int width, int height, int x, int y)
{
	XCopyArea(display, images[image], images[FACE], NormalGC,
		  xoffset, yoffset, width, height, x, y);
}

/*
 * Display a letter in one of two fonts, at the specified x position.
 * Note that 10 is passed for special characters `:' or `1' at the
 * end of the font.
 */
void draw_letter(int letter, int font, int x)
{
	copy_image(font, image_info[font].charwidth * letter, 0,
		   image_info[font].charwidth, image_info[font].height,
		   x, image_info[font].y);
}

/* Display an image at its normal location. */
void draw_image(int image)
{
	copy_image(image, 0, 0,
		   image_info[image].width, image_info[image].height,
		   image_info[image].x, image_info[image].y);
}

void recalc_window(apm_info cur_info)
{
	int time_left, hour_left, min_left, digit, x;
	static int blinked;

	/* Display if it's plugged in. */
	switch (cur_info.ac_line_status) {
	case AC_LINE_STATUS_ON:
		draw_image(PLUGGED);
		break;
	default:
		draw_image(UNPLUGGED);
	}

	/* Display the appropriate color battery. */
	switch (cur_info.battery_status) {
	case BATTERY_STATUS_HIGH:
	case BATTERY_STATUS_CHARGING:
		draw_image(BATTERY_HIGH);
		break;
	case BATTERY_STATUS_LOW:
		draw_image(BATTERY_LOW);
		break;
	case BATTERY_STATUS_CRITICAL: /* blinking red battery */
		if (blinked)
			draw_image(BATTERY_CRITICAL);
		else
			draw_image(BATTERY_BLINK);
		blinked = !blinked;
		break;
	default:
		draw_image(BATTERY_NONE);
	}

	/* Show if the battery is charging. */
	if (cur_info.battery_flags & BATTERY_FLAGS_CHARGING)
		draw_image(CHARGING);
	else
		draw_image(NOCHARGING);

	/*
	 * Display the percent left dial. This has the side effect of
	 * clearing the time left field.
	 */
	x = DIAL_MULTIPLIER * cur_info.battery_percentage;
	if (x >= 0) {
		/* Start by displaying bright on the dial. */
		copy_image(DIAL_BRIGHT, 0, 0,
			   x, image_info[DIAL_BRIGHT].height,
			   image_info[DIAL_BRIGHT].x,
			   image_info[DIAL_BRIGHT].y);
	}
	/* Now display dim on the remainder of the dial. */
	copy_image(DIAL_DIM, x, 0,
		   image_info[DIAL_DIM].width - x,
		   image_info[DIAL_DIM].height,
		   image_info[DIAL_DIM].x + x,
		   image_info[DIAL_DIM].y);

	/* Show percent remaining */
	if (cur_info.battery_percentage >= 0) {
		digit = cur_info.battery_percentage / 10;
		if (digit == 10) {
			/* 11 is the `1' for the hundreds place. */
			draw_letter(11, SMALLFONT, HUNDREDS_OFFSET);
			digit = 0;
		}
		draw_letter(digit, SMALLFONT, TENS_OFFSET);
		digit = cur_info.battery_percentage % 10;
		draw_letter(digit, SMALLFONT, ONES_OFFSET);
	} else {
		/* There is no battery, so we need to dim out the
		 * percent sign that is normally bright. */
		draw_letter(10, SMALLFONT, PERCENT_OFFSET);
	}

	/* Show time left */

	/* A negative number means that it is unknown. Dim the field. */
	if (cur_info.battery_time < 0) {
		draw_letter(10, BIGFONT, COLON_OFFSET);
		redraw_window();
		return;
	}

	if (cur_info.using_minutes)
		time_left = cur_info.battery_time;
	else
		time_left = cur_info.battery_time / 60;
	hour_left = time_left / 60;
	min_left = time_left % 60;
	digit = hour_left / 10;
	draw_letter(digit, BIGFONT, HOURS_TENS_OFFSET);
	digit = hour_left % 10;
	draw_letter(digit, BIGFONT, HOURS_ONES_OFFSET);
	digit = min_left / 10;
	draw_letter(digit, BIGFONT, MINUTES_TENS_OFFSET);
	digit = min_left % 10;
	draw_letter(digit, BIGFONT, MINUTES_ONES_OFFSET);

	redraw_window();
}

void snd_crit(void)
{
	int audio, n;

	if (crit_audio) {
		audio = open("/dev/audio", O_WRONLY);
		if (audio >= 0) {
			n = write(audio, crit_audio, crit_audio_size);
			if (n != crit_audio_size)
				fprintf(stderr, "write failed (%d/%d bytes)\n", n, crit_audio_size);
			close(audio);
		}
	}
}

void alarmhandler(int sig)
{
	apm_info cur_info;
	int old_status;

#ifdef UPOWER
	if (use_upower) {
		if (upower_read(1, &cur_info) != 0)
			error("Cannot read upower information.");
	} else if (use_acpi) {
#else
	if (use_acpi) {
#endif
		if (acpi_read(battnum, &cur_info) != 0)
			error("Cannot read ACPI information.");
	}
#ifdef HAL
	else if (use_simplehal) {
		if (simplehal_read(battnum, &cur_info) != 0)
			error("Cannot read HAL information.");
	}
#endif
	else if (!use_sonypi) {
		if (apm_read(&cur_info) != 0)
			error("Cannot read APM information.");
	} else {
		if (sonypi_read(&cur_info) != 0)
			error("Cannot read sonypi information.");
	}

	old_status = cur_info.battery_status;

	/* Always calculate remaining lifetime? apm and acpi both use a
	 * negative number here to indicate error, missing battery, or
	 * cannot determine time. */
	if (always_estimate_remaining || cur_info.battery_time < 0)
		estimate_timeleft(&cur_info);

	/* Override the battery status? */
	if ((low_pct > -1 || critical_pct > -1) &&
	    cur_info.ac_line_status != AC_LINE_STATUS_ON) {
		if (cur_info.battery_percentage <= critical_pct)
			cur_info.battery_status = BATTERY_STATUS_CRITICAL;
		else if (cur_info.battery_percentage <= low_pct)
			cur_info.battery_status = BATTERY_STATUS_LOW;
		else
			cur_info.battery_status = BATTERY_STATUS_HIGH;
	}

	/* If APM data changes redraw and wait for next update */
	/* Always redraw if the status is critical, to make it blink. */
	if (!apm_change(&cur_info) || cur_info.battery_status == BATTERY_STATUS_CRITICAL)
		recalc_window(cur_info);

	if ((old_status == BATTERY_STATUS_HIGH) &&
	    (cur_info.battery_status == BATTERY_STATUS_LOW)) {
		snd_crit();
	} else if (cur_info.battery_status == BATTERY_STATUS_CRITICAL) {
		snd_crit();
		cmd_crit(crit_command, cur_info.battery_percentage,
			 cur_info.battery_time);
	}

	alarm(delay);
}

void check_battery_num(int real, int requested)
{
	if (requested > real || requested < 1) {
		error("There %s only %i batter%s, and you asked for number %i.",
		      real == 1 ? "is" : "are",
		      real,
		      real == 1 ? "y" : "ies",
		      requested);
	}
}

int main(int argc, char *argv[])
{
	make_window(parse_commandline(argc, argv), argc, argv);

	/*  Check for APM support (returns 0 on success). */
	if (apm_exists() == 0) {
		if (!delay)
			delay = 1;
	}
#ifdef HAL
	/* Check for hal support. */
	else if (simplehal_supported()) {
		use_simplehal = 1;
		if (!delay)
			delay = 2;
	}
#endif
#ifdef UPOWER
	else if (upower_supported()) {
		use_upower = 1;
		delay = 2;
	}
#endif
	/* Check for ACPI support. */
	else if (acpi_supported() && acpi_batt_count > 0) {
		check_battery_num(acpi_batt_count, battnum);
		use_acpi = 1;
		if (!delay)
			delay = 3; /* slow interface! */
	} else if (sonypi_supported()) {
		use_sonypi = 1;
		low_pct = 10;
		critical_pct = 5;
		if (!delay)
			delay = 1;
	} else {
		error("No APM, ACPI, UPOWER, HAL or SPIC support detected.");
	}

	load_images();
	load_audio();

	signal(SIGALRM, alarmhandler);
	alarmhandler(SIGALRM);

	while (1) {
		XEvent ev;
		XNextEvent(display, &ev);
		if (ev.type == Expose)
			redraw_window();
	}
}
