/*  wmnet -- X IP accounting monitor
 *  Copyright 1998 Jesse B. Off
 *  Copyright 2000 Katharine Osborne
 *
 *  $Id: wmnet.c,v 1.28 1998/10/07 03:42:28 joff Exp joff $
 *
 *  This software is released under the GNU Public License agreement.
 *  No warranties, whatever.... you know the usuals.... this is free
 *  software.  if you use it, great... if you wanna make a change to it,
 *  great, but please send me the diff.
 *
 *  CHANGELOG:
 *  3/24/1998  --  First release, wrote this yesterday, so it may have
 *                 some bugs.
 *  3/25/1998  --  added logarithmic scaling.
 *                 some touch ups on the updateSpeedometer() to be a
 *                   little more helpful
 *                 added a little more width to the speedometer display
 *                    from a report that the text sometimes was drawn off
 *                    the scale (couldnt confirm, is it fixed?)
 *                 fixed prob for speeds > 9.9 megabytes per second (I
 *                    think... I can't go that fast myself
 *                 Made default maxspeed 6000 which I assume is better
 *                    for modems.
 *  5/11/1998  --  I witnessed the little problem with the speedometer
 *                    going off into the main window so I set a clipmask
 *                    Still don't understand how it could go off..I'm
 *                    using a fixed size font...
 *  5/13/1998  --  Modified the way it gets its stats from /proc/net/ip_acct
 *                    You now have to give it explicit accounting rule numbers
 *                    through the -T and -R options or else it defaults to the
 *                    fist two in tx, rx order, best just to make sure
 *                    you  set the -T and -R options.
 *  5/15/1998  --  Completed interface rewrite, definitely have more graphing
 *                    space, but I dont know if I like it yet.
 *  5/16/1998  --  Added shaded graphs, xload style.  Its a lot prettier!
 *  5/20/1998  --  Fixed afterstep wharfability problem... should also atleast
 *                    display in other WM's
 *  6/16/1998  --  Fixed handling of WM_DELETE_WINDOW, strtol() to strtoul(),
 *                    and now uses getopt_long.
 *  6/16/1998  --  Put in --withdrawn and --normalstate options to explicitly
 *                    set the type.  Still tries to auto-detect wmaker though.
 *  6/16/1998  --  Put in --execute and --promisc options.
 *  6/17/1998  --  Some code clean-ups.
 *  6/18/1998  --  Implemented a labeling mechanism.
 *  6/18/1998  --  Slowed down the speedometer display... it was getting annoying.
 *  6/23/1998  --  Split up to wmnet.c and wmnet.h
 *  8/5/1998   --  New options --device and --driver
 *  5/4/2000   --  Support added for OpenBSD
 */

#include<stdlib.h>
#include<stdio.h>
#include<X11/X.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/Xatom.h>
#if defined (__FreeBSD__) || defined (__OpenBSD__)
# include<sys/socket.h>
# include"getopt.h"
#else
# include<getopt.h>
#endif
#include<net/if.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include<math.h>
#include<sys/time.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/wait.h>
#include<X11/extensions/shape.h>

#include "XPM/arrow.xbm"
#include "wmnet.h"


/* Called on exit() from atexit() */
void exit_func(void) {
	XCloseDisplay(dpy);
}

/* Generic signal handler, if its a SIGCHLD, do a wait() to remove the zombie */
void got_signal(int x) {
	if(x == SIGCHLD) {
		wait(NULL);
		return;
	} else exit(7);
}



