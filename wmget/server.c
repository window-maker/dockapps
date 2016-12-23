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
    server.c - Manage the dock app display, accept & spawn jobs

    When ``wmget --dock'' is invoked, main() calls server(), defined
    below.  This initializes the dock app window, the shared memory
    segment, and the job input queue, and then enters the main loop
    whereby it accepts X events (such as redraws or clicks), accepts
    job requests (from request(), in request.c), and monitors shared
    memory, updating the display as necessary.  It forks off children to
    handle accepted jobs; see retrieve.c.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "wmget.h"
#include "wmget.xpm"
#include "dockapp/dockapp.h"


static ServerConfig config;

/***********************************************************************
 * Text Drawing
 *  The various CHAR_* consts locate and dimension the chars on the xpm.
 *  Call init_font() to set up the CHAR_X and CHAR_Y tables, then
 *  draw_string() to put text on the xpm.
 */
static const int CHAR_WIDTH = 6;
static const int CHAR_HEIGHT = 7;

static const int CHAR_UCALPHA_X = 1;
static const int CHAR_UCALPHA_Y = 85;
static const int CHAR_LCALPHA_X = 1;
static const int CHAR_LCALPHA_Y = 95;
static const int CHAR_SYMNUM_X = 1;
static const int CHAR_SYMNUM_Y = 105;

static int CHAR_X[128];
static int CHAR_Y[128];

static void init_font (void)
{
    int i;
    int *cx, *cy;

    for (i = 0, cx = CHAR_X, cy = CHAR_Y; i < 128; ++i, ++cx, ++cy) {
        if (i > 'z') {
            *cx = CHAR_SYMNUM_X;    /* 1st SYMNUM is the space */
            *cy = CHAR_SYMNUM_Y;
        } else if (i >= 'a') {
            *cx = CHAR_LCALPHA_X + CHAR_WIDTH * (i - 'a');
            *cy = CHAR_LCALPHA_Y;
        } else if (i > 'Z') {
            *cx = CHAR_SYMNUM_X;
            *cy = CHAR_SYMNUM_Y;
        } else if (i >= 'A') {
            *cx = CHAR_UCALPHA_X + CHAR_WIDTH * (i - 'A');
            *cy = CHAR_UCALPHA_Y;
        } else if (i > '9') {
            *cx = CHAR_SYMNUM_X;
            *cy = CHAR_SYMNUM_Y;
        } else if (i >= ' ') {
            *cx = CHAR_SYMNUM_X + CHAR_WIDTH * (i - ' ');
            *cy = CHAR_SYMNUM_Y;
        } else {
            *cx = CHAR_SYMNUM_X;
            *cy = CHAR_SYMNUM_Y;
        }
    }
}

static void draw_string (const char *str, int x, int y)
{
    for ( ; *str; ++str) {
        dockapp_overlay_pixmap (
                CHAR_X[(int)*str], CHAR_Y[(int)*str],
                x, y,
                CHAR_WIDTH, CHAR_HEIGHT);
        x += CHAR_WIDTH;
    }
}


/***********************************************************************
 * Button Widgets
 */

static const int BTN_PAUSE_X = 128;
static const int BTN_STOP_X = 147;
static const int BTN_Y = 37;
static const int BTN_WIDTH = 19;
static const int BTN_HEIGHT = 9;


/***********************************************************************
 * Progress Bars
 */

/* Coords and dimensions refer to the pbars, excluding the borders
 * which make up the ``ditches''
 */
static const int PBAR_Y[4] = {
    5,
    20,
    35,
    50,
};

static const int PBAR_X = 3;

/* These are the graphics for the bars themselves.
 */
static const int PBAR_FULL_X = 67;
static const int PBAR_FULL_Y = 37;

static const int PBAR_EMPTY_X = 67;
static const int PBAR_EMPTY_Y = 47;

static const int PBAR_LENGTH = 58;
static const int PBAR_HEIGHT = 9;

static int bar_selected = -1;

