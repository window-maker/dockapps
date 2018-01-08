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
/* this code was based on XGlobe :
   renderer.cpp
   *   
   * This file is part of XGlobe. See README for details.
   *
   * Copyright (C) 1998 Thorsten Scheuermann
   *
   * This program is free software; you can redistribute it and/or modify
   * it under the terms of the GNU General Public Licenses as published by
   * the Free Software Foundation.
   ************************************************************************/
/* 
   Some parts of this files should be rewritten to not depend on 
   WindowMaker version
*/

#include "wmglobe.h"

static RColor mygetMapColorLinear
    (double longitude, double latitude, double angle);

/*
 * static RColor getMapColor(double longitude, double latitude, double angle);
 */

static void randomPosition();
void setViewPos(double lat, double lon);
static void myRPutPixel(int x, int y, RColor * color);
#ifdef WITH_MARKERS
#if (WITH_MARKERS == 1)
static void invertPixel(int x, int y);
static void put_cross(int x, int y);
static void put_dot_cross(int x, int y, RColor * color);
#endif
#endif
static void getquarter(RImage * image, int x, int y, MPO * m[4], int dx,
		       int dy);
static void updateTime(int force);
static struct timeval timeaccel(struct timeval t);






struct timeval timeaccel(struct timeval t)
{
    struct timeval at;
    double rr;

    t = diftimev(t, tini);
    rr = floor((double) t.tv_sec * time_multi +
	       (double) t.tv_usec * time_multi / 1000000.);
/*** something bad may appen if time_multi=max after 41 minutes (overflow) ***/
    while (rr > (double) LONG_MAX)
	rr -= (2.0 * (double) LONG_MAX + 1.0);
    at.tv_sec = (int) rr;
    at.tv_usec = (int) (t.tv_usec * time_multi) % 1000000;
    return addtimev(at, tbase);
}






static void myRPutPixel(int x, int y, RColor * color)
{
    int ofs;
    unsigned char *sr, *sg, *sb;

    ofs = (y * DIAMETRE + x) * 3;
    sr = small->data + (ofs++);
    sg = small->data + (ofs++);
    sb = small->data + (ofs);

    *sr = color->red;
    *sg = color->green;
    *sb = color->blue;
    return;
}






#if WITH_MARKERS
static void invertPixel(int x, int y)
{
    int ofs;
    unsigned char *sr, *sg, *sb;

    ofs = (y * DIAMETRE + x) * 3;
    sr = small->data + (ofs++);
    sg = small->data + (ofs++);
    sb = small->data + (ofs);

#if ( CROSS_INVERT == 1 )
    *sr = 255 - *sr;
    *sg = 255 - *sg;
    *sb = 255 - *sb;
#else
    if (*sb > 127 || *sg > 127 || *sr > 127)
	*sr = *sg = *sb = 0;
    else
	*sr = *sg = *sb = 255;
#endif
    return;
}

#define T_CADRE(x,y) ((x) < DIAMETRE && (x) >= 0 \
&& (y) < DIAMETRE && (y) >= 0 && tabsolu[(x)][(y)])





static void put_cross(int x, int y)
{
    int i, x_cross, y_cross;

    if (T_CADRE(x, y)) {
	if (!fun && sens != 1) {
	    x = DIAMETRE - 1 - x;
	    y = DIAMETRE - 1 - y;
	}
	for (i = 2; i <= CROSS_LENGTH; i++) {
	    x_cross = x + i;
	    y_cross = y;
	    if (T_CADRE(x_cross, y_cross))
		invertPixel(x_cross, y_cross);
	    x_cross = x - i;

	    if (T_CADRE(x_cross, y_cross))
		invertPixel(x_cross, y_cross);
	    x_cross = x;
	    y_cross = y - i;
	    if (T_CADRE(x_cross, y_cross))
		invertPixel(x_cross, y_cross);

	    y_cross = y + i;
	    if (T_CADRE(x_cross, y_cross))
		invertPixel(x_cross, y_cross);
	}
    }
}








