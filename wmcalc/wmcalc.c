/****************************************************************
 *  File:     wmcalc.c
 *  Version:  0.3
 *  Date:     January 17, 2001
 *  Author:   Edward H. Flora <ehflora@access1.net>
 *
 *  This file is a part of the wmcalc application.  As such, this
 *  file is licensed under the GNU General Public License, version 2.
 *  A copy of this license may be found in the file COPYING that should
 *  have been distributed with this file.  If not, please refer to
 *  http://www.gnu.org/copyleft/gpl.html for details.
 *
 ****************************************************************
    Description:
     This file contains the main program code for the wmcalc
     application.  Wmcalc is a dockapp designed for the WindowMaker or
     Afterstep window managers (although it should run well in most
     others.)  wmcalc is a four-function (and more) calculator that
     has a small enough footprint that you may leave it open on your
     desktop at all times, for convenient use.


    Change History:
     Date       Modification
     01/17/01   Updated to use XLookupString
     12/10/00   Revised includes, extracting X libs to wmcalc_x.h
     11/09/00   Added "locked" memory capabilities
     11/08/00   Added Code to Support Keyboard / focus
     10/29/00   Implemented memory use, configuration files, and a
                quickstart button for a larger calculator.  Also
		abstracted some of the macros, global vars, function
		prototypes, etc out to independent header files, to
		eliminate some dependency issues between source files.
     02/10/00   Added keyboard event code, without success
     12/21/99   Original product release, version 0.1
     11/26/99   Original file creation

 ****************************************************************/

#include "wmcalc_x.h"
#include <X11/XKBlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wmcalc_c.h"
#include "wmcalc_err.h"
#include "wmcalc_t.h"
#include "wmcalc_g.h"
#include "wmcalc_f.h"

#include "backdrop.xpm"           /* background graphic */
#include "calcbuttons.xpm"        /* graphic of buttons */
#include "charmap.xpm"            /* pixmap of characters */
#include "mask.xbm"

/* Global Variables */
/* Ok, so I didn't get them all extracted.  Should look into this
   further */
int N = 1;		       /* Button number pressed to goto app # */
int border  = 0;
int Verbose = 0;               /* Debug flag */
int mmouse  = 1;               /* flag to enable middle mouse (hold
				  over from wmbutton */
int button_pressed = -1;       /* button to be drawn pressed */
char PlusMinusFlag = '+';      /* flag for sign of number in display */
char OpFlag = ' ';             /* Operation requested */
int ExpFlag = 0;               /* Flag if in scientific notation */
int DecFlag = 0;               /* Flag if a decimal is in display */
int ImgFlag = 0;               /* Flag if a number is imaginary */
int StrCnt = 0;
double RegisterA = 0.0;        /* Main working register, displayed */
double RegisterB = 0.0;        /* Second register to add to, etc */
char DispString[DISPSIZE+1];   /* Pointer to string of display */
ButtonArea button_region[NUM_BUTTONS];  /* Array of buttons */

char *app_name = "wmcalc";     /* Name of app, for window management */

/****************************************************************
 *  Function:     main
 ****************************************************************
    Description:
      This is the main Program control function for wmcalc.  It
      contains all the X11 windows function calls, as well as other
      general operations.

    Change History:
    Date       Modification
    01/17/01   Updated to use XLookupString to get KeySym
    11/09/00   Added Events for focus and keyboard work.
    11/01/00   File Header Added
    21/09/01   Added global configfile by Gordon Fraser
 ****************************************************************/
