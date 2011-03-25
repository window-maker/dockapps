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
#include <sys/stat.h>

#include "wmweather+.h"
#include "forecast.h"
#include "getLine.h"
#include "convert.h"
#include "download.h"
#include "diff.h"
#include "die.h"
#include "sunzenith.h"
#include "moon.h"
#include "subst.h"

/* Important variables */
#define ETA_MAX 21
static time_t eta_time=0;
static char *eta_file=NULL;
static char *eta_newfile=NULL;
static char *eta_req[2]={ NULL, NULL };
static struct forecast forecasts[ETA_MAX];


/********* init functions ************/
static int parse_eta(char *file);

static void reset_eta(void){
    int i;
    
    for(i=0; i<ETA_MAX; i++) reset_forecast(&forecasts[i]);
}

void init_eta(void){
    char *e;
    int i;
    struct subst_val subs[]={
        { 's', STRING, &eta_station },
        { 0, 0, 0 }
    };
    
    strncpy(bigbuf, eta_station, BIGBUF_LEN-14);
    bigbuf[BIGBUF_LEN-14]='\0';
    for(e=bigbuf; *e!='\0'; e++);
    strcpy(e, ".eta.txt");
    eta_file=get_pid_filename(bigbuf);
    strcpy(e, ".new-eta.txt");
    eta_newfile=get_pid_filename(bigbuf);

    if((eta_req[0]=subst(eta_uri, subs))==NULL) die("init_eta");
    if(eta_post!=NULL && (eta_req[1]=subst(eta_post, subs))==NULL) die("init_eta");
    eta_time=0;

    /* Remove stale file */
    unlink(eta_file);
    unlink(eta_newfile);
    reset_eta();
    for(i=0; i<ETA_MAX; i++) add_forecast(&forecasts[i], "ETA", eta_station);
}


/********* download functions ************/

static void eta_callback(char *filename, void *v){
    struct stat statbuf;

    if(stat(eta_newfile, &statbuf)>=0){
        if(S_ISREG(statbuf.st_mode) && statbuf.st_size!=0
           && diff(eta_newfile, eta_file) && parse_eta(eta_newfile)){
            eta_time=find_next_time(eta_newfile, "MOS GUIDANCE", 720);
            rename(eta_newfile, eta_file);
        } else {
            unlink(eta_newfile);
            if(!parse_eta(eta_file)) reset_eta();
        }
    }
}

void eta_cleanup(void){
    if(eta_file==NULL) return;
    unlink(eta_newfile);
    unlink(eta_file);
}

void update_eta(int force){
    time_t t;

    if(eta_file==NULL) return;

    t=time(NULL)/60;
    if(!force && eta_time>t) return;

    eta_time=find_next_time(eta_file, "MOS GUIDANCE", 15);
    download_file(eta_newfile, eta_req[0], eta_req[1], force?DOWNLOAD_KILL_OTHER_REQUESTS:0, eta_callback, NULL);
}


/********* parse functions ************/

#define NEXT(s) free(s); \
    len=getLine(&s, fp); \
    if(strstr(s, "</PRE>")!=NULL) len=0;

#define DIE() return (free(s), fclose(fp), 0)
#define SPLIT(s) { \
    ID[0]=s[0]; \
    ID[1]=s[1]; \
    ID[2]=s[2]; \
    ID[3]='\0'; \
    memset(split,'\0',sizeof(split)); \
    for(n=0, c=s+4; c<s+len && n<ETA_MAX; n++, c+=3){ \
        split[n][0]=c[0]; \
        split[n][1]=c[1]; \
        split[n][2]=c[2]; \
        split[n][3]='\0'; \
    } \
}
#define ASSIGN(field) \
    for(n=0; n<ETA_MSG_MAX; n++) forecasts[n].field=atoi(split[n]);

#define ASSIGN2(field, inval) \
    for(n=0; n<ETA_MSG_MAX; n++){ \
        i=atoi(split[n]); \
        if(i!=inval) forecasts[n].field=i; \
    }

