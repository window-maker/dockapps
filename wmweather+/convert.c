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

#include <math.h>
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
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "convert.h"

/*
 * To indicate unavailable data
 *   999 is used for temperature
 *   x<0 is used for rh, pressure, and windspeed
 */


/* Calculations */

int rh_C(int temp_C, int dewpt_C){
    float f;
    
    if(temp_C==999 || dewpt_C==999) return 999;

    f=1782.75*(dewpt_C-temp_C)/((237.7+dewpt_C)*(237.7+temp_C));
    return round(pow(10, f+2));
}

int rh_F(int temp_F, int dewpt_F){
    float f;
    
    if(temp_F==999 || dewpt_F==999) return 999;

    f=3208.95*(dewpt_F-temp_F)/((395.86+dewpt_F)*(395.86+temp_F));
    return round(pow(10, f+2));
}

int heatindex_C(int temp_C, int rh){
#if 1
    if(temp_C==999 || temp_C<21 || rh<0) return 999;
    return heatindex_F(temp_C2F(temp_C), rh);
#else
    int temp2, rh2; 
    
    if(temp_C==999 || temp_C<38 || rh<0) return 999;

    temp2=temp_C*temp_C;
    rh2=rh*rh;
    return round(16.18754948 + 2.900509394*temp_C - 0.0221545692*temp2 + 4.20938791*rh - 0.26300889*temp_C*rh + 0.0039811176*temp2*rh - 0.02956469*rh2 + 0.001305828*temp_C*rh2 - 6.4476e-06*temp2*rh2);
#endif
}

int heatindex_F(int temp_F, int rh){
    int temp2, temp3, rh2, rh3;

    if(temp_F==999 || temp_F<70 || rh<0) return 999;

    temp2=temp_F*temp_F;
    temp3=temp2*temp_F;
    rh2=rh*rh;
    rh3=rh2*rh;
    return round(16.923 + .185212*temp_F + 5.37941*rh - .100254*temp_F*rh + (9.41695e-3)*temp2 + (7.28898e-3)*rh2 + (3.45372e-4)*temp2*rh - (8.14971e-4)*temp_F*rh2 + (1.02102e-5)*temp2*rh2 - (3.8646e-5)*temp3 + (2.91583e-5)*rh3 + (1.42721e-6)*temp3*rh + (1.97483e-7)*temp_F*rh3 - (2.18429e-8)*temp3*rh2 + (8.43296e-10)*temp2*rh3 - (4.81975e-11)*temp3*rh3);
#if 0
    return round(-42.379 + 2.04901523*temp_F + 10.14333127*rh - 0.22475541*temp_F*rh - .00683783*temp2 - .05481717*rh2 + .00122874*temp2*rh + .00085282*temp_F*rh2 - .00000199*temp2*rh2);
#endif
}

int windchill_C(int temp_C, int windspeed){
    if(temp_C==999 || windspeed<0) return 999;

    return windchill_F(temp_C2F(temp_C), windspeed);
}

int windchill_F(int temp_F, int windspeed){
    double ret;
    if(temp_F==999 || windspeed<0) return 999;

    ret=35.74 + 0.6215*temp_F + (-35.75 + 0.4275*temp_F)*pow(windspeed*50292/57875.0, 0.16);
    if(ret>temp_F) return temp_F;
    return round(ret);
}

/* Length Conversions */

int in2cm(int in){
    if(in<0) return in;
    return round(in*2.54);
}

float m2mi(int meters){
    if(meters<0) return meters;
    return meters*125/201168;
}

/* Windspeed Conversions */

int knots2mph(int knots){
    if(knots<0) return knots;
    return round(knots*57875/50292.0);
}

int knots2kph(int knots){
    if(knots<0) return knots;
    return round(knots*463/250.0);
}

int kph2knots(int kph){
    if(kph<0) return kph;
    return round(kph*250/463.0);
}

int knots2mps(int knots){
    if(knots<0) return knots;
    return round(knots*463/900.0);
}

int mps2knots(int mps){
    if(mps<0) return mps;
    return round(mps*900/463.0);
}

