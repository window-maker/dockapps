/*
 * Copyright (c) 2007 Daniel Borca  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "xhookey.h"


#define LOG(x)	printf x


static void
grab_key (Display *dpy, KeyCode keycode, unsigned int modifier, Window win)
{
    XGrabKey(dpy, keycode, modifier, (win ? win : DefaultRootWindow(dpy)),
	     False, GrabModeAsync, GrabModeAsync);
}


int
xhk_grab (Display *dpy, int num, KTUPLE keys[])
{
    int screen = DefaultScreen(dpy);
    for (screen = 0; screen < ScreenCount(dpy); screen++) {
	int i;
	for (i = 0; i < num; i++) {
	    grab_key(dpy, keys[i].key, keys[i].mod, RootWindow(dpy, screen));
	}
    }
    return 0;
}


void
xhk_ungrab (Display *dpy)
{
    int screen = DefaultScreen(dpy);
    for (screen = 0; screen < ScreenCount(dpy); screen++) {
	XUngrabKey(dpy, AnyKey, AnyModifier, RootWindow(dpy, screen));
    }
}


int
xhk_parse (int argc, char **argv, KTUPLE **out_keys)
{
    int i = 0;
    KTUPLE *keys;

    keys = malloc((argc - 1) * sizeof(KTUPLE));
    if (keys == NULL) {
	return -1;
    }

    while (--argc) {
	const char *p = *++argv;
	if (!strncmp(p, "-key=", 5)) {
	    char *q;
	    errno = 0;
	    p += 5;
	    keys[i].mod = 0;
	    for (;;) {
		if (!strncmp(p, "Shift+", 6)) {
		    p += 6;
		    keys[i].mod |= ShiftMask;
		    continue;
		}
		if (!strncmp(p, "Control+", 8)) {
		    p += 8;
		    keys[i].mod |= ControlMask;
		    continue;
		}
		if (!strncmp(p, "Alt+", 4)) {
		    p += 4;
		    keys[i].mod |= Mod1Mask;
		    continue;
		}
		break;
	    }
	    keys[i].key = strtol(p, &q, 0);
	    if (errno || q == p || !isspace(*q)) {
		LOG(("ignoring `%s'\n", p));
		continue;
	    }
	    while (isspace(*++q)) {
	    }
	    keys[i].command = q;
	    LOG(("key%d: 0x%X+%d -> \"%s\"\n", i, keys[i].mod, keys[i].key, keys[i].command));
	    i++;
	}
    }

    if (i) {
	*out_keys = keys;
    } else {
	free(keys);
    }
    return i;
}


#if XHK_SIGFORK
void
xhk_sig_handler (int sig)
{
    if (sig == SIGCHLD) {
	int status;
	pid_t child;
	/* if more than one child exits at approximately the same time, the signals may get merged. */
	while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
	    LOG(("pid %d exited\n", child));
	}
    }
}
#endif


#if XHK_XERROR
int
xhk_eks_handler (Display *dpy, XErrorEvent *evt)
{
    static int been_there_done_that = 0;
    if (!been_there_done_that) {
	been_there_done_that++;
	fprintf(stderr, "*** X ERROR %d\n", evt->error_code);
	(void)dpy;
    }
    return 0;
}
#endif


static int
display_env (Display *dpy)
{
    const char *display_name = DisplayString(dpy);
    char *envstr = malloc(strlen(display_name) + 8 + 1);
    if (envstr != NULL) {
	strcat(strcpy(envstr, "DISPLAY="), display_name);
	putenv(envstr);
    }
}


static int
run_command (Display *dpy, KTUPLE *key)
{
    pid_t pid;

#if XHK_SIGFORK

    pid = fork();
    if (pid < 0) {
	return -1;
    }
    if (pid == 0) {
	if (setsid() < 0) {
	    exit(-1);
	}
	display_env(dpy);
	exit(execlp("sh", "sh", "-c", key->command, (char *)0));
    }

    return 0;

#else	/* !XHK_SIGFORK */

    int status;

    pid = fork();
    if (pid < 0) {
	return -1;
    }
    if (pid == 0) {
	if (setsid() < 0) {
	    exit(-1);
	}
	/* fork again and exit, so that init(1) reaps the grandchildren */
	pid = fork();
	if (pid < 0) {
	    exit(-1);
	}
	if (pid > 0) {
	    exit(0);
	}
	display_env(dpy);
	exit(execlp("sh", "sh", "-c", key->command, (char *)0));
    }
    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);

#endif	/* !XHK_SIGFORK */
}


int
xhk_run (Display *dpy, XEvent *evt, int num, KTUPLE keys[])
{
    if (evt->type == KeyPress) {
	int i;
	unsigned int mod = evt->xkey.state & (ShiftMask | ControlMask | Mod1Mask);
	for (i = 0; i < num; i++) {
	    if (evt->xkey.keycode == keys[i].key && mod == keys[i].mod) {
		LOG(("key: 0x%X+%d, cmd: %s\n", keys[i].mod, keys[i].key, keys[i].command));
		return run_command(dpy, &keys[i]);
	    }
	}
    }
    return -1;
}
