/*
 *  Calculates Moon's position according to Brown's Lunar Theory...
 */

#include <math.h>
#define DegPerRad       57.29577951308232087680
#define RadPerDeg        0.01745329251994329576

double angle360();

void addthe(double, double, double, double, double*, double*);
void addsol(double, double, double, double, int, int, int, int);
void addn(double, int, int, int, int);
void term(int, int, int, int, double*, double*);

double	TwoPi = 6.283185308;
double	ARC = 206264.81;
double	sine(), frac();
double 	DLAM, DLAMS;
double 	DS;
double 	GAM1C;
double 	SINPI;
double  N;
double  CO[14][5], SI[14][5];

double Moon(double T, double *LAMBDA, double *BETA, double *R, double *AGE){

double	T2;
double	S1, S2, S3, S4, S5, S6, S7;
double	DL0, DL, DD, DGAM, DLS, DF;
double	L0, L, LS, F, D;
double	ARG = 0.0, FAC = 0.0;
int	MAX = 0, i, j;
double	S; 


    T2 = T*T;
    DLAM = 0.0, DS = 0.0, GAM1C = 0.0; SINPI = 3422.7000;

    /*
     * Long Periodic variations
     */
    S1 = sine( 0.19833 + 0.05611*T );
    S2 = sine( 0.27869 + 0.04508*T );
    S3 = sine( 0.16827 - 0.36903*T );
    S4 = sine( 0.34734 - 5.37261*T );
    S5 = sine( 0.10498 - 5.37899*T );
    S6 = sine( 0.42681 - 0.41855*T );
    S7 = sine( 0.14943 - 5.37511*T );
    DL0 = 0.84*S1 + 0.31*S2 + 14.27*S3 + 7.26*S4 + 0.28*S5 + 0.24*S6;
    DL  = 2.94*S1 + 0.31*S2 + 14.27*S3 + 9.34*S4 + 1.12*S5 + 0.83*S6;
    DLS = -6.40*S1 - 1.89*S6;
    DF = 0.21*S1 + 0.31*S2 + 14.27*S3 - 88.70*S4 - 15.30*S5 + 0.24*S6 - 1.86*S7;
    DD = DL0 - DLS;
    DGAM = -3332e-9 * sine( 0.59734 - 5.37261*T)
	    -539e-9 * sine( 0.35498 - 5.37899*T)
	     -64e-9 * sine( 0.39943 - 5.37511*T);



    L0  = TwoPi*frac( 0.60643382 + 1336.85522467*T - 0.00000313*T2 ) + DL0/ARC;
    L   = TwoPi*frac( 0.37489701 + 1325.55240982*T + 0.00002565*T2 ) + DL/ARC;
    LS  = TwoPi*frac( 0.99312619 +   99.99735956*T - 0.00000044*T2 ) + DLS/ARC;
    F   = TwoPi*frac( 0.25909118 + 1342.22782980*T - 0.00000892*T2 ) + DF/ARC;
    D   = TwoPi*frac( 0.82736186 + 1236.85308708*T - 0.00000397*T2 ) + DD/ARC;

    for (i=1; i<=4; ++i){
        switch (i){
	    case 1: 
		ARG = L,  MAX = 4, FAC = 1.000002208;
		break;
	    case 2: 
		ARG = LS, MAX = 3, FAC = 0.997504612 - 0.002495388*T;
		break;
	    case 3: 
		ARG = F,  MAX = 4, FAC = 1.000002708 + 139.978*DGAM;
		break;
	    case 4: 
		ARG = D,  MAX = 6, FAC = 1.0;
		break;
        }
    
        CO[6+0][i] = 1.0, CO[6+1][i] = cos(ARG)*FAC;
        SI[6+0][i] = 0.0, SI[6+1][i] = sin(ARG)*FAC;
        for (j=2; j<=MAX; ++j) addthe(CO[6+j-1][i], SI[6+j-1][i], CO[6+1][i], SI[6+1][i], &CO[6+j][i], &SI[6+j][i]);
        for (j=1; j<=MAX; ++j) {
    	    CO[6-j][i] = CO[6+j][i];
	    SI[6-j][i] = -SI[6+j][i];
        }


    }



    /*
     *  Solar1
     */
    addsol(   13.902,   14.06,-0.001,   0.2607,0, 0, 0, 4);
    addsol(    0.403,   -4.01,+0.394,   0.0023,0, 0, 0, 3);
    addsol( 2369.912, 2373.36,+0.601,  28.2333,0, 0, 0, 2);
    addsol( -125.154, -112.79,-0.725,  -0.9781,0, 0, 0, 1);
    addsol(    1.979,    6.98,-0.445,   0.0433,1, 0, 0, 4);
    addsol(  191.953,  192.72,+0.029,   3.0861,1, 0, 0, 2);
    addsol(   -8.466,  -13.51,+0.455,  -0.1093,1, 0, 0, 1);
    addsol(22639.500,22609.07,+0.079, 186.5398,1, 0, 0, 0);
    addsol(   18.609,    3.59,-0.094,   0.0118,1, 0, 0,-1);
    addsol(-4586.465,-4578.13,-0.077,  34.3117,1, 0, 0,-2);
    addsol(   +3.215,    5.44,+0.192,  -0.0386,1, 0, 0,-3);
    addsol(  -38.428,  -38.64,+0.001,   0.6008,1, 0, 0,-4);
    addsol(   -0.393,   -1.43,-0.092,   0.0086,1, 0, 0,-6);
    addsol(   -0.289,   -1.59,+0.123,  -0.0053,0, 1, 0, 4);
    addsol(  -24.420,  -25.10,+0.040,  -0.3000,0, 1, 0, 2);
    addsol(   18.023,   17.93,+0.007,   0.1494,0, 1, 0, 1);
    addsol( -668.146, -126.98,-1.302,  -0.3997,0, 1, 0, 0);
    addsol(    0.560,    0.32,-0.001,  -0.0037,0, 1, 0,-1);
    addsol( -165.145, -165.06,+0.054,   1.9178,0, 1, 0,-2);
    addsol(   -1.877,   -6.46,-0.416,   0.0339,0, 1, 0,-4);
    addsol(    0.213,    1.02,-0.074,   0.0054,2, 0, 0, 4);
    addsol(   14.387,   14.78,-0.017,   0.2833,2, 0, 0, 2);
    addsol(   -0.586,   -1.20,+0.054,  -0.0100,2, 0, 0, 1);
    addsol(  769.016,  767.96,+0.107,  10.1657,2, 0, 0, 0);
    addsol(   +1.750,    2.01,-0.018,   0.0155,2, 0, 0,-1);
    addsol( -211.656, -152.53,+5.679,  -0.3039,2, 0, 0,-2);
    addsol(   +1.225,    0.91,-0.030,  -0.0088,2, 0, 0,-3);
    addsol(  -30.773,  -34.07,-0.308,   0.3722,2, 0, 0,-4);
    addsol(   -0.570,   -1.40,-0.074,   0.0109,2, 0, 0,-6);
    addsol(   -2.921,  -11.75,+0.787,  -0.0484,1, 1, 0, 2);
    addsol(   +1.267,    1.52,-0.022,   0.0164,1, 1, 0, 1);
    addsol( -109.673, -115.18,+0.461,  -0.9490,1, 1, 0, 0);
    addsol( -205.962, -182.36,+2.056,  +1.4437,1, 1, 0,-2);
    addsol(    0.233,    0.36, 0.012,  -0.0025,1, 1, 0,-3);
    addsol(   -4.391,   -9.66,-0.471,   0.0673,1, 1, 0,-4);


    /*
     *  Solar2
     */
    addsol(    0.283,    1.53,-0.111,  +0.0060,1,-1, 0,+4);
    addsol(   14.577,   31.70,-1.540,  +0.2302,1,-1, 0, 2);
    addsol(  147.687,  138.76,+0.679,  +1.1528,1,-1, 0, 0);
    addsol(   -1.089,    0.55,+0.021,   0.0   ,1,-1, 0,-1);
    addsol(   28.475,   23.59,-0.443,  -0.2257,1,-1, 0,-2);
    addsol(   -0.276,   -0.38,-0.006,  -0.0036,1,-1, 0,-3);
    addsol(    0.636,    2.27,+0.146,  -0.0102,1,-1, 0,-4);
    addsol(   -0.189,   -1.68,+0.131,  -0.0028,0, 2, 0, 2);
    addsol(   -7.486,   -0.66,-0.037,  -0.0086,0, 2, 0, 0);
    addsol(   -8.096,  -16.35,-0.740,   0.0918,0, 2, 0,-2);
    addsol(   -5.741,   -0.04, 0.0  ,  -0.0009,0, 0, 2, 2);
    addsol(    0.255,    0.0 , 0.0  ,   0.0   ,0, 0, 2, 1);
    addsol( -411.608,   -0.20, 0.0  ,  -0.0124,0, 0, 2, 0);
    addsol(    0.584,    0.84, 0.0  ,  +0.0071,0, 0, 2,-1);
    addsol(  -55.173,  -52.14, 0.0  ,  -0.1052,0, 0, 2,-2);
    addsol(    0.254,    0.25, 0.0  ,  -0.0017,0, 0, 2,-3);
    addsol(   +0.025,   -1.67, 0.0  ,  +0.0031,0, 0, 2,-4);
    addsol(    1.060,    2.96,-0.166,   0.0243,3, 0, 0,+2);
    addsol(   36.124,   50.64,-1.300,   0.6215,3, 0, 0, 0);
    addsol(  -13.193,  -16.40,+0.258,  -0.1187,3, 0, 0,-2);
    addsol(   -1.187,   -0.74,+0.042,   0.0074,3, 0, 0,-4);
    addsol(   -0.293,   -0.31,-0.002,   0.0046,3, 0, 0,-6);
    addsol(   -0.290,   -1.45,+0.116,  -0.0051,2, 1, 0, 2);
    addsol(   -7.649,  -10.56,+0.259,  -0.1038,2, 1, 0, 0);
    addsol(   -8.627,   -7.59,+0.078,  -0.0192,2, 1, 0,-2);
    addsol(   -2.740,   -2.54,+0.022,   0.0324,2, 1, 0,-4);
    addsol(    1.181,    3.32,-0.212,   0.0213,2,-1, 0,+2);
    addsol(    9.703,   11.67,-0.151,   0.1268,2,-1, 0, 0);
    addsol(   -0.352,   -0.37,+0.001,  -0.0028,2,-1, 0,-1);
    addsol(   -2.494,   -1.17,-0.003,  -0.0017,2,-1, 0,-2);
    addsol(    0.360,    0.20,-0.012,  -0.0043,2,-1, 0,-4);
    addsol(   -1.167,   -1.25,+0.008,  -0.0106,1, 2, 0, 0);
    addsol(   -7.412,   -6.12,+0.117,   0.0484,1, 2, 0,-2);
    addsol(   -0.311,   -0.65,-0.032,   0.0044,1, 2, 0,-4);
    addsol(   +0.757,    1.82,-0.105,   0.0112,1,-2, 0, 2);
    addsol(   +2.580,    2.32,+0.027,   0.0196,1,-2, 0, 0);
    addsol(   +2.533,    2.40,-0.014,  -0.0212,1,-2, 0,-2);
    addsol(   -0.344,   -0.57,-0.025,  +0.0036,0, 3, 0,-2);
    addsol(   -0.992,   -0.02, 0.0  ,   0.0   ,1, 0, 2, 2);
    addsol(  -45.099,   -0.02, 0.0  ,  -0.0010,1, 0, 2, 0);
    addsol(   -0.179,   -9.52, 0.0  ,  -0.0833,1, 0, 2,-2);
    addsol(   -0.301,   -0.33, 0.0  ,   0.0014,1, 0, 2,-4);
    addsol(   -6.382,   -3.37, 0.0  ,  -0.0481,1, 0,-2, 2);
    addsol(   39.528,   85.13, 0.0  ,  -0.7136,1, 0,-2, 0);
    addsol(    9.366,    0.71, 0.0  ,  -0.0112,1, 0,-2,-2);
    addsol(    0.202,    0.02, 0.0  ,   0.0   ,1, 0,-2,-4);

    /* 
     *  Solar3
     */
    addsol(    0.415,    0.10, 0.0  ,  0.0013,0, 1, 2, 0);
    addsol(   -2.152,   -2.26, 0.0  , -0.0066,0, 1, 2,-2);
    addsol(   -1.440,   -1.30, 0.0  , +0.0014,0, 1,-2, 2);
    addsol(    0.384,   -0.04, 0.0  ,  0.0   ,0, 1,-2,-2);
    addsol(   +1.938,   +3.60,-0.145, +0.0401,4, 0, 0, 0);
    addsol(   -0.952,   -1.58,+0.052, -0.0130,4, 0, 0,-2);
    addsol(   -0.551,   -0.94,+0.032, -0.0097,3, 1, 0, 0);
    addsol(   -0.482,   -0.57,+0.005, -0.0045,3, 1, 0,-2);
    addsol(    0.681,    0.96,-0.026,  0.0115,3,-1, 0, 0);
    addsol(   -0.297,   -0.27, 0.002, -0.0009,2, 2, 0,-2);
    addsol(    0.254,   +0.21,-0.003,  0.0   ,2,-2, 0,-2);
    addsol(   -0.250,   -0.22, 0.004,  0.0014,1, 3, 0,-2);
    addsol(   -3.996,    0.0 , 0.0  , +0.0004,2, 0, 2, 0);
    addsol(    0.557,   -0.75, 0.0  , -0.0090,2, 0, 2,-2);
    addsol(   -0.459,   -0.38, 0.0  , -0.0053,2, 0,-2, 2);
    addsol(   -1.298,    0.74, 0.0  , +0.0004,2, 0,-2, 0);
    addsol(    0.538,    1.14, 0.0  , -0.0141,2, 0,-2,-2);
    addsol(    0.263,    0.02, 0.0  ,  0.0   ,1, 1, 2, 0);
    addsol(    0.426,   +0.07, 0.0  , -0.0006,1, 1,-2,-2);
    addsol(   -0.304,   +0.03, 0.0  , +0.0003,1,-1, 2, 0);
    addsol(   -0.372,   -0.19, 0.0  , -0.0027,1,-1,-2, 2);
    addsol(   +0.418,    0.0 , 0.0  ,  0.0   ,0, 0, 4, 0);
    addsol(   -0.330,   -0.04, 0.0  ,  0.0   ,3, 0, 2, 0);

    N = 0.0;
    addn(-526.069, 0, 0,1,-2); addn(  -3.352, 0, 0,1,-4);
    addn( +44.297,+1, 0,1,-2); addn(  -6.000,+1, 0,1,-4);
    addn( +20.599,-1, 0,1, 0); addn( -30.598,-1, 0,1,-2);
    addn( -24.649,-2, 0,1, 0); addn(  -2.000,-2, 0,1,-2);
    addn( -22.571, 0,+1,1,-2); addn( +10.985, 0,-1,1,-2);

    DLAM += +0.82*sine( 0.7736   -62.5512*T ) + 0.31*sine( 0.0466  -125.1025*T )
            +0.35*sine( 0.5785   -25.1042*T ) + 0.66*sine( 0.4591 +1335.8075*T )
            +0.64*sine( 0.3130   -91.5680*T ) + 1.14*sine( 0.1480 +1331.2898*T )
            +0.21*sine( 0.5918 +1056.5859*T ) + 0.44*sine( 0.5784 +1322.8595*T )
            +0.24*sine( 0.2275    -5.7374*T ) + 0.28*sine( 0.2965    +2.6929*T )
            +0.33*sine( 0.3132    +6.3368*T );









    *LAMBDA = 360.0*frac( (L0+DLAM/ARC)/TwoPi );

    S = F + DS/ARC;
    FAC = 1.000002708 + 139.978*DGAM;
    *BETA = (FAC*(18518.511 + 1.189 + GAM1C)*sin(S) - 6.24*sin(3*S) + N)/3600.0;

    SINPI *= 0.999953253;
    *R = ARC/SINPI;


    DLAMS = 6893.0 * sin(LS) + 72.0 * sin(2.0*LS);

    *AGE = 29.530589*frac((D+(DLAM-DLAMS)/ARC)/TwoPi);
/*
printf("Diff = %f\n", 360.0*frac((D+(DLAM-DLAMS)/ARC)/TwoPi));
*/

    /*
     *  Return the phase.
     */
/*
    return( 0.5*(1.0 - cos(D+(DLAM-DLAMS)/ARC)) );
*/
    return( *AGE/29.530589 );



}