static void draw_pbar (int trough_x, int trough_y, int value, int max)
{
    int width = ((unsigned long) PBAR_LENGTH * value) / max;

    dockapp_copy_pixmap (
            PBAR_FULL_X, PBAR_FULL_Y,
            trough_x, trough_y,
            width, PBAR_HEIGHT);

    dockapp_copy_pixmap (
            PBAR_EMPTY_X, PBAR_EMPTY_Y,
            trough_x + width, trough_y,
            PBAR_LENGTH - width, PBAR_HEIGHT);
}

static const char *const DEFAULT_TEXT[] = {
    "    wmget",
    "",
    "",
    "",
};

static void draw_pbars (void)
{
    int i;

    for (i = 0; i < 4; ++i) {
        Job *j = &shmem->jobs[i];

        if (j->status == J_EMPTY) {
            draw_pbar (PBAR_X, PBAR_Y[i], 0, 1);
            draw_string (DEFAULT_TEXT[i], PBAR_X + 1, PBAR_Y[i] + 1);
            continue;
        }

        if (i == bar_selected) {
            /* percentage (or error) + stop button */

            draw_pbar (PBAR_X, PBAR_Y[i], 0, 1);

            if (j->status != J_FAILED) {
                char pct[4];
                sprintf (pct, "%02lu%%",
                         100L * j->progress / j->prog_max);

                draw_string (pct, PBAR_X + 1, PBAR_Y[i] + 1);
            } else {
                char err[9];
                strncpy (err, j->error, 8);
                err[8] = '\0';
                draw_string (err, PBAR_X + 1, PBAR_Y[i] + 1);
            }

            dockapp_copy_pixmap (
                    BTN_STOP_X, BTN_Y,
                    PBAR_X + PBAR_LENGTH - BTN_WIDTH, PBAR_Y[i],
                    BTN_WIDTH, BTN_HEIGHT);
        } else {
            /* name + scrollbar, or error */
            draw_pbar (PBAR_X, PBAR_Y[i], j->progress, j->prog_max);
            draw_string (j->options.display, PBAR_X + 1, PBAR_Y[i] + 1);
        }
    }
}

/***********************************************************************
 * Shared memory segment: global pointer, constructor
 */
Shmem *shmem = 0;

static int init_shmem (void)
{
    int shmid;
    int i;

    if ((shmid = shmget (IPC_PRIVATE, sizeof *shmem, SHM_R | SHM_W)) < 0) {
        error_sys ("could not allocate shared memory segment [shmget()]");
        return 1;
    }

    if ((shmem = shmat (shmid, 0, 0)) == (void *) -1) {
        error_sys ("could not attach shared memory segment [shmat()]");
        return 1;
    }

    for (i = 0; i < 4; ++i) {
        shmem->jobs[i].status = J_EMPTY;
        shmem->jobs[i].options.display[0] = 0;
        shmem->jobs[i].progress = 0;
        shmem->jobs[i].prog_max = 0;
    }

    return 0;
}

/***********************************************************************
 * start_job(): Spawn a new process and call retrieve() in there.
 * Note that `j' must be in shared memory.
 */
static int start_job (Job *j)
{
    int f;

    j->prog_max = 1;
    j->progress = 0;
    j->status = J_INIT;
    j->stop_request = 0;

    f = fork ();
    if (f < 0) {
        error_sys ("could not create child process [fork()]");
        return 1;
    } else if (f == 0) {    /* child */
        retrieve (j);

        if (j->status == J_FAILED) {
            /* Sleep until user acks the error.
             */
            while (!j->stop_request) {
                struct timespec sleeptime = { 0, 100000000L };
                nanosleep (&sleeptime, NULL);
            }
        }

        j->status = J_EMPTY;
        exit (0);
    }

    return 0;
}

/***********************************************************************
 * The Job Queue.  Okay, this is a little cheesy right now.
 */
static Job *job_queue[MAX_QUEUED_JOBS] = { 0 };
static size_t job_queue_depth = 0;
static job_id_t next_job_id = 1;       /* Job id 0 is never valid */


/***********************************************************************
 * process_queue(): If a job has finished, pull it from its slot.
 * If a slot is open, pull the next job from the queue.
 */
