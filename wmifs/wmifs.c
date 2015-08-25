/*  WMiFS - a complete network monitoring dock.app
    Copyright (C) 1997, 1998 Martijn Pieterse <pieterse@xs4all.nl>
    Copyright (C) 1997, 1998 Antoine Nulle <warp@xs4all.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

	Best viewed with vim5, using ts=4

	This code was mainly put together by looking at the
	following programs:

	asclock
		A neat piece of equip, used to display the date
		and time on the screen.
		Comes with every AfterStep installation.

		Source used:
			How do I create a not so solid window?
			How do I open a window?
			How do I use pixmaps?

	pppstats
		A program that prints the amount of data that
		is transferred over a ppp-line.

		Source used:
			How do I read the ppp device?
	------------------------------------------------------------

	Author: Martijn Pieterse (pieterse@xs4all.nl)

	This program was hacked together between the 7th of March
	and the 14th of March 1998.

	This program might be Y2K resistant. We shall see. :)

	This program is distributed under the GPL license.
	(as were asclock and pppstats)

	Known Features: (or in non M$ talk, BUGS)
		* only ppp0 will be read
			use wmifs if you want to read more than one ppp connection
			not sure about this.
		* the leds won't be reliable with
		  more than 1 ppp connection
		* there is an iconwin, and win variable.
		  I have no clue why only win shouldn't
		  be enough. Will check it out later.
		* The afterstep what seems the shift the
		  pixmap a bit. Don't know how and why.
		  It works in the WindowManager.

	Things to do:
		Split up main()

	----
	Thanks
	----

	Most of the ideas, and jumpstarting it:

	#linuxnl, without this irc-channel wmppp would've never seen the light!

	CCC (Constructive Code Criticism):

	Marcelo E. Magallon
		Thanks a LOT! It takes a while to get me convinced... :)


	Minor bugs and ideas:

	Marc De Scheemaecker / David Mihm / Chris Soghoian /
	Alessandro Usseglio Viretta

	and ofcourse numerous ppl who send us bug reports.
	(numerous? hmm.. todo: rephrase this :) )


	----
	Changes:
	---
	02/29/2004 (Tom Marshall, tommy@home.tig-grr.com)
		* Patch to add a special interface name "auto" for the -i
		  option. "wmifs -i auto" will automatically select the
		  first up interface.
	01/08/2004 (Peter Samuelson, peter@samba-tng.org)
		* Patch to make make sampling and scrolling intervals
		  customizable, adds new options -I and -s.
	01/15/2002 (Matyas Koszik, koszik@debijan.lonyay.edu.hu)
		* Patch that fixes segfaults on long interface names.
	08/31/2001 (Davi Leal, davileal@terra.es)
		* Patch that cuts long interface names, so they look
		  good in wmifs. For example, "dummy0" gets displayed
		  as "dumm0", "vmnet10" as "vmn10", etc.
	06/16/2001 (Jorge García, Jorge.Garcia@uv.es)
		* Added the LockMode, so wmifs doesn't swap to another
		  interface if the one requested with "-i" isn't up.
	05/06/2001 (Jordi Mallach, jordi@sindominio.net)
		* Integrated many patches, fixing issues with suspended
		  wmifs.
	07/21/1999 (Stephen Pitts, smpitts@midsouth.rr.com)
		* Added new constant: BUFFER_SIZE to determine the size
		  of the buffer used in fgets() operations. Right now,
		  its at 512 bytes. Fixed crashing on my system when
		  one line of /proc/net/dev was longer than 128 bytes
	04/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Changed the "middle of the waveform" line color
		* Moved the RedrawWindow out of the main loop.
		  Lightens the system load
	02/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Torn wmppp and wmifs apart.
		  This is gonna be wmifs
		* Added parse_rcfile
		* Added waitpid, to get rid of the zombies, spotteb by Alessandro Usseglio Viretta
	30/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Used the DrawStats routine from wmifs in wmppp
		* I decided to add a list in this source file
		  with name of ppl who helped me build this code better.
		* I finally removed the /proc/net/route dependancy
		  All of the connections are taken from /proc/net/dev.
		  /proc/net/route is still used for checking if it is on-line.
	27/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* WMIFS: stats scrolled, while red led burning
		* WMPPP: changed positions of line speed
	25/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Changed the checknetdevs routine, a lot!
	23/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added line speed detection. via seperate exec. (this has to be suid root!)
		  Speed has to be no more than 99999
		* Added speed and forcespeed in ~/.wmppprc and /etc/wmppprc
		* wmifs: added on-line detection scheme, update the bitmap coordinates
		* wmppp: the x-button now allways disconnects.
	22/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added /etc/wmppprc support, including "forced" mode.
		* Added /etc/wmifsrc support, including "forced" mode.
	21/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Moved the stats one pixel down.
		* Added status led in wmifs.
		* Changed RX/TX leds of wmifs to resemble wmppp
		* Added the "dot" between eth.0 ppp.0 etc.
		* Changed to wmifs stats to match wmppp stats (only pppX changed)
		* Made sure that when specified -t 1, it stayed that way, even
		  when longer than 100 minutes online
		* With -t 1, jump from 59:59 to 01:00 instead of 99:59 to 01:40
	16/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added "all" devices in wmifs
		* Added "lo" support only if aked via -i
		* Added on-line time detection (using /var/run/ppp0.pid)
		* Added time-out for the orange led. (one minute)
	15/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Another wmppp-master.xpm.
			Line speed detection being the main problem here.. :(
		* Moved START_COMMAND / STOP_COMMAND to ~/.wmppprc
			Return 0, everything went ok.
			Return 10, the command did not work.
			Please note, these functions are ran in the background.
		* Removed the ability to configure
		* When "v" is clicked, an orange led will appear.
		  if the connect script fails (return value == 10)
		  it will become dark again. Else the on-line detection will
		  pick it up, and "green" it.
	14/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added "-timer"
		* Added "-display" support
		* Changed pixmap to a no-name pixmap.
			+ Changed LED positions
			+ Changed Timer position
			+ Changed Stats Size
	05/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added ~/.wmifsrc support.
		* Put some code in DrawStats
		* Check devices when pressing "device change"
	03/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added code for wmifs
	28/03/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* forgot what i did.. :)
	27/03/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added on-line detection
			Scan through /proc/net/route and check everye line
			for "ppp".
		* A bit of code clean-up.
*/

