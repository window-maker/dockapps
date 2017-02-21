
/***********************************************************************
 *   Code is stol^H^H^H^Hbased on wmppp, wmload, and wmtme
 *
 *   Author: Ben Cohen <buddog@aztec.asu.edu>
 *
 *   Contributors:
 *        Thomas Nemeth <tnemeth@multimania.com> -- Pushed button highlighting
 *      Craig Maloney <craig@ic.net> -- CTRL + ALT + key option
 *
 *
 *   This program is distributed under the GPL license.
 *
 *
 *   Best viewed with tab = 4 ( in vi set ts=4 )
 *
 ***********************************************************************/



#define VERSION "Ver 1.2.8 -- May 25, 1999"

#include <Xlib.h>
#include <Xutil.h>
#include <xpm.h>
#include <extensions/shape.h>
#include <keysym.h>
#include <stdio.h>
#include <math.h>

#include "backdrop.xpm"
#include "buttons.xpm"
#include "pushed_buttons.xpm"
#include "mask.xbm"

#define NO (0)
#define YES (1)

#define MAX_X_BUT 4
#define MAX_Y_BUT 4


/***********************************************************************
 *         Globals..    OK.. there's too many globals.. so sue me.
 ***********************************************************************/

Display * display;
int screen;
Window rootwin, win, iconwin;
GC gc;
int depth;
Pixel bg_pixel, fg_pixel;

XSizeHints xsizehints;
XWMHints * xwmhints;
XClassHint xclasshint;


typedef struct _XpmIcon {
    Pixmap pixmap;
    Pixmap mask;
    XpmAttributes attributes;
} XpmIcon;


Pixmap pixmask;

typedef struct _button_region {
    int x,y;
    int i,j;
} ButtonArea;

ButtonArea button_region[16];

XpmIcon template, visible, buttons, pbuttons;

unsigned int control = 0;

int numb_of_workspaces = 16;   /* Number of buttons to display.
                                  Initially set high. Changed by
                                  -n switch or button 2or3            */

int N = 1;                    /* Button number pressed to goto WS     */
int B = 1;                    /* Button number pressed to number WS's */
int alt_key_type = 1;
int border = 5;
int Verbose = 0;

char * app_name = "wmcp";



/***********************************************************************
 *         Function Prototypes
 ***********************************************************************/

void switchToWorkspaceN( int workspace );
void redraw( int xbut, int ybut );
void getPixmaps();
int whichButton( int x, int y, int xbut, int ybut );
int flush_expose(Window w);
void show_usage();






/***********************************************************************
 *         main
 ***********************************************************************/