int knots2beaufort(int knots){
    if(knots<0) return knots;
    if(knots<1) return 0;
    if(knots<=3) return 1;
    if(knots<=6) return 2;
    if(knots<=10) return 3;
    if(knots<=16) return 4;
    if(knots<=21) return 5;
    if(knots<=27) return 6;
    if(knots<=33) return 7;
    if(knots<=40) return 8;
    if(knots<=47) return 9;
    if(knots<=55) return 10;
    if(knots<=63) return 11;
    return 12;
}


/* Temperature Conversions */

int temp_C2F(int temp_C){
    if(temp_C==999) return 999;
    return round(temp_C*9/5.0+32);
}

int temp_F2C(int temp_F){
    if(temp_F==999) return 999;
    return round((temp_F-32)*5/9.0);
}


/* Pressure Conversions */

float inHg2mmHg(float inHg){
    if(inHg<0) return inHg;
    return inHg*25.4;
}

float inHg2hPa(float inHg){
    if(inHg<0) return inHg;
    return inHg*33.8639;
}

float inHg2atm(float inHg){
    if(inHg<0) return inHg;
    return inHg*.033421052632;
}

float hPa2inHg(float hPa){
    if(hPa<0) return hPa;
    return hPa/33.8639;
}


/* Time Conversions */

/* NOTE: y%400==100 because y=year-1900 */
#define is_leap(y) (y%4==0 && (y%100!=0 || y%400==100))

/* mktime for UTC, more or less.
 * Differences:
 *   - no range checking
 *   - never recalculates tm_wday or tm_yday
 */
time_t mkgmtime(struct tm *tm){
    static long msec[]={0, 2678400, 5097600, 7776000, 10368000, 13046400, 15638400, 18316800, 20995200, 23587200, 26265600, 28857600};
    time_t t;
    int i;

    t=0;
    if(tm->tm_year>70){
        for(i=70; i<tm->tm_year; i++){
            t+=31536000;
            if(is_leap(i)) t+=86400;
        }
    } else if(tm->tm_year<70){
        for(i=69; i>=tm->tm_year; i--){
            t-=31536000;
            if(is_leap(i)) t-=86400;
        }
    }
    t+=msec[tm->tm_mon];
    if(tm->tm_mon>1 && is_leap(tm->tm_year)) t+=86400;
    t+=(((tm->tm_mday-1)*24+tm->tm_hour)*60+tm->tm_min)*60+tm->tm_sec;

    return t;
}

int utc2local(int hm, int *month, int *day, int *year, int *wday){
    time_t t=time(NULL);
    struct tm *tm;
    
    tm=gmtime(&t);
    tm->tm_hour=hm/100;
    tm->tm_min=hm%100;
    if(month!=NULL && *month!=-1) tm->tm_mon=*month-1;
    if(day!=NULL && *day!=-1) tm->tm_mday=*day;
    if(year!=NULL && *year!=-1) tm->tm_year=*year;

    t=mkgmtime(tm);
    tm=localtime(&t);

    if(month!=NULL) *month=tm->tm_mon+1;
    if(day!=NULL) *day=tm->tm_mday;
    if(year!=NULL) *year=tm->tm_year;
    if(wday!=NULL) *wday=tm->tm_wday;
    return tm->tm_hour*100+tm->tm_min;
}

int local2utc(int hm, int *month, int *day, int *year, int *wday){
    time_t t=time(NULL);
    struct tm *tm;
    
    tm=localtime(&t);
    tm->tm_hour=hm/100;
    tm->tm_min=hm%100;
    if(month!=NULL && *month!=-1) tm->tm_mon=*month-1;
    if(day!=NULL && *day!=-1) tm->tm_mday=*day;
    if(year!=NULL && *year!=-1) tm->tm_year=*year;

    t=mktime(tm);
    tm=gmtime(&t);

    if(month!=NULL) *month=tm->tm_mon+1;
    if(day!=NULL) *day=tm->tm_mday;
    if(year!=NULL) *year=tm->tm_year;
    if(wday!=NULL) *wday=tm->tm_wday;
    return tm->tm_hour*100+tm->tm_min;
}

