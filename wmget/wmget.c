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
    wmget.c - main() and a few common functions

    This file contains the main() for wmget; it simply checks for the
    presence of ``dock'' as the first argument and invokes server(),
    request(), cancel(), or list() as appropriate.  Also found here are
    a few global utilities.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "wmget.h"

void debug_dump_job (Job *job)
{
    if (output_level () < OL_DEBUG)
        return;

    debug ("id = %lu\n"
           "status = %d\n"
           "progress=%d/%d\n"
           "stop_request=%d\n"
           "display=[%s]\n"
           "source_url=[%s]\n"
           "save_to=[%s]\n"
           "continue_from=%d\n\n",
           job->job_id,
           job->status, job->progress, job->stop_request,
           job->prog_max, job->options.display, job->source_url,
           job->options.save_to, job->options.continue_from);
}


const char *home_directory (void)
{
    static char *home_directory = 0;
    struct passwd *pwent;

    if (home_directory)
        return home_directory;

    home_directory = getenv ("HOME");

    if (!home_directory) {
        pwent = getpwuid (geteuid ());
        home_directory = strdup (pwent->pw_dir);
    }

    return home_directory;
}


int main (int argc, char **argv)
{
    char **arg;
    int (*function) (int, char **) = &request;

    if (argc < 2) {
        usage ();
        return 0;
    }

    /* Cheat, because we don't invoke getopt until we know what mode
     * we're in...
     */
    for (arg = argv; *arg; ++arg) {
        if (strcmp (*arg, "dock") == 0) {
            function = &server;

        } else if (strcmp (*arg, "cancel") == 0) {
            function = &cancel;

        } else if (strcmp (*arg, "list") == 0) {
            function = &list;

        } else if (strcmp (*arg, "-v") == 0
                || strcmp (*arg, "--version") == 0) {
            puts (WMGET_VERSION_BANNER);
            puts (WMGET_COPYRIGHT);
            return 0;

        } else if (strcmp (*arg, "-h") == 0
                || strcmp (*arg, "--help") == 0) {
            usage ();
            return 0;
        }
    }

    return function (argc, argv);
}