static int process_queue (void)
{
    size_t i;

    for (i = 0; i < MAX_ACTIVE_JOBS; ++i) {
        switch (shmem->jobs[i].status) {
            default:    /* job occupying slot */
                continue;

            case J_EMPTY:
                /* aha.  see if there is anything queued up */
                if (!job_queue_depth)
                    continue;

                shmem->jobs[i] = *job_queue[--job_queue_depth];

                free (job_queue[job_queue_depth]);

                debug ("Pulled new active job %lu off queue",
                       shmem->jobs[i].job_id);

                if (start_job (&shmem->jobs[i]))
                    return 1;

                continue;
        }
    }

    return 0;
}

/***********************************************************************
 * cancel_job(): Cancel a job.  If it's running, stop it; if it's
 * in the queue, dequeue it; if it's nowhere, do nothing.
 */
static int cancel_job (job_id_t job_id)
{
    Job *j;
    Job **jp;

    /* First search the active jobs. */
    for (j = shmem->jobs; j < shmem->jobs + MAX_ACTIVE_JOBS; ++j) {
        if (j->job_id == job_id) {
            switch (j->status) {
                case J_EMPTY:
                case J_STOPPING:
                case J_COMPLETE:
                    /* Job has already completed. */
                    return 0;
                case J_FAILED:
                    /* Job has already failed; this simply clears it
                     * out. */
                    j->status = J_COMPLETE;
                    return 0;
                default:
                    /* just to keep the compiler warnings at bay */
                    break;
            }

            ++j->stop_request;  /* Request job termination. */

            return 0;
        }
    }

    /* Okay, now search the pending queue. */
    for (jp = job_queue; jp < job_queue + job_queue_depth; ++jp) {
        if (*jp && (*jp)->job_id == job_id) {
            /* Simply delete it from the queue. */
            --job_queue_depth;
            memmove (jp, jp + 1,
                 ((job_queue + job_queue_depth) - jp) * sizeof (Job *));

            return 0;
        }
    }

    /* Job not found.  This is not an error, as we assume that the
     * job has already died off.
     */
    return 0;
}

/***********************************************************************
 * client_*(): Issue a response back to the client.
 */
static void client_error (FILE *fp, const char *fmt, ...)
{
    va_list ap;
    va_start (ap, fmt);

    fprintf (fp, RESPONSE_ERROR " ");
    vfprintf (fp, fmt, ap);
    va_end (ap);
}

static void client_job_accepted (FILE *fp, job_id_t job_id)
{
    fprintf (fp, RESPONSE_JOB_ACCEPTED " %lu\n", job_id);
}

static void client_job_canceled (FILE *fp, job_id_t job_id)
{
    fprintf (fp, RESPONSE_JOB_CANCELED " %lu\n", job_id);
}

static void client_list_header (FILE *fp)
{
    debug ("client_list_header()");

    fprintf (fp, RESPONSE_LIST_COMING "\n");
}

static void client_list_job (FILE *fp, Job *j)
{
    const char *status;

    switch (j->status) {
        default:
            status = "UNKNOWN: Internal Error!";
            break;

        case J_INIT:
            status = "INIT: Waiting to start";
            break;

        case J_RUNNING:
            status = "RUNNING: Currently retrieving";
            break;

        case J_PAUSED:
            status = "PAUSED: Download suspended";
            break;

        case J_STOPPING:
            status = "STOPPING: Got stop request";
            break;

        case J_COMPLETE:
            status = "COMPLETE: Download complete";
            break;

        case J_FAILED:
            status = j->error;
            break;
    }

    fprintf (fp, "Job %lu [%9s]: %lu/%lu %s\n%s => %s\n\n",
             j->job_id, j->options.display, j->progress, j->prog_max,
             status, j->source_url, j->options.save_to);
}


static int insert_job (Job *j)
{
    if (job_queue_depth >= MAX_QUEUED_JOBS) {
        error ("Job queue full");
        free (j);
        return 1;
    }

    j->job_id = next_job_id++;

    debug ("Accepted job...");
    debug_dump_job (j);

    job_queue[job_queue_depth++] = j;

    return 0;
}


