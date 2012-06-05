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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdio.h>
#include <unistd.h>
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
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <pcre.h>

#include "wmweather+.h"
#include "metar.h"
#include "warnings.h"
#include "download.h"
#include "convert.h"
#include "die.h"
#include "sunzenith.h"
#include "moon.h"
#include "subst.h"

/* Important variables */
static time_t metar_time=0;

static char *metar_newfile=NULL;
static char *metar_file=NULL;
static char *metar_req[2]={ NULL, NULL };

struct current_weather current;

/* Regular Expressions */
static pcre *station_time;
static pcre *wind;
static pcre *weather;
static pcre *vis[4];
static pcre *temp;
static pcre *pressure;
static int ovecsize;

/* prototypes */
static int parse_metar(char *file);

/* functions */

static void reset_current(struct current_weather *c){
    c->last_update=time(NULL);
    c->month=0;
    c->date=-1;
    c->time=-1;
    c->temp=999;
    c->rh=-1;
    c->winddir=-1;
    c->windspeed=-1;
    c->pressure=-1;
    c->heatindex=999;
    c->windchill=999;
    c->sky=-1;
    c->vis=7;
    c->obs=0;
    c->frz=0;
    c->snow=0;
    c->rain=0;
    c->tstorm=0;
    c->moon=NAN;
}

#define compile(var, re) \
    var=pcre_compile(re, 0, (const char **)&e, &i, NULL); \
    if(var==NULL) die("init_metar PCRE error: %s at %i", e, i); \
    pcre_fullinfo(var, NULL, PCRE_INFO_CAPTURECOUNT, &i); \
    if(i>ovecsize) ovecsize=i;

void init_metar(void){
    int i;
    char *e;
    struct subst_val subs[]={
        { 's', STRING, &metar_station },
        { 0, 0, 0 }
    };
    
    snprintf(bigbuf, BIGBUF_LEN, "%s.metar.txt", metar_station);
    metar_file=get_pid_filename(bigbuf);
    snprintf(bigbuf, BIGBUF_LEN, "%s.new-metar.txt", metar_station);
    metar_newfile=get_pid_filename(bigbuf);

    if((metar_req[0]=subst(metar_uri, subs))==NULL) die("init_metar");
    if(metar_post!=NULL && (metar_req[1]=subst(metar_post, subs))==NULL) die("init_metar");

    metar_time=0;

    ovecsize=0;

    strncpy(bigbuf, metar_station, BIGBUF_LEN-25);
    bigbuf[BIGBUF_LEN-25]='\0';
    strcat(bigbuf, " ((?:\\d\\d)?)(\\d\\d\\d\\d)Z( .* )");
    compile(station_time, bigbuf);
    compile(wind, " (VRB|\\d\\d\\d)(\\d\\d\\d?)(?:G\\d\\d\\d?)?(KT|MPS|KMH)((?: \\d\\d\\dV\\d\\d\\d)?) ");
    compile(weather, " ((?:-|\\+|VC)?)((?:MI|PR|BC|DR|BL|SH|TS|FZ)?)((?:DZ|RA|SN|SG|IC|PE|PL|GR|GS|UP){0,3})((?:BR|FG|FU|VA|DU|SA|HZ|PY)?)((?:PO|SQ|FC|SS|DS)?)\\b");
    compile(vis[0], " (\\d+)SM ");
    compile(vis[1], " (\\d+)/(\\d+)SM ");
    compile(vis[2], " (\\d+) (\\d+)/(\\d+)SM ");
    compile(vis[3], " (\\d{4})[NS]?[EW]? ");
    compile(temp, " (M?\\d\\d\\d?)/((?:M?\\d\\d\\d?)?) ");
    compile(pressure, " ([AQ])(\\d\\d\\d\\d) ");

    ovecsize=(ovecsize+1)*3;

    /* Remove stale file */
    unlink(metar_file);
    unlink(metar_newfile);
    reset_current(&current);
    current.last_update = 0; // This was not a real "update", just an init
}
#undef compile

static void metar_callback(char *filename, void *v){
    struct stat statbuf;

    if(stat(metar_newfile, &statbuf)>=0){
        if(S_ISREG(statbuf.st_mode) && statbuf.st_size!=0
           && parse_metar(metar_newfile)){
            rename(metar_newfile, metar_file);
        } else {
            unlink(metar_newfile);
            if(!parse_metar(metar_file)) reset_current(&current);
        }
    }

    update_warnings(v!=NULL);
}

void metar_cleanup(void){
    unlink(metar_newfile);
    unlink(metar_file);
}

void update_metar(int force){
    time_t t;
    
    t=time(NULL)/60;
    if(!force && metar_time>t) return;

    metar_time=t+15;
    download_file(metar_newfile, metar_req[0], metar_req[1], force?DOWNLOAD_KILL_OTHER_REQUESTS:0, metar_callback, force?"":NULL);
}


