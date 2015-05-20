/*
	Best viewed with vim5, using ts=4

	This code was mainly put together by looking at the
	following programs:

	asclock
		A neat piece of equip, used to display the date
		and time on the screen.
		Comes with every WindowMaker installation.

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

	Authors: Martijn Pieterse (pieterse@xs4all.nl)
			 Antoine Nulle (warp@xs4all.nl)

	This program might be Y2K resistant. We shall see. :)

	This program is distributed under the GPL license.
	(as were asclock and pppstats)

	Known Features: (or in non M$ talk, BUGS)
		* none known so far in this release

	----
	Thanks
	----

	CCC (Constructive Code Criticism):

	Marcelo E. Magallon
		Thanks a LOT! It takes a while to get me convinced... :)


	Minor bugs and ideas:

	Marc De Scheemaecker / David Mihm / Chris Soghoian /
	Alessandro Usseglio Viretta

	and ofcourse numerous ppl who send us bug reports.
	(numerous? hmm.. todo: rephrase this :) )
	Make that numerous m8ey :)

	----
	Changes:
	---
	05/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added:
			Speed-O-Meter (after 60 seconds)
			Fixed Error reporting when pressing X
			Removed the ugly kb lines
			Stopped clearing on-line time when pressing X
			Added createXBMfromXPM
	08/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Removed some code from get_statistics
		* Check if "ifdown" is empty before execCommanding it!
	07/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Made the program use the xpm like warp wanted it to be :)
	04/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added pppX support. (EXPERIMENTAL!)
		  Removed HARD_CODED_DEV. (that stayed in long! :) )
		* Changed 33600 speed indication to 33k6
		  Bugs if larger than 115k2 (depends on how much 1's present)
		  Moved the speed ind. code to DrawSpeedInd
		* Added 1k lines in the stats
		* Moved all the "ppp0" references into HARD_CODED_DEV.
		  for easy change
	03/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Removed the number after -t.
	02/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Removed the heyho code :)
		* Changed read_rc_file to parse_rcfile. suggested bt Marcelo E. Magallon
		* Added some extra checks for the -t option.
		  If no number was given, it would core dump
	30/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added execCommand code. (taken from windowmaker soure, as advised by Marcelo E. Magallon)
		* Cleaned the source up a bit
		* Decided to split op wmppp and wmifs
		  This is gonna be wmppp
		* Used the DrawStats routine from wmifs in wmppp
		* I decided to add a list in this source file
		  with name of ppl who helped me build this code better.
		* I finally removed the /proc/net/route dependency
		  All of the connections are taken from /proc/net/dev.
		  /proc/net/route is still used for checking if it is on-line.
	27/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* WMIFS: stats scrolled, while red led burning
		* WMPPP: changed positions of line speed
	25/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Changed the checknetdevs routine, a lot!
	23/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added line speed detection. via separate exec. (this has to be suid root!)
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

#include <X11/X.h>                     /* for ButtonPress, ButtonRelease, etc */
#include <X11/Xlib.h>                  /* for XEvent, XButtonEvent, etc */
#include <sys/socket.h>                /* for SOCK_DGRAM */
#include <linux/ppp_defs.h>            /* for ppp_stats, pppstat */
#include <net/if_ppp.h>                /* for ifpppstatsreq, etc */
#include <signal.h>                    /* for signal */
#include <stdio.h>                     /* for fprintf, stderr, NULL, etc */
#include <stdlib.h>                    /* for exit, atoi, getenv, etc */
#include <string.h>                    /* for strcpy, memset, strcmp, etc */
#include <sys/ioctl.h>                 /* for ioctl */
#include <sys/socket.h>                /* for socket, AF_INET */
#include <sys/stat.h>                  /* for stat, st_mtime */
#include <sys/types.h>                 /* for pid_t */
#include <sys/wait.h>                  /* for waitpid, WNOHANG */
#include <time.h>                      /* for timespec, nanosleep, time */
#include "wmgeneral/misc.h"            /* for execCommand */
#include "wmgeneral/wmgeneral.h"       /* for copyXPMArea, RedrawWindow, etc */
#include "wmppp-master.xpm"            /* for wmppp_master_xpm */

  /***********/
 /* Defines */
