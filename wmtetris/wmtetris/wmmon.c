/*
	Code based on wmppp/wmifs

	[Orig WMPPP comments]

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
	
	------------------------------------------------------------

	Authors: Martijn Pieterse (pieterse@xs4all.nl)
		 Antoine Nulle (warp@xs4all.nl)

	This program is distributed under the GPL license.
	(as were asclock and pppstats)

	----
	Changes:
	----

	18/05/1998 (Antoine Nulle, warp@xs4all.nl)
		* MEM/SWAP/UPTIME only updated when visible
		* Using global file descriptors to reduce file
		  system overhead, both updates are based on a diff 
		  supplied by Dave Harden (dharden@wisewire.com) 
	15/05/1998 (Antoine Nulle, warp@xs4all.nl)
		* Fixed memory overflow in the MEM gaugebar
		* MEM gauge displays now real used mem
		  (buffered + cached mem removed)
	14/05/1998 (Antoine Nulle, warp@xs4all.nl)
		* Added -i & -s kludge for selecting startupmode,
		  tijno, don't hate me for this :)
	12/05/1998 (Antoine Nulle, warp@xs4all.nl)
		* Finetuned master-xpm, tijno don't worry, no 
		  reprogramming needed this time ;-)
	07/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added disk i/o
	03/05/1998 (Antoine Nulle, warp@xs4all.nl)
		* Added new master-xpm which contains the gfx
		  for the upcoming SysInfo part :P
	02/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Removed a lot of code, that was put in wmgeneral
	23/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added zombie destroying code (aka wait :) )
	18/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added CPU-on-screen.
		* Added -display command line
	15/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Fixed a bug in the stats routine
		  (Top 3 bright pixels were not shown when 100% loaded)
		* Changed xpm to a no-title one.
		  This included the reprogramming of all positions.
		  warpstah, i hate you! ;)
	05/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* First Working Version
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/param.h>
#include <sys/types.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"

#include "wmmon-master.xpm"
#include "wmmon-mask.xbm"

  /***********/
 /* Defines */
/***********/

#define LEFT_ACTION (NULL)
#define RIGHT_ACTION (NULL)
#define MIDDLE_ACTION (NULL)

#define WMMON_VERSION "1.0.b2"

  /********************/
 /* Global Variables */
/********************/

char	*ProgName;
int	stat_current = 0; /* now global */
FILE	*fp_meminfo;
FILE	*fp_stat;
FILE	*fp_loadavg;

/* functions */
void usage(void);
void printversion(void);
void DrawStats(int *, int, int, int, int);
void DrawStats_io(int *, int, int, int, int);

void wmmon_routine(int, char **);

void main(int argc, char *argv[]) {

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
			case 'v' :
				printversion();
				exit(0);
				break;
			case 'i' :
				stat_current = 1;
				break;
			case 's' :
				stat_current = 2;
				break;
			default:
				usage();
				exit(0);
				break;
			}
		}
	}

	wmmon_routine(argc, argv);
}

/*******************************************************************************\
|* wmmon_routine															   *|
\*******************************************************************************/

typedef struct {

	char	name[5];			/* "cpu0..cpuz", eventually.. :) */
	int		his[55];
	int		hisaddcnt;
	long	rt_stat;
	long	statlast;
	long	rt_idle;
	long	idlelast;
	
} stat_dev;

#define MAX_STAT_DEVICES (4)
stat_dev	stat_device[MAX_STAT_DEVICES];

char		*left_action;
char		*right_action;
char		*middle_action;


int checksysdevs(void);
void get_statistics(char *, long *, long *, long *);
void DrawActive(char *);

void update_stat_cpu(stat_dev *);
void update_stat_io(stat_dev *);
void update_stat_mem(stat_dev *st, stat_dev *st2);
void update_stat_swp(stat_dev *);

