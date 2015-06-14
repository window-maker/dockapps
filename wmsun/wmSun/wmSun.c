/*
 *
 *  	wmSun-1.03 (C) 1999 Mike Henderson (mghenderson@lanl.gov)
 * 
 *  		- Shows Sun Rise/Set Times....
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
 *                  - support for 8-bit displays.
 *                  - more detailed documentation.
 *                  - eclipses?
 *                  - add buttons to play will date and lat lon...
 *                    Could be something like this;
 *                       First click brings up buttons to change date.
 *                       Second click brings up buttons to change lat/lon.
 *                       Third goes back to display 
 *                       Set time delay to go back to display if user doesnt do it...
 *
 *
 *                  
 *                 
 *
 *      Changes:
 *
 * 	Version 1.03 - released February 4, 1999.
 *                     cosmetic for AfterStep users. removed spurious black line at RHS edge an mask.
 *
 *
 *	Version 1.02 - released January 12, 1999.
 *		       Added support for User-specified date and Time difference so that
 *		       you can have the display be correct in local time even for remote
 *                     lat/lons. (I am in Hawaii right now. I dont want to reset the time on
 *                     my laptop, so with these new options I can still get the correct
 *                     local times of rise/set. Sunset is awesome here in Kona!!! And the calcs
 *                     seem to be quite good -- it's a good test here because the Sun sets over
 *                     the Pacific (no mountains are in the way)).
 *
 *	Version 1.01 - released January 6, 1999.
 *                     Fixed stupid bug in Date change montior.
 *
 *	Version 1.0 - released January 5, 1999.
 *
 *
 */





/*  
 *   Includes  
 */
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "../wmgeneral/wmgeneral.h"
#include "wmSun_master.xpm"
#include "wmSun_mask.xbm"



/* 
 *  Delay between refreshes (in microseconds) 
 */
#define DELAY 10000L
#define WMSUN_VERSION "1.03"

#define DegPerRad       57.29577951308232087680
#define RadPerDeg        0.01745329251994329576





void ParseCMDLine(int argc, char *argv[]);
void pressEvent(XButtonEvent *xev);


int	ToggleWindow = 0;
int	nMAX = 1;
int	Flag = 1; 
int	UseUserTimeDiff = 0;
int	UseUserDate = 0;
long	UserDate;
double 	Glat, Glon, SinGlat, CosGlat, TimeZone, UserTimeDiff;


int	xDigit[11] = {8, 18, 27, 37, 46, 55, 64, 74, 83, 92, 102};




/*  
 *   main  
 */
