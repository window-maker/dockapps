/* $Id: wmwlmon.c,v 1.41 2008/05/13 09:26:22 hacki Exp $ */

/*
 * Copyright (c) 2005, 2006 Marcus Glocker <marcus@nazgul.ch>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/xpm.h>

#include "wl.h"
#include "xutils.h"
#include "bitmaps/wmwlmon_mask.xbm"
#include "bitmaps/wmwlmon_master.xpm"

/*
 * defines
 */
#define DELAY		10000
#define NWIDLENP	52
#define NWIDLENC	8

/*
 * prototypes
 */
int	main(int, char **);
void	usage(int);
void	signal_handler(const int);
void	debugloop(char *);
void	draw_nwid(char *, int);
void	draw_string(const char *, const int, const int);
void	draw_signal(const int, const int);
int	scroll_lcd(const int, const int, const int, const int, const int,
	    const char *);
int	scroll_bounce(const int, const int, const int, const int, const int,
	    const char *);
int	scroll_fade(const int, const int, const int, const int, const int,
	    const char *);

/*
 * global variables for this file
 */
static const char	*version = "1.0";
volatile sig_atomic_t	quit = 0;
char			TimeColor[30] = "#ffff00";
char			BackgroundColor[30] = "#181818";

struct coordinates {
	int	x;
	int	y;
	int	w;
	int	h;
} fonts[128] = {
	{  60, 102,   6,   8 },	/* Dec   0 nul NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   1 soh NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   2 stx NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   3 etx NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   4 eot NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   5 enq NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   6 ack NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   7 bel NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   8  bs NOTUSED */
	{  60, 102,   6,   8 },	/* Dec   9  ht NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  10  nl NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  11  vt NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  12  np NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  13  cr NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  14  so NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  15  si NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  16 dle NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  17 dc1 NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  18 dc2 NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  19 dc3 NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  20 dc4 NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  21 nak NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  22 syn NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  23 etb NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  24 can NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  25  em NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  26 sub NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  27 esc NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  28  fs NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  29  gs NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  30  rs NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  31  us NOTUSED */
	{  60, 102,   6,   8 },	/* Dec  32  sp NOTUSED */
	{  66, 102,   6,   8 },	/* Dec  33 '!' */
	{  72, 102,   6,   8 },	/* Dec  34 '"' */
	{  78, 102,   6,   8 },	/* Dec  35 '#' */
	{  60, 102,   6,   8 }, /* Dec  36 '$' NOTUSED */
	{  90, 101,   6,   8 },	/* Dec  37 '%' */
	{  96, 102,   6,   8 },	/* Dec  38 '&' */
	{ 102, 102,   6,   8 },	/* Dec  39 ''' */
	{ 108, 102,   6,   8 },	/* Dec  40 '(' */
	{ 114, 102,   6,   8 },	/* Dec  41 ')' */
	{ 120, 102,   6,   8 },	/* Dec  42 '*' */
	{ 126, 102,   6,   8 },	/* Dec  43 '+' */
	{ 132, 102,   6,   8 },	/* Dec  44 ',' */
	{ 138, 102,   6,   8 }, /* Dec  45 '-' */
	{ 144, 102,   6,   8 }, /* Dec  46 '.' */
	{ 150, 102,   6,   8 }, /* Dec  47 '/' */
	{   0, 102,   6,   6 },	/* Dec  48 '0' */
	{   6, 102,   6,   6 },	/* Dec  49 '1' */
	{  12, 102,   6,   6 }, /* Dec  50 '2' */
	{  18, 102,   6,   6 }, /* Dec  51 '3' */
	{  24, 102,   6,   6 }, /* Dec  52 '4' */
	{  30, 102,   6,   6 },	/* Dec  53 '5' */
	{  36, 102,   6,   6 },	/* Dec  54 '6' */
	{  42, 102,   6,   6 },	/* Dec  55 '7' */
	{  48, 102,   6,   6 },	/* Dec  56 '8' */
	{  54, 102,   6,   6 },	/* Dec  57 '9' */
	{   0, 112,   6,   8 },	/* Dec  58 ':' */
	{   6, 113,   6,   8 },	/* Dec  59 ';' */
	{  12, 112,   6,   8 },	/* Dec  60 '<'*/
	{  18, 112,   6,   8 },	/* Dec  61 '='*/
	{  24, 112,   6,   8 },	/* Dec  62 '>'*/
	{  30, 112,   6,   8 },	/* Dec  63 '?'*/
	{  36, 112,   6,   8 },	/* Dec  64 '@'*/
	{   0,  79,   6,   8 },	/* Dec  65 'A' */
	{   6,  79,   6,   8 },	/* Dec  66 'B' */
	{  12,  79,   6,   8 },	/* Dec  67 'C' */
	{  18,  79,   6,   8 },	/* Dec  68 'D' */
	{  24,  79,   6,   8 },	/* Dec  69 'E' */
	{  30,  79,   6,   8 },	/* Dec  70 'F' */
	{  36,  79,   6,   8 },	/* Dec  71 'G' */
	{  42,  79,   6,   8 },	/* Dec  72 'H' */
	{  48,  79,   6,   8 },	/* Dec  73 'I' */
	{  54,  79,   6,   8 },	/* Dec  74 'J' */
	{  60,  79,   6,   8 },	/* Dec  75 'K' */
	{  66,  79,   6,   8 },	/* Dec  76 'L' */
	{  72,  79,   6,   8 },	/* Dec  77 'M' */
	{  78,  79,   6,   8 },	/* Dec  78 'N' */
	{  84,  79,   6,   8 },	/* Dec  79 'O' */
	{  90,  79,   6,   8 },	/* Dec  80 'P' */
	{  96,  79,   6,   8 },	/* Dec  81 'Q' */
	{ 102,  79,   6,   8 },	/* Dec  82 'R' */
	{ 108,  79,   6,   8 },	/* Dec  83 'S' */
	{ 114,  79,   6,   8 },	/* Dec  84 'T' */
	{ 120,  79,   6,   8 },	/* Dec  85 'U' */
	{ 126,  79,   6,   8 },	/* Dec  86 'V' */
	{ 132,  79,   6,   8 },	/* Dec  87 'W' */
	{ 138,  79,   6,   8 },	/* Dec  88 'X' */
	{ 144,  79,   6,   8 },	/* Dec  89 'Y' */
	{ 150,  79,   6,   8 },	/* Dec  90 'Z' */
	{  42, 112,   6,   8 },	/* Dec  91 '[' */
	{  48, 112,   6,   8 },	/* Dec  92 '\' */
	{  54, 112,   6,   8 },	/* Dec  93 ']' */
	{  60, 112,   6,   8 },	/* Dec  94 '^' */
	{  66, 113,   6,   8 },	/* Dec  95 '_' */
	{  72, 112,   6,   8 },	/* Dec  96 '`' */
	{   0,  90,   6,   8 },	/* Dec  97 'a' */
	{   6,  90,   6,   8 },	/* Dec  98 'b' */
	{  12,  90,   6,   8 },	/* Dec  99 'c' */
	{  18,  90,   6,   8 },	/* Dec 100 'd' */
	{  24,  90,   6,   8 },	/* Dec 101 'e' */
	{  30,  90,   6,   8 },	/* Dec 102 'f' */
	{  36,  90,   6,   8 },	/* Dec 103 'g' */
	{  42,  90,   6,   8 },	/* Dec 104 'h' */
	{  48,  90,   6,   8 },	/* Dec 105 'i' */
	{  54,  90,   6,   8 },	/* Dec 106 'j' */
	{  60,  90,   6,   8 },	/* Dec 107 'k' */
	{  66,  90,   6,   8 },	/* Dec 108 'l' */
	{  72,  90,   6,   8 },	/* Dec 109 'm' */
	{  78,  90,   6,   8 },	/* Dec 110 'n' */
	{  84,  90,   6,   8 },	/* Dec 111 'o' */
	{  90,  90,   6,   8 },	/* Dec 112 'p' */
	{  96,  90,   6,   8 },	/* Dec 113 'q' */
	{ 102,  90,   6,   8 },	/* Dec 114 'r' */
	{ 108,  90,   6,   8 },	/* Dec 115 's' */
	{ 114,  90,   6,   8 },	/* Dec 116 't' */
	{ 120,  90,   6,   8 },	/* Dec 117 'u' */
	{ 126,  90,   6,   8 },	/* Dec 118 'v' */
	{ 132,  90,   6,   8 },	/* Dec 119 'w' */
	{ 138,  90,   6,   8 },	/* Dec 120 'x' */
	{ 144,  90,   6,   8 },	/* Dec 121 'y' */
	{ 150,  90,   6,   8 },	/* Dec 122 'z' */
	{  78, 112,   6,   8 },	/* Dec 123 '{' */
	{  84, 112,   6,   8 },	/* Dec 124 '|' */
	{  90, 112,   6,   8 },	/* Dec 125 '}' */
	{  60, 112,   6,   8 },	/* Dec 126 '~' */
	{  60, 102,   6,   8 }	/* Dec 127 del NOTUSED */
};

