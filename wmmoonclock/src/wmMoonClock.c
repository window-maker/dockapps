/*
 *
 *  	wmMoonClock-1.26 (C) 1998, 1999 Mike Henderson (mghenderson@lanl.gov)
 * 
 *  		- Shows Moon Phase....
 *          
 * 
 * 
 * 
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation; either version 2, or (at your option)
 * 	any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program (see the file COPYING); if not, write to the
 * 	Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 *      Boston, MA  02111-1307, USA
 *
 *      Things TODO:
 *                  - clean up code! 
 *                  - more detailed documentation.
 *                  - reduce size of pixmap! Dont need it in one pixmap.
 *		      Aslo, does the hi-color pixmap really need all those colors?
 *                  - add rotation of moon so user sees it as they would in reality?
 *                  - eclipses. The calcs are quite acurate so this should be easily doable.
 *		      (Note the Sun position calcs in CalcEphem are low precision -- high is not
 * 		       as costly as the Moon calcs.) Sun posiiton is calculated but not used yet...
 *		    - Next new moons, next full moons, next quarters, etc...
 *		    - Moon names. I.e. Harvest, Blue, etc...
 *                  
 *                 
 *
 *      Changes:
 *      Version 1.27 - 	released June 7, 1999.
 *			fixed a minor bug in computation of azimuth (A in Horizon Coords). Thanks to
 *			Dan Moraru for spotting this one. (There were two SinGlat factors instead of one).
 *
 *      Version 1.26 - 	released April 22, 1999 (?).
 *      Version 1.25 - 	released March 22, 1999.
 *                     	Now auto-detects 8-bit display and forces the LowColor pixmap to
 *			be used. The -low option still works if you need to conserve colors
 *			even on high-color displays.
 *			
 *			Added 3 command line options + code to change colors of the data
 *			entries:
 *                  
 *				-bc <Color> to change background color.                  
 *				-lc <Color> to change color of labels and headers.                  
 *				-dc <Color> to change color of data values.
 *                  
 *      Version 1.24 - 	released February 9, 1999.
 *                     	Added low color support via the -low command line option.
 *                  
 *      Version 1.23 - 	released February 4, 1999.
 *                     	cosmetic for AfterStep users. removed spurious black line at RHS edge an mask.
 *
 *	Version 1.22 - 	released January 8, 1999. 
 *
 *		       	+ Changed PI2 to TwoPi in Moon.c -- Linux Pyth. had probs because
 *                        PI2 was defined in <math.h>.
 *
 *	Version 1.21 -  released January 7, 1999. 
 *    
 *                     + minor bug fixes in Makefile and manpage.
 *
 *	Version 1.2 - released January 3, 1999. 
 *		      Added:
 *
 *			+ Local Time/ Universal Time display.
 *			+ Visible: Yes/No to indicate if Moon is up or not.
 *			+ Frac (percent through orbit -- this is NOT a simple
 *			  conversion of AGE....).
 *			+ Horizon Coords. Altitude is measured up from horizon to 
 *			  Moon in degrees. Azimuth is in degrees from due south.
 *			
 *       	      Also shuffled things around a bit...
 *			
 *
 *
 *	Version 1.1 - released December 24, 1998. 
 *                    Fixed bug in AGE calculation. It now should be highly accurate.
 *                    Note that AGE is not the same as Phase*29.530589 ...
 *                    I have checked with the Astronomical Almanac and it agrees very
 *		      well....
 *
 *
 *	Version 1.0 - released December 16, 1998.
 *
 *
 *
 */





/*  
 *   Includes  
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "CalcEphem.h"
#include "xutils.h"
#include "wmMoonClock_master.xpm"
#include "wmMoonClock_masterLow.xpm"
#include "wmMoonClock_mask.xbm"



/* 
 *  Delay between refreshes (in microseconds) 
 */
#define DELAY 10000L
#define WMMOONCLOCK_VERSION "1.27"