/***********/

/* Fill in and uncomment the hardcoded actions. */
/* #define START_ACTION (NULL) */
/* #define STOP_ACTION (NULL) */
/* #define SPEED_ACTION (NULL) */
/* #define IFDOWN_ACTION (NULL) */

#define STAMP_FILE_PRE "/var/run/wmppp."

/* Defines voor alle coordinate */

#define LED_PPP_RX			(1)
#define LED_PPP_TX			(2)
#define LED_PPP_POWER		(3)

#define BUT_V				(1)
#define BUT_X				(2)

#define TIMER_X				(9)
#define TIMER_Y				(14)

#define TIMER_SRC_Y			(65)
#define TIMER_DES_Y			(6)
#define TIMER_SZE_X			(6)

#define WMPPP_VERSION "1.3.1"

#define ORANGE_LED_TIMEOUT (60)

  /**********************/
 /* External Variables */
/**********************/

extern	char **environ;

  /********************/
 /* Global Variables */
/********************/

char	*ProgName;
char	*active_interface = "ppp0";
int		TimerDivisor=60;
int		updaterate = 5;

int wmppp_mask_width = 64;
int wmppp_mask_height = 64;
char wmppp_mask_bits[64*64];


  /*****************/
 /* PPP variables */
/*****************/

#define 	PPP_UNIT		0
int			ppp_h = -1;

#define		PPP_STATS_HIS	54

int		pixels_per_byte;

int		ppp_history[PPP_STATS_HIS+1][2];

  /***********************/
 /* Function Prototypes */
/***********************/

void usage(void);
void printversion(void);
void DrawTime(int, int);
void DrawStats(int *, int, int, int, int);
void DrawSpeedInd(char *);
void DrawLoadInd(int);

void SetOnLED(int);
void SetErrLED(int);
void SetWaitLED(int);
void SetOffLED(int);

void ButtonUp(int);
void ButtonDown(int);

int get_statistics(long *, long *, long *, long *);
void get_ppp_stats(struct ppp_stats *cur);
int stillonline(char *);
void reread(int);

char	*start_action = NULL;
char	*stop_action = NULL;
char	*speed_action = NULL;
char	*ifdown_action = NULL;
char    *stamp_file = NULL;

char	*start_action_cmdline = NULL;
char	*stop_action_cmdline = NULL;
char	*speed_action_cmdline = NULL;
char	*ifdown_action_cmdline = NULL;
char	*stamp_file_cmdline = NULL;

  /**********************/
 /* Parse Command Line */
/**********************/

int parse_cmdline(int argc, char *argv[]) {

	int		i;

	ProgName = argv[0];
	if (strlen(ProgName) >= 5)
		ProgName += (strlen(ProgName) - 5);

	for (i=1; i<argc; i++) {
		char *arg = argv[i];

		if (*arg=='-') {
			switch (arg[1]) {
			case 'd' :
				if (strcmp(arg+1, "display")) {
					usage();
					exit(1);
				}
				break;
			case 'g' :
				if (strcmp(arg+1, "geometry")) {
					usage();
					exit(1);
				}
				break;
			case 'i' :
				if (!strcmp(arg+1, "i"))
					active_interface = argv[++i];
				else if (!strcmp(arg+1, "ifdown"))
					ifdown_action_cmdline = argv[++i];
				else {
					usage();
					exit(1);
				}
				break;
			case 's' :
				if (!strcmp(arg+1, "speed"))
					speed_action_cmdline = argv[++i];
				else if (!strcmp(arg+1, "start"))
					start_action_cmdline = argv[++i];
				else if (!strcmp(arg+1, "stop"))
					stop_action_cmdline = argv[++i];
				else if (!strcmp(arg+1, "stampfile"))
					stamp_file_cmdline = argv[++i];
				else {
					usage();
					exit(1);
				}
				break;
			case 't' :
				TimerDivisor = 1;
				break;
			case 'u' :
				i++;
				if (!argv[i]) {
					usage();
					exit(1);
				}
				updaterate = atoi(argv[i]);
				if (updaterate < 1 || updaterate > 10) {
					usage();
					exit(1);
				}
				break;
			case 'v' :
				printversion();
				exit(0);
				break;
			default:
				usage();
				exit(0);
				break;
			}
		}
	}

	return 0;
}

  /**********/
 /* reread */
