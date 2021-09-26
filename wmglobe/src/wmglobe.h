/*     WMGlobe 1.3  -  All the Earth on a WMaker Icon
 *     copyright (C) 1998,99,2000,01 Jerome Dumonteil <jerome.dumonteil@linuxfr.org>
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ***************************************************************************/
/* it uses some functions of : Xglobe, Xearth, wmgeneral, wmaker/wrlib
 ***************************************************************************/


#ifndef WMG_HEADER_H
#define WMG_HEADER_H

#ifndef MY_EXTERN
#define MY_EXTERN extern
#endif

/* customization : see wmgoption.h */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>		/*toupper */
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <assert.h>
#include <X11/Xatom.h>
#include <locale.h>

#include "wraster.h"
#include "wmgoption.h"

#define FALSE 	0
#define TRUE 	1
#define STRONG  2
#define MAX(x, y)       ((x) < (y) ? (y) : (x))
#define MIN(x, y)       ((x) > (y) ? (y) : (x))
#define ABS(a)          ((a) < 0 ? -(a) : (a))

#define PTFIXED   1
#define PTSUN     2
#define PTRANDOM  3
#define PTMOON    4

#ifndef PI
#define PI 3.141592653
#endif

/*
 * wmglobe
 * variables globales
 */

/************/
/* Typedefs */
/************/
#define MAX_MOUSE_REGION (8)

typedef struct {
    int enable;
    int top;
    int bottom;
    int left;
    int right;
} MOUSE_REGION;

MY_EXTERN MOUSE_REGION mouse_region[MAX_MOUSE_REGION];

typedef struct MPO {
    int r, g, b;
} MPO;

MY_EXTERN MPO *md[4], *mn[4];
MY_EXTERN double solu[DIAMETRE][DIAMETRE][3];
MY_EXTERN int tabsolu[DIAMETRE][DIAMETRE];
MY_EXTERN int solution;

typedef struct {
    Pixmap pixmap;
    Pixmap mask;
    XpmAttributes attributes;
} XpmIcon;

MY_EXTERN Display *dpy;
MY_EXTERN char *dayfile, *nightfile, *dpy_name;
MY_EXTERN Pixmap pix, pixmask;
MY_EXTERN XEvent Event;
MY_EXTERN RImage *map, *small, *mapnight;
MY_EXTERN XpmIcon screenpos, scrdate, scrdiv, numpix, txtpix, wmg;
MY_EXTERN Window iconwin, win;

MY_EXTERN int onlyshape, option_iw;
MY_EXTERN GC NormalGC;

/********* rendering********/

#if WITH_MARKERS
MY_EXTERN double marker[MAX_MARKERS][3];
MY_EXTERN int nb_marker, sun_marker, moon_marker;
MY_EXTERN RColor sun_col, moon_col;
MY_EXTERN double moon_lat,moon_long;
#endif

MY_EXTERN double delay, time_multi;
/*
 * struct timeval delta_tim, last_tim, next_tim, render_tim, base_tim,
 *  vec_tim;
 *
 * time_t beg_time, ini_time,t1901;
 */
MY_EXTERN struct timeval tlast, tnext, trend, tdelay, tini, tbase;
MY_EXTERN time_t tsunpos;

MY_EXTERN int sens, fun, funx, funy, oknimap, mratiox, mratioy, gotoscr;

MY_EXTERN int typecadre, p_type, use_nightmap,
    use_default_nightmap, use_nmap_ini, firstTime,
    stoprand, do_something, iop;

MY_EXTERN double v_lat, v_long, old_dvlat, old_dvlong, dv_lat, dv_long;
MY_EXTERN double dlat, dlong, addlat, addlong, ratiox, ratioy, dawn;
MY_EXTERN double sun_lat;
MY_EXTERN double sun_long;
MY_EXTERN double fov;
MY_EXTERN double radius;
MY_EXTERN double proj_dist;		/* distance to projection plane */
MY_EXTERN double center_dist;		/*  distance to center of earth */
MY_EXTERN double ambient_light;		/* how dark is the dark side? */
MY_EXTERN double light_x, light_y, light_z;	/* vector of sunlight with lengt 1 */
MY_EXTERN double c_coef, b_coef;
MY_EXTERN double zoom;
MY_EXTERN int radius_proj, aml;		/* radius of sphere on screen */

MY_EXTERN RColor noir;
#ifdef DEBUG
MY_EXTERN double minhz;
#endif

MY_EXTERN int stable;

/****************************************************************/
/* Function Prototypes                                          */
/****************************************************************/
int main(int argc, char *argv[]);

void AddMouseRegion(int index, int left, int top, int right, int bottom);
int CheckMouseRegion(int x, int y);
void RedrawWindowXYWH(int x, int y, int w, int h);
void set_defaults();
void loadxpm(Window drawable);
void cmdline(int argc, char *argv[]);
void screen_back();
void rotation_terre(int x, int y, int lat_flag);
void zooming(int facto);
struct timeval diftimev(struct timeval t1, struct timeval t2);
struct timeval addtimev(struct timeval t1, struct timeval t2);
struct timeval getimev();

void setZoom(double z);
void calcDistance();
void renderFrame();
void initmyconvert();
int myRConvertImage(RContext * context, RImage * image, Pixmap * pixmap);
RContext *myRCreateContext
    (Display * dpy, int screen_number, RContextAttributes * attribs);
void setTime(struct timeval t);
void recalc(int calme);
void sun_position(time_t ssue, double *lat, double *lon);
void moon_position(time_t ssue, double *lat, double *lon);
void transform_marker(int m);
void setViewPos(double lat, double lon);
int ripalpha(RImage * image);
RImage*
RScaleImage(RImage *image, unsigned new_width, unsigned new_height);
void
RReleaseImage(RImage *image);

#endif
