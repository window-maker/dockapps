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
    config.c - Implementation of command-line and RC-file configuration

    The code in this file parses RC files and command lines and provides
    defaults for what you don't specify.  Used to configure both
    requests and servers.
*/

#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "wmget.h"





/* Option characters.
 */
#define O(s,l,a,t)      optchar_##l = s,
enum {
#include "config.def"
};
#undef O



/***********************************************************************
 * clear_request(): Initialize an empty Request object.
 */
void clear_request (Request *req)
{
    req->source_url = 0;
    req->display = 0;
    req->save_to = 0;
    req->overwrite = -1;
    req->continue_from = -1;
    req->proxy = 0;
    req->follow = -1;
    req->user_agent = 0;

    req->use_ascii = -1;
    req->referer = 0;
    req->include = -1;
    req->interface = 0;
    req->proxy_auth = 0;
    req->auth = 0;
}


static void set_silent ()
{
    set_output_level (OL_SILENT);
}


static void set_verbose ()
{
    set_output_level (OL_DEBUG);
}


static void set_output (Request *r, ServerConfig *c, const char *value)
{
    if (r) r->save_to = value;
    if (c) {
        struct stat st;

        /* For server configuration, we must set the default to an
         * absolute path.
         */
        if (value[0] == '/') {
            if (strlen (value) + 1 > sizeof c->job_defaults.save_to) {
                error (
"Download directory name too long!  Defaulting to home directory");
                strcpy (c->job_defaults.save_to, home_directory ());
            } else {
                strcpy (c->job_defaults.save_to, value);
            }
        } else {
            if (strlen (value) + strlen (home_directory ()) + 2
                    > sizeof c->job_defaults.save_to) {
                error (
"Download directory name too long!  Defaulting to home directory");
                strcpy (c->job_defaults.save_to, home_directory ());
            } else {
                strcpy (c->job_defaults.save_to, home_directory ());
                strcat (c->job_defaults.save_to, "/");
                strcat (c->job_defaults.save_to, value);
            }
        }

        /* And we need to make sure it's really a directory.
         */
        if (    stat (c->job_defaults.save_to, &st)
             || !S_ISDIR (st.st_mode)) {
            error (
"``output'' option is not a valid directory!  Defaulting to home");
            strcpy (c->job_defaults.save_to, home_directory ());
        }
    }
}


static void set_display (Request *r, ServerConfig *c, const char *value)
{
    if (r) r->display = value;
    if (c) info ("Cannot set ``display'' option on dockapp");
}


static void set_overwrite (Request *r, ServerConfig *c)
{
    if (r) r->overwrite = 1;
    if (c) c->job_defaults.overwrite = 1;
}


static void set_continue (Request *r, ServerConfig *c)
{
    if (r) r->continue_from = 1;
    if (c) c->job_defaults.continue_from = 1;
}


static void set_proxy (Request *r, ServerConfig *c, const char *value)
{
    if (r) r->proxy = value;
    if (c) STRCPY_TO_ARRAY (c->job_defaults.proxy, value);
}


static void set_follow (Request *r, ServerConfig *c, const char *value)
{
    if (r) r->follow = atoi (value);
    if (c) c->job_defaults.follow = atoi (value);
}


static void set_user_agent (Request *r, ServerConfig *c, const char *v)
{
    if (r) r->user_agent = v;
    if (c) STRCPY_TO_ARRAY (c->job_defaults.user_agent, v);
}


static void set_ascii (Request *r, ServerConfig *c)
{
    if (r) r->use_ascii = 1;
    if (c) c->job_defaults.use_ascii = 1;
}


static void set_referer (Request *r, ServerConfig *c, const char *v)
{
    if (r) r->referer = v;
    if (c) STRCPY_TO_ARRAY (c->job_defaults.referer, v);
}


static void set_headers (Request *r, ServerConfig *c)
{
    if (r) r->include = 1;
    if (c) c->job_defaults.include = 1;
}


static void set_interface (Request *r, ServerConfig *c, const char *v)
{
    if (r) r->interface = v;
    if (c) STRCPY_TO_ARRAY (c->job_defaults.interface, v);
}


static void set_proxy_auth (Request *r, ServerConfig *c, const char *v)
{
    if (r) r->proxy_auth = v;
    if (c) STRCPY_TO_ARRAY (c->job_defaults.proxy_auth, v);
}


static void set_auth (Request *r, ServerConfig *c, const char *v)
{
    if (r) r->auth = v;
    if (c) STRCPY_TO_ARRAY (c->job_defaults.auth, v);
}


/***********************************************************************
 * load_cmdline(): Parse options from the command line.
 */