int main( int argc, char **argv ) {
  XEvent report;
  XGCValues xgcValues;
  XTextProperty app_name_atom;
  int err_code = OKAY;
  int dummy = 0;
  int i;
  char Geometry_str[64] = "64x64+0+0";
  char Display_str[64] = "";
  int KeywithMask = NO_BUTTON;
  KeySym ksym;
  XComposeStatus compose;
  char buffer[20];
  int bufsize = 20;


  strcpy(configfile, getenv("HOME"));  /* Added to wmbutton by Casey Harkin, 3/6/99 */
  strcat(configfile, CONFFILENAME);    /* Fixed Bug - didn't look in home directory */
                                       /* but startup directory */
  strcat(tempfile, CONFTEMPFILE);      /* Setup name for temp file */

  /* Clear the Calculator Display */
  for(i=0; i<DISPSIZE; i++) DispString[i] = ' ';
  DispString[DISPSIZE] = '\0';

  /* Parse Command Line Arguments */
  for ( i=1; i < argc; i++ ) {
    if ( *argv[i] == '-' ) {
      switch ( *(argv[i]+1) ) {
      case 'v':                        /* Turn on Verbose (debugging) Mode */
	Verbose = 1;
	break;
      case 'g':                        /* Set Geometry Options */
	if ( ++i >= argc ) show_usage();
	sscanf(argv[i], "%s", Geometry_str);
	if ( Verbose ) printf("Geometry is: %s\n", Geometry_str);
	break;
      case 'd':                        /* Set display */
	if ( ++i >= argc ) show_usage();
	sscanf(argv[i], "%s", Display_str);
	if ( Verbose ) printf("Display is: %s\n", Display_str);
	break;
      case 'h':                        /* Show Help Message */
	show_usage();
	break;
      case 'f':                        /* use config file <filename> */
	if ( ++i >= argc ) show_usage();
	sscanf(argv[i], "%s", configfile);
	if ( Verbose ) printf("Using Config File: %s\n", configfile);
	break;
      default:                         /* other, unknown, parameters */
	show_usage();
	break;
      }
    }
  } /* End of loop to process command line options */

  /* Open display on requested X server */
  if ( (display = XOpenDisplay(Display_str)) == NULL ) {
    error_handler(ERR_X_DISPLAY, Display_str);
  }

  screen  = DefaultScreen(display);
  rootwin = RootWindow(display,screen);
  depth   = DefaultDepth(display, screen);

  bg_pixel = WhitePixel(display, screen );
  fg_pixel = BlackPixel(display, screen );

  xsizehints.flags  = USSize | USPosition;
  xsizehints.width  = APP_WIDTH;
  xsizehints.height = APP_HEIGHT;

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

  if ( (win = XCreateSimpleWindow(display,
				  rootwin,
				  xsizehints.x,
				  xsizehints.y,
				  xsizehints.width,
				  xsizehints.height,
				  border,
				  fg_pixel, bg_pixel) ) == 0 ) {
    error_handler(ERR_X_CREATE_WINDOW, NULL);
  }

  if ( (iconwin = XCreateSimpleWindow(display,
				      win,
				      xsizehints.x,
				      xsizehints.y,
				      xsizehints.width,
				      xsizehints.height,
				      border,
				      fg_pixel, bg_pixel) ) == 0 ) {
    error_handler(ERR_X_CREATE_WINDOW, NULL);
  }

  /* Set up shaped windows */
  /*Gives the appicon a border so you can grab and move it. */

  if ( ( pixmask = XCreateBitmapFromData(display,
					 win,
					 (char *)mask_bits,
					 mask_width,
					 mask_height) )  == 0 ) {
    error_handler(ERR_X_CREATE_BITMAP, NULL);
  }

  XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask, ShapeSet );
  XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);

  /* Convert in pixmaps from .xpm includes. */
  getPixmaps();

  /* Interclient Communication stuff */
  /* Appicons don't work with out this stuff */
  xwmhints = XAllocWMHints();
  xwmhints->flags = WindowGroupHint | IconWindowHint | StateHint;
  xwmhints->icon_window = iconwin;
  xwmhints->window_group = win;
  xwmhints->initial_state = WithdrawnState;
  XSetWMHints( display, win, xwmhints );

  xclasshint.res_name  = app_name;
  xclasshint.res_class = app_name;
  XSetClassHint( display, win, &xclasshint );

  XSetWMNormalHints( display, win, &xsizehints );

  /* Tell window manager what the title bar name is. We never see */
  /* this anyways in the WithdrawnState      */
  if ( XStringListToTextProperty(&app_name, 1, &app_name_atom) == 0 ) {
    error_handler(ERR_SETUP_WINDOW_NAME, app_name);
  }
  XSetWMName( display, win, &app_name_atom );

  /* Create Graphic Context */
  if (( gc = XCreateGC(display, win,(GCForeground | GCBackground), &xgcValues))
       == NULL ) {
    error_handler(ERR_CREATE_GC, NULL);
  }

  /* XEvent Masks. We want both windows to process X events */
  XSelectInput(display, win,
	       ExposureMask |
	       ButtonPressMask |
	       ButtonReleaseMask |	/* added ButtonReleaseMask *charkins*/
	       PointerMotionMask |
	       FocusChangeMask |
	       LeaveWindowMask |
	       KeyPressMask |           /* Try this to get keyboard working */
	       StructureNotifyMask |
	       EnterWindowMask );
  XSelectInput(display, iconwin,
	       ExposureMask |
	       ButtonPressMask |
	       ButtonReleaseMask |	/* added ButtonReleaseMask *charkins*/
	       PointerMotionMask |
	       FocusChangeMask |
	       LeaveWindowMask |
	       KeyPressMask |           /* Try this to get keyboard working */
	       StructureNotifyMask |
	       EnterWindowMask );

  /* Store the 'state' of the application for restarting */
  XSetCommand( display, win, argv, argc );

  /* Window won't ever show up until it is mapped.. then drawn after a 	*/
  /* ConfigureNotify */
  XMapWindow( display, win );

  /* Read Configuration File */
  err_code = read_config();
  if (err_code) {
    error_handler(err_code, configfile);
  }

  /* X Event Loop */
  while (1) {
    XNextEvent(display, &report );
    switch (report.type) {
    case Expose:
      if (report.xexpose.count != 0) {
	break;
      }
      if ( Verbose ) printf("Event: Expose\n");
      redraw();
      break;
    case ConfigureNotify:
      if ( Verbose ) printf("Event: ConfigureNotify\n");
      /*      redraw(); */
      break;

    case KeyPress:
      if (Verbose) printf("Event: Key state: 0x%x  Key: 0x%x\n",
			  report.xkey.state, report.xkey.keycode);

      /*      ksym = XLookupKeysym(&(report.xkey), report.xkey.state); */
      /* KeywithMask - this allows Left, middle, and right button functions
	 to be implemented via keyboard */
      XLookupString(&(report.xkey), buffer, bufsize, &ksym, &compose);
      if (Verbose) printf("Keysym is: 0x%x\n", (int) ksym);
      KeywithMask = whichKey(ksym);
      ExecFunc( KeywithMask );
      redraw();
      break;

    case ButtonPress:	/* draw button pressed, don't launch *charkins*/
      switch (report.xbutton.button) {
      case Button1:
	N = whichButton(report.xbutton.x, report.xbutton.y );
	if ( (N >= 0) && (N <= NUM_BUTTONS) ) {
	  button_pressed = N + LMASK;
	  /*	  redraw(); */
	}
	if ( Verbose )
	  printf("Button 1:x=%d y=%d N=%d\n",
		 report.xbutton.x, report.xbutton.y, N+LMASK);
	break;
      case Button2:
	if (mmouse) {
	  N = whichButton(report.xbutton.x, report.xbutton.y );
	  if ( (N >= 0) && (N <= NUM_BUTTONS) ) {
	    button_pressed = N + MMASK;
	    /*	    redraw(); */
	  }
	  if ( Verbose )
	    printf("Button 2:x=%d y=%d N=%d\n",
		   report.xbutton.x, report.xbutton.y, N+MMASK);
	}
	break;
      case Button3:
	N = whichButton(report.xbutton.x, report.xbutton.y );
	if ( (N >= 0) && (N <= NUM_BUTTONS) ) {
	  button_pressed = N + RMASK;
	  /*	  redraw(); */
	}
	if ( Verbose )
	  printf("Button 3:x=%d y=%d N=%d\n",
		 report.xbutton.x, report.xbutton.y, N+RMASK);
	break;
      }
      break;
    case ButtonRelease:	/* If still over button, it was a real button press */
      switch (report.xbutton.button) {
      case Button1:
	N = whichButton(report.xbutton.x, report.xbutton.y );
	if ( (N >= 0) && (N <= NUM_BUTTONS) && (N == button_pressed - LMASK))
	  ExecFunc(N + LMASK);
	button_pressed=-1;
	redraw();
	if ( Verbose )
	  printf("Button 1:x=%d y=%d N=%d\n",
		 report.xbutton.x, report.xbutton.y, N+LMASK);
	break;
      case Button2:
	if (mmouse) {
	  N = whichButton(report.xbutton.x, report.xbutton.y );
	  if ( (N >= 0) && (N <= NUM_BUTTONS) && (N == button_pressed - MMASK))
	    ExecFunc( N + MMASK);
	  button_pressed=-1;
       	  redraw();
	  if ( Verbose )
	    printf("Button 2:x=%d y=%d N=%d\n",
		   report.xbutton.x, report.xbutton.y, N+MMASK);
	}
	break;
      case Button3:
	N = whichButton(report.xbutton.x, report.xbutton.y );
	if ( (N >= 0) && (N <= NUM_BUTTONS) && (N == button_pressed - RMASK))
	  ExecFunc( N + RMASK);
	button_pressed=-1;
      	redraw();
	if ( Verbose )
	  printf("Button 3:x=%d y=%d N=%d\n",
		 report.xbutton.x, report.xbutton.y, N+RMASK);
	break;
      }
      break;
    case DestroyNotify:
      if ( Verbose ) printf("Requested Program Quit.\n");
      XFreeGC(display, gc);
      XDestroyWindow(display,win);
      XDestroyWindow(display,iconwin);
      XCloseDisplay(display);
      exit(OKAY);
      break;
    case EnterNotify:
    case LeaveNotify:
      XSetInputFocus(display, PointerRoot, RevertToParent, CurrentTime);
      if (Verbose) printf("Focus Change\n");
      break;
    }
  }
  return (OKAY);
} /***** End of main program ***********************************/

