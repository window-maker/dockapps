/*
    wmget - A background download manager as a Window Maker dock app
    Copyright (c) 2001-2003 Aaron Trickey <aaron@amtrickey.net>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    ********************************************************************
    dockapp/da_x.c - General X11 code

    This file encapsulates all the X11 interface code, such as drawing,
    initialization, and selection handling.
*/

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <unistd.h>

#include "dockapp.h"

#define DOCKAPP_EXPOSE_INTERNALS
#include "da_mouse.h"

#define SELECTION_MAX 1024

static Display *display;

/* We need to update both the ``icon'' and ``main'' windows
 * simultaneously... icon for WindowMaker and main for AfterStep...
 */
static Window icon_win;
static Window main_win;

static Pixmap icon_pixmap;

static GC copy_gc;
static GC xor_gc;


/* request the X selection; when the server responds, we will get it in
 * da_xfd_callback.
 */
static void da_request_selection ()
{
    Atom prop;

    prop = XInternAtom (display, "XSEL_DATA", False);
    XConvertSelection (
            display,
            XA_PRIMARY,     /* only use the PRIMARY selection */
            XA_STRING,      /* we only accept strings (the 90% case) */
            prop,
            icon_win,
            CurrentTime);
}

static dockapp_rv_t (*da_selection_cb) (void *, const char *);
static void *da_selection_cbdata;

static dockapp_rv_t da_receive_selection (XEvent *event)
{
    Atom actual_type;
    int actual_format;
    unsigned long actual_length;
    unsigned long remaining;
    unsigned char *data;
    dockapp_rv_t rv;

    if (event->xselection.selection != XA_PRIMARY)
        return dockapp_ok;

    if (event->xselection.property == None) {
        fprintf (stderr, "! selection can't become string !\n");
        return dockapp_ok;
    }

    XGetWindowProperty (
            event->xselection.display,
            event->xselection.requestor,
            event->xselection.property,
            0,              /* from byte zero... */
            SELECTION_MAX,  /* to this many bytes */
            False,          /* do not delete after retrieval */
            AnyPropertyType,
            &actual_type,   /* actual type returned */
            &actual_format, /* actual format returned */
            &actual_length, /* in 8, 16, or 32 bit units */
            &remaining,     /* unread content (always in 8 bit units!) */
            &data);         /* property content */

    /* We do not support non-string data or incremental transfer. */
    if (actual_type != XA_STRING || actual_format != 8) {
        fprintf (stderr, "! selection unavailable or not of known type !\n");

        if (actual_length) {
            XFree (data);
        }

        return dockapp_ok;
    }


    rv = da_selection_cb (da_selection_cbdata, (const char *)data);

    XFree (data);

    return rv;
}


dockapp_rv_t dockapp_request_selection_string (
        dockapp_rv_t (*cb) (void *, const char *),
        void *cbdata)
{
    da_selection_cb = cb;
    da_selection_cbdata = cbdata;
    da_request_selection ();

    return dockapp_ok;
}


static void da_redraw_icon (void)
{
    XEvent event;

    /* slurp up any queued Expose events */
    while (XCheckTypedWindowEvent (
                display,
                event.xany.window,
                Expose,
                &event))
        ;

    XCopyArea (
            display,
            icon_pixmap,
            icon_win,
            copy_gc,
            0, 0, 64, 64,       /* source coords */
            0, 0);              /* dest coords */

    XCopyArea (
            display,
            icon_pixmap,
            main_win,
            copy_gc,
            0, 0, 64, 64,       /* source coords */
            0, 0);              /* dest coords */
}


/* this is the callback bound to the X server socket descriptor
 */
static dockapp_rv_t da_xfd_callback (
        void *unused_cbdata,
        short unused_revents)
{
    (void)unused_cbdata;
    (void)unused_revents;

    XSync (display, False);

    while (XPending (display)) {

        XEvent event;

        XNextEvent (display, &event);

        switch (event.type) {
            case ButtonPress:
                da_mouse_button_down (
                        event.xbutton.x,
                        event.xbutton.y,
                        event.xbutton.state);
                break;

            case ButtonRelease:
                /* da_mouse_button_up() returns the callback rv of the
                 * callback which is invoked, if any
                 */
                return da_mouse_button_up (
                        event.xbutton.x,
                        event.xbutton.y,
                        event.xbutton.state);
                break;

            case Expose:
                da_redraw_icon ();
                break;

            case DestroyNotify:
                XCloseDisplay (display);
                return dockapp_exit;

            case SelectionNotify:
                return da_receive_selection (&event);
        }
    }

    return dockapp_ok;
}


int da_x_error_handler (Display *display, XErrorEvent *xerr)
{
    char msgbuf[1024];

    XGetErrorText (display, xerr->error_code, msgbuf, sizeof msgbuf);

    fprintf (stderr, "X11 error: %s\n", msgbuf);

    return 0;
}


/* the following code is stolen largely from wmgeneral.c...
 */
