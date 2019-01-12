#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <libdockapp/wmgeneral.h>

#include "asmon-master.xpm"
#include "asmon-mask.xbm"

#ifdef __solaris__
#include <utmp.h>
#endif

#define EXEC_ON_CLICK 1
#define ASMON_VERSION "0.72"
#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7

#define BCHAR_WIDTH 6
#define BCHAR_HEIGHT 9

#define BOFFX   (9)
#define BOFFY   (111)
#define BREDX   (3)
#define BREDY   (111)
#define BGREENX (87)
#define BGREENY (66)

#define LITEW   (4)
#define LITEH   (4)

#define B_OFF   (0)
#define B_RED   (1)
#define B_GREEN (2)

/* Evil globals I haven't removed yet */
long last_pageins = 0, last_pageouts = 0;
long last_swapins = 0, last_swapouts = 0;
//double old;
static int has_kern26 = 0;

#ifdef EXEC_ON_CLICK
char Command[256] = "";
#endif

/* functions */
void DrawUptime(void);
void usage(void);
void printversion(void);
void asmon_routine(int Xpid, int allmem);
void DrawLite(int state, int dx, int dy);
void DrawCPU(void);
void DrawLoad(void);
#ifdef __solaris__
float DrawMemSwap(void);
extern int getLoad(float *);
extern int getSwap(unsigned long *, unsigned long *);
extern int getMem(unsigned long *, unsigned long *);
extern int getCPU(unsigned long *, unsigned long *,
		  unsigned long *, unsigned long *,
		  unsigned long *, unsigned long *,
		  unsigned long *, unsigned long *);
#else
void DrawXmem(int Xpid, float total);
float DrawMemSwap(float total, int allmem);
#endif

int main(int argc, char *argv[])
{
	FILE *fp;
	int i;
	int allmem = 1;
	int Xpid = 1;
	char *ProgName;
	struct utsname name;
	int kernMajor, kernMinor, kernRev;

	ProgName = argv[0];

	if (strlen(ProgName) >= 5)
		ProgName += (strlen(ProgName) - 5);

	for (i = 1; i < argc; i++) {
		char *arg = argv[i];
		if (*arg == '-') {
			switch (arg[1]) {
			case 'd':
				if (strcmp(arg + 1, "display")) {
					usage();
					exit(1);
				}
				break;
#ifdef EXEC_ON_CLICK
			case 'e':
				if (argv[++i]) {
					strncpy(Command, argv[i], 253);
					strcat(Command, " &");
				} else {
					usage();
					exit(0);
				}
				break;
#endif
			case 'v':
				printversion();
				exit(0);
				break;
			case 'u':
#ifdef __solaris__
				fprintf(stderr,
					"X Server memory stats unavailable for Solaris.\n");
				exit(0);
#endif
				Xpid = 0;
				break;
			default:
				usage();
				exit(0);
				break;
			}
		}
	}
#ifndef __solaris__
	if (Xpid != 0) {
		if ((fp = fopen("/var/run/server.0.pid", "r")) != NULL) {
			fscanf(fp, " %d", &Xpid);
			fclose(fp);
		} else {
			if ((fp = fopen("/tmp/.X0-lock", "r")) != NULL) {
				fscanf(fp, " %d", &Xpid);
				fclose(fp);
			} else {
				Xpid = 0;
			}
		}
	}
#endif
	/* Open 64x64 window */
	openXwindow(argc, argv, asmon_master_xpm, asmon_mask_bits,
		    asmon_mask_width, asmon_mask_height);

	if (uname(&name) != -1) {
		if (strcmp(name.sysname, "Linux") == 0) {
			sscanf(name.release, "%d.%d.%d", &kernMajor, &kernMinor,
			       &kernRev);
			if ((kernMajor == 2) && (kernMinor == 6))
				has_kern26 = 1;
		}
	} else {
		fprintf(stderr, "Can't find system name\n");
		exit(1);
	}

	asmon_routine(Xpid, allmem);
	return (0);
}

/**************************************************************************/