int main( int argc, char ** argv )
{
    XEvent report;
    XGCValues xgcValues;

    XTextProperty app_name_atom;

    int dummy = 0;
    int i;
    int xbut = 2;
    int ybut = 2;

    int is_shaped = YES;
    int window_state = WithdrawnState;

    char Geometry_str[64] = "64x64+5+5";
    char Display_str[64] = "";


    /* Parse Command Line Arguments */

    for ( i=1; i<argc; i++ ) {
        if ( *argv[i] == '-' ) {
            switch ( *(argv[i]+1) ) {
                case 'v':
                    Verbose = 1;
                    break;
                case 'g':
                    if ( ++i >= argc ) show_usage();
                    sscanf(argv[i], "%s", Geometry_str);
                    if ( Verbose ) printf("Geometry is: %s\n", Geometry_str);
                    break;
                case 'd':
                    if ( ++i >= argc ) show_usage();
                    sscanf(argv[i], "%s", Display_str);
                    if ( Verbose ) printf("Display is: %s\n", Display_str);
                    break;
                case 'n':
                    if ( ++i >= argc ) show_usage();
                    sscanf(argv[i], "%d", &numb_of_workspaces);
                    if ( Verbose )
                        printf("Numb of Workspaces: %d\n", numb_of_workspaces);
                    break;
                case 's':
                    if ( ++i >= argc ) show_usage();
                    if ( *argv[i] == 'n' ) {
                        is_shaped = NO;
                    } else {
                        is_shaped = YES;
                    }
                    break;
                case 'w':
                    if ( ++i >= argc ) show_usage();
                    switch ( *argv[i] ) {
                        case 'i':
                            window_state = IconicState;
                            break;
                         case 'w':
                            window_state = WithdrawnState;
                            break;
                         case 'n':
                            window_state = NormalState;
                           break;
                    }
                    break;
                case 'x':
                    if ( ++i >= argc ) show_usage();
                    sscanf(argv[i], "%d", &xbut);
                    if ( xbut < 1 || xbut > MAX_X_BUT ) xbut = 3;
                    if ( Verbose ) printf("Num X buttons=%d\n", xbut);
                    break;
                case 'y':
                    if ( ++i >= argc ) show_usage();
                    sscanf(argv[i], "%d", &ybut);
                    if ( ybut < 1 || ybut > MAX_Y_BUT ) ybut = 3;
                    if ( Verbose ) printf("Num Y buttons=%d\n", ybut);
                    break;
                case 'a':
                    if ( ++i >= argc ) show_usage();
                    sscanf(argv[i], "%d", &alt_key_type);
                    if ( Verbose ) printf("Alt Key is: %d\n", alt_key_type);
                    break;
                case 'c':
                    control=ControlMask;
                    if ( Verbose ) printf ("Control Key Modifier added\n");
                    break;
                case 'h':
                    show_usage();
                    break;
            }
        }
    }







    if ( (display = XOpenDisplay(Display_str)) == NULL ) {
        fprintf(stderr,"Fail: XOpenDisplay for %s\n", Display_str);
        exit(-1);
    }


    screen = DefaultScreen(display);
    rootwin = RootWindow(display,screen);
    depth = DefaultDepth(display, screen);

    bg_pixel = WhitePixel(display, screen );
    fg_pixel = BlackPixel(display, screen );



    xsizehints.flags = USSize | USPosition;
    xsizehints.width = 64;
    xsizehints.height = 64;


    /* Parse Geometry string and fill in sizehints fields */

    XWMGeometry(display, screen,
                Geometry_str,
                NULL,
                border,
                &xsizehints,
                &xsizehints.x,
                &xsizehints.y,
                &xsizehints.width,
                &xsizehints.height,
                &dummy);


    if ( (win = XCreateSimpleWindow(
        display,
        rootwin,
        xsizehints.x,
        xsizehints.y,
        xsizehints.width,
        xsizehints.height,
        border,
        fg_pixel, bg_pixel) ) == 0 ) {
            fprintf(stderr,"Fail: XCreateSimpleWindow\n");
            exit(-1);
    }

    if ( (iconwin = XCreateSimpleWindow(
        display,
        win,
        xsizehints.x,
        xsizehints.y,
        xsizehints.width,
        xsizehints.height,
        border,
        fg_pixel, bg_pixel) ) == 0 ) {
            fprintf(stderr,"Fail: XCreateSimpleWindow\n");
            exit(-1);
    }



    /* Set up shaped windows                                    */
    /* Gives the appicon a border so you can grab and move it.  */

    if ( is_shaped == YES ) {
        if ( ( pixmask = XCreateBitmapFromData(display, win,
                            mask_bits, mask_width, mask_height) ) == 0 ) {
                fprintf(stderr,"Fail: XCreateBitmapFromData\n");
        }

        XShapeCombineMask(display,win,    ShapeBounding,0,0, pixmask,ShapeSet);
        XShapeCombineMask(display,iconwin,ShapeBounding,0,0, pixmask,ShapeSet);
    }




    /* Convert in pixmaps from .xpm includes. */
    getPixmaps();




    /* Interclient Communication stuff */
    /* Appicons don't work with out this stuff */

    xwmhints = XAllocWMHints();
    xwmhints->flags = WindowGroupHint | IconWindowHint | StateHint;
    xwmhints->icon_window = iconwin;
    xwmhints->window_group = win;
    xwmhints->initial_state = window_state;


    XSetWMHints( display, win, xwmhints );

    xclasshint.res_name = "wmcp";
    xclasshint.res_class = "WMcp";
    XSetClassHint( display, win, &xclasshint );

    XSetWMNormalHints( display, win, &xsizehints );





    /* Tell window manager what the title bar name is. We never see        */
    /* this anyways in the WithdrawnState                                 */

    if ( XStringListToTextProperty(&app_name, 1, &app_name_atom) == 0 ) {
        fprintf(stderr,"%s: Can't set up window name\n", app_name);
        exit(-1);
    }
    XSetWMName( display, win, &app_name_atom );



    /* Create Graphic Context */

    if ( (gc = XCreateGC(
        display,
        win,
        (GCForeground | GCBackground),
        &xgcValues)) == NULL ) {
            fprintf(stderr,"Fail: XCreateGC\n");
            exit(-1);
    }





    /* XEvent Masks. We want both window to process X events */

    XSelectInput(
        display,
        win,
        ExposureMask |
        ButtonPressMask |
        PointerMotionMask |
        StructureNotifyMask );

    XSelectInput(
        display,
        iconwin,
        ExposureMask |
        ButtonPressMask |
        PointerMotionMask |
        StructureNotifyMask );



    /* Store the 'state' of the application for restarting */

    XSetCommand( display, win, argv, argc );


    /* Window won't ever show up until it is mapped.. then drawn after a     */
    /* ConfigureNotify                                                         */

    XMapWindow( display, win );



    /* X Event Loop */

    while (1) {
        XNextEvent(display, &report );
        switch (report.type) {


            case Expose:
                if (report.xexpose.count != 0) {
                    break;
                }
                if ( Verbose ) fprintf(stdout,"Event: Expose\n");
                redraw( xbut, ybut );
                break;


            case ConfigureNotify:
                if ( Verbose ) fprintf(stdout,"Event: ConfigureNotify\n");
                redraw( xbut, ybut );
                break;


            case ButtonPress:
	            if ( Verbose )
                    printf ("numb_of_workspaces=%d\n", numb_of_workspaces);
                switch (report.xbutton.button) {
                    case Button1:
                        N = whichButton(report.xbutton.x, report.xbutton.y, xbut, ybut );
                        if ( N >= 0 && N <= numb_of_workspaces ) {
                            switchToWorkspaceN( N );
                            redraw(xbut, ybut);
                        }
                        if ( Verbose )
                            fprintf(stdout,"Button 1:x=%d y=%d N=%d\n",
                                report.xbutton.x, report.xbutton.y, N);
                        break;

                    case Button2:
                    case Button3:
                        B = whichButton(report.xbutton.x, report.xbutton.y, xbut, ybut );
                        if ( B >= 0 && B <= 9 ) {
                            numb_of_workspaces = B;
                            redraw(xbut, ybut);
                        }
                        if ( Verbose )
                            fprintf(stdout,"Button 2or3:x=%d y=%d B=%d\n",
                                report.xbutton.x, report.xbutton.y, B);
                        break;
                    }

                break;


            case DestroyNotify:
                if ( Verbose )
                    fprintf(stdout, "Bye\n");
                XFreeGC(display, gc);
                XDestroyWindow(display,win);
                XDestroyWindow(display,iconwin);
                XCloseDisplay(display);
                if ( Verbose )
                    fprintf(stdout, "Bye\n");
                exit(0);
                break;

        }
    }


return (0);
}



