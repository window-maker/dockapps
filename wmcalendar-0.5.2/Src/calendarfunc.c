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

#include "calendarfunc.h"
#include <stdio.h>



long persian_jdn(struct icaltimetype dt)
{
    const long PERSIAN_EPOCH = 1948321; /* The JDN of 1 Farvardin 1*/
    int epbase;
    long epyear;
    long mdays;
    long jdn;
    epbase = dt.year - 474;
    epyear = 474 + (epbase % 2820);
    if (dt.month <= 7)
        mdays = (dt.month - 1) * 31;
    else
        mdays = (dt.month - 1) * 30 + 6;
    jdn = dt.day + mdays ;
    jdn += (((epyear * 682) - 110) / 2816) ;
    jdn	+= (epyear - 1) * 365;
    jdn += (epbase / 2820) * 1029983 ;
    jdn += (PERSIAN_EPOCH - 1);
    return jdn;
}

double tmoonphase( long n, int nph)
{
    double jd, t, t2, t3, k, ma, sa, tf, xtra;
    k = n + nph/4.0;  t = k/1236.85;  t2 = t*t; t3 = t2*t;
    jd =  2415020.75933 + 29.53058868*k - 1.178e-4 * t2
	- 1.55e-7 * t3
	+ 3.3e-4 * sin (RPD * (166.56 +132.87*t -0.009173*t2));
    
        /* Sun's mean anomaly */
    sa =  RPD * (359.2242 + 29.10535608*k - 3.33e-5 * t2 - 3.47e-6 * t3);
    
    /* Moon's mean anomaly */
    ma =  RPD * (306.0253 + 385.81691806*k + 0.0107306*t2 +1.236e-5 *t3);
    
    /* Moon's argument of latitude */
    tf = RPD * 2.0 * (21.2964 + 390.67050646*k -0.0016528*t2
                      -2.39e-6 * t3);
    
    /* should reduce to interval 0-1.0 before calculating further */
    if (nph==0 || nph==2)
	/* Corrections for New and Full Moon */
	xtra = (0.1734 - 0.000393*t) * sin(sa)
	    +0.0021*sin(sa*2)
	    -0.4068*sin(ma) +0.0161*sin(2*ma) -0.0004*sin(3*ma)
	    +0.0104*sin(tf)
	    -0.0051*sin(sa+ma) -0.0074*sin(sa-ma)
	    +0.0004*sin(tf+sa) -0.0004*sin(tf-sa)
	    -0.0006*sin(tf+ma) +0.0010*sin(tf-ma)
	    +0.0005*sin(sa+ 2*ma);
    else if (nph==1 || nph==3) {
	xtra = (0.1721 - 0.0004*t) * sin(sa)
	    +0.0021*sin(sa*2)
	    -0.6280*sin(ma) +0.0089*sin(2*ma) -0.0004*sin(3*ma)
	    +0.0079*sin(tf)
	    -0.0119*sin(sa+ma) -0.0047*sin(sa-ma)
	    +0.0003*sin(tf+sa) -0.0004*sin(tf-sa)
	    -0.0006*sin(tf+ma) +0.0021*sin(tf-ma)
	    +0.0003*sin(sa+ 2*ma) +0.0004*sin(sa-2*ma)
	    -0.0003*sin(2*sa+ma);
	if (nph==1)
	    xtra = xtra +0.0028 -0.0004*cos(sa) +0.0003*cos(ma);
	else
	    xtra = xtra -0.0028 +0.0004*cos(sa) -0.0003*cos(ma);
    } else {
	printf("tmoonphase: illegal phase number\n");
	exit(1);
    }
    /* convert from Ephemeris Time (ET) to (approximate)
       Universal Time (UT) */
    jd += xtra - (0.41 +1.2053*t +0.4992*t2)/1440;
    return (jd);
}

                                                       
double visible(long n,double * rjd)
{
    double jd;
    float tf;
    long d;

    jd = tmoonphase(n,0);  
    *rjd = jd;
    d = jd;
    tf = (jd - d);
    if (tf<=0.5)  /*new moon starts in the afternoon */
	return(jd+1.0);
    /* new moon starts before noon */
    tf = (tf-0.5)*24 +TIMZ;  /* local time */
    if (tf>TIMDIF) return(jd+1.0);  /*age at sunset < min for visiblity*/
    return(jd);
}


