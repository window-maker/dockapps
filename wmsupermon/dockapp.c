/*
 * 2006 - changes by Sergei Golubchik
 *   + set window title, better wm hints
 *   + multi-window support
 */

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
 */

#include "dockapp.h"

#include <string.h>
#include <X11/extensions/shape.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


static char *progName = NULL;
static unsigned d_width, d_height;
static DACallbacks d_callbacks = {NULL, NULL, NULL, NULL, NULL, NULL};
static int d_iswmaker = 0;
static int d_timeout = 0;

Display *DADisplay = NULL;

static unsigned char*
PropGetCheckProperty(Display *dpy, Window window, Atom hint, Atom type,
                     int format, int count, int *retCount)
{
    Atom type_ret;
    int fmt_ret;
    unsigned long nitems_ret;
    unsigned long bytes_after_ret;
    unsigned char *data;
    int tmp;

    if (count <= 0)
        tmp = 0xffffff;
    else
        tmp = count;

    if (XGetWindowProperty(dpy, window, hint, 0, tmp, False, type,
                           &type_ret, &fmt_ret, &nitems_ret, &bytes_after_ret,
                           (unsigned char **)&data)!=Success || !data)
        return NULL;

    if ((type!=AnyPropertyType && type!=type_ret)
        || (count > 0 && nitems_ret != count)
        || (format != 0 && format != fmt_ret)) {
        XFree(data);
        return NULL;
    }

    if (retCount)
        *retCount = nitems_ret;

    return data;
}


static Bool
iswmaker(Display *dpy)
{
    Atom *data;
    Atom atom;
    Atom noticeboard;
    int i, count;

    atom = XInternAtom(dpy, "_WINDOWMAKER_WM_PROTOCOLS", False);
    noticeboard = XInternAtom(dpy, "_WINDOWMAKER_NOTICEBOARD", False);

    data = (Atom*)PropGetCheckProperty(dpy, DefaultRootWindow(dpy), atom,
                                       XA_ATOM, 32, -1, &count);

    if (!data)
        return False;

    for (i = 0; i < count; i++) {
        if (data[i] == noticeboard) {
            Window *win;
            void *d;

            XFree(data);

            win = (Window*)PropGetCheckProperty(dpy, DefaultRootWindow(dpy),
                                                noticeboard, XA_WINDOW, 32, -1,
                                                &count);

            if (!win) {
                return False;
            }

            d = PropGetCheckProperty(dpy, *win, noticeboard, XA_WINDOW, 32, 1,
                                     NULL);
            if (d) {
                XFree(d);

                return True;
            }
            return False;
        }
    }

    XFree(data);

    /* not 100% sure */
    return True;
}



void
DAInitialize(char *display, char *name, unsigned width, unsigned height,
             int argc, char **argv, Window *out)
{
    XClassHint *chint;
    XWMHints *hints;
    XTextProperty wname;
    Window DAWindow, DALeader;

    d_width = width;
    d_height = height;

    progName = argv[0];

    if (!DADisplay)
        DADisplay = XOpenDisplay(display);
    if (!DADisplay) {
        printf("%s: could not open display %s!\n", progName,
               XDisplayName(display));

        exit(1);
    }

    d_iswmaker = iswmaker(DADisplay);

    DAWindow = XCreateSimpleWindow(DADisplay, DefaultRootWindow(DADisplay),
                                   0, 0, width, height, 0, 0, 0);
    DALeader = XCreateSimpleWindow(DADisplay, DefaultRootWindow(DADisplay),
                                   0, 0, 1, 1, 0, 0, 0);

    chint = XAllocClassHint();
    if (!chint) {
        printf("%s: cant allocate memory for class hints!\n", progName);
        exit(1);
    }
    chint->res_class = name;
    chint->res_name = strrchr(argv[0], '/');
    if (!chint->res_name)
        chint->res_name = argv[0];
    else
        chint->res_name++;

    XSetClassHint(DADisplay, DAWindow, chint);
    XSetClassHint(DADisplay, DALeader, chint);
    XFree(chint);

    hints = XAllocWMHints();
    if (!hints) {
        printf("%s: cant allocate memory for hints!\n", progName);
        exit(1);
    }
    hints->flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
    hints->initial_state = WithdrawnState;
    hints->window_group = DALeader;
    hints->icon_window = DAWindow;

    XSetWMHints(DADisplay, DALeader, hints);
    XSetWMHints(DADisplay, DAWindow, hints);

    XSetCommand(DADisplay, DALeader, argv, argc);
    XSetCommand(DADisplay, DAWindow, argv, argc);

    if (XStringListToTextProperty(&name, 1, &wname) == 0) {
      fprintf(stderr, "%s: can't allocate window name\n", name);
      exit(1);
    }

    XSetWMName(DADisplay, DALeader, &wname);
    XSetWMName(DADisplay, DAWindow, &wname);
    XFlush(DADisplay);

    *out++=DAWindow;
    *out++=DALeader;
}