static int parse_eta(char *file){
    int ETA_MSG_MAX = ETA_MAX;
    FILE *fp;
    char *s, *c;
    int len;
    int mon, day;
    int h, i=0, j, k, m, n, x, y, z;
    char ID[4];
    char split[ETA_MAX][4];

    reset_eta();
    if((fp=fopen(file, "r"))==NULL) return 0;

    /* Look for something like an ETA coded forecast */
    c=NULL;
    while(!feof(fp)){
        len=getLine(&s, fp);
        if((c=strstr(s, "MOS GUIDANCE"))!=NULL) break;
        free(s);
    }
    if(c==NULL) return (fclose(fp), 0);
    c=strchr(c, '/');
    if(c==NULL || !isdigit(*(c-1)) || !isdigit(*(c+1))) DIE();
    m=atoi(c-2);
    c=strchr(c+1, '/');
    if(c==NULL || !isdigit(*(c-1)) || !isdigit(*(c+1))) DIE();
    y=atoi(c+1)-1900;

    /* get first date */
    NEXT(s);
    if(len<10) DIE();
    if(strncmp(s, "DT ", 3)) DIE();
    mon=13;
    c=s;
    i=4;
    while(mon>12){
        c=strchr(c+1, '/');
        if(c==NULL) DIE();
        for(mon=1; mon<=12; mon++){
            if(!strncmp(c+1, monthnames[mon], 3) && isspace(*(c+4))) break;
            if(!strncmp(c+1, monthnames2[mon], 4) && isspace(*(c+5))){
                i=5;
                break;
            }
        }
    }
    day=atoi(c+i);
    if(day<1) DIE();
    if(c>s+4) day--;
    if(mon<m) y++;

    NEXT(s);
    if(len<10) DIE();
    if(strncmp(s, "HR ", 3)) DIE();
    x=day;
    m=mon;
    SPLIT(s);
    for(n=0; n<ETA_MAX; n++){
        if(split[n][0]=='\0'){
            ETA_MSG_MAX = n;
            break;
        }
        i=atoi(split[n]);
        if(i==0){
            x++;
            fix_date(&mon, &x, &y, NULL);
        }
        m=mon;
        j=x;
        z=y;
        h=utc2local(i*100, &m, &j, &z, &k)/100;
        forecasts[n].month=m;
        forecasts[n].day=j;
        forecasts[n].year=z;
        forecasts[n].hour=h;
        forecasts[n].wday=k;
        if(latitude!=999 && calcSolarZenith(latitude, longitude, y, mon, x, i*60)>90)
            forecasts[n].moon=calc_moon(m, j, z, h*100);
    }

    while(1){
        NEXT(s);
        if(len<=10) break;
        SPLIT(s);

        if(!strcmp(ID, "X/N")) j=1;
        else if(!strcmp(ID, "N/X")) j=2;
        else j=0;
        if(j!=0){
            for(n=0; n<ETA_MSG_MAX; n++){
                if(!isdigit(split[n][2])) continue;
                i=atoi(split[n]);
                k=day+(j>>1);
                for(m=0; m<ETA_MSG_MAX; m++){
                    if((j&1)==1 &&
                       ((forecasts[m].day==k-1 && forecasts[m].hour>=19)
                        || (forecasts[m].day==k && forecasts[m].hour<19)))
                        forecasts[m].high=i;
                    if((j&1)==0 &&
                       ((forecasts[m].day==k-1 && forecasts[m].hour>=8)
                        || (forecasts[m].day==k && forecasts[m].hour<8)))
                        forecasts[m].low=i;
                }
                j++;
            }
            continue;
        }
        if(!strcmp(ID, "TMP")){
            ASSIGN(temp);
            continue;
        }
        if(!strcmp(ID, "DPT")){
            ASSIGN(dewpt);
            continue;
        }
        if(!strcmp(ID, "WDR")){
            for(n=0; n<ETA_MSG_MAX; n++){
                i=atoi(split[n]);
                if(i==99) forecasts[n].winddir=0;
                else forecasts[n].winddir=((int)((i+1.125)/2.25))%16+1;
            }
            continue;
        }
        if(!strcmp(ID, "WSP")){
            ASSIGN2(windspeed, 99);
            continue;
        }
        if(!strcmp(ID, "P06")){
            for(m=0; m<ETA_MSG_MAX; m++){
                if(!isdigit(split[m][2])) continue;
                i=atoi(split[m]);
                if(i!=999){
                    forecasts[m].pcp_total=i;
                    /* ETA_MSG_MAX-2 because the last 2
                     * are already 6-hour intervals */
                    if(m>0 && m<ETA_MSG_MAX-2) forecasts[m-1].pcp_total=i;
                }
            }
            continue;
        }
        if(!strcmp(ID, "T06")){
            for(m=1; m<ETA_MSG_MAX; m+=2){
                if(!isdigit(split[m][2])) continue;
                i=atoi(split[m]); if(i==999) i=0;
                j=atoi(split[m+1]+1); if(j==99) j=0;
                j=i*j/100;
                forecasts[m].tstorm=forecasts[m+1].tstorm=i;
                forecasts[m].svtstorm=forecasts[m+1].svtstorm=j;
            }
            continue;
        }
        if(!strcmp(ID, "Q06")){
            for(m=0; m<ETA_MSG_MAX; m++){
                if(!isdigit(split[m][2])) continue;
                i=atoi(split[m]);
                if(i!=999){
                    forecasts[m].precipamt=i;
                    /* ETA_MSG_MAX-2 because the last 2
                     * are already 6-hour intervals */
                    if(m>0 && m<ETA_MSG_MAX-2) forecasts[m-1].precipamt=i;
                }
            }
            continue;
        }
        if(!strcmp(ID, "CLD")){
            for(m=0; m<ETA_MSG_MAX; m++){
                if(split[m][1]=='C') forecasts[m].sky=0;
                if(split[m][1]=='F') forecasts[m].sky=1;
                if(split[m][1]=='S') forecasts[m].sky=2;
                if(split[m][1]=='B') forecasts[m].sky=3;
                if(split[m][1]=='O') forecasts[m].sky=4;
            }
            continue;
        }
        if(!strcmp(ID, "VIS")){
            ASSIGN2(vis, 9);
            continue;
        }
        if(!strcmp(ID, "OBV")){
            for(m=0; m<ETA_MSG_MAX; m++){
                if(split[m][2]=='N') forecasts[m].obs=0;
                if(split[m][2]=='R' || split[m][2]=='G') forecasts[m].obs=1;
                if(split[m][2]=='Z') forecasts[m].obs=2;
                if(split[m][2]=='L') forecasts[m].obs=3;
            }
            continue;
        }
        if(!strcmp(ID, "POZ")){
            ASSIGN2(frz, 999);
            continue;
        }
        if(!strcmp(ID, "POS")){
            ASSIGN2(snow, 999);
            continue;
        }
    }
    free(s);
    fclose(fp);

    for(m=0; m<ETA_MSG_MAX; m++){
        forecasts[m].rh=rh_F(forecasts[m].temp, forecasts[m].dewpt);
        forecasts[m].heatindex=heatindex_F(forecasts[m].temp, forecasts[m].rh);
        forecasts[m].windchill=windchill_F(forecasts[m].temp, forecasts[m].windspeed);
        forecasts[m].rain=93-forecasts[m].frz-forecasts[m].snow;
        forecasts[m].rain=forecasts[m].rain*forecasts[m].pcp_total/93;
        forecasts[m].snow=forecasts[m].snow*forecasts[m].pcp_total/93;
        forecasts[m].frz=forecasts[m].frz*forecasts[m].pcp_total/93;
    }
    
    return 1;
}
#undef NEXT
#undef DIE
#undef SPLIT
#undef ASSIGN
#undef ASSIGN2
