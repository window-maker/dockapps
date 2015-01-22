/*
 *
 *  	wmfsm-0.33 (C) 1999 Stefan Eilemann (Stefan.Eilemann@dlr.de)
 * 
 *  		- Shows file system usage ala mfsm
 * 
 * 
 * 
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation; either version 2, or (at your option)
 * 	any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program (see the file COPYING); if not, write to the
 * 	Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 *      Boston, MA  02111-1307, USA
 *
 *
 *
 *      Changes: #include "../ChangeLog" 
 *
 */





/*  
 *   Includes  
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "../wmgeneral/wmgeneral.h"
#include "wmfsm_master.xpm"
#include "wmfsm_mask.xbm"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef SVR4
#include <netdb.h>
#include <sys/systeminfo.h>
#endif /* SVR4 */

#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef STATFS_2_ARGUMENTS
# define STATFS(a,b) statfs(a,b)
#elif defined STATFS_4_ARGUMENTS
# define STATFS(a,b) statfs(a,b,sizeof(struct statfs),0)
#else
# define STATFS(a,b) statfs(a,b)	/* Maybe configure got messed up */
#endif


/*
  #if defined IRIX64
  # include <sys/types.h>
  # define STATFS(a,b) statfs(a,b,sizeof(struct statfs),0)
  #elif defined linux 
  # define STATFS(a,b) statfs(a,b)
  #elif  defined(SunOS)
  # define STATFS(a,b) statfs(a,b,sizeof(struct statfs),0)
  #elif defined(__OpenBSD__) || defined(__FreeBSD__)
  # define STATFS(a,b) statfs(a,b)
  # include <sys/param.h>
  #endif		
*/

#define _GNU_SOURCE
#include <getopt.h>
/* 
 *  Delay between refreshes (in microseconds), uses the 10*DELAY_10
 *   coz irix has max of 100000L :(. 
 */
#define DELAY_10 99999L
#define WMFSM_VERSION "0.33"
#define LENMP 5			/*max 10, number char for mountpoint */

/*modes for drawing*/
#define NORMAL 0
#define FIRE 1


void ParseCMDLine(int argc, char *argv[]);
void pressEvent(XButtonEvent * xev);
void readFileSystems(void);
void excludeFileSystems(void);
void print_usage(void);

int UpToDate = 0;
int ForceUpdate = 1;
char *mp[100];
int per[9];
int numberfs;
int mode = FIRE;
int blink = 1;
int blinkper = 95;
char dummy[4096];
char *myName;
int delay = DELAY_10;

int xpos[] = { 66, 71, 76, 81, 86, 91,	/* A B C D E F */
	66, 71, 76, 81, 86, 91,	/* G H I J K L */
	66, 71, 76, 81, 86, 91,	/* M N O P Q R */
	66, 71, 76, 81, 86, 91,	/* S T U V W X */
	66, 71, 76, 81, 86, 91,	/* Y Z / _ - . */
	96, 101,		/* 0 1 */
	96, 101,		/* 2 3 */
	96, 101,		/* 4 5 */
	96, 101,		/* 6 7 */
	96, 101
};				/* 8 9 */

int ypos[] = { 4, 4, 4, 4, 4, 4,
	9, 9, 9, 9, 9, 9,
	14, 14, 14, 14, 14, 14,
	19, 19, 19, 19, 19, 19,
	24, 24, 24, 24, 24, 24,
	4, 4,
	9, 9,
	14, 14,
	19, 19,
	24, 24
};


/*  
 *   main  
 */