/**********/

void reread(int signal) {
	char			*p;
	char			temp[128];

	rckeys wmppp_keys[] = {
		{ "start", &start_action },
		{ "stop", &stop_action },
		{ "speed", &speed_action },
		{ "ifdown", &ifdown_action },
		{ "stampfile", &stamp_file },
		{ NULL, NULL }
	};

	strcpy(temp, "/etc/wmppprc");
	parse_rcfile(temp, wmppp_keys);

	p = getenv("HOME");
	if (p == NULL) {
		fprintf(stderr,
			"error: HOME environment variable not defined\n");
		exit(EXIT_FAILURE);
	}
	strcpy(temp, p);
	strcat(temp, "/.wmppprc");
	parse_rcfile(temp, wmppp_keys);

	strcpy(temp, "/etc/wmppprc.fixed");
	parse_rcfile(temp, wmppp_keys);

	/* command line options take precedence */
	if (start_action_cmdline)
		strcpy(start_action, start_action_cmdline);
	if (stop_action_cmdline)
		strcpy(stop_action, stop_action_cmdline);
	if (speed_action_cmdline)
		strcpy(speed_action, speed_action_cmdline);
	if (ifdown_action_cmdline)
		strcpy(ifdown_action, ifdown_action_cmdline);
	if (stamp_file_cmdline)
		strcpy(stamp_file, stamp_file_cmdline);

}

  /********/
 /* Main */
/********/

