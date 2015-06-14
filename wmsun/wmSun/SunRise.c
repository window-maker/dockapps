#include <stdio.h>
#include <math.h>

#define DegPerRad       57.29577951308232087680
#define RadPerDeg        0.01745329251994329576

extern	double	Glon, SinGlat, CosGlat, TimeZone;

double    cosEPS = 0.91748;
double    sinEPS = 0.39778;
double    P2  = 6.283185307;


SunRise(int year, int month, int day, double LocalHour, double *UTRise, double *UTSet){

    double	UT, ym, y0, yp, SinH0;
    double	xe, ye, z1, z2, SinH(), hour24();
    int		Rise, Set, nz;

    SinH0 = sin( -50.0/60.0 * RadPerDeg );


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


UTTohhmm(double UT, int *h, int *m){


    if (UT < 0.0) {
	*h = -1.0;
	*m = -1.0;
    } else {
        *h = (int)UT;
        *m = (int)((UT-(double)(*h))*60.0+0.5);
    }

}






Interp(double ym, double y0, double yp, double *xe, double *ye, double *z1, double *z2, int *nz){

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

    return(0);


}




double SinH(int year, int month, int day, double UT){

    double	TU0, TU, TU2, TU3, LambdaMoon, BetaMoon, R, AGE, frac(), jd();
    double	RA_Sun, DEC_Sun, T0, gmst, lmst, Tau, epsilon;
    double	M, DL, L, SL, X, Y, Z, RHO;
    

    TU0 = (jd(year, month, day, 0.0) - 2451545.0)/36525.0;

    TU = (jd(year, month, day, UT+62.0/3600.0) - 2451545.0)/36525.0;
    TU2 = TU*TU;
    TU3 = TU2*TU;

    M = P2*frac(0.993133 + 99.997361*TU);
    DL = 6893.0*sin(M) + 72.0*sin(2.0*M);
    L = P2*frac(0.7859453 + M/P2 + (6191.2*TU+DL)/1296e3);
    SL = sin(L);
    X = cos(L); Y = cosEPS*SL; Z = sinEPS*SL; RHO = sqrt(1.0-Z*Z);
    DEC_Sun = atan2(Z, RHO);
    RA_Sun = (48.0/P2)*atan(Y/(X+RHO));
    if (RA_Sun < 0) RA_Sun += 24.0;

    RA_Sun = RA_Sun*15.0*RadPerDeg;

    /*
     *  Compute Greenwich Mean Sidereal Time (gmst)
     */
    UT = 24.0*frac( UT/24.0 );
/*
    gmst = 6.697374558 + 1.0027379093*UT + (8640184.812866*TU0 +(0.093104-6.2e-6*TU)*TU2)/3600.0;
*/
    gmst = 6.697374558 + 1.0*UT + (8640184.812866+(0.093104-6.2e-6*TU)*TU)*TU/3600.0;
    lmst = 24.0*frac( (gmst-Glon/15.0) / 24.0 );

    Tau = 15.0*lmst*RadPerDeg - RA_Sun;
    return( SinGlat*sin(DEC_Sun) + CosGlat*cos(DEC_Sun)*cos(Tau) );


}


/*
 *  Compute the Julian Day number for the given date.
 *  Julian Date is the number of days since noon of Jan 1 4713 B.C.
 */
double jd(ny, nm, nd, UT)
int ny, nm, nd;
double UT;
{
        double A, B, C, D, JD, MJD, day;

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

double frac(double x){

    x -= (int)x;
    return( (x<0) ? x+1.0 : x );

}