#define _DEFAULT_SOURCE
#include <X11/X.h>                     /* for ButtonPress, ButtonRelease, etc */
#include <X11/Xlib.h>                  /* for XEvent, XButtonEvent, etc */
#include <X11/xpm.h>                   /* for XpmColorSymbol, Pixel, etc */
#include <ctype.h>                     /* for toupper */
#include <linux/ppp_defs.h>            /* for ppp_stats, pppstat */
#include <net/if_ppp.h>                /* for ifpppstatsreq, ifr__name, etc */
#include <stddef.h>                    /* for size_t */
#include <stdio.h>                     /* for fprintf, NULL, stderr, etc */
#include <stdlib.h>                    /* for exit, atof, atoi, getenv */
#include <string.h>                    /* for strcmp, strcpy, strlen, etc */
#include <sys/ioctl.h>                 /* for ioctl */
#include <sys/socket.h>                /* for socket, AF_INET */
#include <sys/time.h>                  /* for timeval, gettimeofday */
#include <sys/wait.h>                  /* for waitpid, WNOHANG */
#include <time.h>                      /* for timespec, nanosleep */
#include <libdockapp/misc.h>           /* for execCommand */
#include <libdockapp/wmgeneral.h>      /* for copyXPMArea, display, etc */
#include "wmifs-mask.xbm"              /* for wmifs_mask_bits, etc */
#include "wmifs-master.xpm"            /* for wmifs_master_xpm */

/* How often to check for new network interface, in ms */
#define CHECK_INTERFACE_INTERVAL 5000

/* How often to query the interfaces, in ms */
#define DEFAULT_SAMPLE_INTERVAL 50

  /***********/
 /* Defines */
/***********/

#ifndef ifr__name
#define ifr__name ifr_name
#endif

#ifndef stats_ptr
#define stats_ptr stats.p.FIXME
#endif

/* Fill in and uncomment the hardcoded actions */
/* #define LEFT_ACTION (NULL) */
/* #define MIDDLE_ACTION (NULL) */
/* #define RIGHT_ACTION (NULL) */

/* Defines voor alle coordinate */