struct icaltimetype jdn_islamic(long jdn)
{
    struct icaltimetype h;
    double mjd, rjd;
    long k, hm;

    /* obtain first approx. of how many new moons since the beginning
       of the year 1900 */
    h = jdn_civil(jdn);
    k = 0.6 + (h.year + ((int) (h.month - 0.5)) / 12.0 + h.day 
	       / 365.0 - 1900) * 12.3685;
    do{
	mjd = visible(k--, &rjd);
    } while (mjd > jdn);  
    k++;
    /*first of the month is the following day*/
    hm = k - 1048;
    h.year = 1405 + (hm / 12);
    
    h.month =  (hm % 12) +1;
    if (hm != 0 && h.month <= 0) {
	h.month += 12; 
	h.year--;
    }
    if (h.year<=0) 
	h.year--;
    h.day = jdn - mjd + 1;

    return h;
}


long islamic_jdn(struct icaltimetype dt)
{
        double jd, rjd;
        long k;
       
        k = dt.month + dt.year * 12 - NMONTHS; /* # of m since 1/1/1405 */
        jd = visible(k + 1048L, &rjd) + dt.day;
        return jd;
}




struct icaltimetype jdn_persian(long jdn)
{
    struct icaltimetype ret, h;
    int iYear, iMonth, iDay;
    int depoch;
    int cycle;
    int cyear;
    int ycycle;
    int aux1, aux2;
    int yday;
    h.day = 1;
    h.month = 1;
    h.year = 475;
    depoch = jdn - persian_jdn(h);
    cycle = depoch / 1029983;
    cyear = depoch % 1029983;
    if( cyear == 1029982)
        ycycle = 2820;
    else{
        aux1 = cyear / 366;
        aux2 = cyear % 366;
        ycycle = (((2134 * aux1) + (2816 * aux2) + 2815) / 1028522) + aux1 + 1;
    }
    iYear = ycycle + (2820 * cycle) + 474;
    if (iYear <= 0)
        iYear = iYear - 1;
    h.year = iYear;
    yday = (jdn - persian_jdn(h)) + 1;
    if(yday <= 186 )
        iMonth = Ceil((yday-1) / 31);
    else
        iMonth = Ceil((yday - 7) / 30);
    iMonth++;
    h.month = iMonth;
    iDay = (jdn - persian_jdn(h)) + 1;
    ret.day = iDay;
    ret.month = iMonth;
    ret.year = iYear;
    ret.is_date = 1; 
    return ret;
}



int Ceil(float number)
{
    int ret;
    if(number>0)
	number += 0.5;
    ret = number;
    return ret;
}



long civil_jdn(struct icaltimetype dt)
{
    long jdn = ((1461 * (dt.year + 4800 + ((dt.month - 14) / 12))) / 4)
	+ ((367 * (dt.month - 2 - 12 * (((dt.month - 14) / 12)))) / 12)
	- ((3 * (((dt.year + 4900 + ((dt.month - 14) / 12)) / 100))) / 4)
	+ dt.day - 32075;
    return jdn;
}



struct icaltimetype jdn_civil(long jdn)
{
    long l, n, i, j;
    struct icaltimetype ret;
    int iday, imonth, iyear;
    l = jdn + 68569;
    n = ((4 * l) / 146097);
    l = l - ((146097 * n + 3) / 4);
    i = ((4000 * (l + 1)) / 1461001);
    l = l - ((1461 * i) / 4) + 31;
    j = ((80 * l) / 2447);
    iday = l - ((2447 * j) / 80);
    l = (j / 11);
    imonth = j + 2 - 12 * l;
    iyear = 100 * (n - 49) + i + l;
    ret.day = iday;
    ret.month = imonth;
    ret.year = iyear;
    ret.hour = 0; 
    ret.minute = 0; 
    ret.second = 0; 
    return ret;
}



struct icaltimetype civil_persian(struct icaltimetype dt)
{
    return(jdn_persian(civil_jdn(dt)));
}


struct icaltimetype civil_islamic(struct icaltimetype dt)
{
    return(jdn_islamic(civil_jdn(dt)));
}



struct icaltimetype persian_civil(struct icaltimetype dt)
{
    return(jdn_civil(persian_jdn(dt)));
}


struct icaltimetype islamic_civil(struct icaltimetype dt)
{
    return(jdn_civil(islamic_jdn(dt)));
}
   
