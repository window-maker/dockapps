/*
 *
 *  	wmCalClock-1.25 (C) 1998, 1999 Mike Henderson (mghenderson@lanl.gov)
 * 
 *  		- Its a Calendar Clock....
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
 *
 *      Changes:
 *
 *	Version 1.25  -	released July 2, 1999.
 *			Some optimization + ignores double click if no command set (patch from
 *			Robert Horn).
 *
 *	Version 1.24  -	released March 27, 1999.
 *			Added support for additional fonts for time field;
 *		
 *				-tekton for Tekton
 *				-arial  for Arial (Helvetica) (this is the same font as usual)
 *				-luggerbug for LuggerBug 
 *				-comicsans for ComicSans
 *				-jazz for JazzPoster
 *
 *			Different width fonts get used depending on whether or not seconds
 *			are displayed.
 *
 *	Version 1.23  -	released March 20, 1999.
 *		        Switched from wmgeneral.c stuff to xutils.c (a more stripped down version
 *		   	of wmgeneral).
 *			Centered Calendar text better.
 *			Added command line options and code to change colors of the time
 *			field digits and the background of the time field. So now you can
 *			get things like darkblue on light grey or very dark color on an LCD-ish
 *			colored background (e.g. wmCalClock -bc #6e9e69 -tc #001100)..
 *			Rewrote the command line parsing routine.
 *
 *	Version 1.21  -	released February 4, 1999.
 *		      	cosmetic for AfterStep users. removed spurious black line at RHS edge of mask.
 *
 *	Version 1.20  -	released January 14, 1999.
 *                    	Changed support for LowColor Pixmap. Now, check for Depth
 *                    	automatically. If its <= 8, then use LowColor.
 *
 *
 *	Version 1.11  -	released January 8, 1999.
 *		       	Fixed bug in 12-hour mode. Now displays 12:xx:xx AM instead
 *                     	of 0:xx:xx AM.
 *
 *
 *	Version 1.10  - released January 7, 1999.
 *                     	Added support for LowColor Pixmap. (21 colors may still be a
 *		       	bit high, but the poor saps with 8-bit displays can at least run 
 *                     	it now.)
 *
 *	Version 1.02  -	released January 7, 1999.
 *                     	Fixed bug in AM/PM determination...
 *
 *	Version 1.01  -	released January 3, 1999.
 *                     	Added "-S" option to inhibit drawing of seconds.
 *
 *	Version 1.00  -	released December 16, 1998.
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
#include "xutils.h"
#include "wmCalClock_master.xpm"
#include "wmCalClock_master_LowColor.xpm"
#include "wmCalClock_mask.xbm"



/* 
 *  Delay between refreshes (in microseconds) 
 */
#define DELAY 10000L
#define WMCALCLOCK_VERSION "1.25"





void ParseCMDLine(int argc, char *argv[]);
void ButtonPressEvent(XButtonEvent *);
void print_usage();




int	xsMonth[12] = { 150, 170, 190, 212, 233, 256, 276, 294, 317, 337, 357, 380 };
int	xeMonth[12] = { 168, 188, 210, 231, 254, 275, 292, 314, 335, 355, 377, 398 };
int	xdMonth[12];
int	yMonth = 80; 
int	ydMonth = 13;

int	xsDayOfWeek[7] = { 293, 150, 177, 201, 228, 253, 271 };
int	xeDayOfWeek[7] = { 314, 175, 199, 226, 250, 269, 290 };
int	xdDayOfWeek[7];
int	yDayOfWeek = 95; 
int	ydDayOfWeek = 13;

/*
 *  I think this is 28??-pixel high adobe-myriad-bold
 */
int	xsDayOfMonth[31] = {        118, 161, 205, 248, 291, 335, 378, 421, 465,
				75, 118, 162, 205, 248, 292, 335, 378, 422, 465,
				75, 118, 162, 205, 248, 292, 335, 378, 422, 465,
				75, 118 };
/*
 *  I think this is 16-pixel high adobe-myriad-bold?
 */