/****************************************************************
 *  Function:     redraw
 ****************************************************************
    Description:
     This function maintains the appearance of the application
     by copying the "visible" pixmap to the two windows (the withdrawn
     main window, and the icon window which is the main windows's icon
     image).

    Change History:
    Date       Modification
    11/1/00    Function Header updated
 ****************************************************************/
void redraw() {

  XCopyArea(display, template.pixmap, visible.pixmap, gc, 0, 0,
	    template.attributes.width, template.attributes.height, 0, 0 );

  defineButtonRegions();

  /* Copy button to icon */
  XCopyArea(display, buttons.pixmap, visible.pixmap, gc,
	    1, 1, 53, 40, 6, 20);

  flush_expose( win );
  XCopyArea(display, visible.pixmap, win, gc, 0, 0,
	    visible.attributes.width, visible.attributes.height, 0, 0 );
  flush_expose( iconwin );
  XCopyArea(display, visible.pixmap, iconwin, gc, 0, 0,
	    visible.attributes.width, visible.attributes.height, 0, 0 );
  /*  if ( Verbose ) printf("In Redraw()\n"); */
  displaystr();
}  /***** End of function redraw() ********************************/

/****************************************************************
 *  Function:     whichButton
 ****************************************************************
    Description:
     Return the button at the x,y coordinates. The button need not
     be visible ( drawn ). Return -1 if no button match.

    Change History:
    Date       Modification
    11/1/00    Function Header Updated
 ****************************************************************/
