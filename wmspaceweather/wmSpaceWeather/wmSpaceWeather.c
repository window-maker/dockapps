/*
 *
 *  	wmSpaceWeather-1.04 (C) 1998 Mike Henderson (mghenderson@lanl.gov)
 * 
 *  		- Its a Space Weather Monitor
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
 *      ToDo:
 *
 *	- The whole NOAA space weather www site is pretty screwed up! I currently have
 *	  to grab data from 2 separate files to get all that I need. But it seems that
 *        sometimes one of the files shows less than it should. This seems to be related to
 *        the way they update the 2 separate files... I will have to find some way of
 *        making that more robust.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *      Changes:
 *
 *	Version 1.04 - released Feb 18, 1999
 *			Added double click capability. Double clicking on mouse button 1 sends 
 *			URL (defined via ne command line option -url) to netscape.
 *
 *	Version 1.03 - released Feb 11, 1999
 *		       Changed display a bit. When no data is
 *		       available, it now shows nothing (before it would
 *		       show false junk...). Modified Perl Script GetKp.
 *
 *
 *	Version 1.02 - released Feb 8, 1999
 *		       bug fixes...
 *
 *	Version 1.0b - released Dec 19, 1998
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
#include "../wmgeneral/wmgeneral.h"
#include "wmSpaceWeather_master.xpm"
#include "wmSpaceWeather_mask.xbm"



/* 
 *  Delay between refreshes (in microseconds) 
 */
#define DELAY 10000L
#define WMSPACEWEATHER_VERSION "1.04"

int             GotFirstClick1, GotDoubleClick1;
int             GotFirstClick2, GotDoubleClick2;
int             GotFirstClick3, GotDoubleClick3;
int             DblClkDelay;
char            URL[1024];
int		ForceUpdate2;


void ParseCMDLine(int argc, char *argv[]);
void pressEvent(XButtonEvent *xev);










/*  
 *   main  
 */