static Pixmap da_create_shaping_bitmap (char **xpm)
{
    int     i,j,k;
    int     width, height, numcol, depth;
    unsigned long zero=0;
    unsigned char bwrite;
    int     bcount;
    unsigned long     curpixel;

    char xbm_data[64*64];
    char *xbm = xbm_data;

    sscanf(*xpm, "%d %d %d %d", &width, &height, &numcol, &depth);

    for (k=0; k!=depth; k++)
    {
        zero <<=8;
        zero |= xpm[1][k];
    }

    for (i=numcol+1; i < numcol+64+1; i++) {
        bcount = 0;
        bwrite = 0;
        for (j=0; j<64*depth; j+=depth) {
            bwrite >>= 1;

            curpixel=0;
            for (k=0; k!=depth; k++)
            {
                curpixel <<=8;
                curpixel |= xpm[i][j+k];
            }

            if ( curpixel != zero ) {
                bwrite += 128;
            }
            bcount++;
            if (bcount == 8) {
                *xbm = bwrite;
                xbm++;
                bcount = 0;
                bwrite = 0;
            }
        }
    }

    return XCreateBitmapFromData (
            display,
            icon_win,
            xbm_data,
            64, 64);
}


dockapp_rv_t dockapp_init_gui (
        char *appname,
        char *argv[],
        char **xpmdata)
{
    int screen;
    Window root;
    XpmAttributes xpm_att;
    Pixmap xpm_mask;
    unsigned long black, white;

    (void)argv; /* not currently parsed */

    /* initialize x error handler */
    XSetErrorHandler (da_x_error_handler);

    /* get some stuff out */
    display = XOpenDisplay (0);

    screen = DefaultScreen (display);

    root = RootWindow (display, screen);

    black = BlackPixel (display, screen);
    white = WhitePixel (display, screen);

    /* construct the pixmap */
    xpm_att.valuemask = XpmReturnPixels | XpmReturnExtensions;

    XpmCreatePixmapFromData (
            display,
            root,
            xpmdata,
            &icon_pixmap,
            &xpm_mask,
            &xpm_att);

    /* construct the windows */
    main_win = XCreateSimpleWindow (
            display,
            root,               /* parent */
            0, 0, 64, 64,       /* pos & size */
            1,                  /* border width */
            black,              /* foreground */
            white);             /* background */

    icon_win = XCreateSimpleWindow (
            display,
            main_win,           /* parent */
            0, 0, 64, 64,       /* pos & size */
            1,                  /* border width */
            black,              /* foreground */
            white);             /* background */

    /* request all interesting X events */
    XSelectInput (display, main_win,
            ButtonPressMask | ExposureMask | ButtonReleaseMask |
            PointerMotionMask | StructureNotifyMask);

    XSelectInput (display, icon_win,
            ButtonPressMask | ExposureMask | ButtonReleaseMask |
            PointerMotionMask | StructureNotifyMask);

    /* apply the shaping bitmap */
    XShapeCombineMask (
            display,
            icon_win,
            ShapeBounding,
            0, 0,
            da_create_shaping_bitmap (xpmdata),
            ShapeSet);

    XShapeCombineMask (
            display,
            main_win,
            ShapeBounding,
            0, 0,
            da_create_shaping_bitmap (xpmdata),
            ShapeSet);


    /* set wm hints and title */
    {
        XClassHint classhint;
        XWMHints wmhints;

        classhint.res_name = appname;
        classhint.res_class = appname;
        XSetClassHint (display, main_win, &classhint);

        wmhints.initial_state = WithdrawnState;
        wmhints.icon_window = icon_win;
        wmhints.window_group = main_win;
        wmhints.flags = StateHint | IconWindowHint | WindowGroupHint;
        XSetWMHints (display, main_win, &wmhints);

        XStoreName (display, main_win, appname);
    }

    /* now we need two gc's to do drawing & copying... one which paints
     * by overwriting pixels, and one which paints by xor'ing pixels
     */
    {
        XGCValues gcv;

        gcv.graphics_exposures = 0;
        gcv.function = GXcopy;

        copy_gc = XCreateGC (
                display,
                root,
                GCGraphicsExposures | GCFunction,
                &gcv);

        gcv.function = GXxor;

        xor_gc = XCreateGC (
                display,
                root,
                GCGraphicsExposures | GCFunction,
                &gcv);
    }

    /* finally, show the window */
    XMapWindow (display, main_win);

    /* invoke da_xfd_callback once manually to process all events which
     * were generated by the above work... after this, it will be called
     * by the framework whenever there is activity on the server socket
     */
    da_xfd_callback (0, 0);

    /* now that we're up, add the X file descriptor to the main poll
     * loop and bind it to our event handler
     */
    return dockapp_add_pollfd (
            XConnectionNumber (display),
            POLLIN | POLLPRI,
            da_xfd_callback,
            0);
}


void dockapp_copy_pixmap (
        int source_x, int source_y,
        int target_x, int target_y,
        int w, int h)
{
    XCopyArea (
            display,
            icon_pixmap,
            icon_pixmap,
            copy_gc,
            source_x, source_y,
            w, h,
            target_x, target_y);

    da_redraw_icon ();
}


void dockapp_overlay_pixmap (
        int source_x, int source_y,
        int target_x, int target_y,
        int w, int h)
{
    XCopyArea (
            display,
            icon_pixmap,
            icon_pixmap,
            xor_gc,
            source_x, source_y,
            w, h,
            target_x, target_y);

    da_redraw_icon ();
}













