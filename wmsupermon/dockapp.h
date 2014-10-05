/*
 *
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
 */

#ifndef _DOCKAPP_H_
#define _DOCKAPP_H_

/*
 * This is a simple (trivial) library for writing Window Maker dock
 * applications, or dockapps (those that only show up in the dock), easily.
 */

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <stdlib.h>
#include <stdio.h>

extern Display *DADisplay;

/* the callbacks for events related to the dockapp window your program wants
 * to handle */
typedef struct {
    /* the dockapp window was destroyed */
    void (*destroy)(Window win);
    /* button pressed */
    void (*buttonPress)(Window win, int button, int state, int x, int y);
    /* button released */
    void (*buttonRelease)(Window win, int button, int state, int x, int y);
    /* pointer motion */
    void (*motion)(Window win, int x, int y);
    /* pointer entered dockapp window */
    void (*enter)(Window win);
    /* pointer leaved dockapp window */
    void (*leave)(Window win);
    /* timer expired */
    void (*timeout)(Window win);
} DACallbacks;

/* option argument types */
enum {
        DONone,                        /* simple on/off flag */
        DOInteger,                     /* an integer number */
        DOString,                      /* a string */
        DONatural                      /* positive integer number */
};

typedef struct {
    char *shortForm;                   /* short form for option, like -w */
    char *longForm;                    /* long form for option, like --withdrawn */
    char *description;                 /* description for the option */

    short type;                        /* type of argument */

    Bool used;                         /* if the argument was passed in the
                                        cmd line */
    /* the following are only set if the "used" field is True */
    union {                            /* a ptr for the value that was passed
                                        in the command line */
        void *ptr;

        int *integer;

        char **string;
    } value;
} DAProgramOption;

/*
 * DAParseArguments-
 *      Command line argument parser. The program is exited if there are
 * syntax errors.
 *
 * -h, --help and --version are automatically handled (causing the program
 * to exit)
 *
 */
void DAParseArguments(int argc, char **argv, DAProgramOption *options,
                      int count, char *programDescription,
                      char *versionDescription);

/*
 * DAInitialize-
 *      Initialize the dockapp, open a connection to the X server,
 * create the needed windows and setup them to become an appicon window.
 * It will automatically detect if Window Maker is present and use
 * an appropriate form form
 *
 * You must call this always before calling anything else (except for
 * DAParseArguments())
 *
 * Arguments:
 *      display - the name of the display to connect to. Use "" to use the
 *              default value
 *      name - the name of your dockapp, used as the class name for
 *              the WM_CLASS hint. Like WMYAClock
 *      width, height - the size of the dockapp window. 48x48 is the
 *              preferred size
 *      argc, argv - the program arguments. argv[0] will be used as the
 *              instance name for the WM_CLASS hint.
 */
void
DAInitialize(char *display, char *name, unsigned width, unsigned height,
             int argc, char **argv, Window *out);

/*
 * DASetShape-
 *      Sets the shape mask of the dockapp to the specified one. This is
 * optional. If you pass None as shapeMask, the dockapp will become
 * non-shaped.
 *
 * This is only needed if you want the dockapp to be shaped.
 */
void DASetShape(Window *window, Pixmap shapeMask);

/*
 * DASetPixmap-
 *      Sets the image pixmap for the dockapp. Once you set the image with
 * it, you don't need to handle expose events.
 */
void DASetPixmap(Window *window, Pixmap pixmap);

/*
 * DAMakePixmap-
 *      Creates a pixmap suitable for using with DASetPixmap()
 */
Pixmap DAMakePixmap(Window *window);

/*
 * DAMakePixmapFromData-
 *      Creates a pixmap and mask from XPM data
 */
Bool DAMakePixmapFromData(Window *window, char **data, Pixmap *pixmap,
                          Pixmap *mask, unsigned *width, unsigned *height);

/*
 * Returns a color.
 */
unsigned long DAGetColor(char *colorName);
/*
 * DAShow-
 *      Opens the dockapp.
 *
 * Always call this function or the dockapp won't show up.
 */
void DAShow(Window *window);

/*
 * DASetCallbacks-
 *      Register a set of callbacks for events like mouse clicks.
 *
 * Only needed if you want to receive some event.
 */
void DASetCallbacks(Window *window, DACallbacks *callbacks);

/*
 * DASetTimeout-
 *      Sets a timeout for the DAEventLoop(). The timeout callback
 * will be called whenever the app doens't get any events from the
 * X server in the specified time.
 */
void DASetTimeout(int milliseconds);

/*
 * DANextEventOrTimeout-
 *      Waits until an event is received or the timeout limit is
 * expired. Returns True if an event was received.
 */
Bool DANextEventOrTimeout(XEvent *event, unsigned long millisec);

/*
 * DAProcessEvent-
 *      Processes an event. Returns True if the event was handled and
 * False otherwise.
 *
 * Must be called from your event loop, unless you use DAEventLoop()
 */
Bool DAProcessEvent(Window *window, XEvent *event);

/*
 * DAEventLoop-
 *      Enters an event loop where events are processed until the dockapp
 * is closed. This function never returns.
 */
void DAEventLoop(Window *window);

#endif