int main(int argc, char *argv[]) {


struct tm		*Time;
XEvent		event;
int			i, n, s, k, m, dt1, dt2;
int 		Year, Month, Day, DayOfMonth, OldDayOfMonth;
int			Hours, Mins, Secs, OldSecs, xoff, D[10], xsize;
long		CurrentLocalTime;
int			height, UpToDate, LEDOn;
double		UT, TU, TU2, TU3, T0, gmst, hour24();


double		jd(), CurrentJD, LatestAvailJD, tim, DeltaT;
long int		TimeTag[8];
int 		Kp[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
double		E1, E2, P1, P2, P3;
char		Xray[10], digit[2];
FILE		*fp;










	  
    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);
	   

	   
    openXwindow(argc, argv, wmSpaceWeather_master, wmSpaceWeather_mask_bits, wmSpaceWeather_mask_width, wmSpaceWeather_mask_height);



	   
    /*
     *  Loop until we die
     */
    n = 32000;
    s = 32000;
    m = 32000;
    dt1 = 32000;
    dt2 = 32000;
    LEDOn = 0;
    DblClkDelay = 32000;
    ForceUpdate2 = 1;
    while(1) {




	/*
	 *  Keep track of # of seconds
	 */
	if (m > 100){
	
	    m = 0;
	    ++dt1;
	    ++dt2;
	
	} else {
	
	    /*
	     *  Increment counter
	     */
	    ++m;
	
	}





        /*
         *  Double Click Delays
         *  Keep track of click events. If Delay too long, set GotFirstClick's to False.
         */
        if (DblClkDelay > 15) {

            DblClkDelay = 0;
            GotFirstClick1 = 0; GotDoubleClick1 = 0;
            GotFirstClick2 = 0; GotDoubleClick2 = 0;
            GotFirstClick3 = 0; GotDoubleClick3 = 0;

        } else {

            ++DblClkDelay;

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
	 *  Redraw 
	 */
	RedrawWindow();










	/*
	 *  Check the Kp file every (approx.) 2 seconds.
	 *  Can significantly reduce this frequency later. But its
	 *  easier to debug this way...
	 *  Do this before trying to download again! The file may be there and it
	 *  may be Up-To-Date!
	 */
	if (dt2 > 2){
	    
	    dt2 = 0;

	    /*
	     *  Compute Current Julian Date
	     */
	    CurrentLocalTime = time(CurrentTime);
	    Time = gmtime(&CurrentLocalTime);
	    Year  = Time->tm_year+1900;
	    Month = Time->tm_mon+1;
	    Day   = Time->tm_mday;
	    Hours = Time->tm_hour;
	    Mins  = Time->tm_min;
	    Secs  = Time->tm_sec;
	    UT = (double)Hours + (double)Mins/60.0 + (double)Secs/3600.0;
	    CurrentJD = jd(Year, Month, Day, UT);
	    


    	    /*
    	     *  Read in Kp values
    	     */
    	    if ((fp = fopen("/tmp/LatestKp.txt", "r")) != NULL){

    	        for (i=0; i<8; ++i){
	    	    fscanf(fp, "%ld %d", &TimeTag[i], &Kp[i]);
		    if (Kp[i] < 0) TimeTag[i] = 190001011;
    	        }
	    	fscanf(fp, "%lf", &P1);
	    	fscanf(fp, "%lf", &P2);
	    	fscanf(fp, "%lf", &P3);
	    	fscanf(fp, "%lf", &E1);
	    	fscanf(fp, "%lf", &E2);
	    	fscanf(fp, "%10s",  Xray);
    	        fclose(fp);

	    } else {

    	        for (i=0; i<8; ++i) {
		    Kp[i] = -1;
		    TimeTag[i] = 190001011;
		}

	    }




	    /*
	     *  Compute Julian Date for latest available Kp
	     */
	    tim = TimeTag[7];
	    Year = tim/100000;
	    tim -= Year*100000;
	    Month = tim/1000;
	    tim -= Month*1000;
	    Day = tim/10;
	    tim -= Day*10;
	    UT = tim*3.0;
	    LatestAvailJD = jd(Year, Month, Day, UT);

	    DeltaT = (CurrentJD - LatestAvailJD)*24.0;
	    UpToDate = (DeltaT <= 3.0) ? 1 : 0;

	    if (!UpToDate){

		/*
	     	 * shift data back
		 */
		k = (int)(DeltaT/3.0);

		if ((k>=0)&&(k<=7)){
		    for (i=0; i<8-k; ++i) Kp[i] = Kp[i+k];
		    for (i=8-k; i<8; ++i) Kp[i] = -1;
		}


	    } 

	} 






	/*
	 *  Update Kp Bars etc...
	 */
	if (n > 200){

	    n = 0;

	    copyXPMArea(5, 67, 47, 20, 5, 39);
	
	    for (i=0; i<8; ++i){
	        if ((Kp[i] >= 0)&&(Kp[i] <= 9)){
	            height = 2*Kp[i] + 1;
	            copyXPMArea(53, 86-height+1, 5, height, 5+5*i+i, 58-height+1);
	        }
	    }

	    /*
	     *  Update Xray display...
	     */
	    if (Xray[0] != 'Z'){
	        switch(Xray[0]){
		    case 'B':
			    copyXPMArea(66, 17, 5, 7, 37, 25);
			    break;
		    case 'C':
			    copyXPMArea(72, 17, 5, 7, 37, 25);
			    break;
		    case 'M':
			    copyXPMArea(78, 17, 5, 7, 37, 25);
			    break;
		    case 'X':
			    copyXPMArea(84, 17, 5, 7, 37, 25);
			    break;
	        }
	        digit[0] = Xray[1]; digit[1] = '\0';
	        copyXPMArea(atoi(digit)*6+66, 25, 5, 7, 43, 25);
	        copyXPMArea(127, 30, 3, 3, 49, 30);
	        digit[0] = Xray[3]; digit[1] = '\0';
	        copyXPMArea(atoi(digit)*6+66, 25, 5, 7, 53, 25);
	    }




	    /*
	     *  Update E1 LED...
	     */
	    if ((E1 > 0)&&(E1 < 1e6))
	        copyXPMArea(66, 12, 4, 4, 25, 7);
	    else if ((E1 >= 1e6)&&(E1 < 1e7))
	        copyXPMArea(66,  7, 4, 4, 25, 7);
	    else if (E1 > 1e7)
	        copyXPMArea(66,  2, 4, 4, 25, 7);


	    /*
	     *  Update E2 LED...
	     */
	    if ((E2 > 0)&&(E2 < 1e3))
	        copyXPMArea(66, 12, 4, 4, 31, 7);
	    else if ((E2 >= 1e3)&&(E2 < 1e4))
	        copyXPMArea(66,  7, 4, 4, 31, 7);
	    else if (E2 > 1e4)
	        copyXPMArea(66,  2, 4, 4, 31, 7);



	    /*
	     *  Update P1 LED...
	     */
	    if ((P1 > 0)&&(P1 < 1e2))
	        copyXPMArea(66, 12, 4, 4, 22, 16);
	    else if ((P1 >= 1e2)&&(P1 < 1e3))
	        copyXPMArea(66,  7, 4, 4, 22, 16);
	    else if (P1 > 1e3)
	        copyXPMArea(66,  2, 4, 4, 22, 16);


	    /*
	     *  Update P2 LED...
	     */
	    if ((P2 > 0)&&(P2 < 0.5e0))
	        copyXPMArea(66, 12, 4, 4, 28, 16);
	    else if ((P2 >= 0.5e0)&&(P2 < 0.5e1))
	        copyXPMArea(66,  7, 4, 4, 28, 16);
	    else if (P2 > 0.5e1)
	        copyXPMArea(66,  2, 4, 4, 28, 16);


	    /*
	     *  Update P3 LED...
	     */
	    if ((P3 > 0)&&(P3 < 0.3e0))
	        copyXPMArea(66, 12, 4, 4, 34, 16);
	    else if ((P3 >= 0.3e0)&&(P3 < 0.3e1))
	        copyXPMArea(66,  7, 4, 4, 34, 16);
	    else if (P3 > 0.3e1)
	        copyXPMArea(66,  2, 4, 4, 34, 16);









	} else {

	    /*
	     *  Increment counter
	     */
	    ++n;

	}





	/*
	 *  Update the blinking LED which indicates whether or not the
	 *  display is up-to-date
	 */
	if (s > 20){

	    s = 0;

	    if (LEDOn){
	
	        if (UpToDate)
	            copyXPMArea(65, 82, 4, 4, 54, 53);
	        else
	            copyXPMArea(65, 76, 4, 4, 54, 53);

	        LEDOn = 0;

	    } else { 

	        if (UpToDate)
	            copyXPMArea(60, 82, 4, 4, 54, 53);
	        else
	            copyXPMArea(60, 76, 4, 4, 54, 53);

	        LEDOn = 1;
	
	    }
	} else {

	    /*
	     *  Increment counter
	     */
	    ++s;

	}

	    








	/*
 	 *  Check every 5 min if the values are not up to date...
	 */
	if (((!UpToDate)&&(dt1 > 300))||ForceUpdate2){

	    dt1 = 0;

    	    /*
    	     *  Execute Perl script to grab the Latest Kp values
    	     */
    	    system("GetKp &");

	    ForceUpdate2 = 0;

	}




	/* 
	 *  Wait for next update 
	 */
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

        } else if ((!strcmp(argv[i], "-url"))||(!strcmp(argv[i], "-u"))){

            strcpy(URL, argv[++i]);

        } else {

	    printf("\nwmSpaceWeather version: %s\n", WMSPACEWEATHER_VERSION);
	    printf("\nusage: wmSpaceWeather [-h] [-url <www URL>]\n\n");
	    printf("\t-url <URL>\tURL to send to Netscape with Button1 double click.\n\n");
	    printf("\t-h\t\tDisplay help screen.\n\n");
            exit(1);

        }
    }

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




/*
 *  This routine handles button presses.
 */
void pressEvent(XButtonEvent *xev){

    char Command[512];


    DblClkDelay = 0;
    if ((xev->button == Button1) && (xev->type == ButtonPress)){
        if (GotFirstClick1) GotDoubleClick1 = 1;
        else GotFirstClick1 = 1;
    } else if ((xev->button == Button2) && (xev->type == ButtonPress)){
        if (GotFirstClick2) GotDoubleClick2 = 1;
        else GotFirstClick2 = 1;
    } else if ((xev->button == Button3) && (xev->type == ButtonPress)){
        if (GotFirstClick3) GotDoubleClick3 = 1;
        else GotFirstClick3 = 1;
    }

    /*
     *  We got a double click on Mouse Button1 (i.e. the left one)
     */
    if (GotDoubleClick1) {
        GotFirstClick1 = 0;
        GotDoubleClick1 = 0;
        sprintf(Command, "netscape -remote 'openURL(%s)' || netscape '%s' &", URL, URL);
        system(Command);
    }


    /*
     *  We got a double click on Mouse Button2 (i.e. the left one)
     */
    if (GotDoubleClick2) {
    }


    /*
     *  We got a double click on Mouse Button3 (i.e. the left one)
     */
    if (GotDoubleClick3) {
        GotFirstClick3 = 0;
        GotDoubleClick3 = 0;
        ForceUpdate2 = 1;
    }



   return;

}

