/*
 * main.c
 * Copyright (C) Renan Vedovato Traba 2012 <rvt10@inf.ufpr.br>
 *
  		ButtonMaker is free software: you can redistribute it and/or modify it
        under the terms of the GNU General Public License as published by the
        Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        ButtonMaker is distributed in the hope that it will be useful, but
        WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
        See the GNU General Public License for more details.

        You should have received a copy of the GNU General Public License along
        with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <Imlib2.h>

#include "wmgeneral.h"
#include "pixmaps.h"

char *cmd, *fname;
int isize=50;

/* Prototypes */
static void print_usage(char *);
static void ParseCMDLine(int argc, char *argv[]);

static void print_usage(char * pname)
{
    printf("%s version: %s\n", pname, VERSION);
    printf("By Renan Vedovato Traba <rvt10@inf.ufpr.br>\n\n");
    printf("\t-c, --command [CMD]\t which command will be called \n");
    printf("\t-i, --icon [ICON]\t icon displayed\n");
    printf("\t-s, --size [SIZE]\t size of the icon\n");
    printf("\tdouble click to run CMD\n");
    printf("\tright click to exit\n");
    printf("\neg:\n%s -c sudo apt-get update -i TerminalGNUstep.tiff -s 32\n", pname);
}

static void ParseCMDLine(int argc, char *argv[])
{
    int i, j, k, size=2;
    for (i=1; i < argc; i++) {
	if (!strcmp(argv[i], "-display")) {
	    i++;
	} else if ( ((!strcmp(argv[i], "--icon")) || (!strcmp(argv[i], "-i"))) && (i+1 < argc) ) {
	    fname = argv[++i];
	} else if ( ((!strcmp(argv[i], "--size")) || (!strcmp(argv[i], "-s"))) && (i+1 < argc) ) {
	    isize = atoi(argv[++i]);
	} else if ( ((!strcmp(argv[i], "--command")) || (!strcmp(argv[i], "-c"))) && (i+1 < argc) ) {
	    for (j=i+1; j < argc; j++) {
		if (argv[j][0] == '-') {
		    break;
		} else  {
		    size+=1+strlen(argv[j]);
		}
	    }
	    cmd = (char *) malloc((size)*sizeof(char));
	    for (k=i+1; k < j; k++) {
		strcat(cmd, argv[k]);
		strcat(cmd, " ");
	    }
	    strcat(cmd, "&");
	    i = j-1;
	} else {
	    print_usage(argv[0]);
	    exit(1);
	}
    }
    if ((argc < 3) || (!cmd)) {
	print_usage(argv[0]);
	exit(1);
    }
}

int main(int argc, char *argv[])
{
    int b;

    extern int d_depth;
    extern Window iconwin, win;
    Visual *visual;
    XEvent event;
    Pixmap xpm;
    Time lastTime=0;

    Imlib_Image image;

    ParseCMDLine(argc, argv);
    openXwindow(argc, argv, xpm_master, xpm_mask_bits, xpm_mask_width, xpm_mask_height);

    xpm = XCreatePixmap(display, win, 64, 64, d_depth);
    XFillRectangle(display, xpm, NormalGC, 0, 0, 64, 64);

    if (fname) {
	visual = DefaultVisual(display, DefaultScreen(display));

	imlib_context_set_dither(1);
	imlib_context_set_display(display);
	imlib_context_set_visual(visual);

	image = imlib_load_image(fname);
	imlib_context_set_image(image);
	imlib_context_set_drawable(xpm);
	imlib_render_image_on_drawable_at_size(0, 0, isize, isize);
    }
    b = (64-isize)/2;
    /* Loop Forever */
    while (1) {
	/* Process any pending X events. */
	while (XPending(display)) {
	    XNextEvent(display, &event);
	    switch (event.type) {
		case Expose:
		    RedrawWindow();
		    XCopyArea(display, xpm, iconwin, NormalGC, 0, 0, isize, isize, b, b);
		    break;
		case MotionNotify:
		    break;
		case ButtonPress:
		    /*printf("ButtonPress\n");*/
		    break;
		case ButtonRelease:
		    if (event.xbutton.button == Button1) {
			if (event.xbutton.time - lastTime < 250) {
			    if (system(cmd) == -1) {
				fprintf(stdout, "Failed to run command:%s\n", cmd);
				exit(0);
			    }
			} else {
			    lastTime = event.xbutton.time;
			}
		    } else if (event.xbutton.button == Button3) {
			exit(0);
		    }
		    /*printf("ButtonRelease\n");*/
		    break;
	    }
	}
	usleep(10000);
    }
    /* we should never get here */
    return (0);
}