static void put_dot_cross(int x, int y, RColor * color)
{
    int i, x_cross, y_cross;

    if (T_CADRE(x, y)) {
	if (!fun && sens != 1) {
	    x = DIAMETRE - 1 - x;
	    y = DIAMETRE - 1 - y;
	}
	for (i = 2; i <= CROSS_LENGTH; i += 2) {
	    x_cross = x + i;
	    y_cross = y;
	    if (T_CADRE(x_cross, y_cross))
		invertPixel(x_cross, y_cross);
	    x_cross = x - i;

	    if (T_CADRE(x_cross, y_cross))
		invertPixel(x_cross, y_cross);
	    x_cross = x;
	    y_cross = y - i;
	    if (T_CADRE(x_cross, y_cross))
		invertPixel(x_cross, y_cross);

	    y_cross = y + i;
	    if (T_CADRE(x_cross, y_cross))
		invertPixel(x_cross, y_cross);

	    x_cross = x + i + 1;
	    y_cross = y;
	    if (T_CADRE(x_cross, y_cross))
		myRPutPixel(x_cross, y_cross, color);
	    x_cross = x - i - 1;

	    if (T_CADRE(x_cross, y_cross))
		myRPutPixel(x_cross, y_cross, color);
	    x_cross = x;
	    y_cross = y - i - 1;
	    if (T_CADRE(x_cross, y_cross))
		myRPutPixel(x_cross, y_cross, color);

	    y_cross = y + i + 1;
	    if (T_CADRE(x_cross, y_cross))
		myRPutPixel(x_cross, y_cross, color);
	}
    }
}
#endif








static void getquarter(RImage * image, int x, int y, MPO * m[4], int dx,
		       int dy)
{
    int xx;
    register int ofs;

/*** hope this is faster than calculation with floats .... ****/

    x %= image->width;
    xx = x;
    y %= image->height;
    ofs = (y * image->width + x) * 3;
    m[0]->r = image->data[ofs++];
    m[0]->g = image->data[ofs++];
    m[0]->b = image->data[ofs];

    xx++;
    xx %= image->width;
    ofs = (y * image->width + xx) * 3;
    m[1]->r = image->data[ofs++];
    m[1]->g = image->data[ofs++];
    m[1]->b = image->data[ofs];

    y++;
    y %= image->height;
    ofs = (y * image->width + x) * 3;
    m[2]->r = image->data[ofs++];
    m[2]->g = image->data[ofs++];
    m[2]->b = image->data[ofs];

    ofs = (y * image->width + xx) * 3;
    m[3]->r = image->data[ofs++];
    m[3]->g = image->data[ofs++];
    m[3]->b = image->data[ofs];


/*
 * m[0]->r=((m[0]->r*(256-dx)*(256-dy))+
 *      (m[1]->r*dx*(256-dy))+
 *      (m[2]->r*(256-dx)*dy)+
 *      (m[3]->r*dx*dy))>>16;
 * m[0]->g=((m[0]->g*(256-dx)*(256-dy))+
 *      (m[1]->g*dx*(256-dy))+
 *      (m[2]->g*(256-dx)*dy)+
 *      (m[3]->g*dx*dy))>>16;
 * m[0]->b=((m[0]->b*(256-dx)*(256-dy))+
 *      (m[1]->b*dx*(256-dy))+
 *      (m[2]->b*(256-dx)*dy)+
 *      (m[3]->b*dx*dy))>>16;
 */

    if ((ofs = m[1]->r - m[0]->r) != 0)
	m[0]->r += (ofs * dx) >> 8;
    if ((ofs = m[1]->g - m[0]->g) != 0)
	m[0]->g += (ofs * dx) >> 8;
    if ((ofs = m[1]->b - m[0]->b) != 0)
	m[0]->b += (ofs * dx) >> 8;

    if ((ofs = m[3]->r - m[2]->r) != 0)
	m[2]->r += (ofs * dx) >> 8;
    if ((ofs = m[3]->g - m[2]->g) != 0)
	m[2]->g += (ofs * dx) >> 8;
    if ((ofs = m[3]->b - m[2]->b) != 0)
	m[2]->b += (ofs * dx) >> 8;

    if ((ofs = m[2]->r - m[0]->r) != 0)
	m[0]->r += (ofs * dy) >> 8;
    if ((ofs = m[2]->g - m[0]->g) != 0)
	m[0]->g += (ofs * dy) >> 8;
    if ((ofs = m[2]->b - m[0]->b) != 0)
	m[0]->b += (ofs * dy) >> 8;

    return;
}









