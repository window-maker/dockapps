#ifndef I_DOCKAPP_H
#define I_DOCKAPP_H
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
    dockapp/dockapp.h - Public interface to the dockapp library

    The dockapp library, distributed as part of the wmget program,
    provides a fairly simple way to write dockapps.  It abstracts away
    the actual drawing, input, and main-loop logic.
*/

#include <X11/X.h>      /* to get Button?Mask, etc. */
#include <sys/poll.h>   /* to get POLLIN, POLLOUT, etc. */


/**********************************************************************
 * BASIC TYPES
 */

/* RV type.  When writing a callback, you should generally return ok or
 * exit, to ask the dockapp to proceed or quit.
 */
typedef enum {
    dockapp_ok = 0,
    dockapp_exit,
    dockapp_invalid_arg,
    dockapp_rv_too_many_clickregions,
} dockapp_rv_t;


/**********************************************************************
 * STARTUP/RUNTIME
 */

/* initializes the dockapp's X display
 */
dockapp_rv_t dockapp_init_gui (
        char *appname,                  /* desired dockapp name */
        char *argv[],                   /* X/dockapp args */
        char **xpmdata);                /* XPM data */


/* the main loop... when this returns, your program should exit
 */
dockapp_rv_t dockapp_run (void);


/* this asks dockapp_run() to periodically call an arbitrary function
 * (you can only have one of these installed at a time).  All sorts of
 * other things can pre-empt a periodic callback, so consider the 'msec'
 * parameter to be an interval floor.
 */
void dockapp_set_periodic_callback (
        long msec,                      /* approx. interval */
        dockapp_rv_t (*cb) (void *),
        void *cbdata);

void dockapp_remove_periodic_callback (void);


/* you can ask dockapp_run() to add an fd to an internal efficient
 * poll-list... see poll(2) for full documentation; the semantics here
 * are that you give fd and pollevents and we create a struct pollfd;
 * when we invoke the callback, the pollstatus arg will contain the
 * revents field from the struct pollfd
 */
dockapp_rv_t dockapp_add_pollfd (
        int fd,                         /* the fd to poll */
        short pollevents,               /* see poll(2), e.g. POLLIN */
        dockapp_rv_t (*cb) (void *, short pollstatus),
        void *cbdata);

dockapp_rv_t dockapp_remove_pollfd (
        int fd);                        /* the fd to remove */



/**********************************************************************
 * CLICKREGIONS
 */

/* a clickregion is simply an area where the user can click to trigger a
 * callback
 * (Helpful list of buttonmask components: ShiftMask, LockMask,
 * ControlMask, Mod1Mask .. Mod5Mask, Button1Mask .. Button5Mask)
 */
dockapp_rv_t dockapp_add_clickregion (
        int x, int y, int w, int h,
        int buttonmask,
        dockapp_rv_t (*cb) (void *, int x, int y),
        void *cbdata);




/**********************************************************************
 * PIXMAP COPY/OVERLAY OPERATIONS
 *  Most dockapp drawing will consist of simple pixel moving from one
 *  part of the pixmap to another.
 */

/* copy pixels, overwriting the target
 */
void dockapp_copy_pixmap (
        int source_x, int source_y,     /* copy from where */
        int target_x, int target_y,     /* copy to where */
        int w, int h);                  /* copy how much */

/* use an XOR raster-op to overlay the source on the target; call this
 * again with the same arguments to undo the first overlay
 */
void dockapp_overlay_pixmap (
        int source_x, int source_y,     /* copy from where */
        int target_x, int target_y,     /* copy to where */
        int w, int h);                  /* copy how much */




/**********************************************************************
 * X SELECTION
 */

/* request the current X selection... only non-incremental strings
 * supported... if there is no selection, callback gets called with
 * (const char *)0; otherwise, it gets called with the string (which
 * will still be owned by the X server, so you need to copy it if you
 * want to keep it)
 */
dockapp_rv_t dockapp_request_selection_string (
        dockapp_rv_t (*cb) (void *, const char *),
        void *cbdata);


#endif /* I_DOCKAPP_H */

