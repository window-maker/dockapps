/*
 *
 *  	wmGrabImage-1.00 (C) 1999 Mike Henderson (mghenderson@lanl.gov)
 * 
 *  		- Monitors an image on the WWW in a WindowMaker DockApp
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
 *	Platforms tested on: Linux, Solaris 2.6, IRIX 6.5
 *
 *
 *	ToDo:	Add inicator to show if image is uptodate or not
 *		Get rid of GrabImage and put all that stuff in wmGrabImage.c
 *		Center image in icon - this info should be in the returned Attributes struct.
 *		etc...
 *
 *		I am still not sure we are handling color resources properly... Seems to work OK on my 24-bit displays...
 *
 *
 *
 *
 *      Changes:
 *	Version 0.72  - May 27, 2001
 *			Fixed resource leak (XFreeColors)
 *
 *	Version 0.71  - May 24, 2001
 *			Added support for "zooming" the icon display 
 *			on a region 
 *
 *	Version 0.70  - March 28, 1999.
 *			Added support for local images (e.g. file:///home/mgh/junk.gif.)
 *
 *	Version 0.66  - February 23, 1999.
 *			Added help line for -delay option. And added a man page.
 *
 *	Version 0.65  - February 11, 1999.
 *                      Now starts netscape if not already running...
 *
 *	Version 0.64  - February 9, 1999.
 *                      Added command-line option for "Time between updates"
 *
 *	Version 0.63  - February 9, 1999.
 *			Fixed (potential) memory leak in the ShapeMask Pixmap 
 *                      (gets returned only if color None is used in xpm file)
 *			Added XFreeColors call.
 *
 *	Version 0.62  - February 4, 1999.
 *			buf fixes. Added -c option to center vertically.
 *
 *
 *	Version 0.61  - February 2, 1999.
 *			Added Attributes support for Xpm Creation.
 *        		Also, added a kludge to GrabImage to stream edit
 * 			any "Transparent" colors that may get generated. (Change them
 *			to black.)
 *
 *	Version 0.6   - February 2, 1999.
 * 			Some bug fixes. Added Double click support.
 *			Now, DoubleClick's do the following:
 *
 *				Button1: display original size image via xv.
 *
 *				Button2: send another URL (specified with the -http
 *			        	 command line option) to netscape.
 * 
 *				Button3: Force Update.
 *
 *
 *	Version 0.5   - initial (very) beta release February 1, 1999.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../wmgeneral/wmgeneral.h"
#include "wmGrabImage_master.xpm"
#include "wmGrabImage_mask.xbm"



/* 
 *  Delay between refreshes (in microseconds) 
 */
#define DELAY 10000L
#define WMGRABIMAGE_VERSION "0.72"





void ParseCMDLine(int argc, char *argv[]);
void pressEvent(XButtonEvent *xev);
void print_usage();


char 		ImageURL[1024], XpmFileName[256], ImageFileName[1024];
char		HttpURL[1024];
char		*ConvertGeometry= NULL;
int		UpToDate = 0;
int		ForceUpdate = 1;
int		ForceUpdate2 = 1;
int		GotFirstClick1, GotDoubleClick1;
int		GotFirstClick2, GotDoubleClick2;
int		GotFirstClick3, GotDoubleClick3;
int		DblClkDelay;
int		CenterImage = 0;
long int	UpdateDELAY = 600;





/*  
 *   main  
 */
