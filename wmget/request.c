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
    request.c - Client code for submitting download requests

    Invoked whenever wmget is run in "request" mode; submits a job to
    the server dockapp.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

#include "wmget.h"


static char *enquote_strdup (const char *string)
{
    int len = strlen (string);
    char *newstr;
    const char *src;
    char *dest;

    for (src = string; *src; src += strcspn (src, ")\\")) {
        ++len;
    }

    dest = newstr = malloc (len + 1);

    for (src = string; *src; ++src) {
        switch (*src) {
            case ')':
            case '\\':
                *dest++ = '\\';
        }

        *dest++ = *src;
    }

    *dest = '\0';

    return newstr;
}


int add_arg_s (char *line, const char *argname, const char *argval)
{
    char *qargval;

    if (strlen (line) + strlen (argname) + strlen (argval) + 3
            > MAXCMDLEN) {
        error ("Too much data to send to server!");
        return 1;
    }

    qargval = enquote_strdup (argval);

    line += strlen (line);
    sprintf (line, " %s(%s)", argname, qargval);

    free (qargval);

    return 0;
}


int add_arg_i (char *line, const char *argname, int argval)
{
    if (strlen (line) + strlen (argname) + 10
            > MAXCMDLEN) {
        error ("Too much data to send to server!");
        return 1;
    }

    line += strlen (line);
    sprintf (line, " %s(%d)", argname, argval);

    return 0;
}


int request (int argc, char **argv)
{
    char line[MAXCMDLEN + 1];
    char *word_break;
    FILE *fp;
    Request req;

    config_request (argc, argv, &req);

    if (!req.source_url) {
        error ("Missing source URL!");
        usage ();
        return 1;
    }

    strcpy (line, CMD_GET);
    if (add_arg_s (line, ARG_GET_SOURCE_URL, req.source_url))
        return 1;

    if (req.display)
        if (add_arg_s (line, ARG_GET_DISPLAY, req.display))
            return 1;

    if (req.save_to)
        if (add_arg_s (line, ARG_GET_SAVE_TO, req.save_to))
            return 1;

    if (req.overwrite != -1)
        if (add_arg_i (line, ARG_GET_OVERWRITE, req.overwrite))
            return 1;

    if (req.continue_from != -1)
        if (add_arg_i (line, ARG_GET_CONTINUE_FROM, req.continue_from))
            return 1;

    if (req.proxy)
        if (add_arg_s (line, ARG_GET_PROXY, req.proxy))
            return 1;

    if (req.follow != -1)
        if (add_arg_i (line, ARG_GET_FOLLOW, req.follow))
            return 1;

    if (req.user_agent)
        if (add_arg_s (line, ARG_GET_UA, req.user_agent))
            return 1;

    if (req.use_ascii != -1)
        if (add_arg_i (line, ARG_GET_USE_ASCII, req.use_ascii))
            return 1;

    if (req.referer)
        if (add_arg_s (line, ARG_GET_REFERER, req.referer))
            return 1;

    if (req.include != -1)
        if (add_arg_i (line, ARG_GET_INCLUDE, req.include))
            return 1;

    if (req.interface)
        if (add_arg_s (line, ARG_GET_INTERFACE, req.interface))
            return 1;

    if (req.proxy_auth)
        if (add_arg_s (line, ARG_GET_PROXY_AUTH, req.proxy_auth))
            return 1;

    if (req.auth)
        if (add_arg_s (line, ARG_GET_AUTH, req.auth))
            return 1;

    debug ("Command line is '%s'", line);

    strcat (line, "\n");

    if (!(fp = iq_client_connect ()))
        return 1;

    if (fputs (line, fp) == EOF) {
        error_sys ("Failed to send command to server (fputs)");
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

    while (*word_break && isspace (*word_break))
        *word_break++ = 0;

    if (strcasecmp (line, RESPONSE_JOB_ACCEPTED)) {
        error ("%s", word_break);
        fclose (fp);
        return 1;
    }

    fclose (fp);

    /* Since we got a RESPONSE_JOB_ACCEPTED, present the user with the
     * job ID for future cancellation.
     */
    printf ("Job ID is %s\n", word_break);

    return 0;
}

