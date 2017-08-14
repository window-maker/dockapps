#include "CalcEphem.h"
#include <math.h>
#include <string.h>

void CalcEphem(date, UT, c)
long int     	 date;	/* integer containing the date (e.g. 960829) */
double  	 UT; 	/* Universal Time */
CTrans 		*c;	/* structure containing all the relevent coord trans info */
{

    int    year, month, day;
    double TU, TU2, TU3, T0;
    double varep, varpi;
    double eccen, epsilon;
    double days, M, E, nu, lambnew;
    double r0, earth_sun_distance;
    double RA, DEC, RA_Moon, DEC_Moon;
    double TDT, AGE, LambdaMoon, BetaMoon, R;
    double jd(), hour24(), angle2pi(), angle360(), kepler(), Moon(), NewMoon();
    double Ta, Tb, Tc, frac();
    double SinGlat, CosGlat, Tau, lmst, x, y, z;
    double SinTau, CosTau, SinDec, CosDec;





    c->UT = UT;
    year = (int)(date/10000);
    month = (int)( (date - year*10000)/100 );
    day = (int)( date - year*10000 - month*100 );
    c->year = year;
    c->month = month;
    c->day = day;

    c->doy = DayofYear(year, month, day);
    c->dow = DayofWeek(year, month, day, c->dowstr);



    /*
     *  Compute Greenwich Mean Sidereal Time (gmst)
     *  The TU here is number of Julian centuries
     *  since 2000 January 1.5
     *  From the 1996 astronomical almanac
     */
    TU = (jd(year, month, day, 0.0) - 2451545.0)/36525.0;
    TU2 = TU*TU;
    TU3 = TU2*TU;
    T0 = (6.0 + 41.0/60.0 + 50.54841/3600.0) + 8640184.812866/3600.0*TU
            + 0.093104/3600.0*TU2 - 6.2e-6/3600.0*TU3;
    T0 = hour24(T0);
    c->gmst = hour24(T0 + UT*1.002737909);

    lmst = 24.0*frac( (c->gmst - c->Glon/15.0) / 24.0 );






    /*
     *
     *   Construct Transformation Matrix from GEI to GSE  systems
     *
     *
     *   First compute:
     *          mean ecliptic longitude of sun at epoch TU (varep)
     *          elciptic longitude of perigee at epoch TU (varpi)
     *          eccentricity of orbit at epoch TU (eccen)
     *
     *   The TU here is the number of Julian centuries since
     *   1900 January 0.0 (= 2415020.0)
     */
    TDT = UT + 59.0/3600.0;
    TU = (jd(year, month, day, TDT) - 2415020.0)/36525.0;
    varep = (279.6966778 + 36000.76892*TU + 0.0003025*TU*TU)*RadPerDeg;
    varpi = (281.2208444 + 1.719175*TU + 0.000452778*TU*TU)*RadPerDeg;
    eccen = 0.01675104 - 0.0000418*TU - 0.000000126*TU*TU;
    c->eccentricity = eccen;



    /*
     * Compute the Obliquity of the Ecliptic at epoch TU
     * The TU in this formula is the number of Julian
     * centuries since epoch 2000 January 1.5
     */
    TU  = (jd(year, month, day, TDT) - jd(2000, 1, 1, 12.0))/36525.0;
    epsilon = (23.43929167 - 0.013004166*TU - 1.6666667e-7*TU*TU
                - 5.0277777778e-7*TU*TU*TU)*RadPerDeg;
    c->epsilon = epsilon;


    /*
     * Compute:
     *          Number of Days since epoch 1990.0 (days)
     *          The Mean Anomaly (M)
     *          The True Anomaly (nu)
     *	    The Eccentric Anomaly via Keplers equation (E)
     *
     *
     */
    days  = jd(year, month, day, TDT) - jd(year, month, day, TDT);
    M = angle2pi(2.0*M_PI/365.242191*days + varep - varpi);
    E = kepler(M, eccen);
    nu = 2.0*atan( sqrt((1.0+eccen)/(1.0-eccen))*tan(E/2.0) );
    lambnew = angle2pi(nu + varpi);
    c->lambda_sun = lambnew;


    /*
     *  Compute distance from earth to the sun
     */
    r0 = 1.495985e8;  /* in km */
    earth_sun_distance  = r0*(1-eccen*eccen)/(1.0 + eccen*cos(nu))/6371.2;
    c->earth_sun_dist = earth_sun_distance;





    /*
     * Compute Right Ascension and Declination of the Sun
     */
    RA = angle360(atan2(sin(lambnew)*cos(epsilon), cos(lambnew))*180.0/M_PI);
    DEC = asin(sin(epsilon)*sin(lambnew))*180.0/M_PI;
    c->RA_sun = RA;
    c->DEC_sun = DEC;






    /*
     * Compute Moon Phase and AGE Stuff. The AGE that comes out of Moon()
     * is actually the Phase converted to days. Since AGE is actually defined
     * to be time since last NewMoon, we need to figure out what the JD of the
     * last new moon was. Thats done below....
     */
    TU = (jd(year, month, day, TDT) - 2451545.0)/36525.0;
    c->MoonPhase = Moon(TU, &LambdaMoon, &BetaMoon, &R, &AGE);
    LambdaMoon *= RadPerDeg;
    BetaMoon *= RadPerDeg;


    RA_Moon  = angle360(atan2(sin(LambdaMoon)*cos(epsilon)-tan(BetaMoon)*sin(epsilon), cos(LambdaMoon))*DegPerRad);
    DEC_Moon = asin( sin(BetaMoon)*cos(epsilon) + cos(BetaMoon)*sin(epsilon)*sin(LambdaMoon))*DegPerRad;
    c->RA_moon = RA_Moon;
    c->DEC_moon = DEC_Moon;


    /*
     *  Compute Alt/Az coords
     */
    Tau = (15.0*lmst - RA_Moon)*RadPerDeg;
    CosGlat = cos(c->Glat*RadPerDeg); SinGlat = sin(c->Glat*RadPerDeg);
    CosTau = cos(Tau); SinTau = sin(Tau);
    SinDec = sin(DEC_Moon*RadPerDeg); CosDec = cos(DEC_Moon*RadPerDeg);
    x = CosDec*CosTau*SinGlat - SinDec*CosGlat;
    y = CosDec*SinTau;
    z = CosDec*CosTau*CosGlat + SinDec*SinGlat;
    c->A_moon = DegPerRad*atan2(y, x);
    c->h_moon = DegPerRad*asin(z);
    c->Visible = (c->h_moon < 0.0) ? 0 : 1;



    /*
     * Compute accurate AGE of the Moon
     */
    Tb = TU - AGE/36525.0; /* should be very close to minimum */
    Ta = Tb - 0.4/36525.0;
    Tc = Tb + 0.4/36525.0;
    c->MoonAge = (TU - NewMoon(Ta, Tb, Tc))*36525.0;



    /*
     * Compute Earth-Moon distance
     */
    c->EarthMoonDistance = R;



}