/***********************************************************************
 *   redraw
 *
 *     Map the button region coordinates.
 *
 *   Draw the appropriate number of buttons on the 'visible' Pixmap
 *   using data from the 'buttons' pixmap.
 *
 *   Then, copy the 'visible' pixmap to the two windows ( the withdrawn
 *   main window and the icon window which is the main window's icon image.)
 *
 ***********************************************************************/

void redraw( int xbut, int ybut )
{
    int n;
    int i,j;
    int dest_x, dest_y;  /* size of a whole button */
    int step_x, step_y;  /* size of half a button for corner copying */

    int offset = 8;      /* skip pixels past the window's border image */

    int xbut_size;
    int ybut_size;


	xbut_size = 48 / xbut;
	ybut_size = 48 / ybut;


    XCopyArea(     display,
            template.pixmap, visible.pixmap,
            gc,
            0, 0,
            template.attributes.width,
            template.attributes.height,
            0, 0 );


    for ( j=0; j < ybut; j++ ) {
        for ( i=0; i < xbut; i++ ) {
            n = i + j * xbut;

            dest_x = ( i * xbut_size ) + offset;
            dest_y = ( j * ybut_size ) + offset;
            step_x = 24 / xbut;
            step_y = 24 / ybut;

            /* Define button mouse coords */

            button_region[n].x = dest_x;
            button_region[n].y = dest_y;
            button_region[n].i = dest_x + xbut_size - 1;
            button_region[n].j = dest_y + ybut_size - 1;


            /* Copy button images for valid workspaces */

            if (  (n + 1) <= numb_of_workspaces ) {

                /* Draw normal button */
                /* Edited by Gert Beumer */
                if ( (n + 1) != N ) {

                    /* upper left */
                    XCopyArea(    display,
                            buttons.pixmap,
                            visible.pixmap,
                            gc,
                            0,0,
                            step_x, step_y,
                            dest_x,
                            dest_y);
                    /* lowwer left */
                    XCopyArea(    display,
                            buttons.pixmap,
                            visible.pixmap,
                            gc,
                            0,48 - step_y,
                            step_x, step_y,
                            dest_x,
                            dest_y + step_y);
                    /* lowwer right */
                    XCopyArea(    display,
                            buttons.pixmap,
                            visible.pixmap,
                            gc,
                            48 - step_x,48 - step_y,
                            step_x, step_y,
                            dest_x + step_x,
                            dest_y + step_y);
                    /* upper right */
                    XCopyArea(    display,
                            buttons.pixmap,
                            visible.pixmap,
                            gc,
                            48 - step_x,0,
                            step_x, step_y,
                            dest_x + step_x,
                            dest_y);

                    /* Draw the numbers */
                    XCopyArea(    display,
                            buttons.pixmap,
                            visible.pixmap,
                            gc,
                            n * 5,
                            48,
                            5,5,
                            dest_x + (48 - xbut * 5) / (2 * xbut),
                            dest_y + (48 - ybut * 5) / (2 * ybut));
                    }

                /* Draw pushed button */
                /* Added by Thomas Nemeth, Edited by Gert Beumer */
                if ( (n + 1) == N ) {
                    /* draw the four parts */

                    /* upper left */
                    XCopyArea(    display,
                            pbuttons.pixmap,
                            visible.pixmap,
                            gc,
                            0,0,
                            step_x, step_y,
                            dest_x,
                            dest_y);
                    /* lowwer left */
                    XCopyArea(    display,
                            pbuttons.pixmap,
                            visible.pixmap,
                            gc,
                            0,48 - step_y,
                            step_x, step_y,
                            dest_x,
                            dest_y + step_y);
                    /* lowwer right */
                    XCopyArea(    display,
                            pbuttons.pixmap,
                            visible.pixmap,
                            gc,
                            48 - step_x,48 - step_y,
                            step_x, step_y,
                            dest_x + step_x,
                            dest_y + step_y);
                    /* upper right */
                    XCopyArea(    display,
                            pbuttons.pixmap,
                            visible.pixmap,
                            gc,
                            48 - step_x,0,
                            step_x, step_y,
                            dest_x + step_x,
                            dest_y);

                    /* Draw the numbers */
                    XCopyArea(    display,
                            pbuttons.pixmap,
                            visible.pixmap,
                            gc,
                            n * 5,
                            48,
                            5,5,
                            dest_x + (48 - xbut * 5) / (2 * xbut),
                            dest_y + (48 - ybut * 5) / (2 * ybut));
                    }


            }
        }
    }

    flush_expose( win );
    XCopyArea(     display,
            visible.pixmap, win, gc,
            0, 0,
            visible.attributes.width,
            visible.attributes.height,
            0, 0 );

    flush_expose( iconwin );
    XCopyArea(     display,
            visible.pixmap, iconwin, gc,
            0, 0,
            visible.attributes.width,
            visible.attributes.height,
            0, 0 );


    if ( Verbose )
            fprintf(stdout,"In Redraw()\n");
}