void ParseCMDLine(int argc, char *argv[]);
void pressEvent(XButtonEvent *xev);
void print_usage();


int	ToggleWindow = 0;
int	nMAX = 1;
int	Flag = 1; 
double 	Glat, Glon, SinGlat, CosGlat, TimeZone;
int	UseLowColorPixmap = 0;
char    LabelColor[30]    = "#a171ff";
char    DataColor[30]     = "#3dafff";
char    BackColor[30]     = "#010101";






/*  
 *   main  
 */
int main(int argc, char *argv[]) {





    struct tm		*GMTTime, *LocalTime;
    XEvent		event;
    int			i, n, k, j, ImageNumber;
    int 		Year, Month, DayOfWeek, DayOfMonth;
    int			Hours, Mins, Secs, OldSecs, digit, xoff, xsize;
    long		CurrentLocalTime, CurrentGMTTime, date;
    double		UT, val, RA, DEC, UTRise, UTSet, LocalHour, hour24();
    int			D, H, M, S, sgn, A, B, q;
    char		str[10];
    CTrans           	c;





  
    /*
     *  Parse any command line arguments.
     */
    Glat = Glon = 0.0;
    ParseCMDLine(argc, argv);
    c.Glat = Glat, c.Glon = Glon;
    Glat *= RadPerDeg; SinGlat = sin( Glat ); CosGlat = cos( Glat );
   

   
    initXwindow(argc, argv);
    if ((DisplayDepth <= 8)||UseLowColorPixmap)
        openXwindow(argc, argv, wmMoonClock_masterLow, wmMoonClock_mask_bits, wmMoonClock_mask_width, wmMoonClock_mask_height, BackColor, LabelColor, DataColor);
    else
        openXwindow(argc, argv, wmMoonClock_master, wmMoonClock_mask_bits, wmMoonClock_mask_width, wmMoonClock_mask_height, BackColor, LabelColor, DataColor);




   
    /*
     *  Loop until we die
     */
    n = 32000;
    while(1) {


	if (Flag) {
	    n = 32000;
	    Flag = 0;
	}




	/*
	 *  The Moon Ephemeris calculations are somewhat costly (the Moon is one of the most
	 *  difficult objects to compute position for). So only process every nMAXth cycle of this
	 *  loop. We run outer loop it faster to catch expose events, button presses, etc...
	 *
	 */
	if (n>nMAX){

	    n = 0;

            CurrentGMTTime = time(CurrentTime); GMTTime = gmtime(&CurrentGMTTime); 
	    UT = GMTTime->tm_hour + GMTTime->tm_min/60.0 + GMTTime->tm_sec/3600.0;
	    Year = GMTTime->tm_year+1900;
	    Month = GMTTime->tm_mon+1;
	    DayOfMonth = GMTTime->tm_mday;
	    date = Year*10000 + Month*100 + DayOfMonth;
	    CurrentLocalTime = CurrentGMTTime; LocalTime = localtime(&CurrentLocalTime);
	    LocalHour = LocalTime->tm_hour + LocalTime->tm_min/60.0 + LocalTime->tm_sec/3600.0;
	    TimeZone = UT - LocalHour;


            CalcEphem(date, UT, &c);





	    if (ToggleWindow == 0){

	        /*
	         *  Update Moon Image
	         */

		nMAX = 1000;
	        ImageNumber = (int)(c.MoonPhase * 60.0 + 0.5);
	        if (ImageNumber > 59) ImageNumber = 0;
	        j = ImageNumber/10;
	        i = ImageNumber%10;
	        copyXPMArea(67+58*i, 2+58*j, 54, 54, 5, 5);

	    } else if (ToggleWindow == 1){

	        /*
	         *  Update Numerical Display
	         */


		nMAX = 500;


		/*
		 * Clear plotting area
		 */
	        copyXPMArea(4, 69, 56, 56, 4, 4);



		/*
		 *  Paste up LT and UT. 
		 */
		val = LocalHour;
		H = (int)val; val = (val - H)*60.0;
		M = (int)val; val = (val - M)*60.0;
		S = (int)val;
                digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 25, 6);
                digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 25+5, 6);
                copyXPMArea(117, 353, 1, 6, 25+10, 6);
                digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 25+12, 6);
                digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 25+17, 6);

		val = UT;
		H = (int)val; val = (val - H)*60.0;
		M = (int)val; val = (val - M)*60.0;
		S = (int)val;
                digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 25, 15);
                digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 25+5, 15);
                copyXPMArea(117, 353, 1, 6, 25+10, 15);
                digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 25+12, 15);
                digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 25+17, 15);





		/*
		 *  Paste up AGE. 
		 */
		val = c.MoonAge;
		q = (val < 10.0) ? 5 : 0;
		A = (int)val;
		val = (val - A)*100.0;
		B = (int)val;
		digit = A/10; if (digit != 0) copyXPMArea(67+digit*5, 353, 5, 7, 26-q, 24);
		digit = A%10; copyXPMArea(67+digit*5, 353, 5, 7, 26+5-q, 24);
	        copyXPMArea(62, 357, 3, 3, 26+11-q, 28);
		digit = B/10; copyXPMArea(67+digit*5, 353, 5, 7, 26+15-q, 24);
		digit = B%10; copyXPMArea(67+digit*5, 353, 5, 7, 26+20-q, 24);
	        copyXPMArea(143, 354, 3, 3, 26+25-q, 23);




		/*
		 *  Paste up Phase (Percent Illuminated). 
		 */
		val = 0.5*( 1.0 - cos(c.MoonPhase*6.2831853) );
		val *= 100.0;
		A = (int)(val+0.5);
		if (A == 100){
		    copyXPMArea(72, 353, 5, 7, 32+5, 42);
		    copyXPMArea(67, 353, 5, 7, 32+10, 42);
		    copyXPMArea(67, 353, 5, 7, 32+15, 42);
	            copyXPMArea(147, 353, 5, 7, 32+20, 42);
	 	} else if (A >= 10){
		    digit = A/10; copyXPMArea(67+digit*5, 353, 5, 7, 32+5, 42);
		    digit = A%10; copyXPMArea(67+digit*5, 353, 5, 7, 32+10, 42);
	            copyXPMArea(147, 353, 5, 7, 32+15, 42);
		} else {
		    digit = A; copyXPMArea(67+digit*5, 353, 5, 7, 32+5, 42);
	            copyXPMArea(147, 353, 5, 7, 32+10, 42);
		}




		/*
		 *  Paste up Frac (Percent of way through current lunar cycle). 
		 */
		val = c.MoonPhase*100.0;
		A = (int)(val+0.5);
		if (A == 100){
		    copyXPMArea(72, 353, 5, 7, 27+5, 33);
		    copyXPMArea(67, 353, 5, 7, 27+10, 33);
		    copyXPMArea(67, 353, 5, 7, 27+15, 33);
	            copyXPMArea(147, 353, 5, 7, 27+20, 33);
	 	} else if (A >= 10){
		    digit = A/10; copyXPMArea(67+digit*5, 353, 5, 7, 27+5, 33);
		    digit = A%10; copyXPMArea(67+digit*5, 353, 5, 7, 27+10, 33);
	            copyXPMArea(147, 353, 5, 7, 27+15, 33);
		} else {
		    digit = A; copyXPMArea(67+digit*5, 353, 5, 7, 27+5, 33);
	            copyXPMArea(147, 353, 5, 7, 27+10, 33);
		}


		/*
		 *  Paste up Visible Status. 
		 */
		if (c.Visible)
	            copyXPMArea(6,  327, 13, 6, 46, 51);
		else
	            copyXPMArea(26, 327,  9, 6, 46, 51);




	    } else if (ToggleWindow == 2){

		/*
		 *  Plot up Moon Rise/Set Times
		 */


		nMAX = 500;

		/*
		 * Clear plotting area
		 */
	        copyXPMArea(4, 134, 56, 56, 4, 4);


		/*
		 *   Do Yesterday's first
		 */
	        MoonRise(Year, Month, DayOfMonth-1, LocalHour, &UTRise, &UTSet); 
		UTTohhmm(UTRise, &H, &M); 
		if (H >= 0){
		    digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 7, 19);
		    digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 7+5, 19);
		    copyXPMArea(117, 354, 1, 4, 7+10, 20);
		    digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 7+12, 19);
		    digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 7+17, 19);
		} else {
		    copyXPMArea(57, 355, 5, 1, 7, 22); copyXPMArea(57, 355, 5, 1, 7+5, 22);
		    copyXPMArea(117, 354, 1, 4, 7+10, 20);
		    copyXPMArea(57, 355, 5, 1, 7+12, 22); copyXPMArea(57, 355, 5, 1, 7+17, 22);
		}
		UTTohhmm(UTSet, &H, &M); 
		if (H >= 0){
		    digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 35, 19);
		    digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 35+5, 19);
		    copyXPMArea(117, 354, 1, 4, 35+10, 20);
		    digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 35+12, 19);
		    digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 35+17, 19);
		} else {
		    copyXPMArea(57, 355, 5, 1, 35, 22); copyXPMArea(57, 355, 5, 1, 35+5, 22);
		    copyXPMArea(117, 354, 1, 4, 35+10, 20);
		    copyXPMArea(57, 355, 5, 1, 35+12, 22); copyXPMArea(57, 355, 5, 1, 35+17, 22);
		}


		/*
		 *  Plot up todays Rise/Set times.
		 */
	        MoonRise(Year, Month, DayOfMonth,   LocalHour, &UTRise, &UTSet);
		UTTohhmm(UTRise, &H, &M); 
		if (H >= 0){
		    digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 7, 29);
		    digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 7+5, 29);
		    copyXPMArea(117, 354, 1, 4, 7+10, 30);
		    digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 7+12, 29);
		    digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 7+17, 29);
		} else {
		    copyXPMArea(57, 355, 5, 1, 7, 32); copyXPMArea(57, 355, 5, 1, 7+5, 32);
		    copyXPMArea(117, 354, 1, 4, 7+10, 30);
		    copyXPMArea(57, 355, 5, 1, 7+12, 32); copyXPMArea(57, 355, 5, 1, 7+17, 32);
		}
		UTTohhmm(UTSet, &H, &M); 
		if (H >= 0){
		    digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 35, 29);
		    digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 35+5, 29);
		    copyXPMArea(117, 354, 1, 4, 35+10, 30);
		    digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 35+12, 29);
		    digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 35+17, 29);
		} else {
		    copyXPMArea(57, 355, 5, 1, 35, 32); copyXPMArea(57, 355, 5, 1, 35+5, 32);
		    copyXPMArea(117, 354, 1, 4, 35+10, 30);
		    copyXPMArea(57, 355, 5, 1, 35+12, 32); copyXPMArea(57, 355, 5, 1, 35+17, 32);
		}



		/*
		 *  Plot up tomorrow's Rise/Set times.
		 */
	        MoonRise(Year, Month, DayOfMonth+1, LocalHour, &UTRise, &UTSet);
		UTTohhmm(UTRise, &H, &M); 
		if (H >= 0){
		    digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 7, 39);
		    digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 7+5, 39);
		    copyXPMArea(117, 354, 1, 4, 7+10, 40);
		    digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 7+12, 39);
		    digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 7+17, 39);
		} else {
		    copyXPMArea(57, 355, 5, 1, 7, 42); copyXPMArea(57, 355, 5, 1, 7+5, 42);
		    copyXPMArea(117, 354, 1, 4, 7+10, 40);
		    copyXPMArea(57, 355, 5, 1, 7+12, 42); copyXPMArea(57, 355, 5, 1, 7+17, 42);
		}
		UTTohhmm(UTSet, &H, &M); 
		if (H >= 0){
		    digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 35, 39);
		    digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 35+5, 39);
		    copyXPMArea(117, 354, 1, 4, 35+10, 40);
		    digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 35+12, 39);
		    digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 35+17, 39);
		} else {
		    copyXPMArea(57, 355, 5, 1, 35, 42); copyXPMArea(57, 355, 5, 1, 35+5, 42);
		    copyXPMArea(117, 354, 1, 4, 35+10, 40);
		    copyXPMArea(57, 355, 5, 1, 35+12, 42); copyXPMArea(57, 355, 5, 1, 35+17, 42);
		}


	    } else if (ToggleWindow == 3){

		/*
		 *  Plot up Horizon Coords
		 */



		nMAX = 500;

		/*
		 * Clear plotting area
		 */
	        copyXPMArea(4, 199, 56, 56, 4, 4);



		/*
		 *  Paste up Azimuth, A
		 */
		val = c.A_moon;
		sgn = (val < 0.0) ? -1 : 0;
		val = fabs(val);
		D = (int)val;
		val = (val-(double)D)*100.0;
		M = (int)val;
		
		if (sgn < 0) copyXPMArea(120, 357, 2, 1, 19, 27);

		/* degrees 100's */
		digit = D/100; copyXPMArea(67+digit*5, 353, 5, 7, 22, 24);
		D -= digit*100;

		/* degrees 10's */
		digit = D/10; copyXPMArea(67+digit*5, 353, 5, 7, 22+5, 24);

		/* degrees 1's */
		digit = D%10; copyXPMArea(67+digit*5, 353, 5, 7, 22+10, 24);

		/* Decimal */
	        copyXPMArea(62, 357, 3, 3, 22+15, 28);

		/*  Decimal Part */
		digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 22+19, 24);

		/*  mins 1's */
		digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 22+24, 24);

		copyXPMArea(120, 353, 3, 3, 22+29, 23);





		/*
		 *  Paste up Altitude, h
		 */
		val = c.h_moon;
		sgn = (val < 0.0) ? -1 : 0;
		val = fabs(val);
		D = (int)val;
		val = (val-(double)D)*100.0;
		M = (int)val;
		
		if (sgn < 0) copyXPMArea(120, 357, 2, 1, 19, 39);

		/* degrees 10's */
		digit = D/10; copyXPMArea(67+digit*5, 353, 5, 7, 22, 36);

		/* degrees 1's */
		digit = D%10; copyXPMArea(67+digit*5, 353, 5, 7, 22+5, 36);

		/* Decimal */
	        copyXPMArea(62, 357, 3, 3, 22+10, 40);

		/*  Decimal Part */
		digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 22+14, 36);

		/*  mins 1's */
		digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 22+19, 36);

		copyXPMArea(120, 353, 3, 3, 22+24, 35);





		/*
		 *  Paste up Earth-Moon Distance (in units of Earth radii). 
		 */
		val = c.EarthMoonDistance;
		A = (int)val;
		val = (val - A)*100.0;
		B = (int)val;
		digit = A/10; if (digit != 0) copyXPMArea(67+digit*5, 353, 5, 7, 30, 47);
		digit = A%10; copyXPMArea(67+digit*5, 353, 5, 7, 30+5, 47);
	        copyXPMArea(62, 357, 3, 3, 30+11, 51);
		digit = B/10; copyXPMArea(67+digit*5, 353, 5, 7, 30+15, 47);
		digit = B%10; copyXPMArea(67+digit*5, 353, 5, 7, 30+20, 47);



	    } else if (ToggleWindow == 4){

		/*
		 *  Plot up RA/DEC Coords
		 */



		nMAX = 500;

		/*
		 * Clear plotting area
		 */
	        copyXPMArea(4, 264, 56, 56, 4, 4);



		/*
		 *  Paste up Right Ascention
		 */
		RA = c.RA_moon/15.0;
		H = (int)RA;
		RA = (RA-(double)H)*60.0;
		M = (int)RA; RA = (RA-(double)M)*60.0;
		S = (int)(RA+0.5);
		
		/* hours 10's */
		digit = H/10; copyXPMArea(67+digit*5, 353, 5, 7, 17, 25);

		/* hours 1's */
		digit = H%10; copyXPMArea(67+digit*5, 353, 5, 7, 17+5, 25);

		/* hour symbol */
		copyXPMArea(138, 354, 3, 3, 17+10, 24);

		/*  mins 10's */
		digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 17+14, 25);

		/*  mins 1's */
		digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 17+19, 25);

		/* min symbol */
		copyXPMArea(124, 353, 3, 3, 17+23, 24);

		/*  secs 10's */
		digit = S/10; copyXPMArea(67+digit*5, 353, 5, 7, 17+27, 25);

		/*  secs 1's */
		digit = S%10; copyXPMArea(67+digit*5, 353, 5, 7, 17+32, 25);

		/* sec symbol */
		copyXPMArea(128, 353, 5, 3, 17+36, 24);




		/*
		 *  Paste up Declination
		 */
		DEC = c.DEC_moon;
		sgn = (DEC < 0.0) ? -1 : 0;
		DEC = fabs(DEC);
		D = (int)DEC;
		DEC = (DEC-(double)D)*60.0;
		M = (int)DEC;
		DEC = (DEC-(double)M)*60.0;
		S = (int)(DEC+0.5);
		
		if (sgn < 0) copyXPMArea(120, 357, 2, 1, 14, 39);


		/* degrees 10's */
		digit = D/10; copyXPMArea(67+digit*5, 353, 5, 7, 17, 36);

		/* degrees 1's */
		digit = D%10; copyXPMArea(67+digit*5, 353, 5, 7, 17+5, 36);

		/* degree symbol */
		copyXPMArea(120, 353, 3, 3, 17+10, 35);

		/*  mins 10's */
		digit = M/10; copyXPMArea(67+digit*5, 353, 5, 7, 17+14, 36);

		/*  mins 1's */
		digit = M%10; copyXPMArea(67+digit*5, 353, 5, 7, 17+19, 36);

		/* min symbol */
		copyXPMArea(124, 353, 3, 3, 17+23, 35);

		/*  secs 10's */
		digit = S/10; copyXPMArea(67+digit*5, 353, 5, 7, 17+27, 36);

		/*  secs 1's */
		digit = S%10; copyXPMArea(67+digit*5, 353, 5, 7, 17+32, 36);

		/* sec symbol */
		copyXPMArea(128, 353, 5, 3, 17+36, 35);




		/*
		 *  Paste up Earth-Moon Distance (in units of Earth radii). 
		 */
		val = c.EarthMoonDistance;
		A = (int)val;
		val = (val - A)*100.0;
		B = (int)val;
		digit = A/10; if (digit != 0) copyXPMArea(67+digit*5, 353, 5, 7, 30, 47);
		digit = A%10; copyXPMArea(67+digit*5, 353, 5, 7, 30+5, 47);
	        copyXPMArea(62, 357, 3, 3, 30+11, 51);
		digit = B/10; copyXPMArea(67+digit*5, 353, 5, 7, 30+15, 47);
		digit = B%10; copyXPMArea(67+digit*5, 353, 5, 7, 30+20, 47);



	    } 



	} else {

	    /*
	     *  Update the counter. 
	     */
	    ++n;

	}








	/* 
	 *   Process any pending X events.
	 */
        while(XPending(display)){
            XNextEvent(display, &event);
            switch(event.type){
                case Expose:
                        RedrawWindow();
                        break;
                case ButtonPress:
                        pressEvent(&event.xbutton);
                        break;
                case ButtonRelease:
                        break;
            }
        }





	
	/* 
	 *  Redraw and wait for next update 
	 */
	RedrawWindow();
	usleep(DELAY);


     }



}