static int load_cmdline (int argc, char **argv,
                         Request *req, ServerConfig *cfg)
{
    int o;
    static const char shortopts[] = {
#define no
#define yes         , ':'
#define O(s,l,a,t) s a,
#include "config.def"
#undef O
#undef no
#undef yes
        0
    };

    static const struct option longopts[] = {
#define no no_argument
#define yes required_argument
#define O(s,l,a,t) { #l, a, 0, s },
#include "config.def"
#undef O
#undef yes
#undef no
        { 0, 0, 0, 0 }
    };

    while ((o = getopt_long (argc, argv, shortopts, longopts, 0))
            != EOF) {
        switch (o) {
            default:
                return 1;

#define yes , optarg
#define no
#define O(s,l,a,t) \
            case optchar_##l: \
                set_##l (req, cfg a); \
                break;
#include "config.def"
#undef O
#undef no
#undef yes
        }
    }

    if (optind < argc) {
        if (req) {
            if (strlen (argv[optind]) > MAXURL) {
                error ("URL too long!");
            } else {
                req->source_url = argv[optind];
            }
            ++optind;
        }

        if (cfg) {
            if (strcasecmp (argv[optind], "dock") == 0) {
                /* That's part of the syntax... ignore. */
                ++optind;
            }
        }
    }

    if (optind < argc) {
        error ("Extra argument: '%s'", argv[optind]);
    }

    return 0;
}


static void read_rcfile (FILE *rcfp, ServerConfig *cfg)
{
    char line[MAXRCLINELEN];

    while (fgets (line, sizeof line, rcfp)) {
        char *name = 0;
        char *value = 0;
        char *value_end = 0;
        char *tictactoe = strchr (line, '#');

        if (tictactoe) {
            *tictactoe = '\0';
        }

        name = line;                        /* locate name: */
        while (*name && isspace (*name))    /* skip leading ws */
            ++name;
        if (!*name) {                       /* no name? skip line */
            continue;
        }
        value = name;                       /* locate value: */
        while (*value && !isspace (*value)) /* skip name */
            ++value;
        if (*value) {                       /* not eol: look for val */
            *value++ = '\0';                /* terminate name */
            while (*value && isspace (*value))  /* skip dividing ws */
                ++value;
            value_end = value + strlen (value); /* right-trim */
            --value_end;
            while (value_end > value && isspace (*value_end)) {
                *value_end-- = 0;
            }
        }


#       define ARG_yes(NAM) \
            if (!*value) { \
                error ("Keyword '" #NAM "' in config file is missing " \
                       "its required argument"); \
            } else { \
                debug ("set " #NAM " (%s)", value); \
                set_##NAM (0, cfg, value); \
            }

#       define ARG_no(NAM) \
            if (*value) { \
                error ("Keyword '" #NAM "' in config file has an "\
                       "extra argument: '%s'", value); \
            } else { \
                debug ("set " #NAM " <no value>"); \
                set_##NAM (0, cfg); \
            }

#       define O(s,l,a,t) \
            if (strcasecmp (name, #l) == 0) { \
                ARG_##a (l) \
            } else

#       include "config.def"

#       undef O
#       undef ARG_yes
#       undef ARG_no
                
            error ("Unknown keyword in config file: %s", name);
    }
}


static void load_rcfile (ServerConfig *cfg)
{
    char rcfile[MAXPATHLEN + 1];
    static const char *rcfile_base = "/.wmgetrc";
    FILE *rcfp = 0;

    strcpy (rcfile, home_directory ());
    if (strlen (rcfile) + strlen (rcfile_base) >= sizeof rcfile) {
        error ("Your home directory name is too long!");
        return;
    }

    strcat (rcfile, rcfile_base);

    if ((rcfp = fopen (rcfile, "rt"))) {
        read_rcfile (rcfp, cfg);
        fclose (rcfp);
    } else {
        /* rcfiles are fully optional... */
        debug_sys ("Could not open rcfile '%s'", rcfile);
    }
}


void config_server (int argc, char **argv, ServerConfig *cfg)
{
    /* Default job options: These take effect unless overridden by
     * server configuration or per-job configuration.
     */
    cfg->job_defaults.display[0] = 0;
    STRCPY_TO_ARRAY (cfg->job_defaults.save_to, home_directory ());
    cfg->job_defaults.overwrite = 0;
    cfg->job_defaults.continue_from = 0;
    cfg->job_defaults.proxy[0] = 0;
    cfg->job_defaults.follow = 5;
    STRCPY_TO_ARRAY (cfg->job_defaults.user_agent, DEFAULT_USER_AGENT);

    cfg->job_defaults.use_ascii = 0;
    cfg->job_defaults.referer[0] = 0;
    cfg->job_defaults.include = 0;
    cfg->job_defaults.interface[0] = 0;
    cfg->job_defaults.proxy_auth[0] = 0;
    cfg->job_defaults.auth[0] = 0;

    load_rcfile (cfg);
    load_cmdline (argc, argv, 0, cfg);
}


void config_request (int argc, char **argv, Request *req)
{
    clear_request (req);

    load_cmdline (argc, argv, req, 0);
}






