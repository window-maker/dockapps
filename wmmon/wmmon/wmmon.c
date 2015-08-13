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

	17/06/2012 (Rodolfo García Peñas (kix), <kix@kix.es>)
		* Code style.
	13/3/2012 (Barry Kelly (wbk), <coydog@devio.us>)
		* Fixed get_statistics() I/O features to work with newer
		  /proc/diskstats instead of the old /proc/stat.
		* Fixes to graph/meter scaling for I/O. Original code let
		  the scaling grow out of control due to inappropriate static
		  data.
		* Eliminated rounding down relatively low stats in getWidth()
		  and DrawStats_io() by using double and float types instead
		  of ints. We now round up tiny values to prevent the system
		  appearing idle when it's not.
	    * Style/readbility edits.
		* TODO: Merge in Gentoo and possibly BSD local patches. This
		  should aid in fixing I/O monitoring on non-Linux systems.
		* TODO: Color swapping. User supplies color values in .rc, and
		  app modifies pixmap in memory on startup. Should be simple.
	    * TODO: address compiler warnings (GCC has gotten pickier over
		  the years).
	17/10/2009 (Romuald Delavergne, romuald.delavergne@free.fr)
		* Support SMP processors in realtime CPU stress meter
	15/05/2004 (Simon Law, sfllaw@debian.org)
		* Support disabling of mode-cycling
	23/10/2003 (Simon Law, sfllaw@debian.org)
		* Eliminated exploitable static buffers
		* Added -geometry support.
		* /proc/meminfo support for Linux 2.6
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

#define _GNU_SOURCE
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

#include <wmgeneral/wmgeneral.h>
#include <wmgeneral/misc.h>

#include "wmmon-master.xpm"
#include "wmmon-mask.xbm"

  /***********/
 /* Defines */
/***********/
#define WMMON_VERSION "1.2.b2"
#define HISTORY_ENTRIES 55
#define HISTORY_ENTRIES 55
#define MAX_CPU (10) /* depends on graph height */
#define MAX_STAT_DEVICES (4)

  /********************/
 /* Global Variables */
/********************/
int stat_current = 0; /* now global */
int mode_cycling = 1; /* Allow mode-cycling */
int cpu_avg_max  = 0; /* CPU stress meter with average and max for SMP */
int show_buffers = 0; /* wbk adding per Gentoo -b enhancement. */

FILE *fp_meminfo;
FILE *fp_stat;
FILE *fp_loadavg;
FILE *fp_diskstats;    /* wbk new io stats API */

/* functions */
void usage(char*);
void printversion(void);
void DrawStats(int *, int, int, int, int);
void DrawStats_io(int *, int, int, int, int);
void wmmon_routine(int, char **);

int main(int argc, char *argv[])
{
	int i;
	char *name = argv[0];

	/* Parse Command Line */
	for (i = 1; i < argc; i++) {
		char *arg = argv[i];

		if (*arg=='-')
			switch (arg[1]) {
			case 'd' :
				if (strcmp(arg+1, "display")) {
					usage(name);
					return 1;
				}
				break;
			case 'g' :
				if (strcmp(arg+1, "geometry")) {
					usage(name);
					return 1;
				}
			case 'l' :
				mode_cycling = 0;
				break;
			case 'c' :
				cpu_avg_max = 1;
				break;
			case 'i' :
				stat_current = 1;
				break;
			case 'b' :
				show_buffers = 1;
				break;
			case 's' :
				stat_current = 2;
				break;
			case 'v' :
				printversion();
				return 0;
			default:
				usage(name);
				return 1;
			}
	}

	wmmon_routine(argc, argv);
	exit(0);
}

/*******************************************************************************\
|* wmmon_routine								*|
\*******************************************************************************/

typedef struct {
	char name[5];	/* "cpu0..cpuz", eventually.. :) */
	int his[HISTORY_ENTRIES];
	int hisaddcnt;
	long rt_stat;
	long statlast;
	long rt_idle;
	long idlelast;
	/* Processors stats */
	long *cpu_stat;
	long *cpu_last;
	long *idle_stat;
	long *idle_last;
} stat_dev;

stat_dev stat_device[MAX_STAT_DEVICES];

char *left_action, *right_action, *middle_action;
int nb_cpu, cpu_max;

int getNbCPU(void);
unsigned long getWidth(long, long);
int checksysdevs(void);
void get_statistics(char *, long *, long *, long *, long *, long *);
void DrawActive(char *);

