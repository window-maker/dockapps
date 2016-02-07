#ifndef I_WMGET_H
#define I_WMGET_H
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
    wmget.h - Common definitions for all wmget modules

    This file defines the public entry points in each *.c file in the
    main wmget program, as well as some common constants, global
    variables, and the structure of the shared memory segment.
*/


#include <sys/param.h>
#include <curl/curl.h>

#define WMGET_VERSION           PACKAGE_VERSION
#define WMGET_VERSION_BANNER    "wmget " WMGET_VERSION \
                                ", compiled with libcurl " \
                                        LIBCURL_VERSION
#define WMGET_COPYRIGHT         "Copyright (c) 2001-2003 " \
                                 "Aaron Trickey <aaron@amtrickey.net>" \
                                 "; ABSOLUTELY NO WARRANTY"
                                                

#define DEFAULT_USER_AGENT \
    "wmget/" WMGET_VERSION " (libcurl/" LIBCURL_VERSION ")"

#define MAXRCLINELEN    1024
#define MAXURL          1024
#define MAXUA           255
#define MAXAUTH         100
#define MAXIF           255
#define MAXCMDLEN       2048
#define MAXCMDARGS      20
#define MAX_DISPLAY     9

#define MAX_ACTIVE_JOBS 4       /* a number constrained by the UI */
#define MAX_QUEUED_JOBS 20

/* Command language */

/* The GET command and its arguments: */
#define CMD_GET                 "GET"
#define ARG_GET_SOURCE_URL       "FROM"
#define ARG_GET_DISPLAY          "DISP"
#define ARG_GET_SAVE_TO          "TO"
#define ARG_GET_OVERWRITE        "OVER"
#define ARG_GET_CONTINUE_FROM    "CONT"
#define ARG_GET_PROXY            "PROXY"
#define ARG_GET_FOLLOW           "FOLLOW"
#define ARG_GET_UA               "UA"
#define ARG_GET_USE_ASCII        "ASCII"
#define ARG_GET_REFERER          "REF"
#define ARG_GET_INCLUDE          "INCL"
#define ARG_GET_INTERFACE        "IF"
#define ARG_GET_PROXY_AUTH       "PROXY-AUTH"
#define ARG_GET_AUTH             "AUTH"

#define CMD_CANCEL              "CANCEL"
#define ARG_CANCEL_JOBID         "JOBID"

#define CMD_LIST                "LIST"

#define RESPONSE_JOB_ACCEPTED   "ACCEPTED"
#define RESPONSE_JOB_CANCELED   "CANCELED"
#define RESPONSE_LIST_COMING    "JOBLIST"
#define RESPONSE_ERROR          "ERROR"


/* Debug trace text output levels */
typedef enum {
    OL_SILENT,
    OL_NORMAL,
    OL_DEBUG,
} OutputLevel;


/* The various states an individual job may be in.
 */
typedef enum {
    J_EMPTY,        /* this job slot is empty */
    J_INIT,         /* slot has been taken, job not yet started */
    J_RUNNING,      /* this job is running */
    J_PAUSED,       /* this job is paused */
    J_STOPPING,     /* a stop request has come from the user */
    J_COMPLETE,     /* job complete, cleaning up */
    J_FAILED,       /* job failed! */
} JobStatus;


/* The type of a job ID; these are allocated by the dockapp and are
 * never reused within its lifetime.
 */
typedef unsigned long job_id_t;


/* User-configurable job options.
 */
typedef struct {
    char  display[MAX_DISPLAY + 1]; /* Text to display */
    char  save_to[MAXPATHLEN + 1];  /* Full pathname to save to */
                                    /* (For srvr, this MUST be a dir) */
    int   overwrite;                /* Allow overwrite of save_to? */
    int   continue_from;            /* Byte# to resume from */
    char  proxy[MAXURL + 1];        /* Proxy to use (or empty string) */
    int   follow;                   /* How many redirects to follow */
    char  user_agent[MAXUA + 1];    /* User-agent string to provide */

    int   use_ascii;                /* Force FTP to ASCII */
    char  referer[MAXURL + 1];      /* Specify referer */
    int   include;                  /* Include HTTP headers in output */
    char  interface[MAXIF + 1];     /* Limit to given interface */
    char  proxy_auth[MAXAUTH + 1];  /* Proxy authentication */
    char  auth[MAXAUTH + 1];        /* Site authentication */

} JobOptions;


/* A command-line download request.  Strings are NULL if defaulted;
 * integers are -1.
 */
typedef struct {
    const char *source_url;         /* MANDATORY.  Duh. */
    const char *display;
    const char *save_to;
    int         overwrite;
    int         continue_from;
    const char *proxy;
    int         follow;
    const char *user_agent;

    int         use_ascii;
    const char *referer;
    int         include;
    const char *interface;
    const char *proxy_auth;
    const char *auth;

} Request;


/* The totality of a running or queued job:
 */
typedef struct {
    job_id_t job_id;
    JobStatus status;
    char error[CURL_ERROR_SIZE + 1];
    unsigned long progress;
    unsigned long prog_max;
    int stop_request;
    char source_url[MAXURL + 1];    /* URL to fetch */

    JobOptions options;
} Job;


/* The shared-memory structure containing the active job list.  (Pending
 * jobs are queued in the dockapp's private data segment.)
 */
typedef struct {
    Job jobs[MAX_ACTIVE_JOBS];
} Shmem;


/* Specifies the server configuration.  This is used only by server.c,
 * and gets populated by config_server() in config.c.
 */
typedef struct {
    JobOptions job_defaults;
} ServerConfig;


/* Convenience macro
 */
#define STRCPY_TO_ARRAY(to,from) \
    do { \
        strncpy (to, from, sizeof to); \
        to[sizeof to - 1] = '\0'; \
    } while (0)


/* configure.c */
extern void config_server (int argc, char **argv, ServerConfig *cfg);
extern void clear_request (Request *req);
extern void config_request (int argc, char **argv, Request *req);

/* usage.c */
extern void usage (void);

/* iq.c */
extern int iq_server_init (void);       /* called once, by server  */
extern FILE *iq_server_accept (void);   /* returns new cxn or NULL */
extern FILE *iq_client_connect (void);  /* called by each client   */
extern int iq_get_listen_fd (void);     /* so you can select/poll  */

/* server.c */
extern Shmem *shmem;
extern int server (int argc, char **argv);

/* request.c */
extern int request (int argc, char **argv);

/* cancel.c */
extern int cancel (int argc, char **argv);

/* list.c */
extern int list (int argc, char **argv);

/* retrieve.c */
extern int retrieve (Job *job);

/* wmget.c */
extern const char *home_directory (void);
extern void debug_dump_job (Job *job);

/* messages.c */
extern void set_output_level (OutputLevel lev);
extern OutputLevel output_level (void);
extern void error (const char *fmt, ...);
extern void error_sys (const char *fmt, ...);
extern void info (const char *fmt, ...);
extern void debug (const char *fmt, ...);
extern void debug_sys (const char *fmt, ...);


#endif /* I_WMGET_H */