static Job *init_job (Request *req, FILE *errfp)
{
    struct stat st;
    const char *base_first;
    const char *base_last;
    size_t base_sz;

    Job *j = malloc (sizeof (Job));
    if (!j) {
        client_error (errfp, "Dockapp out of memory!");
        return 0;
    }

    STRCPY_TO_ARRAY (j->source_url, req->source_url);

    j->status = J_INIT;
    j->progress = j->prog_max = 0;
    j->stop_request = 0;
    j->options = config.job_defaults;

    /* Copy over any applicable options---except save_to and display,
     * which merit special consideration below.
     */
    if (req->overwrite != -1)
        j->options.overwrite = req->overwrite;

    if (req->continue_from != -1)
        j->options.continue_from = req->continue_from;

    if (req->proxy)
        STRCPY_TO_ARRAY (j->options.proxy, req->proxy);

    if (req->follow != -1)
        j->options.follow = req->follow;

    if (req->user_agent)
        STRCPY_TO_ARRAY (j->options.user_agent, req->user_agent);

    if (req->use_ascii != -1)
        j->options.use_ascii = req->use_ascii;

    if (req->referer)
        STRCPY_TO_ARRAY (j->options.referer, req->referer);

    if (req->include != -1)
        j->options.include = req->include;

    if (req->interface)
        STRCPY_TO_ARRAY (j->options.interface, req->interface);

    if (req->proxy_auth)
        STRCPY_TO_ARRAY (j->options.proxy_auth, req->proxy_auth);

    if (req->auth)
        STRCPY_TO_ARRAY (j->options.auth, req->auth);

    /* Extract the "base name" (last slash-delimited component) of the
     * source URL for future use.
     */
    base_last = j->source_url + strlen (j->source_url) - 1;
    while (*base_last == '/' && base_last > j->source_url)
        --base_last;
    base_first = base_last;
    while (*base_first != '/' && base_first > j->source_url)
        --base_first;
    base_sz = base_last - base_first;
    ++base_first;   /* get it past that initial slash */

    if (base_sz == 0) {
        /* Uh, oh... invalid source_url anyway... give up. */
        client_error (errfp, "Invalid URL '%s'", j->source_url);
        goto RETURN_NULL;
    }

    debug ("baselen %d", base_sz);

    /* If no display-name was provided, use the basename.
     */
    if (req->display) {
        STRCPY_TO_ARRAY (j->options.display, req->display);
    } else {
        size_t n = base_sz;
        if (n > sizeof j->options.display - 1)
            n = sizeof j->options.display - 1;
        strncpy (j->options.display, base_first, n);
        j->options.display[n] = '\0';
        debug ("display was empty... set it to %s", j->options.display);
    }


    /* If there was a save-to location provided, copy it into the job.
     * If it's a relative path, make it relative to the download
     * directory.  If it wasn't given, just copy the download dir.
     */
    if (req->save_to) {
        if (req->save_to[0] == '/') {
            debug ("Reqest contained absolute dir.");
        } else {
            STRCPY_TO_ARRAY (j->options.save_to,
                             config.job_defaults.save_to);
            if (strlen (j->options.save_to) + strlen (req->save_to) + 2
                    > MAXPATHLEN) {
                client_error (errfp,
                              "Download output pathname too long");
                goto RETURN_NULL;
            }
            strcat (j->options.save_to, "/");
            strcat (j->options.save_to, req->save_to);

            debug ("Resolved output to '%s'", j->options.save_to);
        }
    } else {
        STRCPY_TO_ARRAY (j->options.save_to,
                         config.job_defaults.save_to);
        debug ("Defaulted output to '%s'", j->options.save_to);
    }


    /* Now we've got something... let's see what it is...
     */
    if (stat (j->options.save_to, &st)) {
        if (errno == ENOENT) {
            /* Name of a file which doesn't exist... ready to save. */
            debug ("Target does not exist.");
            return j;
        }
        error_sys ("could not stat(`%s')", j->options.save_to);
        client_error (errfp, "Failed when checking pathname '%s'",
                      j->options.save_to);
        goto RETURN_NULL;
    }

    /* If it's a directory name, append the basename from above and
     * re-stat.
     */
    if (S_ISDIR (st.st_mode)) {
        int offset = strlen (j->options.save_to);
        debug ("Is a directory.");
        if (offset + base_sz + 2 > sizeof j->options.save_to) {
            client_error (errfp, "Save-to path too long!");
            goto RETURN_NULL;
        }

        j->options.save_to[offset] = '/';
        strncpy (j->options.save_to + offset + 1, base_first, base_sz);
        j->options.save_to[offset + 1 + base_sz] = '\0';

        debug ("Extended to %s", j->options.save_to);

        if (stat (j->options.save_to, &st)) {
            if (errno == ENOENT) {
                return j;
            }
            error_sys ("could not stat(`%s')", j->options.save_to);
            client_error (errfp, "Failed when checking pathname '%s'",
                          j->options.save_to);
            goto RETURN_NULL;
        }
    }

    /* If we're here, it's not a directory but it exists. */
    debug ("%s Exists!", j->options.save_to);
    if (!j->options.overwrite && !j->options.continue_from) {
        client_error (errfp,
                      "File '%s' exists and --overwrite not specified",
                      j->options.save_to);
        goto RETURN_NULL;
    }

    /* For continuations, get the file length.  If the file does not
     * exist, just disable continuation; this is not an error.
     * (Continuation may now be permanently enabled in the RC file.)
     */
    if (j->options.continue_from) {
        if (S_ISREG (st.st_mode)) {
            j->options.continue_from = st.st_size;
        } else {
            j->options.continue_from = 0;
        }
    }

    /* Finally, check permissions */
    if ((st.st_mode & S_IWOTH)
        || ((st.st_mode & S_IWGRP) && st.st_gid == getegid ())
        || ((st.st_mode & S_IWUSR) && st.st_uid == geteuid ()))
        return j;

    client_error (errfp, "File '%s' exists and is not writable.\n",
                  j->options.save_to);

RETURN_NULL:
    free (j);

    return 0;
}


