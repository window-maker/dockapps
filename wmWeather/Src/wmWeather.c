/*
 *
 *  	wmWeather-1.31 (C) 1999 Mike Henderson (mghenderson@lanl.gov)
 * 
 *  		- Shows Local Weather conditions
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
 * 	Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *      Boston, MA 02110-1301 USA
 *
 *
 *
 *
 *      ToDo:
 *
 *	      -	Add a GTK popup window to display data in a nicer way. Currently just use
 *		xmessage...
 *              
 *	      - Add "current conditions" graphic (as background?). I.e. one of those little
 *		cartoons that show clouds or sun with rain or snow, etc. on it...
 *
 *	      - Scrolling line to display "sundry" parameters.
 *
 *	      - Or maybe auto-switch between panels at some user-defined rate?
 *
 *
 *      Changes:
 *
 *      Version 1.31  - released May 4, 1999.
 *			fixed some conversion bugs in wind speed..
 *
 *      Version 1.30  - released April 13, 1999.
 *			Fixed a bug whereby the App would crash when trying to gain input
 *			focus under non-WindowMaker WMs (focus is now grabbed by
 *			`PointerRoot' not `iconwin').
 *			
 *			Added StationID and `time-of-last-update' labels. To do this I needed
 *			to shrink the fonts down and scrunch them together a bit more.
 *
 *			Added new command line option to change their color;
 *				-tc <color>
 *
 *			Added code to properly decode wind speed when in MPS.
 *
 *			Fixed bug in beaufort wind speed calcs.
 *
 *      Version 1.29  - released March 17, 1999.
 *			Reorganized wmgeneral.c and renamed it xutils.c (wmgeneral.h
 *			-> xutils.h as well ). Also moved it into the same directory as wmWeather.
 *			Now, the openXwindow is split into 2 parts. You first need to call
 *			initXwindow(argc, argv). This allows us to check the display depth
 *			before we commit to a particular pixmap (this will be useful in my
 *			other DockApps to dynamically set appropriate pixmaps based on depth).
 *			Got rid of alot of the other routines that I never use.
 *
 *			Added 4 more command line option to set the colors of the text:
 *
 *				-bc  <color> for setting the BackGround color.
 *				-lc  <color> for setting the Label color.
 *				-dc  <color> for setting the Data color.
 *				-wgc <color> for setting the Wind Gust color.
 *
 *			Also cleaned up the pixmap to minimize the number of colors used.
 *		
 *			Changed metric toggle to work with a key press (any key). 		
 *		
 *			Added double click support. Now double clicking does the following:
 *				
 *				Double Mouse Left: pops up the fully decoded METAR file
 *						   in xmessage.
 *
 *				Double Mouse Middle: Currently undefined.
 *
 *				Double Mouse Right: Forces a new update (i.e. download.)
 *
 *
 *      Version 1.28  - released March 9, 1999.
 *			Changed -celsius (-c) option to -metric (-m). Naming makes more
 *			sense that way...
 *
 *			Added -W option to display WindChill instead of DewPoint.
 *			Since Windchill is not always available, we only show it if its
 *			available. If its not, we paste up DewPoint as default.
 *
 *			Also added -mps option to display wind speed in units of 
 *			meters/second (when in -metric mode).
 *
 *      Version 1.27  - released March 8, 1999.
 *			fixed bug in speed calculation when wind is gusting.
 *
 *      Version 1.26  - released February 24, 1999.
 *			Added -delay option. 
 *
 *      Version 1.25  - released February 16, 1999.
 *			Added Wind speeds on the 'Beaufort scale'
 *			Thanks to Paul Martin <pm@zetnet.net> for this addition.
 *
 *      Version 1.24  - released February 12, 1999.
 *			Added --passive-ftp option to wget.
 *
 *      Version 1.23  - released February 2, 1999.
 *                      Few more bug fixes...
 *                      Added support for different Pressure units...
 *
 *
 *      Version 1.22  - released February 1, 1999.
 *                 	Fixed minor bug in direction abbreviations. Added a bit more to man
 *                	page.
 *
 *      Version 1.21  - released January 29, 1999.
 *                	Fixed a problem in the perl script. Made the file paths absolute.
 *
 *	Version 1.2   - released January 29, 1999.
 *			Added Wind speed line. Ended up decoding the Raw METAR line.
 *			Fixed a few bugs...
 *			Changed location of files from /tmp to ~/.wmWeatherReports
 *			Changed units of pressure and wind speed to mmHg and km/h
 *                      when Metric is set. (Really should change the flag to -metric).
 *
 *	Version 1.1   - released January 25, 1999.
 *			Bug fixes...
 *			Added command line switch to display Temp's in deg. C
 *
 *
 *	Version 1.0   - released January 19, 1999.
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
#include <ctype.h>
#include <time.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "xutils.h"
#include "wmWeather_master.xpm"
#include "wmWeather_mask.xbm"



/* 
 *  Delay between refreshes (in microseconds) 
 */