void update_stat_cpu(stat_dev *, long *, long *);
void update_stat_io(stat_dev *);
void update_stat_mem(stat_dev *st, stat_dev *st2);
void update_stat_swp(stat_dev *);

void wmmon_routine(int argc, char **argv)
{
	rckeys wmmon_keys[] = {
		{ "left", &left_action },
		{ "right", &right_action },
		{ "middle", &middle_action },
		{ NULL, NULL }
	};

	unsigned long i, j;
	long k;
	XEvent Event;
	int but_stat = -1;

	int stat_online;

	long starttime, curtime, nexttime;
	long istat, idle, *istat2, *idle2;

	FILE *fp;
	char *conffile = NULL;

	int xpm_X = 0, xpm_Y = 0;

	long online_time = 0;
	long ref_time = 0;
	long cnt_time;


	fp = fopen("/proc/uptime", "r");
	fp_meminfo = fopen("/proc/meminfo", "r");
	fp_loadavg = fopen("/proc/loadavg", "r");
	fp_stat = fopen("/proc/stat", "r");
	fp_diskstats = fopen("/proc/diskstats", "r");

	if (fp) {
		if (fscanf(fp, "%ld", &online_time) == EOF)
			perror("Error! fscanf() of /proc/uptime failed!\n");
		ref_time = time(0);
		fclose(fp);
	}

	for (i = 0; i < MAX_STAT_DEVICES; i++) {
		for (j = 0; j < HISTORY_ENTRIES; j++)
			stat_device[i].his[j] = 0;

		stat_device[i].hisaddcnt = 0;
	}

	/* wbk - I don't fully understand this. Probably just a means of providing
	 * test cases. ifdef'ing to clear compiler warnings. TODO: remove.		*/
#ifdef LEFT_ACTION
	if (LEFT_ACTION)
	  left_action = strdup(LEFT_ACTION);
#endif
#ifdef RIGHT_ACTION
	if (RIGHT_ACTION)
		right_action = strdup(RIGHT_ACTION);
#endif
#ifdef MIDDLE_ACTION
	if (MIDDLE_ACTION)
		middle_action = strdup(MIDDLE_ACTION);
#endif

	/* Scan through the .rc files */
	if (asprintf(&conffile, "/etc/wmmonrc") >= 0) {
		parse_rcfile(conffile, wmmon_keys);
		free(conffile);
	}

	if (asprintf(&conffile, "%s/.wmmonrc", getenv("HOME")) >= 0) {
		parse_rcfile(conffile, wmmon_keys);
		free(conffile);
	}

	if (asprintf(&conffile, "/etc/wmmonrc.fixed") >= 0) {
		parse_rcfile(conffile, wmmon_keys);
		free(conffile);
	}

	stat_online = checksysdevs();

	nb_cpu = getNbCPU();
	stat_device[0].cpu_stat = calloc(nb_cpu, sizeof(long));
	stat_device[0].cpu_last = calloc(nb_cpu, sizeof(long));
	stat_device[0].idle_stat = calloc(nb_cpu, sizeof(long));
	stat_device[0].idle_last = calloc(nb_cpu, sizeof(long));
	if (!stat_device[0].cpu_stat ||
	    !stat_device[0].cpu_last ||
	    !stat_device[0].idle_stat ||
	    !stat_device[0].idle_last) {
		fprintf(stderr, "%s: Unable to alloc memory !\n", argv[0]);
		exit(1);
	}

	istat2 = calloc(nb_cpu, sizeof(long));
	idle2 = calloc(nb_cpu, sizeof(long));
	if (!istat2 || !idle2) {
		fprintf(stderr, "%s: Unable to alloc memory !!\n", argv[0]);
		exit(1);
	}

	openXwindow(argc, argv, wmmon_master_xpm, wmmon_mask_bits,
		    wmmon_mask_width, wmmon_mask_height);

	/* add mouse region */
	AddMouseRegion(0, 12, 13, 58, 57);
	AddMouseRegion(1, 5, 5, 24, 14);

	starttime = time(0);
	nexttime = starttime + 10;

	/* Collect information on each panel */
	for (i = 0; i < stat_online; i++) {
		get_statistics(stat_device[i].name, &k, &istat, &idle, istat2, idle2);
		stat_device[i].statlast = istat;
		stat_device[i].idlelast = idle;
		if (i == 0 && nb_cpu > 1) {
			int cpu;
			for (cpu = 0; cpu < nb_cpu; cpu++) {
				stat_device[i].cpu_last[cpu] = istat2[cpu];
				stat_device[i].idle_last[cpu] = idle2[cpu];
			}
		}
	}

	/* Set the mask for the current window */
	switch (stat_current) {
		case 0:
		case 1:
			xpm_X = 0;
			setMaskXY(0, 0);
			break;
		case 2:
			xpm_X = 64;
			setMaskXY(-64, 0);
		default:
			break;
	}

	/* Draw statistics */
	if (stat_current == 0) {
		DrawStats(stat_device[stat_current].his,
                HISTORY_ENTRIES-1, 40, 5, 58);
	} else if (stat_current == 1) {
		DrawStats_io(stat_device[stat_current].his,
                HISTORY_ENTRIES, 40, 5, 58);
	}

	DrawActive(stat_device[stat_current].name);

	while (1) {
		curtime = time(NULL);

		waitpid(0, NULL, WNOHANG);


		update_stat_cpu(&stat_device[0], istat2, idle2);
		update_stat_io(&stat_device[1]);

		if(stat_current == 2)
			update_stat_mem(&stat_device[2], &stat_device[3]);

		if (stat_current < 2) {
			i = stat_current;

			/* Load ding is 45 pixels hoog */
			copyXPMArea(0, 64, 32, 12, 28, 4);

			if (i == 0 && nb_cpu > 1) {
				if (nb_cpu > MAX_CPU || cpu_avg_max) {
					/* show average CPU */
					j = getWidth(stat_device[i].rt_stat, stat_device[i].rt_idle);
					copyXPMArea(32, 64, j, 6, 28, 4);
					/* Show max CPU */
					j = getWidth(stat_device[i].cpu_stat[cpu_max],
									stat_device[i].idle_stat[cpu_max]);
					copyXPMArea(32, 70, j, 6, 28, 10);
				} else {
					int cpu;
					for (cpu = 0; cpu < nb_cpu; cpu++) {
						j = getWidth(stat_device[i].cpu_stat[cpu],
										stat_device[i].idle_stat[cpu]);
						copyXPMArea(32, 65, j,
									  MAX_CPU / nb_cpu, 28,
									  5 + (MAX_CPU / nb_cpu) * cpu);
					}
				}
			} else {
				j = getWidth(stat_device[i].rt_stat, stat_device[i].rt_idle);
				copyXPMArea(32, 64, j, 12, 28, 4);
			}
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

			/*--------------------- swap?     ------------------*/
			j = stat_device[3].rt_idle;
			if (j != 0)
				j = (stat_device[3].rt_stat * 100) / j;

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

			if (curtime > nexttime) /* dont let APM suspends make this crazy */
				nexttime = curtime;

			for (i=0; i<stat_online; i++) {
			        stat_dev *sd = stat_device + i;

				if (sd->his[HISTORY_ENTRIES-1])
					sd->his[HISTORY_ENTRIES-1] /= sd->hisaddcnt;

				for (j = 1; j < HISTORY_ENTRIES; j++)
					sd->his[j-1] = sd->his[j];

				if (i == stat_current) {
					if (i == 0)
						DrawStats(sd->his, HISTORY_ENTRIES - 1, 40, 5, 58);
					else if (i == 1)
						DrawStats_io(sd->his, HISTORY_ENTRIES - 1, 40, 5, 58);
				}
				sd->his[HISTORY_ENTRIES-1] = 0;
				sd->hisaddcnt = 0;

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
						break;
					case 1:
						if (mode_cycling) {
							stat_current++;
							if (stat_current == stat_online)
								stat_current = 0;

							DrawActive(stat_device[stat_current].name);
							if (stat_current == 0)
								DrawStats(stat_device[stat_current].his,
									  HISTORY_ENTRIES-1, 40, 5, 58);

							if (stat_current == 1)
								DrawStats_io(stat_device[stat_current].his,
									     HISTORY_ENTRIES-1, 40, 5, 58);

							if (stat_current == 2) {
								xpm_X = 64;
								setMaskXY(-64, 0);
							} else {
								xpm_X = 0;
								setMaskXY(0, 0);
							}
							RedrawWindowXY(xpm_X, xpm_Y);
						}
						break;
					}
				}
				break;
			}
		}
		usleep(250000L);
	}
}


