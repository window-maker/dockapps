/*
 *
 *  	wmxss-0.10 (C) 1999 Mike Henderson (mghenderson@lanl.gov)
 *
 *  		- Its a DockApp front end for xscreensaver
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
 *	Version 0.10  -	released Aug 11, 1999.
 *			just playing around right now....
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
#include "wmxss_master.xpm"
#include "wmxss_mask.xbm"



/*
 *  Delay between refreshes (in microseconds)
 */
#define DELAY 10000L
#define WMXSS_VERSION "0.10"





void ParseCMDLine(int argc, char *argv[]);
void ButtonPressEvent(XButtonEvent *);
void print_usage();


int     GotFirstClick1, GotDoubleClick1;
int     GotFirstClick2, GotDoubleClick2;
int     GotFirstClick3, GotDoubleClick3;
int     DblClkDelay;
int	HasExecute;
char    ExecuteCommand[1024];





char    TimeColor[30]    	= "#ffff00";
char    BackgroundColor[30]    	= "#181818";






/*
 *   main
 */
int main(int argc, char *argv[]) {


    XEvent		event;
    int			n;
    char 		Command[512];


    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);







    initXwindow(argc, argv);
    openXwindow(argc, argv, wmxss_master, wmxss_mask_bits, wmxss_mask_width, wmxss_mask_height);



    if (HasExecute){
        sprintf(Command, "%s -window-id 0x%x &", ExecuteCommand, (int)iconwin);
        system(Command);
    }



    /*
     *  Loop until we die
     */
    n = 32000;
    while(1) {


	if ( n>10){

	    n = 0;

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
        if (DblClkDelay > 150) {

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
/*
	RedrawWindow();
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

        } else if (!strcmp(argv[i], "-tc")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmxss: No color found\n");
                print_usage();
                exit(-1);
            }
            strcpy(TimeColor, argv[++i]);

        } else if (!strcmp(argv[i], "-bc")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmxss: No color found\n");
                print_usage();
                exit(-1);
            }
            strcpy(BackgroundColor, argv[++i]);

        } else if (!strcmp(argv[i], "-e")){

            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmxss: No command given\n");
                print_usage();
                exit(-1);
            }
	    strcpy(ExecuteCommand, argv[++i]);
	    HasExecute = 1;

        } else {

	    print_usage();
            exit(1);
	}

    }


}


void print_usage(){

    printf("\nwmxss version: %s\n", WMXSS_VERSION);
    printf("\nusage: wmxss [-e \"Command\"] \n\n");
    printf("\t-e \"Command\"\tCommand to execute via double click of mouse button 1.\n");
    printf("\t-h\t\tDisplay help screen.\n");
    printf("\nExample: wmxss -e xflame\n\n");

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
        system("xscreensaver-demo");
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

