/*
 * Copyright (c) 2005 Alban G. Hertroys
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
 * $Id: daargs.h,v 1.3 2005/04/17 17:15:33 dalroi Exp $
 */

#include "dockapp.h"

/*
 * Context structure to keep track of globals
 */
struct DAContext {
    int		argc;			/* Raw input data */
    char	**argv;

    int		windowed;
    int		width, height;
    int		timeOut;

    DACallbacks	callbacks;

    char	*programName;		/* shortcut to argv[0] */

    DAProgramOption	**options;	/* Array of option pointers */
    short		optionCount;
};


struct DAContext* DAContextInit();
void DAFreeContext();

