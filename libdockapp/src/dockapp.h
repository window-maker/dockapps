/*
 * Copyright (c) 1999-2005 Alfredo K. Kojima, Alban Hertroys
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
 */

#ifndef _DOCKAPP_H_
#define _DOCKAPP_H_
#define DA_VERSION      20050716

/*
 * This is a simple (trivial) library for writing Window Maker dock
 * applications, or dockapps (those that only show up in the dock), easily.
 *
 * It is very limited and can be only used for dockapps that open a single
 * appicon for process in only ibe single display, but this seems to be
 * enough for most, if not all, dockapps.
 */

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/xpm.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>


/* class-name for X-resources and Window Maker attributes */
#define RES_CLASSNAME "DockApp"


/* !!! DEPRECATED !!!
 * This feature may be removed in the future. Use DAEventCallbacks instead.
 *
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
	char        *shortForm; /* short form for option, like -w	*/
	char        *longForm;  /* long form for option, like --withdrawn */
	char        *description; /* description for the option		*/

	short type;             /* type of argument			*/
	Bool used;              /* if the argument was passed on the cmd-line */

	/* the following are only set if the "used" field is True		*/
	union {
		void    *ptr;   /* a ptr for the value that was passed	*/
		int     *integer; /* on the command line			*/
		char    **string;
	} value;
} DAProgramOption;


typedef XRectangle DARect;

/*
 * A callback function for an event on an "action rectangle"
 */
typedef void DARectCallback(int x, int y, DARect rect, void *data);

/*
 * The action rectangle structure
 */
typedef struct {
	DARect rect;
	DARectCallback      *action;
} DAActionRect;


/* option argument types */
enum {
	DONone,                 /* simple on/off flag			*/
	DOInteger,              /* an integer number			*/
	DOString,               /* a string				*/
	DONatural               /* positive integer number		*/
};


/* Shaped pixmaps: Shapes in combination with pixmaps */
typedef struct {
	Pixmap pixmap;
	Pixmap shape;                   /* needs a 1-bit plane GC (shapeGC). */
	GC drawGC, clearGC, shapeGC;
	DARect geometry;                /* position and size */
	DAActionRect    *triggers;
} DAShapedPixmap;



extern Display  *DADisplay;
extern Window DAWindow, DALeader, DAIcon;   /* see [NOTE] */
extern int DADepth;
extern Visual   *DAVisual;
extern GC DAGC, DAClearGC;
extern DARect DANoRect;
extern unsigned long DAExpectedVersion;

/* [NOTE]
 * DALeader is the group-leader window, DAIcon is the icon window.
 * DAWindow is the one of these two that is visible. Depending on whether the
 * dockapp was started normally or windowed, that will be DAIcon and DALeader
 * respectively.
 * DAIcon is None if the dockapp runs in "windowed mode".
 */


/*
 * Set the version of the library that the dockapp expects.
 * This is a date in the format 'yyyymmdd'. You can find this date
 * in NEWS.
 */
void DASetExpectedVersion(unsigned long expectedVersion);


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

void DAInitialize(char *display, char *name, unsigned width, unsigned height,
		  int argc, char **argv);

void DAOpenDisplay(char *display, int argc, char **argv);

void DACreateIcon(char *name, unsigned width, unsigned height,
		  int argc, char **argv);

void DAProposeIconSize(unsigned width, unsigned height);


/*
 * Wrapper functions for global variables.
 */

/* Get/Set DADisplay value. For use with external code.
 * Call with NULL as only argument. Other arguments are for backward compatibility
 * only.
 * XXX: Argument list is a kludge.
 */
Display *DAGetDisplay(char *d, ...);
void DASetDisplay(Display *display);

/* Get program name (from argv[0]). Returns a reference. */
char *DAGetProgramName();

/* Get/Set DAWindow and DALeader values respectively. For use with external code. */
Window DAGetWindow(void);
void DASetWindow(Window window);

Window DAGetLeader(void);
void DASetLeader(Window leader);

Window DAGetIconWindow(void);
void DASetIconWindow(Window icon_win);

/* Get/Set DADepth; the display depth. For use with external code. */
int DAGetDepth(void);
void DASetDepth(int depth);

/* Get/Set DAVisual; the visual type for the screen. For use with external code. */
Visual *DAGetVisual(void);
void DASetVisual(Visual *visual);


/*
 * DASetShapeWithOffset-
 *	Sets the shape mask of the dockapp to the specified one. This is
 * optional. If you pass None as shapeMask, the dockapp will become
 * non-shaped.
 *
 * This is only needed if you want the dockapp to be shaped.
 */
#define DASetShape(shapeMask) \
	(DASetShapeWithOffset((shapeMask), 0, 0))
#define DASetShapeForWindow(window, shapeMask) \
	(DASetShapeWithOffsetForWindow((window), (shapeMask), 0, 0))

void DASetShapeWithOffset(Pixmap shapeMask, int x_ofs, int y_ofs);
void DASetShapeWithOffsetForWindow(Window window, Pixmap shapeMask,
				   int x_ofs, int y_ofs);