void calcDistance()
{
    double tan_a;

    tan_a = (zoom * DIAMETRE / 2.0) / proj_dist;
/*
 *     distance of camera to center of earth ( = coordinate origin)
 */
    center_dist = radius / sin(atan(tan_a));
    c_coef = center_dist * center_dist - radius * radius;
    solution = FALSE;
    return;
}









void renderFrame()
{
    int py, px;
    RColor teinte;

    double dir_x, dir_y, dir_z;	/* direction of cast ray */

    double hit_x, hit_y, hit_z;	/* hit position on earth surface */

    double hit2_x, hit2_y, hit2_z;	/* mirrored hit position on earth surface */

    double sp_x, sp_y, sp_z;	/* intersection point of globe and ray */

    double a;			/*  coeff. of quardatic equation */

    double udroot;		/*  racine */
    double wurzel;
    double r;			/* r' */

    double s1, s2, s;		/*distance between intersections and
				   camera position */

    double longitude, latitude;	/* coordinates of hit position */

    double light_angle;		/* cosine of angle between sunlight and
				   surface normal */

    int startx, endx;		/* the region to be painted */

    int starty, endy;

    double m11;
    double m12;
    double m13;
    double m21;
    double m22;
    double m23;
    double m31;
    double m32;
    double m33;

    a = dir_x = dir_y = 0;
    dir_z = -proj_dist;

#ifdef DEBUG
    fprintf(stdout, "solution : %d\n", solution);
#endif
    /*
     * clear image
     */
    if (solution == FALSE)
	RClearImage(small, &noir);
    /*
     *  rotation matrix    
     */

    m11 = cos(v_long);
    m22 = cos(v_lat);
    m23 = sin(v_lat);
    m31 = -sin(v_long);
    m12 = m23 * m31;
    m13 = -m22 * m31;
    m21 = 0.;
    m32 = -m23 * m11;
    m33 = m22 * m11;

    /*
     *  calc. radius of projected sphere
     */
    if (solution == FALSE) {
	b_coef = 2 * center_dist * dir_z;
	radius_proj =
	    (int) sqrt(b_coef * b_coef / (4 * c_coef) - dir_z * dir_z) + 1;
    }

    if (fun) {
	starty = DIAMETRE / 2 - radius_proj - 3;
	endy = DIAMETRE - starty - 1;
	if ((double) starty < (double) (-funy))
	    starty = -funy;
	if ((double) starty > (double) (DIAMETRE - 1 - funy))
	    starty = DIAMETRE - 1 - funy;
	if ((double) endy < (double) (-funy))
	    endy = -funy;
	if ((double) endy > (double) (DIAMETRE - 1 - funy))
	    endy = DIAMETRE - 1 - funy;

	if (solution == FALSE) {
	    int i, j;
	    if (starty + funy > 0)
		for (j = 0; j < starty + funy; j++)
		    for (i = 0; i < DIAMETRE; i++)
			tabsolu[i][j] = 0;
	    if (endy + 1 + funy <= DIAMETRE - 1)
		for (j = endy + funy + 1; j < DIAMETRE; j++)
		    for (i = 0; i < DIAMETRE; i++)
			tabsolu[i][j] = 0;
	}

	for (py = starty; py <= endy; py++) {

	    startx = DIAMETRE / 2 - 6 -
		(int) sqrt(MAX((radius_proj * radius_proj -
				(py - DIAMETRE / 2) *
				(py - DIAMETRE / 2)), 0.0));

	    endx = DIAMETRE - startx - 1;
	    if ((double) startx < (double) (-funx))
		startx = -funx;

	    if ((double) startx > (double) (DIAMETRE - 1 - funx))
		startx = DIAMETRE - 1 - funx;

	    if ((double) endx < (double) (-funx))
		endx = -funx;
	    if ((double) endx > (double) (DIAMETRE - 1 - funx))
		endx = DIAMETRE - 1 - funx;

	    if (solution == FALSE) {
		int i;
		if (startx + funx > 0)
		    for (i = 0; i < startx + funx; i++)
			tabsolu[i][py + funy] = 0;
		if (endx + 1 + funx <= DIAMETRE - 1)
		    for (i = endx + 1 + funx; i < DIAMETRE; i++)
			tabsolu[i][py + funy] = 0;
	    }


	    /*
	     *  calculate offset into image data
	     */

	    for (px = startx; px <= endx; px++) {
		if (solution == FALSE) {
		    dir_x = (double) px - DIAMETRE / 2 + 0.5;
		    dir_y = -(double) py + DIAMETRE / 2 - 0.5;

		    a = dir_x * dir_x + dir_y * dir_y + dir_z * dir_z;

		    udroot = b_coef * b_coef - 4 * a * c_coef;	/*what's under the
								   sq.root when solving the                                                                        quadratic equation */
		    if (udroot >= 0) {
			tabsolu[px + funx][py + funy] = 1;
			wurzel = sqrt(udroot);
			s1 = (-b_coef + wurzel) / (2. * a);
			s2 = (-b_coef - wurzel) / (2. * a);
			s = (s1 < s2) ? s1 : s2;	/* smaller solution belongs 
							   to nearer intersection */
			solu[px + funx][py + funy][0] = s * dir_x;
			solu[px + funx][py + funy][1] = s * dir_y;
			solu[px + funx][py + funy][2] =
			    center_dist + s * dir_z;
		    } else {
			tabsolu[px + funx][py + funy] = 0;
		    }
		}

		if (tabsolu[px + funx][py + funy]) {	/*  solution exists <=> 
							   intersection exists */
		    sp_x = solu[px + funx][py + funy][0];	/* sp = camera pos + s*dir */
		    sp_y = solu[px + funx][py + funy][1];
		    sp_z = solu[px + funx][py + funy][2];

		    hit_x = m11 * sp_x + m12 * sp_y + m13 * sp_z;
		    hit_y = m22 * sp_y + m23 * sp_z;
		    hit_z = m31 * sp_x + m32 * sp_y + m33 * sp_z;

		    hit2_x = -m11 * sp_x + m12 * sp_y + m13 * sp_z;
		    hit2_y = m22 * sp_y + m23 * sp_z;
		    hit2_z = -m31 * sp_x + m32 * sp_y + m33 * sp_z;
/*** hope hit_z wont get too close to zero *******/
		    if (ABS(hit_z) < 0.001) {
			if (hit_x * hit_z > 0.)
			    longitude = PI / 2.;
			else
			    longitude = -PI / 2.;
			if (hit_z > 0.)
			    hit_z = 0.001;
			else
			    hit_z = -0.001;
		    } else {
			longitude = atan(hit_x / hit_z);
		    }

		    if (hit_z < 0.)
			longitude += PI;

		    r = (double) sqrt(hit_x * hit_x + hit_z * hit_z);

		    latitude = atan(-hit_y / r);

		    light_angle =
			(light_x * hit_x + light_y * hit_y +
			 light_z * hit_z) / radius;

		    /*
		     *  Set pixel in image
		     */

		    teinte =
			mygetMapColorLinear(longitude, latitude,
					    light_angle);
			/* here dont use myRPutPixel since we prefer some
			error detection about limits */
		    RPutPixel(small, px + funx, py + funy, &teinte);
		}
	    }			/*px */
	}			/*py */

    }


/*** not fun : ***/
    else {
	starty = DIAMETRE / 2 - radius_proj - 3;
	starty = (starty < 0) ? 0 : starty;
	endy = DIAMETRE - starty - 1;
/*
 * py  0 to 63 max
 */
	if (solution == FALSE) {
	    int i, j;
	    if (starty > 0)
		for (j = 0; j < starty; j++)
		    for (i = 0; i < DIAMETRE; i++)
			tabsolu[i][j] = 0;
	    if (endy + 1 <= DIAMETRE - 1)
		for (j = endy + 1; j < DIAMETRE; j++)
		    for (i = 0; i < DIAMETRE; i++)
			tabsolu[i][j] = 0;
	}


	for (py = starty; py <= endy; py++) {
	    startx = DIAMETRE / 2 - 6 -
		(int) sqrt(MAX((radius_proj * radius_proj -
				(py - DIAMETRE / 2) *
				(py - DIAMETRE / 2)), 0.0));
	    startx = (startx < 0) ? 0 : startx;
/*
 *   0<= startx <=31
 */
	    if (solution == FALSE) {
		int i;
		if (startx > 0)
		    for (i = 0; i < startx; i++) {
			tabsolu[i][py] = 0;
			tabsolu[DIAMETRE - 1 - i][py] = 0;
		    }
	    }

	    for (px = startx; px < DIAMETRE / 2; px++) {
		if (solution == FALSE) {
		    dir_x = (double) px - DIAMETRE / 2 + 0.5;

		    dir_y = -(double) py + DIAMETRE / 2 - 0.5;

		    a = dir_x * dir_x + dir_y * dir_y + dir_z * dir_z;

		    /*what's under the sq.root when solving the
		       quadratic equation */
		    udroot = b_coef * b_coef - 4 * a * c_coef;
		    if (udroot >= 0) {
			tabsolu[px][py] = 1;
			tabsolu[DIAMETRE - 1 - px][py] = 1;
			wurzel = sqrt(udroot);
			s1 = (-b_coef + wurzel) / (2. * a);
			s2 = (-b_coef - wurzel) / (2. * a);
			s = (s1 < s2) ? s1 : s2;	/* smaller solution 
							   belongs to nearer 
							   intersection */
			/* sp = camera pos + s*dir */
			solu[px][py][0] = s * dir_x;
			solu[px][py][1] = s * dir_y;
			solu[px][py][2] = center_dist + s * dir_z;
		    } else {
			tabsolu[px][py] = 0;
			tabsolu[DIAMETRE - 1 - px][py] = 0;
		    }
		}
		if (tabsolu[px][py]) {	/*  solution exists <=> 
					   intersection exists */
		    sp_x = solu[px][py][0];
		    sp_y = solu[px][py][1];
		    sp_z = solu[px][py][2];
		    hit_x = m11 * sp_x + m12 * sp_y + m13 * sp_z;
		    hit_y = m22 * sp_y + m23 * sp_z;
		    hit_z = m31 * sp_x + m32 * sp_y + m33 * sp_z;

		    hit2_x = -m11 * sp_x + m12 * sp_y + m13 * sp_z;
		    hit2_y = m22 * sp_y + m23 * sp_z;
		    hit2_z = -m31 * sp_x + m32 * sp_y + m33 * sp_z;

/*** hope hit_z wont get too close to zero *******/
#ifdef DEBUG
		    if (ABS(hit_z) < ABS(minhz)) {
			minhz = hit_z;
			fprintf(stdout, "should >>0 : hit_z %f\n", hit_z);
			fprintf(stdout, "             hit_x %f\n", hit_x);
			fprintf(stdout, "             ratio %f\n",
				hit_x / hit_z);
			fprintf(stdout, "             long  %f\n",
				atan(hit_x / hit_z));

			sleep(1);
		    }
#endif
		    if (ABS(hit_z) < 0.001) {
			if (hit_x * hit_z > 0.)
			    longitude = PI / 2.;
			else
			    longitude = -PI / 2.;
			if (hit_z > 0.)
			    hit_z = 0.001;
			else
			    hit_z = -0.001;
		    } else {
			longitude = atan(hit_x / hit_z);
		    }

		    if (hit_z < 0.)
			longitude += PI;

		    r = (double) sqrt(hit_x * hit_x + hit_z * hit_z);

		    latitude = atan(-hit_y / r);

		    light_angle =
			(light_x * hit_x + light_y * hit_y +
			 light_z * hit_z) / radius;
		    if (sens == 1) {

			/*
			 *  Set pixel in image
			 */

			teinte =
			    mygetMapColorLinear(longitude, latitude,
						light_angle);
			myRPutPixel(px, py, &teinte);

			/*
			 * mirror the left half-circle of the globe: 
			 * we need a new position and have to recalc. the 
			 * light intensity
			 */

			light_angle =
			    (light_x * hit2_x + light_y * hit2_y +
			     light_z * hit2_z) / radius;
			teinte =
			    mygetMapColorLinear(2 * v_long - longitude,
						latitude, light_angle);
			myRPutPixel(DIAMETRE - px - 1, py, &teinte);
		    } else {
			/* sens==-1 */
			/*
			 *  Set pixel in image
			 */

			teinte =
			    mygetMapColorLinear(longitude, latitude,
						light_angle);
			myRPutPixel(DIAMETRE - px - 1, DIAMETRE - py - 1,
				    &teinte);

			/*
			 * mirror the left half-circle of the globe: 
			 * we need a new position and have 
			 * to recalc. the light intensity
			 */

			light_angle =
			    (light_x * hit2_x + light_y * hit2_y +
			     light_z * hit2_z) / radius;
			teinte =
			    mygetMapColorLinear(2 * v_long - longitude,
						latitude, light_angle);
			myRPutPixel(px, DIAMETRE - py - 1, &teinte);
		    }
		}
	    }			/*px */
	}			/*py */


    }				/*else fun */
#if WITH_MARKERS
/* markers */
    if (nb_marker) {
	int i;
	double mx, my, mz;

	for (i = 0; i < nb_marker; i++) {

	    mx = m11 * marker[i][0] + m31 * marker[i][2];
	    mz = -m31 * marker[i][0] + m11 * marker[i][2];
	    my = m22 * marker[i][1] - m23 * mz;
	    mz = m23 * marker[i][1] + m22 * mz;

	    if (mz > 0) {
		if (i == sun_marker) {
		    put_dot_cross((int)
				  (mx * radius_proj + DIAMETRE / 2 + funx),
				  (int) (-my * radius_proj + DIAMETRE / 2 +
					 funy), &sun_col);
		} else if (i == moon_marker) {
		    put_dot_cross((int)
				  (mx * radius_proj + DIAMETRE / 2 + funx),
				  (int) (-my * radius_proj + DIAMETRE / 2 +
					 funy), &moon_col);
		} else {
		    put_cross((int)
			      (mx * radius_proj + DIAMETRE / 2 + funx),
			      (int) (-my * radius_proj + DIAMETRE / 2 +
				     funy));
		}
	    }
	}
    }
#endif
    solution = TRUE;
    return;
}