double kepler(M, e)
double M, e;
{
        int n=0;
        double E, Eold, eps = 1.0e-8;



        E = M + e*sin(M);
        do{
                Eold = E;
                E = Eold + (M-Eold+e*sin(Eold))
                        /(1.0-e*cos(Eold));
                ++n;
        }while((fabs(E-Eold) > eps) && (n < 100));
        return(E);
}




int DayofYear(year, month, day)
int year, month, day;
{
	double jd();
        return((int)(jd(year, month, day, 0.0) - jd(year, 1, 0, 0.0)));
}




int DayofWeek(year, month, day, dowstr)
int year, month, day;
char dowstr[];
{
        double JD, A, Afrac, jd();
        int n, iA;

        JD = jd(year, month, day, 0.0);
        A = (JD + 1.5)/7.0;
        iA = (int)A;
        Afrac = A - (double)iA;
        n = (int)(Afrac*7.0 + 0.5);
        switch(n){
                case 0:
                        strcpy(dowstr, "Sunday");
                        break;
                case 1:
                        strcpy(dowstr, "Monday");
                        break;
                case 2:
                        strcpy(dowstr, "Tuesday");
                        break;
                case 3:
                        strcpy(dowstr, "Wednesday");
                        break;
                case 4:
                        strcpy(dowstr, "Thursday");
                        break;
                case 5:
                        strcpy(dowstr, "Friday");
                        break;
                case 6:
                        strcpy(dowstr, "Saturday");
                        break;
        }
	return(n);
}





/*
 *  Compute the Julian Day number for the given date.
 *  Julian Date is the number of days since noon of Jan 1 4713 B.C.
 */
double jd(ny, nm, nd, UT)
int ny, nm, nd;
double UT;
{
        double A, B, C, D, JD, day;

        day = nd + UT/24.0;


        if ((nm == 1) || (nm == 2)){
                ny = ny - 1;
                nm = nm + 12;
        }

        if (((double)ny+nm/12.0+day/365.25)>=(1582.0+10.0/12.0+15.0/365.25)){
                        A = ((int)(ny / 100.0));
                        B = 2.0 - A + (int)(A/4.0);
        }
        else{
                        B = 0.0;
        }

        if (ny < 0.0){
                C = (int)((365.25*(double)ny) - 0.75);
        }
        else{
                C = (int)(365.25*(double)ny);
        }

        D = (int)(30.6001*(double)(nm+1));


        JD = B + C + D + day + 1720994.5;
        return(JD);

}

double hour24(hour)
double hour;
{
        int n;

        if (hour < 0.0){
                n = (int)(hour/24.0) - 1;
                return(hour-n*24.0);
        }
        else if (hour > 24.0){
                n = (int)(hour/24.0);
                return(hour-n*24.0);
        }
        else{
                return(hour);
        }
}

double angle2pi(angle)
double angle;
{
        int n;
        double a;
        a = 2.0*M_PI;

        if (angle < 0.0){
                n = (int)(angle/a) - 1;
                return(angle-n*a);
        }
        else if (angle > a){
                n = (int)(angle/a);
                return(angle-n*a);
        }
        else{
                return(angle);
        }
}

double angle360(angle)
double angle;
{
        int n;

        if (angle < 0.0){
                n = (int)(angle/360.0) - 1;
                return(angle-n*360.0);
        }
        else if (angle > 360.0){
                n = (int)(angle/360.0);
                return(angle-n*360.0);
        }
        else{
                return(angle);
        }
}


void Radec_to_Cart(ra, dec, r)
double  ra, dec;	/* RA and DEC */
Vector *r;		/* returns corresponding cartesian unit vector */
{

    /*
     *  Convert ra/dec from degrees to radians
     */
    ra  *= RadPerDeg;
    dec *= RadPerDeg;


    /*
     *  Compute cartesian coordinates (in GEI)
     */
    r->x = cos(dec) * cos(ra);
    r->y = cos(dec) * sin(ra);
    r->z = sin(dec);

}





int LeapYear(year)
int year;
{
    if ((year%100 == 0)&&(year%400 != 0)) return(0);
    else if (year%4 == 0) return(1);
    else return(0);
}