/*
 * usage
 */
void
usage(int mode)
{
	extern char	*__progname;

	if (mode) {
		fprintf(stderr, "%s %s\n", __progname, version);
		exit(1);
	}

	fprintf(stderr, "usage: %s ", __progname);
	fprintf(stderr, "[-Dhvw] [-i interface] [-s scrolling] ");
	fprintf(stderr, "[-display display]\n\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "  -D\t\t: Debug mode.\n");
	fprintf(stderr, "  -h\t\t: This help.\n");
	fprintf(stderr, "  -v\t\t: Shows version.\n");
	fprintf(stderr, "  -w\t\t: Enable WaveLAN compatibility.\n");
	fprintf(stderr, "  -i\t\t: Set interface.\n");
	fprintf(stderr, "  -s\t\t: Network ID scrolling.\n");
	fprintf(stderr, "    \t\t  0 = Disable 1 = LCD (default) 2 = Bounce ");
	fprintf(stderr, "3 = Fade\n");
	fprintf(stderr, "  -display\t: Set display name.\n");

	exit(1);
}

/*
 * signal handler
 */
void
signal_handler(const int sig)
{
	switch (sig) {
	case SIGINT:
		/* FALLTHROUGH */
	case SIGTERM:
		quit = 1;
		break;
	case SIGCHLD:
		/* ignore */
		break;
	case SIGHUP:
		/* ignore */
		break;
	case SIGQUIT:
		/* ignore */
		break;
	case SIGALRM:
		/* ignore */
		break;
	case SIGPIPE:
		/* ignore */
		break;
	default:
		/* ignore */
		break;
	}
}

/*
 * main()
 *	wireless monitor wmdockapp
 * Return:
 *	0
 */
int
main(int argc, char *argv[])
{
	int		i, r, ch, opt_wavelan = 0;
	int		opt_debug = 0, nic_status = 1, opt_scroll = 1;
	char		*opt_interface = NULL, *opt_display = NULL;
	char		*nwid = NULL, *speed = NULL, *status = NULL;
	char		sig[4], chn[4];
#ifdef __OpenBSD__
	const char	*errstr;
#endif
	XEvent		event;

	/*
	 * get command line options
	 */

	/* keep the old good -display tradition */
	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-display"))
			argv[i] = "-d";
	}

	while ((ch = getopt(argc, argv, "Dd:hi:s:vw")) != -1) {
		switch (ch) {
		case 'D':
			opt_debug = 1;
			break;
		case 'd':
			opt_display = strdup(optarg);
			break;
		case 'i':
			opt_interface = strdup(optarg);
			break;
		case 's':
#ifdef __OpenBSD__
			opt_scroll = strtonum(optarg, 0, 3, &errstr);
			if (errstr)
				usage(0);
#else
			opt_scroll = strtol(optarg, 0, 10);
#endif
			break;
		case 'v':
			usage(1);
			break;
		case 'w':
			opt_wavelan = 1;
			break;
		case 'h':
			/* FALLTHROUGH */
		default:
			usage(0);
			/* NOTREACHED */
		}
	}

	/*
	 * get interface
	 */
	if (opt_interface == NULL) {
		/* scan for interface */
		opt_interface = get_first_wnic();
		if (opt_interface == NULL) {
			fprintf(stderr, "No wireless interface found\n");
			exit(1);
		}
	} else {
		/* check defined interface */
		if (!check_nic(opt_interface)) {
			fprintf(stderr, "%s: no such interface\n",
			    opt_interface);
			exit(1);
		} else {
			if (get_wep(opt_interface) == -1) {
				fprintf(stderr, "%s: not a wireless ",
				    opt_interface);
				fprintf(stderr, "interface\n");
				exit(1);
			}
		}
	}
	/* is the interface wi? */
	if ((strncmp(opt_interface, "wi", 2) == 0) && isdigit(opt_interface[2]))
		opt_wavelan = 1;

	/*
	 * install signal handler
	 */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGCHLD, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGALRM, signal_handler);
	signal(SIGPIPE, signal_handler);

	/*
	 * run in debug mode
	 */
	if (opt_debug)
		debugloop(opt_interface); /* does not return */

	/*
	 * init X window
	 */
	initXwindow(opt_display);

	/*
	 * open X window
	 */
	openXwindow(argc, argv, wmwlmon_master, wmwlmon_mask_bits,
	    wmwlmon_mask_width, wmwlmon_mask_height);

	/*
	 * main loop
	 */
	for (i = 100; !quit; i++) {
		/*
		 * get wirless network state and draw it
		 */
		if (i == 100) {
			/* reset all */
			i = 0;
			copyXPMArea(65, 0, 64, 64, 0, 0);

			/* check if interface still exists */
			nic_status = check_nic(opt_interface);
			if (!nic_status)
				continue;

			/* interface name */
			draw_string(opt_interface, 6, 7);

			/* network status */
			status = get_status(opt_interface);
			if (!strcmp(status, "active"))
				copyXPMArea(0, 67, 4, 4, 53, 8);

			/* network id */
			nwid = get_nwid(opt_interface);

			/* speed */
			speed = get_speed(opt_interface);
			if (speed != NULL) {
				copyXPMArea(28, 67, 23, 8, 29, 36);
				if (strlen(speed) == 1)
					draw_string(speed, 18, 36);
				if (strlen(speed) == 2)
					draw_string(speed, 12, 36);
				if (strlen(speed) == 3)
					draw_string(speed, 6, 36);
			}

			/* signal strength */
			if (opt_wavelan)
				r = get_wi_signal(opt_interface);
			else
				r = get_signal(opt_interface, nwid);

			if (r > 0) {
				copyXPMArea(15, 67, 10, 6, 30, 29);
				snprintf(sig, sizeof(sig), "%d", r);
				if (strlen(sig) == 1)
					draw_string(sig, 18, 29);
				if (strlen(sig) == 2)
					draw_string(sig, 12, 29);
				if (strlen(sig) == 3)
					draw_string(sig, 6, 29);
				if (speed != NULL)
					draw_signal(atoi(speed), r);
			}

			/* channel */
			r = get_channel(opt_interface);
			if (r > 0) {
				copyXPMArea(61, 67, 10, 6, 30, 44);
				snprintf(chn, sizeof(sig), "%d", r);
				if (strlen(chn) == 1)
					draw_string(chn, 18, 44);
				if (strlen(chn) == 2)
					draw_string(chn, 12, 44);
				if (strlen(chn) == 3)
					draw_string(chn, 6, 44);
			}

			/* wep */
			r = get_wep(opt_interface);
			if (r > 0)
				copyXPMArea(54, 67, 4, 6, 48, 44);
		}

		if (nwid != NULL && i % 2 == 0 && nic_status) {
			/* reset nwid area */
			copyXPMArea(70, 50, 54, 9, 5, 50);
			draw_nwid(nwid, opt_scroll);
		}

		/*
	 	 * process pending X events
	 	 */
		while (XPending(display)) {
			XNextEvent(display, &event);
			switch(event.type) {
			case Expose:
				RedrawWindow();
				break;
			}
		}

		/*
		 * redraw
		 */
		RedrawWindow();
		usleep(DELAY);
	}

	exit(0);
}