void wmmon_routine(int argc, char **argv) {

	rckeys		wmmon_keys[] = {
		{ "left", &left_action },
		{ "right", &right_action },
		{ "middle", &middle_action },
		{ NULL, NULL }
	};

	unsigned long		i,j;
	long		k;
	XEvent		Event;
	int			but_stat = -1;

	int			stat_online;

	long		starttime;
	long		curtime;
	long		nexttime;

	long		istat;
	long		idle;

	FILE		*fp;
	char		temp[128];
	char		*p;

	int			xpm_X = 0, xpm_Y = 0;

	long		online_time = 0;
	long		ref_time = 0;
	long		cnt_time;


	fp = fopen("/proc/uptime", "r");
	fp_meminfo = fopen("/proc/meminfo", "r");
	fp_loadavg = fopen("/proc/loadavg", "r");
	fp_stat = fopen("/proc/stat", "r");

	if (fp) {
		fscanf(fp, "%ld", &online_time);
		ref_time = time(0);
		fclose(fp);
	}

	for (i=0; i<MAX_STAT_DEVICES; i++) {
		for (j=0; j<55; j++) {
			stat_device[i].his[j] = 0;
		}
		stat_device[i].hisaddcnt = 0;
	}

	if (LEFT_ACTION) left_action = strdup(LEFT_ACTION);
	if (RIGHT_ACTION) right_action = strdup(RIGHT_ACTION);
	if (MIDDLE_ACTION) middle_action = strdup(MIDDLE_ACTION);

	strcpy(temp, "/etc/wmmonrc");
	parse_rcfile(temp, wmmon_keys);

	p = getenv("HOME");
	strcpy(temp, p);
	strcat(temp, "/.wmmonrc");
	parse_rcfile(temp, wmmon_keys);
	
	strcpy(temp, "/etc/wmmonrc.fixed");
	parse_rcfile(temp, wmmon_keys);

	stat_online = checksysdevs();


	openXwindow(argc, argv, wmmon_master_xpm, wmmon_mask_bits, wmmon_mask_width, wmmon_mask_height);

	/* add mouse region */
	AddMouseRegion(0, 12, 13, 58, 57);
	AddMouseRegion(1, 5, 5, 24, 14);

	starttime = time(0);
	nexttime = starttime + 10;

	for (i=0; i<stat_online; i++) {
		get_statistics(stat_device[i].name, &k, &istat, &idle);
		stat_device[i].statlast = istat;
		stat_device[i].idlelast = idle;
	}
	if (stat_current == 0) DrawStats(stat_device[stat_current].his, 54, 40, 5, 58);
	if (stat_current == 1) {
		DrawStats_io(stat_device[stat_current].his, 54, 40, 5, 58);
	}
	if (stat_current == 2) {
		xpm_X = 64;
		setMaskXY(-64, 0);
	} else {
		xpm_X = 0;
		setMaskXY(0, 0);
	}
	DrawActive(stat_device[stat_current].name);

	while (1) {
		curtime = time(0);

		waitpid(0, NULL, WNOHANG);


		update_stat_cpu(&stat_device[0]);
		update_stat_io(&stat_device[1]);

		if(stat_current == 2) {
			update_stat_mem(&stat_device[2], &stat_device[3]);
//			update_stat_swp(&stat_device[3]);
		}

		if (stat_current < 2) {
			i = stat_current;
		
			/* Load ding is 45 pixels hoog */
			copyXPMArea(0, 64, 32, 12, 28, 4);

			j = (stat_device[i].rt_stat + stat_device[i].rt_idle);
			if (j != 0) {
				j = (stat_device[i].rt_stat * 100) / j;
			}
			j = j * 0.32;
			if (j > 32) j = 32;
			copyXPMArea(32, 64, j, 12, 28, 4);
		} else {
			/* Nu zal ie wel 3 zijn. */

			copyXPMArea(0, 64, 32, 12, 28+64, 4);
			copyXPMArea(0, 64, 32, 12, 28+64, 18);

			j = stat_device[2].rt_idle;
			if (j != 0) {
				j = (stat_device[2].rt_stat * 100) / j;
			}
			j = j * 0.32;
			if (j > 32) j = 32;
			copyXPMArea(32, 64, j, 12, 28+64, 4);
			/*---------------------           ------------------*/
			j = stat_device[3].rt_idle;
			if (j != 0) {
				j = (stat_device[3].rt_stat * 100) / j;
			}
			j = j * 0.32;
			if (j > 32) j = 32;
			copyXPMArea(32, 64, j, 12, 28+64, 18);

			/*----------- online tijd neerzetten! ----------*/
			
			cnt_time = time(0) - ref_time + online_time;

			/* cnt_time = uptime in seconden */
			/*
				secs = 108,47
				mins = 89,47
				uren = 70,47
				digits = 40,78, 6breed, 9hoog
			*/
			i = cnt_time % 60;
			cnt_time /= 60;
			copyXPMArea(40 + (i % 10)*7, 78, 6, 9, 115, 47);
			copyXPMArea(40 + (i / 10)*7, 78, 6, 9, 108, 47);

			i = cnt_time % 60;
			cnt_time /= 60;
			copyXPMArea(40 + (i % 10)*7, 78, 6, 9, 96, 47);
			copyXPMArea(40 + (i / 10)*7, 78, 6, 9, 89, 47);

			i = cnt_time % 24;
			cnt_time /= 24;
			copyXPMArea(40 + (i % 10)*7, 78, 6, 9, 77, 47);
			copyXPMArea(40 + (i / 10)*7, 78, 6, 9, 70, 47);

			/* De rest is dagen!  5x7*/
			
			i = cnt_time;
			copyXPMArea(66 + (i % 10)*6, 66, 5, 7, 88, 35);
			i /= 10;
			copyXPMArea(66 + (i % 10)*6, 66, 5, 7, 82, 35);
			i /= 10;
			copyXPMArea(66 + (i % 10)*6, 66, 5, 7, 76, 35);
			i /= 10;
			copyXPMArea(66 + (i % 10)*6, 66, 5, 7, 70, 35);
		}

		if (curtime >= nexttime) {
			nexttime+=10;

			for (i=0; i<stat_online; i++) {
				if (stat_device[i].his[54])
					stat_device[i].his[54] /= stat_device[i].hisaddcnt;

				for (j=1; j<55; j++) {
					stat_device[i].his[j-1] = stat_device[i].his[j];
				}

				if (i == stat_current) {
					if (i == 0) DrawStats(stat_device[i].his, 54, 40, 5, 58);
					if (i == 1) DrawStats_io(stat_device[i].his, 54, 40, 5, 58);
				}
				stat_device[i].his[54] = 0;
				stat_device[i].hisaddcnt = 0;
				
			}
		}
		RedrawWindowXY(xpm_X, xpm_Y);
	
		while (XPending(display)) {
			XNextEvent(display, &Event);
			switch (Event.type) {
			case Expose:
				RedrawWindowXY(xpm_X, xpm_Y);
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
					case 1:
						stat_current++;
						printf("current stat is :%d\n", stat_current);
						if (stat_current == stat_online)
							stat_current = 0;

						DrawActive(stat_device[stat_current].name);
						if (stat_current == 0) DrawStats(stat_device[stat_current].his, 54, 40, 5, 58);
						if (stat_current == 1) {
							DrawStats_io(stat_device[stat_current].his, 54, 40, 5, 58);
						}
						if (stat_current == 2) {
							xpm_X = 64;
							setMaskXY(-64, 0);
						} else {
							xpm_X = 0;
							setMaskXY(0, 0);
						}
						RedrawWindowXY(xpm_X, xpm_Y);
						break;
					}
				}
				break;
			}
		}

		usleep( stat_current == 0 ? 100000L : 200000L);
	}
}

