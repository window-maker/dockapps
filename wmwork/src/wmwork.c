/* $Id: wmwork.c,v 1.67 2005/12/02 07:36:59 godisch Exp $
 * vim: set noet ts=4:
 *
 * Copyright (c) 2002-2005 Martin A. Godisch <martin@godisch.de>
 */

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <X11/xpm.h>

#ifdef SOLARIS
#  include <strings.h>
#  define PID_T "%lu"
#else  /* SOLARIS */
#  include <getopt.h>
#  define PID_T "%u"
#endif /* SOLARIS */

#include "wmgeneral.h"
#include "wmwork.h"
#include "wmwork.xpm"

#define BUT_START (1)
#define BUT_PAUSE (2)
#define BUT_STOP  (3)
#define BUT_PREV  (4)
#define BUT_NEXT  (5)

#define CHAR_SRC_X1(N) ((N - 'A') * 5)
#define CHAR_SRC_Y1    (71)
#define CHAR_SRC_X2(N) ((N - ' ') * 5)
#define CHAR_SRC_Y2    (64)
#define CHAR_SRC_X3(N) ((N - '{') * 5 + 65)
#define CHAR_SRC_Y3    (57)
#define TIMER_SRC_X(N) (N * 5 + 80)
#define TIMER_SRC_Y    (64)

char
	wmwork_mask_bits[64*64],
	*dirName  = NULL,
	*logname  = NULL,
	*lockname = NULL;
struct timeval
	now;
struct timezone
	tz;
struct Project
	*current  = NULL,
	*first    = NULL;
time_t
	sess_time = 0;
int
	path_len  = 0,
	do_exit   = 0;

static void handler(int signo)
{
	do_exit = 1;
}

static void at_exit(void)
{
	write_log();
	write_record();
	if (unlink(lockname) < 0) {
		fprintf(stderr, "%s: cannot unlink '%s'\n", PACKAGE_NAME, lockname);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
	}
}

int main(int argc, char *argv[])
{
	int n,
		show_days = 0,
		but_stat  = 0,
		microtm   = 0,
		running   = 0,
		force     = 0;
	time_t
		last_time = 0;
	char
		*geometry = NULL,
		*xdisplay = NULL;
	static int signals[] =
		{SIGALRM, SIGHUP, SIGINT, SIGPIPE, SIGTERM, SIGUSR1, SIGUSR2, 0};
	XEvent Event;

	assert(sizeof(char) == 1);

	umask(077);
	do_opts(argc, argv, &show_days, &xdisplay, &geometry, &force);

	path_len = strlen(getenv("HOME")) + strlen("/.wmwork/worklog") + 1;
	if ((dirName = malloc(path_len)) == NULL || (logname = malloc(path_len)) == NULL || (lockname = malloc(path_len)) == NULL) {
		fprintf(stderr, "%s: cannot allocate memory for path variable\n", PACKAGE_NAME);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		exit(1);
	}
	snprintf(dirName,  path_len, "%s/.wmwork", getenv("HOME"));
	snprintf(logname,  path_len, "%s/worklog", dirName);
	snprintf(lockname, path_len, "%s/.#LOCK",  dirName);
	if (chdir(dirName) < 0) {
		if (errno == ENOENT) {
			if (mkdir(dirName, 0777)) {
				fprintf(stderr, "%s: cannot mkdir '%s'\n", PACKAGE_NAME, dirName);
				fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
				exit(1);
			}
			compat();
		} else {
			fprintf(stderr, "%s: cannot chdir into '%s'\n", PACKAGE_NAME, dirName);
			fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
			exit(1);
		}
	}

	for (n = 0; signals[n]; n++) {
		if (signal(signals[n], handler) == SIG_ERR) {
			fprintf(stderr, "%s: cannot set handler for signal %d\n", PACKAGE_NAME, signals[n]);
			fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		}
	}

	make_lock(force);
	atexit(at_exit);
	read_log();
	current = first;

	initXwindow(xdisplay);
	createXBMfromXPM(wmwork_mask_bits, wmwork_master_xpm, 64, 64);
	openXwindow(argc, argv, wmwork_master_xpm, wmwork_mask_bits, 64, 64, geometry, NULL);
	AddMouseRegion(BUT_START,  5, 48, 22, 58);
	AddMouseRegion(BUT_PAUSE, 23, 48, 40, 58);
	AddMouseRegion(BUT_STOP,  41, 48, 58, 58);
	AddMouseRegion(BUT_PREV,   5, 33, 16, 43);
	AddMouseRegion(BUT_NEXT,  47, 33, 58, 43);
	drawTime(current->time, sess_time, microtm, show_days, running);
	drawProject(current->name);

	while (1) {
		last_time = now.tv_sec;
		gettimeofday(&now, &tz);
		if (running) {
			current->time += now.tv_sec - last_time;
			sess_time     += now.tv_sec - last_time;
			microtm        = now.tv_usec;
			drawTime(current->time, sess_time, microtm, show_days, running);
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
			case ButtonPress:
				n = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				switch (n) {
				case BUT_START:
				case BUT_PAUSE:
				case BUT_STOP:
				case BUT_PREV:
				case BUT_NEXT:
					ButtonDown(n);
					break;
				}
				but_stat = n;
				RedrawWindow();
				break;
			case ButtonRelease:
				n = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				switch (but_stat) {
				case BUT_START:
				case BUT_PAUSE:
				case BUT_STOP:
				case BUT_PREV:
				case BUT_NEXT:
					ButtonUp(but_stat);
					break;
				}
				if (but_stat && n == but_stat) {
					switch (but_stat) {
					case BUT_START:
						running = 1;
						break;
					case BUT_PAUSE:
						running = 0;
						break;
					case BUT_STOP:
						write_log();
						write_record();
						running   = 0;
						sess_time = 0;
						break;
					case BUT_PREV:
						if (!running && sess_time == 0)
							current = current->prev;
						break;
					case BUT_NEXT:
						if (!running && sess_time == 0)
							current = current->next;
						break;
					}
					drawTime(current->time, sess_time, microtm, show_days, running);
					drawProject(current->name);
				}
				RedrawWindow();
				but_stat = 0;
				break;
			}
		}
		usleep(50000L);
		if (do_exit)
			exit(0);
	}
}