void
debugloop(char *interface)
{
	char	*status, *nwid, *speed;
	int	r;

	/*
	 * debug loop
	 */
	while (!quit) {
		/* check if interface still exists */
		if (!check_nic(interface)) {
			printf("NIC gone!\n");
			sleep(2);
			continue;
		}

		/* interface name */
		printf("NIC:\t %s\n", interface);

		/* network status */
		status = get_status(interface);
		if (status != NULL)
			printf("Status:\t %s\n", status);

		/* network id */
		nwid = get_nwid(interface);
		if (nwid != NULL)
			printf("NwID:\t %s\n", nwid);

		/* speed */
		speed = get_speed(interface);
		if (speed != NULL)
			printf("Speed:\t %s Mbps\n", speed);

		/* signal strength */
		r = get_signal(interface, nwid);
		if (r > 0)
			printf("Signal:\t %d dB\n", r);

		/* channel */
		r = get_channel(interface);
		if (r > 0)
			printf("Channel: %d\n", r);

		/* wep */
		r = get_wep(interface);
		if (r > 0)
			printf("WEP:\t enabled\n");
		else
			printf("WEP:\t disabled\n");

		printf("\n");
		sleep(2);
	}

	exit(0);
}

void
draw_nwid(char *nwid, int scroll)
{
	static int	offset = 0;
	static char	*save_nwid;
	char		 network_id_cut[NWIDLENC + 1];

	if (strlen(nwid) > NWIDLENC) {
		if (save_nwid == NULL || strcmp(nwid, save_nwid)) {
			free(save_nwid);
			if ((save_nwid = strdup(nwid)) == NULL)
				err(1, NULL);
			offset = 0;
		}
		switch (scroll) {
		case 0:
			/* off */
			strlcpy(network_id_cut, nwid, sizeof(network_id_cut));
			draw_string(network_id_cut, 6, 51);
			break;
		case 1:
			/* lcd */
			offset = scroll_lcd(0, 124, 6, 51, offset, nwid);
			break;
		case 2:
			/* bounce */
			offset = scroll_bounce(0, 124, 6, 51, offset, nwid);
			break;
		case 3:
			/* fade */
			offset = scroll_fade(0, 124, 6, 51, offset, nwid);
			break;
		}
	} else
		draw_string(nwid, 6, 51);
}

