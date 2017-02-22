/* persian calendar functions original code is by Kees Couprie
   http://www.geocities.com/couprie/calmath/      */

/* islamic conversion and moonphase calculation is taken from
 *
 * hdate
 *
 * Copyright (c) 1992 by Waleed A. Muhanna
 *
 * Permission for nonprofit use and redistribution of this software and
 * its documentation is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation.
 *
 * No representation is made about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * Send any comments/suggestions/fixes/additions to:
 *		wmuhanna@magnus.acs.ohio-state.edu
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <libical/ical.h>
#define TIMZ 3.0
#define MINAGE 13.5
#define SUNSET 19.5 /*approximate */
#define TIMDIF (SUNSET-MINAGE)
#define NMONTHS  (1405*12+1)
#define RPD     (0.01745329251994329577) /* radians per degree (pi/180) */
/*general*/
int days_in_month(int month, int year, int calendar);
int day_of_week(struct icaltimetype dt);

/* related to gregorian calendar*/
struct icaltimetype get_civil(struct icaltimetype dt, int calendar);
long civil_jdn(struct icaltimetype dt);
struct icaltimetype jdn_civil(long jdn);
int isGregorianLeap(int year);
int days_in_gregorian_month(int month, int year);

/*related to persian calendar*/
long persian_jdn(struct icaltimetype dt);
struct icaltimetype jdn_persian(long jdn);
struct icaltimetype civil_persian(struct icaltimetype dt);
struct icaltimetype persian_civil(struct icaltimetype dt);
int isPersianLeap(int year);
int days_in_persian_month(int month, int year);

/*related to islamic calendar*/
long islamic_jdn(struct icaltimetype dt);
struct icaltimetype jdn_islamic(long jdn);
struct icaltimetype civil_islamic(struct icaltimetype dt);
struct icaltimetype islamic_civil(struct icaltimetype dt);
int days_in_islamic_month(int month, int year);


/*moonphase*/
int moon(struct icaltimetype dt);


/*day comparing functions*/
int daysComp(struct icaltimetype d1, struct icaltimetype d2);
int daysEqual(struct icaltimetype d1, struct icaltimetype d2);
int daysLater(struct icaltimetype d1, struct icaltimetype d2);
int daysEarlier(struct icaltimetype d1, struct icaltimetype d2);
int daysLaterEqual(struct icaltimetype d1, struct icaltimetype d2);
int daysEarlierEqual(struct icaltimetype d1, struct icaltimetype d2);

int Ceil(float number);

int     datemoon[200][2];   /* hashtable for moonphase.[jdn%32][0] stores moonphase of day jdn,
			    [jdn%32][1] stores jdn. */