int whichButton( int x, int y ) {
  int index;

  for ( index=0; index < NUM_BUTTONS; index++ ) {
    if ( (x >= button_region[index].x) && (x <= button_region[index].i) &&
	 (y >= button_region[index].y) && (y <= button_region[index].j)    ) {
      return(index);
    }
  }
  return(NO_BUTTON);
}  /***** End of function whichButton() **************************/

/****************************************************************
 *  Function:     whichKey
 ****************************************************************
    Description:
      This function decodes the keycode passed in and converts this to
      a function number to execute.  This is not simply the Button number,
      but also contains the LMASK, MMASK, or RMASK to pass into ExecFunc to
      handle expanded functions.

    Change History:
    Date       Modification
    01/17/01   Updated to take a KeySym, rather than a KeyCode
    11/09/00   Original Function creation
 ****************************************************************/
int whichKey (KeySym keysym) {
  extern int      Verbose;
  int        func   = NO_BUTTON;

  if (Verbose) printf("KeySym 0x%x received, decoding...\n", (int) keysym);

  switch(keysym) {
  case XK_Escape:
  case XK_space:
  case XK_KP_Space:
    func = LMASK + 0;
    break;
  case XK_Delete:
    func = MMASK + 0;
    break;
  case XK_BackSpace:
    func = RMASK + 0;
    break;
  case XK_Return:
  case XK_KP_Equal:
  case XK_KP_Enter:
  case XK_equal:
    func = LMASK + 19;
    break;
  case XK_KP_0:
  case XK_0:
    func = LMASK + 17;
    break;
  case XK_KP_1:
  case XK_1:
    func = LMASK + 12;
    break;
  case XK_KP_2:
  case XK_2:
    func = LMASK + 13;
    break;
  case XK_KP_3:
  case XK_3:
    func = LMASK + 14;
    break;
  case XK_KP_4:
  case XK_4:
    func = LMASK + 7;
    break;
  case XK_KP_5:
  case XK_5:
    func = LMASK + 8;
    break;
  case XK_KP_6:
  case XK_6:
    func = LMASK + 9;
    break;
  case XK_KP_7:
  case XK_7:
    func = LMASK + 2;
    break;
  case XK_KP_8:
  case XK_8:
    func = LMASK + 3;
    break;
  case XK_KP_9:
  case XK_9:
    func = LMASK + 4;
    break;
  case XK_KP_Decimal:
  case XK_period:
    func = LMASK + 18;
    break;
  case XK_KP_Divide:
  case XK_slash:
    func = LMASK + 5;
    break;
  case XK_KP_Subtract:
  case XK_minus:
    func = LMASK + 15;
    break;
  case XK_KP_Multiply:
  case XK_asterisk:
    func = LMASK + 10;
    break;
  case XK_KP_Add:
  case XK_plus:
    func = LMASK + 20;
    break;
  case XK_R:
  case XK_r:
    func = LMASK + 1;
    break;
  case XK_asciicircum:
    func = LMASK + 6;
    break;
  case XK_C:
  case XK_c:
    func = LMASK + 11;
    break;
  case XK_N:
  case XK_n:
    func = LMASK + 16;
    break;
  case XK_F1:
    func = MMASK + 12;
    break;
  case XK_F2:
    func = MMASK + 13;
    break;
  case XK_F3:
    func = MMASK + 14;
    break;
  case XK_F4:
    func = MMASK + 7;
    break;
  case XK_F5:
    func = MMASK + 8;
    break;
  case XK_F6:
    func = MMASK + 9;
    break;
  case XK_F7:
    func = MMASK + 2;
    break;
  case XK_F8:
    func = MMASK + 3;
    break;
  case XK_F9:
    func = MMASK + 4;
    break;
  case XK_F10:
    func = MMASK + 17;
    break;
  default:
    if (Verbose) printf("Unknown Keysym, ignoring.\n");
    func = NO_BUTTON;
    break;
  }
  return(func);
}  /***** End of function whichKey() ****************************/

