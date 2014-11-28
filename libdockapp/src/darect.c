/*
 * Copyright (c) 2002-2005 Alban Hertroys
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

 * $Id: darect.c,v 1.3 2005/04/17 11:52:32 dalroi Exp $
 */

#include "dockapp.h"

/* TODO: Add a feature to detect the pointer entering/leaving a rectangle */

void
DAProcessActionRects(int x, int y, DAActionRect *actionrects, int count,
	void *data)
{
    int index = 0;

    if (!actionrects)
	return;

    while ( (index < count) &&
	    ((x < actionrects[index].rect.x) ||
	     (x > actionrects[index].rect.x + actionrects[index].rect.width) ||
	     (y < actionrects[index].rect.y) ||
	     (y > actionrects[index].rect.y + actionrects[index].rect.height)))
	index++;

    if (index == count)
	return;

    if (actionrects[index].action)
	(*actionrects[index].action)(x - actionrects[index].rect.x,
				     y - actionrects[index].rect.y,
				     actionrects[index].rect,
				     data);
}

