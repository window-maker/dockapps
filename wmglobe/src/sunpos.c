/*     WMGlobe 0.5  -  All the Earth on a WMaker Icon
 *     copyright (C) 1998,99 Jerome Dumonteil <jerome.dumonteil@capway.com>
 * sunpos.c is taken from Xearth :
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

#define Epsilon_g    (279.403303*(TWOPI/360))
#define OmegaBar_g   (282.768422*(TWOPI/360))
#define Eccentricity (0.016713)

/*
 * MeanObliquity gives the mean obliquity of the earth's axis at epoch
 * 1990.0 (computed as 23.440592 degrees according to the method given
 * in duffett-smith, section 27)
 */
#define MeanObliquity (23.440592*(TWOPI/360))

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
	N = fmod(N, TWOPI);
	if (N < 0)
		N += TWOPI;

	M_sun = N + Epsilon_g - OmegaBar_g;
	if (M_sun < 0)
		M_sun += TWOPI;

	E = solve_keplers_equation(M_sun);
	v = 2 * atan(sqrt((1 + Eccentricity) / (1 - Eccentricity)) * tan(E / 2));

	return (v + OmegaBar_g);
}


/*
 * convert from ecliptic to equatorial coordinates
 * (after duffett-smith, section 27)
 */
static void ecliptic_to_equatorial(double lambda, double beta, double *alpha, double *delta)
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
void GetSunPos(time_t ssue, double *lat, double *lon)
/*     time_t  ssue;               seconds since unix epoch */
/*     double *lat;                (return) latitude        */
/*     double *lon;                (return) longitude       */
{
	double lambda;
	double alpha, delta;
	double tmp;

	lambda = sun_ecliptic_longitude(ssue);
	ecliptic_to_equatorial(lambda, 0.0, &alpha, &delta);

	tmp = alpha - (TWOPI / 24) * GST(ssue);
	if (tmp < -PI) {
		do
			tmp += TWOPI;
		while (tmp < -PI);
	} else if (tmp > PI) {
		do
			tmp -= TWOPI;
		while (tmp < -PI);
	}
	*lon = tmp;
	*lat = delta;
}
