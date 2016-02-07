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
    list.c - Client code for retrieving the current job list

    When invoked with the ``list'' argument, main() calls list(),
    defined below.  This code sends a LIST command to the server and
    dumps the returned list to stdout.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wmget.h"

int list (int argc, char **argv)
{
    char line[MAXCMDLEN + 1];
    char *word_break;
    FILE *fp;

    /* No additional options or arguments.
     */
    if (argc > 2) {
        error ("Extra arguments: list takes none");
        return 1;
    }

    (void)argv;

    if (!(fp = iq_client_connect ()))
        return 1;

    if (fprintf (fp, "LIST\r\n") == EOF) {
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

    if (strcasecmp (line, RESPONSE_LIST_COMING)) {
        error ("Server responded with error: %s", word_break);
        fclose (fp);
        return 1;
    }

    while (fgets (line, sizeof line - 1, fp)) {
        fputs (line, stdout);
    }

    fclose (fp);

    return 0;
}


