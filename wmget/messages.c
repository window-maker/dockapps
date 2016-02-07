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
    messages.c - functions for writing error, status, & debug messages
*/

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "wmget.h"

static OutputLevel output_level_;


void set_output_level (OutputLevel lev)
{
    output_level_ = lev;
}


OutputLevel output_level (void)
{
    return output_level_;
}


void error (const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    fputs ("\n", stderr);
}

void error_sys (const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    perror (" ");
}

void info (const char *fmt, ...)
{
    va_list ap;

    if (output_level_ < OL_NORMAL)
        return;

    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    fputs ("\n", stderr);
}

void debug (const char *fmt, ...)
{
    va_list ap;

    if (output_level_ < OL_DEBUG)
        return;

    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    fputs ("\n", stderr);
}

void debug_sys (const char *fmt, ...)
{
    va_list ap;

    if (output_level_ < OL_DEBUG)
        return;

    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    perror (" ");
}