struct icaltimetype get_civil(struct icaltimetype dt, int calendar){
    if(calendar == 0)
	return dt;
    else if(calendar == 1)
	return persian_civil(dt);
    else if(calendar == 2)
	return islamic_civil(dt);

}


int isPersianLeap(int year)
{
    struct icaltimetype dt, dold;

    dt.year = year;
    dt.month = 12;
    dt.day = 30;
    dold = jdn_civil(2453108);
    dt = civil_persian(persian_civil(dt));
    if(dt.day == 1)
	return 0;
    return 1 ;
}

int isGregorianLeap(int year)
{
    if(year % 4 !=0)
	return 0;
    if(year % 100 == 0 && year % 400 != 0)
	return 0;
    return 1;
}

int days_in_month(int month, int year, int calendar)
{
    if(calendar == 0)
	return days_in_gregorian_month(month, year);
    else if(calendar == 1)
	return days_in_persian_month(month, year);
    else if(calendar == 2)
	return days_in_islamic_month(month, year);
}



int days_in_persian_month(int month, int year)
{
    if(month < 7)
	return 31;
    if(month < 12)
	return 30;
    if(isPersianLeap(year))
	return 30;
    return 29;
}


int days_in_islamic_month(int month, int year)
{
    struct icaltimetype dt, dold;
    month++;
    if(month == 13){
	month = 1;
	year++;
    }
    dt.year = year;
    dt.month = month;
    dt.day = 1;
    dt = jdn_islamic(islamic_jdn(dt) - 1);
    return dt.day;
    
}

int days_in_gregorian_month(int month, int year)
{
    switch(month){
    case 4:
    case 6:
    case 9:
    case 11:
	return 30;
    case 2:
	return 28 + isGregorianLeap(year);
    default:
	return 31;
    }
}

int day_of_week(struct icaltimetype dt)
{
    int jd;
    jd = civil_jdn(dt);
    jd --;
    return jd % 7;
}

int moon(struct icaltimetype h)
{
    int k, jd, i ,j,  jdn;
    long mjd = 0;
    jd = civil_jdn(h);
    if(datemoon[jd%200][1] == jd)
	return datemoon[jd%200][0];

    k = 1 + (h.year + ((int) (h.month - 0.5)) / 12.0 + h.day / 365.0 - 1900) * 12.3685;
    do {
	mjd = tmoonphase(k--, 0);
    } while (mjd > jd);  

    k--;

    for(i = tmoonphase(k,0); i < tmoonphase(k+5,0 ); i++){
	datemoon[i%200][0] = 0;
	datemoon[i%200][1] = i;
    }

    for(j = 0; j < 6; j++){
	for(i = 0; i < 4; i++){
	    mjd = tmoonphase(k, i);
	    datemoon[mjd%200][0] = i+1;
	    datemoon[mjd%200][1] = mjd;
	}
	k++;
    }

    if(datemoon[jd%200][1] == jd)
	return datemoon[jd%200][0];
    return 0;
}


int daysComp(struct icaltimetype d1, struct icaltimetype d2){
    if(d1.year < d2.year)
	return 1; /* d1 < d2 */
    if(d1.year > d2.year)
	return 2; /* d1 > d2 */

    /* years are equal */
    if(d1.month < d2.month)
	return 1; /* d1 < d2 */
    if(d1.month > d2.month)
	return 2; /* d1 > d2 */

    /* months are equal */
    if(d1.day < d2.day)
	return 1; /* d1 < d2 */
    if(d1.day > d2.day)
	return 2; /* d1 > d2 */

    return 0; /* days are equal */
}


int daysEqual(struct icaltimetype d1, struct icaltimetype d2){
    if(daysComp(d1, d2) == 0)
	return 1;
    return 0;
}


int daysLater(struct icaltimetype d1, struct icaltimetype d2){
    if(daysComp(d1, d2) == 1)
	return 1;
    return 0;
}


int daysEarlier(struct icaltimetype d1, struct icaltimetype d2){
    if(daysComp(d1, d2) == 2)
	return 1;
    return 0;
}


int daysLaterEqual(struct icaltimetype d1, struct icaltimetype d2){
    if(daysComp(d1, d2) != 2)
	return 1;
    return 0;
}


int daysEarlierEqual(struct icaltimetype d1, struct icaltimetype d2){
    if(daysComp(d1, d2) != 1)
	return 1;
    return 0;
}