void usage(void)
{
	fprintf(stderr,
		"\nasmon %s - by Brad Hall (bkh@rio.vg)\n\t\toriginally based on Timecop's wmcpu\n\n",
		ASMON_VERSION);
	fprintf(stderr,
		"The top bar: left is the CPU usage, right is the load average\n");
	fprintf(stderr,
		"The middle bar: left memory usage devided by ticks into shared, buffers, and\n\t\t cached, respectively, and the number of megs used\n");
	fprintf(stderr,
		"The lower bar: the left swap usage and the number of megs swappedd avg\n");
	fprintf(stderr,
		"The bottom: the left is a set of LED's marking page's and swap's, the right is\n\t\t a bar representing the amount of memory that the X server \n\t\t is taking up, and the exact megs\n\n usage:\n");
	fprintf(stderr, "\t-display <display name>\n");
	fprintf(stderr, "\t-h\tthis screen\n");
	fprintf(stderr, "\t-v\tprint the version number\n");
#ifndef __solaris__
	fprintf(stderr,
		"\t-u\tforce asmon to show uptime, rather than X mem use\n");
#endif
#ifdef EXEC_ON_CLICK
	fprintf(stderr, "\t-e cmd\texecute 'cmd' on mouse click\n");
#endif
	fprintf(stderr, "\n");
}

/**************************************************************************/

void printversion(void)
{
	fprintf(stderr, "asmon %s\n", ASMON_VERSION);
}

/**************************************************************************/

void asmon_routine(int Xpid, int allmem)
{
	int xpm_X = 0, xpm_Y = 0, count = 0;
	XEvent Event;
	float total = 0.0;

	while (1) {
		DrawCPU();
		DrawLoad();

		/* Only run every 15 iterations */
		if (count == 0 || count == 15)
#ifdef __solaris__
			total = DrawMemSwap();
#else
			total = DrawMemSwap(total, allmem);
#endif

#ifdef __solaris__
		DrawUptime();
#else
		/* X mem or Uptime? */
		if (Xpid == 0) {
			DrawUptime();
		} else {
			if (count == 5)
				DrawXmem(Xpid, total);
		}
#endif

		/* Redraw Windows */
		RedrawWindowXY(xpm_X, xpm_Y);
		while (XPending(display)) {
			XNextEvent(display, &Event);
			switch (Event.type) {
#ifdef EXEC_ON_CLICK
			case ButtonPress:
#if 0
				fprintf(stderr, "system(%s)\n", Command);
#endif
				if (Command[0])
					system(Command);
				break;
#endif
			case Expose:
				RedrawWindowXY(xpm_X, xpm_Y);
				break;
			case DestroyNotify:
				XCloseDisplay(display);
				exit(0);
				break;
			}
		}
		count++;
		if (count > 30)
			count = 0;
		usleep(150000);
	}
}

/**************************************************************************/

#ifdef __solaris__

/* CPU Usage Meter */
void DrawCPU(void
    ) {
	unsigned long cpuIdle, cpuUser, cpuKern, cpuWait;
	unsigned long pageIn, pageOut, swapIn, swapOut;

	/* remember the statistics read last time */
	static float cpustat[4] = { 0.0, 0.0, 0.0, 0.0 };
	float fields[4] = { 0.0, 0.0, 0.0, 0.0 };
	float cputotal = 0.0;

	getCPU(&cpuIdle, &cpuUser, &cpuKern, &cpuWait,
	       &pageIn, &pageOut, &swapIn, &swapOut);

	// Calculate CPU stuff
	fields[0] = ((float)cpuIdle - cpustat[0]);
	cpustat[0] = (float)cpuIdle;
	cputotal += fields[0];

	fields[1] = ((float)cpuUser - cpustat[1]);
	cpustat[1] = (float)cpuUser;
	cputotal += fields[1];

	fields[2] = ((float)cpuKern - cpustat[2]);
	cpustat[2] = (float)cpuKern;
	cputotal += fields[2];

	fields[3] = ((float)cpuWait - cpustat[3]);
	cpustat[3] = (float)cpuWait;
	cputotal += fields[3];

	// CPU Bar
	if (cputotal > 0) {
		cputotal = ((cputotal - (fields[0] + fields[2])) * 1.55);
		if (cputotal > 26)
			cputotal = 26;
		copyXPMArea(3, 84, cputotal, 9, 5, 5);
		copyXPMArea(15, 105, (27 - cputotal), 9, (5 + cputotal), 5);
		copyXPMArea(16, 46, 2, 14, 32, 2);
	}
	// Page In/Out
	if (pageIn > last_pageins)
		DrawLite(B_RED, 5, 48);
	else
		DrawLite(B_OFF, 5, 48);

	if (pageOut > last_pageouts)
		DrawLite(B_RED, 10, 48);
	else
		DrawLite(B_OFF, 10, 48);

	last_pageins = pageIn;
	last_pageouts = pageOut;

	// Swap In/Out
	if (swapIn > last_swapins)
		DrawLite(B_RED, 5, 53);
	else
		DrawLite(B_OFF, 5, 53);

	if (swapOut > last_swapouts)
		DrawLite(B_RED, 10, 53);
	else
		DrawLite(B_OFF, 10, 53);

	last_swapins = swapIn;
	last_swapouts = swapOut;
}

