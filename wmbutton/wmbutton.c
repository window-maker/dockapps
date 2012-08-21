/***********************************************************************
 *   Code is based on wmppp, wmload, wmtime, wmcp, and asbutton
 *   Author: Edward H. Flora <ehflora@ksu.edu>
 *   Ver 0 Rel 6.1    Jan 23, 2005
 *
 *   Contributors:
 *              Christian 'Greek0' Aichinger <Greek0@gmx.net>
 *                  Did some code cleanup and fixed several memory leaks.
 *              Ralf Horstmann <ralf.horstmann@gmx.de>
 *                  Added ability to load pixmaps at startup,
 *                  without having to re-compile
 *              Michael Cohrs <camico@users.sourceforge.net>
 *                  Added Tool Tips, and updated graphics
 *              Bruno Essmann <essmann@users.sourceforge.net>)
 *                  Creator of wmpager
 *              Casey Harkins <charkins@cs.wisc.edu>
 *                  Bug fix reading config file path - 3/6/99
 *                  Added button-presses, and other - denoted by *charkins*
 *              Ben Cohen <buddog@aztec.asu.edu>
 *                  original author of wmcp (et al.)
 *              Thomas Nemeth <tnemeth@multimania.com>
 *                  contributor to wmcp
 *              Michael Henderson <mghenderson@lanl.gov>
 *                  Application ideas, suggestions
 *              Ryan ?? <pancake@mindspring.com>
 *                  Modified wmbutton to asbutton.
 *                  Note: asbutton is a seperate program, not associated
 *                        with wmbutton (just as wmbutton is not associated
 *                        with wmcp)
 *              Jon Bruno
 *                  Web Page Development
 *    The contributors listed above are not necessarily involved with the
 *    development of wmbutton.  I'm listing them here partially as thanks for
 *    helping out, catching bugs in the code, etc.
 ***********************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "wmbutton.h"

#include "backdrop.xpm"           /* background graphic */
#include "buttons.xpm"            /* graphic of 9 buttons */
#include "mask.xbm"               /* Border Graphic */

/*************** Function Prototypes ***********************************/
void redraw(void);
void getPixmaps(void);
int  whichButton(int x, int y);   // determine which button has been pressed
void SetWmHints();
void SetClassHints();

/***********************************************************************
 * 		Globals..    OK.. there's too many globals.
 *                           Feel free and fix it, if you'd like.
 ***********************************************************************/
Display *display;
int screen;
Window rootwin, win, iconwin;
GC gc;
int depth;
Pixel bg_pixel, fg_pixel;

struct Config_t Config;

typedef struct _XpmIcon {
	Pixmap pixmap;
	Pixmap mask;
	XpmAttributes attributes;
} XpmIcon;

typedef struct _button_region {
	int x, y;
	int i, j;
} ButtonArea;

ButtonArea button_region[9];

XpmIcon template, visible, buttons;

int border = 0;
int button_pressed = -1;	/* button to be drawn pressed *charkins*/

char *app_name = "wmbutton";

/***********************************************************************
 * 		Main
 ***********************************************************************/
