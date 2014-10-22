/*
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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include <net/ppp_defs.h>
#include <net/if_ppp.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"

#include "wmifs-master.xpm"
#include "wmifs-mask.xbm"

/* How often to check for new network interface, in ms */
#define CHECK_INTERFACE_INTERVAL 5000

/* How often to query the interfaces, in ms */
#define DEFAULT_SAMPLE_INTERVAL 50

  /***********/
 /* Defines */
/***********/

/* Fill in the hardcoded actions */
#define LEFT_ACTION (NULL)
#define MIDDLE_ACTION (NULL)
#define RIGHT_ACTION (NULL)

/* Defines voor alle coordinate */

#define LED_NET_RX			(4)
#define LED_NET_TX			(5)
#define LED_NET_POWER		(6)

#define WMIFS_VERSION "1.3b1"

/* the size of the buffer read from /proc/net/ */
#define BUFFER_SIZE 512
  /**********************/
 /* External Variables */
/**********************/

extern	char **environ;

  /********************/
 /* Global Variables */
/********************/

char	*ProgName;
char	*active_interface = NULL;
int		TimerDivisor=60;
int		WaveForm=0;
int		LockMode=0;
int		SampleInt = DEFAULT_SAMPLE_INTERVAL;
int		ScrollSpeed = CHECK_INTERFACE_INTERVAL;

  /*****************/
 /* PPP variables */
/*****************/

#define 	PPP_UNIT		0
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

int main(int argc, char *argv[]) {

	int		i;
	

	/* Parse Command Line */

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
			case 'i' :
				active_interface = argv[i+1];
				i++;
				break;
			case 'I' :
				SampleInt = atof(argv[i+1]) * 1000;
				i++;
				break;
			case 'l' :
				LockMode = 1;
				break;
			case 's' :
				ScrollSpeed = atof(argv[i+1]) * 1000;
				i++;
				break;
			case 'v' :
				printversion();
				exit(0);
				break;
			case 'w' :
				WaveForm = 1;
				break;
			default:
				usage();
				exit(0);
				break;
			}
		}
	}

	wmifs_routine(argc, argv);
	return 0;
}

/*******************************************************************************\
|* wmifs_routine															   *|
\*******************************************************************************/

#define MAX_STAT_DEVICES 16