#define DELAY 10000L
#define WMWEATHER_VERSION "1.31"
#define DEFAULT_UPDATEDELAY 900L




void  ParseCMDLine(int argc, char *argv[]);
void  ButtonPressEvent(XButtonEvent *);
void  KeyPressEvent(XKeyEvent *);
char *StringToUpper(char *);


char 		StationID[10];
int		UpToDate = 0;
int		ShowWindChill = 0;
int		WindChillAvail = 1;
int		Metric = 0;
int		Beaufort = 0;
int		ForceUpdate = 1;
int		ForceDownload = 1;
int		PressureUnits = 0;
int		MetersPerSecond = 0;
double		PressureConv = 1.0;
long		UpdateDelay;
int             GotFirstClick1, GotDoubleClick1;
int             GotFirstClick2, GotDoubleClick2;
int             GotFirstClick3, GotDoubleClick3;
int             DblClkDelay;


/*
 * In a more readable form the Compass directions are:
 * static char *CompassDirection[] = { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
 *						"S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"}
 * We convert to digits in the sequence `NWSE' so we dont have to put all these
 * combinations into the pixmap.
 */
static char 	*CompassDirection[] = { "0", "003", "03", "303", "3", "323", "23", "223", 
						"2", "221", "21", "121", "1", "101", "01", "001"};



char	LabelColor[30]    	= "#79bdbf";
char	WindGustColor[30] 	= "#ff0000";
char	DataColor[30]     	= "#ffbf50";
char	BackColor[30]     	= "#181818";
char	StationTimeColor[30]    = "#c5a6ff";



/*  
 *   main  
 */