int
main(int argc, char *argv[])
{

	XEvent event;
	int dx, dy;
	char hostname[100];
	int i, j, k;
	int c, on[9] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
	struct statfs buffer;

	/*
	 *  Parse any command line arguments.
	 */
	myName = strdup(argv[0]);
	ParseCMDLine(argc, argv);


	openXwindow(argc, argv, wmfsm_master_xpm, wmfsm_mask_bits, wmfsm_mask_width, wmfsm_mask_height);


#ifndef SVR4
	if (gethostname(hostname, 100) != 0) {
		perror("gethostname");
		exit(10);
	}
#else
	if (sysinfo(SI_HOSTNAME, hostname, 100) < 0) {
		perror("sysinfo(SI_HOSTNAME)");
		exit(20);
	}
#endif
	while (1) {

		readFileSystems ();
		usleep (100);

		/* 
		 *   Process any pending X events.
		 */
		while (XPending(display)) {
			XNextEvent(display, &event);
			switch (event.type) {
			case Expose:
				RedrawWindow();
				break;
			case ButtonPress:
				pressEvent(&event.xbutton);
				mode = (mode + 1) % 2;
				break;
			case ButtonRelease:
				break;
			}
		}
		/*! Here read data
		 */
		for (i = 0; i < numberfs; i++) {
			if (STATFS(mp[i], &buffer) != -1) {
				per[i] = 100 - (int) ((float) buffer.f_bfree / (float) buffer.f_blocks * 100.0);
			}
		}
		/*!
		 *  Draw window.
		 */
		/*
		 * Clear window.
		 */
		copyXPMArea(5, 69, 54, 54, 5, 5);
		/*! here draw values
		 * void copyXPMArea(int x, int y, int sx, int sy, int dx, int dy)
		 */

		if (numberfs > 4) {
			for (i = 0, dy = 0; i < numberfs; i++) {
				for (j = 0, dx = 0; j < LENMP && j < strlen(mp[i]); j++) {
					k = j + (strlen(mp[i]) > LENMP ? strlen(mp[i]) - LENMP : 0);
					c = (int) mp[i][k];
					switch (c) {
					case '/':
						c = 123;
						break;
					case '_':
						c = 124;
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						c += 79;
						break;
					default:
						c = tolower(c);
						break;
					}
					copyXPMArea(xpos[c - 97], ypos[c - 97], 5, 5, 6 + dx, 5 + dy);
					dx += 5;
				}
				c = (int) ((float) per[i] * 0.52 + 66.0);
				k = (10 - LENMP) * 5 + 2;
				k = (float) per[i] / 100.0 * k;
				k = LENMP * 5 + k;
				k = k < LENMP * 5 + 1 ? LENMP * 5 + 1 : k;
				c = c > 66 ? c : 66;
				c = c > 117 ? 117 : c;
				if (blink) {
					if ((int) per[i] >= blinkper)
						on[i] = !on[i];
					else
						on[i] = 1;
				}
				else
					on[i] = 1;

				for (j = LENMP * 5; j < k; j++) {
					if (mode == FIRE && on[i])
						copyXPMArea(66 + (float) (j - LENMP * 5) * 52.0 / (float) ((10 - LENMP) * 5), 29, 1, 5, 6 + j, 5 + dy);
					else if (mode == NORMAL && on[i])
						copyXPMArea(c, 29, 1, 5, 6 + j, 5 + dy);
				}
				dy += 6;
			}
		}
		else {		/*one fs in two lines */
			for (i = 0, dy = 0; i < numberfs; i++) {
				for (j = 0, dx = 0; j < 10 && j < strlen(mp[i]); j++) {
					c = (int) mp[i][j + (strlen(mp[i]) > 10 ? strlen(mp[i]) - 10 : 0)];
					switch (c) {
					case '/':
						c = 123;
						break;
					case '_':
						c = 124;
						break;
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						c += 79;
						break;
					default:
						c = tolower(c);
						break;
					}
					copyXPMArea(xpos[c - 97], ypos[c - 97], 5, 5, 6 + dx, 5 + dy);
					dx += 5;
				}
				dy += 6;
				c = (int) ((float) per[i] * 0.52 + 66.0);
				k = (float) per[i] / 100.0 * 52;
				k = k < 1 ? 1 : k;
				c = c > 66 ? c : 66;
				c = c > 117 ? 117 : c;
				if (blink) {
					if ((int) per[i] >= blinkper)
						on[i] = !on[i];
					else
						on[i] = 1;
				}
				else
					on[i] = 1;

				for (j = 0; j < k; j++) {
					if (mode == FIRE && on[i])
						copyXPMArea(66 + (float) (j) * 52.0 / 50.0, 29, 1, 5, 6 + j, 5 + dy);
					else if (mode == NORMAL && on[i])
						copyXPMArea(c, 29, 1, 5, 6 + j, 5 + dy);
				}
				dy += 6;
			}
		}
		if (numberfs < 9) {
			for (j = 0, dx = 0, dy = 47; j < 10 && j < strlen(hostname); j++) {
				c = (int) hostname[j];
				switch (c) {
				case '/':
					c = 123;
					break;
				case '_':
					c = 124;
					break;
				case '-':
					c = 125;
					break;
				case '.':
					c = 126;
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					c += 79;
					break;
				default:
					c = tolower(c);
					break;
				}
				copyXPMArea(xpos[c - 97], ypos[c - 97], 5, 5, 6 + dx, 6 + dy);
				dx += 5;
			}
		}

		RedrawWindow();
		/* 
		 *  Wait for next update 
		 */
		i = 0;
		while (i < 10) {
			usleep(delay);
			i++;
		}
	}
}