/*
 * DASetPixmap-
 *	Sets the image pixmap for the dockapp. Once you set the image with it,
 * you don't need to handle expose events.
 */
void DASetPixmap(Pixmap pixmap);
void DASetPixmapForWindow(Window window, Pixmap pixmap);

/*
 * DAMakePixmap-
 *	Creates a pixmap suitable for use with DASetPixmap()
 */
Pixmap DAMakePixmap(void);

/*
 * DAMakeShape-
 *	Creates a shape pixmap suitable for use with DASetShape()
 */
Pixmap DAMakeShape(void);

/*
 * DAMakeShapeFromData-
 *	Creates a shape pixmap suitable for use with DASetShape() from XBM data.
 */
Pixmap DAMakeShapeFromData(char *data, unsigned int width, unsigned int height);

/*
 * DAMakeShapeFromFile-
 *	Creates a shape pixmap suitable for use with DASetShape() from an
 *	XBM file.
 */
Pixmap DAMakeShapeFromFile(char *filename);

/*
 * DAMakePixmapFromData-
 *	Creates a pixmap and mask from XPM data
 *	Returns true on success, false on failure.
 */
Bool DAMakePixmapFromData(char **data, Pixmap *pixmap, Pixmap *mask,
			  unsigned short *width, unsigned short *height);

/*
 * DAMakePixmapFromFile-
 *	Creates a pixmap and mask from an XPM file
 *	Returns true on success, false on failure.
 */
Bool DAMakePixmapFromFile(char *filename, Pixmap *pixmap, Pixmap *mask,
			  unsigned short *width, unsigned short *height);

/*
 * DAMakeShapedPixmap-
 *	Creates a shaped pixmap with width & height of dockapp window.
 */
DAShapedPixmap *DAMakeShapedPixmap();

/*
 * DAMakeShapedPixmapFromData-
 *      Creates a shaped pixmap from XPM-data.
 *      Returns shaped pixmap on success, NULL on failure.
 */
DAShapedPixmap *DAMakeShapedPixmapFromData(char **data);

/*
 * DAMakeShapedPixmapFromFile-
 *      Creates a shaped pixmap from an XPM file.
 *      Returns shaped pixmap on success, NULL on failure.
 */
DAShapedPixmap *DAMakeShapedPixmapFromFile(char *filename);

/*
 * DAFreeShapedPixmap-
 *      Free memory reserved for a ShapedPixmap
 */
void DAFreeShapedPixmap(DAShapedPixmap *dasp);

/*
 * DASPCopyArea-
 *      Copies shape-mask and pixmap-data from an area in one shaped pixmap
 * into another shaped pixmap.
 */
void DASPCopyArea(DAShapedPixmap *src, DAShapedPixmap *dst,
		  int x1, int y1, int w, int h, int x2, int y2);

/*
 * DASPClear-
 *      Clears a shaped pixmaps contents.
 */
void DASPClear(DAShapedPixmap *dasp);

/* DASPShow-
 *      Displays the pixmap in the dockapp-window.
 */
void DASPSetPixmap(DAShapedPixmap *dasp);
void DASPSetPixmapForWindow(Window window, DAShapedPixmap *dasp);

/*
 * DAGetColor-
 *	Returns an X color index.
 */
unsigned long DAGetColor(char *colorName);

/*
 * DAShow-
 *	Displays the dockapp.
 *
 * Always call this function or the dockapp won't show up.
 */
void DAShow(void);

/*
 * DAShowWindow-
 *      Display a window. Also displays subwindows if it is the dockapp leader
 *      window (DALeader).
 */
void DAShowWindow(Window window);

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
void DASetTimeout(int milliseconds);

/*
 * DANextEventOrTimeout-
 *	Waits until a event is received or the timeout limit has
 * expired. Returns True if an event was received.
 */
Bool DANextEventOrTimeout(XEvent *event, unsigned long milliseconds);

/*
 * DAProcessEvent-
 *	Processes an event. Returns True if the event was handled and
 * False otherwise.
 *
 * Must be called from your event loop, unless you use DAEventLoop()
 */
Bool DAProcessEvent(XEvent *event);
Bool DAProcessEventForWindow(Window window, XEvent *event);

/*
 * DAEventLoop-
 *	Enters an event loop where events are processed until the dockapp
 * is closed. This function never returns.
 */
void DAEventLoop(void);
void DAEventLoopForWindow(Window window);

/*
 * DAProcessActionRects-
 *      Processes the current coordinates with one of the functions in
 * the array of action rectangles. Coordinates are converted to relative
 * coordinates in one of the rectangles. The last item in the array of
 * action rectangles must be NULL.
 */
void DAProcessActionRects(int x, int y, DAActionRect *actionrects,
			  int count, void *data);


/*
 * Error handling functions
 */

/* Print a warning to stderr and continue */
void DAWarning(const char *fmt, ...);

/* Print an error to stderr and exit with 1 */
void DAError(const char *fmt, ...);

#endif
