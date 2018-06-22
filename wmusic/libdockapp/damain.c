/*
 * Copyright (c) 1999 Alfredo K. Kojima
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

 * $Id: damain.c,v 1.1.1.1 2004/02/27 02:53:10 john Exp $
 */

#include "dockapp.h"


#define MIN(a, b)	(a < b ? a : b)


char		*progName = NULL;
int		d_width, d_height;
int		d_windowed = 0;
int		d_timeout = 0;
DACallbacks	d_callbacks = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};

DARect		DANoRect = {0, 0, 0, 0};
Display		*DADisplay = NULL;
Window		DALeader = None;
Window		DAWindow = None;
int		DADepth = 0;
Visual		*DAVisual = NULL;
GC		DAGC = NULL;
DARect		DAPreferredIconSizes = {-1, -1, 0, 0};


void
DAGetDisplay(char *display, int argc, char **argv)
{
    progName	= argv[0];
    
    /* Open Connection to X Server */
    DADisplay = XOpenDisplay(display);
    if (!DADisplay) {
	printf("%s: could not open display %s!\n", progName,
		XDisplayName(display));
	exit(EXIT_FAILURE);
    }
}


void
DAProposeIconSize(unsigned width, unsigned height)
{
    XIconSize		*iconSizes;
    int			nrSizes = 0;
    
    d_width	= width;
    d_height	= height;
	
    /* Get the nearest allowed icon size if the WM specifies such */
    iconSizes = XAllocIconSize();
    if (XGetIconSizes(DADisplay, DefaultRootWindow(DADisplay), 
		&iconSizes, &nrSizes)) {
	int i;
	int da = -1;
	int min_w = -1, min_h = -1;
	int max_w = 0, max_h = 0;

	for (i = 0; i < nrSizes; i++)  {
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
	    
	    w1 = (iconSizes[i].max_width  - width ) % iconSizes[i].width_inc;
	    h1 = (iconSizes[i].max_height - height) % iconSizes[i].height_inc;
	    w = MIN(w1, iconSizes[i].width_inc  - w1);
	    h = MIN(h1, iconSizes[i].height_inc - h1);

    	    if ((w * h < da) || (da == -1)) {
		d_width  = width  + w;
		d_height = height + h;
		da = w * h;
	    }
	}

	DAPreferredIconSizes.x = min_w;
	DAPreferredIconSizes.y = min_h;
	DAPreferredIconSizes.width  = max_w;
	DAPreferredIconSizes.height = max_h;

	if (da == -1) {	/* requested size is out of bounds */
	    printf("Warning (%s): requested icon-size (%d x %d) is out of the range "
		    "allowed by the window manager\n",
		    progName, d_width, d_height);
	}
    }
    XFree(iconSizes);
}


void
DACreateIcon(char *name, unsigned width, unsigned height, int argc, char **argv)
{
    XClassHint		*classHint;
    XWMHints		*wmHints;
    XGCValues		gcv;


    d_width  = width;
    d_height = height;
    
    /* Create Windows */
    DAWindow = XCreateSimpleWindow(DADisplay, DefaultRootWindow(DADisplay),
	    0, 0, width, height, 0, 0, 0);
    DALeader = XCreateSimpleWindow(DADisplay, DefaultRootWindow(DADisplay),
	    0, 0, 1, 1, 0, 0, 0);
    
    /* Set ClassHint */
    if (!(classHint = XAllocClassHint()))
	printf("%s: can't allocate memory for class hints!\n", progName), exit(1);
    classHint->res_class = "DockApp";
    classHint->res_name = name;
    
    XSetClassHint(DADisplay, DALeader, classHint);
    XFree(classHint);
    
    /* Set WMHints */
    if (!(wmHints = XAllocWMHints()))
	printf("%s: can't allocate memory for wm hints!\n", progName), exit(1);
    
    wmHints->flags = IconWindowHint|WindowGroupHint;
    
    if (!d_windowed) {
    	wmHints->flags |= StateHint;
    	wmHints->initial_state = WithdrawnState;
    }
    
    //Hadess
    {
	    Status status;
	    XTextProperty title;

	    status = XStringListToTextProperty(&progName, 1, &title);
	    XSetWMName(DADisplay, DALeader, &title);
	    XSetWMName(DADisplay, DAWindow, &title);
       //     XStoreName(DADisplay, DALeader, progName);
	 //   XStoreName(DADisplay, DAWindow, progName);
    }
    wmHints->window_group = DALeader;
    wmHints->icon_window = DAWindow;
    
    XSetWMHints(DADisplay, DALeader, wmHints);
    
    /* Set Command to start the app so it can be docked properly */
    XSetCommand(DADisplay, DALeader, argv, argc);
    
    DADepth	= DefaultDepth(DADisplay, DefaultScreen(DADisplay));
    DAVisual	= DefaultVisual(DADisplay, DefaultScreen(DADisplay));
    DAGC	= DefaultGC(DADisplay, DefaultScreen(DADisplay));
    gcv.graphics_exposures = False;
    XChangeGC(DADisplay, DAGC, GCGraphicsExposures, &gcv);
    
    XFlush(DADisplay);
}