int	xeDayOfMonth[31] = {         147, 193, 236, 282, 324, 368, 411, 454, 498,
				107, 146, 192, 235, 281, 323, 367, 410, 453, 497,
				108, 147, 193, 236, 282, 324, 368, 411, 454, 498,
				108, 147 };
int	xeDayOfMonth2[31] = {        144, 190, 234, 278, 320, 365, 407, 451, 494,
				103, 143, 189, 233, 277, 319, 364, 406, 450, 493,
				104, 144, 190, 234, 278, 320, 365, 407, 451, 494,
				104, 144 };

/*
 *  I think this is 16-pixel high adobe-myriad-bold?
 */
int	yDayOfMonth[31] = { 5, 5, 5, 5, 5, 5, 5, 5, 5,
				30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
				55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
				80, 80 };
int	xdDayOfMonth[31];
int	xdDayOfMonth2[31];
int	ydDayOfMonth = 23;




/* 
 *  Luggerbug Font Narrow - 13 pixels high.
 */
int	xsDigits_Luggerbug13n[11] = { 75, 84, 92, 101, 110, 119, 127, 136, 143, 151, 159 };
int	xeDigits_Luggerbug13n[11] = { 80, 89, 97, 106, 115, 123, 132, 139, 147, 156, 159 };
int	xdDigits_Luggerbug13n[11];
int	yDigits_Luggerbug13n  = 150;
int	ydDigits_Luggerbug13n = 13;

/* 
 *  Luggerbug Font - 13 pixels high.
 */
int	xsDigits_Luggerbug13[11] = { 75, 89, 103, 117, 131, 146, 159, 172, 184, 197, 208 };
int	xeDigits_Luggerbug13[11] = { 83, 97, 110, 125, 139, 153, 166, 178, 191, 205, 209 };
int	xdDigits_Luggerbug13[11];
int	yDigits_Luggerbug13  = 136;
int	ydDigits_Luggerbug13 = 13;

/* 
 *  ComicSans Font - 12 pixels high.
 */
int	xsDigits_ComicSans12n[11] = { 338, 349, 359, 370, 380, 390, 401, 411, 422, 432, 444 };
int	xeDigits_ComicSans12n[11] = { 343, 353, 364, 374, 385, 396, 406, 417, 427, 438, 445 };
int	xdDigits_ComicSans12n[11];
int	yDigits_ComicSans12n  = 123;
int	ydDigits_ComicSans12n = 12;

/* 
 *  ComicSans Font - 11 pixels high.
 */
int	xsDigits_ComicSans11[11] = { 338, 353, 366, 380, 392, 407, 420, 434, 448, 461, 471 };
int	xeDigits_ComicSans11[11] = { 345, 357, 372, 386, 400, 413, 427, 441, 454, 468, 473 };
int	xdDigits_ComicSans11[11];
int	yDigits_ComicSans11  = 111;
int	ydDigits_ComicSans11 = 11;

/* 
 *  JazzPoster Font Narrow  - 12 pixels high.
 */
int	xsDigits_JazzPoster12n[11] = { 211, 220, 226, 233, 241, 249, 256, 263, 271, 278, 286 };
int	xeDigits_JazzPoster12n[11] = { 217, 223, 231, 238, 247, 253, 261, 268, 276, 284, 286 };
int	xdDigits_JazzPoster12n[11];
int	yDigits_JazzPoster12n  = 122;
int	ydDigits_JazzPoster12n = 12;

/* 
 *  JazzPoster Font  - 12 pixels high.
 */
int	xsDigits_JazzPoster12[11] = { 211, 225, 234, 246, 258, 271, 282, 293, 305, 317, 328 };
int	xeDigits_JazzPoster12[11] = { 221, 230, 243, 254, 268, 278, 290, 301, 314, 325, 329 };
int	xdDigits_JazzPoster12[11];
int	yDigits_JazzPoster12  = 109;
int	ydDigits_JazzPoster12 = 12;


/* 
 *  Tekton Font  - 12 pixels high Narrow (13 pixels high actually).
 */
