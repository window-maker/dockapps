/*     WMGlobe 0.5  -  All the Earth on a WMaker Icon
 *     copyright (C) 1998,99 Jerome Dumonteil <jerome.dumonteil@capway.com>
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
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define ABS(a)          ((a) < 0 ? -(a) : (a))

#define PTFIXED 1
#define PTSUNREL 2
#define PTRANDOM 3

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

MOUSE_REGION mouse_region[MAX_MOUSE_REGION];

typedef struct MPO {
	int r, g, b;
} MPO;

MPO *md[4], *mn[4];

double soluce[DIAMETRE / 2][DIAMETRE][4];
int solution;

/************/

typedef struct {
	Pixmap pixmap;
	Pixmap mask;
	XpmAttributes attributes;
} XpmIcon;

/************/


Display *dpy;

char *dayfile, *nightfile, *dpy_name;

Pixmap pix, pixmask;

XEvent Event;

RImage *map, *small, *mapnight;

XpmIcon screenpos, scrdate, scrdiv, numpix, txtpix, wmg;

Window iconwin, win;
int onlyshape, option_iw;
GC NormalGC;

/********* rendering********/

double delay, time_multi;
/*
 * struct timeval delta_tim, last_tim, next_tim, render_tim, base_tim,
 *  vec_tim;
 * 
 * time_t beg_time, ini_time,t1901;
 */
struct timeval tlast, tnext, trend, tdelay, tini, tbase;
time_t tsunpos;

int sens, fun, funx, funy, oknimap, mratiox, mratioy, gotoscr;

int typecadre, p_type, use_nightmap, use_nmap_ini, firstTime, stoprand,
 do_something, iop;

double v_lat, v_long, old_dvlat, old_dvlong, dv_lat, dv_long;
double dlat, dlong, addlat, addlong, ratiox, ratioy, dawn;

double sun_lat;
double sun_long;

double fov;
double radius;
double proj_dist;		/* distance to projection plane */

double center_dist;		/*  distance to center of earth */

double ambient_light;		/* how dark is the dark side? */

double light_x, light_y, light_z;	/* vector of sunlight with lengt 1 */

double c_coef, b_coef;
double zoom;
int radius_proj, aml;		/* radius of sphere on screen */

RColor noir;
#ifdef DEBUG
double minhz;
#endif

/****************************************************************/
/* Function Prototypes                                          */
/****************************************************************/
int main(int argc, char *argv[]);
/****************************************************************/
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

/***************************************************************/
void setZoom(double z);
void calcDistance();
void renderFrame();
void initmyconvert();
int myRConvertImage(RContext * context, RImage * image, Pixmap * pixmap);
RContext *myRCreateContext
 (Display * dpy, int screen_number, RContextAttributes * attribs);
void setTime(struct timeval t);
void recalc(int calme);
void GetSunPos(time_t ssue, double *lat, double *lon);
void setViewPos(double lat, double lon);

/***************************************************************/

#endif
