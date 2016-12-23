/*
 * Copyright (C) 12 Jun 2003 Tomas Cermak
 *
 * This file is part of wmradio program.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <getopt.h>
#include "config.h"
#include "wm_envelope.h"
#include "wmradio.h"
#include "skin.h"
#include "rc.h"

Display *main_display = 0;
Window root, applet, icon, buffer;

GC NormalGC;
unsigned long gcm;
XGCValues gcv;

char radio_continue;

Atom wm_delete_window_atom, wm_protocol_atom;

Pixel GetColor(char *ColorName, Display * disp, Window win)
{
    XColor Color;
    XWindowAttributes Attributes;

    XGetWindowAttributes(disp, win, &Attributes);
    Color.pixel = 0;

    if (!XParseColor(disp, Attributes.colormap, ColorName, &Color))
        printf("wmradio: can't parse %s\n", ColorName);
    else if (!XAllocColor(disp, Attributes.colormap, &Color))
        printf("wmradio: can't allocate %s\n", ColorName);

    return Color.pixel;
}

int parse_command_line(int argc, char *argv [],RadioInfo *info)
{
    static struct option long_options[] = {
        {"dont-quit-mode",0,0,0},           /* 0 */
        {"start-muted",0,0,0},
	{"help",0,0,0},
	{"version",0,0,0},

	{"osd",0,0,0},
	{"osd-font",1,0,0},            /* 5 */
	{"osd-color",1,0,0},
	{"osd-position",1,0,0},
	{"osd-shadow-offset",1,0,0},
	{"osd-timeout",1,0,0},

	{"skin",1,0,0},

	{0,0,0,0}
    };
    int option_index = 0;
    int opt;
    int x,y,offset,timeout;

    while(1) {
        opt = getopt_long(argc,argv,"qmhvof:c:p:s:t:k:",long_options, &option_index);
	if( opt == -1 ) break;
	if( opt == 0 ) opt = option_index;
 	switch(opt) {
	case 0:
	case 'q':
	    info->dont_quit_mode = 1;
	    rc_set_variable_as_int(SECTION_CONFIG,"dont-quit-mode",1);
	    break;
	case 1:
        case 'm':
	    rc_set_variable_as_int(SECTION_CONFIG,"start-muted",1);
	    break;
	case 2:
	case 'h':
	    printf("wmradio [options]\n"
		   "  options are:\n"
		   "  -h|--help              print this help\n"
		   "  -m|--start-muted       program starts, but doesn't open radio device\n"
		   "  -q|--dont-quit-mode    program doesn't quit, just close radio device\n"
		   "  -v|--version           print version and quit\n"
		   "  -o|--osd               use osd\n"
		   "  -f|--osd-font font     osd font\n"
		   "  -c|--osd-color         font color\n"
		   "  -p|--osd-position      display position (-p 10x30 for example)\n"
		   "  -s|--osd-shadow-offset shadow offset\n"
		   "  -t|--osd-timeout       osd timeout\n"
		   "  -k|--skin              skin\n"
		   );
	    return 0;
	case 3:
	case 'v':
	    printf("This is %s %s\n", PACKAGE,VERSION);
	    return 0;
	case 4:
	case 'o':
	    rc_set_variable_as_int(SECTION_CONFIG,"osd",1);
	    break;
	case 5:
	case 'f':
	    rc_set_variable(SECTION_CONFIG,"osd-font",optarg);
	    break;
	case 6:
	case 'c':
	    rc_set_variable(SECTION_CONFIG,"osd-color",optarg);
	    break;
	case 7:
	case 'p':
	    if (sscanf(optarg,"%ix%i", &x, &y) < 2) {
		fprintf(stderr, "%s: incorrect syntax in OSD position\n", argv[0]);
	    } else {
		rc_set_variable_as_int(SECTION_CONFIG,"osd-position",x);
		/* rc_set_variable_as_int(SECTION_CONFIG,"osd-position",y); */
	    }
	    break;
	case 8:
	case 's':
	    if (sscanf(optarg,"%i", &offset) < 1) {
		fprintf(stderr, "%s: incorrect syntax in OSD shadow offset\n", argv[0]);
	    } else {
		rc_set_variable_as_int(SECTION_CONFIG,"osd-shadow-offset",offset);
	    }
	    break;
	case 9:
	case 't':
	    if (sscanf(optarg,"%i", &timeout) < 1) {
		fprintf(stderr, "%s: incorrect syntax in OSD timeout\n", argv[0]);
	    } else {
		rc_set_variable_as_int(SECTION_CONFIG,"osd-timeout",timeout);
	    }
	    break;
	case 10:
	case 'k':
	    rc_set_variable(SECTION_CONFIG,"skin",optarg);
	    break;
	}
    }
    return 1;
}