/*
 * draw string
 */
void
draw_string(const char *string, const int x, const int y)
{
	int	c, i, offset;

	for (i = 0, offset = x; string[i] != '\0'; i++) {
		c = string[i];

		copyXPMArea(fonts[c].x, fonts[c].y, fonts[c].w, fonts[c].h,
		    offset, y);
		offset += fonts[c].w;
	}
}

/*
 * calculate the signal strength in percents and draw it
 */
void
draw_signal(const int speed, const int signal)
{
	int	i, bars, loop, offset;
	float	have, need;

	bars = 26;
	need = speed / 1.10;
	have = (signal * bars) / need;
	if (have >= bars)
		loop = bars;
	else
		loop = floor(have);

	for (i = 0, offset = 6; i < loop; i++, offset += 2)
		copyXPMArea(10, 67, 1, 6, offset, 21);
}

/*
 * scroll_lcd()
 *	scrolls a string from left to right separated by a space
 * Return:
 *	offset = success
 */
int
scroll_lcd(const int src_x, const int src_y, const int dst_x,
    const int dst_y, const int offset, const char *nwid)
{
	int	len, pos, frame;

	/* copy offset */
	pos = offset;

	/* draw network id */
	if (pos == src_x) {
		copyXPMArea(0, 132, 160, 8, src_x, src_y);
		draw_string(nwid, src_x, src_y);
	}

	/* calculate image length */
	len = (strlen(nwid) + 1) * 6;

	/* reached end of source, reset */
	if (pos == len)
		pos = src_x;

	/* copy scroll frame to destination */
	frame = len - pos;
	if (frame >= NWIDLENP)
		copyXPMArea(pos, src_y, NWIDLENP, 8, dst_x, dst_y);
	else {
		copyXPMArea(pos, src_y, frame, 8, dst_x, dst_y);
		copyXPMArea(src_x, src_y, NWIDLENP - frame, 8, dst_x + frame,
		    dst_y);
	}

	/* move to next pixel */
	pos++;

	return (pos);
}