typedef struct {

	char	name[8];
	int		his[55][2];
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

void wmifs_routine(int argc, char **argv) {

	rckeys	wmifs_keys[] = {
		{ "left", &left_action },
		{ "middle", &middle_action },
		{ "right", &right_action },
		{ NULL, NULL }
	};


	int			i,j;
	XEvent		Event;
	int			but_stat = -1;

	int			stat_online;
	int			stat_current;

	unsigned int	curtime;
	unsigned int	nexttime;
	struct timeval	tv, tv2;

	long		ipacket, opacket, istat, ostat;

	char		temp[BUFFER_SIZE];
	char		*p;

	for (i=0; i<MAX_STAT_DEVICES; i++) {
		stat_devices[i].name[0] = 0;
		for (j=0; j<48; j++) {
			stat_devices[i].his[j][0] = 0;
			stat_devices[i].his[j][1] = 0;
		}
	}

	stat_online = checknetdevs();

	stat_current = 0;
	if (active_interface) {
		int isauto = !strcmp(active_interface, "auto");
		for (i=0; i<stat_online; i++) {
			if ((isauto && stillonline(stat_devices[i].name)) ||
			    !strcmp(stat_devices[i].name, active_interface)) {
				stat_current = i;
				break;
			}
		}
	}
	
	if (LEFT_ACTION) left_action = strdup(LEFT_ACTION);
	if (MIDDLE_ACTION) middle_action = strdup(MIDDLE_ACTION);
	if (RIGHT_ACTION) right_action = strdup(RIGHT_ACTION);

	/* Scan throught  the .rc files */
	strcpy(temp, "/etc/wmifsrc");
	parse_rcfile(temp, wmifs_keys);

	p = getenv("HOME");
	strcpy(temp, p);
	strcat(temp, "/.wmifsrc");
	parse_rcfile(temp, wmifs_keys);

	strcpy(temp, "/etc/wmifsrc.fixed");
	parse_rcfile(temp, wmifs_keys);

	openXwindow(argc, argv, wmifs_master_xpm, wmifs_mask_bits, wmifs_mask_width, wmifs_mask_height);

	/* > Button */
	AddMouseRegion(0, 5, 5, 35, 15);
	AddMouseRegion(1, 5, 20, 58, 58);

	gettimeofday(&tv2, NULL);
	nexttime = ScrollSpeed;

	for (i=0; i<stat_online; i++) {
		get_statistics(stat_devices[i].name, &ipacket, &opacket, &istat, &ostat);
		stat_devices[i].istatlast = istat;
		stat_devices[i].ostatlast = ostat;
	}

	DrawActiveIFS(stat_devices[stat_current].name);

	while (1) {
		gettimeofday(&tv, NULL);
		curtime = (tv.tv_sec - tv2.tv_sec) * 1000 
			+ (tv.tv_usec - tv2.tv_usec) / 1000;

		waitpid(0, NULL, WNOHANG);

		for (i=0; i<stat_online; i++) {
			get_statistics(stat_devices[i].name, &ipacket, &opacket, &istat, &ostat);
			stat_devices[i].his[53][0] += istat - stat_devices[i].istatlast;
			stat_devices[i].his[53][1] += ostat - stat_devices[i].ostatlast;


			if (i == stat_current) {
				if (!stillonline(stat_devices[i].name)) {
					SetErrLED(LED_NET_POWER);
				} else {
					SetOnLED(LED_NET_POWER);
				}

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
			RedrawWindow();
		}
		
		if (curtime >= nexttime) {
			nexttime=curtime+ScrollSpeed;

			for (i=0; i<stat_online; i++) {
				if (i == stat_current) {

					DrawStats(&stat_devices[i].his[0][0], 54, 40, 5, 58);
				}
				if (stillonline(stat_devices[i].name)) {
					for (j=1; j<54; j++) {
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
				i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);

				but_stat = i;
				break;
			case ButtonRelease:
				i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);

				if (but_stat == i && but_stat >= 0) {
					switch (but_stat) {
					case 0 :
						/* re-read the table */
						strcpy(temp, stat_devices[stat_current].name);
						stat_online = checknetdevs();
						stat_current = 0;
						for (i=0; i<stat_online; i++) {
							if (!strcmp(temp, stat_devices[i].name)) {
								stat_current = i;
							}
						}
					
						stat_current++;
						if (stat_current == stat_online) stat_current = 0;

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

		usleep(SampleInt * 1000);
	}
}

/*******************************************************************************\
|* void DrawActiveIFS(char *)												   *|
\*******************************************************************************/

void DrawActiveIFS(char *real_name) {

	/* Cijfers op: 0,65
	   Letters op: 0,75
	   Alles 9 hoog, 6 breedt

	   Destinatie: 5,5
	*/

	int		i;
	int		c;
	int		k;
	int		len;
	char		name[256];


	copyXPMArea(5, 84, 30, 10, 5, 5);


	strcpy(name,real_name);
	len = strlen(name);
	if (len > 5)
	{
		for (i=len-5; i<len && !(name[i]>='0' && name[i]<='9'); i++)  ;
		for (; i<=len; i++) /* '=' to get the '\0' character moved too \*/
			name[i-(len-5)] = name[i];
	}

	k = 5;
	for (i=0; name[i]; i++) {
		if (i == strlen(name)-1 && strlen(name) <= 4 && name[strlen(name)-1] >= '0' && name[strlen(name)-1] <= '9') {
			copyXPMArea(61, 64, 4, 9, k, 5);
			k+=4;
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

int get_statistics(char *devname, long *ip, long *op, long *is, long *os) {

	FILE				*fp;
	char				temp[BUFFER_SIZE];
	char				*p;
	char				*tokens = " |:\n";
	int					input, output;
	int					i;
	int					found;
	struct ppp_stats	ppp_cur, ppp_old;
	static int			ppp_opened = 0;

	
	if (!strncmp(devname, "ppp", 3)) {
		if (!ppp_opened) {
			/* Open the ppp device. */
			memset(&ppp_cur, 0, sizeof(ppp_cur));
			if ((ppp_h = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
				return -1;
			get_ppp_stats(&ppp_cur);
			ppp_old = ppp_cur;
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
	fgets(temp, BUFFER_SIZE, fp);
	fgets(temp, BUFFER_SIZE, fp);

	input = -1;
	output = -1;
	i = 0;
	found = -1;

	p = strtok(temp, tokens);
	do {
		if (!(strcmp(p, "packets"))) {
			if (input == -1) input = i;
			else output = i;
		}
		i++;
		p = strtok(NULL, tokens);
	} while (input == -1 || output == -1);

	while (fgets(temp, BUFFER_SIZE, fp)) {
		if (strstr(temp, devname)) {
			found = 0;
			p = strtok(temp, tokens);
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
				p = strtok(NULL, tokens);
			} while (input != -1 || output != -1);
		}
	}
	fclose(fp);

	return found;
}

/*******************************************************************************\
|* stillonline																   *|
\*******************************************************************************/

int stillonline(char *ifs) {

	FILE	*fp;
	char	temp[BUFFER_SIZE];
	int		i;

	i = 0;
	fp = fopen("/proc/net/route", "r");
	if (fp) {
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

int checknetdevs(void) {

	FILE	*fd;
	char	temp[BUFFER_SIZE];
	char	*p;
	int		i=0,j;
	int		k;
	int		devsfound=0;
	char	*tokens = " :\t\n";
	char	foundbuffer[MAX_STAT_DEVICES][8];

	for (i=0; i<MAX_STAT_DEVICES; i++) {
		foundbuffer[i][0] = 0;
	}

	/* foundbuffer vullen met info uit /proc/net/dev */

	fd = fopen("/proc/net/dev", "r");
	if (fd) {
		/* Skip the first 2 lines */
		fgets(temp, BUFFER_SIZE, fd);
		fgets(temp, BUFFER_SIZE, fd);
		while (fgets(temp, BUFFER_SIZE, fd)) {
			p = strtok(temp, tokens);
			if(p == NULL) {
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

	for (i=0; i<MAX_STAT_DEVICES; i++) {
		/* Loop stat_devices na, als die naam niet voorkomt in foundbuffer, kill! */

		if (stat_devices[i].name[0]) {
			k = 0;
			for (j=0; j<MAX_STAT_DEVICES; j++) {
				if (!strcmp(stat_devices[i].name, foundbuffer[j])) {
					k = 1;
					foundbuffer[j][0] = 0;
				}
			}
			if (!k) stat_devices[i].name[0] = 0;
		}
	}

	for (i=0, j=0; j<MAX_STAT_DEVICES; i++, j++) {

		while (!stat_devices[j].name[0] && j < MAX_STAT_DEVICES)
			j++;

		if (j < MAX_STAT_DEVICES && i != j) {
			stat_devices[i] = stat_devices[j];
		}
	}
	i--;

	for (j=0; j<MAX_STAT_DEVICES; j++) {
		if (foundbuffer[j][0]) {
			
			strcpy(stat_devices[i].name, foundbuffer[j]);
			
			for (k=0; k<48; k++) {
				stat_devices[i].his[k][0] = 0;
				stat_devices[i].his[k][1] = 0;
			}

			i++;
		}
	}
	if (LockMode && active_interface != NULL) {
		k = 0;
		for (j=0; j<i; j++)
			if (!strcmp(stat_devices[j].name, active_interface)) {
				k = 1;
				break;
			}
		if (!k) {
			strcpy(stat_devices[i].name, active_interface);
			for (k=0; k<48; k++) {
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

void DrawStats(int *his, int num, int size, int x_left, int y_bottom) {

	int		pixels_per_byte;
	int		j,k;
	int		*p;
	int		p0,p1,p2,p3;

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
		p0 = p[0];
		p1 = p[1];


		if (WaveForm) {
			p2 = 0;
			p3 = 1;
			for (j=0; j<size; j++) {
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
			for (j=0; j<size; j++) {
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

void usage(void) {

	fprintf(stderr, "\nwmifs - programming: tijno, (de)bugging & design: warpstah, webhosting: bobby \n\n");
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "\t-d <display name>\n");
	fprintf(stderr, "\t-h\tthis help screen\n");
	fprintf(stderr, "\t-i <interface name>\tdefault (as it appears in /proc/net/route)\n");
	fprintf(stderr, "\t-I <interval>\tsampling interval, in seconds (default: 0.05)\n");
	fprintf(stderr, "\t-l\tstarts in lock mode\n");
	fprintf(stderr, "\t-s <interval>\tscrolling interval, in seconds (default: 5)\n");
	fprintf(stderr, "\t-v\tprint the version number\n");
	fprintf(stderr, "\t-w\twaveform load\n");
	fprintf(stderr, "\n");
}

/*******************************************************************************\
|* printversion																   *|
\*******************************************************************************/

void printversion(void) {

	fprintf(stderr, "%s\n", WMIFS_VERSION);
}

/*******************************************************************************\
|* get_ppp_stats															   *|
\*******************************************************************************/

void get_ppp_stats(struct ppp_stats *cur) {

	struct ifpppstatsreq    req;

	memset(&req, 0, sizeof(req));

	req.stats_ptr = (caddr_t) &req.stats;

	sprintf(req.ifr__name, "ppp%d", PPP_UNIT);

	if (ioctl(ppp_h, SIOCGPPPSTATS, &req) < 0) {
/*		fprintf(stderr, "heyho!\n"); */
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
|* SetOnLED 																   *|
\*******************************************************************************/
void SetOnLED(int led) {

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
|* SetOffLED																   *|
\*******************************************************************************/
void SetOffLED(int led) {

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
|* SetErrLED 																   *|
\*******************************************************************************/
void SetErrLED(int led) {

	switch (led) {
	case LED_NET_POWER:
		copyXPMArea(LED_ERR_NET_X, LED_ERR_NET_Y, LED_SZE_X, LED_SZE_Y,  LED_PWR_X, LED_PWR_Y);
		break;
	}
}