/***********************************************************************
 *  whichButton
 *
 *  Return the button that at the x,y coordinates. The button need not
 *  be visible ( drawn ). Return -1 if no button match.
 *
 ***********************************************************************/

int whichButton( int x, int y, int xbut, int ybut )
{
    int index;

    for ( index=0; index< xbut*ybut; index++ ) {
        if ( x >= button_region[index].x &&
             x <= button_region[index].i &&
             y >= button_region[index].y &&
             y <= button_region[index].j  ) {
            return( index + 1 );
        }
    }
    return(-1);
}





/***********************************************************************
 * switchToWorkspaceN()
 *
 * Send the Synthetic Key Press event with the appropriate
 * [ meta key + 1-4 key ] combo.  Alt seems to usualy be Mod1Mask.
 *
 ***********************************************************************/

void switchToWorkspaceN( int workspace ) {

    XEvent sendEvent;

    sendEvent.xkey.type = KeyPress;
    sendEvent.xkey.window = rootwin;
    sendEvent.xkey.root = rootwin;
    sendEvent.xkey.subwindow = 0x0;
    switch ( alt_key_type ) {
        case 1:
            sendEvent.xkey.state = Mod1Mask+control;
            break;
        case 2:
            sendEvent.xkey.state = Mod2Mask+control;
            break;
        case 3:
            sendEvent.xkey.state = Mod3Mask+control;
            break;
        case 4:
            sendEvent.xkey.state = Mod4Mask+control;
            break;
    }
    sendEvent.xkey.keycode = XKeysymToKeycode(display, 0x30 + workspace );
    sendEvent.xkey.same_screen = True;
    sendEvent.xkey.display = display;
    sendEvent.xkey.send_event = False;

    XSendEvent( display, rootwin, True, KeyPressMask, &sendEvent );
}