#define LED_NET_RX			(4)
#define LED_NET_TX			(5)
#define LED_NET_POWER		(6)

#define WMIFS_VERSION "1.6"

/* the size of the buffer read from /proc/net/ */
#define BUFFER_SIZE 512
  /**********************/
 /* External Variables */
/**********************/

extern	char **environ;

  /********************/
 /* Global Variables */
/********************/

char	*active_interface = NULL;
int		TimerDivisor = 60;
int		WaveForm = 0;
int		LockMode = 0;
int		SampleInt = DEFAULT_SAMPLE_INTERVAL;
int		ScrollSpeed = CHECK_INTERFACE_INTERVAL;
XpmIcon wmgen;
char color[256];

  /*****************/
 /* PPP variables */
/*****************/

#define		PPP_UNIT		0
int			ppp_h = -1;

#define		PPP_STATS_HIS	54

  /***********************/
 /* Function Prototypes */
/***********************/

void usage(void);
void printversion(void);
void DrawTime(int, int);
void DrawStats(int *, int, int, int, int);

void SetOnLED(int);
void SetErrLED(int);
void SetWaitLED(int);
void SetOffLED(int);

void ButtonUp(int);
void ButtonDown(int);

void wmifs_routine(int, char **);

void get_ppp_stats(struct ppp_stats *cur);


  /********/
 /* Main */
/********/

int main(int argc, char *argv[])
{

	int		i;

	color[0] = 0;

	/* Parse Command Line */

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];

		if (*arg == '-') {
			switch (arg[1]) {
			case 'c' :
				if (argc > i+1) {
					strcpy(color, argv[i+1]);
					i++;
				}
				break;
			case 'd':
				if (strcmp(arg+1, "display")) {
					usage();
					exit(1);
				}
				break;
			case 'g':
				if (strcmp(arg+1, "geometry")) {
					usage();
					exit(1);
				}
				break;
			case 'i':
				active_interface = argv[i+1];
				i++;
				break;
			case 'I':
				SampleInt = atof(argv[i+1]) * 1000;
				i++;
				break;
			case 'l':
				LockMode = 1;
				break;
			case 's':
				ScrollSpeed = atof(argv[i+1]) * 1000;
				i++;
				break;
			case 'v':
				printversion();
				exit(0);
				break;
			case 'w':
				WaveForm = 1;
				break;
			default:
				if (strcmp(argv[i-1], "-geometry")) {
					usage();
					exit(0);
				}
				break;
			}
		}
	}

	wmifs_routine(argc, argv);
	return 0;
}

Pixel scale_pixel(Pixel pixel, float scale)
{
       int red, green, blue;

       red = pixel / ( 1 << 16 );
       green = pixel % (1 << 16) / (1 << 8);
       blue = pixel % (1 << 8);

       red *= scale;
       green *= scale;
       blue *= scale;

       return red * (1 << 16) + green * (1 << 8) + blue;
}

/*******************************************************************************\
|* wmifs_routine															   *|
\*******************************************************************************/

#define MAX_STAT_DEVICES 16

typedef struct {

	char	name[8];
	int	his[55][2];
	long	istatlast;
	long	ostatlast;

} stat_dev;

stat_dev	stat_devices[MAX_STAT_DEVICES];

char		*left_action = NULL;
char		*right_action = NULL;
char		*middle_action = NULL;

int checknetdevs(void);
int get_statistics(char *, long *, long *, long *, long *);
int stillonline(char *);
void DrawActiveIFS(char *);

