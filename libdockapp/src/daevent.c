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

 * $Id: daevent.c,v 1.8 2005/04/17 17:15:33 dalroi Exp $
 */

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "dockapp.h"
#include "daargs.h"
#include "dautil.h"

extern struct DAContext	*_daContext;
extern Atom		WM_DELETE_WINDOW;

Bool
DAProcessEvent(XEvent *event)
{
    if (event->xany.window == DAWindow)
	return DAProcessEventForWindow(DAWindow, event);
    else if (event->xany.window == DALeader)
	/* XXX: Is this superfluous now that DAWindow always references the
	 * dockapp window?
	 */
	return DAProcessEventForWindow(DALeader, event);
    else
	/* XXX: What about handling events for child windows? */
	return False;
}

Bool
DAProcessEventForWindow(Window window, XEvent *event)
{
    if (event->xany.window != window)
	return False;

    switch (event->type) {
	case ClientMessage:
	    if (event->xclient.data.l[0] != WM_DELETE_WINDOW) {
		break;
	    }
	    /* fallthrough */
	case DestroyNotify:
	    if (_daContext->callbacks.destroy)
		(*_daContext->callbacks.destroy)();

	    DAFreeContext();
	    XCloseDisplay(DADisplay);
#ifdef DEBUG
	    debug("%s: DestroyNotify\n", _daContext->programName);
#endif

	    exit(0);
	    break;
	case ButtonPress:
	    if (_daContext->callbacks.buttonPress)
		(*_daContext->callbacks.buttonPress)(event->xbutton.button,
					   event->xbutton.state,
					   event->xbutton.x,
					   event->xbutton.y);
	    break;
	case ButtonRelease:
	    if (_daContext->callbacks.buttonRelease)
		(*_daContext->callbacks.buttonRelease)(event->xbutton.button,
					     event->xbutton.state,
					     event->xbutton.x,
					     event->xbutton.y);
	    break;
	case MotionNotify:
	    if (_daContext->callbacks.motion)
		(*_daContext->callbacks.motion)(event->xmotion.x,
				      event->xmotion.y);
	    break;
	case EnterNotify:
	    if (_daContext->callbacks.enter)
		(*_daContext->callbacks.enter)();
	    break;
	case LeaveNotify:
	    if (_daContext->callbacks.leave)
		(*_daContext->callbacks.leave)();
	    break;
	default:
	    return False;
    }
    
    return True;
}

void
DAEventLoop(void)
{
    DAEventLoopForWindow(DAWindow);
}

void
DAEventLoopForWindow(Window window)
{
    XEvent event;

    for (;;) {
	if (_daContext->timeOut >= 0) {
	    if (!DANextEventOrTimeout(&event, _daContext->timeOut)) {
		if (_daContext->callbacks.timeout)
		    (*_daContext->callbacks.timeout)();
		continue;
	    }
	}
	else
	    XNextEvent(DADisplay, &event);

	DAProcessEventForWindow(window, &event);
    }
}

Bool
DANextEventOrTimeout(XEvent *event, unsigned long milliseconds)
{
    struct timeval	timeout;
    fd_set		rset;
    
    XSync(DADisplay, False);
    if (XPending(DADisplay)) {
	XNextEvent(DADisplay, event);
	return True;
    }
    
    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_usec = (milliseconds % 1000) * 1000;
    
    FD_ZERO(&rset);
    FD_SET(ConnectionNumber(DADisplay), &rset);
    
    if (select(ConnectionNumber(DADisplay)+1, &rset, NULL, NULL, &timeout) > 0) {
	XNextEvent(DADisplay, event);
	return True;
    }
    
    return False;
}

