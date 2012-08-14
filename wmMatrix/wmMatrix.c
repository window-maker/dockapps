/*
 *
 *  	wmMatrix-0.2 (C) 1999 Mike Henderson (mghenderson@lanl.gov)
 *
 *  		- A DockApp version of Jamie Zawinski's xmatrix screensaver hack.
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
 * 	Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *      Boston, MA 02110-1301 USA
 *
 *
 *      Changes:
 *
 *	Version 0.2  -	released Aug 16, 1999.
 *
 *
 *      ToDo:
 *		Tie speed to cpu load or some such thing. (Idea from Ken Steen.) Still need to work
 *		on how best to accomplish this....
 *
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
#include "xutils.h"
#include "wmMatrix_master.xpm"
#include "wmMatrix_mask.xbm"
#include "matrix.h"


/*
 *  Delay between refreshes (in microseconds)
 */
#define DELAY 10000UL
#define WMMATRIX_VERSION "0.2"


void 	 ParseCMDLine(int argc, char *argv[]);
void 	 ButtonPressEvent(XButtonEvent *);
void 	 print_usage();
m_state *init_matrix( Display *, Window );
void 	 draw_matrix( m_state *, int );


int     	 GotFirstClick1, GotDoubleClick1;
int     	 GotFirstClick2, GotDoubleClick2;
int     	 GotFirstClick3, GotDoubleClick3;
int     	 DblClkDelay;
/*int		 HasExecute;*/
char*    	 ExecuteCommand = "xmatrixsmall";
char		*progname  = "wmMatrix";
char		*progclass = "WMMatrix";
int		 PixmapSize;
char *DoubleClickCmd = NULL;
char*   TimeColor    	= "#ffff00";
char*   BackgroundColor	= "#181818";


/*
 *   main
 */
int main(int argc, char *argv[]) {
    XEvent		 event;
    int			 n, k, m;
/*    float		 avg1;*/
    /*char 		 Command[512];*/
    m_state     	*state;
/*    FILE		*fp;*/


    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);
    if(DoubleClickCmd==NULL)
	DoubleClickCmd=strdup("xscreensaver-demo");
    /*HasExecute = 1;*/
    initXwindow(argc, argv);
    openXwindow(argc, argv, wmMatrix_master, wmMatrix_mask_bits, wmMatrix_mask_width, wmMatrix_mask_height);
    state = init_matrix( display, iconwin );

/*
    if (HasExecute){
        sprintf(Command, "%s -window-id 0x%x &", ExecuteCommand, (int)iconwin);
        system(Command);
    }
*/

    /*
     *  Loop until we die
     */
    n = k = m = 32000;
    while(1) {
#if 0
	if ( n>10 ){
	    n = 0;
	    if ( (fp = fopen("/proc/loadavg", "r")) != NULL ){
	        fscanf(fp, "%f", &avg1); avg1 *= 10.0; fclose(fp);
		m = (int)(40.0 - 1.00*avg1 + 0.5);
		if (m < 0) m = 0;
	    } else {
		printf("problem opening /proc/loadavg file for read\n");
		exit(-1);
	    }
	} else {
	    /*
	     *  Update the counter. 
	     */
	    ++n;
	}
#endif
	m=0;
	if (k > m){
	    k = 0;
	    draw_matrix( state, 40 );
	} else {
	    ++k;
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
	 *  sleep till next update. I cant seem to get usleep or select to work properly
	 *  with args smaller than 10000. A kernel tick problem? If I comment out the next line,
	 *  the app screams (chews up cpu too). Or if I use DELAY of 0 it also screams.
	 *  But a delay of 1 or higher is slow.....
	 *
	 */
	short_uusleep(DELAY);
     }
}


/*
 *   ParseCMDLine()
 */
void ParseCMDLine(int argc, char *argv[]) {
    int  i;
    PixmapSize = 2;
    for (i = 1; i < argc; i++) {
	if (!strcmp(argv[i], "-display")){
            ++i;
	} else if (!strcmp(argv[i], "-c")){
            if ((i+1 >= argc)||(argv[i+1][0] == '-')) {
                fprintf(stderr, "wmMatrix: No command given\n");
                print_usage();
                exit(-1);
            }
	    if(DoubleClickCmd!=NULL)
	      free(DoubleClickCmd);
	    DoubleClickCmd=strdup(argv[++i]);
        } else if (!strcmp(argv[i], "-sml")){
	    PixmapSize  = 1;
        } else if (!strcmp(argv[i], "-med")){
	    PixmapSize  = 2;
        } else if (!strcmp(argv[i], "-lrg")){
	    PixmapSize  = 3;
        } else {
	    print_usage();
            exit(1);
	}
    }
}


void print_usage() {
    printf("\nwmMatrix version: %s\n", WMMATRIX_VERSION);
    printf("\t-h\t\tDisplay help screen.\n");
    printf("\t-sml\t\tUse small size pixmap.\n");
    printf("\t-med\t\tUse medium size pixmap.\n");
    printf("\t-lrg\t\tUse large size pixmap.\n");
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
        system(DoubleClickCmd);
    }


    /*
     *  We got a double click on Mouse Button2 (i.e. the middle one)
     */
    if (GotDoubleClick2) {
        GotFirstClick2 = 0;
        GotDoubleClick2 = 0;
    }


    /*
     *  We got a double click on Mouse Button3 (i.e. the right one)
     */
    if (GotDoubleClick3) {
        GotFirstClick3 = 0;
	GotDoubleClick3 = 0;
    }
   return;
}