/* 
 *   ParseCMDLine()  
 */
void ParseCMDLine(int argc, char *argv[]) {

    int  i;
    char *eptr;
  
    for (i = 1; i < argc; i++) {

        if (!strcmp(argv[i], "-display")){

  	    ++i;

        } else if (!strcmp(argv[i], "-bc")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmMoonClock: -bc needs a color argument.\n");
                print_usage();
                exit(-1);
            }
            strcpy(BackColor, argv[++i]);

        } else if (!strcmp(argv[i], "-lc")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmMoonClock: -lc needs a color argument.\n");
                print_usage();
                exit(-1);
            }
            strcpy(LabelColor, argv[++i]);

        } else if (!strcmp(argv[i], "-dc")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmMoonClock: -dc needs a color argument.\n");
                print_usage();
                exit(-1);
            }
            strcpy(DataColor, argv[++i]);

        } else if (!strcmp(argv[i], "-low")){

	    UseLowColorPixmap = 1;

        } else if (!strcmp(argv[i], "-lat")){

            if (i+1 >= argc){
		fprintf(stderr, "wmMoonClock: -lat needs a value in degrees.\n");
		print_usage();
		exit(-1);
	    } else {
		Glat = strtod(argv[i+1], &eptr);
		if ((Glat == 0.0)&&(eptr == argv[i+1])){
                    fprintf(stderr, "wmMoonClock: could not convert latitude %s.\n", argv[i+1]);
                    print_usage();
                    exit(-1);
		}
		++i;
            }

        } else if (!strcmp(argv[i], "-lon")){

            if (i+1 >= argc){
		fprintf(stderr, "wmMoonClock: -lat needs a value in degrees.\n");
		print_usage();
		exit(-1);
	    } else {
		Glon = strtod(argv[i+1], &eptr);
		if ((Glon == 0.0)&&(eptr == argv[i+1])){
                    fprintf(stderr, "wmMoonClock: could not convert longitude %s.\n", argv[i+1]);
                    print_usage();
                    exit(-1);
		}
		++i;
            }

	} else {
	    
	    print_usage();
	    exit(1);
	}
    }

}


