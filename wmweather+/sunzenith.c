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

/* Algorithms from http://www.srrb.noaa.gov/highlights/sunrise/azel.html */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

#include "convert.h"

/*  Purpose: calculate the Geometric Mean Longitude of the Sun (degrees) */
double calcGeomMeanLongSun(double t) {
    double L0 = 280.46646 + t * (36000.76983 + 0.0003032 * t);
    while(L0 > 360.0) {
        L0 -= 360.0;
    }
    while(L0 < 0.0) {
        L0 += 360.0;
    }
    return L0;
}


/*  Purpose: calculate the Geometric Mean Anomaly of the Sun (degrees) */
double calcGeomMeanAnomalySun(double t) {
    return 357.52911 + t * (35999.05029 - 0.0001537 * t);
}


/*  Purpose: calculate the eccentricity of earth's orbit */
double calcEccentricityEarthOrbit(double t) {
    return 0.016708634 - t * (0.000042037 + 0.0000001267 * t);
}


/*  Purpose: calculate the equation of center for the sun (degrees) */
double calcSunEqOfCenter(double t) {
    double m = deg2rad(calcGeomMeanAnomalySun(t));

    return sin(m) * (1.914602 - t * (0.004817 + 0.000014 * t)) + sin(m+m) * (0.019993 - 0.000101 * t) + sin(m+m+m) * 0.000289;
}


/*  Purpose: calculate the true longitude of the sun (degrees) */
double calcSunTrueLong(double t) {
    return calcGeomMeanLongSun(t) + calcSunEqOfCenter(t);
}


/*  Purpose: calculate the apparent longitude of the sun (degrees) */
double calcSunApparentLong(double t) {
    return calcSunTrueLong(t) - 0.00569 - 0.00478 * sin(deg2rad(125.04-1934.136*t));
}


/*  Purpose: calculate the mean obliquity of the ecliptic (degrees) */
double calcMeanObliquityOfEcliptic(double t) {
    return 23.0 + (26.0 + ((21.448 - t*(46.8150 + t*(0.00059 - t*(0.001813))))/60.0))/60.0;
}


/*  Purpose: calculate the corrected obliquity of the ecliptic (degrees) */
double calcObliquityCorrection(double t) {
    return calcMeanObliquityOfEcliptic(t) + 0.00256*cos(deg2rad(125.04-1934.136*t));
}


/*  Purpose: calculate the declination of the sun (degrees) */
double calcSunDeclination(double t) {
    return rad2deg(asin(sin(deg2rad(calcObliquityCorrection(t))) *
                         sin(deg2rad(calcSunApparentLong(t)))));
}


/*  Purpose: calculate the difference between true solar time and mean
 *              solar time (minutes)
 */
double calcEquationOfTime(double t) {
    double l0 = deg2rad(calcGeomMeanLongSun(t));
    double e = calcEccentricityEarthOrbit(t);
    double m = deg2rad(calcGeomMeanAnomalySun(t));
    double y = tan(deg2rad(calcObliquityCorrection(t))/2.0);
    double sinm = sin(m);

    y *= y;

    return rad2deg(y*sin(2.0*l0) - 2.0*e*sinm + 4.0*e*y*sinm*cos(2.0*l0)
                    - 0.5*y*y*sin(4.0*l0) - 1.25*e*e*sin(2.0*m))*4.0;
}


double calcSolarZenith(double latitude, double longitude, int year, int month, int day, int timeUTC){
    double T, trueSolarTime, hourAngle, solarDec, csz, zenith, exoatmElevation, te, refractionCorrection;

    T=jd2jcent(mdy2jd(year, month, day) + timeUTC/1440.0);
    trueSolarTime = timeUTC + calcEquationOfTime(T) - 4.0 * longitude;
    hourAngle = trueSolarTime / 4.0 - 180.0;
    solarDec = calcSunDeclination(T);
    csz = sin(deg2rad(latitude)) * sin(deg2rad(solarDec)) +
        cos(deg2rad(latitude)) * cos(deg2rad(solarDec)) *
        cos(deg2rad(hourAngle));
    zenith=rad2deg(acos(csz));
    exoatmElevation = 90.0 - zenith;
    if (exoatmElevation > 85.0) {
        refractionCorrection = 0.0;
    } else {
        te = tan(deg2rad(exoatmElevation));
        if (exoatmElevation > 5.0) {
            refractionCorrection = 58.1/te - 0.07/(te*te*te) +
                0.000086/(te*te*te*te*te);
        } else if (exoatmElevation > -0.575) {
            refractionCorrection = 1735.0 + exoatmElevation*(-518.2 + exoatmElevation*(103.4 + exoatmElevation*(-12.79 + exoatmElevation*0.711)));
        } else {
            refractionCorrection = -20.774 / te;
        }
        refractionCorrection = refractionCorrection / 3600.0;
    }
    return zenith - refractionCorrection;
}