void update_stat_cpu(stat_dev *st) {

	long	k, istat, idle;

	get_statistics(st->name, &k, &istat, &idle);

	st->rt_idle = idle - st->idlelast;
	st->idlelast = idle;

	st->rt_stat = istat - st->statlast;
	st->statlast = istat;

	st->his[54] += k;
	st->hisaddcnt += 1;
}

void update_stat_io(stat_dev *st) {

	long			j, k, istat, idle;
	static long		maxdiskio = 0;

	get_statistics(st->name, &k, &istat, &idle);

	st->rt_idle = idle - st->idlelast;
	st->idlelast = idle;

	st->rt_stat = istat - st->statlast;
	st->statlast = istat;

	j = st->rt_stat;
	if (maxdiskio < j) {
		maxdiskio = j;
	}
	st->rt_idle = maxdiskio - j;

	st->his[54] += st->rt_stat;
	st->hisaddcnt += 1;
}

void update_stat_mem(stat_dev *st, stat_dev *st2) {

	char	temp[128];
	unsigned long free, shared, buffers, cached;

	freopen("/proc/meminfo", "r", fp_meminfo);
	while (fgets(temp, 128, fp_meminfo)) {
		if (strstr(temp, "Mem:")) {
			sscanf(temp, "Mem: %ld %ld %ld %ld %ld %ld",
			       &st->rt_idle, &st->rt_stat,
			       &free, &shared, &buffers, &cached);
			st->rt_idle >>= 10;
			st->rt_stat -= buffers+cached;
			st->rt_stat >>= 10;
//			break;
		}
		if (strstr(temp, "Swap:")) {
			sscanf(temp, "Swap: %ld %ld", &st2->rt_idle, &st2->rt_stat);
			st2->rt_idle >>= 10;
			st2->rt_stat >>= 10;
			break;
		}
	}
}