void ButtonDown(int button)
{
	switch (button) {
	case BUT_START:
		copyXPMArea( 65, 25, 18, 11,  5, 48);
		break;
	case BUT_PAUSE:
		copyXPMArea( 84, 25, 18, 11, 23, 48);
		break;
	case BUT_STOP:
		copyXPMArea(103, 25, 18, 11, 41, 48);
		break;
	case BUT_PREV:
		copyXPMArea(122, 25, 12, 11,  5, 33);
		break;
	case BUT_NEXT:
		copyXPMArea(135, 25, 12, 11, 47, 33);
		break;
	}
}

void ButtonUp(int button)
{
	switch (button) {
	case BUT_START:
		copyXPMArea( 65, 13, 18, 11,  5, 48);
		break;
	case BUT_PAUSE:
		copyXPMArea( 84, 13, 18, 11, 23, 48);
		break;
	case BUT_STOP:
		copyXPMArea(103, 13, 18, 11, 41, 48);
		break;
	case BUT_PREV:
		copyXPMArea(122, 13, 12, 11,  5, 33);
		break;
	case BUT_NEXT:
		copyXPMArea(135, 13, 12, 11, 47, 33);
		break;
	}
}

void ButtonEnable(int button)
{
	return ButtonUp(button);
}

void ButtonDisable(int button)
{
	switch (button) {
	case BUT_START:
		copyXPMArea( 65,  1, 18, 11,  5, 48);
		break;
	case BUT_PAUSE:
		copyXPMArea( 84,  1, 18, 11, 23, 48);
		break;
	case BUT_STOP:
		copyXPMArea(103,  1, 18, 11, 41, 48);
		break;
	case BUT_PREV:
		copyXPMArea(122,  1, 12, 11,  5, 33);
		break;
	case BUT_NEXT:
		copyXPMArea(135,  1, 12, 11, 47, 33);
		break;
	}
}