/****************************************************************
 *  Function:     getPixmaps
 ****************************************************************
    Description:
     Load XPM data into X Pixmaps.

     * Pixmap 'template' contains the untouched window backdrop image.
     * Pixmap 'visible' is the template pixmap with buttons drawn on it.
            -- what is seen by the user.
     * Pixmap 'buttons' holds the images for individual buttons that are
       later copied onto Pixmap visible.
     * Pixmap 'charmap' holds the character map for the characters in
       the display.

    Change History:
    Date       Modification
    11/1/00    Function Header Updated
 ****************************************************************/
void getPixmaps() {
  template.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);
  visible.attributes.valuemask  |= (XpmReturnPixels | XpmReturnExtensions);
  buttons.attributes.valuemask  |= (XpmReturnPixels | XpmReturnExtensions);
  charmap.attributes.valuemask  |= (XpmReturnPixels | XpmReturnExtensions);

  /* Template Pixmap. Never Drawn To. */
  if ( XpmCreatePixmapFromData(	display, rootwin, backdrop_xpm,
				&template.pixmap, &template.mask,
				&template.attributes) != XpmSuccess ) {
    error_handler(ERR_CREATE_PIXMAP, "template");
  }

  /* Visible Pixmap. Copied from template Pixmap and then drawn to. */
  if ( XpmCreatePixmapFromData(	display, rootwin, backdrop_xpm,
				&visible.pixmap, &visible.mask,
				&visible.attributes) != XpmSuccess ) {
    error_handler(ERR_CREATE_PIXMAP, "visible");
  }

  /* Button Pixmap.  */
  if ( XpmCreatePixmapFromData(	display, rootwin, calcbuttons_xpm,
				&buttons.pixmap, &buttons.mask,
				&buttons.attributes) != XpmSuccess ) {
    error_handler(ERR_CREATE_PIXMAP, "buttons");
  }

  /* Character Map Pixmap.  */
  if ( XpmCreatePixmapFromData(	display, rootwin, charmap_xpm,
				&charmap.pixmap, &charmap.mask,
				&charmap.attributes) != XpmSuccess ) {
    error_handler(ERR_CREATE_PIXMAP, "charmap");
  }
} /***** End of function getPixmaps() *****************************/

