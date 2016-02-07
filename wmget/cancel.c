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
    cancel.c - Client code for canceling download requests

    When invoked with the ``cancel'' argument, main() calls cancel(),
    defined below.  This code expects a job ID and forms and submits
    a CANCEL request to the server.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wmget.h"

int cancel (int argc, char **argv)
{
    char line[MAXCMDLEN + 1];
    char *word_break;
    FILE *fp;
    job_id_t job_id;

    /* The argument vector we receive is guaranteed to start with
     * { <program-name>, cancel }.  Currently, we don't support any
     * options or anything; we just expect a single nonzero job ID.
     */
    if (argc < 3) {
        error ("What do you want me to cancel?  (Missing job number)");
        return 1;
    } else if (argc > 3) {
        error ("Extra arguments: cancel takes only a job number");
        return 1;
    } else if (!(job_id = strtoul (argv[2], &word_break, 0))
               || *word_break) {
        error ("Invalid job number (must be an integer > 0)");
        return 1;
    }

    if (!(fp = iq_client_connect ()))
        return 1;

    if (fprintf (fp, "CANCEL JOBID(%lu)\r\n", job_id) == EOF) {
        error_sys ("Could not submit command to server");
        fclose (fp);
        return 1;
    }

    if (!fgets (line, sizeof line - 1, fp)) {
        error ("Server did not respond to command!");
        fclose (fp);
        return 1;
    }

    /* Extract the first word and compare. */
    word_break = line + strcspn (line, " \t\r\n");

    if (*word_break)
        *word_break++ = 0;

    if (strcasecmp (line, RESPONSE_JOB_CANCELED)) {
        error ("Server responded with error: %s", word_break);
        fclose (fp);
        return 1;
    }

    fclose (fp);

    return 0;
}


