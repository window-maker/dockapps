/*
 * Copyright (c) 1999-2003 Alfredo K. Kojima, Alban Hertroys
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#include "dockapp.h"
#include "daargs.h"
#include "dautil.h"
#include <ctype.h>

#define MIN(a, b)       (a < b ? a : b)

struct DAContext *_daContext;

DARect DANoRect = {0, 0, 0, 0};
Display         *DADisplay = NULL;
Window DALeader = None;
Window DAIcon = None;
Window DAWindow = None;
int DADepth = 0;
Visual          *DAVisual = NULL;
unsigned long DAExpectedVersion = 0;
GC DAGC = NULL, DAClearGC = NULL;
DARect DAPreferredIconSizes = {-1, -1, 0, 0};
Atom WM_DELETE_WINDOW;


void
DAOpenDisplay(char *display, int argc, char **argv)
{
	/* Open Connection to X Server */
	DADisplay = XOpenDisplay(display);
	if (!DADisplay) {
		printf("%s: could not open display %s!\n", _daContext->programName,
		       XDisplayName(display));
		exit(EXIT_FAILURE);
	}

	DADepth     = DefaultDepth(DADisplay, DefaultScreen(DADisplay));
	DAVisual    = DefaultVisual(DADisplay, DefaultScreen(DADisplay));
	DAGC        = DefaultGC(DADisplay, DefaultScreen(DADisplay));
}


void
DAProposeIconSize(unsigned width, unsigned height)
{
	XIconSize           *iconSizes;
	int nrSizes = 0;

	_daContext->width   = width;
	_daContext->height  = height;

	/* Get the nearest allowed icon size if the WM specifies such */
	iconSizes = XAllocIconSize();
	if (XGetIconSizes(DADisplay, DefaultRootWindow(DADisplay),
			  &iconSizes, &nrSizes)) {
		int i;
		int da = -1;
		int min_w = -1, min_h = -1;
		int max_w = 0, max_h = 0;

		for (i = 0; i < nrSizes; i++) {
			int w1, h1, w, h;

			if ((max_w < iconSizes[i].max_width) ||
			    (max_h < iconSizes[i].max_height)) {
				max_w = iconSizes[i].max_width;
				max_h = iconSizes[i].max_height;
			}

			if ((min_w > iconSizes[i].min_width) ||
			    (min_h > iconSizes[i].min_height) ||
			    (min_w == -1)) {
				min_w = iconSizes[i].min_width;
				min_h = iconSizes[i].min_height;
			}

			if ((width  > iconSizes[i].max_width) ||
			    (width  < iconSizes[i].min_width) ||
			    (height > iconSizes[i].max_height) ||
			    (height < iconSizes[i].min_height))
				continue;

			w1 = (iconSizes[i].max_width  - width) % iconSizes[i].width_inc;
			h1 = (iconSizes[i].max_height - height) % iconSizes[i].height_inc;
			w = MIN(w1, iconSizes[i].width_inc  - w1);
			h = MIN(h1, iconSizes[i].height_inc - h1);

			if ((w * h < da) || (da == -1)) {
				_daContext->width  = width  + w;
				_daContext->height = height + h;
				da = w * h;
			}
		}

		DAPreferredIconSizes.x = min_w;
		DAPreferredIconSizes.y = min_h;
		DAPreferredIconSizes.width  = max_w;
		DAPreferredIconSizes.height = max_h;

		if (da == -1) /* requested size is out of bounds */
			DAWarning("Requested icon-size (%d x %d) is out of the range "
				  "allowed by the window manager\n",
				  _daContext->width, _daContext->height);
	}
	XFree(iconSizes);
}


void
DACreateIcon(char *name, unsigned width, unsigned height, int argc, char **argv)
{
	XClassHint          *classHint;
	XWMHints            *wmHints;
	XTextProperty        window_name;
	XGCValues gcv;
	unsigned long valueMask;
	char                *resourceValue;
	char                 class[100];

	if (!_daContext)
		_daContext = DAContextInit(argc, argv);

	_daContext->width  = width;
	_daContext->height = height;

	/* Create Windows */
	DALeader = XCreateSimpleWindow(DADisplay, DefaultRootWindow(DADisplay),
				       0, 0, width, height, 0, 0, 0);

	if (!_daContext->windowed) {
		DAIcon = XCreateSimpleWindow(DADisplay, DefaultRootWindow(DADisplay),
					     0, 0, width, height, 0, 0, 0);
		DAWindow = DAIcon;
	} else {
		DAIcon = None;
		DAWindow = DALeader;
	}

	/* Set ClassHint */
	classHint = XAllocClassHint();
	if (!classHint)
		printf("%s: can't allocate memory for class hints!\n",
		       _daContext->programName), exit(1);
	if (!_daContext->windowed) {
		classHint->res_class = RES_CLASSNAME;
	} else {
		snprintf(class, 100, "%c%s", toupper(name[0]), name + 1);
		classHint->res_class = class;
	}
	classHint->res_name = name;
	XSetClassHint(DADisplay, DALeader, classHint);
	XFree(classHint);

	/* Set WMName */
	if (XStringListToTextProperty(&name, 1, &window_name) == 0)
		printf("%s: can't allocate window name.\n",
			_daContext->programName), exit(1);
	XSetWMName(DADisplay, DALeader, &window_name);

	/* Set WMHints */
	wmHints = XAllocWMHints();
	if (!wmHints)
		printf("%s: can't allocate memory for wm hints!\n",
		       _daContext->programName), exit(1);

	wmHints->flags = WindowGroupHint;
	wmHints->window_group = DALeader;

	if (!_daContext->windowed) {
		wmHints->flags |= IconWindowHint|StateHint;
		wmHints->icon_window = DAIcon;
		wmHints->initial_state = WithdrawnState;
	}

	XSetWMHints(DADisplay, DALeader, wmHints);
	XFree(wmHints);

	/* Set WMProtocols */
	WM_DELETE_WINDOW = XInternAtom(DADisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(DADisplay, DALeader, &WM_DELETE_WINDOW, 1);

	/* Set Command to start the app so it can be docked properly */
	XSetCommand(DADisplay, DALeader, argv, argc);

	gcv.graphics_exposures = False;
	valueMask = GCGraphicsExposures;

	/* continue setting the foreground GC */
	resourceValue = XGetDefault(DADisplay, RES_CLASSNAME, "foreground");
	if (resourceValue) {
		gcv.foreground = DAGetColor(resourceValue);
		valueMask |= GCForeground;
	}

	XChangeGC(DADisplay, DAGC, valueMask, &gcv);

	/* set background GC values before setting value for foreground */
	resourceValue = XGetDefault(DADisplay, RES_CLASSNAME, "background");
	if (resourceValue)
		gcv.foreground = DAGetColor(resourceValue);

	DAClearGC = XCreateGC(DADisplay, DAWindow,
			      GCGraphicsExposures|GCForeground, &gcv);

	XFlush(DADisplay);
}


void
DAShow(void)
{
	DAShowWindow(DALeader);
}


void
DAShowWindow(Window window)

{
	XMapRaised(DADisplay, window);
	if ((window == DALeader) && !_daContext->windowed)
		XMapSubwindows(DADisplay, DAIcon);
	else
		XMapSubwindows(DADisplay, window);

	XFlush(DADisplay);
}


/* Deprecated */
void
DAInitialize(char *display, char *name, unsigned width, unsigned height,
	     int argc, char **argv)
{
	DAOpenDisplay(display, argc, argv);
	DACreateIcon(name, width, height, argc, argv);
}