#else

/* CPU Usage Meter */
void DrawCPU(void)
{
	FILE *fp;
	static double cpustat[7];	/* remember the statistics read last time */
	//double fields[7], info[7], cputotal=0.0,idlee=0.0;
	double fields[7], info[7], cputotal = 0.0;
	long pageins = 0, pageouts = 0, swapins = 0, swapouts = 0;
	char buf[128];
	int i;

	if ((fp = fopen("/proc/stat", "r")) != NULL) {
		if (has_kern26 > 0) {
			// CPU data
			fscanf(fp, "cpu %lf %lf %lf %lf %lf %lf %lf", info,
			       info + 1, info + 2, info + 3, info + 4, info + 5,
			       info + 6);

			fclose(fp);

			if ((fp = fopen("/proc/vmstat", "r")) != NULL) {
				// gather data for LED's
				while (fgets(buf, 127, fp)) {
					if (strstr(buf, "pgpgin"))
						sscanf(buf, "pgpgin %ld",
						       &pageins);

					if (strstr(buf, "pgpgout"))
						sscanf(buf, "pgpgout %ld",
						       &pageouts);

					if (strstr(buf, "pswpin"))
						sscanf(buf, "pswpin %ld",
						       &swapins);

					if (strstr(buf, "pswpout"))
						sscanf(buf, "pswpout %ld",
						       &swapouts);
				}
				fclose(fp);
			}
		} else {
			// CPU data
			fscanf(fp, "cpu %lf %lf %lf %lf", info, info + 1,
			       info + 2, info + 3);

			// gather data for LED's
			while (fgets(buf, 127, fp)) {
				if (strstr(buf, "page"))
					sscanf(buf, "page %ld %ld", &pageins,
					       &pageouts);

				if (strstr(buf, "swap"))
					sscanf(buf, "swap %ld %ld", &swapins,
					       &swapouts);
			}
			fclose(fp);
		}

		// Calculate CPU stuff
		if (has_kern26 > 0) {
			for (i = 0; i < 7; i++) {
				fields[i] = info[i] - cpustat[i];
				cputotal += fields[i];
				cpustat[i] = info[i];
			}
		} else {
			for (i = 0; i < 4; i++) {
				fields[i] = info[i] - cpustat[i];
				cputotal += fields[i];
				cpustat[i] = info[i];
			}
		}
		//idlee=info[3]-old;

		//old=info[3]; 

		// CPU Bar

		//cputotal = 100 * l1 ;
		//cputotal=(100-(idlee*100/16))*26/100;
		if (cputotal > 0) {
			cputotal = (cputotal - (fields[3] + fields[4])) * 1.55;
			if (cputotal > 26)
				cputotal = 26;
			copyXPMArea(3, 84, cputotal, 9, 5, 5);
			copyXPMArea(15, 105, (27 - cputotal), 9, (5 + cputotal),
				    5);
			copyXPMArea(16, 46, 2, 14, 32, 2);
		}
		// Page In/Out
		if (pageins > last_pageins) {
			DrawLite(B_RED, 5, 48);
		} else {
			DrawLite(B_OFF, 5, 48);
		}

		if (pageouts > last_pageouts) {
			DrawLite(B_RED, 10, 48);
		} else {
			DrawLite(B_OFF, 10, 48);
		}
		last_pageins = pageins;
		last_pageouts = pageouts;

		// Swap In/Out
		if (swapins > last_swapins) {
			DrawLite(B_RED, 5, 53);
		} else {
			DrawLite(B_OFF, 5, 53);
		}

		if (swapouts > last_swapouts) {
			DrawLite(B_RED, 10, 53);
		} else {
			DrawLite(B_OFF, 10, 53);
		}
		last_swapins = swapins;
		last_swapouts = swapouts;
	}
}

#endif

