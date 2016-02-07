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
    usage.c - Displays usage information

    Provides usage(), which gets invoked whenever the user asks for
    help, or provides bad options.
*/

#include <stdio.h>

#include "wmget.h"


void usage (void)
{
    printf (
WMGET_VERSION_BANNER "\n"
WMGET_COPYRIGHT "\n"
"Usage:\n"
" wmget dock [options]          # To start up the dockapp\n"
" wmget [options] <URL>         # To request a download\n"
" wmget cancel <job-id>         # To cancel a download\n"
" wmget list                    # To show current and pending jobs\n"
" wmget --version               # (or -v) To print the version\n"
" wmget --help                  # (or -h) To print this text\n"
"Options:\n");

#define yes " ..."
#define no "    "
#define O(s,l,a,t) \
    printf (" -%c|--%-15s " t "\n", s, #l a);
#include "config.def"
#undef O
#undef no
#undef optional
#undef required
}