void update_stat_swp(stat_dev *st) {

	char	temp[128];

	fseek(fp_meminfo, 0, SEEK_SET);
	while (fgets(temp, 128, fp_meminfo)) {
		if (strstr(temp, "Swap:")) {
			sscanf(temp, "Swap: %ld %ld", &st->rt_idle, &st->rt_stat);
			st->rt_idle >>= 10;
			st->rt_stat >>= 10;
			break;
		}
	}

}

/*******************************************************************************\
|* get_statistics															   *|
\*******************************************************************************/

void get_statistics(char *devname, long *is, long *ds, long *idle) {

	int	i;
	char	temp[128];
	char	*p;
	char	*tokens = " \t\n";
	float	f;
	long	maxdiskio=0;

	*is = 0;
	*ds = 0;
	*idle = 0;

	if (!strncmp(devname, "cpu", 3)) {
		fseek(fp_stat, 0, SEEK_SET);
		while (fgets(temp, 128, fp_stat)) {
			if (strstr(temp, "cpu")) {
				p = strtok(temp, tokens);
				/* 1..3, 4 == idle, we don't want idle! */
				for (i=0; i<3; i++) {
					p = strtok(NULL, tokens);
					*ds += atol(p);
				}
				p = strtok(NULL, tokens);
				*idle = atol(p);
			}
		}
		fp_loadavg = freopen("/proc/loadavg", "r", fp_loadavg);
		fscanf(fp_loadavg, "%f", &f);
		*is = (long) (100 * f);
	}

	if (!strncmp(devname, "i/o", 3)) {

		fseek(fp_stat, 0, SEEK_SET);
		while (fgets(temp, 128, fp_stat)) {
			if (strstr(temp, "disk_rio") || strstr(temp, "disk_wio")) {
				p = strtok(temp, tokens);
				/* 1..4 */
				for (i=0; i<4; i++) {
					p = strtok(NULL, tokens);
					*ds += atol(p);
				}
			}
		}
		if (*ds > maxdiskio) maxdiskio = *ds;
	}
}

/*******************************************************************************\
|* checksysdevs																   *|
\*******************************************************************************/