void fix_date(int *month, int *day, int *year, int *wday){
    time_t t=time(NULL);
    struct tm *tm;
    
    tm=gmtime(&t);
    if(month!=NULL && *month!=-1) tm->tm_mon=*month-1;
    if(day!=NULL && *day!=-1) tm->tm_mday=*day;
    if(year!=NULL && *year!=-1) tm->tm_year=*year;

    t=mkgmtime(tm);
    tm=gmtime(&t);

    if(month!=NULL) *month=tm->tm_mon+1;
    if(day!=NULL) *day=tm->tm_mday;
    if(year!=NULL) *year=tm->tm_year;
    if(wday!=NULL) *wday=tm->tm_wday;
}

int hm2min(int hm){
    return hm/100*60+hm%100;
}

/* Letter Case (destructive!) */

char *str_upper(char *str){
    char *c;

    for(c=str; *c!='\0'; c++){
        *c=toupper(*c);
    }
    return str;
}

char *str_lower(char *str){
    char *c;

    for(c=str; *c!='\0'; c++){
        *c=tolower(*c);
    }
    return str;
}


/* Angle conversions */

/* Convert radian angle to degrees */
double rad2deg(double angle) {
    return 180.0*angle/PI;
}

/* Convert degree angle to radians */
double deg2rad(double angle) {
    return PI*angle/180.0;
}


/* Date conversions */

/*  Numerical day-of-year from month, day and year */
int mdy2doy(int mn, int dy, int y) {
    return 275*mn/9 - ((y%4==0 && (y%100!=0 || y%400==100))?1:2)*(mn + 9)/12 + dy-30;
}

/* Julian day from month/day/year */
double mdy2jd(int year, int month, int day) {
    int A, B;

    year+=1900;
    if (month <= 2) {
        year -= 1;
        month += 12;
    }
    A=year/100;
    B=2 - A + A/4;

    return (int)(365.25*(year + 4716)) + (int)(30.6001*(month+1)) + day + B - 1524.5;
}

/* convert Julian Day to centuries since J2000.0. */
double jd2jcent(double jd) {
    return (jd - 2451545.0)/36525.0;
}

/* convert centuries since J2000.0 to Julian Day. */
double jcent2jd(double t) {
    return t * 36525.0 + 2451545.0;
}


/* Lat/Long conversions */

static double parse_dd_or_dms(char *s, char **e){
    double deg;

    *e=s;
    if(strchr(s, 'x') || strchr(s, 'X')) return NAN;
    if(!strchr(s, '\'')){
        if(!isdigit(*s) && *s!='.') return NAN;
        return strtod(s, e);
    }
    
    if(!isdigit(*s)) return NAN;
    deg=strtol(s, e, 10);
    if(*e==s || *e==NULL || **e!='\'') return deg;
    s=++(*e);
    if(!isdigit(*s)) return deg;
    deg+=strtol(s, e, 10)/60.0;
    if(*e==s || *e==NULL || **e!='\'') return deg;
    s=++(*e);
    if(!isdigit(*s) && *s!='.') return NAN;
    deg+=strtod(s, e)/3600.0;
    if(*e!=s && *e!=NULL && **e=='\'') (*e)++;
    return deg;
}

int str2dd(char *s, double *lat, double *lon){
    char *e;
    int dir=0;
    char c;

    c=toupper(*s);
    if(c=='+' || c=='N'){
        s++; dir=1;
    }
    if(c=='-' || c=='S'){
        s++; dir=-1;
    }

    *lat=parse_dd_or_dms(s, &e);
    if(isnan(*lat) || e==NULL || e==s || *e=='\0') return 0;
    if(!dir){
        c=toupper(*e);
        if(c=='N') dir=1;
        if(c=='S') dir=-1;
        if(dir) e++;
    }
    if(dir<0) *lat=-*lat;

    while(isspace(*e)) e++;
    if(*e=='\0') return 0;

    s=e; dir=0;
    c=toupper(*s);
    if(c=='+' || c=='W'){
        s++; dir=1;
    }
    if(c=='-' || c=='E'){
        s++; dir=-1;
    }

    *lon=parse_dd_or_dms(s, &e);
    if(isnan(*lon) || e==s) return 0;
    if(e==NULL || *e=='\0') return 1;
    if(dir==0){
        c=toupper(*e);
        if(c=='W') dir=1;
        if(c=='E') dir=-1;
        if(dir!=0) e++;
    }
    if(dir<0) *lon=-*lon;

    return (*e=='\0');
}