void update_stat_cpu(stat_dev *st, long *istat2, long *idle2)
{
	long k, istat, idle;

	get_statistics(st->name, &k, &istat, &idle, istat2, idle2);

	st->rt_idle = idle - st->idlelast;
	st->idlelast = idle;

	st->rt_stat = istat - st->statlast;
	st->statlast = istat;

	if (nb_cpu > 1) {
		int cpu;
		unsigned long  max, j;
		cpu_max = 0; max = 0;
		for (cpu = 0; cpu < nb_cpu; cpu++) {
			st->idle_stat[cpu] = idle2[cpu] - st->idle_last[cpu];
			st->idle_last[cpu] = idle2[cpu];

			st->cpu_stat[cpu] = istat2[cpu] - st->cpu_last[cpu];
			st->cpu_last[cpu] = istat2[cpu];

			j = st->cpu_stat[cpu] + st->idle_stat[cpu];

			if (j != 0)
				j = (st->cpu_stat[cpu] << 7) / j;

			if (j > max) {
				max = j;
				cpu_max = cpu;
			}
		}
	}

	st->his[HISTORY_ENTRIES-1] += k;
	st->hisaddcnt += 1;
}


void update_stat_io(stat_dev *st)
{
	long j, k, istat, idle;

	/* Periodically re-sample. Sometimes we get anomalously high readings;
	 * this discards them. */
	static int stalemax = 300;
	static long maxdiskio = 0;
	if (--stalemax <= 0) {
		maxdiskio = 0;
		stalemax = 300;
	}

	get_statistics(st->name, &k, &istat, &idle, NULL, NULL);

	st->rt_idle = idle - st->idlelast;
	st->idlelast = idle;

	st->rt_stat = istat - st->statlast;
	st->statlast = istat;

	/* remember peak for scaling of upper-right meter. */
	j = st->rt_stat;
	if (maxdiskio < j)
		maxdiskio = j;

	/* Calculate scaling factor for upper-right meter. "/ 5" will clip
	* the highest peaks, but makes moderate values more visible. We are
	* compensating for wild fluctuations which are probably caused by
	* kernel I/O buffering.
	*/
	st->rt_idle = (maxdiskio - j) / 5;
	if (j > 0 && st->rt_idle < 1)
		st->rt_idle = 1;      /* scale up tiny values so they are visible */

	st->his[HISTORY_ENTRIES-1] += st->rt_stat;
	st->hisaddcnt += 1;
}