/* Does generic setting up of wmnet, (option parsing) and calls setupX() */
void setup_wmnet(int argc, char **argv) {
	int c;
	XColor thecolor;
	struct sigaction signal_action;
	char *txcolorString = NULL, *rxcolorString = NULL, *labelfgcolorString = NULL, *labelbgcolorString = NULL;
	char *parser = NULL;
#ifdef linux
	const struct option long_options[19] = {
#else
	const struct option long_options[17] = {
#endif
		{"device", required_argument, NULL, 'W'},
		{"label", required_argument, NULL, 'L'},
		{"labelfg", required_argument, NULL, 'F'},
		{"labelbg", required_argument, NULL, 'B'},
		{"logscale", no_argument, NULL, 'l'},
		{"help", no_argument, NULL, 'h'},
		{"execute", required_argument, NULL, 'e'},
#ifdef linux
		{"txrule", required_argument, NULL, 'T'},
		{"rxrule", required_argument, NULL, 'R'},
#endif
		{"txcolor", required_argument, NULL, 't'},
		{"rxcolor", required_argument, NULL, 'r'},
		{"maxrate", required_argument, NULL, 'x'},
		{"withdrawn", no_argument, NULL, 'w'},
		{"normalstate", no_argument, NULL, 'n'},
		{"promisc", required_argument, NULL, 'p'},
		{"unpromisc", required_argument, NULL, 'u'},
		{"driver", required_argument, NULL, 'D'},
		{"version", no_argument, NULL, 'v'},
		{0, 0, 0, 0}
	};



	/* Get options */
#ifdef linux
	while((c = getopt_long(argc, argv, "W:F:B:L:vp:u:wnle:R:T:r:t:D:d:x:h", long_options, NULL)) != EOF) {
#else
	while((c = getopt_long(argc, argv, "W:F:B:L:vp:u:wnle:r:t:D:d:x:h", long_options, NULL)) != EOF) {
#endif
		switch(c) {
			case 'v':
				printf("wmnet 1.06\n"
				       "Copyright (C) 1998, 2000 Jesse B. Off, Katharine Osborne <kaos@digitalkaos.net>\n"
				       "This program is released under the terms of the GNU Public License.\n");
				exit(14);
				break;
			case 'W':
				device = strdup(optarg);
				break;
			case 'D':
				parser = strdup(optarg);
				break;
#ifdef linux
			case 'T':
				out_rule = strtoul(optarg, NULL, 10);
				out_rule_string = strdup(optarg);
				break;
			case 'R':
				in_rule = strtoul(optarg, NULL, 10);
				in_rule_string = strdup(optarg);
				break;
#endif
			case 'L':
				graphbox_height = 35;
				if (label == NULL) {
					label = strdup(optarg);
				} else {
					fprintf(stderr, "wmnet: duplicate --label\n");
					exit(22);
				}
				break;
			case 'B':
				if (labelbgcolorString == NULL) {
					labelbgcolorString = (char *)alloca(strlen(optarg)+1);
					strncpy(labelbgcolorString, optarg, strlen(optarg)+1);
				} else {
					fprintf(stderr, "wmnet: duplicate --labelbg\n");
					exit(23);
				}
				break;
			case 'F':
				if (labelfgcolorString == NULL) {
					labelfgcolorString = (char *)alloca(strlen(optarg)+1);
					strncpy(labelfgcolorString, optarg, strlen(optarg)+1);
				} else {
					fprintf(stderr, "wmnet: duplicate --labelfg\n");
					exit(23);
				}
				break;
			case 'l':
				logscale = True;
				break;
			case 'r':
				if (rxcolorString == NULL) {
					rxcolorString = (char *)alloca(strlen(optarg)+1);
					strncpy(rxcolorString, optarg, strlen(optarg)+1);
				} else {
					fprintf(stderr, "wmnet: duplicate --rxcolor\n");
					exit(18);
				}
				break;
			case 't':
				if (txcolorString == NULL) {
					txcolorString = (char *)alloca(strlen(optarg)+1);
					strncpy(txcolorString, optarg, strlen(optarg)+1);
				} else {
					fprintf(stderr, "wmnet: duplicate --rxcolor\n");
					exit(19);
				}
				break;
			case 'x':
				maxRate = strtoul(optarg, NULL, 10);
				break;
			case 'd':
				delayTime = strtoul(optarg, NULL, 10);
				/* Having delayTime > 950000 causes some weirdness */
				delayTime = delayTime > 950000 ? 950000 : delayTime;
				break;
			case 'e':
				if (click_command == NULL) {
					click_command = strdup(optarg);
				} else {
					fprintf(stderr, "wmnet: duplicate --execute\n");
					exit(17);
				}
				break;
			case 'w':
				specified_state = WithdrawnState;
				break;
			case 'n':
				specified_state = NormalState;
				break;
			case 'u':
				{
					int fds;
					struct ifreq ifr;
					strncpy(ifr.ifr_name, optarg, IFNAMSIZ );
					ifr.ifr_name[IFNAMSIZ-1] = 0;
					if ((fds = socket(AF_INET, SOCK_DGRAM, 0)) == -1 || ioctl(fds, SIOCGIFFLAGS, &ifr) == -1 ) {
						perror("wmnet");
						exit(20);
					}
					if ((ifr.ifr_flags & IFF_PROMISC) != 0) {  /* Is promiscuous mode not already unset? */
						ifr.ifr_flags &= ~IFF_PROMISC;
						if (geteuid() != 0) {
							fprintf(stderr, "wmnet: this must be suid or you must be root!\n");
						}
						if(ioctl(fds, SIOCSIFFLAGS, &ifr) != 0) {
							fprintf(stderr, "wmnet: cannot unset promiscuous mode on %s\n", optarg);
							exit(21);
						}
					}
					close(fds);
				}
				break;
			case 'p':
				{
					int fds;
					struct ifreq ifr;
					strncpy(ifr.ifr_name, optarg, IFNAMSIZ );
					ifr.ifr_name[IFNAMSIZ-1] = 0;
					if ((fds = socket(AF_INET, SOCK_DGRAM, 0)) == -1 || ioctl(fds, SIOCGIFFLAGS, &ifr) == -1 ) {
						perror("wmnet");
						exit(16);
					}
					if ((ifr.ifr_flags & IFF_PROMISC) == 0) {  /* Is promiscuous mode not already set? */
						ifr.ifr_flags |= IFF_PROMISC;
						if (geteuid() != 0) {
							fprintf(stderr, "wmnet: this must be suid or you must be root!\n");
						}
						if(ioctl(fds, SIOCSIFFLAGS, &ifr) != 0) {
							fprintf(stderr, "wmnet: cannot set promiscuous mode on %s\n", optarg);
							exit(13);
						}
					}
					close(fds);
				}
				break;
			default:
				printf("wmnet-- v1.06 Katharine Osborne <kaos@digitalkaos.net>\n"
				       "http://www.digitalkaos.net/linux/wmnet/\n"
				       "-----------------------------------------------------\n"
				       "  -h, --help               this help\n"
                                       "  -v, --version            display version information\n"
                                       "  -L, --label=LABEL        display LABEL on bottom of window\n"
                                       "  -F, --labelfg=COLOR      foreground color for the label\n"
                                       "  -B, --labelbg=COLOR      background color for the label\n"
                                       "  -e, --execute=COMMAND    run COMMAND on click\n"
#ifdef linux
				       "  -T, --txrule=RULE        accounting rule number (ipfwadm) or\n"
				       "                           IP chain name (ipchains) to monitor for tx\n"
				       "  -R, --rxrule=RULE        accounting rule number (ipfwadm) or\n"
				       "                           IP chain name (ipchains) to monitor for rx\n"
#endif
				       "  -W, --device=DEVICE      monitor DEVICE for stats (devstats,kmem,pppstats)\n"
                                       "  -w, --withdrawn          start up in withdrawn state\n"
                                       "  -n, --normalstate        start up in normal, shaped state\n"
				       "  -t, --txcolor=COLOR      color for tx\n"
				       "  -r, --rxcolor=COLOR      color for rx\n"
				       "  -x, --maxrate=BYTES      max transfer rate for graph scale (default 120000)\n"
                                       "  -p, --promisc=DEVICE     put DEVICE into promiscuous mode to apply\n"
                                       "                           accounting rules to all network packets\n"
                                       "  -u, --unpromisc=DEVICE   turn off promiscuous mode on DEVICE\n"
				       "  -D, --driver=DRIVER      use DRIVER to get statistics\n"
				       "  -l, --logscale           use a logarithmic scale (great for ethernet\n"
                                       "                           connections with -x 10000000)\n"
				       "  -d DELAY                 delay time for polling statistics\n"
                                       "                           in microseconds  (default 100000)\n"
				       "\n");
				printf("Compiled in drivers: [%s]\n\n", available_drivers());
                                printf("Report bugs to joff@iastate.edu\n");
				exit(3);
		}
	}
	/* Relinquish suid privileges if there */
	seteuid(getuid());

	stat_gather = setup_driver(parser);


	if (txcolorString == NULL) txcolorString = "white";
	if (rxcolorString == NULL) rxcolorString = "red";
	if (labelfgcolorString == NULL) labelfgcolorString = "white";
	if (labelbgcolorString == NULL) labelbgcolorString = "black";

	/* Change dir to /, security precaution, and common courtesy */
	if (chdir("/") == -1) {
		perror("wmnet: chdir()");
		exit(16);
	}

	/* Open X Display */
	if ((dpy = XOpenDisplay(NULL)) == NULL) {
		fprintf(stderr,"wmnet: doh...can't connect to X server, giving up\n");
		exit(1);
	}

	/* assure ourself for a graceful exit */
	if (atexit(exit_func)) {
		fprintf(stderr,"wmnet: atexit() failed\n");
		exit(6);
	}

	signal_action.sa_handler = got_signal;
	sigemptyset(&signal_action.sa_mask);
	signal_action.sa_flags = (SA_NOCLDSTOP|SA_RESTART);
#ifdef linux
	signal_action.sa_restorer = NULL;
#endif
	if ((sigaction(SIGCHLD, &signal_action, NULL) == -1) ||
	    (sigaction(SIGINT, &signal_action, NULL) == -1) ||
	    (sigaction(SIGTERM, &signal_action, NULL) == -1)) {
		fprintf(stderr,"wmnet: couldn't set signal handler\n");
		exit(8);
	}


	/* Setup initial foreground color */
	if(rxcolorString) {
		if(!XParseColor(dpy, DefaultColormap(dpy, screen), rxcolorString, &thecolor)) {
			fprintf(stderr, "wmnet: what the heck is %s for a color?\n", rxcolorString);
			exit(12);
		}
		shadesOf(&thecolor, rx_pixel);
	}
	if(txcolorString) {
		if(!XParseColor(dpy, DefaultColormap(dpy, screen), txcolorString, &thecolor)) {
			fprintf(stderr, "wmnet: what the heck is %s for a color?\n", txcolorString);
			exit(5);
		}
		shadesOf(&thecolor, tx_pixel);
	}
	if(labelfgcolorString) {
		if(!XParseColor(dpy, DefaultColormap(dpy, screen), labelfgcolorString, &thecolor)) {
			fprintf(stderr, "wmnet: what the heck is %s for a color?\n", labelfgcolorString);
			exit(24);
		}
		XAllocColor(dpy, DefaultColormap(dpy, screen), &thecolor);
		labelfg_pixel = thecolor.pixel;
	}
	if(labelbgcolorString) {
		if(!XParseColor(dpy, DefaultColormap(dpy, screen), labelbgcolorString, &thecolor)) {
			fprintf(stderr, "wmnet: what the heck is %s for a color?\n", labelbgcolorString);
			exit(25);
		}
		XAllocColor(dpy, DefaultColormap(dpy, screen), &thecolor);
		labelbg_pixel = thecolor.pixel;
	}

	/* usleep() in between polls to /proc/net/ip_acct */
	if (delayTime <= 0) delayTime = 25000;

	/* Setup the X windows, GC's, initial states, etc */
	setupX();
	XSetCommand(dpy, main_window, argv, argc);

	/* Get the initial stats for startup */
	stat_gather();


	/* Rock n Roll */
	XMapWindow(dpy, main_window);

}

/* Called from setup_wmnet() to initialize the X windows stuff */
void setupX(void) {

	XWMHints hints;
	XSizeHints shints;
	XGCValues gcv;
	XColor color;
	XRectangle bound = { 0, 0, 56, 56 };


	screen = DefaultScreen(dpy);

	delete_atom = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	if (delete_atom == None) {
		fprintf(stderr,"wmnet: I need WindowMaker running! (or at least some window manager)\n");
		exit(2);
	}
	if (XInternAtom(dpy,"_WINDOWMAKER_WM_FUNCTION", True) != None) {
		if (specified_state == -1) specified_state = WithdrawnState;
	} else {
		if (specified_state == -1) specified_state = NormalState;
	}

	root_window = DefaultRootWindow(dpy);
	createWin(&main_window);

	color.red = color.green = color.blue = 12000;
	XAllocColor(dpy, DefaultColormap(dpy, screen), &color);
	darkgrey_pixel = color.pixel;

	color.red = color.green = color.blue = 32000;
	XAllocColor(dpy, DefaultColormap(dpy, screen), &color);
	grey_pixel = color.pixel;


	if ((arrow = XCreateBitmapFromData(dpy, root_window, arrow_bits, arrow_width, arrow_height)) == None) {
		fprintf(stderr, "wmnet: unable to create arrow bitmap\n");
		exit(11);
	}
	gcv.graphics_exposures = False;
	gcv.foreground = tx_pixel[HIGH_INTENSITY];
	gcv.background = darkgrey_pixel;
	gcv.font = XLoadFont(dpy, "5x8");
	graphics_context = XCreateGC(dpy, root_window, (GCFont|GCGraphicsExposures|GCForeground|GCBackground), &gcv);
	black_pixel = BlackPixel(dpy, screen);
	white_pixel = WhitePixel(dpy, screen);

	hints.window_group = main_window;
	hints.initial_state = specified_state;
	if (specified_state == WithdrawnState) {
		createWin(&icon_window);
		visible_window = &icon_window;
		hints.icon_window = icon_window;
	} else {
		visible_window = &main_window;
		hints.icon_window = None;
	}
	hints.flags = WindowGroupHint | StateHint | IconWindowHint;
	XSetWMHints(dpy,main_window,&hints);
	XSetWMProtocols(dpy, main_window, &delete_atom, 1);

	shints.min_width = 64;
	shints.min_height = 64;
	shints.max_width = 64;
	shints.max_height = 64;
	shints.flags = PMinSize | PMaxSize;
	XSetWMNormalHints(dpy, main_window, &shints);


	XStoreName(dpy, main_window, "wmnet");
	XShapeCombineRectangles(dpy, *visible_window, ShapeBounding, 4, 4, &bound, 1, ShapeBounding, 0);
	XSelectInput(dpy, *visible_window, (ExposureMask|ButtonPressMask));
	XMapSubwindows(dpy, *visible_window);

}

/* Utility function to create a window for setupX() */
void createWin(Window *win) {
	XClassHint classHint;
	XSetWindowAttributes windowAttrib;
	*win = XCreateSimpleWindow(dpy, root_window, 10, 10, 64, 64, 0, 0, 0);
	classHint.res_name = "wmnet";
	classHint.res_class = "WMNET";
	windowAttrib.background_pixmap = ParentRelative;
	XChangeWindowAttributes(dpy, *win, CWBackPixmap, &windowAttrib);
	XSetClassHint(dpy, *win, &classHint);
}



/* Handles Expose events, repaints the window */
void redraw(XExposeEvent *ee) {
        static XRectangle cliprect = { 4, 51, 56, 9 };
	XSetForeground(dpy, graphics_context, darkgrey_pixel);
/*	if (wmaker_present == False) XFillRectangle(dpy, *visible_window, graphics_context, 0, 0, 64, 64); */
	XFillRectangle(dpy, *visible_window, graphics_context, GRAPHBOX_X, GRAPHBOX_Y, GRAPHBOX_WIDTH, GRAPHBOX_HEIGHT);


	XSetForeground(dpy, graphics_context, black_pixel);
	XFillRectangle(dpy, *visible_window, graphics_context, TOPBOX_X, TOPBOX_Y, TOPBOX_WIDTH, TOPBOX_HEIGHT);
	XDrawLine(dpy, *visible_window, graphics_context, GRAPHBOX_X_LEFT, GRAPHBOX_Y_TOP, GRAPHBOX_X_LEFT, GRAPHBOX_Y_BOTTOM);
	XDrawPoint(dpy, *visible_window, graphics_context, GRAPHBOX_X_RIGHT, GRAPHBOX_Y_TOP);
	if (label) {
		XSetForeground(dpy, graphics_context, labelbg_pixel);
		XFillRectangle(dpy, *visible_window, graphics_context, LABEL_X, LABEL_Y, LABEL_WIDTH, LABEL_HEIGHT);
		XSetClipRectangles(dpy, graphics_context, 0, 0, &cliprect, 1, Unsorted);
		XSetForeground(dpy, graphics_context, labelfg_pixel);
		XDrawString(dpy, *visible_window, graphics_context, 5, 58, label, strlen(label));
		XSetClipMask(dpy, graphics_context, None);
	}


	XSetForeground(dpy, graphics_context, white_pixel);
	XDrawLine(dpy, *visible_window, graphics_context, GRAPHBOX_X_RIGHT, GRAPHBOX_Y_BOTTOM, GRAPHBOX_X_RIGHT, (GRAPHBOX_Y_TOP + 1));
	XDrawLine(dpy, *visible_window, graphics_context, GRAPHBOX_X_LEFT, GRAPHBOX_Y_BOTTOM, GRAPHBOX_X_RIGHT, GRAPHBOX_Y_BOTTOM);


	XSetForeground(dpy, graphics_context, grey_pixel);
	XSetBackground(dpy, graphics_context, black_pixel);
	XCopyPlane(dpy, arrow, *visible_window, graphics_context, 7, 0, 7, 9, 53, 5, 1);
	XCopyPlane(dpy, arrow, *visible_window, graphics_context, 0, 0, 7, 9, 46, 5, 1);
}


/* Main loop that is called every delaytime.  This calls stat_gather() and updateSpeedometer() when needed
 * and takes care of the displaying and scrolling the graph */
void tock(void) {
	unsigned long since;
	int y, yy;
	unsigned long rate_rx, rate_tx;
	double percent_tx, percent_rx;
	/* static array containing the last 8 samples... for use in averaging and smoothing the graph a little */
	static unsigned long lifo_in[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned long lifo_out[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned int t = 0, blank = 0;

	if (gettimeofday(&timenow, NULL)) {
		perror("wmnet: gettimeofday()");
		exit(10);
	}
	since = (timenow.tv_sec * 1000000L + timenow.tv_usec) - (timelast.tv_sec * 1000000L + timelast.tv_usec);
	if (since > displayDelay) {
		lifo_in[t] = diffbytes_in * (1000000L / since);
		lifo_out[t] = diffbytes_out * (1000000L / since);
		t = (t + 1) % 8;



		/* in */
		rate_rx = (lifo_in[0] + lifo_in[1] + lifo_in[2] + lifo_in[3] + lifo_in[4] + lifo_in[5] + lifo_in[6] + lifo_in[7]) / (unsigned long)8;
		if(logscale) percent_rx = (log10(  ((rate_rx * 10000/ maxRate) < 1) ? 1 : ((double)rate_rx / (double)maxRate) * 10000.) / 4.);
		else percent_rx = (double)(rate_rx) / maxRate;
		y =  GRAPH_Y_BOTTOM - (GRAPH_HEIGHT * percent_rx) ;
		y = y < GRAPH_Y_UPPER ? GRAPH_Y_UPPER : y;

		/* out */
		rate_tx = (lifo_out[0] + lifo_out[1] + lifo_out[2] + lifo_out[3] + lifo_out[4] + lifo_out[5] + lifo_out[6] + lifo_out[7]) / (unsigned long)8;
		if(logscale) percent_tx = (log10(  ((rate_tx * 10000 / maxRate ) < 1) ? 1 : ((double)rate_tx / (double)maxRate * 10000.)) / 4.);
		else percent_tx = (double)(rate_tx) / maxRate;
		yy =  GRAPH_Y_UPPER + (GRAPH_HEIGHT * percent_tx) ;
		yy = yy > GRAPH_Y_BOTTOM ? GRAPH_Y_BOTTOM : yy;


		/* only update the speedometer every 7th displayDelay */
		if (t == 7) updateSpeedometer(rate_rx, rate_tx);

		/* blank var is just for stopping executing the X* funcs when the disp is all black */
		if ((y == GRAPH_Y_BOTTOM && yy == GRAPH_Y_UPPER) && (diffbytes_in + diffbytes_out) == 0) blank++; else blank = 0;
		if (blank < (GRAPH_WIDTH + 1) ) {
			XCopyArea(dpy, *visible_window, *visible_window, graphics_context, GRAPH_X + 1,
			   GRAPH_Y, GRAPH_WIDTH - 1, GRAPH_HEIGHT, GRAPH_X, GRAPH_Y);
			XSetForeground(dpy, graphics_context, darkgrey_pixel);
			XDrawLine(dpy, *visible_window, graphics_context, GRAPH_X_RIGHT, y, GRAPH_X_RIGHT, yy);
			if (( (yy == GRAPH_Y_UPPER && diffbytes_out > 0 && rate_rx > rate_tx) || (rate_rx >= rate_tx && yy != GRAPH_Y_UPPER)) ) {
				drawColoredLine(GRAPH_Y_UPPER, yy, tx_pixel);
			}
			if ( y != GRAPH_Y_BOTTOM || diffbytes_in > 0) {
				drawColoredLine(GRAPH_Y_BOTTOM, y, rx_pixel);
			}
			if (( (yy == GRAPH_Y_UPPER && diffbytes_out > 0) || (rate_rx < rate_tx && yy != GRAPH_Y_UPPER)) ) {
				drawColoredLine(GRAPH_Y_UPPER, yy, tx_pixel);
			}
		}


		diffbytes_in = diffbytes_out = 0;
		timelast = timenow;
	}

	if (!stat_gather()) {  /* Anything change? */
		current_rx = rx;
		current_tx = tx;
		XSetBackground(dpy, graphics_context, black_pixel);
		if(current_tx == True) {
			XSetForeground(dpy, graphics_context, tx_pixel[HIGH_INTENSITY]);
			XCopyPlane(dpy, arrow, *visible_window, graphics_context, 7, 0, 7, 9, 53, 5, 1);
			/* XFillRectangle(dpy, *visible_window, graphics_context, 55, 5, 4, 4);  */
		}
		else {
			XSetForeground(dpy, graphics_context, grey_pixel);
			XCopyPlane(dpy, arrow, *visible_window, graphics_context, 7, 0, 7, 9, 53, 5, 1);
		}
		if(current_rx == True) {
			XSetForeground(dpy, graphics_context, rx_pixel[HIGH_INTENSITY]);
			XCopyPlane(dpy, arrow, *visible_window, graphics_context, 0, 0, 7, 9, 46, 5, 1);
			/* XFillRectangle(dpy, *visible_window, graphics_context, 55, 12, 4, 4);  */
		}
		else {
			XSetForeground(dpy, graphics_context, grey_pixel);
			XCopyPlane(dpy, arrow, *visible_window, graphics_context, 0, 0, 7, 9, 46, 5, 1);
		}
	}

}

/* paints the speedometer area with whichever is greater, rxRate or txRate */
int updateSpeedometer(int rxRate, int txRate) {
	double rate;
	char astring[10];
	unsigned long color;
	static XRectangle cliprect = { 4, 5, 37, 8 };
	static int rxRate_last = 0 , txRate_last = 0;
	static Bool clear = True, collectandreturn = True;

	/* This is ugly, I don't like this, but it slows the speedometer down a touch */
	if (collectandreturn == True) {
		txRate_last = txRate;
		rxRate_last = rxRate;
		collectandreturn = False;
		return 1;
	}
	collectandreturn = True;



	if (txRate > rxRate) {
		rate = (txRate + txRate_last) / 2000.;
		color = tx_pixel[HIGH_INTENSITY];
	} else {
		rate = (rxRate + rxRate_last) / 2000.;
		color = rx_pixel[HIGH_INTENSITY];
	}


	if (!clear) {
		XSetForeground(dpy, graphics_context, black_pixel);
		XFillRectangle(dpy, *visible_window, graphics_context, 4, 5, 37, 9);
	}
	if (rate < .1) {
		clear = True;
		return(1);
	} else if (rate < 1.) {
		snprintf(astring, 10, "%db/s", (unsigned int)(rate * 1000.));
	} else if (rate >= 1. && rate < 10.)
		snprintf(astring, 10, "%1.2fk/s", rate);
	else if (rate >= 10. && rate < 100.)
		snprintf(astring, 10, "%2.1fk/s", rate);
	else if (rate >= 100. && rate < 1000.)
		snprintf(astring, 10, "%dk/s", (unsigned int)rate);
	else if (rate > 1000. && rate < 10000.)
		snprintf(astring, 10, "%1.2fM/s", (rate / 1000.));
	else if (rate > 10000. && rate < 100000.)
		snprintf(astring, 10, "%2.1fM/s", (rate / 1000.));
	else sprintf(astring, "XXXX");

	XSetForeground(dpy, graphics_context, color);
	XSetClipRectangles(dpy, graphics_context, 0, 0, &cliprect, 1, Unsorted);
	XDrawString(dpy, *visible_window, graphics_context, 4, 13, astring, strlen(astring));
	XSetClipMask(dpy, graphics_context, None);
	clear = False;
	return(0);

}


/* called from within tock to draw the shaded lines making up our bar-graph */
void drawColoredLine(int y1, int y2, unsigned long *shadecolor) {
	int subline[4], i;
	static unsigned int linebreaks[3] = { 50, 90, 100 };
	subline[0] = y1;
	for(i = 0; i < 3; i++) {
		if (y1 > y2) subline[i+1] = y1 - (((y1 - y2) * linebreaks[i]) / 100);
		else subline[i+1] = y1 + (((y2 - y1) * linebreaks[i]) / 100);
		XSetForeground(dpy, graphics_context, shadecolor[i]);
		XDrawLine(dpy, *visible_window, graphics_context, GRAPH_X_RIGHT, subline[i], GRAPH_X_RIGHT, subline[i+1]);
	}
}


/* Returns in returnarray a 3 value array containing 3 shades (low, normal, and high) of XColor shade.
 * Called from setup_wmnet on startup and never called again.
 */
void shadesOf(XColor *shade, unsigned long *returnarray) {
	XAllocColor(dpy, DefaultColormap(dpy, screen), shade);
	returnarray[HIGH_INTENSITY] = shade->pixel;

	shade->red = (8 * shade->red / 10);
	shade->green = (8 * shade->green / 10);
	shade->blue = (8 * shade->blue / 10);
	XAllocColor(dpy, DefaultColormap(dpy, screen), shade);
	returnarray[NORMAL_INTENSITY] = shade->pixel;

	shade->red = 8 * shade->red / 10;
	shade->green = 8 * shade->green / 10;
	shade->blue = 8 * shade->blue / 10;
	XAllocColor(dpy, DefaultColormap(dpy, screen), shade);
	returnarray[LOW_INTENSITY] = shade->pixel;
}


/* Here is main, clear at the bottom.  Handles the event loop and calls tock() every delayTime milliseconds */
int main(int argc, char ** argv) {
	unsigned int done = False;
	XEvent event;

	setup_wmnet(argc, argv);

	while(!done) {
		while(XPending(dpy)) {
			XNextEvent(dpy, &event);
			switch(event.type) {
			case Expose:
				redraw((XExposeEvent *)&event);
				break;
			case ClientMessage:
				if(event.xclient.data.l[0] == delete_atom)
					done = True;
				break;
			case ButtonPress:
				if(event.xbutton.button == Button1 && click_command != NULL) {
					if (fork() == 0) {
						execl("/bin/sh", "sh", "-c", click_command, NULL);
						perror("wmnet: execl()");
						exit(15);
					}
				}
				break;
			}
		}

	       	/* Wait for a bit, updating is done in tock() */
		usleep(delayTime);
		tock();
	}
	return 0;
}

