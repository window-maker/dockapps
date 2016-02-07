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
    retrieve.c - Child process code for performing downloads

    When the server [server.c] accepts a download job, it fork()s a
    new process which in turn invokes this file's retrieve().  This
    code takes care of setting up and invoking libcurl to grab the
    data.  The shared memory segment (shmem, wmget.h) is inherited from
    the server and contains the Job structure through which the child
    process communicates with the server.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#include <curl/curl.h>
#include <curl/easy.h>

#include "wmget.h"

#if LIBCURL_VERSION_NUM >= 0x070905
#   define PROGRESS double
#else
#   define PROGRESS size_t
#endif

static int progress_callback (
        void *data,
        PROGRESS total,
        PROGRESS progress,
        PROGRESS unused1,
        PROGRESS unused2)
{
    Job *job = data;

    (void)unused1;
    (void)unused2;

    if (job->stop_request) {
        /* Abort transfer. */
        job->status = J_STOPPING;
        return 1;
    }

    if (!total) {
        debug ("Total bytes unknown!");
        total = progress * 2 + 1;   /* just to make the bar halfway */
    }
    job->progress = (unsigned long)progress;
    job->prog_max = (unsigned long)total;

    debug ("progress_callback (%d/%d)", job->progress, job->prog_max);

    return 0;
}


void write_error_file (Job *job, const char *msg)
{
    char error_file_name[MAXPATHLEN + 1];
    FILE *error_file;

    strcpy (error_file_name, job->options.save_to);
    strncat (error_file_name, ".ERROR",
            MAXPATHLEN - strlen (error_file_name));

    if (!(error_file = fopen (error_file_name, "w")))
        return;

    fprintf (error_file, "Download failed:\n");
    fprintf (error_file, " From URL: %s\n", job->source_url);
    fprintf (error_file, " To file: %s\n", job->options.save_to);
    fprintf (error_file, " Error: %s\n", msg);
    fprintf (error_file, " (" WMGET_VERSION_BANNER ")\n");
    fclose (error_file);
}


int retrieve (Job *job)
{
    CURL *curl;
    CURLcode rc;
    FILE *outfp;
    JobOptions *opts;

    debug ("Retrieval process %d running job:", getpid());
    debug_dump_job (job);

    if (job->options.continue_from) {
        outfp = fopen (job->options.save_to, "a");
    } else {
        outfp = fopen (job->options.save_to, "w");
    }

    if (!outfp) {
        error_sys ("could not open `%s' for output",
                   job->options.save_to);
        return 1;
    }

    curl = curl_easy_init ();
    if (!curl) {
        error ("could not initialize libcurl");
        write_error_file (job, "could not initialize libcurl");
        fclose (outfp);
        return 1;
    }

    curl_easy_setopt (curl, CURLOPT_FILE, outfp);
    curl_easy_setopt (curl, CURLOPT_URL, job->source_url);
    curl_easy_setopt (
            curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
    curl_easy_setopt (curl, CURLOPT_PROGRESSDATA, (void *)job);
    curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, job->error);
    curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0);

    if (job->options.continue_from) {
        curl_easy_setopt (curl, CURLOPT_RESUME_FROM,
                          (long)job->options.continue_from);
    }

    /* Now load the job's user-configurable parameters:
     */
    opts = &job->options;

    if (opts->proxy[0]) {
        curl_easy_setopt (curl, CURLOPT_PROXY, opts->proxy);
    }

    if (opts->follow) {
        curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt (curl, CURLOPT_MAXREDIRS, opts->follow);
    } else {
        curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 0);
    }

    if (opts->user_agent[0]) {
        curl_easy_setopt (curl, CURLOPT_USERAGENT, opts->user_agent);
    }

    if (opts->use_ascii) {
        curl_easy_setopt (curl, CURLOPT_TRANSFERTEXT, 1);
    }

    if (opts->referer[0]) {
        curl_easy_setopt (curl, CURLOPT_REFERER, opts->referer);
    }

    if (opts->include) {
        curl_easy_setopt (curl, CURLOPT_HEADER, 1);
    }

    if (opts->interface[0]) {
        curl_easy_setopt (curl, CURLOPT_INTERFACE, opts->interface);
    }

    if (opts->proxy_auth[0]) {
        curl_easy_setopt (curl, CURLOPT_PROXYUSERPWD, opts->proxy_auth);
    }

    if (opts->auth[0]) {
        curl_easy_setopt (curl, CURLOPT_USERPWD, opts->auth);
    }


    /* If wmget is verbose, set libcurl to verbose too...
     */
    if (output_level () > OL_NORMAL)
        curl_easy_setopt (curl, CURLOPT_VERBOSE, 1);


    /* Finally, perform the download:
     */
    job->status = J_RUNNING;
    rc = curl_easy_perform (curl);

    if (rc) {
        if (job->status == J_STOPPING) {
            info ("aborted by user");
            job->status = J_COMPLETE;

        } else {
            error (job->error);
            write_error_file (job, job->error);
            job->status = J_COMPLETE;
        }
    } else {
        job->status = J_COMPLETE;
    }

    curl_easy_cleanup (curl);
    fclose (outfp);

    if (rc)
        return 1;

    return 0;
}