void update_stat_mem(stat_dev *st, stat_dev *st2)
{
	static char *line = NULL;
	static size_t line_size = 0;

	unsigned long swapfree;
	unsigned long free, shared, buffers, cached;

	if (freopen("/proc/meminfo", "r", fp_meminfo) == NULL)
		perror("freopen() of /proc/meminfo failed!)\n");

	while ((getline(&line, &line_size, fp_meminfo)) > 0) {
		/* The original format for the first two lines of /proc/meminfo was
		 * Mem: total used free shared buffers cached
		 * Swap: total used free
		 *
		 * As of at least 2.5.47 these two lines were removed, so that the
		 * required information has to come from the rest of the lines.
		 * On top of that, used is no longer recorded - you have to work
		 * this out yourself, from total - free.
		 *
		 * So, these changes below should work. They should also work with
		 * older kernels, too, since the new format has been available for
		 * ages.
		 */
		if (strstr(line, "MemTotal:"))
			sscanf(line, "MemTotal: %ld", &st->rt_idle);
		else if (strstr(line, "MemFree:"))
			sscanf(line, "MemFree: %ld", &free);
		else if (strstr(line, "MemShared:"))
			sscanf(line, "MemShared: %ld", &shared);
		else if (strstr(line, "Buffers:"))
			sscanf(line, "Buffers: %ld", &buffers);
		else if (strstr(line, "Cached:"))
			sscanf(line, "Cached: %ld", &cached);
		else if (strstr(line, "SwapTotal:"))
			sscanf(line, "SwapTotal: %ld", &st2->rt_idle);
		else if (strstr(line, "SwapFree:"))
			sscanf(line, "SwapFree: %ld", &swapfree);
	}

	/* memory use - rt_stat is the amount used, it seems, and this isn't
	 * recorded in current version of /proc/meminfo (as of 2.5.47), so we
	 * calculate it from MemTotal - MemFree
	 */
	st->rt_stat = st->rt_idle - free;

	/* wbk -b flag (from Gentoo patchkit) */
	if (!show_buffers)
		st->rt_stat -= buffers+cached;
	/* As with the amount of memory used, it's not recorded any more, so
	 * we have to calculate it ourselves.
	 */
	st2->rt_stat = st2->rt_idle - swapfree;
}