void do_opts(int argc, char *argv[], int *show_days, char **xdisplay, char **geometry, int *force)
{
	static struct option long_opts[] = {
		{"days",     0, NULL, 'd'},
		{"force",    0, NULL, 'f'},
		{"help",     0, NULL, 'h'},
		{"version",  0, NULL, 'v'},
		{"display",  1, NULL,  0 },
		{"geometry", 1, NULL,  1 },
		{NULL,       0, NULL,  0}};
	int i, opt_index = 0;

#ifdef SOLARIS
	for (i = 1; i < argc; i++) {
		if (*argv[i] == '-') {
			if (argv[i][1] == 'd')
				*show_days = 1;
			else if (argv[i][1] == 'f')
				*force = 1;
			else if (argv[i][1] == 'h') {
#else /* SOLARIS */
	while (1) {
		if ((i = getopt_long(argc, argv, "dfhv", long_opts, &opt_index)) == -1)
			break;
		switch (i) {
		case 'd':
			*show_days = 1;
			break;
		case 'f':
			*force = 1;
			break;
		case 'h':
#endif /* SOLARIS */
			printf("usage: %s [options]\n", argv[0]);
			printf("  -d, --days        display time in ddd.hh:mm instead of hhh:mm:ss,\n");
			printf("  -f, --force       overwrite stale lock files,\n");
			printf("  -h, --help        display this command line summary,\n");
			printf("  -v, --version     display the version number,\n");
			printf("  --display=<id>    open the mini window on display <id>, e.g. ':0.0',\n");
			printf("  --geometry=<pos>  open the mini window at position <pos>, e.g. '+10+10'.\n");
			exit(0);
#ifdef SOLARIS
			} else if (argv[i][1] == 'v') {
#else /* SOLARIS */
		case 'v':
#endif /* SOLARIS */
			printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
			printf("copyright (c) 2002-2005 Martin A. Godisch <martin@godisch.de>\n");
			exit(0);
#ifdef SOLARIS
			} else if (!strcmp(argv[i], "--display"))
				*xdisplay = optarg;
			else if (!strcmp(argv[i], "--geometry"))
				*geometry = optarg;
			else break;
		} else
			break;
	}
	if (i < argc) {
		fprintf(stderr, "%s: invalid argument -- %s\n", PACKAGE_NAME, argv[i]);
		exit(1);
	}
#else /* SOLARIS */
		case 0:
			*xdisplay = optarg;
			break;
		case 1:
			*geometry = optarg;
			break;
		case '?':
			exit(1);
		}
	}
	if (optind < argc) {
		fprintf(stderr, "%s: invalid argument -- %s\n", PACKAGE_NAME, argv[optind]);
		exit(1);
	}
#endif /* SOLARIS */
}