int	xsDigits_Tekton12n[11] = { 75, 84, 90, 97, 105, 114, 122, 131, 138, 147, 156 };
int	xeDigits_Tekton12n[11] = { 81, 86, 94, 103, 111, 119, 128, 135, 144, 152, 157 };
int	xdDigits_Tekton12n[11];
int	yDigits_Tekton12n  = 122;
int	ydDigits_Tekton12n = 13;

/* 
 *  Tekton Font  - 12 pixels high.
 */
int	xsDigits_Tekton12[11] = { 75, 89, 98, 111, 124, 137, 150, 164, 176, 191, 205  };
int	xeDigits_Tekton12[11] = { 84, 92, 105, 119, 132, 145, 159, 171, 185, 199, 206  };
int	xdDigits_Tekton12[11];
int	yDigits_Tekton12  = 108;
int	ydDigits_Tekton12 = 12;

/*
 *  Monotype-arial-bold-narrow - 10 pixels high.
 */
int     xsDigits_Arial10[11] = { 320, 326, 333, 339, 346, 352, 358, 364, 371, 377, 384 };
int     xeDigits_Arial10[11] = { 325, 331, 338, 344, 351, 357, 363, 369, 376, 382, 385 };
int	xdDigits_Arial10[11];
int	yDigits_Arial10  = 95;
int	ydDigits_Arial10 = 10;


int     xsDigits[11];
int     xeDigits[11];
int	xdDigits[11];
int	yDigits;
int	ydDigits;


int     xsAMPM[2] = { 390, 396 };
int     xeAMPM[2] = { 394, 400 };
int	xdAMPM[2];
int	yAMPM  = 95;
int	ydAMPM = 6;
int	Show24HourTime = 0;
int	ShowGreenwichTime = 0;
int	ShowSiderealTime = 0;
double	Longitude;
int	Flag = 0;
int	Beep = 0;
int	Volume = 100;
int	ShowSeconds = 1;
int	UseLowColorPixmap = 0;
int	UseArial = 0;
int	UseComicSans = 0;
int	UseTekton = 0;
int	UseLuggerbug = 0;
int	UseJazzPoster = 0;
int     GotFirstClick1, GotDoubleClick1;
int     GotFirstClick2, GotDoubleClick2;
int     GotFirstClick3, GotDoubleClick3;
int     DblClkDelay;
int     HasExecute = 0;		/* controls perf optimization */
char	ExecuteCommand[1024];


char    TimeColor[30]    	= "#ffff00";
char    BackgroundColor[30]    	= "#181818";






/*  
 *   main  
 */
