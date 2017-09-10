/*
 * fookb.c
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 */

#include <stdlib.h>		/* malloc() */
#include <stdio.h>		/* puts() */

/* X Window headers */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* XKB fun */
#include <X11/XKBlib.h>

#include <libdockapp/dockapp.h>

/* My own fun */
#include "fookb.h"
#include "images.h"
#include "sound.h"
#include "opts.h"

char *icon1 = NULL;
char *icon2 = NULL;
char *icon3 = NULL;
char *icon4 = NULL;
char *iconboom = NULL;
char *display = NULL;

#define sterror(x) (void)printf("Strange error, please report! %s:%d, %s\n",\
		__FILE__, __LINE__, x)

int main(int argc, register char *argv[])
{
	XkbEvent labuda;        /* Xkb event. X event will be labuda.core */
	int state = 0;		/* We suppose that latin keyboard is the
				   primal state FIXME */
	Pixmap pixmap;
	DAProgramOption options[] = {
		{NULL, "--icon1",
		 "Icon to show for the 1st Xkb group",
		 DOString, False, {&icon1}},
		{NULL, "--icon2",
		 "Icon to show for the 2nd Xkb group",
		 DOString, False, {&icon2}},
		{NULL, "--icon3",
		 "Icon to show for the 3rd Xkb group",
		 DOString, False, {&icon3}},
		{NULL, "--icon4",
		 "Icon to show for the 4th Xkb group",
		 DOString, False, {&icon4}},
		{NULL, "--iconboom",
		 "Icon to show when Xkb system goes crazy",
		 DOString, False, {&iconboom}},
		{"-d", "--display",
		 "X display to use (normally not needed)",
		 DOString, False, {&display}},
	};


	DAParseArguments(argc, argv, options, 6,
			 "XKB state indicator for Window Maker",
			 PACKAGE_STRING);

	DAOpenDisplay(display, argc, argv);
	read_images(DADisplay);		/* Let's read icon images */
	DACreateIcon(PACKAGE_NAME, get_width(), get_height(), argc, argv);
	XSelectInput(DADisplay, DAWindow, ButtonPressMask);

/* We would like receive the only Xkb event: XkbStateNotify. And only
 * when XkbLockGroup happens. */

	if (False == XkbSelectEvents(DADisplay,
			XkbUseCoreKbd,
			XkbAllEventsMask,
			0)) {
		sterror("Cannot XkbSelectEvents. It's your problem -- not mine.");
		exit(EXIT_FAILURE);
	} /* Deselect all events */

	if (False == XkbSelectEventDetails(DADisplay,
				XkbUseCoreKbd,
				XkbStateNotify,
				XkbAllEventsMask,
				XkbGroupLockMask)) {
		sterror("Cannot XkbSelectEventDetails. It's your problem -- not mine.");
		exit(EXIT_FAILURE);
	} /* Select XkbStateNotify/XkbgroupLock */

	pixmap = DAMakePixmap();

	update_window(pixmap, DAGC, state, DADisplay);
	DASetPixmap(pixmap);

	DAShow();

/* HELLO! HELLO! HELLO! Is that our GOOD FRIEND main loop here? */
	while (1) {
		XNextEvent(DADisplay, &labuda.core);
		switch (labuda.core.type) {

		case ButtonPress:

			switch (labuda.core.xbutton.button) {
			case Button1:
#ifdef DEBUG
				puts("Button1 pressed.");
#endif
				XkbLockGroup(DADisplay,
					     XkbUseCoreKbd,
					     (state + 1) % 4);
				break;
			case Button2:
#ifdef DEBUG
				puts("Button2 pressed.");
#endif
				XkbLockGroup(DADisplay,
					     XkbUseCoreKbd,
					     (state + 3) % 4);
				break;
			case Button3:
#ifdef DEBUG
				puts("Button3 pressed, bye.");
#endif
				XFreeGC(DADisplay, DAGC);
				XDestroyWindow(DADisplay, DAWindow);
				XCloseDisplay(DADisplay);
				exit(0);
			}
			break;
		default:	/* XkbLockGroup happens : FIXME */
			drip();
			state = labuda.state.group;
#ifdef DEBUG
			printf("%u\n", state);
#endif
			if ((state < 0) || (state > 4))
				state = 4;
			update_window(pixmap, DAGC, state, DADisplay);
			DASetPixmap(pixmap);

#ifdef DEBUG
			puts(".");	/* XkbLockGroup happens */
#endif
		}
	}
}