void drawTime(time_t time1, time_t time2, int microtm, int show_days, const int running)
{
	time_t d1 = 0, d2 = 0, h1 = 0, h2 = 0;
	short  m1 = 0, m2 = 0, s1 = 0, s2 = 0;

	if (time1 >= 3600000 || time2 >= 3600000)
		show_days = 1;

	if (show_days) {
		d1 = time1 / 86400;
		d2 = time2 / 86400;
		time1 %=     86400;
		time2 %=     86400;
		if (d1 >= 1000 || d2 >= 1000) {
			d1 %= 1000;
			d2 %= 1000;
		}
	}
	h1 = time1 / 3600;
	h2 = time2 / 3600;
	time1 %=     3600;
	time2 %=     3600;
	m1 = time1 /   60;
	m2 = time2 /   60;
	s1 = time1 %   60;
	s2 = time2 %   60;

	if (show_days) {
		copyXPMArea(TIMER_SRC_X(d1 / 100), TIMER_SRC_Y, 5, 7,  7,  6);
		d1 %= 100;
		copyXPMArea(TIMER_SRC_X(d1 /  10), TIMER_SRC_Y, 5, 7, 13,  6);
		copyXPMArea(TIMER_SRC_X(d1 %  10), TIMER_SRC_Y, 5, 7, 19,  6);
		copyXPMArea(TIMER_SRC_X(h1 /  10), TIMER_SRC_Y, 5, 7, 29,  6);
		copyXPMArea(TIMER_SRC_X(h1 %  10), TIMER_SRC_Y, 5, 7, 35,  6);
		copyXPMArea(TIMER_SRC_X(m1 /  10), TIMER_SRC_Y, 5, 7, 45,  6);
		copyXPMArea(TIMER_SRC_X(m1 %  10), TIMER_SRC_Y, 5, 7, 51,  6);

		copyXPMArea(TIMER_SRC_X(d2 / 100), TIMER_SRC_Y, 5, 7,  7, 20);
		d2 %= 100;
		copyXPMArea(TIMER_SRC_X(d2 /  10), TIMER_SRC_Y, 5, 7, 13, 20);
		copyXPMArea(TIMER_SRC_X(d2 %  10), TIMER_SRC_Y, 5, 7, 19, 20);
		copyXPMArea(TIMER_SRC_X(h2 /  10), TIMER_SRC_Y, 5, 7, 29, 20);
		copyXPMArea(TIMER_SRC_X(h2 %  10), TIMER_SRC_Y, 5, 7, 35, 20);
		copyXPMArea(TIMER_SRC_X(m2 /  10), TIMER_SRC_Y, 5, 7, 45, 20);
		copyXPMArea(TIMER_SRC_X(m2 %  10), TIMER_SRC_Y, 5, 7, 51, 20);
	} else {
		copyXPMArea(TIMER_SRC_X(h1 / 100), TIMER_SRC_Y, 5, 7,  7,  6);
		h1 %= 100;
		copyXPMArea(TIMER_SRC_X(h1 /  10), TIMER_SRC_Y, 5, 7, 13,  6);
		copyXPMArea(TIMER_SRC_X(h1 %  10), TIMER_SRC_Y, 5, 7, 19,  6);
		copyXPMArea(TIMER_SRC_X(m1 /  10), TIMER_SRC_Y, 5, 7, 29,  6);
		copyXPMArea(TIMER_SRC_X(m1 %  10), TIMER_SRC_Y, 5, 7, 35,  6);
		copyXPMArea(TIMER_SRC_X(s1 /  10), TIMER_SRC_Y, 5, 7, 45,  6);
		copyXPMArea(TIMER_SRC_X(s1 %  10), TIMER_SRC_Y, 5, 7, 51,  6);

		copyXPMArea(TIMER_SRC_X(h2 / 100), TIMER_SRC_Y, 5, 7,  7, 20);
		h2 %= 100;
		copyXPMArea(TIMER_SRC_X(h2 /  10), TIMER_SRC_Y, 5, 7, 13, 20);
		copyXPMArea(TIMER_SRC_X(h2 %  10), TIMER_SRC_Y, 5, 7, 19, 20);
		copyXPMArea(TIMER_SRC_X(m2 /  10), TIMER_SRC_Y, 5, 7, 29, 20);
		copyXPMArea(TIMER_SRC_X(m2 %  10), TIMER_SRC_Y, 5, 7, 35, 20);
		copyXPMArea(TIMER_SRC_X(s2 /  10), TIMER_SRC_Y, 5, 7, 45, 20);
		copyXPMArea(TIMER_SRC_X(s2 %  10), TIMER_SRC_Y, 5, 7, 51, 20);
	}

	if (microtm < 500000 || !running) {
		if (show_days) {
			copyXPMArea(161, CHAR_SRC_Y1 + 4, 1, 3, 26, 10);
			copyXPMArea(161, CHAR_SRC_Y1 + 4, 1, 3, 26, 24);
		} else {
			copyXPMArea(161, CHAR_SRC_Y1, 1, 7, 26,  6);
			copyXPMArea(161, CHAR_SRC_Y1, 1, 7, 26, 20);
		}
		copyXPMArea(161, CHAR_SRC_Y1, 1, 7, 42,  6);
		copyXPMArea(161, CHAR_SRC_Y1, 1, 7, 42, 20);
	} else {
		if (!show_days) {
			copyXPMArea(163, CHAR_SRC_Y1, 1, 7, 26,  6);
			copyXPMArea(163, CHAR_SRC_Y1, 1, 7, 26, 20);
		}
		copyXPMArea(163, CHAR_SRC_Y1, 1, 7, 42,  6);
		copyXPMArea(163, CHAR_SRC_Y1, 1, 7, 42, 20);
	}
}

void drawProject(const char *name)
{
	int i;

	for (i = 0; i < 3; i++) {
		copyXPMArea(CHAR_SRC_X2(' '), CHAR_SRC_Y2, 5, 7, 23 + i * 6, 35);
		if (i >= strlen(name))
			continue;
		if (name[i] >= 'A' && name[i] <= '`')
			copyXPMArea(CHAR_SRC_X1(name[i]), CHAR_SRC_Y1, 5, 7, 23 + i * 6, 35);
		else if (name[i] >= ' ' && name[i] <= '@')
			copyXPMArea(CHAR_SRC_X2(name[i]), CHAR_SRC_Y2, 5, 7, 23 + i * 6, 35);
		else if (name[i] >= '{' && name[i] <= '~')
			copyXPMArea(CHAR_SRC_X3(name[i]), CHAR_SRC_Y3, 5, 7, 23 + i * 6, 35);
	}
}