double sine(double phi){

    return( sin(TwoPi*frac(phi)) );

}


double frac(double x){

    x -= (int)x;
    return( (x<0) ? x+1.0 : x );

}



void addsol(double COEFFL, double COEFFS, double COEFFG, double COEFFP, int P, int Q, int R, int S){

    double	X, Y;

    term(P, Q, R, S, &X, &Y);
    DLAM += COEFFL*Y;
    DS += COEFFS*Y;
    GAM1C += COEFFG*X;
    SINPI += COEFFP*X;


}



void term(int P, int Q, int R, int S, double *X, double *Y){

    double	XX, YY;
    int		k, I[5];

    I[1] = P, I[2] = Q, I[3] = R, I[4] = S, XX = 1.0, YY = 0.0;
    for (k=1; k<=4; ++k){
	if (I[k] != 0.0){
	    addthe(XX, YY, CO[6+I[k]][k], SI[6+I[k]][k], &XX, &YY);
	}
    }
    *X = XX;
    *Y = YY;

}



void addthe(double C1, double S1, double C2, double S2, double *C, double *S){

    *C = C1*C2 - S1*S2;
    *S = S1*C2 + C1*S2;


}


void addn(double COEFFN, int P, int Q, int R, int S){

    double	X, Y;

    term(P, Q, R, S, &X, &Y);
    N += COEFFN*Y;


}



