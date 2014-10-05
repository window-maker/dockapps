#include <stdio.h>
#include <math.h>
#include "MoonRise.h"
#include "Moon.h"

#define DegPerRad       57.29577951308232087680
#define RadPerDeg        0.01745329251994329576

extern	double	Glon, SinGlat, CosGlat, TimeZone;


void MoonRise(int year, int month, int day, double LocalHour, double *UTRise, double *UTSet){

    double	UT, ym, y0, yp, SinH0;
    double	xe, ye, z1, z2, SinH(), hour24();
    int		Rise, Set, nz;

    SinH0 = sin( 8.0/60.0 * RadPerDeg );


    UT = 1.0+TimeZone;
    *UTRise = -999.0;
    *UTSet = -999.0;
    Rise = Set = 0;
    ym = SinH(year, month, day, UT-1.0) - SinH0;

    while ( (UT <= 24.0+TimeZone) ) {

	y0 = SinH(year, month, day, UT) - SinH0;
	yp = SinH(year, month, day, UT+1.0) - SinH0;

	Interp(ym, y0, yp, &xe, &ye, &z1, &z2, &nz);

	switch(nz){

		case 0:
			break;
		case 1:
			if (ym < 0.0){
			    *UTRise = UT + z1;
			    Rise = 1;
			} else {
			    *UTSet = UT + z1;
			    Set = 1;
			}
			break;
		case 2:
			if (ye < 0.0){
			    *UTRise = UT + z2;
			    *UTSet = UT + z1;
			} else {
			    *UTRise = UT + z1;
			    *UTSet = UT + z2;
			}
			Rise = 1;
			Set = 1;
			break;
	}
	ym = yp;
	UT += 2.0;

    }

    if (Rise){
        *UTRise -= TimeZone;
        *UTRise = hour24(*UTRise);
    } else {
        *UTRise = -999.0;
    }

    if (Set){
        *UTSet -= TimeZone;
        *UTSet = hour24(*UTSet);
    } else {
        *UTSet = -999.0;
    }

}


void UTTohhmm(double UT, int *h, int *m){


    if (UT < 0.0) {
	*h = -1.0;
	*m = -1.0;
    } else {
        *h = (int)UT;
        *m = (int)((UT-(double)(*h))*60.0+0.5);
	if (*m == 60) {
	    /*
	     *  If it was 23:60 this should become 24:00
	     *  I prefer this designation to 00:00. So dont reset h to 0 when it goes above 24.
	     */
	    *h += 1;
	    *m = 0;
	}
    }

}






void Interp(double ym, double y0, double yp, double *xe, double *ye, double *z1, double *z2, int *nz){

    double	a, b, c, d, dx;

    *nz = 0;
    a = 0.5*(ym+yp)-y0;
    b = 0.5*(yp-ym);
    c = y0;
    *xe = -b/(2.0*a);
    *ye = (a*(*xe) + b) * (*xe) + c;
    d = b*b - 4.0*a*c;

    if (d >= 0){
	dx = 0.5*sqrt(d)/fabs(a);
	*z1 = *xe - dx;
	*z2 = *xe+dx;
	if (fabs(*z1) <= 1.0) *nz += 1;
	if (fabs(*z2) <= 1.0) *nz += 1;
	if (*z1 < -1.0) *z1 = *z2;
    }



}




double SinH(int year, int month, int day, double UT){

    double	TU, frac(), jd();
    double	RA_Moon, DEC_Moon, gmst, lmst, Tau, Moon();
    double	angle2pi();

    TU = (jd(year, month, day, UT) - 2451545.0)/36525.0;

    /* this is more accurate, but wasteful for this -- use low res approx.
    TU2 = TU*TU;
    TU3 = TU2*TU;
    Moon(TU, &LambdaMoon, &BetaMoon, &R, &AGE);
    LambdaMoon *= RadPerDeg;
    BetaMoon *= RadPerDeg;
    epsilon = (23.43929167 - 0.013004166*TU - 1.6666667e-7*TU2 - 5.0277777778e-7*TU3)*RadPerDeg;
    RA_Moon  = angle2pi(atan2(sin(LambdaMoon)*cos(epsilon)-tan(BetaMoon)*sin(epsilon), cos(LambdaMoon)));
    DEC_Moon = asin( sin(BetaMoon)*cos(epsilon) + cos(BetaMoon)*sin(epsilon)*sin(LambdaMoon));
    */

    MiniMoon(TU, &RA_Moon, &DEC_Moon);
    RA_Moon *= 15.0*RadPerDeg;
    DEC_Moon *= RadPerDeg;

    /*
     *  Compute Greenwich Mean Sidereal Time (gmst)
     */
    UT = 24.0*frac( UT/24.0 );
    /* this is for the ephemeris meridian???
    gmst = 6.697374558 + 1.0027379093*UT + (8640184.812866+(0.093104-6.2e-6*TU)*TU)*TU/3600.0;
    */
    gmst = UT + 6.697374558 + (8640184.812866+(0.093104-6.2e-6*TU)*TU)*TU/3600.0;
    lmst = 24.0*frac( (gmst-Glon/15.0) / 24.0 );

    Tau = 15.0*lmst*RadPerDeg - RA_Moon;
    return( SinGlat*sin(DEC_Moon) + CosGlat*cos(DEC_Moon)*cos(Tau) );


}