int read_log(void)
{
	struct Project
		*p = NULL;
	FILE
		*F = NULL;
	char
		buffer[512],
		*colon = NULL,
		*s     = NULL;
	int i, line, n = 0;

	if ((first = malloc(sizeof(struct Project))) == NULL) {
		fprintf(stderr, "%s: cannot allocate memory for element %d\n", PACKAGE_NAME, n);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		exit(1);
	}
	strcpy(first->name, "---");
	first->time = 0;
	first->prev = first;
	first->next = first;

	if ((F = fopen(logname, "r")) == NULL) {
		if (errno != ENOENT) {
			fprintf(stderr, "%s: cannot open '%s' for reading\n", PACKAGE_NAME, logname);
			fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		}
		return 1;
	}

	for (line = 1;; line++) {
		fgets(buffer, sizeof(buffer), F);
		if (feof(F))
			break;

		if (n == 0) {
			p = first;
		} else if ((p = malloc(sizeof(struct Project))) == NULL) {
			fprintf(stderr, "%s: cannot allocate memory for element %d\n", PACKAGE_NAME, n);
			fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
			return(n);
		}

		if ((s = index(buffer, '\n')) != NULL)
			*s = '\0';
		if (buffer[0] == '#' || buffer[0] == '\0') {
			if (n)
				free(p);
			continue;
		}
		if ((colon = index(buffer, ':')) == NULL) {
			fprintf(stderr, "%s: missing ':' in '%s' line %d\n", PACKAGE_NAME, logname, line);
			if (n)
				free(p);
			continue;
		}
		i = colon - buffer < 3 ? colon - buffer : 3;
		p->name[i] = '\0';
		for (i--; i >= 0; i--) {
			p->name[i] = toupper(buffer[i]);
			if (p->name[i] == '/') {
				fprintf(stderr, "%s: '/' is fs delimiter in '%s' line %d\n", PACKAGE_NAME, logname, line);
				fprintf(stderr, "%s: converting forbidden '/' in project id into '|'\n", PACKAGE_NAME);
				p->name[i] = '|';
			}
		}
		p->time = strtol(++colon, &s, 10);
		if (*s && *s != ':' && *s != ' ') {
			fprintf(stderr, "%s: error converting timestamp '%s' in '%s' line %d\n", PACKAGE_NAME, colon, logname, line);
			if (n)
				free(p);
			continue;
		}
		if (*s++ == ':') {
			if ((p->comment = strdup(s)) == NULL) {
				fprintf(stderr, "%s: ignored error while aquiring memory for comment string '%s' in '%s' line %d\n", PACKAGE_NAME, s, logname, line);
				fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
			}
		} else
			p->comment = NULL;

		p->prev = first->prev;
		p->next = first;
		p->prev->next = p;
		p->next->prev = p;
		n++;
	}

	fclose(F);
	return(n);
}

int write_log(void)
{
	struct Project
		*p = NULL;
	FILE
		*F = NULL;

	if ((F = fopen(logname, "w")) == NULL) {
		fprintf(stderr, "%s: cannot open '%s' for writing\n", PACKAGE_NAME, logname);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		F = stderr;
	} else {
		fprintf(F, "# wmwork configuration file\n");
		fprintf(F, "# do not edit while wmwork is running\n\n");
	}
	p = first;
	do {
		if (F == stderr)
			fprintf(F, "%s: > %s:%li:%s\n", PACKAGE_NAME, p->name, p->time, p->comment ? p->comment : "");
		else
			fprintf(F, "%s:%li:%s\n", p->name, p->time, p->comment ? p->comment : "");
		p = p->next;
	} while (p != first);

	if (F == stderr)
		return 0;
	fclose(F);
	return 1;
}

