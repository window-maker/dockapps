#include "config.h"

/*  Copyright (C) 2002  Brad Jorsch <anomie@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#if TM_IN_SYS_TIME
# if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  if HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   include <time.h>
#  endif
# endif
#else
#include <time.h>
#endif
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <pcre.h>

#include "wmweather+.h"
#include "download.h"
#include "getLine.h"
#include "diff.h"
#include "die.h"
#include "subst.h"

/* Important variables */
static char *warning_filename1=NULL;
static char *warning_endptr1=NULL;

static char *filenames[]={
    "tornado", "flash_flood>warning", "flash_flood>watch",
    "flash_flood>statement", "flood>warning", "flood>coastal",
    "flood>statement", "severe_weather_stmt", "special_weather_stmt",
    "thunderstorm", "special_marine", "urgent_weather_message", "non_precip",
    "fire_weather", "lake_shore", NULL
};
static char **reqs[sizeof(filenames)/sizeof(*filenames)-1][2];

unsigned long current_warnings;
static unsigned long *zone_current_warnings;

/* Regular Expressions */
static pcre *expires;
static int ovecsize;

/* prototypes */
static int check_warning(char *file);


/* functions */

#define compile(var, re) \
    var=pcre_compile(re, 0, (const char **)&e, &i, NULL); \
    if(var==NULL) die("init_warnings PCRE error: %s at %i", e, i); \
    pcre_fullinfo(var, NULL, PCRE_INFO_CAPTURECOUNT, &i); \
    if(i>ovecsize) ovecsize=i;

void init_warnings(void){
    int i, j, z=0;
    char *e;
    struct subst_val subs[]={
        { 'z', STRING, NULL },
        { 'f', STRING, &bigbuf },
        { 0, 0, 0 }
    };
    
    /* Count zones, and find length of longest */
    for(i=0; warning_zones[i]!=NULL; i++){
        j=strlen(warning_zones[i]);
        if(j>z) z=j;
    }

    /* Allocate char ptrs for each filename for each zone */
    for(j=0; filenames[j]!=NULL; j++){
        reqs[j][0]=malloc(sizeof(char *)*i);
        reqs[j][1]=malloc(sizeof(char *)*i);
        if(reqs[j][0]==NULL || reqs[j][1]==NULL) die("init_warnings malloc");
    }
    zone_current_warnings=calloc(i, sizeof(*zone_current_warnings));
    if(zone_current_warnings==NULL) die("init_warnings malloc");
    
    /* Allocate filename base */
    e=get_pid_filename("");
    i=strlen(e);
    warning_filename1=malloc(i+z+32);
    if(warning_filename1==NULL)
        die("init_warnings malloc");

    strcpy(warning_filename1, e);
    free(e);
    warning_endptr1=warning_filename1+i;

    /* Setup misc vars */
    current_warnings=0;
    ovecsize=0;
    compile(expires, "Expires:(\\d+)(\\d\\d)(\\d\\d)(\\d\\d)(\\d\\d);");
    ovecsize=(ovecsize+1)*3;

    /* Remove stale files, and allocate URIs */
    for(z=0; warning_zones[z]!=NULL; z++){
        subs[0].val=warning_zones+z;
        for(i=0; filenames[i]!=NULL; i++){
            sprintf(warning_endptr1, "%s.%s.txt", warning_zones[z], filenames[i]);
            unlink(warning_filename1);
            sprintf(warning_endptr1, "%s.new-%s.txt", warning_zones[z], filenames[i]);
            unlink(warning_filename1);
            strncpy(bigbuf, filenames[i], BIGBUF_LEN);
            bigbuf[BIGBUF_LEN-1]='\0';
            for(j=0; bigbuf[j]; j++){
                if(bigbuf[j]=='>') bigbuf[j]='/';
            }
            if((reqs[i][0][z]=subst(warning_uri, subs))==NULL) die("init_warning");
            reqs[i][1][z]=NULL;
            if(warning_post!=NULL && (reqs[i][1][z]=subst(warning_post, subs))==NULL) die("init_warning");
        }
    }
}
#undef compile

struct callback_data {
    int zone;
    int warning;
};

static void warning_callback(char *filename, void *v){
    struct stat statbuf;
    struct callback_data *d=(struct callback_data *)v;

    sprintf(warning_endptr1, "%s.%s.txt", warning_zones[d->zone], filenames[d->warning]);
    if(stat(filename, &statbuf)>=0){
        if(S_ISREG(statbuf.st_mode) && statbuf.st_size!=0
           && check_warning(filename)
           && diff(filename, warning_filename1)){
            current_warnings|=1<<d->warning;
            zone_current_warnings[d->zone]|=1<<d->warning;
            rename(filename, warning_filename1);
        } else {
            unlink(filename);
        }
    }
}

void warnings_cleanup(void){
    int i, z;

    if(warning_filename1==NULL) return;
    for(z=0; warning_zones[z]!=NULL; z++){
        for(i=0; filenames[i]!=NULL; i++){
            sprintf(warning_endptr1, "%s.%s.txt", warning_zones[z], filenames[i]);
            unlink(warning_filename1);
            sprintf(warning_endptr1, "%s.new-%s.txt", warning_zones[z], filenames[i]);
            unlink(warning_filename1);
        }
    }
}