int main(int argc, char **argv) {

	int			j;

	int				but_stat;

	long			starttime;
	long			currenttime;
	long			waittime;
	long			ppptime;
	int				hour,minute;

	long			ppp_send,ppp_sl=-1;
	long			ppp_recv,ppp_rl=-1;
	long			ppp_sbytes,ppp_rbytes;
	long			ppp_osbytes,ppp_orbytes;

	struct stat		st;

	pid_t			stop_child = 0;
	pid_t			start_child = 0;
	int				status;

	XEvent			Event;

	char			temp[128];

	int				speed_ind=60;

	/* Initialize some stuff */

	get_statistics(&ppp_rl, &ppp_sl, &ppp_orbytes, &ppp_osbytes);

	/* Scan through ~/.wmifsrc for the mouse button actions. */
	#ifdef START_ACTION
	    start_action = strdup(START_ACTION);
	#endif
	#ifdef STOP_ACTION
	    stop_action = strdup(STOP_ACTION);
	#endif
	#ifdef SPEED_ACTION
	    speed_action = strdup(SPEED_ACTION);
	#endif
	#ifdef IFDOWN_ACTION
	    ifdown_action = strdup(IFDOWN_ACTION);
	#endif
	#ifdef STAMP_FILE_PRE
           sprintf (temp, "%s%s", STAMP_FILE_PRE, active_interface);
           stamp_file = strdup (temp);
	#endif

	reread(0);
	parse_cmdline(argc, argv);
	signal(SIGHUP, reread);

	/* Open the display */

	createXBMfromXPM(wmppp_mask_bits, wmppp_master_xpm, wmppp_mask_width, wmppp_mask_height);

	openXwindow(argc, argv, wmppp_master_xpm, wmppp_mask_bits, wmppp_mask_width, wmppp_mask_height);

	/* V Button */
	AddMouseRegion(0, 35, 48, 46, 58);
	/* x Button */
	AddMouseRegion(1, 47, 48, 58, 58);

	starttime = 0;
	currenttime = time(0);
	ppptime = 0;
	but_stat = -1;
	waittime = 0;
	copyXPMArea(28, 95, 25, 11, 5, 48);

/* wmppp main loop */
	while (1) {
		int i;
		long lasttime;
		struct timespec ts;

		lasttime = currenttime;
		currenttime = time(0);
		/* Check if any child has left the playground */
		i = waitpid(0, &status, WNOHANG);
		if (i == stop_child && stop_child != 0) {

			starttime = 0;
			SetOffLED(LED_PPP_POWER);
			SetOffLED(LED_PPP_RX);
			SetOffLED(LED_PPP_TX);
			copyXPMArea(28, 95, 25, 11, 5, 48);

			RedrawWindow();

			stop_child = 0;
		}
		if (i == start_child && start_child != 0) {
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) == 10) {

					starttime = 0;
					copyXPMArea(28, 95, 25, 11, 5, 48);
					SetOffLED(LED_PPP_POWER);
					DrawTime(0, 1);
					RedrawWindow();
				}
				start_child = 0;
			}
		}

		/* On-line detectie! 1x per second */

		if (currenttime != lasttime) {
			i = 0;

			if (stillonline(active_interface)) {
				i = 1;
				if (!starttime) {
					starttime = currenttime;

					if (stat(stamp_file, &st) == 0)
						starttime = st.st_mtime;

					SetOnLED(LED_PPP_POWER);
					waittime = 0;

					copyXPMArea(28, 95, 25, 11, 5, 48);

					if (speed_action)
						DrawSpeedInd(speed_action);

					speed_ind = currenttime + 60;

					RedrawWindow();
				}
			}
			if (!i && starttime) {
				starttime = 0;
				SetErrLED(LED_PPP_POWER);

				copyXPMArea(0, 95, 26, 11, 5, 48);

				if (ifdown_action)
					execCommand(ifdown_action);

				RedrawWindow();
			}
		}

		if (waittime && waittime <= currenttime) {
			SetOffLED(LED_PPP_POWER);
			RedrawWindow();
			waittime = 0;
		}

		/* If we are on-line. Print the time we are */
		if (starttime) {
			i = currenttime - starttime;

			i /= TimerDivisor;

			if (TimerDivisor == 1)
				if (i > 59 * 60 + 59) i /= 60;

			minute = i % 60;
			hour = (i / 60) % 100;
			i = hour * 100 + minute;

			DrawTime(i, currenttime % 2);
			/* We are online, so we can check for send/recv packets */

			get_statistics(&ppp_recv, &ppp_send, &ppp_rbytes, &ppp_sbytes);

			if (ppp_send != ppp_sl) SetOnLED(LED_PPP_TX);
			else 					SetOffLED(LED_PPP_TX);

			if (ppp_recv != ppp_rl) SetOnLED(LED_PPP_RX);
			else 					SetOffLED(LED_PPP_RX);

			ppp_sl = ppp_send;
			ppp_rl = ppp_recv;

			/* Every five seconds we check to load on the line */

			if ((currenttime - ppptime >= 0) || (ppptime == 0)) {

				ppptime = currenttime + updaterate;

				ppp_history[PPP_STATS_HIS][0] = ppp_rbytes - ppp_orbytes;
				ppp_history[PPP_STATS_HIS][1] = ppp_sbytes - ppp_osbytes;

				ppp_orbytes = ppp_rbytes;
				ppp_osbytes = ppp_sbytes;

				DrawStats(&ppp_history[0][0], 54, 25, 5, 43);

				for (j=1; j<55; j++) {
					ppp_history[j-1][0] = ppp_history[j][0];
					ppp_history[j-1][1] = ppp_history[j][1];
				}
				if (currenttime > speed_ind) {
					DrawLoadInd((ppp_history[54][0] + ppp_history[54][1]) / updaterate);
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
				while (start_child | stop_child) {
					i = waitpid(0, &status, WNOHANG);
					if (i == stop_child) stop_child = 0;
					if (i == start_child) start_child = 0;
					ts.tv_sec = 0;
					ts.tv_nsec = 50000000L;
					nanosleep(&ts, NULL);
				}
				exit(0);
				break;
			case ButtonPress:
				i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				switch (i) {
				case 0:
					ButtonDown(BUT_V);
					break;
				case 1:
					ButtonDown(BUT_X);
					break;
				}
				but_stat = i;

				RedrawWindow();
				break;
			case ButtonRelease:
				i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				/* Button but_stat omhoogdoen! */
				switch (but_stat) {
				case 0:
					ButtonUp(BUT_V);
					break;
				case 1:
					ButtonUp(BUT_X);
					break;
				}

				if (i == but_stat && but_stat >= 0) {
					switch (i) {
					case 0:
						if (!starttime) {
							/* Reread the rcfiles. */
							reread(0);
							copyXPMArea(28, 95, 25, 11, 5, 48);
							DrawTime(0, 1);
							if (start_action)
								start_child = execCommand(start_action);
							SetWaitLED(LED_PPP_POWER);
							waittime = ORANGE_LED_TIMEOUT + currenttime;
						}

						break;
					case 1:
						if (stop_child == 0) {
							if (stop_action)
								stop_child = execCommand(stop_action);
						}
						break;
					}
				}
				RedrawWindow();

				but_stat = -1;
				break;
			default:
				break;
			}
		}
		ts.tv_sec = 0;
		ts.tv_nsec = 50000000L;
		nanosleep(&ts, NULL);
	}
	return 0;
}

