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

 * $Id: daevent.c,v 1.1.1.1 2004/02/27 02:53:10 john Exp $
 */

#include <sys/time.h>
#include "dockapp.h"

extern Window		DALeader;
extern DACallbacks	d_callbacks;
extern int		d_timeout;

Bool
DAProcessEvent(XEvent *event)
{
    if ((event->xany.window != DAWindow) && (event->xany.window != DALeader))
	return False;
    
    switch (event->type) {
	case DestroyNotify:
	    if (d_callbacks.destroy)
		(*d_callbacks.destroy)();
	    exit(0);
	    break;
	case ButtonPress:
	    if (d_callbacks.buttonPress)
		(*d_callbacks.buttonPress)(event->xbutton.button,
					   event->xbutton.state,
					   event->xbutton.x,
					   event->xbutton.y);
	    break;
	case ButtonRelease:
	    if (d_callbacks.buttonRelease)
		(*d_callbacks.buttonRelease)(event->xbutton.button,
					     event->xbutton.state,
					     event->xbutton.x,
					     event->xbutton.y);
	    break;
	case MotionNotify:
	    if (d_callbacks.motion)
		(*d_callbacks.motion)(event->xmotion.x,
				      event->xmotion.y);
	    break;
	case EnterNotify:
	    if (d_callbacks.enter)
		(*d_callbacks.enter)();
	    break;
	case LeaveNotify:
	    if (d_callbacks.leave)
		(*d_callbacks.leave)();
	    break;
	default:
	    return False;
    }
    
    return True;
}

void
DAEventLoop(void)
{
    XEvent event;

    for (;;) {
	if (d_timeout >= 0) {
	    if (!DANextEventOrTimeout(&event, d_timeout)) {
		if (d_callbacks.timeout)
		    (*d_callbacks.timeout)();
		continue;
	    }
	}
	else
	    XNextEvent(DADisplay, &event);

	DAProcessEvent(&event);
    }
}

Bool
DANextEventOrTimeout(XEvent *event, unsigned long miliseconds)
{
    struct timeval	timeout;
    fd_set		rset;
    
    XSync(DADisplay, False);
    if (XPending(DADisplay)) {
	XNextEvent(DADisplay, event);
	return True;
    }
    
    timeout.tv_sec = miliseconds / 1000;
    timeout.tv_usec = (miliseconds % 1000) * 1000;
    
    FD_ZERO(&rset);
    FD_SET(ConnectionNumber(DADisplay), &rset);
    
    if (select(ConnectionNumber(DADisplay)+1, &rset, NULL, NULL, &timeout) > 0) {
	XNextEvent(DADisplay, event);
	return True;
    }
    
    return False;
}