void wmifs_routine(int argc, char **argv)
{

	rckeys	wmifs_keys[] = {
		{ "left", &left_action },
		{ "middle", &middle_action },
		{ "right", &right_action },
		{ NULL, NULL }
	};


	int			i, j;
	XEvent		Event;
	int			but_stat = -1;

	int			stat_online;
	int			stat_current;
	int			first_time = 1;

	unsigned int	curtime;
	unsigned int	nexttime;
	struct timeval	tv, tv2;

	long		ipacket, opacket, istat, ostat;

	char		temp[BUFFER_SIZE];
	char		*p;

	for (i = 0; i < MAX_STAT_DEVICES; i++) {
		stat_devices[i].name[0] = 0;
		for (j = 0; j < 48; j++) {
			stat_devices[i].his[j][0] = 0;
			stat_devices[i].his[j][1] = 0;
		}
	}

	stat_online = checknetdevs();

	stat_current = 0;
	if (active_interface) {
		int isauto = !strcmp(active_interface, "auto");
		for (i = 0; i < stat_online; i++) {
			if ((isauto && stillonline(stat_devices[i].name)) ||
			    !strcmp(stat_devices[i].name, active_interface)) {
				stat_current = i;
				break;
			}
		}
	}

#ifdef LEFT_ACTION
	left_action = strdup(LEFT_ACTION);
#endif
#ifdef MIDDLE_ACTION
	middle_action = strdup(MIDDLE_ACTION);
#endif
#ifdef RIGHT_ACTION
	right_action = strdup(RIGHT_ACTION);
#endif

	/* Scan throught the .rc files */
	parse_rcfile(CONF"/wmifsrc", wmifs_keys);

	p = getenv("HOME");
	if (p == NULL || *p == 0) {
		fprintf(stderr, "Unknown $HOME directory, please check your environment\n");
		return;
	}
	strcpy(temp, p);
	strcat(temp, "/.wmifsrc");
	parse_rcfile(temp, wmifs_keys);

	parse_rcfile(CONF"/wmifsrc.fixed", wmifs_keys);

       /* set user-defined colors */
       if (color[0] != 0) {
               Window  Root;
               XColor col;
               XWindowAttributes attributes;
               int screen;
               Pixel pixel;
#define NUMSYMBOLS 4
               XpmColorSymbol user_color[NUMSYMBOLS] = {
                       {NULL, "#2081B2CAAEBA", 0}, /* + */
                       {NULL, "#28A23CF338E3", 0}, /* O */
                       {NULL, "#000049244103", 0}, /* @ */
                       {NULL, "#18618A288617", 0}, /* # */
                        };


               /* code based on GetColor() from wmgeneral.c */
               /* we need a temporary display to parse the color */
               display = XOpenDisplay(NULL);
               screen = DefaultScreen(display);
               Root = RootWindow(display, screen);
               XGetWindowAttributes(display, Root, &attributes);

               col.pixel = 0;
               if (!XParseColor(display, attributes.colormap, color, &col)) {
                       fprintf(stderr, "wmtime: can't parse %s.\n", color);
                       goto draw_window;
               } else if (!XAllocColor(display, attributes.colormap, &col)) {
                       fprintf(stderr, "wmtime: can't allocate %s.\n", color);
                       goto draw_window;
               }

               pixel = col.pixel;

               /* replace colors from wmtime-master.xpm */
               user_color[0].pixel = pixel;
               user_color[1].pixel = scale_pixel(pixel, .3);
               user_color[2].pixel = scale_pixel(pixel, .4);
               user_color[3].pixel = scale_pixel(pixel, .8);

               wmgen.attributes.valuemask |= XpmColorSymbols;
               wmgen.attributes.numsymbols = NUMSYMBOLS;
               wmgen.attributes.colorsymbols = user_color;

               XCloseDisplay(display);
       }

draw_window:
	openXwindow(argc, argv, wmifs_master_xpm, (char*)wmifs_mask_bits, wmifs_mask_width, wmifs_mask_height);

	/* > Button */
	AddMouseRegion(0, 5, 5, 35, 15);
	AddMouseRegion(1, 5, 20, 58, 58);

	gettimeofday(&tv2, NULL);
	nexttime = ScrollSpeed;

	DrawActiveIFS(stat_devices[stat_current].name);

	while (1) {
		struct timespec ts;

		gettimeofday(&tv, NULL);
		curtime = (tv.tv_sec - tv2.tv_sec) * 1000
			+ (tv.tv_usec - tv2.tv_usec) / 1000;

		waitpid(0, NULL, WNOHANG);

		for (i = 0; i < stat_online; i++) {
			get_statistics(stat_devices[i].name, &ipacket, &opacket, &istat, &ostat);

			if (first_time) {
				first_time = 0;
			} else {
				stat_devices[i].his[53][0] += istat - stat_devices[i].istatlast;
				stat_devices[i].his[53][1] += ostat - stat_devices[i].ostatlast;
			}

			if (i == stat_current) {
				if (!stillonline(stat_devices[i].name))
					SetErrLED(LED_NET_POWER);
				else
					SetOnLED(LED_NET_POWER);

				if (stat_devices[i].istatlast == istat)
					SetOffLED(LED_NET_RX);
				else
					SetOnLED(LED_NET_RX);

				if (stat_devices[i].ostatlast == ostat)
					SetOffLED(LED_NET_TX);
				else
					SetOnLED(LED_NET_TX);
			}

			stat_devices[i].istatlast = istat;
			stat_devices[i].ostatlast = ostat;
		}
		RedrawWindow();

		if (curtime >= nexttime) {
			nexttime = curtime + ScrollSpeed;

			DrawStats(&stat_devices[stat_current].his[0][0], 54, 40, 5, 58);
			for (i = 0; i < stat_online; i++) {
				if (stillonline(stat_devices[i].name)) {
					for (j = 1; j < 54; j++) {
						stat_devices[i].his[j-1][0] = stat_devices[i].his[j][0];
						stat_devices[i].his[j-1][1] = stat_devices[i].his[j][1];
					}
					stat_devices[i].his[53][0] = 0;
					stat_devices[i].his[53][1] = 0;
				}
			}
			RedrawWindow();
		}

		while (XPending(display)) {
			XNextEvent(display, &Event);
			switch (Event.type) {
			case Expose:
				RedrawWindow();
				break;
			case DestroyNotify:
				XCloseDisplay(display);
				exit(0);
				break;
			case ButtonPress:
				but_stat = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				break;
			case ButtonRelease:
				i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);

				if (but_stat == i && but_stat >= 0) {
					switch (but_stat) {
					case 0:
						/* re-read the table */
						strcpy(temp, stat_devices[stat_current].name);
						stat_online = checknetdevs();
						stat_current = 0;
						for (i = 0; i < stat_online; i++) {
							if (!strcmp(temp, stat_devices[i].name))
								stat_current = i;
						}

						stat_current++;
						if (stat_current == stat_online)
							stat_current = 0;

						DrawActiveIFS(stat_devices[stat_current].name);

						DrawStats(&stat_devices[stat_current].his[0][0], 54, 40, 5, 58);
						break;
					case 1:
						switch (Event.xbutton.button) {
						case 1:
							if (left_action)
								execCommand(left_action);
							break;
						case 2:
							if (middle_action)
								execCommand(middle_action);
							break;
						case 3:
							if (right_action)
								execCommand(right_action);
							break;
						}
						break;

					}
				}
				but_stat = -1;
				RedrawWindow();
				break;
			}
		}
		ts.tv_sec = 0;
		ts.tv_nsec = SampleInt * 1000000;
		nanosleep(&ts, NULL);
	}
}