/*
 *   ParseCMDLine()
 */
void
ParseCMDLine(int argc, char *argv[])
{

	int c, option_index, arg;
	static struct option long_options[] = {
		{"fire", no_argument, 0, 'f'},
		{"normal", no_argument, 0, 'n'},
		{"blink", no_argument, &blink, 1},
		{"noblink", no_argument, &blink, 0},
		{"delay", required_argument, 0, 'd'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};
	while (1) {
		c = getopt_long(argc, argv, "bd:fhn", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 0:	/* If blink or noblink was toggled */
			break;
		case 'b':
			blink = 1;
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'f':
			mode = FIRE;
			break;
		case 'h':
			print_usage();
			exit(0);
		case 'n':
			mode = NORMAL;
			break;
		case '?':
			break;
		default:
			print_usage();
			exit(1);
		}
	}
}


void
print_usage()
{

	printf("\nwmfsm version: %s\n", WMFSM_VERSION);
	printf("\nusage: wmfsm \n");
	printf("\t--normal, -n\t\tDraw bars in normal mode.\n");
	printf("\t--fire, -f\t\t\tDraw bars in fire mode.\n");
	printf("\t--[no]blink\t\tBlinks if a filesystem is 95 percent full.\n");
	printf("\t-display <Display>\tUse alternate X display.\n");
	printf("\t--delay <number>, -d\tUse a delay that is not the default.\n");
	printf("\t-h\t\t\tDisplay help screen.\n");
}

void
pressEvent(XButtonEvent * xev)
{
	ForceUpdate = 1;
	return;
}

void
readFileSystems()
{
	/* Look for the goods between #if defined(__OpenBSD__) -- tschroed */
#if defined(__OpenBSD__) || defined(__FreeBSD__)
#define MAXMOUNT	32
	struct statfs sfs[MAXMOUNT];
	int fscount;

	/* OpenBSD has no /etc/mtab, we use getfsstat instead -- tschroed */
	if ((fscount = getfsstat(sfs, sizeof(sfs), 0)) == -1) {
		perror("getfsstat");
		exit(1);
	}
	for (numberfs = 0; numberfs < fscount; numberfs++)
		/* We won't watch RO mounts. */
		if (sfs[numberfs].f_flags & MNT_RDONLY) {
			numberfs--;
			fscount--;
		}
		else
			mp[numberfs] = strdup(sfs[numberfs].f_mntonname);
#else /* __OpenBSD__ || __FreeBSD__ */

	FILE *fp;
	char mountPoint[255], dummy[255], fstype[255], options[255];

#if defined(SunOS)
	/* Solaris uses /etc/mnttab */
	fp = fopen("/etc/mnttab", "r");
	if (!fp) {
		fprintf(stderr, "%s:Can't open /etc/mnttab for reading\n", myName);
		exit(1);
	}
#else
	fp = fopen("/etc/mtab", "r");
	if (!fp) {
		fprintf(stderr, "%s:Can't open /etc/mtab for reading\n", myName);
		exit(1);
	}
#endif

	numberfs = 0;
	while (!feof(fp) && numberfs < 100) {
#if defined(SunOS)
		/* only five entries per row in /etc/mnttab */
		fscanf(fp, "%s %s %s %s %s\n", dummy, mountPoint, fstype, options, dummy);
#else
		fscanf(fp, "%s %s %s %s %s %s\n", dummy, mountPoint, fstype, options, dummy, dummy);
#endif

		if (
#if defined IRIX64 || defined(SunOS)
			   strcmp(fstype, "hwgfs") && strcmp(fstype, "autofs") && strcmp(fstype, "proc") && strcmp(fstype, "fd") && !strstr(options, "ignore")
#elif defined linux
			   strcmp(fstype, "proc") && strcmp(fstype, "tmpfs") && strcmp(fstype, "devfs") && strcmp(fstype, "ramfs") && strcmp(fstype, "sysfs") && strcmp(fstype, "devpts") && strcmp(fstype, "usbfs")
#else
			   1
#endif
			) {
			mp[numberfs++] = strdup(mountPoint);
		}
	}
#endif /* __OpenBSD__ || __FreeBSD__ */

	fclose (fp);
	excludeFileSystems();
}

void
excludeFileSystems()
{
	char confFileName[255];
	char workString[255];
	int i, j, exnumberfs = 0;
	int start = 0, excluded, finalnumberfs = 0;
	char *mount_points[100];
	FILE *confFile;
	int include = -1;
	int included = 0;

	strncpy(confFileName, (char *) getenv("HOME"), 245);
	strcat(confFileName, "/.wmfsmrc");
	confFile = fopen(confFileName, "r");
	if (confFile) {
		while (!feof(confFile)) {
			if (fgets(workString, 255, confFile)) {
				if(strstr(workString, "\n")){
					/* This is probably dangerous, but easy */
					*(strstr(workString, "\n")) = '\0';
				}
				if (!strcmp(workString, "[include]")) {
					include = 1;
				}
				else if (!strcmp(workString, "[exclude]")) {
					include = 0;
				}
				else {
					mount_points[exnumberfs] = strdup(workString);
					exnumberfs++;
				}
			}

		}
		if (include < 0) {
			printf("wmfsm: Please specify either to include or exclude filesystems.\n");
			exit(1);
		}
		fclose(confFile);
	}
	else {
		numberfs = numberfs > 9 ? 9 : numberfs;
		return;
	}
	if (!exnumberfs) {
		numberfs = numberfs > 9 ? 9 : numberfs;
		return;
	}
	excluded = 0;
	for (i = 0; i < numberfs; i++) {
		for (j = 0, excluded = 0; j < exnumberfs; j++) {
			if (!strcmp(mp[i], mount_points[j]) && !include) {
				excluded = 1;
			} else if (!strcmp(mp[i], mount_points[j]) && include) {
				included = 1;
			}
		}
		if ((!excluded && !include) || (included && include))
			mp[finalnumberfs++] = strdup(mp[i]);
		included = excluded = 0;
	}
	for (j = 0; j < exnumberfs; j++)
		free(mount_points[j]);
	for (j = finalnumberfs; j < numberfs; j++)
		free(mp[j]);
	numberfs = finalnumberfs > 9 ? 9 : finalnumberfs;
}