void
DASetShape(Window *window, Pixmap shapeMask)
{
    XShapeCombineMask(DADisplay, *window, ShapeBounding, 0, 0, shapeMask,
                      ShapeSet);
    XFlush(DADisplay);
}


void
DASetPixmap(Window *window, Pixmap pixmap)
{
    XSetWindowBackgroundPixmap(DADisplay, *window, pixmap);
    XClearWindow(DADisplay, *window);
    XFlush(DADisplay);
}


Pixmap
DAMakePixmap(Window *window)
{
    Pixmap p;

    p = XCreatePixmap(DADisplay, *window, d_width, d_height,
                      DefaultDepth(DADisplay, DefaultScreen(DADisplay)));

    return p;
}



Bool
DAMakePixmapFromData(Window *window, char **data, Pixmap *pixmap, Pixmap *mask,
                     unsigned *width, unsigned *height)
{
    Pixmap unused;
    if (!mask)
      mask=&unused;

    XpmAttributes xpmat;

    xpmat.valuemask = XpmCloseness;
    xpmat.closeness = 40000;

    if (XpmCreatePixmapFromData(DADisplay, *window, data, pixmap, mask,
                                &xpmat)!=0) {
        return False;
    }

    *width = xpmat.width;
    *height = xpmat.height;

    return True;
}


void
DAShow(Window *window)
{
    XMapRaised(DADisplay, window[d_iswmaker]);

    XFlush(DADisplay);
}


void
DASetCallbacks(Window *window, DACallbacks *callbacks)
{
    long mask = 0;

    d_callbacks = *callbacks;

    if (callbacks->buttonPress)
        mask |= ButtonPressMask;

    if (callbacks->buttonRelease)
        mask |= ButtonReleaseMask;

    XSelectInput(DADisplay, *window, mask);
    XFlush(DADisplay);
}


Bool
DAProcessEvent(Window *window, XEvent *event)
{
    if (event->xany.window != window[0] && event->xany.window != window[1])
        return False;

    switch (event->type) {
     case DestroyNotify:
        if (d_callbacks.destroy) {
            (*d_callbacks.destroy)(window[0]);
        }
        exit(0);
        break;

     case ButtonPress:
        if (d_callbacks.buttonPress) {
            (*d_callbacks.buttonPress)(window[0], event->xbutton.button, event->xbutton.state,
                                       event->xbutton.x, event->xbutton.y);
        }
        break;

     case ButtonRelease:
        if (d_callbacks.buttonRelease) {
            (*d_callbacks.buttonRelease)(window[0], event->xbutton.button, event->xbutton.state,
                                         event->xbutton.x, event->xbutton.y);
        }
        break;

     case MotionNotify:
        if (d_callbacks.motion) {
            (*d_callbacks.motion)(window[0], event->xbutton.x, event->xbutton.y);
        }
        break;

     case EnterNotify:
        if (d_callbacks.enter) {
            (*d_callbacks.enter)(window[0]);
        }
        break;

     case LeaveNotify:
        if (d_callbacks.leave) {
            (*d_callbacks.leave)(window[0]);
        }
        break;

     default:
        return False;
        break;
    }

    return True;
}