/*******************************************************************************\
|* get_statistics															   *|
\*******************************************************************************/

int get_statistics(long *ip, long *op, long *is, long *os) {

	struct ppp_stats	ppp_cur;
	static int			ppp_opened = 0;


	if (!ppp_opened) {
		/* Open the ppp device. */
		memset(&ppp_cur, 0, sizeof(ppp_cur));
		if ((ppp_h = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			return -1;
		ppp_opened = 1;
	}

	get_ppp_stats(&ppp_cur);

	*op = ppp_cur.p.ppp_opackets;
	*ip = ppp_cur.p.ppp_ipackets;

	*is = ppp_cur.p.ppp_ibytes;
	*os = ppp_cur.p.ppp_obytes;

	return 0;
}

/*******************************************************************************\
|* stillonline																   *|
\*******************************************************************************/

int stillonline(char *ifs) {

	FILE	*fp;
	int		i;

	i = 0;
	fp = fopen("/proc/net/route", "r");
	if (fp) {
		char temp[128];
		while (fgets(temp, 128, fp)) {
			if (strstr(temp, ifs)) {
				i = 1; /* Line is alive */
			}
		}
		fclose(fp);
	}
	return i;
}

/*******************************************************************************\
|* DrawTime																	   *|
\*******************************************************************************/

void DrawTime(int i, int j) {

	int	k = 1000;

	copyXPMArea(TIMER_SZE_X*((i / k)%10)+1, TIMER_SRC_Y, 5, 7, 6+6*0, TIMER_DES_Y);
	k = k /10;
	copyXPMArea(TIMER_SZE_X*((i / k)%10)+1, TIMER_SRC_Y, 5, 7, 6+6*1, TIMER_DES_Y);
	k = k /10;

	if (j)
		copyXPMArea(62, TIMER_SRC_Y, 1, 7, 6+6*2+1, TIMER_DES_Y);
	else
		copyXPMArea(63, TIMER_SRC_Y, 1, 7, 6+6*2+1, TIMER_DES_Y);

	copyXPMArea(TIMER_SZE_X*((i / k)%10)+1, TIMER_SRC_Y, 5, 7, 6+6*2 + 4, TIMER_DES_Y);
	k = k /10;
	copyXPMArea(TIMER_SZE_X*((i / k)%10)+1, TIMER_SRC_Y, 5, 7, 6+6*3 + 4, TIMER_DES_Y);
}

/*******************************************************************************\
|* DrawStats																   *|
\*******************************************************************************/

void DrawStats(int *his, int num, int size, int x_left, int y_bottom) {

	int		pixels_per_byte;
	int		j,k;
	int		*p;

	pixels_per_byte = 1*size;
	p = his;
	for (j=0; j<num; j++) {
		if (p[0] + p[1] > pixels_per_byte)
			pixels_per_byte = p[0] + p[1];
		p += 2;
	}

	pixels_per_byte /= size;
	p = his;

	for (k=0; k<num; k++) {


		for (j=0; j<size; j++) {

			if (j < p[0] / pixels_per_byte)
				copyXPMArea(57+2, 85, 1, 1, k+x_left, y_bottom-j);
			else if (j < (p[0] + p[1]) / pixels_per_byte)
				copyXPMArea(57+1, 85, 1, 1, k+x_left, y_bottom-j);
			else
				copyXPMArea(57+0, 85, 1, 1, k+x_left, y_bottom-j);
		}
		p += 2;
	}
}

/*******************************************************************************\
|* DrawSpeedInd																   *|
\*******************************************************************************/

void PrintLittle(int i, int *k) {

	switch (i) {
	case -2:
		*k -= 5;
		/* Print the "k" letter */
		copyXPMArea(11*5-5, 86, 4, 9, *k, 48);
		break;
	case -1:
		*k -= 5;
		copyXPMArea(13*5-5, 86, 4, 9, *k, 48);
		break;
	case 0:
		*k -= 5;
		copyXPMArea(45, 86, 5, 9, *k, 48);
		break;
	default:
		*k -= 5;
		copyXPMArea(i*5-5, 86, 5, 9, *k, 48);
		break;
	}
}

void DrawSpeedInd(char *speed_action) {

	int	k;
	FILE	*fp;

	fp = popen(speed_action, "r");

	if (fp) {
		char *p;
		char temp[128];
		int i, linespeed;

		linespeed = 0;

		while (fgets(temp, 128, fp))
			;

		pclose(fp);

		if ((p=strstr(temp, "CONNECT"))) {
			linespeed = atoi(p + 8);
		}

		k = 30;

		i = (linespeed % 1000) / 100;
		linespeed /= 1000;
		PrintLittle(i, &k);

		k -= 5;
		copyXPMArea(50, 86, 5, 9, k, 48);

		do {
			PrintLittle(linespeed % 10, &k);
			linespeed /= 10;
		} while (linespeed);
	}
}

/*******************************************************************************\
|* DrawLoadInd																   *|
\*******************************************************************************/

void DrawLoadInd(int speed) {

	int		i, k;

	k = 30;
	for (i=0; i<5; i++) PrintLittle(-1, &k);

	k = 30;

	/* If speed is greater than 99999, display it in K */
	if (speed > 99999 )
	{
		speed /= 1024 ;
		PrintLittle(-2, &k) ;
	}

	do {
		PrintLittle(speed % 10, &k);
		speed /= 10;
	} while (speed);
}

/*******************************************************************************\
|* usage																	   *|
\*******************************************************************************/

void usage(void) {

	fprintf(stderr, "\nwmppp - programming: tijno, design & ideas: warp\n\n");
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "-display <display name>\n");
	fprintf(stderr, "-geometry +XPOS+YPOS         initial window position\n");
	fprintf(stderr, "-h                           this help screen\n");
	fprintf(stderr, "-i <device>                  (ppp0, ppp1, etc) EXPERIMENTAL! Please send\n");
	fprintf(stderr, "                             bugreports!\n");
	fprintf(stderr, "-t                           set the on-line timer to MM:SS instead of HH:MM\n");
	fprintf(stderr, "-u <update rate>             (1..10), default 5 seconds\n");
	fprintf(stderr, "-speed <cmd>                 command to report connection speed\n");
	fprintf(stderr, "-start <cmd>                 command to connect\n");
	fprintf(stderr, "-stop  <cmd>                 command to disconnect\n");
	fprintf(stderr, "-ifdown <cmd>                command to redial\n");
	fprintf(stderr, "-stampfile <path>            file used to calculate uptime\n");
	fprintf(stderr, "-v                           print the version number\n");
	fprintf(stderr, "\n");
}

/*******************************************************************************\
|* printversion																   *|
\*******************************************************************************/

void printversion(void) {

	fprintf(stderr, "%s\n", WMPPP_VERSION);
}

/*******************************************************************************\
|* get_ppp_stats															   *|
\*******************************************************************************/

void get_ppp_stats(struct ppp_stats *cur) {

	struct ifpppstatsreq    req;

	memset(&req, 0, sizeof(req));

	req.stats_ptr = (void*) &req.stats;

	strcpy(req.ifr__name, active_interface);

	if (ioctl(ppp_h, SIOCGPPPSTATS, &req) >= 0)
		*cur = req.stats;
}

#define LED_ON_X (50)
#define LED_ON_Y (80)
#define LED_OFF_Y (75)
#define LED_OFF_X (50)

#define LED_ERR_X (56)
#define LED_ERR_Y (75)
#define LED_WTE_X (56)
#define LED_WTE_Y (80)
#define LED_SZE_X (4)
#define LED_SZE_Y (4)

#define LED_PWR_X (53)
#define LED_PWR_Y (7)
#define LED_SND_X (47)
#define LED_SND_Y (7)
#define LED_RCV_X (41)
#define LED_RCV_Y (7)

#define LED_SW_X (38)
#define LED_SW_Y (14)

/*******************************************************************************\
|* SetOnLED 																   *|
\*******************************************************************************/
void SetOnLED(int led) {

	switch (led) {
	case LED_PPP_POWER:
		copyXPMArea(LED_ON_X, LED_ON_Y, LED_SZE_X, LED_SZE_Y,  LED_PWR_X, LED_PWR_Y);
		break;
	case LED_PPP_RX:
		copyXPMArea(LED_ON_X, LED_ON_Y, LED_SZE_X, LED_SZE_Y,  LED_RCV_X, LED_RCV_Y);
		break;
	case LED_PPP_TX:
		copyXPMArea(LED_ON_X, LED_ON_Y, LED_SZE_X, LED_SZE_Y,  LED_SND_X, LED_SND_Y);
		break;
	}
}

/*******************************************************************************\
|* SetOffLED																   *|
\*******************************************************************************/
void SetOffLED(int led) {

	switch (led) {
	case LED_PPP_POWER:
		copyXPMArea(LED_OFF_X, LED_OFF_Y, LED_SZE_X, LED_SZE_Y,  LED_PWR_X, LED_PWR_Y);
		break;
	case LED_PPP_RX:
		copyXPMArea(LED_OFF_X, LED_OFF_Y, LED_SZE_X, LED_SZE_Y,  LED_RCV_X, LED_RCV_Y);
		break;
	case LED_PPP_TX:
		copyXPMArea(LED_OFF_X, LED_OFF_Y, LED_SZE_X, LED_SZE_Y,  LED_SND_X, LED_SND_Y);
		break;

	}
}

/*******************************************************************************\
|* SetErrLED 																   *|
\*******************************************************************************/
void SetErrLED(int led) {

	switch (led) {
	case LED_PPP_POWER:
		copyXPMArea(LED_ERR_X, LED_ERR_Y, LED_SZE_X, LED_SZE_Y,  LED_PWR_X, LED_PWR_Y);
		break;
	}
}

/*******************************************************************************\
|* SetWaitLED 																   *|
\*******************************************************************************/
void SetWaitLED(int led) {

	switch (led) {
	case LED_PPP_POWER:
		copyXPMArea(LED_WTE_X, LED_WTE_Y, LED_SZE_X, LED_SZE_Y,  LED_PWR_X, LED_PWR_Y);
		break;
	}
}

/*******************************************************************************\
|* ButtonUp 																   *|
\*******************************************************************************/
void ButtonUp(int button) {

	switch (button) {
	case BUT_V :
		copyXPMArea(24, 74, 12, 11, 35, 48);
		break;
	case BUT_X :
		copyXPMArea(36, 74, 12, 11, 47, 48);
		break;
	}
}

/*******************************************************************************\
|* ButtonDown																   *|
\*******************************************************************************/
void ButtonDown(int button) {

	switch (button) {
	case BUT_V :
		copyXPMArea(0, 74, 12, 11, 35, 48);
		break;
	case BUT_X :
		copyXPMArea(12, 74, 12, 11, 47, 48);
		break;
	}
}