void video_mainloop(void)
{
    XEvent xe;
    RadioEvent re;

    radio_continue = 1;
    while (radio_continue) {
        /* X Events */
        while (XPending(main_display)) {
            XNextEvent(main_display, &xe);
            switch (xe.type) {
            case Expose:
		re.type = REVENT_EXPOSE;
		wmradio_handle_event(&re);
                break;
	    case ClientMessage:
	        if(xe.xclient.message_type == wm_protocol_atom) {
		    Atom a = (xe.xclient.data.l)[0];
		    if( a == wm_delete_window_atom ) {
			re.type = REVENT_QUIT;
			printf("quit\n");
			wmradio_handle_event(&re);
		    }
		}
		break;
            case DestroyNotify:
		re.type = REVENT_QUIT;
		wmradio_handle_event(&re);
		printf("quit\n");
                return;
            case ButtonPress:
 	        if(xe.xbutton.button < 4) {
		    re.type = REVENT_BUTTON_PRESS;
		    if(xe.xbutton.button == 4) re.type = REVENT_SCROLL_UP;
		    if(xe.xbutton.button == 5) re.type = REVENT_SCROLL_DOWN;
		    re.x = xe.xbutton.x;
		    re.y = xe.xbutton.y;
		    re.button = xe.xbutton.button;
		    re.control = xe.xbutton.state & ControlMask ?
		        CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
		    re.shift = xe.xbutton.state & ShiftMask ?
		        CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
		    wmradio_handle_event(&re);
		}
                break;
            case ButtonRelease:
		re.type = REVENT_BUTTON_RELEASE;
		re.x = xe.xbutton.x;
		re.y = xe.xbutton.y;
		re.button = xe.xbutton.button;
		re.control = xe.xbutton.state & ControlMask ?
		    CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
		re.shift = xe.xbutton.state & ShiftMask ?
		    CONTROL_STATE_PRESSED : CONTROL_STATE_NOT_PRESSED;
 	        if(re.button == 4) re.type = REVENT_SCROLL_UP;
 	        if(re.button == 5) re.type = REVENT_SCROLL_DOWN;
		wmradio_handle_event(&re);
                break;
	    }
        } /* XPending */
	usleep(100000);
	re.type = REVENT_TIMER;
	wmradio_handle_event(&re);
    }
    XCloseDisplay(main_display);
}

void video_close(void)
{
    radio_continue = 0;
}

void video_draw(float freq,int stereo)
{
    skin_to_window(main_display,buffer, NormalGC,freq,stereo);
    XCopyArea(main_display,buffer, icon, NormalGC,0,0,skin_width(),skin_height(),0,0);
    XCopyArea(main_display,buffer, applet, NormalGC,0,0,skin_width(),skin_height(),0,0);
}

int main(int argc, char *argv [])
{
    Pixel foreground,background;
    XWMHints WmHints;
    Status status;
    XClassHint classhint;
    XTextProperty title;
    int screen;
    char * appletname = "WmRadio";

    wmradio_init_radio_info();
    rc_read_config();
    parse_command_line(argc,argv,wmradio_radio_info());

    main_display = XOpenDisplay(NULL);
    if (!main_display) {
        printf("wmradio: can't open display %s.\n",XDisplayName(NULL));
        return 0;
    }
    screen = DefaultScreen(main_display);
    root = RootWindow(main_display,screen);

    background = GetColor("black", main_display, root);
    foreground = GetColor("white", main_display, root);
    create_skin(rc_get_variable(SECTION_CONFIG,"skin","default.skin"),main_display,root);
    wmradio_init();
    applet = XCreateSimpleWindow(main_display,
				 root,
				 0,0,skin_width(),skin_height(),
				 0,
				 foreground,background);
    icon   = XCreateSimpleWindow(main_display,
				 root,
				 0,0,skin_width(),skin_height(),
				 0,
				 foreground,background);
    WmHints.flags = StateHint | IconWindowHint;
    WmHints.initial_state = WithdrawnState;
    WmHints.icon_window = icon;
    WmHints.window_group = applet;
    WmHints.flags |= WindowGroupHint;

    XSetWMHints(main_display,
		applet,
		&WmHints);

    buffer = XCreatePixmap(main_display,
			   root,
			   skin_width(),skin_height(),
			   DefaultDepth(main_display,screen)/*16  color_depth */);

    status = XStringListToTextProperty(&appletname, 1, &title);
    XSetWMName(main_display, applet, &title);
    XSetWMName(main_display, icon, &title);
    classhint.res_name = "wmradio" ;
    classhint.res_class = "WMRADIO";
    XSetClassHint(main_display, applet, &classhint);
    XStoreName(main_display, applet, "WmRadio");
    XSetIconName(main_display, applet, "WmRadio");

    wm_delete_window_atom = XInternAtom(main_display, "WM_DELETE_WINDOW", 0);
    wm_protocol_atom = XInternAtom(main_display, "WM_PROTOCOLS", 0);
    XSetWMProtocols(main_display, applet, &wm_delete_window_atom, 1);

    status = XMapWindow(main_display, applet);
    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = foreground;
    gcv.background = background;
    gcv.graphics_exposures = 0;
    NormalGC = XCreateGC(main_display, root, gcm, &gcv);
    XSelectInput(main_display, applet,
		 ButtonPressMask | ExposureMask |
		 ButtonReleaseMask | PointerMotionMask |
		 StructureNotifyMask);
    XSelectInput(main_display, icon,
		 ButtonPressMask | ExposureMask |
		 ButtonReleaseMask | PointerMotionMask |
		 StructureNotifyMask);
    video_mainloop();
    wmradio_done();
    return 0;
}

