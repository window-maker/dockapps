/* csnlock v1.02
 * Copyright (C) 2002 Simon Hunter (lists@sprig.dyn.dhs.org)
 *
 * cnslock is a dock application that displays the current state of the
 * three lock keys (caps, num, and scroll)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have receive a copy of the GNU General Public License along with
 * this program; if you still want it, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Hacked from various wm applets.
 *
 */

/* general includes */
#include <X11/Xlib.h>
#include <libdockapp/dockapp.h>

#include "include/kleds.h"
#include "graphics/caps_num_scroll.xpm"

/* stuff */
void cnslock_init(void);
void cnslock_update(void);

/* globals */
Pixmap led_pix;
Pixmap win_pix;

/* the main routine */
int main(int argc, char **argv)
{
	DACallbacks eventCallbacks = {NULL, NULL, NULL, NULL, NULL, NULL,
				      cnslock_update};

	DAParseArguments(argc, argv, NULL, 0,
			 "This is an applet that displays the various states of"
			 "the CAPS, NUM and SCROLL\nLOCK keys.",
			 "cnslock applet v" VERSION);
	DAInitialize(NULL, "cnslock", 56, 56, argc, argv);
	DASetCallbacks(&eventCallbacks);
	DASetTimeout(20);
	cnslock_init();
	DAShow();
	DAEventLoop();

	return 0;
}

void cnslock_init(void)
{
	int status;
	short unsigned int w, h;

	win_pix = DAMakePixmap();
	XFillRectangle(DADisplay, win_pix, DAGC, 0, 0, 56, 56);
	DAMakePixmapFromData(caps_num_scroll_xpm, &led_pix, NULL, &w, &h);

	status = check_kleds();

	if (status & 1)
		XCopyArea(DADisplay, led_pix, win_pix, DAGC,
			  9, 9, 20, 26, 9, 9);
	if (status & 2)
		XCopyArea(DADisplay, led_pix, win_pix, DAGC,
			  32, 13, 15, 20, 32, 13);
	if (status & 3)
		XCopyArea(DADisplay, led_pix, win_pix, DAGC,
			  8, 38, 40, 10, 8, 38);
	DASetPixmap(win_pix);
}

/* update caps, num, scroll lock */
void cnslock_update(void)
{
	static int status;
	int new_status;
	int new_pix = 0;

	new_status = check_kleds();

	if ((status & 1) != (new_status & 1)) {
		if ((new_status & 1) == 1)
			XCopyArea(DADisplay, led_pix, win_pix, DAGC,
				  9, 9, 20, 26, 9, 9);
		else
			XFillRectangle(DADisplay, win_pix, DAGC, 9, 9, 20, 26);
		new_pix = 1;
	}
	if ((status & 2) != (new_status & 2)) {
		if ((new_status & 2) == 2)
			XCopyArea(DADisplay, led_pix, win_pix, DAGC,
				  32, 13, 15, 20, 32, 13);
		else
			XFillRectangle(DADisplay, win_pix, DAGC,
				       32, 13, 15, 20);
		new_pix = 1;
	}
	if ((status & 4) != (new_status & 4)) {
		if ((new_status & 4) == 4)
			XCopyArea(DADisplay, led_pix, win_pix, DAGC,
				  8, 38, 40, 10, 8, 38);
		else
			XFillRectangle(DADisplay, win_pix, DAGC, 8, 38, 40, 10);
		new_pix = 1;
	}

	if (new_pix)
		DASetPixmap(win_pix);
	status = new_status;
}