static RColor
mygetMapColorLinear(double longitude, double latitude, double angle)
{
    RColor point;
    int x, y, xl, yl, dx, dy, ang;

    if (longitude < 0.)
	longitude += 2 * PI;

    latitude += PI / 2;

    longitude += PI;
    if (longitude >= 2 * PI)
	longitude -= 2 * PI;

    if (angle > 0)
	ang = (int) floor((1 - ((1 - angle) * dawn)) * 256);
    else
	ang = angle * 256;

    xl = (int) (longitude * mratiox);
    yl = (int) (latitude * mratioy);

    x = xl >> 8;
    y = yl >> 8;
    dx = xl - (x << 8);
    dy = yl - (y << 8);

    if (use_nightmap) {
	if (ang > 0) {
	    getquarter(map, x, y, md, dx, dy);
	    getquarter(mapnight, x, y, mn, dx, dy);

	    md[0]->r = ((mn[0]->r * (256 - ang) + md[0]->r * ang)) >> 8;
	    md[0]->g = ((mn[0]->g * (256 - ang) + md[0]->g * ang)) >> 8;
	    md[0]->b = ((mn[0]->b * (256 - ang) + md[0]->b * ang)) >> 8;
	} else {
	    getquarter(mapnight, x, y, md, dx, dy);
	}
    } else {
	getquarter(map, x, y, md, dx, dy);
	if (ang > 0) {
	    md[0]->r =
		((md[0]->r * aml +
		  md[0]->r * ang / 256 * (256 - aml))) >> 8;
	    md[0]->g =
		((md[0]->g * aml +
		  md[0]->g * ang / 256 * (256 - aml))) >> 8;
	    md[0]->b =
		((md[0]->b * aml +
		  md[0]->b * ang / 256 * (256 - aml))) >> 8;
	} else {
	    md[0]->r = (md[0]->r * aml) >> 8;
	    md[0]->g = (md[0]->g * aml) >> 8;
	    md[0]->b = (md[0]->b * aml) >> 8;
	}
    }

    point.red = (unsigned char) md[0]->r;
    point.green = (unsigned char) md[0]->g;
    point.blue = (unsigned char) md[0]->b;
    point.alpha = 255; /* in fun mode, we use original Rputpixel that need alpha?*/
    return point;
}