void update_stat_swp(stat_dev *st)
{
	static char *line = NULL;
	static size_t line_size = 0;
	unsigned long swapfree;

	fseek(fp_meminfo, 0, SEEK_SET);
	while ((getline(&line, &line_size, fp_meminfo)) > 0) {
		/* As with update_stat_mem(), the format change to /proc/meminfo has
		 * forced some changes here. */
		if (strstr(line, "SwapTotal:"))
			sscanf(line, "SwapTotal: %ld", &st->rt_idle);
		else if (strstr(line, "SwapFree:"))
			sscanf(line, "SwapFree: %ld", &swapfree);
	}
	st->rt_stat = st->rt_idle - swapfree;
}

/*******************************************************************************\
|* get_statistics								*|
\*******************************************************************************/
void get_statistics(char *devname, long *is, long *ds, long *idle, long *ds2, long *idle2)
{
	int i;
	static char *line = NULL;
	static size_t line_size = 0;
	char *p;
	char *tokens = " \t\n";
	float f;

	*is = 0;
	*ds = 0;
	*idle = 0;

	if (!strncmp(devname, "cpu", 3)) {
		fseek(fp_stat, 0, SEEK_SET);
		while ((getline(&line, &line_size, fp_stat)) > 0) {
			if (strstr(line, "cpu")) {
				int cpu = -1;	/* by default, cumul stats => average */
				if (!strstr(line, "cpu ")) {
					sscanf(line, "cpu%d", &cpu);
					ds2[cpu] = 0;
					idle2[cpu] = 0;
				}
				p = strtok(line, tokens);
				/* 1..3, 4 == idle, we don't want idle! */
				for (i=0; i<3; i++) {
					p = strtok(NULL, tokens);
					if (cpu == -1)
						*ds += atol(p);
					else
						ds2[cpu] += atol(p);
				}
				p = strtok(NULL, tokens);
				if (cpu == -1)
					*idle = atol(p);
				else
					idle2[cpu] = atol(p);
			}
		}
		if ((fp_loadavg = freopen("/proc/loadavg", "r", fp_loadavg)) == NULL)
			perror("ger_statistics(): freopen(proc/loadavg) failed!\n");

		if (fscanf(fp_loadavg, "%f", &f) == EOF)
			perror("fscanf() failed to read f\n");
		*is = (long) (100 * f);
	}

	if (!strncmp(devname, "i/o", 3)) {
		if (fseek(fp_diskstats, 0, SEEK_SET) == -1)
			perror("get_statistics() seek failed\n");

		/* wbk 20120308 These are no longer in /proc/stat. /proc/diskstats
		 * seems to be the closest replacement. Under modern BSD's, /proc is
		 * now deprecated, so iostat() might be the answer.
		 *	      http://www.gossamer-threads.com/lists/linux/kernel/314618
		 * has good info on this being removed from kernel. Also see
		 * kernel sources Documentation/iostats.txt
		 *
		 * TODO: We will end up with doubled values. We are adding the
		 * aggregate to the individual partition, due to device selection
		 * logic. Either grab devices' stats with numbers, or without (sda
		 * OR sda[1..10]. Could use strstr() return plus offset, but would
		 * have to be careful with bounds checking since we're in a
		 *  limited buffer. Or just divide by 2 (inefficient). Shouldn't
		 * matter for graphing (we care about proportions, not numbers).  */
		while ((getline(&line, &line_size, fp_diskstats)) > 0) {
			if (strstr(line, "sd") || strstr(line, "sr")) {
				p = strtok(line, tokens);
				/* skip 3 tokens, then use fields from
				`* linux/Documentation/iostats.txt	     */
				for (i = 1; i <= 6; i++)
					p = strtok(NULL, tokens);

				*ds += atol(p);
				for (i = 7; i <= 10; i++)
					p = strtok(NULL, tokens);

				*ds += atol(p);
			}
		}
	}
}


