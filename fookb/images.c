/*
 * images.c
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 */

#include <stdio.h>		/* puts() */
#include <stdlib.h>
#include <string.h>

/* Xpm fun */
#include <X11/xpm.h>

//#include "fookb.h"
#include "params.h"
#include "images.h"

/* Let us make lint happy! */
#define lputs(x) (void)(puts(x))

static int w = 0;
static int h = 0;

static XImage *stupid_picture[5];	/* Icons for fookb */

int get_width() {
	return w;
}

int get_height() {
	return h;
}

static int get_one_image(char *name, int index, Display *dpy)
{
	int foo;

	foo = XpmReadFileToImage(dpy, name, &stupid_picture[index],
				 NULL, NULL);

	if (foo < 0)
		return foo;

	if (0 == w) {
		w = stupid_picture[index]->width;
		if (0 == w) {
			lputs("FATAL: Icon1 has zero width!");
			exit(EXIT_FAILURE);
		}
		if (w > 64) {
			lputs("Warning: Icon width is more than 64. Strange things may happen if using Window Maker.");
		}
	}

	if (0 == h) {
		h = stupid_picture[index]->height;
		if (0 == h) {
			lputs("FATAL: Icon1 had zero height!");
			exit(EXIT_FAILURE);
		}
		if (h > 64) {
			lputs("Warning: Icon height is more than 64. Strange things may happen if using Window Maker.");
		}
	}

	if (w != stupid_picture[index]->width) {
		lputs("FATAL: Not all icons are of the same width!");
		exit(EXIT_FAILURE);
	}

	if (h != stupid_picture[index]->height) {
		lputs("FATAL: Not all iconse are of the same height!");
		exit(EXIT_FAILURE);
	}
	return (foo);
}

void read_images(Display *dpy)
{
	int i;
	int res;
	int status = 0;

	for (i = 0; i < 5; i++) {

		switch (i) {
		case 0:
			res = get_one_image(read_param("Icon1"), 0, dpy);
			break;
		case 1:
			res = get_one_image(read_param("Icon2"), 1, dpy);
			break;
		case 2:
			res = get_one_image(read_param("Icon3"), 2, dpy);
			break;
		case 3:
			res = get_one_image(read_param("Icon4"), 3, dpy);
			break;
		default:
			res = get_one_image(read_param("IconBoom"), 4, dpy);
			break;
		}

		switch (res) {
		case XpmOpenFailed:
			lputs("Xpm file open failed:");
			status = 1 << 5;
			break;
		case XpmFileInvalid:
			lputs("Xpm file is invalid:");
			status = 1 << 6;
			break;
		case XpmNoMemory:
			lputs("No memory for open xpm file:");
			status = 1 << 7;
			break;
		default:
			break;
		}

		if (!(status == 0)) {
			status += 1 << i;
			switch (i) {
			case 0:
				lputs(read_param("Icon1"));
				break;
			case 1:
				lputs(read_param("Icon2"));
				break;
			case 2:
				lputs(read_param("Icon3"));
				break;
			case 3:
				lputs(read_param("Icon4"));
				break;
			case 4:
				lputs(read_param("IconBoom"));
				break;
			default:
				lputs("UNKNOWN ERROR! PLEASE REPORT!!!");
				exit(-2);
			}
			exit(status);
		}
	}

}

void update_window(Drawable win, GC gc, unsigned int whattodo, Display *dpy)
{
	int err;

	err = XPutImage(dpy, win, gc, stupid_picture[whattodo],
			0, 0, 0, 0, w, h);

	if (0 == err) return;

	switch (err) {
		case BadDrawable:
			lputs("Fatal error, XPutImage returns BadDrawable. "
					"Please report!");
			break;
		case BadGC:
			lputs("Fatal error, XPutImage returns BadGC. "
					"Please report!");
			break;
		case BadMatch:
			lputs("Fatal error, XPutImage returns BadMatch. "
					"Please report!");
			break;
		case BadValue:
			lputs("Fatal error, XPutImage returns BadValue. "
					"Please report!");
			break;
		default:
			lputs("Fatal error, XPutImage returns unknown error. "
					"Please report, but probably "
					"it is a bug in X.");
			break;
	}
}