/****************************************************************
 *  Function:     flush_expose
 ****************************************************************
    Description:
      This function is a hold-over from previous programs (wmcp).
      The use of this function is not well understood.

    Change History:
    Date       Modification
    11/1/00    Function header updated.
 ****************************************************************/
int flush_expose(Window w) {
  XEvent      dummy;
  int         i=0;

  while (XCheckTypedWindowEvent(display, w, Expose, &dummy)) i++;
  return(i);
} /***** End of function flush_expose() *************************/

/****************************************************************
 *  Function:     defineButtonRegion
 ****************************************************************
    Description:
     This function defines the start and end x and y coordinates for
     the various buttons used in wmcalc.

     There should be a better way to do this, as right now, changing
     the pixmap calcbuttons.xpm may require the modification of these
     magic numbers.

    Change History:
    Date       Modification
    11/1/00    Function header updated
 ****************************************************************/
void defineButtonRegions(void) {
  int ndx = 0;   /* button index */

  button_region[0].x = 1;      /* Define display region button */
  button_region[0].i = 62;
  button_region[0].y = 6;
  button_region[0].j = 17;

  for (ndx =  1; ndx <= 5; ndx++) {  /* Define y coord's for top row */
    button_region[ndx].y = 20;
    button_region[ndx].j = 29;
  }
  for (ndx =  6; ndx <= 10; ndx++) {  /* Define y coord's for 2nd row */
    button_region[ndx].y = 30;
    button_region[ndx].j = 39;
  }
  for (ndx = 11; ndx <= 15; ndx++) {  /* Define y coord's for 3rd row */
    button_region[ndx].y = 40;
    button_region[ndx].j = 49;
  }
  for (ndx = 16; ndx <= 20; ndx++) {  /* Define y coord's for bottom row */
    button_region[ndx].y = 50;
    button_region[ndx].j = 59;
  }
  for (ndx = 1; ndx <= 16; ndx+=5) {  /* Define x coord's for Left column */
    button_region[ndx].x = 5;
    button_region[ndx].i = 16;
  }
  for (ndx = 2; ndx <= 17; ndx+=5) {  /* Define x coord's for 2nd Left column */
    button_region[ndx].x = 17;
    button_region[ndx].i = 26;
  }
  for (ndx = 3; ndx <= 18; ndx+=5) {  /* Define x coord's for middle column */
    button_region[ndx].x = 27;
    button_region[ndx].i = 36;
  }
  for (ndx = 4; ndx <= 19; ndx+=5) {  /* Define x coord's for 2nd right column */
    button_region[ndx].x = 37;
    button_region[ndx].i = 46;
  }
  for (ndx = 5; ndx <= 20; ndx+=5) {  /* Define x coord's for 2nd right column */
    button_region[ndx].x = 47;
    button_region[ndx].i = 57;
  }
}  /***** End of function defineButtonRgions() *************************/

/****************************************************************
 *  Function:     displaychar
 ****************************************************************
    Description:
      This function displays individual characters to the "display".
      This function should only be called from displaystr().

    Change History:
    Date       Modification
    11/09/00   Added "Locked" character and capabilities
    11/01/00   Function header updated
    10/30/00   Updated to include the memory indicators as well.
 ****************************************************************/
void displaychar(char ch, int location) {
  ButtonArea dispchar;
  int locatx, locaty;

  dispchar = getboundaries(ch);  /* Get the region of the charmap
				    containing the character to
				    display */

  locaty = 6;
  locatx = 2 + location * 6;

  /* If the character is a memory display character, use the memory
     location display region.  Valid Characters are:
     '_'  -  No data in Memory Location
     '='  -  Value in Memory Location, Not Locked
     '#'  -  Constant in Memory Location, Locked
  */
  if ((ch == '=') || (ch == '_') || ch == '#') {
    locaty = 15;
  }

  XCopyArea(display, charmap.pixmap, win, gc,
	    dispchar.x, dispchar.y,
	    dispchar.i-dispchar.x, dispchar.j-dispchar.y,
	    locatx, locaty);
  XCopyArea(display, charmap.pixmap, iconwin, gc,
	    dispchar.x, dispchar.y,
	    dispchar.i-dispchar.x, dispchar.j-dispchar.y,
	    locatx, locaty);

}  /***** End of Function displaychar() **************************/