/**************************************************************************/

/* Load Average */
void DrawLoad(void)
{
	int tempy, tempa;
	static float oldv = -1.0;
	float ftmp;

#ifdef __solaris__
	if (getLoad(&ftmp) != -1) {
#else
	FILE *fp;
	if ((fp = fopen("/proc/loadavg", "r")) != NULL) {
		fscanf(fp, "%f", &ftmp);
		fclose(fp);
#endif
		if (oldv != ftmp) {
			oldv = ftmp;
			tempa = (ftmp + 0.005) * 100;
			tempy = tempa % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 50, 5);
			tempy = tempa / 10;
			tempy = tempy % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 44, 5);
			copyXPMArea(65, 66, 3, 9, 41, 5);
			tempy = tempa / 100;
			if (tempy > 9) {
				tempy = (tempy - 10);
				copyXPMArea(3 + (tempy * 6), 95, 6, 9, 34, 5);
			} else {
				copyXPMArea(3 + (tempy * 6), 66, 6, 9, 34, 5);
			}
		}
	}
}

/**************************************************************************/

#ifdef __solaris__

/* Mem/Swap Meter */
float DrawMemSwap(void
    ) {
	unsigned long memMax, memFree, swapMax, swapFree;
	unsigned long MEMmem, MEMswap;
	float memUsed, swapUsed;
	int tempy, tempa;

	getMem(&memMax, &memFree);
	memUsed = (float)(memMax - memFree);

	getSwap(&swapMax, &swapFree);
	swapUsed = (float)(swapMax - swapFree);

	/* MEM Meter */
	if (memMax == 0)
		MEMmem = 0;
	else {
		if (((float)memMax / 1048576) >= 1)
			MEMmem = ((memUsed * 31) / (float)memMax);
		else
			MEMmem = ((memUsed * 36) / (float)memMax);
	}

	// refresh
	copyXPMArea(4, 115, 55, 11, 4, 18);

	// Bar
	copyXPMArea(3, 75, MEMmem, 9, 5, 19);
	copyXPMArea(15, 105, (36 - MEMmem), 9, (5 + MEMmem), 19);
	// Numbers
	tempa = (memUsed / 1048576);
	tempy = (tempa % 10);
	copyXPMArea((3 + (tempy * 6)), 66, 6, 9, 50, 19);
	tempy = ((tempa / 10) % 10);
	copyXPMArea((3 + (tempy * 6)), 66, 6, 9, 44, 19);
	tempy = ((tempa / 100) % 10);
	if (tempy != 0) {
		copyXPMArea((3 + (tempy * 6)), 66, 6, 9, 38, 19);
		copyXPMArea(16, 46, 2, 14, 35, 16);
	} else {
		copyXPMArea(16, 46, 2, 14, 41, 16);
	}

	// refresh
	copyXPMArea(4, 115, 55, 11, 4, 32);

	/* SWAP Meter */
	if (swapMax == 0)
		MEMswap = 0;
	else {
		if (((float)swapMax / 1048576) >= 1)
			MEMswap = ((swapUsed * 31) / (float)swapMax);
		else
			MEMswap = ((swapUsed * 36) / (float)swapMax);
	}
	// Bar
	copyXPMArea(3, 75, MEMswap, 9, 5, 33);
	copyXPMArea(15, 105, (36 - MEMswap), 9, (5 + MEMswap), 33);
	// Numbers
	tempa = (swapUsed / 1048576);
	tempy = (tempa % 10);
	copyXPMArea((3 + (tempy * 6)), 66, 6, 9, 50, 33);
	tempy = ((tempa / 10) % 10);
	copyXPMArea((3 + (tempy * 6)), 66, 6, 9, 44, 33);
	tempy = ((tempa / 100) % 10);
	if (tempy != 0) {
		copyXPMArea((3 + (tempy * 6)), 66, 6, 9, 38, 33);
		copyXPMArea(16, 46, 2, 14, 42, 16);
	} else {
		copyXPMArea(16, 46, 2, 14, 41, 30);
	}

	return (float)memMax;
}

#else