/***********************************************************************
 * process_*(): Implementations of each server command.
 */
static void process_get (
        FILE *fp, char *argnames[], char *argvalues[])
{
    char **an, **av;
    Request req;
    Job *job;

    debug ("process_get()");

    /* Don't waste the user's time if we're full already... */
    if (job_queue_depth >= MAX_QUEUED_JOBS) {
        client_error (fp, "Job queue full");
        return;
    }

    /* Empty out the request object... */
    clear_request (&req);

    /* And parse the args... */
    for (an = argnames, av = argvalues; *an && *av; ++an, ++av) {
        if (strcasecmp (*an, ARG_GET_SOURCE_URL) == 0) {
            if (strlen (*av) > MAXURL) {
                client_error (fp, "Source URL too long");
                return;
            }
            req.source_url = *av;

        } else if (strcasecmp (*an, ARG_GET_DISPLAY) == 0) {
            req.display = *av;

        } else if (strcasecmp (*an, ARG_GET_SAVE_TO) == 0) {
            req.save_to = *av;

        } else if (strcasecmp (*an, ARG_GET_CONTINUE_FROM) == 0) {
            char *end;
            req.continue_from = strtoul (*av, &end, 0);
            if (*end) {
                client_error (fp,
                        ARG_GET_CONTINUE_FROM ": must be an integer");
                return;
            }

        } else if (strcasecmp (*an, ARG_GET_OVERWRITE) == 0) {
            req.overwrite = 1;

        } else if (strcasecmp (*an, ARG_GET_PROXY) == 0) {
            req.proxy = *av;

        } else if (strcasecmp (*an, ARG_GET_FOLLOW) == 0) {
            req.follow = atoi (*av);

        } else if (strcasecmp (*an, ARG_GET_UA) == 0) {
            req.user_agent = *av;

        } else if (strcasecmp (*an, ARG_GET_USE_ASCII) == 0) {
            req.use_ascii = 1;

        } else if (strcasecmp (*an, ARG_GET_REFERER) == 0) {
            req.referer = *av;

        } else if (strcasecmp (*an, ARG_GET_INCLUDE) == 0) {
            req.include = 1;

        } else if (strcasecmp (*an, ARG_GET_INTERFACE) == 0) {
            req.interface = *av;

        } else if (strcasecmp (*an, ARG_GET_PROXY_AUTH) == 0) {
            req.proxy_auth = *av;

        } else if (strcasecmp (*an, ARG_GET_AUTH) == 0) {
            req.auth = *av;

        } else {
            client_error (fp, "Unknown parameter '%s'", *an);
            return;
        }
    }

    job = init_job (&req, fp);
    if (!job)
        return;

    if (insert_job (job)) {
        client_error (fp, "Invalid job parameters");
        free (job);
    } else {
        client_job_accepted (fp, job->job_id);
    }
}