int write_record(void)
{
	char
		*fname = NULL,
		tbuff[64],
		rbuff[64];
	FILE
		*F = NULL;

	if (sess_time == 0)
		return 1;
	strftime(tbuff, sizeof(tbuff), "%a, %d %b %Y %H:%M:%S %z", localtime(&now.tv_sec));
	snprintf(rbuff, sizeof(rbuff), "%s %03li:%02li:%02li", tbuff, sess_time / 3600, sess_time / 60 % 60, sess_time % 60);

	if ((fname = malloc(path_len)) == NULL) {
		fprintf(stderr, "%s: cannot allocate memory for path variable\n", PACKAGE_NAME);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		fprintf(stderr, "%s: > %s\n", PACKAGE_NAME, rbuff);
		return 0;
	}
	snprintf(fname, path_len, "%s/.wmwork/%s", getenv("HOME"), current->name);
	if ((F = fopen(fname, "a")) == NULL) {
		fprintf(stderr, "%s: cannot open '%s' for writing\n", PACKAGE_NAME, fname);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		fprintf(stderr, "%s: > %s\n", PACKAGE_NAME, rbuff);
		free(fname);
		return 0;
	}
	fprintf(F, "%s\n", rbuff);
	fclose(F);
	free(fname);
	return 1;
}

int make_lock(int force)
{
	FILE
		*F  = NULL;
	pid_t
		pid = 0;
	int
		fd  = 0,
		i   = 0;
	char
		*c  = NULL,
		proc[32],
		buffer[256];

	if ((fd = open(lockname, O_WRONLY | O_CREAT | (force ? 0 : O_EXCL), 0666)) < 0) {
		if (errno == EEXIST) {
			if ((F = fopen(lockname, "r")) != NULL) {
				i = fscanf(F, PID_T, &pid);
				fclose(F);
				snprintf(proc, sizeof(proc), "/proc/"PID_T"/exe", pid);
				if (i == 1 && ((i = readlink(proc, buffer, sizeof(buffer)-1)) >= 0 || errno == ENOENT)) {
					buffer[i] = 0;
					if (i < 0 || ((c = rindex(buffer, '/')) != NULL && strcmp(c+1, "wmwork") != 0)) {
						fprintf(stderr, "%s: found stale lock file (pid "PID_T")\n", PACKAGE_NAME, pid);
						return make_lock(1);
					}
				}
			}
			fprintf(stderr, "%s: already running, --force will overwrite the lock file\n", PACKAGE_NAME);
			exit(1);
		}
		fprintf(stderr, "%s: cannot create '%s'\n", PACKAGE_NAME, lockname);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		return 0;
	}
	if ((F = fdopen(fd, "w")) != NULL) {
		fprintf(F, PID_T"\n", getpid());
		fclose(F);
	} else
		close(fd);
	return 1;
}

int compat(void)
{
	char
		*temp1 = NULL,
		*temp2 = NULL;
	DIR *dir   = NULL;
	struct dirent
		*entry = NULL;

	/* BEGIN compatibility section for wmwork < 0.2.0 */

	if ((temp1 = malloc(path_len)) == NULL || (temp2 = malloc(path_len)) == NULL) {
		fprintf(stderr, "%s: cannot allocate memory for path variable\n", PACKAGE_NAME);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		return -1;
	}

	snprintf(temp1, path_len, "%s/.wmworklog", getenv("HOME"));
	if (rename(temp1, logname) < 0) {
		if (errno == ENOENT)
			return 0;
		fprintf(stderr, "%s: cannot rename '%s' to '%s'\n", PACKAGE_NAME, temp1, logname);
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		return -1;
	} else
		fprintf(stderr, "%s: moving '%s' -> '%s'\n", PACKAGE_NAME, temp1, logname);

	if ((dir = opendir(getenv("HOME"))) < 0) {
		fprintf(stderr, "%s: cannot read '%s'\n", PACKAGE_NAME, getenv("HOME"));
		fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
		return -1;
	}
	while ((entry = readdir(dir))) {
		if (strstr(entry->d_name, ".wmwork.") == entry->d_name &&
			strlen(entry->d_name) > 8 && strlen(entry->d_name) <= 11) {
			snprintf(temp1, path_len, "%s/%s", getenv("HOME"), entry->d_name);
			snprintf(temp2, path_len, "%s/%s", dirName, entry->d_name + 8);
			if (rename(temp1, temp2) < 0) {
				fprintf(stderr, "%s: cannot rename '%s' to '%s'\n", PACKAGE_NAME, temp1, temp2);
				fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
			} else
				fprintf(stderr, "%s: moving '%s' to '%s'\n", PACKAGE_NAME, temp1, temp2);
		}
	}
	closedir(dir);
	return 0;

	/* END compatibility section for wmwork < 0.2.0 */
}