int main(int argc, char **argv)
{
	XEvent report;
	XGCValues xgcValues;
	XTextProperty app_name_atom;
	XSizeHints xsizehints;
	Pixmap pixmask;

	int dummy = 0;
	int N = 1;	/* Button number pressed to goto app # */

	/* Added for Tool Tip Support */
	long nTooltipShowDelay = TOOLTIP_SHOW_DELAY;
	long nTooltipReshowDelay = TOOLTIP_RESHOW_DELAY;
	long nTooltipTimer = -1;
	long nTooltipHideTimer = -1;
	long nNow;
	int nTooltipButton = 0, nTooltipX = 0, nTooltipY = 0;

	/* Parse Command Line Arguments */
	parseargs(argc, argv);

	/* Open Display */
	if ((display = XOpenDisplay(Config.Display_str)) == NULL)
		err_mess(FAILDISP, Config.Display_str);

	screen  = DefaultScreen(display);
	rootwin = RootWindow(display, screen);
	depth   = DefaultDepth(display, screen);

	bg_pixel = WhitePixel(display, screen);
	fg_pixel = BlackPixel(display, screen);

	xsizehints.flags  = USSize | USPosition;
	xsizehints.width  = 64;
	xsizehints.height = 64;

	/* Parse Geometry string and fill in sizehints fields */
	XWMGeometry(display, screen,
		    Config.Geometry_str,
		    NULL,
		    border,
		    &xsizehints,
		    &xsizehints.x,
		    &xsizehints.y,
		    &xsizehints.width,
		    &xsizehints.height,
		    &dummy);

	if ((win = XCreateSimpleWindow(display,
				       rootwin,
				       xsizehints.x,
				       xsizehints.y,
				       xsizehints.width,
				       xsizehints.height,
				       border,
				       fg_pixel, bg_pixel)) == 0)
		err_mess(FAILSWIN, NULL);

	if ((iconwin = XCreateSimpleWindow(display,
					   win,
					   xsizehints.x,
					   xsizehints.y,
					   xsizehints.width,
					   xsizehints.height,
					   border,
					   fg_pixel, bg_pixel)) == 0)

		err_mess(FAILICON, NULL);

	/* Set up shaped windows */
	/* Gives the appicon a border so you can grab and move it. */
	if ((pixmask = XCreateBitmapFromData(display,
					     win,
					     mask_bits,
					     mask_width,
					     mask_height)) == 0)
		err_mess(FAILXPM, NULL);

	XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
	XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);

	/* Convert in pixmaps from .xpm includes. */
	getPixmaps();

	/* Interclient Communication stuff */
	/* Appicons don't work with out this stuff */
	SetWmHints();
	SetClassHints();

	XSetWMNormalHints(display, win, &xsizehints);

	/* Tell window manager what the title bar name is. We never see */
	/* this anyways in the WithdrawnState      */
	if (XStringListToTextProperty(&app_name, 1, &app_name_atom) == 0)
		err_mess(FAILWNAM, app_name);

	XSetWMName(display, win, &app_name_atom);

	/* Create Graphic Context */
	if ((gc = XCreateGC(display, win, (GCForeground | GCBackground),
			    &xgcValues)) == NULL)
		err_mess(FAILGC, NULL);

	/* XEvent Masks. We want both window to process X events */
	XSelectInput(display, win,
		     ExposureMask |
		     ButtonPressMask |
		     ButtonReleaseMask | /* added ButtonReleaseMask *charkins*/
		     PointerMotionMask |
		     StructureNotifyMask |
		     LeaveWindowMask);

	XSelectInput(display, iconwin,
		     ExposureMask |
		     ButtonPressMask |
		     ButtonReleaseMask | /* added ButtonReleaseMask *charkins*/
		     PointerMotionMask |
		     StructureNotifyMask |
		     LeaveWindowMask);

	/* Store the 'state' of the application for restarting */
	XSetCommand(display, win, argv, argc);

	/* Window won't ever show up until it is mapped.. then drawn after a 	*/
	/* ConfigureNotify */
	XMapWindow(display, win);

	/* Initialize Tooltip Support */
	initTime();
	initTooltip(!Config.bTooltipDisable, Config.szTooltipFont, Config.bTooltipSwapColors);

	/* X Event Loop */
	while (1) {
		while (XPending(display) || nTooltipTimer == -1) {
			XNextEvent(display, &report);

			switch (report.type) {
			case Expose:
				if (report.xexpose.count != 0)
					break;

				if (Config.Verbose)
					fprintf(stdout, "Event: Expose\n");

				redraw();
				break;

			case ConfigureNotify:
				if (Config.Verbose)
					fprintf(stdout, "Event: ConfigureNotify\n");

				redraw();
				break;

			case MotionNotify:
				if (hasTooltipSupport()) {
					if (!hasTooltip()) {
						nTooltipTimer = currentTimeMillis();
						nTooltipX = report.xbutton.x;
						nTooltipY = report.xbutton.y;
						nTooltipButton = whichButton(report.xbutton.x, report.xbutton.y);
					} else {
						int nButton = whichButton(report.xbutton.x, report.xbutton.y);
						if (nButton != nTooltipButton) {
							hideTooltip();
							nTooltipTimer = -1;
							nTooltipX = report.xbutton.x;
							nTooltipY = report.xbutton.y;
							nTooltipButton = nButton;
							showTooltip(nTooltipButton, nTooltipX, nTooltipY);
						}
					}
				}
				break;

			case LeaveNotify:
				if (Config.Verbose)
					fprintf(stdout, "Event: LeaveNotify\n");

				if (hasTooltip()) {
					hideTooltip();
					nTooltipHideTimer = currentTimeMillis();
				}
				nTooltipTimer = -1;
				break;

			case ButtonPress:	/* draw button pressed, don't launch *charkins*/
				if (hasTooltip()) {
					hideTooltip();
					nTooltipHideTimer = currentTimeMillis();
				}

				switch (report.xbutton.button) {
				case Button1:
					N = whichButton(report.xbutton.x, report.xbutton.y);
					if ((N >= 0) && (N <= NUMB_OF_APPS)) {
						button_pressed = N + LMASK;
						redraw();
					}

					if (Config.Verbose)
						fprintf(stdout, "Button 1:x=%d y=%d N=%d\n",
							report.xbutton.x, report.xbutton.y, N+LMASK);
					break;

				case Button2:
					if (!Config.mmouse) {
						N = whichButton(report.xbutton.x, report.xbutton.y);
						if ((N >= 0) && (N <= NUMB_OF_APPS)) {
							button_pressed = N + MMASK;
							redraw();
						}

						if (Config.Verbose)
							fprintf(stdout, "Button 2:x=%d y=%d N=%d\n",
								report.xbutton.x, report.xbutton.y, N+MMASK);
					}
					break;

				case Button3:
					N = whichButton(report.xbutton.x, report.xbutton.y);
					if ((N >= 0) && (N <= NUMB_OF_APPS)) {
						button_pressed = N + RMASK;
						redraw();
					}

					if (Config.Verbose)
						fprintf(stdout, "Button 3:x=%d y=%d N=%d\n",
							report.xbutton.x, report.xbutton.y, N+RMASK);
					break;
				}
				break;

			case ButtonRelease:   /* launch app here if still over button *charkins*/
				switch (report.xbutton.button) {
				case Button1:
					N = whichButton(report.xbutton.x, report.xbutton.y);
					if ((N >= 0) && (N <= NUMB_OF_APPS) && (N == button_pressed))
						RunAppN(N + LMASK);

					button_pressed = -1;
					redraw();

					if (Config.Verbose)
						fprintf(stdout, "Button 1:x=%d y=%d N=%d\n",
							report.xbutton.x, report.xbutton.y, N+LMASK);
					break;

				case Button2:
					if (!Config.mmouse) {
						N = whichButton(report.xbutton.x, report.xbutton.y);
						if ((N >= 0) && (N <= NUMB_OF_APPS) && (N == button_pressed))
							RunAppN(N + MMASK);

						button_pressed = -1;
						redraw();

						if (Config.Verbose)
							fprintf(stdout, "Button 2:x=%d y=%d N=%d\n",
								report.xbutton.x, report.xbutton.y, N+MMASK);
					}
					break;

				case Button3:
					N = whichButton(report.xbutton.x, report.xbutton.y);
					if ((N >= 0) && (N <= NUMB_OF_APPS) && (N == button_pressed))
						RunAppN(N + RMASK);

					button_pressed = -1;
					redraw();

					if (Config.Verbose)
						fprintf(stdout, "Button 3:x=%d y=%d N=%d\n",
							report.xbutton.x, report.xbutton.y, N+RMASK);
					break;
				}
				break;

			case DestroyNotify:
				if (Config.Verbose)
					fprintf(stdout, "Bye\n");

				destroyTooltip();
				XFreeGC(display, gc);
				XDestroyWindow(display, win);
				XDestroyWindow(display, iconwin);
				XCloseDisplay(display);
				exit(0);
				break;
			}
		}

		usleep(50000);
		nNow = currentTimeMillis();
		if (nTooltipTimer != -1 &&
		    ((nNow > nTooltipTimer + nTooltipShowDelay) ||
		     (nNow < nTooltipHideTimer + nTooltipReshowDelay))) {
			showTooltip(nTooltipButton, nTooltipX, nTooltipY);
			nTooltipTimer = -1;
		}
	}

	return (0);
}
/***********************************************************************/