/*******************************************************************************\
|* getWidth									*|
\*******************************************************************************/
unsigned long getWidth(long actif, long idle)
{
	/* wbk - work with a decimal value so we don't round < 1 down to zero.  */
	double j = 0;
	unsigned long r = 0;

	j = (actif + idle);
	if (j != 0)
		j = (actif * 100) / j;

	j = j * 0.32;

	/* round up very low positive values so they are visible. */
	if (actif > 0 && j < 2)
		j = 2;

	if (j > 32)
		j = 32;

	r = (unsigned long) j;
	return r;
}


/*******************************************************************************\
|* getNbCPU									*|
\*******************************************************************************/
int getNbCPU(void)
{
	static char *line = NULL;
	static size_t line_size = 0;
	int cpu = 0;

	fseek(fp_stat, 0, SEEK_SET);
	while ((getline(&line, &line_size, fp_stat)) > 0) {
		if (strstr(line, "cpu") && !strstr(line, "cpu "))
			sscanf(line, "cpu%d", &cpu);
	}

	return cpu+1;
}


/*******************************************************************************\
|* checksysdevs									*|
\*******************************************************************************/
int checksysdevs(void) {
	strcpy(stat_device[0].name, "cpu0");
	strcpy(stat_device[1].name, "i/o");
	strcpy(stat_device[2].name, "sys");

	return 3;
}


/*******************************************************************************\
|* void DrawActive(char *)							*|
\*******************************************************************************/
void DrawActive(char *name)
{

	/* Alles op X,77
	   CPU: 0
	   I/O: 21

	   20 Breed, 10 hoog
	   Destinatie: 5,5
	*/

	if (name[0] == 'c')
		copyXPMArea(0, 77, 19, 10, 5, 5);
	else if (name[0] == 'i')
		copyXPMArea(19, 77, 19, 10, 5, 5);
}


/*******************************************************************************\
|* DrawStats                                                                   *|
\*******************************************************************************/
void DrawStats(int *his, int num, int size, int x_left, int y_bottom)
{
	int pixels_per_byte, j, k, *p, d;

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
void DrawStats_io(int *his, int num, int size, int x_left, int y_bottom)
{
	float	pixels_per_byte;
	int     j, k, *p;
	/* wbk - Use a double to avoid rounding values of d < 1 to zero. */
	double d = 0;
	int border = 3;

	/* wbk - this should not be static. No need to track the scale, since
	 * we always calculate it on the fly anyway. This static variable did
	 * not get re-initialized when we entered this function, so the scale
	 * would always grow and never shrink.
	 */
	/*static int	global_io_scale = 1;*/
	int	io_scale = 1;

	p = his;
	for (j=0; j<num; j++)
		if (p[j] > io_scale) io_scale = p[j];

	pixels_per_byte = 1.0 * io_scale / size;
	if (pixels_per_byte == 0)
		pixels_per_byte = 1;

	for (k=0; k<num; k++) {
		d = (1.0 * p[0] / pixels_per_byte);

		/* graph values too low for graph resolution */
		if (d > 0 && d < 1) {
			d = 3;
			border = 2;
		} else {
			border = 3;
		}

		for (j=0; j<size; j++) {
			if (j < d - border)
				copyXPMArea(2, 88, 1, 1, k+x_left, y_bottom-j);
			else if (j < d )
				copyXPMArea(2, 89, 1, 1, k+x_left, y_bottom-j);
			else
				copyXPMArea(2, 90, 1, 1, k+x_left, y_bottom-j);
		}
		p += 1;   /* beware... */
	}
}


/*******************************************************************************\
|* usage									*|
\*******************************************************************************/
void usage(char *name)
{
	printf("Usage: %s [OPTION]...\n", name);
	printf("WindowMaker dockapp that displays system information.\n");
	printf("\n");
	printf("  -display DISPLAY     contact the DISPLAY X server\n");
	printf("  -geometry GEOMETRY   position the clock at GEOMETRY\n");
	printf("  -l                   locked view - cannot cycle modes\n");
	printf("  -c                   show average and max CPU for SMP machine.\n");
	printf("                       default if there is more than %d processors\n", MAX_CPU);
	printf("  -i                   start in Disk I/O mode\n");
	printf("  -s                   start in System Info mode\n");
	printf("  -b                   include buffers and cache in memory usage\n");
	printf("  -h                   display this help and exit\n");
	printf("  -v                   output version information and exit\n");
}


/*******************************************************************************\
|* printversion									*|
\*******************************************************************************/
void printversion(void)
{
	printf("WMMon version %s\n", WMMON_VERSION);
}
/* vim: sw=4 ts=4 columns=82
 */