/*******************************************************************************\
|* void DrawActiveIFS(char *)												   *|
\*******************************************************************************/

void DrawActiveIFS(char *real_name)
{

	/* Cijfers op: 0,65
	   Letters op: 0,75
	   Alles 9 hoog, 6 breedt

	   Destinatie: 5,5
	*/

	size_t		i;
	int		k;
	size_t		len;
	char		name[256];


	copyXPMArea(5, 84, 30, 10, 5, 5);


	strcpy(name, real_name);
	len = strlen(name);
	if (len > 5) {
		for (i = len-5; i < len && !(name[i] >= '0' && name[i] <= '9'); i++)
			;
		for (; i <= len; i++) /* '=' to get the '\0' character moved too \*/
			name[i-(len-5)] = name[i];
	}

	k = 5;
	for (i = 0; name[i]; i++) {
		int c;

		if (i == strlen(name)-1 && strlen(name) <= 4 && name[strlen(name)-1] >= '0' &&
		    name[strlen(name)-1] <= '9') {
			copyXPMArea(61, 64, 4, 9, k, 5);
			k += 4;
		}
		c = toupper(name[i]);
		if (c >= 'A' && c <= 'Z') {
			c -= 'A';
			copyXPMArea(c * 6, 74, 6, 9, k, 5);
			k += 6;
		} else {
			c -= '0';
			copyXPMArea(c * 6, 64, 6, 9, k, 5);
			k += 6;
		}
	}

}

/*******************************************************************************\
|* get_statistics															   *|
\*******************************************************************************/