/* Mem/Swap Meter */
float DrawMemSwap(float total, int allmem)
{
	FILE *fp;
	if ((fp = fopen("/proc/meminfo", "r")) != NULL) {
		static float stotal = 0.0, sshared = 0.0, sbuffers =
		    0.0, scached = 0.0;
		char junk[128];
		float used, freeM, shared, buffers, cached, swaptotal,
		    swapused, swapfreeM;
		unsigned long MEMshar, MEMbuff, MEMswap;
		int tempy, tempa;

		if (has_kern26 > 0) {
			float scratch;

			while (!feof(fp)) {
				fgets(junk, 120, fp);
				if (strstr(junk, "MemTotal")) {
					sscanf(junk, "MemTotal: %f kB",
					       &scratch);
					total = scratch * 1024;
				}
				if (strstr(junk, "MemFree")) {
					sscanf(junk, "MemFree: %f kB",
					       &scratch);
					freeM = scratch * 1024;
					used = total - freeM;
				}
				if (strstr(junk, "Buffers")) {
					sscanf(junk, "Buffers: %f kB",
					       &scratch);
					buffers = scratch * 1024;
				}
				if (strstr(junk, "Cached")) {
					sscanf(junk, "Cached: %f kB", &scratch);
					cached = scratch * 1024;
				}
				if (strstr(junk, "SwapTotal")) {
					sscanf(junk, "SwapTotal: %f kB",
					       &scratch);
					swaptotal = scratch * 1024;
				}
				if (strstr(junk, "SwapFree")) {
					sscanf(junk, "SwapFree: %f kB",
					       &scratch);
					swapfreeM = scratch * 1024;
					swapused = swaptotal - swapfreeM;
				}
			}
		} else {
			fgets(junk, 80, fp);
			fscanf(fp, "Mem: %f %f %f %f %f %f\nSwap: %f %f %f",
			       &total, &used, &freeM, &shared, &buffers,
			       &cached, &swaptotal, &swapused, &swapfreeM);
		}
		fclose(fp);

		/* All mem areas */
		if (stotal != total || sshared != shared || sbuffers != buffers
		    || scached != cached) {
			stotal = total;
			sshared = shared;
			sbuffers = buffers;
			scached = cached;
			if ((total / 101048576) >= 1) {
				MEMshar =
				    ((used - buffers - cached) / total) * 27;
				MEMbuff = (buffers / total) * 27;
			} else {
				MEMshar =
				    ((used - buffers - cached) / total) * 33;
				MEMbuff = (buffers / total) * 33;
			}
			// refresh
			copyXPMArea(4, 115, 55, 11, 4, 18);
			// Bar
			if ((total / 101048576) >= 1) {
				copyXPMArea(3, 75, ((used / total) * 28), 9, 5,
					    19);
			} else {
				copyXPMArea(3, 75, ((used / total) * 34), 9, 5,
					    19);
			}
			// Separators
			copyXPMArea(15, 105, 1, 9, 5 + MEMshar, 19);
			copyXPMArea(15, 105, 1, 9, 7 + MEMshar + MEMbuff, 19);
			copyXPMArea(15, 105, (36 - (used / total) * 34), 9,
				    (5 + (used / total) * 34), 19);
			// Numbers                      
			tempa = used / 1048576;
			tempy = tempa % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 50, 19);
			tempy = (tempa / 10) % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 44, 19);
			tempy = (tempa / 100) % 10;
			if ((total / 101048576) >= 1) {
				copyXPMArea(3 + (tempy * 6), 66, 6, 9, 38, 19);
				copyXPMArea(16, 46, 2, 14, 35, 16);
			} else {
				copyXPMArea(16, 46, 2, 14, 41, 16);
			}
		}
		/* SWAP Meter */
		if (swaptotal == 0)
			MEMswap = 0;
		else {
			if ((total / 101048576) >= 1)
				MEMswap = (swapused * 31) / swaptotal;
			else
				MEMswap = (swapused * 36) / swaptotal;
		}
		// refresh
		copyXPMArea(4, 115, 55, 11, 4, 32);
		// Bar
		copyXPMArea(3, 75, MEMswap, 9, 5, 33);
		copyXPMArea(15, 105, (36 - (MEMswap)), 9, 5 + MEMswap, 33);
		// Numbers
		tempa = swapused / 1048576;
		tempy = tempa % 10;
		copyXPMArea(3 + (tempy * 6), 66, 6, 9, 50, 33);
		tempy = (tempa / 10) % 10;
		copyXPMArea(3 + (tempy * 6), 66, 6, 9, 44, 33);
		tempy = tempa / 100;
		if (tempy != 0) {
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 38, 33);
			copyXPMArea(16, 46, 2, 14, 35, 30);
		} else {
			copyXPMArea(16, 46, 2, 14, 41, 30);
		}
	}
	return (total);
}