#define R 0.61803399
#define C 0.38196601

double NewMoon(double ax, double bx, double cx){

    double	f1, f2, x0, x1, x2, x3, Moon();
    double	L, B, Rad, AGE, tol=1e-7;

    x0 = ax;
    x3 = cx;
    if (fabs(cx-bx) > fabs(bx-ax)){
	x1 = bx;
	x2 = bx + C*(cx-bx);
    } else {
	x2 = bx;
	x1 = bx - C*(bx-ax);
    }
    f1 = Moon(x1, &L, &B, &Rad, &AGE);
    f2 = Moon(x2, &L, &B, &Rad, &AGE);
    while (fabs(x3-x0) > tol*(fabs(x1)+fabs(x2))){
	if (f2 < f1){
	    x0 = x1;
	    x1 = x2;
	    x2 = R*x1+C*x3;
	    f1 = f2;
	    f2 = Moon(x2, &L, &B, &Rad, &AGE);
	} else {
	    x3 = x2;
	    x2 = x1;
	    x1 = R*x2+C*x0;
	    f2 = f1;
	    f1 = Moon(x1, &L, &B, &Rad, &AGE);
	}
    }
    if (f1 < f2){
	return(x1);
    } else {
	return(x2);
    }
}





/*
 * MINI_MOON: low precision lunar coordinates (approx. 5'/1')   
 *            T  : time in Julian centuries since J2000        
 *                 ( T=(JD-2451545)/36525 )                   
 *            RA : right ascension (in h; equinox of date)   
 *            DEC: declination (in deg; equinox of date)    
 *
 */
