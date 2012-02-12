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
#include <errno.h>
#include <sys/stat.h>
#include <wraster.h>

#include <X11/xpm.h>

#include "wmweather+.h"
#include "wmgeneral/wmgeneral-x11.h"
#include "wmgeneral/xpm_trans.h"
#include "download.h"
#include "radar.h"
#include "die.h"

/* Important variables */
static time_t radar_time=0;
static char *radar_file=NULL;
static char *radar_newfile=NULL;
static int cropx, cropy, cropw, croph;
static int crossx, crossy;
time_t radar_update_time;
Pixmap radar;
int do_radar_cross;

extern XpmIcon wmgen;
extern GC NormalGC;
extern int screen;
extern int d_depth;

/* prototypes */
static int parse_radar(char *file);


/* functions */

static void reset_radar(Pixmap *r){
    XCopyArea(display, wmgen.pixmap, *r, NormalGC, 124, 18, 52, 40, 0, 0);
    XCopyArea(display, wmgen.pixmap, *r, NormalGC, 108, 89, 15, 14, 18, 13);
}


static void parse_cross(void){
    char *p1, *p2;

    if(radar_cross==NULL) return;
    crossx=strtol(radar_cross, &p1, 10);
    if(crossx<0 || crossx>=52 || p1==NULL || p1==radar_cross || *p1!='x'){
        radar_cross=NULL;
        return;
    }
    crossy=strtol(++p1, &p2, 10);
    if(crossy<0 || crossy>=40 || (p2!=NULL && *p2!='\0')){
        radar_cross=NULL;
        return;
    }
}

static void parse_crop(void){
    char *p1, *p2;

    if(radar_crop==NULL) return;
    cropx=strtol(radar_crop, &p1, 10);
    if(p1==NULL || p1==radar_crop || *p1!='x'){
        radar_crop=NULL;
        return;
    }
    cropy=strtol(++p1, &p2, 10);
    if(p2==NULL || p2==p1 || *p2!='+'){
        radar_crop=NULL;
        return;
    }
    cropw=strtol(p2, &p1, 10);
    if(cropw<1 || p1==NULL || *p1!='+'){
        radar_crop=NULL;
        return;
    }
    croph=strtol(p1, &p2, 10);
    if(croph<1 || (p2!=NULL && *p2!='\0')){
        radar_crop=NULL;
        return;
    }
}


void init_radar(void){
    char *e;
    
    radar=XCreatePixmap(display, wmgen.pixmap, 52, 40, d_depth);
    reset_radar(&radar);

    if(radar_uri==NULL) return;

    e=strrchr(radar_uri, '/');
    if(e==NULL) e=radar_uri;
    else e++;
    snprintf(bigbuf, BIGBUF_LEN-21, "%s.", e);
    for(e=bigbuf; *e!='\0'; e++){
        if(!isalnum(*e) && *e!='.' && *e!='-' && *e!='+' && *e!='%'
           && *e!='?' && *e!='=' && *e!='&') *e='_';
    }
    strcpy(e, "radar-image");
    radar_file=get_pid_filename(bigbuf);
    strcpy(e, "new-radar-image");
    radar_newfile=get_pid_filename(bigbuf);

    radar_update_time=radar_time==0;
    
    parse_crop();
    parse_cross();
    do_radar_cross=0;
    
    /* Delete stale files, if any */
    unlink(radar_file);
    unlink(radar_newfile);
}

static void radar_callback(char *filename, void *v){
    struct stat statbuf;

    if(stat(radar_newfile, &statbuf)>=0){
        if(S_ISREG(statbuf.st_mode) && statbuf.st_size!=0
           && parse_radar(radar_newfile)){
            rename(radar_newfile, radar_file);
        } else {
            unlink(radar_newfile);
            if(!parse_radar(radar_file)) reset_radar(&radar);
        }
    }
}

void radar_cleanup(void){
    if(radar_file==NULL) return;
    unlink(radar_newfile);
    unlink(radar_file);
}

void update_radar(int force){
    time_t t;

    if(radar_file==NULL) return;

    t=time(NULL)/60;
    if(!force && radar_time>t) return;

    radar_time=t+30;
    download_file(radar_newfile, radar_uri, radar_post, force?DOWNLOAD_KILL_OTHER_REQUESTS:0, radar_callback, NULL);
}

static RContext *rc=NULL;

static int parse_radar(char *file){
    RImage *r, *n;
    float w, h;
    RColor col={ 0, 0, 0, 255};
    int x, y, ww, hh;

    errno=0;
    radar_update_time=time(NULL);
    reset_radar(&radar);
    if(rc==NULL){
        rc=RCreateContext(display, screen, NULL);
        if(rc==NULL){
            warn("parse_radar context creation: %s", RMessageForError(RErrorCode));
            return 0;
        }
    }

    r=RLoadImage(rc, file, 0);
    if(!r) return 0;

    if(radar_crop!=NULL){
        x=cropx; y=cropy;
        ww=cropw; hh=croph;
        if(x<0) x+=r->width;
        if(y<0) y+=r->height;
        if(x<0){ ww+=x; x=0; }
        if(y<0){ hh+=y; y=0; }

        if(x>=r->width || y>=r->width || ww<=0 || hh<=0){
            RReleaseImage(r);
            warn("parse_radar radar_crop exceeds image dimensions");
            return 0;
        }

        n=RGetSubImage(r, x, y, ww, hh);
        RReleaseImage(r);
        r=n;
        if(!r){
            warn("parse_radar crop: %s", RMessageForError(RErrorCode));
            return 0;
        }
    }
    
    if(r->width>52 || r->height>40 || (r->width!=52 && r->height!=40)){
        w=r->width/52;
        h=r->height/40;
        if(w>h) h=w;
        else w=h;

        n=RSmoothScaleImage(r, r->width/w, r->height/h);
        RReleaseImage(r);
        r=n;
        if(!r){
            warn("parse_radar scale: %s", RMessageForError(RErrorCode));
            return 0;
        }
    }

    if(r->width!=52 || r->height!=40){
        n=RMakeCenteredImage(r, 52, 40, &col);
        RReleaseImage(r);
        r=n;
        if(!r){
            warn("parse_radar center: %s", RMessageForError(RErrorCode));
            return 0;
        }
    }
    
    if(!RConvertImage(rc, r, &radar)){
        RReleaseImage(r);
        warn("parse_radar convert: %s", RMessageForError(RErrorCode));
        return 0;
    }
    RReleaseImage(r);
    return 1;
}

void put_radar(int x, int y, int font){
    int i;

    XCopyArea(display, radar, wmgen.pixmap, NormalGC, 0, 0, 52, 40, x, y);
    if(font==0) i=0;
    else i=1;
    XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, 124, 60+i, 54, 1, x-1, y-1);
    XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, 124, 60+i, 54, 1, x-1, y+40);
    XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, 162+i, 64, 1, 40, x-1, y);
    XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, 162+i, 64, 1, 40, x+52, y);
    if(radar_cross && do_radar_cross){
        combineWithOpacity(124, 60+i, 52, 1, x, y+crossy, 128);
        combineWithOpacity(162+i, 64, 1, 40, x+crossx, y, 128);
    }
}