int main(int argc, char *argv[]) {

struct tm	*tTime;
XEvent		event;
int		n, s, m, dt1, dt2, dt3, yd;
int		i, j, len;
char		dir[5];
int 		Year, Month, Day;
int		Hours, Mins, Secs;
int		UpdateLTHour = 0.0, UpdateLTMin = 0.0, UpdateUTHour, UpdateUTMin;
long		CurrentLocalTime;
double		UpdateUT, UpdateLT, UT, LT, DT, hour24();


double		jd(), CurrentJD;
char		command[1024], Line[512], FileName[10];
int		Tens, q, digit, chr;
double		Pressure, Temperature, sgn, Humidity, DewPoint, WindChill, val;
double		Direction, Speed;
FILE		*fp;










	  
    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);
	   

	   
    /*
     *  Do the window opening in 2 stages. After the initXwindow() call,
     *  we know what the Depth of the Display is. We can then pick an appropriate
     *  XPM to use. I.e. may want to use one that has fewer colors to make the App work
     *  better on a low-color 8-bit display.
     */
    initXwindow(argc, argv);
    openXwindow(argc, argv, wmWeather_master, wmWeather_mask_bits, wmWeather_mask_width, 
		wmWeather_mask_height, BackColor, LabelColor, WindGustColor, DataColor, StationTimeColor);



	   
    /*
     *  Loop until we die
     */
    n = 32000;
    s = 32000;
    m = 32000;
    dt1 = 32000;
    dt2 = 32000;
    dt3 = 32000;
    DblClkDelay = 32000;
    UpToDate = 0;
    while(1) {




	/*
	 *  Keep track of # of seconds
	 */
	if (m > 100){
	
	    m = 0;
	    ++dt1;
	    ++dt2;
	    ++dt3;
	
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
                        ButtonPressEvent(&event.xbutton);
                        break;
                case KeyPress:
                        KeyPressEvent(&event.xkey);
                        break;
                case ButtonRelease:
                        break;
                case EnterNotify:
                        XSetInputFocus(display, PointerRoot, RevertToParent, CurrentTime);
                        break;
                case LeaveNotify:
                        XSetInputFocus(display, PointerRoot, RevertToParent, CurrentTime);
                        break;

            }
        }





	


	/*
	 *  Check the Current Conditions file every (approx.) several seconds.
	 *  Can significantly reduce this frequency later. But its
	 *  easier to debug this way...
	 *  Do this before trying to download again! The file may be there and it
	 *  may be Up-To-Date!
	 */
	if ((dt2 > 5)||(ForceUpdate)){
	    
	    dt2 = 0;

	    /*
	     *  Compute Current Julian Date
	     */
	    CurrentLocalTime = time(CurrentTime);
	    tTime = gmtime(&CurrentLocalTime);
	    Year  = tTime->tm_year+1900;
	    Month = tTime->tm_mon+1;
	    Day   = tTime->tm_mday;
	    Hours = tTime->tm_hour;
	    Mins  = tTime->tm_min;
	    Secs  = tTime->tm_sec;
	    UT = (double)Hours + (double)Mins/60.0 + (double)Secs/3600.0;
	    CurrentJD = jd(Year, Month, Day, UT);

	    CurrentLocalTime = time(CurrentTime);
	    tTime = localtime(&CurrentLocalTime);
	    Year  = tTime->tm_year+1900;
	    Month = tTime->tm_mon+1;
	    Day   = tTime->tm_mday;
	    Hours = tTime->tm_hour;
	    Mins  = tTime->tm_min;
	    Secs  = tTime->tm_sec;
	    LT = (double)Hours + (double)Mins/60.0 + (double)Secs/3600.0;

	    DT = UT - LT;
	    if (DT > 24.0) DT -= 24.0;
	    if (DT < 0.00) DT += 24.0;
	    


    	    /*
    	     *  Read in weather data
    	     */
	    sprintf(FileName, "%s/.wmWeatherReports/%s.dat", getenv("HOME"), StationID);
    	    if ((fp = fopen(FileName, "r")) != NULL){

	    	fgets(Line, 512, fp);
	    	fgets(Line, 512, fp);
	    	fgets(Line, 512, fp);
	    	fscanf(fp, "%d:%d", &UpdateUTHour, &UpdateUTMin);
		if (UpdateUTHour != 99){
		    UpdateUT = UpdateUTHour + UpdateUTMin/60.0;
		    UpdateLT = UpdateUT - DT;
		    if (UpdateLT < 0.0) UpdateLT += 24.0;
		    if (UpdateLT > 24.0) UpdateLT -= 24.0;
		    UpdateLTHour = (int)UpdateLT;
		    UpdateLTMin = (int)((UpdateLT - (double)UpdateLTHour)*60.0 + 0.5);
		    if (UpdateLTMin >= 60){
			++UpdateLTHour;
			if (UpdateLTHour >= 24) UpdateLTHour = 0;
			UpdateLTMin = 0;
		    }
		} else {
		    UpdateLTHour = 99;
		    UpdateLTMin = 99;
		}

	    	fscanf(fp, "%lf", &Temperature);
		if (Metric) Temperature = (Temperature-32.0)*5.0/9.0;
	    	fscanf(fp, "%lf", &DewPoint);
		if (Metric) DewPoint = (DewPoint-32.0)*5.0/9.0;

	    	fscanf(fp, "%lf", &WindChill);
	        /*
	         *  If WindChill is not available, revert to DewPoint
	         */
	        WindChillAvail = (WindChill < -900.0) ? 0 : 1;
		if (Metric) WindChill = (WindChill-32.0)*5.0/9.0;

	    	fscanf(fp, "%lf", &Pressure); Pressure += 0.005;
		Pressure *= PressureConv;
	    	fscanf(fp, "%lf", &Humidity);
	    	fscanf(fp, "%lf", &Direction);
	    	fscanf(fp, "%lf", &Speed);
		if (Metric){
		    if (MetersPerSecond) Speed *= 0.4473;
		    else if (!Beaufort)  Speed *= 1.609;
		}
    	        fclose(fp);

	    } else {

	    	Temperature = -9999.0;
	    	DewPoint    = -9999.0;
		WindChill   = -9999.0;
	    	Humidity    = -9999.0;
	    	Pressure    = -9999.0;
	    	Direction   = -9999.0;
	    	Speed       = -9999.0;

	    }



	} 







	/*
	 *  Draw window.
	 */
	if ( (dt3 > 5) || ForceUpdate){


	    dt3 = 0;
	    


	    /*
	     * Clear window.
	     */
	    copyXPMArea(5, 69, 54, 54, 5, 5);


	    /*
	     * Paste up Station ID and time of last update.
	     */

	    q = 0;
	    chr = (int)StationID[0] - 65; copyXPMArea(chr*5+2, 128, 5, 6, 7+q, 6); q+= 5;
	    chr = (int)StationID[1] - 65; copyXPMArea(chr*5+2, 128, 5, 6, 7+q, 6); q+= 5;
	    chr = (int)StationID[2] - 65; copyXPMArea(chr*5+2, 128, 5, 6, 7+q, 6); q+= 5;
	    chr = (int)StationID[3] - 65; copyXPMArea(chr*5+2, 128, 5, 6, 7+q, 6); q+= 5;

	    if (UpdateLTHour != 99){
	    	q = 0; 
	    	Tens = (int)(UpdateLTHour);
	    	copyXPMArea(Tens/10*5+2, 135, 5, 6, 36+q, 6); q+= 5;
	    	copyXPMArea(Tens%10*5+2, 135, 5, 6, 36+q, 6); q+= 5;
	    	copyXPMArea(53, 135, 1, 6, 36+q, 6); q+= 2;
	    	Tens = (int)(UpdateLTMin);
	    	copyXPMArea(Tens/10*5+2, 135, 5, 6, 36+q, 6); q+= 5;
	    	copyXPMArea(Tens%10*5+2, 135, 5, 6, 36+q, 6); q+= 5;
	    }









	    /*
	     * Paste up Temperature.
	     */
	    if ((Temperature > -999.0)&&(Temperature < 1000.0)){
	        sgn = (Temperature < 0.0) ? -1.0 : 1.0;
	        Temperature *= sgn;
		Temperature = (double)((int)(Temperature + 0.5));
	        q = 0;
	        if (Temperature >= 100.0){
	            if (sgn < 0.0) { copyXPMArea(0*5+66, 35, 5, 6, 25, 15); q += 5; }
		    digit = (int)(Temperature/100.0);
	            copyXPMArea(digit*5+66, 57, 5, 6, 25+q, 15); q+= 5;
	            Tens = (int)(Temperature-digit*100.0);
	            copyXPMArea(Tens/10*5+66, 57, 5, 6, 25+q, 15); q+= 5;
	            copyXPMArea(Tens%10*5+66, 57, 5, 6, 25+q, 15); q+= 5;
	        } else {
	            if (sgn < 0.0) { copyXPMArea(0*5+66, 35, 5, 6, 25, 15); q += 5; }
	            Tens = (int)(Temperature);
	            if (Tens >= 10) { copyXPMArea(Tens/10*5+66, 57, 5, 6, 25+q, 15); q+= 5; }
	            copyXPMArea(Tens%10*5+66, 57, 5, 6, 25+q, 15); q+= 5;
	        }
		if (Metric){
	            copyXPMArea(72, 34, 3, 3, 25+q, 14); q += 4;
	            copyXPMArea(81, 35, 4, 6, 25+q, 15);
		} else {
	            copyXPMArea(72, 34, 8, 7, 25+q, 14);
		}

	    }






	    /*
	     *   Paste up DewPoint or WindChill Temperature.
	     */
	    if (ShowWindChill && WindChillAvail){
		val = WindChill;
		copyXPMArea(66, 87, 17, 8, 5, 24);
	    } else {
		val = DewPoint;
		copyXPMArea(5, 87, 17, 8, 5, 24);
	    }
	    
	    if ((val > -999.0)&&(val < 1000.0)){
	        sgn = (val < 0.0) ? -1.0 : 1.0;
	        val *= sgn;
		val = (double)((int)(val + 0.5));
	        q = 0;
	        if (val >= 100.0){
	            if (sgn < 0.0) { copyXPMArea(0*5+66, 57, 5, 6, 25, 24); q += 5; }
		    digit = (int)(val/100.0);
	            copyXPMArea(digit*5+66, 57, 5, 6, 25+q, 24); q+= 5;
	            Tens = (int)(val-digit*100.0);
	            copyXPMArea(Tens/10*5+66, 57, 5, 6, 25+q, 24); q+= 5;
	            copyXPMArea(Tens%10*5+66, 57, 5, 6, 25+q, 24); q+= 5;
	        } else {
	            if (sgn < 0.0) { copyXPMArea(0*5+66, 35, 5, 6, 25, 24); q += 5; }
	            Tens = (int)(val);
	            if (Tens >= 10) { copyXPMArea(Tens/10*5+66, 57, 5, 6, 25+q, 24); q+= 5; }
	            copyXPMArea(Tens%10*5+66, 57, 5, 6, 25+q, 24); q+= 5;
	        }
		if (Metric){
	            copyXPMArea(72, 34, 3, 3, 25+q, 23); q += 4;
	            copyXPMArea(81, 35, 4, 6, 25+q, 24);
		} else {
	            copyXPMArea(72, 34, 8, 7, 25+q, 23);
		}
	    }









	    /*
	     * Paste up Pressure.
	     */
	    q = 0; s= 0;
	    if ((Pressure > 0.0)&&(Pressure <= 10000.0)){
		val = Pressure;

		digit = (int)(val/1000.0);
	        if (digit > 0) { copyXPMArea(digit*5+66, 57, 5, 6, 25+q, 33); q += 5; }
		val -= (double)digit*1000;

		digit = (int)(val/100.0);
	        if ((digit > 0)||(Pressure > 999.0)) { copyXPMArea(digit*5+66, 57, 5, 6, 25+q, 33); q += 5; }
		val -= (double)digit*100;

		digit = (int)(val/10.0);
	        if ((digit > 0)||(Pressure > 99.0)) { copyXPMArea(digit*5+66, 57, 5, 6, 25+q, 33); q += 5; }
		val -= (double)digit*10;

		digit = (int)val;
	        copyXPMArea(digit*5+66, 57, 5, 6, 25+q, 33); q += 5;
		val -= (double)digit;


		if ((PressureUnits != 2)||(!Metric)){

	            copyXPMArea(10*5+66+1, 57, 4, 6, 25+q, 33); q += 4;

		   val *= 10; digit = (int)val;
	           copyXPMArea(digit*5+66, 57, 5, 6, 25+q, 33); q += 5;
		   val -= (double)digit;

		    if (!Metric){
		        val *= 10; digit = (int)val;
	                copyXPMArea(digit*5+66, 57, 5, 6, 25+q, 33); q += 5;
		        /* val -= (double)digit; */
		    }
		}


	    }







	    /*
	     * Paste up Humidity.
	     */
	    q = 0;
	    if ((Humidity > -999.0)&&(Humidity <= 100.0)){
	        if (Humidity == 100.0){
	            copyXPMArea(1*5+66, 57, 5, 6, 25+q, 42); q += 5;
	            copyXPMArea(0*5+66, 57, 5, 6, 25+q, 42); q += 5;
	            copyXPMArea(0*5+66, 57, 5, 6, 25+q, 42); q += 5;
	        } else {
	            Tens = (int)(Humidity);
	            if (Tens >= 10) { copyXPMArea(Tens/10*5+66, 57, 5, 6, 25+q, 42); q += 5; }
	            copyXPMArea(Tens%10*5+66, 57, 5, 6, 25+q, 42); q += 5;
	        }
	        copyXPMArea(121, 57, 5, 6, 25+q, 42);

	    }








	    /*
	     * Paste up Wind Info.
	     */
	    if ((Direction == 0.0)&&(Speed == 0.0)){

		/*
		 *   Just write out `Calm' if both values are 0
		 */
		copyXPMArea(66, 4, 23, 7, 25, 51);

	    } else {

		/*
		 *  If the Direction < 0 this means that there was a "Wind direction
		 *  Variability Group" found in the METAR code. I.e. Direction is variable.
		 *  I think value should be the average. In any case flag it by making it a
		 *  different color.
		 *
		 *  Likewise, if Speed < 0, this means that a "Gusty" modifier was found
		 *  in the RAW METAR code, and the value is only the average of the low
		 *  and high values given. Again, flag it in a different color...
		 *
		 */
	        if ((Direction >= -360.0)&&(Direction <= 360.0)){
	            sgn = (Direction < 0.0) ? -1.0 : 1.0;
		    yd = (sgn < 0.0) ? 50 : 43;
	            Direction *= sgn;
	            q = 0;
		    i = (int)(Direction/360.0*16.0 + 0.5);
		    if (i>15) i = 0;
		    strcpy(dir, CompassDirection[i]);
		    len = strlen(dir);
		    for (j=0; j<len; ++j){
			digit = (int)dir[j] - 48;
			copyXPMArea(digit*5+66, yd, 5, 6, 25+q, 51); q+= 5;
		    }
		    q += 2;
	        } else if (Direction > 0.0){

		    /* 
		     *  In this case, the wind direction is variable with speed < 6 Knots.
		     *  A numerical direction is not given in these cases. Just write out 'VRB'.
		     */
		    q = 0;
		    copyXPMArea(4*5+66, 43, 5, 6, 25+q, 51); q+= 5;
		    copyXPMArea(5*5+66, 43, 5, 6, 25+q, 51); q+= 5;
		    copyXPMArea(6*5+66, 43, 5, 6, 25+q, 51); q+= 9;
	        }

                if (Metric && Beaufort) {
                    int beau = 0;
                    int spd;
 
                    sgn = (Speed < 0.0) ? -1.0 : 1.0;
                    spd = (int)(sgn * (int)Speed);
                    if (spd >  1) { beau = 1; }
                    if (spd >  3) { beau = 2; }
                    if (spd >  4) { beau = 3; }
                    if (spd > 10) { beau = 4; }
                    if (spd > 16) { beau = 5; }
                    if (spd > 21) { beau = 6; }
                    if (spd > 27) { beau = 7; }
                    if (spd > 33) { beau = 8; }
                    if (spd > 40) { beau = 9; }
                    if (spd > 47) { beau = 10; }
                    if (spd > 55) { beau = 11; }
                    if (spd > 63) { beau = 12; }
                    if (spd > 71) { beau = 13; }
                    Speed = sgn * (double) beau;
                    q++; copyXPMArea(76, 35, 4, 6, 25+q, 51); q+= 6;
                }


	        if ((Speed > -999.0)&&(Speed < 1000.0)){
	            sgn = (Speed < 0.0) ? -1.0 : 1.0;
		    yd = (sgn < 0.0) ? 64 : 57;
	            Speed *= sgn;
	            if (Speed >= 100.0){
		        digit = (int)(Speed/100.0);
	                copyXPMArea(digit*5+66, yd, 5, 6, 25+q, 51); q+= 5;
	                Tens = (int)(Speed-digit*100.0);
	                copyXPMArea(Tens/10*5+66, yd, 5, 6, 25+q, 51); q+= 5;
	                copyXPMArea(Tens%10*5+66, yd, 5, 6, 25+q, 51); q+= 5;
	            } else {
	                Tens = (int)(Speed);
	                if (Tens >= 10) { copyXPMArea(Tens/10*5+66, yd, 5, 6, 25+q, 51); q+= 5; }
	                copyXPMArea(Tens%10*5+66, yd, 5, 6, 25+q, 51); q+= 5;
	            }
	        }

	    }








	    /*
	     * Make changes visible
	     */
	    RedrawWindow();



	}



	/*
	 *  Reset "force update" flag
	 */
	ForceUpdate = 0;




	/*
	 *  Check every 5 min if the values are not up to date...
	 */
