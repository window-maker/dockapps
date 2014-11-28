/*
 * Copyright (c) 1999-2005 Alfredo K. Kojima, Alban G. Hertroys
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

#include <string.h>

#include "daargs.h"
#include "dautil.h"

extern struct DAContext *_daContext;

/*
 * Function prototypes
 */
void _message(const char *label, const char *fmt, va_list args);


/*
 * Exported functions.
 */
void
DASetExpectedVersion(unsigned long expectedVersion)
{
    DAExpectedVersion = expectedVersion;

    if (expectedVersion > DA_VERSION)
	DAWarning("Version of libdockapp (%u) is older than "
		"version expected (%u)",
		DA_VERSION,
		DAExpectedVersion);
}


Display*
DAGetDisplay(char *d, ...)
{
    /* Be backward compatible */
    if (DAExpectedVersion < 20030126) {
	va_list ap;
	int argc;
	char **argv;

	va_start(ap, d);
	argc = va_arg(ap, int);
	argv = va_arg(ap, char**);
	va_end(ap);

	DAOpenDisplay(d, argc, argv);

	DAWarning("Expected version of libdockapp is not set.");
	DAWarning("Obsolete call to DAGetDisplay().");

	return NULL;
    }

    return DADisplay;
}


void
DASetDisplay(Display *display)
{
    DADisplay = display;
}


Window
DAGetWindow(void)
{
    return DAWindow;
}


void
DASetWindow(Window window)
{
    DAWindow = window;
}


Window
DAGetLeader(void)
{
    return DALeader;
}


void
DASetLeader(Window leader)
{
    DALeader = leader;
}


Window
DAGetIconWindow(void)
{
    return DAIcon;
}


void
DASetIconWindow(Window icon_win)
{
    DAIcon = icon_win;
}


int
DAGetDepth(void)
{
    return DADepth;
}


void
DASetDepth(int depth)
{
    DADepth = depth;
}


Visual*
DAGetVisual(void)
{
    return DAVisual;
}


void
DASetVisual(Visual *visual)
{
    DAVisual = visual;
}

void
DAWarning(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _message("Warning", fmt, args);
    va_end(args);
}

void
DAError(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    _message("Error", fmt, args);
    exit(1);
    va_end(args);
}


/*
 * Local functions
 */

void
_message(const char *label, const char *fmt, va_list args)
{
    char *w_fmt;

    if (_daContext->programName != NULL) {
	/* put default string in front of message, add newline */
	w_fmt = malloc((strlen(_daContext->programName) + strlen(fmt) +13) * sizeof(char));
	sprintf(w_fmt, "%s: %s: %s\n", _daContext->programName, label, fmt);
    } else {
	w_fmt = malloc((strlen(fmt) +1) * sizeof(char));
	sprintf(w_fmt, "%s\n", fmt);
    }

    /* print the message */
    vfprintf(stderr, w_fmt, args);
}


void
debug(const char *fmt, ...)
{
#ifdef DEBUG
    va_list args;

    va_start(args, fmt);
    _message("debug", fmt, args);
    va_end(args);
#endif
}