#endif

/**************************************************************************/

#ifndef __solaris__

/* X Mem Usage */
void DrawXmem(int Xpid, float total)
{
	FILE *fp;
	char buf[128], XFileName[256];
	float ratio;
	long old_Xsize = -1, Xsize = 0;

	sprintf(XFileName, "/proc/%d/status", Xpid);

	if ((fp = fopen(XFileName, "r")) != NULL) {
		while (fgets(buf, 127, fp)) {
			if (strstr(buf, "VmSize"))
				sscanf(buf, "VmSize: %ld", &Xsize);
		}
		if (old_Xsize != Xsize) {
			int tempy, tempa, tempb;
			old_Xsize = Xsize;
			ratio = Xsize / (total / 1024);
			if (Xsize > (total / 1024))
				Xsize = total / 1024;
			Xsize = Xsize / 1024;
			tempy = Xsize % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 50, 48);
			tempa = Xsize / 10;
			tempy = tempa % 10;
			tempb = Xsize / 100;
			if (Xsize > 100) {
				copyXPMArea(3, 84, ((ratio) * 17), 11, 18, 47);
				copyXPMArea(15, 105, (23 - ((ratio) * 17)), 11,
					    (18 + (ratio * 22)), 47);
				copyXPMArea(3 + (tempy * 6), 66, 6, 9, 44, 48);
				copyXPMArea(3 + (tempb * 6), 66, 6, 9, 38, 48);
				copyXPMArea(16, 46, 2, 14, 36, 46);
			} else {
				copyXPMArea(3, 84, ((ratio) * 22), 11, 18, 47);
				copyXPMArea(15, 105, (23 - ((ratio) * 22)), 11,
					    (18 + (ratio * 22)), 47);
				copyXPMArea(3 + (tempy * 6), 66, 6, 9, 44, 48);
				copyXPMArea(16, 46, 2, 14, 41, 46);
			}
		}
		fclose(fp);
	}
}

#endif

/**************************************************************************/

/* Uptime */
void DrawUptime(void)
{
	int upt, days = 0, hours = 0, mins = 0, old_mins = -1, old_hours = -1;

#ifdef __solaris__
	struct utmp *pUtmp;
	struct utmp idUtmp;
	idUtmp.ut_type = BOOT_TIME;
	setutent();
	pUtmp = getutid(&idUtmp);
	upt = (time(0) - pUtmp->ut_time);
#else
	FILE *fp;
	if ((fp = fopen("/proc/uptime", "r")) != NULL)
		fscanf(fp, "%d", &upt);
	fclose(fp);
#endif
	mins = (upt / 60) % 60;
	hours = (upt / 3600) % 24;
	days = (upt / 86400);
	if (old_hours != hours)
		old_hours = hours;
	if (old_mins != mins) {
		int tempy;
		old_mins = mins;
		if (days > 9) {
			copyXPMArea(20, 105, 36, 9, 18, 48);
			tempy = hours % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 50, 48);
			tempy = hours / 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 44, 48);
			copyXPMArea(63, 66, 3, 9, 41, 48);
			tempy = days % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 34, 48);
			tempy = (days / 10) % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 28, 48);
			tempy = days / 100;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 22, 48);
		} else {
			tempy = mins % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 50, 48);
			tempy = mins / 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 44, 48);
			copyXPMArea(63, 66, 3, 9, 41, 48);
			tempy = hours % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 34, 48);
			tempy = hours / 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 28, 48);
			copyXPMArea(63, 66, 3, 9, 25, 48);
			tempy = days % 10;
			copyXPMArea(3 + (tempy * 6), 66, 6, 9, 18, 48);

		}
	}
}

/**************************************************************************/

/* Drawing LED's */
void DrawLite(int state, int dx, int dy)
{
	switch (state) {
	case B_RED:
		copyXPMArea(BREDX, BREDY, LITEW, LITEH, dx, dy);
		break;
	case B_GREEN:
		copyXPMArea(BGREENX, BGREENY, LITEW, LITEH, dx, dy);
		break;
	default:
	case B_OFF:
		copyXPMArea(BOFFX, BOFFY, LITEW, LITEH, dx, dy);
		break;
	}

}

/* EOF */