/*
 * scroll_bounce()
 *	scrolls a string from left to right and bounces back from right to left
 * Return:
 *	offset = success
 */
int
scroll_bounce(const int src_x, const int src_y, const int dst_x,
    const int dst_y, const int offset, const char *nwid)
{
	int		len, pos, frame;
	static int	delay = 0, direction = 0;

	/* copy offset */
	pos = offset;

	/* draw network id */
	if (pos == src_x) {
		copyXPMArea(0, 132, 160, 8, src_x, src_y);
		draw_string(nwid, src_x, src_y);
	}

	/* calculate image length */
	len = strlen(nwid) * 6;

	/* delay */
	if (delay > 0) {
		if (direction == 0)
			copyXPMArea(pos - 1, src_y, NWIDLENP, 8, dst_x, dst_y);
		if (direction == 1)
			copyXPMArea(pos + 1, src_y, NWIDLENP, 8, dst_x, dst_y);
		delay--;

		return (pos);
	}

	/* start */
	if (pos == src_x) {
		delay = 10;
		direction = 0;
	}
	/* end */
	frame = len - pos;
	if (frame == NWIDLENP) {
		delay = 10;
		direction = 1;
	}

	/* copy scroll frame to destination */
	copyXPMArea(pos, src_y, NWIDLENP, 8, dst_x, dst_y);

	/* move to next pixel */
	if (direction == 0)
		pos++;
	if (direction == 1)
		pos--;

	return (pos);
}