void
DAEventLoop(Window *window)
{
    XEvent ev;

    for (;;) {
        if (d_timeout >= 0) {
            if (!DANextEventOrTimeout(&ev, d_timeout)) {
                if (d_callbacks.timeout)
                    (*d_callbacks.timeout)(window[0]);
                continue;
            }
        } else {
            XNextEvent(DADisplay, &ev);
        }
        DAProcessEvent(window, &ev);
    }
}


static DAProgramOption defaultOptions[]= {
    {"-h", "--help", "shows this help text and exit", DONone, False,
        {NULL}},
    {"-v", "--version", "shows program version and exit", DONone, False,
        {NULL}}
};


static void
printHelp(char *prog, char *description, DAProgramOption *options,
          int count)
{
    int j;

    printf("Usage: %s [OPTIONS]\n", prog);
    if (description)
        puts(description);

    for (j = 0; j < count + 2; j++) {
        char blank[35];
        int c;
        int i;

        if (j >= count) {
            options = defaultOptions;
            i = j - count;
        } else {
            i = j;
        }

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
                c+= printf(" <number>");
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
        if (strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) {

            printHelp(argv[0], programDescription, options, count);
            exit(0);

        } else if (strcmp(argv[i],"-v")==0 || strcmp(argv[i], "--version")==0) {

            puts(versionDescription);
            exit(0);

        }

        found = 0;
        for (j = 0; j < count; j++) {
            if ((options[j].shortForm
                && strcmp(options[j].shortForm, argv[i])==0)
                ||
                (options[j].longForm
                && strcmp(options[j].longForm, argv[i])==0)) {

                found = 1;

                options[j].used = True;

                if (options[j].type == DONone)
                    break;

                i++;
                if (i >= argc) {
                    printf("%s: missing argument for option '%s'\n", argv[0],
                           argv[i-1]);
                    exit(1);
                }

                switch (options[j].type) {
                 case DOInteger:
                    {
                        int integer;

                        if (sscanf(argv[i], "%i", &integer)!=1) {
                            printf("%s: error parsing argument for option %s\n",
                                   argv[0], argv[i-1]);
                            exit(1);
                        }
                        *options[j].value.integer = integer;
                    }
                    break;
                 case DONatural:
                    {
                        int integer;

                        if (sscanf(argv[i], "%i", &integer)!=1) {
                            printf("%s: error parsing argument for option %s\n",
                                   argv[0], argv[i-1]);
                            exit(1);
                        }
                        if (integer < 0) {
                            printf("%s: argument %s must be >= 0\n",
                                   argv[0], argv[i-1]);
                            exit(1);
                        }
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
            printHelp(argv[0], programDescription, options, count);
            exit(1);
        }
    }
}


unsigned long
DAGetColor(char *colorName)
{
    XColor color;

    if (!XParseColor(DADisplay,
                     DefaultColormap(DADisplay, DefaultScreen(DADisplay)),
                     colorName, &color)) {
        printf("%s: could not parse color %s\n", progName, colorName);
        exit(1);
    }

    if (!XAllocColor(DADisplay, DefaultColormap(DADisplay, DefaultScreen(DADisplay)),
                     &color)) {
        printf("%s: could not allocate color %s. Using black\n", progName, colorName);
        return BlackPixel(DADisplay, DefaultScreen(DADisplay));
    }

    return color.pixel;
}


void
DASetTimeout(int milliseconds)
{
    d_timeout = milliseconds;
}



Bool
DANextEventOrTimeout(XEvent *event, unsigned long millisec)
{
    struct timeval timeout;
    fd_set rset;

    XSync(DADisplay, False);
    if (XPending(DADisplay)) {
        XNextEvent(DADisplay, event);
        return True;
    }

    timeout.tv_sec = millisec/1000;
    timeout.tv_usec = (millisec%1000)*10;

    FD_ZERO(&rset);
    FD_SET(ConnectionNumber(DADisplay), &rset);

    if (select(ConnectionNumber(DADisplay)+1, &rset, NULL, NULL,
               &timeout) > 0) {
        XNextEvent(DADisplay, event);
        return True;
    }
    return False;
}