int get_statistics(char *devname, long *ip, long *op, long *is, long *os)
{

	FILE				*fp;
	char				temp[BUFFER_SIZE];
	char				*p, *saveptr;
	char				*tokens = " |:\n";
	int					input, output;
	int					i;
	int					found;
	struct ppp_stats	ppp_cur;


	if (!strncmp(devname, "ppp", 3)) {
		static int ppp_opened;

		if (!ppp_opened) {
			/* Open the ppp device. */
			memset(&ppp_cur, 0, sizeof(ppp_cur));
			ppp_h = socket(AF_INET, SOCK_DGRAM, 0);
			if (ppp_h < 0)
				return -1;
			get_ppp_stats(&ppp_cur);
			ppp_opened = 1;
		}

		get_ppp_stats(&ppp_cur);

		*op = ppp_cur.p.ppp_opackets;
		*ip = ppp_cur.p.ppp_ipackets;

		*is = ppp_cur.p.ppp_ibytes;
		*os = ppp_cur.p.ppp_obytes;

		return 0;
	}

	/* Read from /proc/net/dev the stats! */
	fp = fopen("/proc/net/dev", "r");
	if (!fgets(temp, BUFFER_SIZE, fp)) {
		fclose(fp);
		return -1;
	}
	if (!fgets(temp, BUFFER_SIZE, fp)) {
		fclose(fp);
		return -1;
	}

	input = -1;
	output = -1;
	i = 0;
	found = -1;

	p = strtok_r(temp, tokens, &saveptr);
	do {
		if (!(strcmp(p, "packets"))) {
			if (input == -1)
				input = i;
			else
				output = i;
		}
		i++;
		p = strtok_r(NULL, tokens, &saveptr);
	} while (input == -1 || output == -1);

	while (fgets(temp, BUFFER_SIZE, fp)) {
		if (strstr(temp, devname)) {
			found = 0;
			p = strtok_r(temp, tokens, &saveptr);
			i = 0;
			do {
				if (i == input) {
					*ip = *is = atoi(p);
					input = -1;
				}
				if (i == output) {
					*op = *os = atoi(p);
					output = -1;
				}
				i++;
				p = strtok_r(NULL, tokens, &saveptr);
			} while (input != -1 || output != -1);
		}
	}
	fclose(fp);

	return found;
}

/*******************************************************************************\
|* stillonline																   *|
\*******************************************************************************/

int stillonline(char *ifs)
{

	FILE	*fp;
	int		i;

	i = 0;
	fp = fopen("/proc/net/route", "r");
	if (fp) {
		char temp[BUFFER_SIZE];

		while (fgets(temp, BUFFER_SIZE, fp)) {
			if (strstr(temp, ifs)) {
				i = 1; /* Line is alive */
				break;
			}
		}
		fclose(fp);
	}
	return i;
}

/*******************************************************************************\
|* checknetdevs																   *|
\*******************************************************************************/

