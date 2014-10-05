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

/* One-line algorithm from http://www.moonstick.com/moon_phase_emergency.htm
 * It's a bit rough, but it works well enough */

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
#include <math.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "convert.h"

#include "wmgeneral/wmgeneral-x11.h"

static double fpart(double t){
    return t-trunc(t);
}

double calc_moon(int month, int day, int year, int hm){
    time_t t=time(NULL);
    struct tm *tm;
    double p;

    tm=gmtime(&t);
    tm->tm_hour=hm/100;
    tm->tm_min=hm%100;
    tm->tm_sec=0;
    tm->tm_mon=month-1;
    tm->tm_mday=day;
    tm->tm_year=year;
    t=mkgmtime(tm);

    /* This next line is the algorithm. */
    p=fpart(((t/86400.0-11323.0)*850.0+5130.5769)/25101.0);
    if(p>.5) return -.5+cos(2*PI*p)/2;
    return .5-cos(2*PI*p)/2;
}


#define darkside 0.19921875
#define lightside (1-0.19921875)
#define maxwidth 17
static int widths[]={ 7, 11, 13, 15, 15, 17, 17, 17, 17, 17, 17, 17, 15, 15, 13, 11, 7, -1 };
extern int screen;
extern XpmIcon wmgen;
extern GC NormalGC;

/* Duplicates quite a bit of code from combineWithOpacity for speed */
void copySunMoon(int x, int y, double percent){
    XImage *pix;
    unsigned int w, h, bar;
    int foo;
    Window baz;
    int rmask, gmask, bmask;
    unsigned long spixel;
    int xx, terminator, oflag;
    int flag;
    double frac;

    if(isnan(percent)){
        copyPixmapArea(164, 64, 26, 25, x, y);
        return;
    }

    XGetGeometry(display, wmgen.pixmap, &baz, &foo, &foo, &w, &h, &bar, &bar);
    pix=XGetImage(display, wmgen.pixmap, 0, 0, w, h, AllPlanes, ZPixmap);

    if (pix->depth == DefaultDepth(display, screen)) {{
        Visual *visual=DefaultVisual(display, screen);
        rmask = visual->red_mask;
        gmask = visual->green_mask;
        bmask = visual->blue_mask;
    }} else {
        rmask = pix->red_mask;
        gmask = pix->green_mask;
        bmask = pix->blue_mask;
    }

    x+=4; y+=4;
    flag=(percent<0);
    if(flag) percent=-percent;
    for(h=0; widths[h]>0; h++){
        xx=(maxwidth-widths[h])>>1;
        if(flag){
            oflag=1;
            terminator=widths[h]*percent;
        } else {
            oflag=0;
            terminator=widths[h]-widths[h]*percent;
        }
        frac=lightside*fpart(widths[h]*percent)+darkside;
        for(w=0; w<widths[h]; w++){
            spixel=XGetPixel(pix, 168+xx+w, 93+h);
            if(w==terminator){
                oflag=!oflag;
                XPutPixel(pix, x+xx+w, y+h,
                          (((unsigned long)((spixel&rmask)*frac))&rmask) |
                          (((unsigned long)((spixel&gmask)*frac))&gmask) |
                          (((unsigned long)((spixel&bmask)*frac))&bmask));
            } else if(oflag){
                XPutPixel(pix, x+xx+w, y+h, spixel);
            } else {
                XPutPixel(pix, x+xx+w, y+h,
                          (((unsigned long)((spixel&rmask)*darkside))&rmask) |
                          (((unsigned long)((spixel&gmask)*darkside))&gmask) |
                          (((unsigned long)((spixel&bmask)*darkside))&bmask));
            }
        }
    }

    XPutImage(display, wmgen.pixmap, NormalGC, pix, 0, 0, 0, 0, pix->width, pix->height);

    XDestroyImage(pix);
}

#if 0
void copySunMoon(int x, int y, double percent){
    int w, h;
    int xx;
    int frac;
    int flag;

    if(isnan(percent)){
        copyPixmapArea(164, 64, 26, 25, x, y);
        return;
    }

    combineWithOpacity(164, 89, 26, 25, x, y, 51);
    x+=4; y+=4;
    flag=(percent<0);
    if(flag) percent=-percent;
    for(h=0; h<=8; h++){
        w=widths[h]*percent;
        frac=(widths[h]*percent-w)*100;
        if(flag) xx=(17-widths[h])/2;
        else xx=(17+widths[h])/2-w;
        copyPixmapArea(141+xx, 93+h, w, 1, x+xx, y+h);
        copyPixmapArea(141+xx, 109-h, w, 1, x+xx, y+16-h);
        if(flag){
            combineWithOpacity(141+xx+w, 93+h, 1, 1, x+xx+w, y+h, frac);
            combineWithOpacity(141+xx+w, 109-h, 1, 1, x+xx+w, y+16-h, frac);
        } else {
            combineWithOpacity(141+xx-1, 93+h, 1, 1, x+xx-1, y+h, frac);
            combineWithOpacity(141+xx-1, 109-h, 1, 1, x+xx-1, y+16-h, frac);
        }
    }
}
#endif
