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
 * 
 * $Id: dockapp.h,v 1.1.1.1 2004/02/27 02:53:10 john Exp $
 */

#ifndef _DOCKAPP_H_
#define _DOCKAPP_H_

/*
 * This is a simple (trivial) library for writing Window Maker dock
 * applications, or dockapps (those that only show up in the dock), easily.
 *
 * It is very limited and can be only used for dockapps that open a single
 * appicon for process in only ibe single display, but this seems to be
 * enough for most, if not all, dockapps.
 */

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <stdlib.h>
#include <stdio.h>



/*
 * the callbacks for events related to the dockapp window your program wants
 * to handle
 */
typedef struct {
    /* the dockapp window was destroyed */
    void (*destroy)(void);
    /* button pressed */
    void (*buttonPress)(int button, int state, int x, int y);
    /* button released */
    void (*buttonRelease)(int button, int state, int x, int y);
    /* pointer motion */
    void (*motion)(int x, int y);
    /* pointer entered dockapp window */
    void (*enter)(void);
    /* pointer left dockapp window */
    void (*leave)(void);
    /* timer expired */
    void (*timeout)(void);
} DACallbacks;


typedef struct {
    char	*shortForm;	/* short form for option, like -w	*/
    char	*longForm;	/* long form for option, like --withdrawn */
    char	*description;	/* description for the option		*/
    
    short	type;		/* type of argument			*/
    Bool	used;		/* if the argument was passed on the cmd-line */
    
    /* the following are only set if the "used" field is True		*/
    union {
	void	*ptr;		/* a ptr for the value that was passed	*/
	int	*integer;	/* on the command line			*/
	char	**string;
    } value;
} DAProgramOption;

typedef struct {
    int		x, y;
    int		width, height;
} DARect;

typedef void DARectCallback(int x, int y, DARect rect, void *data);

typedef struct {
    DARect		rect;
    DARectCallback	*action;
} DAActionRect;


/* option argument types */
enum {
    DONone,			/* simple on/off flag			*/
    DOInteger,			/* an integer number			*/
    DOString,			/* a string				*/
    DONatural			/* positive integer number		*/
};


extern Display	*DADisplay;
extern Window	DAWindow;
extern int	DADepth;
extern Visual	*DAVisual;
extern GC	DAGC;
extern DARect	DANoRect;


/*
 * DAParseArguments-
 *	Command line arguments parser. The program is exited if there are
 * syntax errors.
 *
 * -h, --help and --version are automatically handled (causing the program
 * to exit)
 * -w is handled automatically as well and causes the dockapp to be run
 * in windowed mode.
 */
void DAParseArguments(int argc, char **argv, DAProgramOption *options,
	int count, char *programDescription, char *versionDescription);

/*
 * DAInitialize-
 *	Initialize the dockapp, open a connection to the X Server,
 * create the needed windows and set them up to become an appicon  window.
 * It will automatically detect if Window Maker is present and use
 * an appropriate window form.
 *
 * You must call this always before calling anything else (except for
 * DAParseArguments())
 *
 * Arguments:
 *	display		- the name of the display to connect to.
 *			Use "" to use the default value.
 *	name		- the name of your dockapp, used as the instance name
 *			for the WM_CLASS hint. Like wmyaclock.
 *			The ClassName is set to "DockApp" on default.
 *	width, height	- the size of the dockapp window. 48x48 is the
 *			preferred size.
 *	argc, argc	- the program arguments. argv[0] will be used
 *			as the instance name for the WM_CLASS hint.
 */

#define DAInitialize(display, name, width, height, argc, argv) \
	    DAGetDisplay(display, argc, argv); \
	    DACreateIcon(name, width, height, argc, argv);


void DAGetDisplay(char *display, int argc, char **argv);

void DACreateIcon(char *name, unsigned width, unsigned height,
	int argc, char **argv);

void DAProposeIconSize(unsigned width, unsigned height);



/*
 * DASetShapeWithOffset-
 *	Sets the shape mask of the dockapp to the specified one. This is
 * optional. If you pass None as shapeMask, the dockapp will become
 * non-shaped.
 *
 * This is only needed if you want the dockapp to be shaped.
 */
#define DASetShape(shapeMask)	(DASetShapeWithOffset((shapeMask), 0, 0))
void DASetShapeWithOffset(Pixmap shapeMask, int x_ofs, int y_ofs);

/*
 * DASetPixmap-
 *	Sets the image pixmap for the dockapp. Once you set the image with it,
 * you don't need to handle expose events.
 */
void DASetPixmap(Pixmap pixmap);

/*
 * DAMakePixmap-
 *	Creates a pixmap suitable for use with DASetPixmap()
 */
Pixmap DAMakePixmap(void);

/*
 * DAMakePixmapFromData-
 *	Creates a pixmap and mask from XPM data
 */
Bool DAMakePixmapFromData(char **data, Pixmap *pixmap, Pixmap *mask,
	unsigned *width, unsigned *height);

/*
 * DAGetColor-
 *	Returns a color.
 */
unsigned long DAGetColor(char *colorName);

/*
 * DAShow-
 *	Opens the dockapp.
 *
 * Always call this function or the dockapp won't show up.
 */
void DAShow(void);

/*
 * DASetCallbacks-
 *	Register a set of callbacks for events like mouse clicks.
 *
 * Only needed if you want to receive some event.
 */
void DASetCallbacks(DACallbacks *callbacks);

/*
 * DASetTimeout-
 *	Sets a timeout for the DAEventLoop(). The timeout callback
 * will be called whenever the app doesn't get any events from the
 * X server in the specified time.
 */
void DASetTimeout(int miliseconds);

/*
 * DANextEventOrTimeout-
 *	Waits until a event is received or the timeout limit has
 * expired. Returns True if an event was received.
 */
Bool DANextEventOrTimeout(XEvent *event, unsigned long miliseconds);

/*
 * DAProcessEvent-
 *	Processes an event. Returns True if the event was handled and
 * False otherwise.
 *
 * Must be called from your event loop, unless you use DAEventLoop()
 */
Bool DAProcessEvent(XEvent *event);

/*
 * DAEventLoop-
 *	Enters an event loop where events are processed until the dockapp
 * is closed. This function never returns.
 */
void DAEventLoop(void);

/*
 * DAProcessActionRects-
 * 	Processes the current coordinates for the functions in the array of
 * action rectangles, converting coordinates to relative coordinates in
 * the rectangles. The last item must be NULL.
 */
void DAProcessActionRects(int x, int y, DAActionRect *actionrects,
	int count, void *data);

#endif