void
DAShow(void)
{
    if (!d_windowed)
	XMapRaised(DADisplay, DALeader);
    else
	XMapRaised(DADisplay, DAWindow);
    
    XFlush(DADisplay);
}


static DAProgramOption defaultOptions[] = {
    {"-h", "--help", "shows this help text and exit", DONone, False, {NULL}},
    {"-v", "--version",  "shows program version and exit", DONone, False, {NULL}},
    {"-w", "--windowed", "runs the application in windowed mode", DONone, False, {NULL}}
};


static void
printHelp(char *prog, char *description, DAProgramOption *options, int count)
{
    int j;
    
    printf("Usage: %s [OPTIONS]\n", prog);
    if (description)
	puts(description);
    
    for (j = 0; j < count + 3; j++) {
	char blank[30];
	int c;
	int i;
	
	if (j >= count) {
	    options = defaultOptions;
	    i = j - count;
	}
	else
	    i = j;
	
	if (options[i].shortForm && options[i].longForm)
	    c = printf("  %s, %s", options[i].shortForm, options[i].longForm);
	else if (options[i].shortForm)
	    c = printf("  %s", options[i].shortForm);
	else if (options[i].longForm)
	    c = printf("  %s", options[i].longForm);
	else
	    continue;

	if (options[i].type != DONone) {
	    switch (options[i].type) {
		case DOInteger:
		    c += printf(" <integer>");
		    break;
		case DOString:
		    c += printf(" <string>");
		    break;
		case DONatural:
		    c += printf(" <number>");
		    break;
	    }
	}
	
	memset(blank, ' ', 30);
	if (c > 29)
	    c = 1;
	blank[30-c] = 0;
	printf("%s %s\n", blank, options[i].description);
    }
}


void
DAParseArguments(int argc, char **argv, DAProgramOption *options,
	int count, char *programDescription, char *versionDescription)
{
    int i, j;
    int found = 0;
    
    for (i = 1; i < argc; i++) {
	if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	    printHelp(argv[0], programDescription, options, count), exit(0);
	if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	    puts(versionDescription), exit(0);
	if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--windowed") == 0) {
	    d_windowed = 1;
	    continue;
	}

	found = 0;
	for (j = 0; j < count; j++) {
	    if ((options[j].shortForm && strcmp(options[j].shortForm, argv[i]) == 0) ||
		    (options[j].longForm && strcmp(options[j].longForm, argv[i]) == 0)) {
		found = 1;
		
		options[j].used = True;
		
		if (options[j].type == DONone)
		    break;
		
		i++;
		if (i >= argc)
		    printf("%s: missing argument for option '%s'\n", argv[0], argv[i-1]), exit(1);
		
		switch (options[j].type) {
		    case DOInteger:
			{
			    int integer;
			    
			    if (sscanf(argv[i], "%i", &integer) != 1)
				printf("%s: error parsing argument for option %s\n",
					argv[0], argv[i-1]), exit(1);
					*options[j].value.integer = integer;
			}
			break;
		    case DONatural:
			{
			    int integer;
			    
			    if (sscanf(argv[i], "%i", &integer) != 1)
				printf("%s: error oarsing argument for option %s\n",
					argv[0], argv[i-1]), exit(1);
					if (integer < 0)
					    printf("%s: argument %s must be >= 0\n",
						    argv[0], argv[i-1]), exit(1);
						    *options[j].value.integer = integer;
			}
			break;
		    case DOString:
			*options[j].value.string = argv[i];
			break;
		}
		break;
	    }
	}
	if (!found) {
	    printf("%s: unrecognized option '%s'\n", argv[0], argv[i]);
	    printHelp(argv[0], programDescription, options, count), exit(1);
	}
    }
}

