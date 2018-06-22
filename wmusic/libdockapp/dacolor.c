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

 * $Id: dacolor.c,v 1.1.1.1 2004/02/27 02:53:10 john Exp $
 */

#include "dockapp.h"

extern char	*progName;

unsigned long
DAGetColor(char *colorName)
{
    XColor color;
    
    if (!XParseColor(DADisplay,
		DefaultColormap(DADisplay, DefaultScreen(DADisplay)),
		colorName, &color))
	printf("%s: could not parse color %s\n", progName, colorName), exit(1);
    
    if (!XAllocColor(DADisplay, DefaultColormap(DADisplay, DefaultScreen(DADisplay)), &color)) {
	printf("%s: could not allocate color %s. Using black\n", progName, colorName);
	return BlackPixel(DADisplay, DefaultScreen(DADisplay));
    }
    
    return color.pixel;
}