void MiniMoon(double T, double *RA, double *DEC){

    double	L0,L,LS,F,D,H,S,N,DL,CB,L_MOON,B_MOON,V,W,X,Y,Z,RHO;
    double	frac(), cosEPS, sinEPS, P2, ARC;

 
    cosEPS = 0.91748;
    sinEPS = 0.39778;
    P2 	= 6.283185307;
    ARC = 206264.8062;


    /*
     * mean elements of lunar orbit
     */
    L0 = frac(0.606433+1336.855225*T); /* mean longitude Moon (in rev) */   
    L  = P2*frac(0.374897+1325.552410*T); /* mean anomaly of the Moon     */
    LS = P2*frac(0.993133+  99.997361*T); /* mean anomaly of the Sun      */
    D  = P2*frac(0.827361+1236.853086*T); /* diff. longitude Moon-Sun     */
    F  = P2*frac(0.259086+1342.227825*T); /* mean argument of latitude    */
    DL  =  +22640.0*sin(L) - 4586.0*sin(L-2.0*D) + 2370.0*sin(2.0*D) +  769.0*sin(2.0*L) 
            -668.0*sin(LS)- 412.0*sin(2.0*F) - 212.0*sin(2.0*L-2.0*D) - 206.0*sin(L+LS-2.0*D)
            +192.0*sin(L+2.0*D) - 165.0*sin(LS-2.0*D) - 125.0*sin(D) - 110.0*sin(L+LS)
            +148.0*sin(L-LS) - 55.0*sin(2.0*F-2.0*D);
    S  =  F + (DL+412.0*sin(2.0*F)+541.0*sin(LS)) / ARC; 
    H  =  F-2.0*D;
    N  =  -526.0*sin(H) + 44.0*sin(L+H) - 31.0*sin(-L+H) - 23.0*sin(LS+H) 
         + 11.0*sin(-LS+H) -25.0*sin(-2.0*L+F) + 21.0*sin(-L+F);
    L_MOON  =  P2 * frac ( L0 + DL/1296e3 ); /* in rad */
    B_MOON  =  ( 18520.0*sin(S) + N ) / ARC; /* in rad */

    /* equatorial coordinates */
    CB = cos(B_MOON);
    X = CB*cos(L_MOON); 
    V = CB*sin(L_MOON); 
    W = sin(B_MOON);
    Y = cosEPS*V-sinEPS*W; 
    Z = sinEPS*V+cosEPS*W; 
    RHO = sqrt(1.0-Z*Z);
    *DEC  =  (360.0/P2)*atan2(Z, RHO); 
    *RA   =  ( 48.0/P2)*atan2(Y, X+RHO); 
    if (*RA<0.0) *RA += 24.0;



}