#define get_substr(n, c) \
    if(pcre_get_substring(s, ovector, ovalue, n, (const char **)&c)<0){ pcre_free_substring(s); return 0; }

static int parse_metar(char *file){
    FILE *fp;
    char *s, *c;
    int ovector[ovecsize];
    int ovalue;
    int len;
    float f;
    int i, j;

    reset_current(&current);
    if((fp=fopen(file, "r"))==NULL) return 0;
    len=fread(bigbuf, sizeof(char), BIGBUF_LEN-2, fp);
    fclose(fp);
    if(len<1) return 0;
    for(i=0; i<len; i++){
        if(isspace(bigbuf[i])) bigbuf[i]=' ';
    }
    c=strstr(bigbuf, " RMK");
    if(c!=NULL) *(c+1)='\0';
    c=strstr(bigbuf, " TEMPO");
    s=strstr(bigbuf, " BECMG");
    if(c!=NULL) *(c+1)='\0';
    if(s!=NULL) *(s+1)='\0';
    /* XXX: parse trend forecast data? */

    len=strlen(bigbuf);
    if(bigbuf[len-1]!=' '){
        bigbuf[len++]=' ';
        bigbuf[len]='\0';
    }
    
    /* Look for something like a METAR coded report */
    ovalue=pcre_exec(station_time, NULL, bigbuf, len, 0, 0, ovector, ovecsize);
    if(ovalue<=0) return 0;
    if(pcre_get_substring(bigbuf, ovector, ovalue, 1, (const char **)&c)<0) return 0;
    if(c[0]!='\0') current.date=atoi(c);
    pcre_free_substring(c);
    if(pcre_get_substring(bigbuf, ovector, ovalue, 2, (const char **)&c)<0) return 0;
    current.time=atoi(c);
    pcre_free_substring(c);

    /* Chop off extraneous stuff */
    if(pcre_get_substring(bigbuf, ovector, ovalue, 3, (const char **)&s)<0) return 0;

    /* windspeed, winddir */
    ovalue=pcre_exec(wind, NULL, s, len, 0, 0, ovector, ovecsize);
    if(ovalue>0){
        get_substr(4, c);
        if(c[0]!='\0'){
            current.winddir=0;
        } else {
            pcre_free_substring(c);
            get_substr(1, c);
            if(c[0]=='V') current.winddir=0;
            else current.winddir=((int)((atoi(c)+11.25)/22.5))%16+1;
        }
        pcre_free_substring(c);
        get_substr(2, c);
        current.windspeed=atoi(c);
        pcre_free_substring(c);
        get_substr(3, c);
        if(c[0]=='M'){ /* MPS */
            current.windspeed=mps2knots(current.windspeed);
        } else if(c[0]=='K' && c[1]=='M'){ /* KMH */
            current.windspeed=kph2knots(current.windspeed);
        }
    }

    /* vis */
    f=99;
    c=strstr(s, " M1/4SM ");
    if(c!=NULL){
        f=0;
        goto wind_done;
    }
    ovalue=pcre_exec(vis[2], NULL, s, len, 0, 0, ovector, ovecsize);
    if(ovalue>0){
        get_substr(2, c);
        i=atoi(c);
        pcre_free_substring(c);
        get_substr(3, c);
        j=atoi(c);
        pcre_free_substring(c);
        get_substr(1, c);
        f=atoi(c)+(float)i/j;
        pcre_free_substring(c);
        goto wind_done;
    }
    ovalue=pcre_exec(vis[1], NULL, s, len, 0, 0, ovector, ovecsize);
    if(ovalue>0){
        get_substr(2, c);
        i=atoi(c);
        pcre_free_substring(c);
        get_substr(1, c);
        f=(float)atoi(c)/i;
        pcre_free_substring(c);
        goto wind_done;
    }
    ovalue=pcre_exec(vis[0], NULL, s, len, 0, 0, ovector, ovecsize);
    if(ovalue>0){
        get_substr(1, c);
        f=atoi(c);
        pcre_free_substring(c);
        goto wind_done;
    }
    c=strstr(s, " CAVOK ");
    if(c!=NULL){
        f=99;
        current.sky=0;
        goto wind_done;
    }
    ovalue=pcre_exec(vis[3], NULL, s, len, 0, 0, ovector, ovecsize);
    if(ovalue>0){
        get_substr(1, c);
        f=m2mi(atoi(c));
        pcre_free_substring(c);
        goto wind_done;
    }
wind_done:
    if(f<=6) current.vis=6;
    if(f<=5) current.vis=5;
    if(f<3) current.vis=4;
    if(f<1) current.vis=3;
    if(f<=.5) current.vis=2;
    if(f<=.25) current.vis=1;

    /* temp, rh */
    ovalue=pcre_exec(temp, NULL, s, len, 0, 0, ovector, ovecsize);
    if(ovalue>0){
        get_substr(1, c);
        if(c[0]=='M') c[0]='-';
        current.temp=atoi(c);
        pcre_free_substring(c);
        get_substr(2, c);
        if(c[0]!='\0'){
            if(c[0]=='M') c[0]='-';
            current.rh=rh_C(current.temp, atoi(c));
        }
        pcre_free_substring(c);
    }

    /* pressure */
    ovalue=pcre_exec(pressure, NULL, s, len, 0, 0, ovector, ovecsize);
    if(ovalue>0){
        get_substr(2, c);
        i=atoi(c);
        pcre_free_substring(c);
        get_substr(1, c);
        if(c[0]=='Q'){
            current.pressure=hPa2inHg(i);
        } else {
            current.pressure=i/100.0;
        }
        pcre_free_substring(c);
    }

    /* sky */
    if(strstr(s, " SKC")!=NULL || strstr(s, " CLR")!=NULL) current.sky=0;
    if(strstr(s, " FEW")!=NULL) current.sky=1;
    if(strstr(s, " SCT")!=NULL) current.sky=2;
    if(strstr(s, " BKN")!=NULL) current.sky=3;
    if(strstr(s, " OVC")!=NULL || strstr(s, " VV")!=NULL) current.sky=4;

    /* obs, frz, snow, rain, tstorm */
    /* There can be multiple weather chunks, so we while loop */
    j=0;
    while((ovalue=pcre_exec(weather, NULL, s, len, j, 0, ovector, ovecsize))>0){{
        char *in, *de, *pp, *ob, *ot;

        j=ovector[0]+1;
        get_substr(0, c);
        i=(c[1]=='\0');
        pcre_free_substring(c);
        if(i) continue;
        

        get_substr(1, in);
        get_substr(2, de);
        get_substr(3, pp);
        get_substr(4, ob);
        get_substr(5, ot);

#define IN(haystack, needle) ((needle[0]=='\0')?0:strstr(haystack, needle))
        if(current.obs<1 && strcmp(de, "FZ") && IN("BR|FG", ob))
            current.obs=1;
        if(current.obs<2 && IN("FU|VA|DU|SA|HZ|PY", ob))
            current.obs=2;
        if(current.obs<3 && IN("PO|SS|DS", ot))
            current.obs=3;
        if(current.obs<3 && IN("DR|BL", de)
           && (strstr(pp, "SN") || IN("DU|SA|PY", ob)))
            current.obs=3;
        if(!strcmp(ot, "FC")){
            current.sky=5;
            current.obs=99;
            current.vis=7;
        }
#undef IN

        i=66;
        if(in[0]=='-' || in[0]=='V') i=33;
        if(in[0]=='+') i=99;
        if(!strcmp(de, "SH")) i=33;
        if(current.frz<i
           && ((!strcmp(de, "FZ") && (strstr(pp, "DZ") || strstr(pp, "RA")))
               || strstr(pp, "IC") || strstr(pp, "PE") || strstr(pp, "PL")
               || strstr(pp, "GR") || strstr(pp, "GS")))
                current.frz=i;
        if(current.snow<i && strcmp(de, "BL")
           && (strstr(pp, "SN") || strstr(pp, "SG")))
            current.snow=i;
        if(current.rain<i && (strstr(pp, "UP")
                              || (strcmp(de, "FZ")
                                  && (strstr(pp, "DZ") || strstr(pp, "RA")))))
            current.rain=i;
        if(current.tstorm<i && !strcmp(de, "TS"))
            current.tstorm=i;

        pcre_free_substring(in);
        pcre_free_substring(de);
        pcre_free_substring(pp);
        pcre_free_substring(ob);
        pcre_free_substring(ot);
    }}
    if(current.obs==99) current.obs=0;
    
    pcre_free_substring(s); /* Done parsing! Just a few final calculations... */

    current.heatindex=heatindex_C(current.temp, current.rh);
    current.windchill=windchill_C(current.temp, current.windspeed);

    /* Figure out the proper month... */
    {
        int mon, day, year, time2; /* holds UTC */
        int y; /* with current.*, holds local time */
        time_t t=time(NULL);
        struct tm *tm=gmtime(&t);
        current.month=tm->tm_mon+1;
        if(tm->tm_mday<current.date) current.month--;
        if(current.month<1){ current.month+=12; tm->tm_year--; }
        y=year=tm->tm_year;
        mon=current.month;
        day=current.date;
        time2=current.time;
        current.time=utc2local((int)current.time, &current.month, &current.date, &y, NULL);
    
        if(latitude!=999 && calcSolarZenith(latitude, longitude, year, mon, day, hm2min(time2))>90)
            current.moon=calc_moon(current.month, current.date, y, current.time);
    }
    return 1;
}


#undef get_substr
