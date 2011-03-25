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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>

#include "wmweather+.h"
#include "forecast.h"
#include "getLine.h"
#include "convert.h"
#include "download.h"
#include "diff.h"
#include "die.h"
#include "subst.h"

/* Important variables */
#define MRF_MAX 7
static time_t mrf_time=0;
static char *mrf_file=NULL;
static char *mrf_newfile=NULL;
static char *mrf_req[2]={ NULL, NULL };
static struct forecast forecasts[MRF_MAX];


/********* init functions ************/
static int parse_mrf(char *file);

static void reset_mrf(void){
    int i;
    
    for(i=0; i<MRF_MAX; i++) reset_forecast(&forecasts[i]);
}

void init_mrf(void){
    int i;
    char *e;
    struct subst_val subs[]={
        { 's', STRING, &mrf_station },
        { 0, 0, 0 }
    };
    
    strncpy(bigbuf, mrf_station, BIGBUF_LEN-14);
    bigbuf[BIGBUF_LEN-14]='\0';
    for(e=bigbuf; *e!='\0'; e++);
    strcpy(e, ".mrf.txt");
    mrf_file=get_pid_filename(bigbuf);
    strcpy(e, ".new-mrf.txt");
    mrf_newfile=get_pid_filename(bigbuf);

    if((mrf_req[0]=subst(mrf_uri, subs))==NULL) die("init_mrf");
    if(mrf_post!=NULL && (mrf_req[1]=subst(mrf_post, subs))==NULL) die("init_mrf");
    mrf_time=0;

    /* Remove stale file */
    unlink(mrf_file);
    unlink(mrf_newfile);
    reset_mrf();
    for(i=0; i<MRF_MAX; i++) add_forecast(&forecasts[i], "MRF", mrf_station);
}

static void mrf_callback(char *filename, void *v){
    struct stat statbuf;

    if(stat(mrf_newfile, &statbuf)>=0){
        if(S_ISREG(statbuf.st_mode) && statbuf.st_size!=0
           && diff(mrf_newfile, mrf_file) && parse_mrf(mrf_newfile)){
            mrf_time=find_next_time(mrf_newfile, "MOS GUIDANCE", 1440);
            rename(mrf_newfile, mrf_file);
        } else {
            unlink(mrf_newfile);
            if(!parse_mrf(mrf_file)) reset_mrf();
        }
    }
}

void mrf_cleanup(void){
    if(mrf_file==NULL) return;
    unlink(mrf_newfile);
    unlink(mrf_file);
}

void update_mrf(int force){
    time_t t;

    if(mrf_file==NULL) return;

    t=time(NULL)/60;
    if(!force && mrf_time>t) return;

    mrf_time=find_next_time(mrf_file, "MOS GUIDANCE", 15);
    download_file(mrf_newfile, mrf_req[0], mrf_req[1], 0, mrf_callback, NULL);
}


#define NEXT(s) free(s); \
    len=getLine(&s, fp); \
    if(strstr(s, "</PRE>")!=NULL) len=0;

#define DIE() return (free(s), fclose(fp), 0)
#define INT(c) (tmp[0]=*c, tmp[1]=*(c+1), tmp[2]=*(c+2), tmp[3]=0, atoi(tmp))