/*
 * scroll_fade()
 *	scrolls a string from left to right, fading it out and begins again
 * Return:
 *	offset = success
 */
int
scroll_fade(const int src_x, const int src_y, const int dst_x,
    const int dst_y, const int offset, const char *nwid)
{
	int		len, pos, frame;
	static int	delay = 0;

	/* copy offset */
	pos = offset;

	/* draw network id */
	if (pos == src_x) {
		copyXPMArea(0, 132, 160, 8, src_x, src_y);
		draw_string(nwid, src_x, src_y);
	}

	/* calculate image length */
	len = (strlen(nwid) + 1) * 6;

	/* delay */
	if (delay > 0) {
		copyXPMArea(pos - 1, src_y, NWIDLENP, 8, dst_x, dst_y);
		delay--;

		return (pos);
	}

	/* end */
	if (pos == len)
		pos = src_x;
	/* start */
	if (pos == src_x)
		delay = 10;

	/* copy scroll frame to destination */
	frame = len - pos;
	if (frame >= NWIDLENP)
		copyXPMArea(pos, src_y, NWIDLENP, 8, dst_x, dst_y);
	else
		copyXPMArea(pos, src_y, frame, 8, dst_x, dst_y);

	/* move to next pixel */
	pos++;

	return (pos);
}