static void randomPosition()
{
    addlat = ((rand() % 30001) / 30000.) * 180. - 90.;
    addlong = ((rand() % 30001) / 30000.) * 360. - 180.;
    return;
}









static void updateTime(int force)
{
/* calcul of sun position every minute */
    if ((trend.tv_sec - tsunpos) >= 60 || force) {
	tsunpos = trend.tv_sec;
	sun_position(tsunpos, &sun_lat, &sun_long);
	light_x = cos(sun_lat) * sin(sun_long);
	light_y = sin(sun_lat);
	light_z = cos(sun_lat) * cos(sun_long);
	do_something = TRUE;
#if WITH_MARKERS
	if (sun_marker >= 0) {
	    marker[sun_marker][0] = light_x;
	    marker[sun_marker][1] = light_y;
	    marker[sun_marker][2] = light_z;
	}
/* ... and the moon position */
	if (moon_marker >= 0 || p_type == PTMOON || force == STRONG) {
	    moon_position(tsunpos, &moon_lat, &moon_long);
	    if (moon_marker >= 0) {
		marker[moon_marker][1] = moon_lat;
		marker[moon_marker][0] = moon_long;
		transform_marker(moon_marker);
	    }
	}
#endif
    }
    return;
}









void transform_marker(int m)
{
    /* long/lat => rotation matrix */
    double dtmp1, dtmp2;

    dtmp1 = sin(marker[m][0]) * cos(marker[m][1]);
    dtmp2 = sin(marker[m][1]);
    marker[m][2] = cos(marker[m][0]) * cos(marker[m][1]);
    marker[m][0] = dtmp1;
    marker[m][1] = dtmp2;
}