static void process_cancel (
        FILE *fp, char *argnames[], char *argvalues[])
{
    char **an, **av;
    job_id_t job_id = 0;   /* job id 0 is never valid */

    debug ("process_cancel()");

    for (an = argnames, av = argvalues; *an && *av; ++an, ++av) {
        if (strcasecmp (*an, ARG_CANCEL_JOBID) == 0) {
            job_id = strtoul (*av, 0, 0);
        } else {
            client_error (fp, "Unknown parameter '%s'", *an);
            return;
        }
    }

    if (job_id == 0) {
        client_error (fp,
            CMD_CANCEL " requires the argument " ARG_CANCEL_JOBID);
        return;
    }

    if (cancel_job (job_id))
        client_error (fp, "Cancel failed");
    else
        client_job_canceled (fp, job_id);
}


void process_list (FILE *fp, char *argnames[], char *argvalues[])
{
    Job *j, **jp;

    (void)argnames;
    (void)argvalues;

    debug ("process_list()");

    client_list_header (fp);

    /* First list the active jobs. */
    for (j = shmem->jobs; j < shmem->jobs + MAX_ACTIVE_JOBS; ++j)
        if (j->status != J_EMPTY)
            client_list_job (fp, j);

    /* Then the waiting jobs. */
    for (jp = job_queue; jp < job_queue + job_queue_depth; ++jp)
        client_list_job (fp, *jp);
}

/***********************************************************************
 * process_request(): Accept a command and parameters, process it,
 * and reply.
 */
static void process_request (FILE *fp)
{
    char command[MAXCMDLEN];
    char *arg;
    int nargs;

    char *argnames[MAXCMDARGS + 1];
    char *argvalues[MAXCMDARGS + 1];

    /* A command line always comes first. */
    if (!fgets (command, sizeof command - 1, fp)) {
        debug ("No command on pipe!");
        return;
    }

    /* Arguments come after whitespace.  Note that not all commands have
     * args.  Each argument looks like this: PARAMETERNAME(VALUE).
     * Hey, don't tell me *none* of you have ever done AS/400 CL....
     */
    nargs = 0;
    arg = command + strcspn (command, " \t\r\n");
    *arg++ = 0;
    arg += strspn (arg, " \t");

    while (*arg && *arg != '\n' && *arg != '\r') {
        if (nargs > MAXCMDARGS - 1) {
            client_error (fp, "Too many arguments!");
            return;
        }

        argnames[nargs] = arg;

        if (!(arg = strchr (arg, '('))) {
            client_error (fp, "Argument missing value");
            return;
        }

        *arg++ = 0;

        argvalues[nargs] = arg;

        /* Arguments are terminated by a ), of course, but they may
         * also contain characters (such as )) quoted by \.
         */
        while (*arg && *arg != ')') {
            if (*arg == '\\') {
                if (!arg[1]) {
                    client_error (fp, "Argument missing closing ), "
                                      "ended with \\ by itself");
                    return;
                }

                /* strlen(arg+1)+1 = strlen(arg)-1+1 = strlen(arg) */
                memmove (arg, arg + 1, strlen (arg));
            }
            ++arg;
        }

        if (!arg) {
            client_error (fp, "Argument missing closing )");
            return;
        }

        *arg++ = 0;

        arg += strspn (arg, " \t");

        ++nargs;
    }

    argnames[nargs] = 0;
    argvalues[nargs] = 0;

    /* Got a valid command/argument set.  Process it. */
    if (strcasecmp (command, CMD_GET) == 0)
        process_get (fp, argnames, argvalues);
    else if (strcasecmp (command, CMD_CANCEL) == 0)
        process_cancel (fp, argnames, argvalues);
    else if (strcasecmp (command, CMD_LIST) == 0)
        process_list (fp, argnames, argvalues);
    else
        client_error (fp, "Unknown command");
}