void update_warnings(int force){
//    time_t t;
    struct stat statbuf;
    int i, z;
    struct callback_data *d;

    if(warning_filename1==NULL) return;
//    t=time(NULL)/60;
//    if(!force && warning_time>t) return;

//    warning_time=t+15;

    for(z=0; warning_zones[z]!=NULL; z++){
        for(i=0; filenames[i]!=NULL; i++){
            /* expire old wanrings */
            sprintf(warning_endptr1, "%s.%s.txt", warning_zones[z], filenames[i]);
            if(stat(warning_filename1, &statbuf)>=0){
                if(!S_ISREG(statbuf.st_mode) || statbuf.st_size==0
                   || !check_warning(warning_filename1)){
                    unlink(warning_filename1);
                    current_warnings&=~(1<<i);
                    zone_current_warnings[z]&=~(1<<i);
                }
            } else {
                current_warnings&=~(1<<i);
                zone_current_warnings[z]&=~(1<<i);
            }

            if((d=malloc(sizeof(*d)))==NULL) continue;
            sprintf(warning_endptr1, "%s.new-%s.txt", warning_zones[z], filenames[i]);
            d->zone=z;
            d->warning=i;
            download_file(warning_filename1, reqs[i][0][z], reqs[i][1][z], DOWNLOAD_NO_404, warning_callback, d);
        }
    }
}


#define get_substr(n, c) \
    if(pcre_get_substring(s, ovector, ovalue, n, (const char **)&c)<0){ free(s); return 0; }

static int check_warning(char *file){
    FILE *fp;
    char *s, *c;
    int len;
    int i;
    time_t t;
    struct tm *tm;
    int ovector[ovecsize];
    int ovalue;

    if((fp=fopen(file, "r"))==NULL) return 0;
    ovalue=-1;
    while((len=getLine(&s, fp))>0){
        ovalue=pcre_exec(expires, NULL, s, len, 0, 0, ovector, ovecsize);
        if(ovalue>0) break;
        free(s);
    }
    fclose(fp);
    if(ovalue<=0) return 0;

    t=time(NULL);
    tm=gmtime(&t);
    get_substr(1, c); i=atoi(c)-1900; pcre_free_substring(c);
    if(tm->tm_year<i){ free(s); return 1; }
    if(tm->tm_year>i){ free(s); return 0; }
    get_substr(2, c); i=atoi(c)-1; pcre_free_substring(c);
    if(tm->tm_mon<i){ free(s); return 1; }
    if(tm->tm_mon>i){ free(s); return 0; }
    get_substr(3, c); i=atoi(c); pcre_free_substring(c);
    if(tm->tm_mday<i){ free(s); return 1; }
    if(tm->tm_mday>i){ free(s); return 0; }
    get_substr(4, c); i=atoi(c); pcre_free_substring(c);
    if(tm->tm_hour<i){ free(s); return 1; }
    if(tm->tm_hour>i){ free(s); return 0; }
    get_substr(5, c); i=atoi(c); pcre_free_substring(c);
    if(tm->tm_min<=i){ free(s); return 1; }
    free(s); return 0;
}

void output_warnings(int all){
    FILE *fp;
    int i, z, len;
    pid_t pid;
    int pipefd[2];

    if(!all && current_warnings==0) return;

    if(pipe(pipefd)) die("output_warnings pipe creation");

    /* Fork to display the file */
    pid=fork();
    if(pid==-1){
        warn("output_warnings fork");
        return;
    }
    /* CHILD: Redirects stdin/stderr/stdout and execs the viewer */
    if(pid==0){
	close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        dup2(devnull, STDOUT_FILENO);
        execl("/bin/sh", "/bin/sh", "-c", viewer, NULL);
        die("output_warnings exec");
    }

    /* PARENT writes warnings to the pipe and returns */
    close(pipefd[0]);
    for(z=0; warning_zones[z]!=NULL; z++){
        if(!zone_current_warnings[z]) continue;
        for(i=0; filenames[i]!=NULL; i++){
            if(!all && !(zone_current_warnings[z]&(1<<i))) continue;

            sprintf(warning_endptr1, "%s.%s.txt", warning_zones[z], filenames[i]);
            if((fp=fopen(warning_filename1, "r"))==NULL) continue;
            snprintf(bigbuf, BIGBUF_LEN, "======== BEGIN %s %s ========\n", warning_zones[z], filenames[i]);
            write(pipefd[1], bigbuf, strlen(bigbuf));
            while(!feof(fp)){
                len=fread(bigbuf, sizeof(char), BIGBUF_LEN, fp);
                write(pipefd[1], bigbuf, len);
            }
            fclose(fp);
        }
        if(!all) zone_current_warnings[z]=0;
    }
    close(pipefd[1]);
    if(!all) current_warnings=0;
    return;
}