int main(int argc, char *argv[]) {


    struct tm		*Time;
    XEvent		event;
    int			i, n, wid, extrady, extradx;
    int 		Year, Month, DayOfWeek, DayOfMonth, OldDayOfMonth;
    int			Hours, Mins, Secs, OldSecs, digit, xoff, D[10], xsize;
    long		CurrentLocalTime;
    double		UT, TU, TU2, TU3, T0, gmst, jd(), hour24();


    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);






    /*
     *  Set the font
     */
    if (UseTekton && !ShowSeconds){

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_Tekton12[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_Tekton12[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_Tekton12[i];
	yDigits = yDigits_Tekton12;
	ydDigits = ydDigits_Tekton12;
	extrady = -1;
	extradx = 0;

    } else if (UseTekton && ShowSeconds){

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_Tekton12n[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_Tekton12n[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_Tekton12n[i];
	yDigits = yDigits_Tekton12n;
	ydDigits = ydDigits_Tekton12n;
	extrady = -2;
	extradx = 1;

    } else if (UseLuggerbug && !ShowSeconds){

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_Luggerbug13[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_Luggerbug13[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_Luggerbug13[i];
	yDigits = yDigits_Luggerbug13;
	ydDigits = ydDigits_Luggerbug13;
	extrady = -2;
	extradx = 1;

    } else if (UseLuggerbug && ShowSeconds){

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_Luggerbug13n[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_Luggerbug13n[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_Luggerbug13n[i];
	yDigits = yDigits_Luggerbug13n;
	ydDigits = ydDigits_Luggerbug13n;
	extrady = -2;
	extradx = 1;

    } else if (UseComicSans && !ShowSeconds){

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_ComicSans11[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_ComicSans11[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_ComicSans11[i];
	yDigits = yDigits_ComicSans11;
	ydDigits = ydDigits_ComicSans11;
	extrady = -1;
	extradx = 1;

    } else if (UseComicSans && ShowSeconds){

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_ComicSans12n[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_ComicSans12n[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_ComicSans12n[i];
	yDigits = yDigits_ComicSans12n;
	ydDigits = ydDigits_ComicSans12n;
	extrady = -1;
	extradx = 1;

    } else if (UseJazzPoster && !ShowSeconds){

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_JazzPoster12[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_JazzPoster12[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_JazzPoster12[i];
	yDigits = yDigits_JazzPoster12;
	ydDigits = ydDigits_JazzPoster12;
	extrady = -1;
	extradx = 0;

    } else if (UseJazzPoster && ShowSeconds){

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_JazzPoster12n[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_JazzPoster12n[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_JazzPoster12n[i];
	yDigits = yDigits_JazzPoster12n;
	ydDigits = ydDigits_JazzPoster12n;
	extrady = -1;
	extradx = 1;

    } else {

	for (i=0; i<11; ++i) xsDigits[i] = xsDigits_Arial10[i];
	for (i=0; i<11; ++i) xeDigits[i] = xeDigits_Arial10[i];
	for (i=0; i<11; ++i) xdDigits[i] = xdDigits_Arial10[i];
	yDigits = yDigits_Arial10;
	ydDigits = ydDigits_Arial10;
	extrady = 0;
	extradx = 0;

    }


    /*
     *  Compute widths of digits etc...
     *  Should hand-encode for efficiency, but its easier to do this for development...
     */
    for (i=0; i<12; ++i)  xdMonth[i] = xeMonth[i] - xsMonth[i] + 1;
    for (i=0; i<7;  ++i)  xdDayOfWeek[i] = xeDayOfWeek[i] - xsDayOfWeek[i] + 1;
    for (i=0; i<31; ++i)  xdDayOfMonth[i] = xeDayOfMonth[i] - xsDayOfMonth[i] + 1;
    for (i=0; i<31; ++i)  xdDayOfMonth2[i] = xeDayOfMonth2[i] - xsDayOfMonth[i] + 1;
    for (i=0; i<11; ++i)  xdDigits[i] = xeDigits[i] - xsDigits[i] + 1;
    for (i=0; i<2;  ++i)  xdAMPM[i] = xeAMPM[i] - xsAMPM[i] + 1;



  
   


   
    
    initXwindow(argc, argv);
    if (DisplayDepth <= 8) UseLowColorPixmap = 1;

    if (UseLowColorPixmap)
        openXwindow(argc, argv, wmCalClock_master_LowColor, wmCalClock_mask_bits, wmCalClock_mask_width, wmCalClock_mask_height);
    else
        openXwindow(argc, argv, wmCalClock_master, wmCalClock_mask_bits, wmCalClock_mask_width, wmCalClock_mask_height);





   
    /*
     *  Loop until we die
     */
    n = 32000;
    OldDayOfMonth = -1;
    OldSecs = -1;
    while(1) {


	/*
	 *  Only process every 10th cycle of this loop. We run it faster 
 	 *  to catch expose events, etc...
	 *
	 */
	if ( HasExecute == 0 || n>10){

	    n = 0;

	    if (ShowGreenwichTime){

                CurrentLocalTime = time(CurrentTime);
	        Time = gmtime(&CurrentLocalTime);
	        DayOfMonth = Time->tm_mday-1;
	        DayOfWeek = Time->tm_wday;
	        Month = Time->tm_mon;
	        Hours = Time->tm_hour;
	        Mins  = Time->tm_min;
	        Secs  = Time->tm_sec;

	    } else if (ShowSiderealTime){

		Show24HourTime = 1;
                CurrentLocalTime = time(CurrentTime);
	        Time = gmtime(&CurrentLocalTime);
	        DayOfMonth = Time->tm_mday-1;
	        DayOfWeek = Time->tm_wday;
		Year  = Time->tm_year + 1900; /* this is NOT a Y2K bug */
	        Month = Time->tm_mon;
	        Hours = Time->tm_hour;
	        Mins  = Time->tm_min;
	        Secs  = Time->tm_sec;
		UT = (double)Hours + (double)Mins/60.0 + (double)Secs/3600.0;

		/*
		 *  Compute Greenwich Mean Sidereal Time (gmst)
		 *  The TU here is number of Julian centuries
		 *  since 2000 January 1.5
		 *  From the 1996 astronomical almanac
		 */
		TU = (jd(Year, Month+1, DayOfMonth+1, 0.0) - 2451545.0)/36525.0;
		TU2 = TU*TU;
		TU3 = TU2*TU;
		T0 = (6.0 + 41.0/60.0 + 50.54841/3600.0) + 8640184.812866/3600.0*TU
			+ 0.093104/3600.0*TU2 - 6.2e-6/3600.0*TU3;
		gmst = hour24(hour24(T0) + UT*1.002737909 + Longitude/15.0);
		Hours = (int)gmst;
		gmst  = (gmst - (double)Hours)*60.0;
		Mins  = (int)gmst;
		gmst  = (gmst - (double)Mins)*60.0;
		Secs  = (int)gmst;

	    } else {

                CurrentLocalTime = time(CurrentTime);
	        Time = localtime(&CurrentLocalTime);
	        DayOfMonth = Time->tm_mday-1;
	        DayOfWeek = Time->tm_wday;
	        Month = Time->tm_mon;
	        Hours = Time->tm_hour;
	        Mins  = Time->tm_min;
	        Secs  = Time->tm_sec;

	    }

	    /*
	     *  Flag indicates AM (Flag=0)  or PM (Flag=1)
	     */
	    if (!Show24HourTime){
		Flag  = (Hours >= 12) ? 1 : 0;
		if (Hours == 0)
		    Hours = 12;
		else 
		    Hours = (Hours > 12) ? Hours-12 : Hours;
	    }




	    /*
	     *  Blank the HH:MM section....
	     */
	    xsize = 0;
	    /* dont show leading zeros */
	    if ((digit = Hours / 10) > 0){
	        D[0] = digit, xsize += (xdDigits[digit]+1);
	    } else{
		D[0] = -1;
	    }
	    digit = Hours % 10, D[1] = digit, xsize += (xdDigits[digit]+1);
	    digit = 10,         D[2] = digit, xsize += (xdDigits[digit]+1);
	    digit = Mins / 10,  D[3] = digit, xsize += (xdDigits[digit]+1);
	    digit = Mins % 10,  D[4] = digit, xsize += (xdDigits[digit]+1);
	    if (ShowSeconds){
	        digit = 10,         D[5] = digit, xsize += (xdDigits[digit]+1);
	        digit = Secs / 10,  D[6] = digit, xsize += (xdDigits[digit]+1);
	        digit = Secs % 10,  D[7] = digit, xsize += (xdDigits[digit]);
	    }
	    xoff = ((Hours>9)&&(!Show24HourTime)&&(ShowSeconds)) ? 28 - xsize/2 : 31 - xsize/2;
	    copyXPMArea(5, 110, 54, 15, 5, 5);


            /* 
             *  Draw Hours
             */
	    
	    /* dont show leading zeros */
	    if (D[0] > -1){
	    	digit = D[0];
	    	copyXPMArea(xsDigits[digit], yDigits, xdDigits[digit], ydDigits, xoff+extradx, 7+extrady);
	    	xoff += (xdDigits[digit]+1);
	    }

	    digit = D[1];
	    copyXPMArea(xsDigits[digit], yDigits, xdDigits[digit], ydDigits, xoff+extradx, 7+extrady);
	    xoff += (xdDigits[digit]+1);

            /* 
             *  Draw Colon
             */
	    digit = 10;
	    copyXPMArea(xsDigits[digit], yDigits, xdDigits[digit], ydDigits, xoff+extradx, 7+extrady);
	    xoff += (xdDigits[digit]+1);

            /* 
             *  Draw Minutes
             */
	    digit = D[3];
	    copyXPMArea(xsDigits[digit], yDigits, xdDigits[digit], ydDigits, xoff+extradx, 7+extrady);
	    xoff += (xdDigits[digit]+1);

	    digit = D[4];
	    copyXPMArea(xsDigits[digit], yDigits, xdDigits[digit], ydDigits, xoff+extradx, 7+extrady);
	    xoff += (xdDigits[digit]+1);

	    if (ShowSeconds){

            	/* 
            	 *  Draw Colon
            	 */
	    	digit = 10;
	    	copyXPMArea(xsDigits[digit], yDigits, xdDigits[digit], ydDigits, xoff+extradx, 7+extrady);
	    	xoff += (xdDigits[digit]+1);

            	/* 
            	 *  Draw Seconds
            	 */
	    	digit = D[6];
	    	copyXPMArea(xsDigits[digit], yDigits, xdDigits[digit], ydDigits, xoff+extradx, 7+extrady);
	    	xoff += (xdDigits[digit]+1);

	    	digit = D[7];
	    	copyXPMArea(xsDigits[digit], yDigits, xdDigits[digit], ydDigits, xoff+extradx, 7+extrady);
	    	xoff += (xdDigits[digit]+3);

	    }


	    /*
	     *  Draw AM/PM indicator if we are using 12 Hour Clock.
	     *  Dont show it if we are using 24 Hour Clock.
	     */
	    if (!Show24HourTime)
	        copyXPMArea(xsAMPM[Flag], yAMPM, xdAMPM[Flag], ydAMPM, 54, 5);





	    /*
	     *  Beep on the hour
	     */
	    if (Beep){
		if ((Mins == 0)&&(Secs == 0)&&(OldSecs != Secs)) XBell(display, Volume);
		OldSecs = Secs;
		
	    }






	    if (OldDayOfMonth != DayOfMonth){


	        /*
	         *  Blank the Calendar section....
	         */
	        copyXPMArea(5, 70, 54, 35, 5, 24);


                /* 
                 *  Draw Day of Week and Month
                 */
		wid = xdDayOfWeek[DayOfWeek] + xdMonth[Month] + 1;
	        copyXPMArea(xsDayOfWeek[DayOfWeek], yDayOfWeek, xdDayOfWeek[DayOfWeek], 
						ydMonth, 33-wid/2, 64-24-4-12);
	        copyXPMArea(xsMonth[Month], yMonth, xdMonth[Month], 
					ydMonth, 33-wid/2+xdDayOfWeek[DayOfWeek]+1, 64-24-4-12);



	        /* 
	         *  Draw Day of Month
	         */
	        copyXPMArea(xsDayOfMonth[DayOfMonth], yDayOfMonth[DayOfMonth], xdDayOfMonth[DayOfMonth], ydDayOfMonth, 32-xdDayOfMonth2[DayOfMonth]/2, 36);


	    }


	    OldDayOfMonth = DayOfMonth;




	} else {

	    /*
	     *  Update the counter. 
	     */
	    ++n;

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
                case ButtonRelease:
                        break;
            }
        }





	
	/* 
	 *  Redraw and wait for next update 
	 */
	RedrawWindow();
	if( HasExecute == 1) {
	  usleep(DELAY);
	} else if( ShowSeconds == 1) {
	  usleep( 200000L);
	} else {
	  usleep( 500000L);
	}


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

        } else if (!strcmp(argv[i], "-jazz")){

	    UseJazzPoster = 1;

        } else if (!strcmp(argv[i], "-arial")){

	    UseArial = 1;

        } else if (!strcmp(argv[i], "-tekton")){

	    UseTekton = 1;

        } else if (!strcmp(argv[i], "-luggerbug")){

	    UseLuggerbug = 1;

        } else if (!strcmp(argv[i], "-comicsans")){

	    UseComicSans = 1;

        } else if (!strcmp(argv[i], "-tc")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmCalClock: No color found\n");
                print_usage();
                exit(-1);
            }
            strcpy(TimeColor, argv[++i]);

        } else if (!strcmp(argv[i], "-bc")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmCalClock: No color found\n");
                print_usage();
                exit(-1);
            }
            strcpy(BackgroundColor, argv[++i]);

        } else if (!strcmp(argv[i], "-24")){

	    Show24HourTime = 1;

        } else if (!strcmp(argv[i], "-b")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmCalClock: No volume given\n");
                print_usage();
                exit(-1);
            }
	    Beep = 1;
	    Volume = atoi(argv[++i]);

        } else if (!strcmp(argv[i], "-e")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmCalClock: No command given\n");
                print_usage();
                exit(-1);
            }
	    strcpy(ExecuteCommand, argv[++i]);
	    HasExecute = 1;

        } else if (!strcmp(argv[i], "-g")){

	    ShowGreenwichTime = 1;

        } else if (!strcmp(argv[i], "-S")){

	    ShowSeconds = 0;

        } else if (!strcmp(argv[i], "-s")){

	    ShowSiderealTime = 1;
	    Longitude = 0.0;

        } else if (!strcmp(argv[i], "-L")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmCalClock: No longitude given\n");
                print_usage();
                exit(-1);
            }
	    ShowSiderealTime = 1;
	    Longitude = atof(argv[++i]);

        } else if (!strcmp(argv[i], "-l")){

	    UseLowColorPixmap = 1;

        } else {

	    print_usage();
            exit(1);
	}

    }
    
    if (!ShowSeconds && !UseArial && !UseJazzPoster 
	&& !UseComicSans && !UseLuggerbug) UseTekton = 1;



}


void print_usage(){

    printf("\nwmCalClock version: %s\n", WMCALCLOCK_VERSION);
    printf("\nusage: wmCalClock [-b <Volume>] [-tc <Color>] [-bc <Color>] [-e \"Command\"] [-S]\n");
    printf("         [-24] [-g] [-s] [-l <longitude>] [-l] [-jazz] [-tekton] [-luggerbug]\n");
    printf("         [-arial] [-comicsans] [-h]\n\n");
    printf("\t-b <Volume>\tBeep on the hour. Volume is between -100 to 100.\n");
    printf("\t-tekton\t\tUse the Tekton font for time field.\n");
    printf("\t-arial\t\tUse the Arial-Narrow (i.e. Helvetica-Narrow) font for time field.\n");
    printf("\t-jazz\t\tUse the JazzPoster font for time field.\n");
    printf("\t-luggerbug\tUse the Luggerbug font for time field.\n");
    printf("\t-comicsans\tUse the ComicSans font for time field.\n");
    printf("\t-tc <Color>\tColor of the time digits (e.g. red or #ff8800).\n");
    printf("\t-bc <Color>\tBackground color.\n");
    printf("\t-e \"Command\"\tCommand to execute via double click of mouse button 1.\n");
    printf("\t-S\t\tDo not show seconds.\n");
    printf("\t-24\t\tShow 24-hour time. Default is 12 hour AM/PM Time.\n");
    printf("\t-g\t\tShow Greenwich time.\n");
    printf("\t-s\t\tShow Greenwich Mean Sidereal Time (GMST) in 24-hour format. \n");
    printf("\t-L <Longitude>\tShow Local Sidereal Time (LST) in 24-hour format. \n");
    printf("\t              \t\tLongitude is in degrees (- for West + for East).\n");
    printf("\t-l\t\tUse a low-color pixmap to conserve colors. On 8-bit displays the\n");
    printf("\t  \t\tlow color pixmap will always be used.\n");
    printf("\t-h\t\tDisplay help screen.\n");
    printf("\nExample: wmCalClock -b 100 -tc #001100 -bc #7e9e69 \n\n");

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







/*
 *  This routine handles button presses.
 *
 *   Double click on
 *              Mouse Button 1: Execute the command defined in the -e command-line option.
 *              Mouse Button 2: No action assigned.
 *              Mouse Button 3: No action assigned.
 *
 *
 */
void ButtonPressEvent(XButtonEvent *xev){

    char Command[512];


    if( HasExecute == 0) return; /* no command specified.  Ignore clicks. */
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
        sprintf(Command, "%s &", ExecuteCommand);
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
    }



   return;

}