void print_usage(){

    printf("\nwmMoonClock version: %s\n", WMMOONCLOCK_VERSION);
    printf("\nusage: wmMoonClock [-display <Display>] [-lat <Latitude>] [-lon <Longitude>] [-h]\n");
    printf("                   [-bc <Color>] [-lc <Color>] [-dc <Color>]\n\n");
    printf("\t-display <Display>\tUse alternate X display.\n");
    printf("\t-bc <Color>   \t\tBackground color. (e.g. #ffbb00 or orange)\n");
    printf("\t-lc <Color>   \t\tColor for lables.\n");
    printf("\t-dc <Color>   \t\tColor for data entries.\n");
    printf("\t-low           \t\tUse lower color pixmap (for 8-bit displays).\n");
    printf("\t-lat <Latitude>\t\tObservers Latitude. Positive in northern\n");
    printf("\t               \t\themisphere, negative in southern hemisphere.\n");
    printf("\t-lon <Longitude>\tObservers Longitude. Greenwich is 0.0, and longitude\n");
    printf("\t                \tincreases positively toward the west. (Alternatively,\n");
    printf("\t                \tnegative numbers can also be used to specify\n");
    printf("\t                \tlongitudes to the east of Greenwich).\n");
    printf("\t-h\t\t\tDisplay help screen.\n\n");

}


/*
 *  This routine handles button presses. Clicking in the window
 *  toggles the display.
 *
 */
void pressEvent(XButtonEvent *xev){

   ++ToggleWindow;
   if (ToggleWindow > 4) ToggleWindow = 0;
   Flag = 1;

   return;

}