int main(int argc, char *argv[]) {

struct tm	*gTime, *gmt;
struct stat	fi;
XEvent		event;
Pixmap		NewPixmap, NewShapeMask;
XpmAttributes	Attributes;
Colormap	cmap;
int		n, s, m, dt1, dt2, dt3, len;
int 		Year, Month, Day;
int		Hours, Mins, Secs;
int		i, j, Width, Height, yoff, fd, Flag;
long		CurrentLocalTime;
double		UT, hour24(), jd(), CurrentJD, OldFileUT, FileUT;
char		command[1040], ImageName[256];
int           havePixmap= 0;









	  
    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);
	   

    /*
     *  Figure out what the name of the image xpm file should be...
     */
    len = strlen(ImageURL);
    for (j = 0, i=0; i<len; ++i){ if (ImageURL[i] == '/') j = i; }
    strcpy(ImageName, ImageURL+j+1);
    sprintf(XpmFileName, "%s/.wmGrabImage/%s.xpm", getenv("HOME"), ImageName);
    sprintf(ImageFileName, "%s/.wmGrabImage/%s", getenv("HOME"), ImageName);

	   
    openXwindow(argc, argv, wmGrabImage_master, wmGrabImage_mask_bits, wmGrabImage_mask_width, wmGrabImage_mask_height);

    cmap = DefaultColormap(display, DefaultScreen(display));


	   
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
    FileUT = -999.0;
    Flag = 1;
    NewShapeMask = 0;
    Attributes.nalloc_pixels = 0;
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
                        pressEvent(&event.xbutton);
                        break;
                case ButtonRelease:
                        break;
            }
        }









	


	/*
	 *  Draw window.
	 */
	if (ForceUpdate||Flag){
	    


            /*
             *  Compute Current Julian Date
             */
            CurrentLocalTime = time(CurrentTime);
            gTime = gmtime(&CurrentLocalTime);
            Year  = gTime->tm_year+1900;
            Month = gTime->tm_mon+1;
            Day   = gTime->tm_mday;
            Hours = gTime->tm_hour;
            Mins  = gTime->tm_min;
            Secs  = gTime->tm_sec;
            UT = (double)Hours + (double)Mins/60.0 + (double)Secs/3600.0;
            CurrentJD = jd(Year, Month, Day, UT);


	    /*
	     * Clear window.
	     */
	    copyXPMArea(5, 69, 54, 54, 5, 5);



	    if (havePixmap) {
	      /* 
	       * free up the colors, if we alloc'd some before 
	       */
	      if (Attributes.nalloc_pixels > 0) 
		XFreeColors(display, cmap,  Attributes.alloc_pixels, 
			    Attributes.nalloc_pixels, 0);
		/*
		 *  Free last pixmap -- we dont need it anymore...
		 *  A ShapeMask is returned if the Pixmap had the color None used.
		 *  We could probably change Transparent to None to make use of this, but for now,
		 *  lets just ignore it...
		 */
		if ( NewShapeMask != 0 ) 
		  XFreePixmap(display, NewShapeMask);
		XFreePixmap(display, NewPixmap);

		XpmFreeAttributes(&Attributes);

		havePixmap= 0;
	    }
	    /*
	     *   Grab new pixmap. Accept a reasonable color match.
	     */
	    Attributes.valuemask   = XpmExactColors | XpmCloseness | XpmReturnAllocPixels;
	    Attributes.exactColors = 0;
	    Attributes.closeness   = 40000;
	    if (XpmReadFileToPixmap(display, Root, XpmFileName, &NewPixmap, &NewShapeMask, &Attributes) >= 0){



		Height = Attributes.height;
		Width  = Attributes.width;
		yoff   = (CenterImage) ? (54 - Height)/2 : 0;
	        XCopyArea(display, NewPixmap, wmgen.pixmap, NormalGC, 0, 0, Width, Height, 5, 5+yoff);


		Flag = 0;
		ForceUpdate = 0;
		havePixmap= 1;
	    }






	    /*
	     * Make changes visible
	     */
	    RedrawWindow();



	}





	/*
	 *  Check xpm file status
	 */
	if (dt2 > 1){

	    dt2 = 0;

	    if ( (fd = open(XpmFileName, O_RDONLY)) >= 0 ) {

		fstat(fd, &fi);
		close(fd);
		gmt = gmtime(&fi.st_mtime);
		OldFileUT = FileUT;
		FileUT = (double)gmt->tm_hour + (double)gmt->tm_min/60.0 + (double)gmt->tm_sec/3600.0;
		if (FileUT != OldFileUT) ForceUpdate = 1;

	    }


	}









	/*
	 *  Check every 5 min if the values are not up to date...
	 */

	if (ForceUpdate2||(dt3 > UpdateDELAY)){

	    dt3 = 0;

	    /*
	     *  Execute Perl script to grab the Latest METAR Report
	     */
	    if (ConvertGeometry != NULL)
	      sprintf(command, "GrabImage %s %s &", ImageURL, ConvertGeometry);
	    else
	      sprintf(command, "GrabImage %s &", ImageURL);
	    system(command);

	    ForceUpdate = 1;
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
 
    ImageURL[0] = '\0';
    for (i = 1; i < argc; i++) {

        if (!strcmp(argv[i], "-display")){

            ++i;

        } else if ((!strcmp(argv[i], "-url"))||(!strcmp(argv[i], "-u"))){

            strcpy(ImageURL, argv[++i]);

        } else if (!strcmp(argv[i], "-delay")){

            UpdateDELAY = atol(argv[++i]);
	    if (UpdateDELAY < 10){
		printf("Try a longer delay\n");
		exit(-1);
	    }

        } else if (!strcmp(argv[i], "-http")){

            strcpy(HttpURL, argv[++i]);

        } else if (!strcmp(argv[i], "-geom")){

            ConvertGeometry= argv[++i];

        } else if (!strcmp(argv[i], "-c")){

            CenterImage = 1;


        } else {
	    print_usage();
            exit(1);
        }
    }
    if (ImageURL[0] == '\0'){
	fprintf(stderr, "You must specify an Image URL\n");
	print_usage();
	exit(-1);
    }

}


void print_usage(){

    printf("\nwmGrabImage version: %s\n", WMGRABIMAGE_VERSION);
    printf("\nusage: wmGrabImage -u <ImageURL> -http <httpURL> -display <Display>] [-h]\n");
    printf("\t\t[-geom <CroppingGeometry>\n\n");
    printf("\t-display <Display>\tUse alternate X display.\n");
    printf("\t-h\t\t\tDisplay help screen.\n");
    printf("\t-url <ImageURL>\t\tThe URL of image to display.\n\n");
    printf("\t-delay <Time>\t\tSet time (in seconds) between updates.\n\n");
    printf("\t-http <httpURL>\t\tThe URL of the WWW page to send to\n\t\t\t\tnetscape with Button2 double click.\n\n");
    printf("\t-c          \tCenter the image vertically in the icon.\n\n");
    printf("\t-geom <CropGeometry>\tThe geometry to pass to convert\n");
    printf("\t\t\t\twhen scaling the image..\n\n");
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
 *   Double click on 
 *		Mouse Button 1: Display full size image with xv.
 *		Mouse Button 2: Send HttpURL to netscape.
 *		Mouse Button 3: Update image right away (i.e. grab a new one).
 *
 *
 */
void pressEvent(XButtonEvent *xev){

    char Command[512];

/*
   ForceUpdate = 1;
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
	sprintf(Command, "xv %s &", ImageFileName);
	system(Command);
    }


    /*
     *  We got a double click on Mouse Button2 (i.e. the left one)
     */
    if (GotDoubleClick2) {
	GotFirstClick2 = 0;
	GotDoubleClick2 = 0;
	sprintf(Command, "netscape -remote 'openURL(%s)' || netscape '%s' &", HttpURL, HttpURL);
	system(Command);
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

