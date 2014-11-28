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

 * $Id: dacallback.c,v 1.4 2005/04/17 11:52:31 dalroi Exp $
 */

#include "dockapp.h"
#include "daargs.h"

extern struct DAContext *_daContext;

void
DASetCallbacks(DACallbacks *callbacks)
{
    long	mask = 0;
    
    _daContext->callbacks = *callbacks;

    if (callbacks->destroy)
	mask |= StructureNotifyMask;
    if (callbacks->buttonPress)
	mask |= ButtonPressMask;
    if (callbacks->buttonRelease)
	mask |= ButtonReleaseMask;
    if (callbacks->motion)
	mask |= PointerMotionMask;
    if (callbacks->enter)
	mask |= EnterWindowMask;
    if (callbacks->leave)
	mask |= LeaveWindowMask;
    
    XSelectInput(DADisplay, DAWindow, mask);
    XFlush(DADisplay);
}

void
DASetTimeout(int milliseconds)
{
    _daContext->timeOut = milliseconds;
}

