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

/*
 * #define DEBUG
 */

#define WMGVERSION "WMGlobe v.0.5   6 fev 1999 <jerome.dumonteil@capway.com>"

/*
 * comment DEFMAPOK to not install the built-in default maps,
 * (smaller binary) usefull if you never use the default map
 */

#define DEFMAPOK

/*
 * uncomment MOUSE_LAT_FULL to supprim the shift+left method of rotate earth
 */

/*
 * #define MOUSE_LAT_NO_SHIFT
 */

/*
 * number of parameter screen : min 1, max 7 (all, the recommanded default)
 * - this doesnt modify the binary size -
 *  1 : postion latitude & longitude
 *  2 : time of viewpoint (+ screen 1)
 *  3 : delay & zoom (+ screen 1 & 2)
 *  4 : light & dawn (+ screen 1..3)
 *  5 : accel & night map (+ screen 1..4)
 *  6 : dlat & dlong (+ screen 1..5)
 *  7 : type of view (+ screen 1..6)
 */
#define NUM_SCREEN	7

/*** 0.04 sec main loop sleep (maximum refresh rate when delay=0) ***/
#define VAL_USLEEP 	40000

#define VAL_USLEEP_SHORT 500

/* waiting time before get back from param screen to earth (seconds) */
#define SCREEN_WAIT	5

#define ZOOM_FACTOR 	1.06
#define ZOOM_MIN 	0.08
#define ZOOM_MAX 	100000.0
#define STOP_RANDOM_FACTOR 1
#define RATIO_ROTATE 	0.5


#define DEFAULT_DELAY 	1.0
#define DEFAULT_V_LAT 	0.0
#define DEFAULT_V_LONG 	0.0
#define DEFAULT_SENS 	1
#define DEFAULT_ZOOM 	1.0
#define DEFAULT_LIGHT 	0.25
#define DEFAULT_BORDER 	0
#define DEFAULT_NIGHTMAP 1	/* 1 or 0  */

#define MAX_DELAY_SEC	86400.0
#define MAX_MULTI_COEF	864000.0
#define MAX_DELTA_LONG	1800.0

/***  (1 - dawn/2)   *****/
#define DEFAULT_DAWN 	0.9

/* change this if not 64x64 icons (not deep tested) you will need to change
   the cadrex.xbm too and a few other things for the parameters menus
   --- DIAMETRE must be a multiple of 2 ---                  */
#define DIAMETRE 	64