/***********************************************************************
 * on_iq_ready(): invoked by the dockapp lib when there are connections
 * pending on the iq
 */
static dockapp_rv_t on_iq_ready (void *unused0, short unused1)
{
    FILE *fp;

    (void)unused0;
    (void)unused1;

    debug ("on_iq_ready");

    if ((fp = iq_server_accept ())) {
        process_request (fp);

        fclose (fp);
    }

    return dockapp_ok;
}

static int init_grim_reaper (void)
{
    struct sigaction sa;

    sa.sa_handler = SIG_IGN;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;
    /* Obsolete - sa.sa_restorer = 0; */

    if (sigaction (SIGCHLD, &sa, 0)) {
        error_sys ("sigaction(SIGCHLD) failed");
        return 1;
    }

    return 0;
}


static dockapp_rv_t on_click_pbar (
        void *cbdata, int x_unused, int y_unused)
{
    int which = (intptr_t)cbdata;
    (void)x_unused;
    (void)y_unused;

    debug ("got a click on pbar %d", which);

    if (bar_selected == which) {
        /* Selected bar gets deselected. */
        bar_selected = -1;

    } else {
        bar_selected = which;
    }

    return dockapp_ok;
}

static dockapp_rv_t on_click_stop (
        void *cbdata, int x_unused, int y_unused)
{
    int which = (intptr_t)cbdata;
    (void)x_unused;
    (void)y_unused;

    debug ("got a click on stop %d", which);

    if (bar_selected == which) {
        /* got a stop request (only works on selected pbar) */
        ++shmem->jobs[which].stop_request;
    }

    return dockapp_ok;
}

static dockapp_rv_t on_periodic_callback (void *cbdata)
{
    (void)cbdata;

    if (process_queue ())
        return dockapp_exit;

    draw_pbars ();

    return dockapp_ok;
}

static dockapp_rv_t on_got_selection (void *cbdata, const char *str)
{
    Request req;
    Job *j;

    (void)cbdata;

    debug ("on_got_selection >> %s", str);

    if (strlen (str) > MAXURL) {
        error ("rejecting job submission: URL too long!");
        return dockapp_ok;
    }

    clear_request (&req);

    req.source_url = str;

    j = init_job (&req, stderr);
    if (!j) {
        return dockapp_ok;
    }

    debug ("submitting job for [%s]...", j->source_url);

    if (insert_job (j)) {
        free (j);
        debug ("insert_job rejected it!");
    }

    return dockapp_ok;
}

static dockapp_rv_t on_middle_click (void *cbdata, int x, int y)
{
    (void)cbdata;
    (void)x;
    (void)y;

    debug ("on_middle_click");

    dockapp_request_selection_string (on_got_selection, 0);

    return dockapp_ok;
}


/* This is the main routine for the dock app (the first instance
 * started)
 */
int server (int argc, char **argv)
{
    intptr_t i;

    config_server (argc, argv, &config);

    if (init_grim_reaper ())
        return 1;

    if (init_shmem ())
        return 1;

    if (iq_server_init ())
        return 1;

    init_font ();

    dockapp_init_gui ("wmget", argv, wmget_xpm);

    for (i = 0; i < 4; ++i) {
        dockapp_add_clickregion (
                PBAR_X + PBAR_LENGTH - BTN_WIDTH, PBAR_Y[i],
                PBAR_LENGTH, PBAR_HEIGHT,
                Button1Mask,
                on_click_stop, (void *)i);

        dockapp_add_clickregion (
                PBAR_X, PBAR_Y[i],
                PBAR_LENGTH, PBAR_HEIGHT,
                Button1Mask,
                on_click_pbar, (void *)i);
    }

    dockapp_add_clickregion (
            0, 0, 64, 64, Button2Mask, on_middle_click, 0);

    dockapp_add_pollfd (iq_get_listen_fd (), POLLIN, on_iq_ready, 0);

    dockapp_set_periodic_callback (400, on_periodic_callback, 0);

    /* Perform one initial refresh.
     */
    on_periodic_callback (0);

    dockapp_run ();

    return 0;
}