void setViewPos(double lat, double lon)
{
    double dif;
    while (lat >= 360.)
	lat -= 360.;
    while (lat <= -360.)
	lat += 360.;
    while (addlat >= 360.)
	addlat -= 360.;
    while (addlat <= -360.)
	addlat += 360.;

    if (lat >= 90.) {
	dif = lat;
	lat = 180. - lat;
	addlat += (lat - dif);
	dlat *= -1;
	if (!fun && !stable) {
	    lon += 180.;
	    addlong += 180.;
	}
	if (!stable)
	    sens *= -1;
    }
    if (lat <= -90.) {
	dif = lat;
	lat = -180. - lat;
	addlat += (lat - dif);
	dlat *= -1;
	if (!fun && !stable) {
	    lon += 180.;
	    addlong += 180.;
	}
	if (!stable)
	    sens *= -1;
    }
    if (lat >= 90.) {
	dif = lat;
	lat = 180. - lat;
	addlat += (lat - dif);
	dlat *= -1;
	if (!fun && !stable) {
	    lon += 180.;
	    addlong += 180.;
	}
	if (!stable)
	    sens *= -1;
    }
    if (lat <= -90.) {
	dif = lat;
	lat = -180. - lat;
	addlat += (lat - dif);
	dlat *= -1;
	if (!fun && !stable) {
	    lon += 180.;
	    addlong += 180.;
	}
	if (!stable)
	    sens *= -1;
    }
    while (lon >= 180.) {
	lon -= 360.;
	addlong -= 360.;
    }
    while (lon <= -180.) {
	lon += 360.;
	addlong += 360.;
    }

    v_lat = lat * PI / 180.;
    v_long = lon * PI / 180.;
    dv_lat = lat;
    dv_long = lon;

    return;
}