/***********************************************************************
 *   redraw
 *
 *	 Map the button region coordinates.
 *
 *   Draw the appropriate number of buttons on the 'visible' Pixmap
 *   using data from the 'buttons' pixmap.
 *
 *   Then, copy the 'visible' pixmap to the two windows ( the withdrawn
 *   main window and the icon window which is the main window's icon image.)
 ***********************************************************************/
void redraw() {
	int n, i, j, dest_x, dest_y, space, offset, bsize = 18;

	if (Config.Verbose)
		fprintf(stdout, "In Redraw()\n");

	space = 0;
	offset = 5;
	XCopyArea(display, template.pixmap, visible.pixmap, gc, 0, 0,
		  template.attributes.width, template.attributes.height, 0, 0);

	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			n = i + j * 3;
			dest_x = i * (bsize + space) + offset + space;
			dest_y = j * (bsize + space) + offset + space;

			/* Define button mouse coords */
			button_region[n].x = dest_x;
			button_region[n].y = dest_y;
			button_region[n].i = dest_x + bsize - 1;
			button_region[n].j = dest_y + bsize - 1;

			/* Copy button images for valid apps */
			if ((n + 1) <= NUMB_OF_APPS)
				XCopyArea(display, buttons.pixmap, visible.pixmap, gc,
					  i * bsize, j * bsize, bsize, bsize, dest_x, dest_y);
		}
	}

	if (button_pressed > 0) {	/* draw pressed button *charkins*/
		if (button_pressed > RMASK)
			button_pressed -= RMASK;
		else if (button_pressed > MMASK)
			button_pressed -= MMASK;
		else if (button_pressed > LMASK)
			button_pressed -= LMASK;

		i = (button_pressed - 1) % 3;	/* get col of button */
		j = (button_pressed - 1) / 3;	/* get row of button */
		dest_x = i * (bsize + space) + offset + space;
		dest_y = j * (bsize + space) + offset + space;
		XSetForeground(display, gc, bg_pixel);
		XDrawLine(display, visible.pixmap, gc,
			  dest_x + 1, dest_y + bsize - 1,
			  dest_x + bsize - 1, dest_y + bsize - 1);
		XDrawLine(display, visible.pixmap, gc,
			  dest_x + bsize - 1, dest_y + bsize - 1,
			  dest_x + bsize - 1, dest_y + 1);
		XSetForeground(display, gc, fg_pixel);
		XDrawLine(display, visible.pixmap, gc,
			  dest_x, dest_y, dest_x + bsize - 2, dest_y);
		XDrawLine(display, visible.pixmap, gc,
			  dest_x, dest_y, dest_x, dest_y + bsize - 2);
	} /*charkins*/

	flush_expose(win);
	XCopyArea(display, visible.pixmap, win, gc, 0, 0,
		  visible.attributes.width, visible.attributes.height, 0, 0);
	flush_expose(iconwin);
	XCopyArea(display, visible.pixmap, iconwin, gc, 0, 0,
		  visible.attributes.width, visible.attributes.height, 0, 0);
}
/***********************************************************************/