static int parse_mrf(char *file){
    FILE *fp;
    char *s, *c;
    int len;
    int mon, day;
    int i, j, m, x, y;
    int flag;
    char tmp[4]={0, 0, 0, 0};

    flag=0;
    reset_mrf();
    if((fp=fopen(file, "r"))==NULL) return 0;

    /* Look for something like an MRF coded forecast */
    c=NULL;
    while(!feof(fp)){
        len=getLine(&s, fp);
        if((c=strstr(s, "MOS GUIDANCE"))!=NULL) break;
        free(s);
    }
    if(c==NULL) return (fclose(fp), 0);
    c=strchr(c, '/');
    if(c==NULL || !isdigit(*(c-1)) || !isdigit(*(c+1))) DIE();
    mon=atoi(c-2);
    x=atoi(c+1);
    if(mon<1 || mon>12 || x<1 || x>31) DIE();
    c=strchr(c+1, '/');
    if(c==NULL || !isdigit(*(c-1)) || !isdigit(*(c+1))) DIE();
    y=atoi(c+1)-1900;

    NEXT(s);
    if(len<10) DIE();
    if(strncmp(s, "FHR", 3)) DIE();

    NEXT(s);
    if(len<10) DIE();
    for(i=0; i<7; i++){
        if(!strncmp(s, wdaynames[i], 3)) break;
    }
    if(i>=7) DIE();
    day=atoi(s+4);
    if(x>25 && day<5) mon++;
    if(x<5 && day>25) mon--;
    for(m=0; m<MRF_MAX; m++){
        day++;
        i++; i%=7;
        fix_date(&mon, &day, &y, &j);
        if(j!=i){
            warn("Something wicked happened with the mrf_parse dates...");
            DIE();
        }
        forecasts[m].month=mon;
        forecasts[m].day=day;
        forecasts[m].year=y;
        forecasts[m].wday=i;
        forecasts[m].hour=-1;
    }
    
    while(1){
        NEXT(s);
        if(len<=10) break;

        if(!strncmp(s, "X/N", 3)){
            for(c=s+12, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                forecasts[m].high=INT(c);
                if(c+4<s+len) forecasts[m].low=INT((c+4));
            }
            continue;
        }
        if(!strncmp(s, "TMP", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=INT(c); j=INT((c+4));
                if(i!=999 && j!=999) forecasts[m].temp=(i+j)/2;
                else if(i!=999) forecasts[m].temp=i;
                else if(j!=999) forecasts[m].temp=j;
            }
            continue;
        }
        if(!strncmp(s, "DPT", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=INT(c); j=INT((c+4));
                if(i!=999 && j!=999) forecasts[m].dewpt=(i+j)/2;
                else if(i!=999) forecasts[m].dewpt=i;
                else if(j!=999) forecasts[m].dewpt=j;
            }
            continue;
        }
        if(!strncmp(s, "WND", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=INT(c); j=INT((c+4));
                if(i!=999 && j!=999) forecasts[m].windspeed=(i+j)/2;
                else if(i!=999) forecasts[m].windspeed=i;
                else if(j!=999) forecasts[m].windspeed=j;
            }
            continue;
        }
        if(!strncmp(s, "T24", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=atoi(c);
                if(i!=999) forecasts[m].tstorm=i;
            }
            continue;
        }
        if(!strncmp(s, "Q24", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=atoi(c);
                if(i!=9) forecasts[m].precipamt=i;
            }
            continue;
        }
        if(!strncmp(s, "SNW", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=atoi(c);
                if(i!=9) forecasts[m].snowamt=i;
            }
            continue;
        }
        if(!strncmp(s, "CLD", 3)){
            for(c=s+13, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                if(*c=='C') forecasts[m].sky=0;
                if(*c=='P'){
                    if(*(c-4)=='C') forecasts[m].sky=2;
                    else if(*(c-4)=='O') forecasts[m].sky=3;
                    else if(c+4<s+len && *(c+4)=='C') forecasts[m].sky=2;
                    else if(c+4<s+len && *(c+4)=='O') forecasts[m].sky=3;
                    else forecasts[m].sky=3;
                }
                if(*c=='O') forecasts[m].sky=4;
            }
            continue;
        }
        if(!strncmp(s, "PZP", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=INT(c); j=INT((c+4));
                if(i!=999 && j!=999) forecasts[m].frz=(i+j)/2;
                else if(i!=999) forecasts[m].frz=i;
                else if(j!=999) forecasts[m].frz=j;
            }
            continue;
        }
        if(!strncmp(s, "PSN", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=INT(c); j=INT((c+4));
                if(i!=999 && j!=999) forecasts[m].snow=(i+j)/2;
                else if(i!=999) forecasts[m].snow=i;
                else if(j!=999) forecasts[m].snow=j;
            }
            continue;
        }
        if(!strncmp(s, "PRS", 3)){
            /* stick "rain & snow" prob into rain for later */
            flag=1;
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=INT(c); j=INT((c+4));
                if(i!=999 && j!=999) forecasts[m].rain=(i+j)/2;
                else if(i!=999) forecasts[m].rain=i;
                else if(j!=999) forecasts[m].rain=j;
            }
            continue;
        }
        if(!strncmp(s, "P24", 3)){
            for(c=s+8, m=0; c<s+len && m<MRF_MAX; c+=8, m++){
                i=atoi(c);
                if(i!=999) forecasts[m].pcp_total=i;
            }
            continue;
        }
    }
    free(s);
    fclose(fp);

    for(m=0; m<MRF_MAX; m++){
        forecasts[m].rh=rh_F(forecasts[m].temp, forecasts[m].dewpt);
        forecasts[m].heatindex=heatindex_F(forecasts[m].temp, forecasts[m].rh);
        forecasts[m].windchill=windchill_F(forecasts[m].temp, forecasts[m].windspeed);
        /* real rain = 100 - frz - snow
         * real snow = snow + "rain & snow"
         */
        i=100-forecasts[m].frz-forecasts[m].snow;
        forecasts[m].snow+=forecasts[m].rain;
        forecasts[m].rain=i;
        forecasts[m].rain=forecasts[m].rain*forecasts[m].pcp_total/100;
        forecasts[m].snow=forecasts[m].snow*forecasts[m].pcp_total/100;
        forecasts[m].frz=forecasts[m].frz*forecasts[m].pcp_total/100;

        /* These aren't really useful here... */
        forecasts[m].temp=999;
        forecasts[m].dewpt=999;
    }
    
    return 1;
}
#undef NEXT
#undef DIE
#undef INT