void recalc(int force)
{
    double coeff, va, vo;
    struct timeval tv, tnow;

    tnow = getimev();
    trend = timeaccel(tnow);
    tv = diftimev(tnow, tlast);

    if (firstTime) {
	firstTime = FALSE;
	updateTime(STRONG);
    } else {
	coeff = (double) tv.tv_sec + tv.tv_usec / 1000000.;

	if (!force) {
/** while !clic button rotate earth **/
	    addlat += dlat * coeff;
	    addlong += dlong * coeff;
	}
    }

    if (addlong != old_dvlong || addlat != old_dvlat || p_type == PTRANDOM) {
	old_dvlong = addlong;
	old_dvlat = addlat;
	do_something = TRUE;
    }
#if WITH_MARKERS
    if (force) {
	if (p_type == PTSUN) {
	    va = sun_lat * 180. / PI;
	    vo = sun_long * 180. / PI;
	    updateTime(TRUE);
	    addlat -= sun_lat * 180. / PI - va;
	    addlong -= sun_long * 180. / PI - vo;
	} else if (p_type == PTMOON) {
	    va = moon_lat * 180. / PI;
	    vo = moon_long * 180. / PI;
	    updateTime(TRUE);
	    addlat -= moon_lat * 180. / PI - va;
	    addlong -= moon_long * 180. / PI - vo;
	}
#else
    if (force && p_type == PTSUN) {
	va = sun_lat * 180. / PI;
	vo = sun_long * 180. / PI;
	updateTime(TRUE);
	addlat -= sun_lat * 180. / PI - va;
	addlong -= sun_long * 180. / PI - vo;
#endif
    } else {
	updateTime(FALSE);
    }

    if (do_something) {
	switch (p_type) {
	case PTSUN:
	    setViewPos(sun_lat * 180. / PI + addlat,
		       sun_long * 180. / PI + addlong);
	    break;

#if WITH_MARKERS
	case PTMOON:
	    setViewPos(moon_lat * 180. / PI + addlat,
		       moon_long * 180. / PI + addlong);
	    break;
#endif
	case PTFIXED:
	    setViewPos(addlat, addlong);
	    break;

	case PTRANDOM:
	    if (stoprand == FALSE)
		randomPosition();
	    else
		stoprand--;
	    setViewPos(addlat, addlong);
	    break;

	default:
	    break;
	}
#ifdef DEBUG
	fprintf(stdout, "%s  render\n", ctime(&trend.tv_sec));
#endif
	renderFrame();
    }
    tlast = tnow;
    tnext = addtimev(tnow, tdelay);

    return;
}






/* convert a 4 layers RGBA image to a 3 layer one */
int ripalpha(RImage * image)
{
    int x, y;
    unsigned char *d, *s, *old;

    if (image == NULL)
	return 0;
    if (image->format == RRGBFormat)
	return 1;

    d = malloc(image->width * image->height * 3 + 4);
    if (!d) {
	RErrorCode = RERR_NOMEMORY;
	puts("error in ripalpha");
	return 0;
    }

    s = image->data;
    old = image->data;
    image->data = d;
    image->format = RRGBFormat;
    for (y = 0; y < image->height; y++) {
	for (x = 0; x < image->width; x++) {
	    *d++ = *s++;
	    *d++ = *s++;
	    *d++ = *s++;
	    s++;
	}
    }
    free(old);
    return 1;
}