/***********************************************************************
 *  whichButton
 *
 *  Return the button that at the x,y coordinates. The button need not
 *  be visible ( drawn ). Return -1 if no button match.
 ***********************************************************************/
int whichButton(int x, int y)
{
	int index;

	for (index = 0; index < NUMB_OF_APPS; index++) {
		if (x >= button_region[index].x &&
		    x <= button_region[index].i &&
		    y >= button_region[index].y &&
		    y <= button_region[index].j)
			return(index + 1);
	}
	return -1;
}
/***********************************************************************/


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
	int loaded = 0;
	template.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);
	visible.attributes.valuemask  |= (XpmReturnPixels | XpmReturnExtensions);
	buttons.attributes.valuemask  |= (XpmReturnPixels | XpmReturnExtensions);

	if (Config.Verbose)
		fprintf(stdout, "In getPixmaps\n");

	/* Template Pixmap. Never Drawn To. */
	if (XpmCreatePixmapFromData(display, rootwin, backdrop_xpm,
				&template.pixmap, &template.mask,
				&template.attributes) != XpmSuccess)
		err_mess(FAILTMPL, NULL);

	/* Visible Pixmap. Copied from template Pixmap and then drawn to. */
	if (XpmCreatePixmapFromData(display, rootwin, backdrop_xpm,
				&visible.pixmap, &visible.mask,
				&visible.attributes) != XpmSuccess)
		err_mess(FAILVIS, NULL);

	/* Button Pixmap.  */
	if (access(Config.buttonfile, R_OK) == 0) {
		/* load buttons from file */
		if (XpmReadFileToPixmap(display, rootwin, Config.buttonfile,
					&buttons.pixmap, &buttons.mask,
					&buttons.attributes) != XpmSuccess)
			err_mess(FAILBUT, NULL);
		else
			loaded = 1;
	}

	if (!loaded) {
		/* Use Builtin Button Pixmap.  */
		if (Config.Verbose)
			fprintf(stdout, "Using builtin buttons pixmap\n");

		if (XpmCreatePixmapFromData(display, rootwin, buttons_xpm,
					    &buttons.pixmap, &buttons.mask,
					    &buttons.attributes) != XpmSuccess)
			err_mess(FAILBUT, NULL);
	}

	if (Config.Verbose)
		fprintf(stdout, "Leaving getPixmaps\n");

}
/*********************************************************************/

void SetWmHints()
{
	XWMHints *xwmhints;

	xwmhints = XAllocWMHints();
	xwmhints->flags = WindowGroupHint | IconWindowHint | StateHint;
	xwmhints->icon_window = iconwin;
	xwmhints->window_group = win;
	xwmhints->initial_state = WithdrawnState;
	XSetWMHints(display, win, xwmhints);
	XFree(xwmhints);
	xwmhints = NULL;
}

void SetClassHints()
{
	XClassHint xclasshint;

	xclasshint.res_name = "wmbutton";
	xclasshint.res_class = "Wmbutton";
	XSetClassHint(display, win, &xclasshint);
}