/****************************************************************
 *  Function:     displaystr
 ****************************************************************
    Description:

    Change History:
    Date       Modification
    11/09/00   Added Capabilities for "Locked" memories
    11/01/00   Function header updated
    10/30/00   Added memory location indicators
 ****************************************************************/
void displaystr(void) {
  extern char   DispString[];
  extern int    MemLock[];
  extern double MemArray[];
  extern int    Verbose;
  int i;

  if (Verbose) printf("Displaystr %s\n", DispString);

  /* Update the alphanumeric display */
  for (i = 0; i < DISPSIZE; i++)
    displaychar(DispString[i], i);

  /* Update the memory location indicators */
  for (i = 0; i < NUM_MEM_CELLS; i++) {
    if (MemArray[i] == 0.0)
      displaychar('_', i);    /* Value NOT stored here */
    else if (MemLock[i] == 0)
      displaychar('=', i);    /* Value IS stored here */
    else
      displaychar('#', i);    /* Constant IS stored here */

  }

} /***** End of function displaystr() ***************************/

/****************************************************************
 *  Function:     show_usage
 ****************************************************************
    Description:
      This function prints a brief usage message to stdout,
      and exits.

    Change History:
    Date       Modification
    11/01/00   Function header updated
 ****************************************************************/
void show_usage(void) {

  printf("\n");
  printf(" %s %s\n",app_name, PACKAGE_VERSION);
  printf("\n");
  printf("usage: %s [-g geometry] [-d display] [-f <filename>] [-v] [-h] \n",
	 app_name);
  printf("\n");
  printf("-g  <geometry>   Window Geometry - ie: 64x64+10+10\n");
  printf("-d  <display>    Display -  ie: 127.0.0.1:0.0\n");
  printf("-f  <filename>   Name of Config file - ie: /home/user/.wmcalc\n");
  printf("-v               Verbose Mode. \n");
  printf("-h               Help. This message.\n");
  printf("\n");
  exit(OKAY);
} /***** End of function show_usage() ***************************/

/****************************************************************
 *  Function:     error_handler
 ****************************************************************
    Description:
     This function will handle all fatal error conditions that exist.
     Error condition codes are kept in wmcalc_err.h

    Change History:
    Date       Modification
    11/1/00    Function created
 ****************************************************************/
void error_handler (int err_code, char *err_string) {
  extern char tempfile[];

  if (err_code == OKAY) {
    /* This case should never happen.
       If it does, somebody screwed up (probably me),
       but don't kill the program!! */
    return;
  }
  else {
    fprintf(stderr, "Error Code %d: ", err_code);
    switch (err_code) {
    case ERR_FILE_NOT_FOUND:
      fprintf(stderr, "Could not open file %s\n",configfile);
      break;
    case ERR_TMP_FILE_FAILED:
      fprintf(stderr, "Could not open temporary file %s\n", tempfile);
      break;
    case ERR_X_CREATE_WINDOW:
      fprintf(stderr, "Could not create simple window\n");
      break;
    case ERR_X_CREATE_BITMAP:
      fprintf(stderr, "Could not create bitmap from data\n");
      break;
    case ERR_SETUP_WINDOW_NAME:
      fprintf(stderr, "Could not setup window name %s\n", err_string);
      break;
    case ERR_CREATE_GC:
      fprintf(stderr, "XCreateGC\n");
      break;
    case ERR_X_DISPLAY:
      fprintf(stderr, "Could not open display on %s\n", err_string);
      break;
    case ERR_CREATE_PIXMAP:
      fprintf(stderr, "Can't Create %s Pixmap\n", err_string);
      break;
    /*   case : */
    /*     break; */
    default:
      fprintf(stderr, "Unknown Error\n");
      break;
    }
  }
  exit(err_code);
} /*****  End of Function error_handler **************************/