int checknetdevs(void)
{

	FILE	*fd;
	int		i = 0, j;
	int		k;
	int		devsfound = 0;
	char	foundbuffer[MAX_STAT_DEVICES][8];

	for (i = 0; i < MAX_STAT_DEVICES; i++)
		foundbuffer[i][0] = 0;

	/* foundbuffer vullen met info uit /proc/net/dev */

	fd = fopen("/proc/net/dev", "r");
	if (fd) {
		char temp[BUFFER_SIZE];

		/* Skip the first 2 lines */
		if (!fgets(temp, BUFFER_SIZE, fd)) {
			fclose(fd);
			return -1;
		}
		if (!fgets(temp, BUFFER_SIZE, fd)) {
			fclose(fd);
			return -1;
		}
		while (fgets(temp, BUFFER_SIZE, fd)) {
			char *p, *saveptr;
			char *tokens = " :\t\n";

			p = strtok_r(temp, tokens, &saveptr);
			if (p == NULL) {
					printf("Barfed on: %s", temp);
					break;
			}
			/* Skip dummy code */

			if (!strncmp(p, "dummy", 5))
				continue;

			/* If p == "lo", and active_interface (as given on the cmd line) != "lo",
			   skip it! */

			if (strcmp(p, "lo") || (active_interface && !strcmp(active_interface, "lo"))) {
				strcpy(foundbuffer[devsfound], p);
				devsfound++;
			}
			if (devsfound >= MAX_STAT_DEVICES)
				break;
		}
		fclose(fd);
	}

	/* Nu foundbuffer naar stat_devices[].name kopieeren */

	for (i = 0; i < MAX_STAT_DEVICES; i++) {
		/* Loop stat_devices na, als die naam niet voorkomt in foundbuffer, kill! */

		if (stat_devices[i].name[0]) {
			k = 0;
			for (j = 0; j < MAX_STAT_DEVICES; j++) {
				if (!strcmp(stat_devices[i].name, foundbuffer[j])) {
					k = 1;
					foundbuffer[j][0] = 0;
				}
			}
			if (!k)
				stat_devices[i].name[0] = 0;
		}
	}

	for (i = 0, j = 0; j < MAX_STAT_DEVICES; i++, j++) {

		while (!stat_devices[j].name[0] && j < MAX_STAT_DEVICES)
			j++;

		if (j < MAX_STAT_DEVICES && i != j)
			stat_devices[i] = stat_devices[j];
	}
	i--;

	for (j = 0; j < MAX_STAT_DEVICES; j++) {
		if (foundbuffer[j][0]) {

			strcpy(stat_devices[i].name, foundbuffer[j]);

			for (k = 0; k < 48; k++) {
				stat_devices[i].his[k][0] = 0;
				stat_devices[i].his[k][1] = 0;
			}

			i++;
		}
	}
	if (LockMode && active_interface != NULL) {
		k = 0;
		for (j = 0; j < i; j++)
			if (!strcmp(stat_devices[j].name, active_interface)) {
				k = 1;
				break;
			}
		if (!k) {
			strcpy(stat_devices[i].name, active_interface);
			for (k = 0; k < 48; k++) {
				stat_devices[i].his[k][0] = 0;
				stat_devices[i].his[k][1] = 0;
			}
			devsfound++;
		}

	}
	return devsfound;
}

/*******************************************************************************\
|* DrawStats																   *|
\*******************************************************************************/

void DrawStats(int *his, int num, int size, int x_left, int y_bottom)
{

	int		pixels_per_byte;
	int		j, k;
	int		*p;
	int		p2, p3;

	pixels_per_byte = size;
	p = his;
	for (j = 0; j < num; j++) {
		if (p[0] + p[1] > pixels_per_byte)
			pixels_per_byte = p[0] + p[1];
		p += 2;
	}

	pixels_per_byte /= size;
	p = his;

	for (k = 0; k < num; k++) {
		int p0, p1;

		p0 = p[0];
		p1 = p[1];


		if (WaveForm) {
			p2 = 0;
			p3 = 1;
			for (j = 0; j < size; j++) {
				if (j < p0 / pixels_per_byte)
					copyXPMArea(100+2, 68, 1, 1, k+x_left, y_bottom-size/2+p2/2);
				else if (j < (p0 + p1) / pixels_per_byte)
					copyXPMArea(100+1, 68, 1, 1, k+x_left, y_bottom-size/2+p2/2);
				else
					copyXPMArea(100+0, 68, 1, 1, k+x_left, y_bottom-size/2+p2/2);

				p2 = (p2 + p3);
				p3 *= -1;
				p2 *= -1;
			}
			copyXPMArea(100+3, 68, 1, 1, k+x_left, y_bottom-size/2);
		} else {
			for (j = 0; j < size; j++) {
				if (j < p0 / pixels_per_byte)
					copyXPMArea(100+2, 68, 1, 1, k+x_left, y_bottom-j);
				else if (j < (p0 + p1) / pixels_per_byte)
					copyXPMArea(100+1, 68, 1, 1, k+x_left, y_bottom-j);
				else
					copyXPMArea(100+0, 68, 1, 1, k+x_left, y_bottom-j);
			}
		}
		p += 2;
	}
}

/*******************************************************************************\
|* usage																	   *|
\*******************************************************************************/

