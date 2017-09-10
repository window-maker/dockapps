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

/* Command line parsing and X resource manager */
#include <X11/Xresource.h>

/* XKB fun */
#include <X11/XKBlib.h>

/* My own fun */
#include "fookb.h"
#include "images.h"
#include "sound.h"
#include "opts.h"

#define sterror(x) (void)printf("Strange error, please report! %s:%d, %s\n",\
		__FILE__, __LINE__, x)


static void getGC(Window win, GC *gc, Display *dpy)
{
	unsigned long valuemask = 0;	/* No data in ``values'' */
	XGCValues values;
	*gc = XCreateGC(dpy, win, valuemask, &values);
	/* FIXME Check if there was an error */
/*	XSetForeground(dpy, *gc, BlackPixel(dpy, scr)); */
}


int main(int argc, register char *argv[])
{
	Display *dpy;
	int scr;
	Window root;

	int err;

	int event_rtrn;	/* for XkbOpenDisplay */
	int error_rtrn;
	int reason_rtrn;

	XkbEvent labuda;	/* Xkb event. X event will be labuda.core */

	Window billy;		/*   _The_ Window  */

#ifdef WMAKER
	Window dilly;		/* ...and his icon */
#endif

	int border = 0;		/* _The_ Window parameters */

	XWMHints *wm_hints;
	XClassHint class_hints;

#ifdef WMAKER			/* If we use Windowmaker, _The_ Window will */
	XSizeHints *BigBunny;	/* be withdrawn. And we do not want to
				   place empty border by hand. */
#endif

	GC gc;			/* Graphic context */

	char resn[] = "fookb";
	char resc[] = "Fookb";


	int state = 0;		/* We suppose that latin keyboard is the
				   primal state FIXME */

	/*
	event_rtrn = malloc(sizeof(int));
	error_rtrn = malloc(sizeof(int));
	reason_rtrn = malloc(sizeof(int));
	*/

	XrmInitialize();	/* We should initialize X resource
				   manager before doing something else 
				 */

	ParseOptions(&argc, argv);	/* We should parse command line
					   options and try to find '-display'
					   before opening X display */

/* Go, fighters, go! */
	dpy = XkbOpenDisplay(mydispname,
			     &event_rtrn,
			     &error_rtrn, NULL, NULL, &reason_rtrn);

/* Oops. */
	if (dpy == NULL) {
		(void)puts("Cannot open display.");
		exit(EXIT_FAILURE);
	}

	scr = DefaultScreen(dpy);
	root = RootWindow(dpy, scr);

	MoreOptions(dpy);		/* Now we can parse X server resource
				   database. It is not available
				   before display is opened */

/* We would like receive the only Xkb event: XkbStateNotify. And only
 * when XkbLockGroup happens. */

	if (False == XkbSelectEvents(dpy,
			XkbUseCoreKbd,
			XkbAllEventsMask,
			0)) {
		sterror("Cannot XkbSelectEvents. It's your problem -- not mine.");
		exit(EXIT_FAILURE);
	} /* Deselect all events */

	if (False == XkbSelectEventDetails(dpy,
				XkbUseCoreKbd,
				XkbStateNotify,
				XkbAllEventsMask,
				XkbGroupLockMask)) {
		sterror("Cannot XkbSelectEventDetails. It's your problem -- not mine.");
		exit(EXIT_FAILURE);
	} /* Select XkbStateNotify/XkbgroupLock */ 
	
	read_images(dpy);		/* Let's read icon images */

/* Run out! */
	billy = XCreateSimpleWindow(dpy,
				    root,
				    0, 0,
				    get_width(), get_height(),
				    border,
				    BlackPixel(dpy, scr),
				    WhitePixel(dpy, scr));
	XStoreName(dpy, billy, "fookb");

#ifdef WMAKER
	dilly = XCreateSimpleWindow(dpy,
				    root,
				    0, 0,
				    get_width(), get_height(),
				    border,
				    BlackPixel(dpy, scr),
				    WhitePixel(dpy, scr));
#endif

	class_hints.res_name = resn;
	class_hints.res_class = resc;

	err = XSetClassHint(dpy, billy, &class_hints);
	switch(err) {
		case BadAlloc:
			sterror("BadAlloc");
			exit(EXIT_FAILURE);
		case BadWindow:
			sterror("BadWindow");
			exit(EXIT_FAILURE);
	}

	wm_hints = XAllocWMHints();
	wm_hints->window_group = billy;

#ifdef WMAKER
	wm_hints->icon_window = dilly;
#endif

	wm_hints->input = False;
	wm_hints->flags = InputHint | WindowGroupHint;

#ifdef WMAKER
	wm_hints->flags = wm_hints->flags | IconWindowHint;
	err = XSetWMHints(dpy, dilly, wm_hints);
	switch(err) {
		case BadAlloc:
			sterror("BadAlloc");
			exit(EXIT_FAILURE);
		case BadWindow:
			sterror("BadWindow");
			exit(EXIT_FAILURE);
	}
	wm_hints->initial_state = WithdrawnState;
	wm_hints->flags = wm_hints->flags | StateHint;
#endif

	err = XSetWMHints(dpy, billy, wm_hints);
	switch(err) {
		case BadAlloc:
			sterror("BadAlloc");
			exit(EXIT_FAILURE);
		case BadWindow:
			sterror("BadWindow");
			exit(EXIT_FAILURE);
	}

#ifdef WMAKER
	/* Look at the comment for XSizeHints * BigBunny */
	BigBunny = XAllocSizeHints();
	if (NULL == BigBunny) {
		(void)printf("Not enough memory, %s:%d.\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
	}
/* Nasty Hack. XSizeHints.x & XSizeHints.y are obsolete. */
	BigBunny->x = 0;
	BigBunny->y = 0;
	BigBunny->flags = PPosition;
	XSetWMNormalHints(dpy, billy, BigBunny);
	XFree(BigBunny);
#endif

/* The only thing we would like to do - update our billy */
#ifndef WMAKER
	XSelectInput(dpy, billy, ExposureMask | ButtonPressMask);
#else				/* ...or dilly?! */
	XSelectInput(dpy, dilly, ExposureMask | ButtonPressMask);
#endif

	XSetCommand(dpy, billy, argv, argc);

/* Programmer supplied functions */
#ifndef WMAKER
	getGC(billy, &gc, dpy);
#else
	getGC(dilly, &gc, dpy);
#endif


/* Let's look */
	XMapWindow(dpy, billy);	/* We would like to see the window. */

/* HELLO! HELLO! HELLO! Is that our GOOD FRIEND main loop here? */
	while (1) {
		XNextEvent(dpy, &labuda.core);
		switch (labuda.core.type) {

		case Expose:	/* We should update our window. */
			if (labuda.core.xexpose.count != 0)
				/* Well, I knew what does it mean,
				   but I forgot :) */
				break;
#ifndef WMAKER
			update_window(billy, gc, state, dpy);
#else
			update_window(dilly, gc, state, dpy);
#endif
			break;
		case ButtonPress:

			switch (labuda.core.xbutton.button) {
			case Button1:
#ifdef DEBUG
				puts("Button1 pressed.");
#endif
				XkbLockGroup(dpy,
					     XkbUseCoreKbd,
					     (state + 1) % 4);
				break;
			case Button2:
#ifdef DEBUG
				puts("Button2 pressed.");
#endif
				XkbLockGroup(dpy,
					     XkbUseCoreKbd,
					     (state + 3) % 4);
				break;
			case Button3:
#ifdef DEBUG
				puts("Button3 pressed, bye.");
#endif
				XFreeGC(dpy, gc);
				XDestroyWindow(dpy, billy);
#ifdef WMAKER
				XDestroyWindow(dpy, dilly);
#endif
				XCloseDisplay(dpy);
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
#ifndef WMAKER
			update_window(billy, gc, state, dpy);
#else
			update_window(dilly, gc, state, dpy);
#endif

#ifdef DEBUG
			puts(".");	/* XkbLockGroup happens */
#endif
		}
	}

}