/*
 *  We still need to add a flashing LED to warn about
 *  times that are out of date. Also need to determine if it is uptodate...
 */
UpToDate = 0;
	if (((!UpToDate)&&(dt1 > UpdateDelay)) || ForceDownload){

	    dt1 = 0;

	    /*
	     *  Execute Perl script to grab the Latest METAR Report
	     */
	    sprintf(command, "GrabWeather %s &", StationID);
	    system(command);

	    ForceDownload = 0;
	    ForceUpdate = 1;

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
    void print_usage();
 
    StationID[0] = '\0';
    PressureUnits = 0;
    MetersPerSecond = 0;
    ShowWindChill = 0;
    UpdateDelay = DEFAULT_UPDATEDELAY;
    for (i = 1; i < argc; i++) {

        if (!strcmp(argv[i], "-display")){

            ++i;

        } else if (!strcmp(argv[i], "-bc")){

	    if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
		fprintf(stderr, "wmWeather: No color found\n");
		print_usage();
		exit(-1);
	    }
            strcpy(BackColor, argv[++i]);

        } else if (!strcmp(argv[i], "-tc")){

	    if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
		fprintf(stderr, "wmWeather: No color found\n");
		print_usage();
		exit(-1);
	    }
            strcpy(StationTimeColor, argv[++i]);

        } else if (!strcmp(argv[i], "-lc")){

	    if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
		fprintf(stderr, "wmWeather: No color found\n");
		print_usage();
		exit(-1);
	    }
            strcpy(LabelColor, argv[++i]);

        } else if (!strcmp(argv[i], "-wgc")){

	    if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
		fprintf(stderr, "wmWeather: No color found\n");
		print_usage();
		exit(-1);
	    }
            strcpy(WindGustColor, argv[++i]);

        } else if (!strcmp(argv[i], "-dc")){

	    if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
		fprintf(stderr, "wmWeather: No color found\n");
		print_usage();
		exit(-1);
	    }
            strcpy(DataColor, argv[++i]);

         } else if (!strcmp(argv[i], "-beaufort")){
 
            Beaufort = 1;

         } else if (!strcmp(argv[i], "-mps")){
 
            MetersPerSecond = 1;

         } else if (!strcmp(argv[i], "-W")){
 
            ShowWindChill = 1;

        } else if ((!strcmp(argv[i], "-metric"))||(!strcmp(argv[i], "-m"))){

	    Metric = 1;

        } else if (!strcmp(argv[i], "-kPa")){

    	    PressureUnits = 1;
	    PressureConv = 3.38639;

        } else if (!strcmp(argv[i], "-hPa")){

    	    PressureUnits = 2;
	    PressureConv = 33.8639;

        } else if (!strcmp(argv[i], "-mmHg")){

    	    PressureUnits = 3;
	    PressureConv = 25.4;
 
        } else if ((!strcmp(argv[i], "-station"))||(!strcmp(argv[i], "-s"))){

	    if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
		fprintf(stderr, "wmWeather: No METAR station ID found\n");
		print_usage();
		exit(-1);
	    }
            strcpy(StationID, StringToUpper(argv[++i]));

        } else if (!strcmp(argv[i], "-delay")) {

	    if( (i+1 >= argc)||(argv[i+1][0] == '-')) {

		fprintf(stderr,"You must give a time with the -delay option.\n");
		print_usage();
		exit(-1);

	    } else if(sscanf(argv[i+1], "%ld", &UpdateDelay) != 1) {

		fprintf(stderr,"Dont understand the delay time you have entered (%s).\n", argv[i+1]);
		print_usage();
		exit(-1);

	    } 
	    /*
	     *  Convert Time to seconds
	     */
	    UpdateDelay *= 60;
	    ++i;
	    
        } else {

	    print_usage();
            exit(-11);

        }
    }

    if (StationID[0] == '\0') {
	fprintf(stderr, "\nwmWeather: You must specify a METAR station code\n\n");
	print_usage();
	exit(1);
    }

    if ((Metric)&&(PressureUnits == 0)){
	PressureUnits = 3;
	PressureConv = 25.4;
    }

    if (!Metric){
	PressureConv = 1.0;
    }

    if (Beaufort && MetersPerSecond){
	fprintf(stderr, "\nwmWeather: You cant use both -beaufort and -mps together.\n\n");
	print_usage();
	exit(1);
    }

}