/***********************************************************************
 *   getPixmaps
 *
 *   Load XPM data into X Pixmaps.
 *
 *   Pixmap template contains the untouched window backdrop image.
 *   Pixmap visible is the template pixmap with buttons drawn on it.
 *          -- what is seen by the user.
 *   Pixmap buttons holds the images for individual buttons that are
 *          later copied onto Pixmap visible.
 ***********************************************************************/
void getPixmaps()
{

    template.attributes.valuemask |=
            (XpmReturnPixels | XpmReturnExtensions);
    visible.attributes.valuemask |=
            (XpmReturnPixels | XpmReturnExtensions);
    buttons.attributes.valuemask |=
            (XpmReturnPixels | XpmReturnExtensions);

    /* Template Pixmap. Never Drawn To. */
        if ( XpmCreatePixmapFromData(    display,
                    rootwin,
                    backdrop_xpm,
                    &template.pixmap,
                       &template.mask,
                    &template.attributes) != XpmSuccess ) {
        fprintf(stderr, "Can't Create 'template' Pixmap");
        exit(1);
    }

    /* Visible Pixmap. Copied from template Pixmap and then drawn to. */
        if ( XpmCreatePixmapFromData(    display,
                    rootwin,
                    backdrop_xpm,
                    &visible.pixmap,
                    &visible.mask,
                    &visible.attributes) != XpmSuccess ) {
        fprintf(stderr, "Can't Create 'visible' Pixmap");
        exit(1);
    }

    /* Buttons Pixmap.  */
        if ( XpmCreatePixmapFromData(    display,
                    rootwin,
                    buttons_xpm,
                    &buttons.pixmap,
                    &buttons.mask,
                    &buttons.attributes) != XpmSuccess ) {
        fprintf(stderr, "Can't Create 'buttons' Pixmap");
        exit(1);
    }

    /* Pushed Buttons Pixmap.  */
        if ( XpmCreatePixmapFromData(    display,
                    rootwin,
                    pushed_buttons_xpm,
                    &pbuttons.pixmap,
                    &pbuttons.mask,
                    &pbuttons.attributes) != XpmSuccess ) {
        fprintf(stderr, "Can't Create 'pbuttons' Pixmap");
        exit(1);
    }
}





/***********************************************************************
 *   flush_expose
 *
 *   Everyone else has one of these... Can't hurt to throw it in.
 *
 ***********************************************************************/
int flush_expose(Window w) {

    XEvent      dummy;
    int         i=0;

    while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
        i++;

    return i;
}


/***********************************************************************
 *         show_usage
 *
 ***********************************************************************/

void show_usage()
{

fprintf(stderr,"

%s
This software is GPL -- do as you wish with it.

Origional Author:
   Ben Cohen <buddog@aztec.asu.edu>

Contributors:
   Thomas Nemeth <tnemeth@multimania.com>
   Craig Maloney <craig@ic.net>
   Gert Beumer <Gert@scintilla.utwente.nl>



usage: wmcp [-g geometry] [-d dpy] [-n workspaces] [-a alt key] [-v]
            [-c] [-w i/n/w] [-s y/n] [-x #] [-y #] [-h]


-g    geometry:    ie: 64x64+10+10
-d    dpy:         Display. ie: 127.0.0.1:0.0
-n    workspaces:  How many buttons to start with.
-a    alt key:     integer 1-4 defining ModXMask (default 1 Mod1Mask).
-w    i/n/w:       Window State: Iconic, Normal, Withdrawn (default Withdrawn)
-s    y/n:         Shaped window: yes or no (default y)
-c                 Sends CTRL + ALT + Key (default only sends ALT + key)
-v                 Verbose. 0=off, 1=on (default 0)
-x                 Number of buttons on the x-direction (1,2,3, or 4)
-y                 Number of buttons in the y-direction (1,2,3, or 4)
-h                 Help. This screen.


",VERSION);

exit(-1);
}


/*
KeyPress event, serial 13, synthetic NO, window 0x25,
    root 0x25, subw 0x0, time 3340683384, (37,254), root:(37,254),
    state 0x8, keycode 10 (keysym 0x31, 1), same_screen YES,
    XLookupString gives 1 characters:  "1"

KeyPress event, serial 13, synthetic YES, window 0xbffff9ac,
    root 0x40009b48, subw 0x4000a670, time 2, (26460,-1352), root:(-1146,7),
    state 0x7de0, keycode 64 (keysym 0xffe9, Alt_L), same_screen YES,
    XLookupString gives 0 characters:  ""
*/