void usage(void)
{

	fprintf(stderr, "\nwmifs - programming: tijno, (de)bugging & design: warpstah, webhosting: bobby\n\n");
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "\t-c <color>\t\tset color\n");
	fprintf(stderr, "\t-display <display name>\tset display\n");
	fprintf(stderr, "\t-geometry +x+y\t\tset window position\n");
	fprintf(stderr, "\t-h\t\t\tthis help screen\n");
	fprintf(stderr, "\t-i <interface name>\tdefault (as it appears in /proc/net/route)\n");
	fprintf(stderr, "\t-I <interval>\t\tsampling interval, in seconds (default: 0.05)\n");
	fprintf(stderr, "\t-l\t\t\tstarts in lock mode\n");
	fprintf(stderr, "\t-s <interval>\t\tscrolling interval, in seconds (default: 5)\n");
	fprintf(stderr, "\t-v\t\t\tprint the version number\n");
	fprintf(stderr, "\t-w\t\t\twaveform load\n");
	fprintf(stderr, "\n");
}

/*******************************************************************************\
|* printversion																   *|
\*******************************************************************************/

void printversion(void)
{

	fprintf(stderr, "%s\n", WMIFS_VERSION);
}

/*******************************************************************************\
|* get_ppp_stats															   *|
\*******************************************************************************/

void get_ppp_stats(struct ppp_stats *cur)
{

	struct ifpppstatsreq    req;

	memset(&req, 0, sizeof(req));

	req.stats_ptr = (void *) &req.stats;

	sprintf(req.ifr__name, "ppp%d", PPP_UNIT);

	if (ioctl(ppp_h, SIOCGPPPSTATS, &req) < 0) {
		/* fprintf(stderr, "heyho!\n") */;
	}
	*cur = req.stats;
}

#define LED_SZE_X (4)
#define LED_SZE_Y (4)

#define LED_ON_NET_X (87)
#define LED_ON_NET_Y (66)
#define LED_OFF_NET_X (93)
#define LED_OFF_NET_Y (66)
#define LED_ERR_NET_X (81)
#define LED_ERR_NET_Y (66)
#define LED_ON_SW_NET_X (49)
#define LED_ON_SW_NET_Y (85)
#define LED_OFF_SW_NET_X (44)
#define LED_OFF_SW_NET_Y (85)

#define LED_PWR_X (53)
#define LED_PWR_Y (7)
#define LED_SND_X (47)
#define LED_SND_Y (7)
#define LED_RCV_X (41)
#define LED_RCV_Y (7)

#define LED_SW_X (38)
#define LED_SW_Y (14)

/*******************************************************************************\
|* SetOnLED                                                                                                                                *|
\*******************************************************************************/
void SetOnLED(int led)
{

	switch (led) {

	case LED_NET_RX:
		copyXPMArea(LED_ON_NET_X, LED_ON_NET_Y, LED_SZE_X, LED_SZE_Y,  LED_RCV_X, LED_RCV_Y);
		break;
	case LED_NET_TX:
		copyXPMArea(LED_ON_NET_X, LED_ON_NET_Y, LED_SZE_X, LED_SZE_Y,  LED_SND_X, LED_SND_Y);
		break;
	case LED_NET_POWER:
		copyXPMArea(LED_ON_NET_X, LED_ON_NET_Y, LED_SZE_X, LED_SZE_Y,  LED_PWR_X, LED_PWR_Y);
		break;
	}
}

/*******************************************************************************\
|* SetOffLED                                                                                                                               *|
\*******************************************************************************/
void SetOffLED(int led)
{

	switch (led) {

	case LED_NET_RX:
		copyXPMArea(LED_OFF_NET_X, LED_OFF_NET_Y, LED_SZE_X, LED_SZE_Y,  LED_RCV_X, LED_RCV_Y);
		break;
	case LED_NET_TX:
		copyXPMArea(LED_OFF_NET_X, LED_OFF_NET_Y, LED_SZE_X, LED_SZE_Y,  LED_SND_X, LED_SND_Y);
		break;
	case LED_NET_POWER:
		copyXPMArea(LED_OFF_NET_X, LED_OFF_NET_Y, LED_SZE_X, LED_SZE_Y,  LED_PWR_X, LED_PWR_Y);
		break;
	}
}

/*******************************************************************************\
|* SetErrLED                                                                                                                               *|
\*******************************************************************************/
void SetErrLED(int led)
{

	switch (led) {
	case LED_NET_POWER:
		copyXPMArea(LED_ERR_NET_X, LED_ERR_NET_Y, LED_SZE_X, LED_SZE_Y,  LED_PWR_X, LED_PWR_Y);
		break;
	}
}
