/*     WMGlobe 1.3  -  All the Earth on a WMaker Icon
 *     copyright (C) 1998,99,2000,01 Jerome Dumonteil <jerome.dumonteil@linuxfr.org>
 * sunpos.c is taken from Xearth 1.0 (and part of 1.1):
 */
/*
 * sunpos.c
 * kirk johnson
 * july 1993
 *
 * code for calculating the position on the earth's surface for which
 * the sun is directly overhead (adapted from _practical astronomy
 * with your calculator, third edition_, peter duffett-smith,
 * cambridge university press, 1988.)
 *
 *
 * Copyright (C) 1989, 1990, 1993, 1994, 1995 Kirk Lauritz Johnson
 *
 * Parts of the source code (as marked) are:
 *   Copyright (C) 1989, 1990, 1991 by Jim Frost
 *   Copyright (C) 1992 by Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify and freely distribute xearth for
 * non-commercial and not-for-profit purposes is hereby granted
 * without fee, provided that both the above copyright notice and this
 * permission notice appear in all copies and in supporting
 * documentation.
 *
 * Unisys Corporation holds worldwide patent rights on the Lempel Zev
 * Welch (LZW) compression technique employed in the CompuServe GIF
 * image file format as well as in other formats. Unisys has made it
 * clear, however, that it does not require licensing or fees to be
 * paid for freely distributed, non-commercial applications (such as
 * xearth) that employ LZW/GIF technology. Those wishing further
 * information about licensing the LZW patent should contact Unisys
 * directly at (lzw_info@unisys.com) or by writing to
 *
 *   Unisys Corporation
 *   Welch Licensing Department
 *   M/S-C1SW19
 *   P.O. Box 500
 *   Blue Bell, PA 19424
 *
 * The author makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT
 * OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*************************************************************************/

#include <math.h>
#include <time.h>

#ifndef PI
#define PI 3.141592653
#endif
#define TWOPI (2*PI)
#define DegsToRads(x) ((x)*(TWOPI/360))

/*
 * the epoch upon which these astronomical calculations are based is
 * 1990 january 0.0, 631065600 seconds since the beginning of the
 * "unix epoch" (00:00:00 GMT, Jan. 1, 1970)
 *
 * given a number of seconds since the start of the unix epoch,
 * DaysSinceEpoch() computes the number of days since the start of the
 * astronomical epoch (1990 january 0.0)
 */

#define EpochStart           (631065600)
#define DaysSinceEpoch(secs) (((secs)-EpochStart)*(1.0/(24*3600)))

/*
 * assuming the apparent orbit of the sun about the earth is circular,
 * the rate at which the orbit progresses is given by RadsPerDay --
 * TWOPI radians per orbit divided by 365.242191 days per year:
 */

#define RadsPerDay (TWOPI/365.242191)

/*
 * details of sun's apparent orbit at epoch 1990.0 (after
 * duffett-smith, table 6, section 46)
 *
 * Epsilon_g    (ecliptic longitude at epoch 1990.0) 279.403303 degrees
 * OmegaBar_g   (ecliptic longitude of perigee)      282.768422 degrees
 * Eccentricity (eccentricity of orbit)                0.016713
 */

#define Epsilon_g    (DegsToRads(279.403303))
#define OmegaBar_g   (DegsToRads(282.768422))
#define Eccentricity (0.016713)

/*
 * MeanObliquity gives the mean obliquity of the earth's axis at epoch
 * 1990.0 (computed as 23.440592 degrees according to the method given
 * in duffett-smith, section 27)
 */
#define MeanObliquity (DegsToRads(23.440592))

/*
 * Lunar parameters, epoch January 0, 1990.0 (from Xearth 1.1)
 */
#define MoonMeanLongitude        DegsToRads(318.351648)
#define MoonMeanLongitudePerigee DegsToRads( 36.340410)
#define MoonMeanLongitudeNode    DegsToRads(318.510107)
#define MoonInclination          DegsToRads(  5.145396)

#define SideralMonth (27.3217)

/*
 * Force an angular value into the range [-PI, +PI]
 */
#define Normalize(x)                        \
  do {                                      \
    if ((x) < -PI)                          \
      do (x) += TWOPI; while ((x) < -PI);   \
    else if ((x) > PI)                      \
      do (x) -= TWOPI; while ((x) > PI);    \
  } while (0)

static double solve_keplers_equation(double M);
static double mean_sun(double D);
static double sun_ecliptic_longitude(time_t ssue);
static void ecliptic_to_equatorial(double lambda, double beta,
				   double *alpha, double *delta);
static double julian_date(int y, int m, int d);
static double GST(time_t ssue);

/*
 * solve Kepler's equation via Newton's method
 * (after duffett-smith, section 47)
 */
static double solve_keplers_equation(double M)
{
    double E;
    double delta;

    E = M;
    while (1) {
	delta = E - Eccentricity * sin(E) - M;
	if (fabs(delta) <= 1e-10)
	    break;
	E -= delta / (1 - Eccentricity * cos(E));
    }
    return E;
}

/*
 * Calculate the position of the mean sun: where the sun would
 * be if the earth's orbit were circular instead of ellipictal.
 */

static double mean_sun(double D)
/*     double D;                   days since ephemeris epoch */
{
    double N, M;

    N = RadsPerDay * D;
    N = fmod(N, TWOPI);
    if (N < 0)
	N += TWOPI;

    M = N + Epsilon_g - OmegaBar_g;
    if (M < 0)
	M += TWOPI;
    return M;
}

/*
 * compute ecliptic longitude of sun (in radians)
 * (after duffett-smith, section 47)
 */