int checksysdevs(void) {

	strcpy(stat_device[0].name, "cpu0");
	strcpy(stat_device[1].name, "i/o");
	strcpy(stat_device[2].name, "sys");

	return 3;
}


/*******************************************************************************\
|* void DrawActive(char *)													   *|
\*******************************************************************************/

void DrawActive(char *name) {

	/* Alles op X,77
	   CPU: 0
	   I/O: 21

	   20 Breed, 10 hoog
	   Destinatie: 5,5
	*/

	if (name[0] == 'c') {
		copyXPMArea(0, 77, 19, 10, 5, 5);
	} else if (name[0] == 'i') {
		copyXPMArea(19, 77, 19, 10, 5, 5);
	}

}

/*******************************************************************************\
|* DrawStats                                                                   *|
\*******************************************************************************/

void DrawStats(int *his, int num, int size, int x_left, int y_bottom) {

	int     pixels_per_byte;
	int     j,k;
	int     *p;
	int		d;

	pixels_per_byte = 100;
	p = his;
	for (j=0; j<num; j++) {
		if (p[0] > pixels_per_byte)
			pixels_per_byte += 100;
		p += 1;
	}

	p = his;

	for (k=0; k<num; k++) {
		d = (1.0 * p[0] / pixels_per_byte) * size;

		for (j=0; j<size; j++) {
		
			if (j < d - 3)
				copyXPMArea(2, 88, 1, 1, k+x_left, y_bottom-j);
			else if (j < d)
				copyXPMArea(2, 89, 1, 1, k+x_left, y_bottom-j);
			else
				copyXPMArea(2, 90, 1, 1, k+x_left, y_bottom-j);
		}
		p += 1;
	}

	/* Nu horizontaal op 100/200/300 etc lijntje trekken! */
	for (j = pixels_per_byte-100; j > 0; j-=100) {
		for (k=0; k<num; k++) {
			d = (40.0 / pixels_per_byte) * j;

			copyXPMArea(2, 91, 1, 1, k+x_left, y_bottom-d);
		}
	}
}

/*******************************************************************************\
|* DrawStats_io                                                                *|
\*******************************************************************************/

void DrawStats_io(int *his, int num, int size, int x_left, int y_bottom) {

	float	pixels_per_byte;
	int     j,k;
	int     *p;
	int		d;

	static int	global_io_scale = 1;

	p = his;
	for (j=0; j<num; j++) {
		if (p[j] > global_io_scale) global_io_scale = p[j];
	}

	pixels_per_byte = 1.0 * global_io_scale / size;
	if (pixels_per_byte == 0) pixels_per_byte = 1;

	for (k=0; k<num; k++) {
		d = (1.0 * p[0] / pixels_per_byte);

		for (j=0; j<size; j++) {
		
			if (j < d - 3)
				copyXPMArea(2, 88, 1, 1, k+x_left, y_bottom-j);
			else if (j < d)
				copyXPMArea(2, 89, 1, 1, k+x_left, y_bottom-j);
			else
				copyXPMArea(2, 90, 1, 1, k+x_left, y_bottom-j);
		}
		p += 1;
	}
}


/*******************************************************************************\
|* usage																	   *|
\*******************************************************************************/

void usage(void) {

	fprintf(stderr, "\nwmmon - programming: tijno, (de)bugging & design warp, webhosting: bobby\n\n");
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "\t-display <display name>\n");
	fprintf(stderr, "\t-h\tthis screen\n");
	fprintf(stderr, "\t-v\tprint the version number\n");
	fprintf(stderr, "\t-i\tstartup in DiskIO mode\n");
	fprintf(stderr, "\t-s\tstartup in SysInfo mode\n");
	fprintf(stderr, "\n");
}

/*******************************************************************************\
|* printversion																   *|
\*******************************************************************************/

void printversion(void) {

	if (!strcmp(ProgName, "wmmon")) {
		fprintf(stderr, "%s\n", WMMON_VERSION);
	}
}