int main(int argc, char *argv[]) {





    struct tm		*GMTTime, *LocalTime;
    XEvent		event;
    int			i, n, k, j, ImageNumber;
    int 		Year, Month, DayOfWeek, OldLocalDayOfMonth;
    int			LocalDayOfMonth,	DayOfMonth;
    int			Hours, Mins, Secs, OldSecs, digit, xoff, xsize;
    long		CurrentLocalTime, CurrentGMTTime, date;
    double		UT, val, RA, DEC, LTRise, LTSet, LocalHour, hour24();
    int			D, H, M, S, sgn, A, B, q;
    char		str[10];





  
    /*
     *  Parse any command line arguments.
     */
    Glat = Glon = 0.0;
    ParseCMDLine(argc, argv);
    Glat *= RadPerDeg; SinGlat = sin( Glat ); CosGlat = cos( Glat );
   

   
    openXwindow(argc, argv, wmSun_master, wmSun_mask_bits, wmSun_mask_width, wmSun_mask_height);




   
    /*
     *  Loop until we die
     */
    n = 32000;
    OldLocalDayOfMonth = -999;
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
	    nMAX = 1000;


            CurrentGMTTime = time(CurrentTime); GMTTime = gmtime(&CurrentGMTTime); 
	    DayOfMonth = GMTTime->tm_mday;

	    UT = GMTTime->tm_hour + GMTTime->tm_min/60.0 + GMTTime->tm_sec/3600.0;
	    Year = GMTTime->tm_year+1900;
	    Month = GMTTime->tm_mon+1;


	    CurrentLocalTime = CurrentGMTTime; LocalTime = localtime(&CurrentLocalTime);
	    LocalDayOfMonth = LocalTime->tm_mday;

	    if ((OldLocalDayOfMonth != LocalDayOfMonth)||(Flag)){

		Flag = 0;

		if (UseUserDate){
	    	    date =  UserDate;
		    Year = date/10000;
		    date -= Year*10000;
		    Month = date/100;
		    date -= Month*100;
		    DayOfMonth = date;
		    date = UserDate;
		} else {
	    	    date = Year*10000 + Month*100 + DayOfMonth;
		}
	    	LocalHour = LocalTime->tm_hour + LocalTime->tm_min/60.0 + LocalTime->tm_sec/3600.0;
	    	TimeZone = (UseUserTimeDiff) ? UserTimeDiff : UT - LocalHour;

	    	/*
	    	 *  Clear Plotting area
	    	 */
	    	copyXPMArea(65, 5, 54, 54, 5, 5);



   	    	/*
	    	 *  Compute Sun Rise/Set Times in Local Time
	    	 */
	    	SunRise(Year, Month, DayOfMonth, LocalHour, &LTRise, &LTSet);

		if (LTRise > 0.0){
	    	    val = LTRise;
	    	    H = (int)val; val = (val-H)*60.0;
	    	    M = (int)val;
	    	    copyXPMArea(xDigit[H/10], 73, 7, 9, 17, 13);
	    	    copyXPMArea(xDigit[H%10], 73, 7, 9, 17+7, 13);
	    	    copyXPMArea(xDigit[10],   75, 3, 6, 17+15, 15);
	    	    copyXPMArea(xDigit[M/10], 73, 7, 9, 17+19, 13);
	    	    copyXPMArea(xDigit[M%10], 73, 7, 9, 17+26, 13);
		} else {
	    	    copyXPMArea(10, 84, 28, 7, 19, 15);
		}

	    
		if (LTSet > 0.0){
	    	    val = LTSet;
	    	    H = (int)val; val = (val-H)*60.0;
	    	    M = (int)val;
	    	    copyXPMArea(xDigit[H/10], 73, 7, 9, 17, 40);
	    	    copyXPMArea(xDigit[H%10], 73, 7, 9, 17+7, 40);
	    	    copyXPMArea(xDigit[10],   75, 3, 6, 17+15, 42);
	    	    copyXPMArea(xDigit[M/10], 73, 7, 9, 17+19, 40);
	    	    copyXPMArea(xDigit[M%10], 73, 7, 9, 17+26, 40);
		} else {
	    	    copyXPMArea(10, 84, 28, 7, 19, 40);
		}

	    }

	    OldLocalDayOfMonth = LocalDayOfMonth;


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
  
    for (i = 1; i < argc; i++) {

        if (!strcmp(argv[i], "-display")){

  	    ++i;

        } else if (!strcmp(argv[i], "-lat")){

	    Glat = atof(argv[++i]);

        } else if (!strcmp(argv[i], "-lon")){

	    Glon = atof(argv[++i]);

        } else if (!strcmp(argv[i], "-td")){

	    UseUserTimeDiff = 1;
	    UserTimeDiff = atof(argv[++i]);

        } else if (!strcmp(argv[i], "-date")){

	    UseUserDate = 1;
	    UserDate = atoi(argv[++i]);

	} else {
	    printf("\nwmSun version: %s\n", WMSUN_VERSION);
	    printf("\nusage: wmSun [-display <Display>] [-lat <Latitude>] [-lon <Longitude>] [-h]\n\n");
	    printf("\t-display <Display>\tUse alternate X display.\n");
	    printf("\t-lat <Latitude>\t\tObservers Latitude. Positive to the west.\n");
	    printf("\t-lon <Longitude>\tObservers Longitude.\n");
	    printf("\t-td <Delta Time>\tUser defined difference between UT an LT (hours).\n");
	    printf("\t-h\t\t\tDisplay help screen.\n\n");
	    exit(1);
	}
    }

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