static double sun_ecliptic_longitude(time_t ssue)
				/* seconds since unix epoch */
{
    double D, N;
    double M_sun, E;
    double v;

    D = DaysSinceEpoch(ssue);

    N = RadsPerDay * D;
    M_sun = mean_sun(D);
    E = solve_keplers_equation(M_sun);
    v = 2 * atan(sqrt((1 + Eccentricity) / (1 - Eccentricity)) *
		 tan(E / 2));

    return (v + OmegaBar_g);
}


/*
 * convert from ecliptic to equatorial coordinates
 * (after duffett-smith, section 27)
 */
static void ecliptic_to_equatorial(double lambda, double beta,
				   double *alpha, double *delta)
/*
 *    double  lambda;            ecliptic longitude
 *    double  beta;               ecliptic latitude
 *    double *alpha;              (return) right ascension
 *    double *delta;           (return) declination
 */
{
    double sin_e, cos_e;

    sin_e = sin(MeanObliquity);
    cos_e = cos(MeanObliquity);

    *alpha = atan2(sin(lambda) * cos_e - tan(beta) * sin_e, cos(lambda));
    *delta = asin(sin(beta) * cos_e + cos(beta) * sin_e * sin(lambda));
}


/*
 * computing julian dates (assuming gregorian calendar, thus this is
 * only valid for dates of 1582 oct 15 or later)
 * (after duffett-smith, section 4)
 */
static double julian_date(int y, int m, int d)
/*
 *     int y;                     year (e.g. 19xx)
 *     int m;                      month (jan=1, feb=2, ...)
 *     int d;                      day of month
 */
{
    int A, B, C, D;
    double JD;

    /* lazy test to ensure gregorian calendar */
/*
 *   ASSERT(y >= 1583);
 */

    if ((m == 1) || (m == 2)) {
	y -= 1;
	m += 12;
    }
    A = y / 100;
    B = 2 - A + (A / 4);
    C = (int) (365.25 * y);
    D = (int) (30.6001 * (m + 1));

    JD = B + C + D + d + 1720994.5;

    return JD;
}


/*
 * compute greenwich mean sidereal time (GST) corresponding to a given
 * number of seconds since the unix epoch
 * (after duffett-smith, section 12)
 */
static double GST(time_t ssue)
   /*time_t ssue;          seconds since unix epoch */
{
    double JD;
    double T, T0;
    double UT;
    struct tm *tm;

    tm = gmtime(&ssue);

    JD = julian_date(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    T = (JD - 2451545) / 36525;

    T0 = ((T + 2.5862e-5) * T + 2400.051336) * T + 6.697374558;

    T0 = fmod(T0, 24.0);
    if (T0 < 0)
	T0 += 24;

    UT = tm->tm_hour + (tm->tm_min + tm->tm_sec / 60.0) / 60.0;

    T0 += UT * 1.002737909;
    T0 = fmod(T0, 24.0);
    if (T0 < 0)
	T0 += 24;

    return T0;
}


/*
 * given a particular time (expressed in seconds since the unix
 * epoch), compute position on the earth (lat, lon) such that sun is
 * directly overhead.
 */
void sun_position(time_t ssue, double *lat, double *lon)
/*     time_t  ssue;               seconds since unix epoch    */
/*     double *lat;                (return) latitude  in rad   */
/*     double *lon;                (return) longitude in rad   */
{
    double lambda;
    double alpha, delta;
    double tmp;

    lambda = sun_ecliptic_longitude(ssue);
    ecliptic_to_equatorial(lambda, 0.0, &alpha, &delta);

    tmp = alpha - (TWOPI / 24) * GST(ssue);
    Normalize(tmp);
    *lon = tmp;
    *lat = delta;
}

/*
 * given a particular time (expressed in seconds since the unix
 * epoch), compute position on the earth (lat, lon) such that the
 * moon is directly overhead.
 *
 * Based on duffett-smith **2nd ed** section 61; combines some steps
 * into single expressions to reduce the number of extra variables.
 */
void moon_position(time_t ssue, double *lat, double *lon)
/*    time_t  ssue;              seconds since unix epoch    */
/*   double *lat;                (return) latitude   in ra   */
/*  double *lon;                 (return) longitude   in ra  */
{
    double lambda, beta;
    double D, L, Ms, Mm, N, Ev, Ae, Ec, alpha, delta;

    D = DaysSinceEpoch(ssue);
    lambda = sun_ecliptic_longitude(ssue);
    Ms = mean_sun(D);

    L = fmod(D / SideralMonth, 1.0) * TWOPI + MoonMeanLongitude;
    Normalize(L);
    Mm = L - DegsToRads(0.1114041 * D) - MoonMeanLongitudePerigee;
    Normalize(Mm);
    N = MoonMeanLongitudeNode - DegsToRads(0.0529539 * D);
    Normalize(N);
    Ev = DegsToRads(1.2739) * sin(2.0 * (L - lambda) - Mm);
    Ae = DegsToRads(0.1858) * sin(Ms);
    Mm += Ev - Ae - DegsToRads(0.37) * sin(Ms);
    Ec = DegsToRads(6.2886) * sin(Mm);
    L += Ev + Ec - Ae + DegsToRads(0.214) * sin(2.0 * Mm);
    L += DegsToRads(0.6583) * sin(2.0 * (L - lambda));
    N -= DegsToRads(0.16) * sin(Ms);

    L -= N;
    lambda = (fabs(cos(L)) < 1e-12) ?
	(N + sin(L) * cos(MoonInclination) * PI / 2) :
	(N + atan2(sin(L) * cos(MoonInclination), cos(L)));
    Normalize(lambda);
    beta = asin(sin(L) * sin(MoonInclination));
    ecliptic_to_equatorial(lambda, beta, &alpha, &delta);
    alpha -= (TWOPI / 24) * GST(ssue);
    Normalize(alpha);
    *lon = alpha;
    *lat = delta;
}