void print_usage(){

    printf("\nwmWeather version: %s\n", WMWEATHER_VERSION);
    printf("\nusage: wmWeather -s <StationID> [-display <Display>] [-h] [-metric] [-kPa] [-hPa] [-mmHg]\n");
    printf("                   [-beaufort] [-mps] [-W] [-delay <Time in Minutes>] [-bc <color>]\n");
    printf("                   [-lc <color>] [-dc <color>] [-wgc <color>] [-tc <color>]\n\n");
    printf("\t-display <Display>\t\tUse alternate X display.\n\n");
    printf("\t-h\t\t\t\tDisplay help screen.\n");
    printf("\n\t-station <METAR StationID>\n");
    printf("\t-s       <METAR StationID>\tThe 4-character METAR Station ID.\n\n");
    printf("\t-W\t\t\t\tIf available, display WindChill instead\n");
    printf("\t\t\t\t\tof DewPoint Temperature.\n");
    printf("\t-metric\n");
    printf("\t-m\t\t\t\tDisplay variables in metric units.\n\n");
    printf("\t-kPa\t\t\t\tWhen toggled to metric display, show pressure\n");
    printf("\t\t\t\t\tin units of kPa.\n\n");
    printf("\t-hPa\t\t\t\tWhen toggled to metric display, show pressure\n");
    printf("\t\t\t\t\tin units of hPa.\n\n");
    printf("\t-mmHg\t\t\t\tWhen toggled to metric display, show pressure\n");
    printf("\t\t\t\t\tin units of mmHg. (This is the default for metric).\n\n");
    printf("\t-beaufort\t\t\tWhen toggled to metric display, show windspeed\n");
    printf("\t\t\t\t\ton the Beaufort scale. (default is km/h.)\n\n");
    printf("\t-mps\t\t\t\tWhen toggled to metric display, show windspeed\n");
    printf("\t\t\t\t\tin units of meters/second. (default is km/h.)\n\n");
    printf("\t-bc  <color>\t\t\tBackground color. (#7e9e69 is a greenish LCD color).\n\n");
    printf("\t-lc  <color>\t\t\tLabel color.\n\n");
    printf("\t-dc  <color>\t\t\tData color.\n\n");
    printf("\t-wgc <color>\t\t\tGusty-wind/variable-direction color.\n\n");
    printf("\t-tc  <color>\t\t\tStation ID and Time color.\n\n");
    printf("\t-delay <Time in Minutes>\tOverride time (in minutes) between updates (default\n");
    printf("\t\t\t\t\tis %ld minutes). (Times are approximate.)\n", DEFAULT_UPDATEDELAY/60);
    printf("\n\nTo find out more about the METAR/TAF system and to find the \n");
    printf("METAR code for your location, look at:\n\n");
    printf("	 http://www.nws.noaa.gov/oso/oso1/oso12/metar.htm \n\n");
    printf("for NOAA's ""National Weather Service METAR/TAF Information"" page.\n");
    printf("To locate your site ID go to  NOAA's ""Meteorological Station Information\nLookup"" page at:\n\n");
    printf("	 http://www.nws.noaa.gov/oso/siteloc.shtml\n\n");

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



/*
 *  This routine handles button presses. 
 *
 *	- Left Mouse single click toggles Deg F/C for temperatures.
 *	- Some other click event should display the full METAR report -- lots of
 *        juicy stuff in there... Should bring up a separate window...
 *
 *
 */
void ButtonPressEvent(XButtonEvent *xev){

    char 	Command[80];

    /*
     *  Process single clicks.
     */
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
	sprintf(Command, "xmessage -center -file %s/.wmWeatherReports/%s.TXT &", getenv("HOME"), StationID);
	system(Command);
    }


    /*
     *  We got a double click on Mouse Button2 (i.e. the left one)
     */
    if (GotDoubleClick2) {
	GotFirstClick2 = 0;
	GotDoubleClick2 = 0;
    }


    /*
     *  We got a double click on Mouse Button3 (i.e. the left one)
     */
    if (GotDoubleClick3) {
	GotFirstClick3 = 0;
	GotDoubleClick3 = 0;
	ForceDownload = 1;
	ForceUpdate = 1;
    }

   return;


}




/*
 *  This routine handles key presses.
 *
 */
void KeyPressEvent(XKeyEvent *xev){

   Metric = !Metric;



   if (Metric){

	switch(PressureUnits){
	    case 0: PressureConv = 25.4;
		    break;

	    case 1: PressureConv = 3.38639;
		    break;

	    case 2: PressureConv = 33.8639;
		    break;

	    case 3: PressureConv = 25.4;
		    break;
	}

   } else if (!Metric){

	PressureConv = 1.0;

   }



   ForceUpdate = 1;

   return;

}


char *StringToUpper(char *String) {

    int    i;

    for (i = 0; i < strlen(String); i++)
	String[i] = toupper(String[i]);

    return String;

}




