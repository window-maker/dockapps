/* wmwebcam - modified vidcat - read the README file for more info */

/*
 * vidcat.c
 *
 * Copyright (C) 1998 - 2000 Rasca, Berlin
 * EMail: thron@gmx.de
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/videodev.h>

#include <jpeglib.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "wmwebcam-mask.xbm"
#include "minirgb.h"


//////////////////////// CHANGE THESE IF NECESSARY ///////////////////////////

#define DEF_WIDTH	352         // changing these requires adjustements
#define DEF_HEIGHT	288         // to the source, use default if possible

#define SENDINGDELAY	60          // default delay between saving jpeg
                                    // images and scriptrunning (in seconds)

#define OUTPUTFILE	"/tmp/wmwebcam.jpg"     // default output file
#define CUSTOMSCRIPT    "wmwebcam.pl"           // default custom script
#define QUAL_DEFAULT	100                     // default jpeg outputquality

//////////////////////////////////////////////////////////////////////////////

char *basename (const char *s);

Display *display;
Window win;
Window iconwin;
GC gc;
MiniRGB ui;                             /* 64x64 main window buffer */
MiniRGB draw;                           /* buffer with images etc */

void new_window(char *name, char *mask_data)
{
    Pixel fg, bg;
    XGCValues gcval;
    XSizeHints sizehints;
    XClassHint classhint;
    XWMHints wmhints;
    int screen;
    Window root;
    Pixmap mask;
    screen = DefaultScreen(display);
    root = DefaultRootWindow(display);

    sizehints.flags = USSize;
    sizehints.width = 64;
    sizehints.height = 64;

    fg = BlackPixel(display, screen);
    bg = WhitePixel(display, screen);

    win = XCreateSimpleWindow(display, root,
                                       0, 0, sizehints.width,
                                       sizehints.height, 1, fg, bg);
    iconwin =
        XCreateSimpleWindow(display, win, 0, 0,
                            sizehints.width, sizehints.height, 1, fg, bg);

    XSetWMNormalHints(display, win, &sizehints);
    classhint.res_name = name;
    classhint.res_class = name;
    XSetClassHint(display, win, &classhint);

    XSelectInput(display, win,
                 ExposureMask | ButtonPressMask | ButtonReleaseMask |
                 StructureNotifyMask);
    XSelectInput(display, iconwin,
                 ExposureMask | ButtonPressMask | ButtonReleaseMask |
                 StructureNotifyMask);

    XStoreName(display, win, name);
    XSetIconName(display, win, name);

    gcval.foreground = fg;
    gcval.background = bg;
    gcval.graphics_exposures = False;
    gc =
        XCreateGC(display, win,
                  GCForeground | GCBackground | GCGraphicsExposures,
                  &gcval);

    mask = XCreateBitmapFromData(display, win, mask_data, 128, 64);

    XShapeCombineMask(display, win, ShapeBounding, 0, 0,
                      mask, ShapeSet);
    XShapeCombineMask(display, iconwin, ShapeBounding, 0,
                      0, mask, ShapeSet);

    wmhints.initial_state = WithdrawnState;
    wmhints.flags = StateHint;
    wmhints.icon_window = iconwin;
    wmhints.icon_x = sizehints.x;
    wmhints.icon_y = sizehints.y;
    wmhints.window_group = win;
    wmhints.flags =
        StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
    XSetWMHints(display, win, &wmhints);

    XMapWindow(display, win);
}

void redraw_window(void)
{
    minirgb_draw(win, gc, 0, 0, 64, 64, &ui);
    minirgb_draw(iconwin, gc, 0, 0, 64, 64, &ui);
}

#define copyXPMArea(x, y, w, h, dx, dy) minirgb_copy(&draw, &ui, x, y, w, h, dx, dy)


char *
get_image (int dev, int width, int height,int *size)
{
	struct video_capability vid_caps;
	struct video_mbuf vid_buf;
	struct video_mmap vid_mmap;
	char *map;
	int len;

	if (ioctl (dev, VIDIOCGCAP, &vid_caps) == -1) {
		perror ("ioctl (VIDIOCGCAP)");
		return (NULL);
	}

	if (ioctl (dev, VIDIOCGMBUF, &vid_buf) == -1) {
		struct video_window vid_win;

		if (ioctl (dev, VIDIOCGWIN, &vid_win) != -1) {
			vid_win.width  = width;
			vid_win.height = height;
			if (ioctl (dev, VIDIOCSWIN, &vid_win) == -1)
				return (NULL);
		}

		map = malloc (width * height * 3);
		len = read (dev, map, width * height * 3);
		if (len <=  0) { free (map); return (NULL); }
		*size = 0;
		return (map);
	}

	map = mmap (0, vid_buf.size, PROT_READ|PROT_WRITE,MAP_SHARED,dev,0);
	if ((unsigned char *)-1 == (unsigned char *)map) {
		perror ("mmap()");
		return (NULL);
	}

	vid_mmap.format = VIDEO_PALETTE_RGB24;
	vid_mmap.frame = 0;
	vid_mmap.width = width;
	vid_mmap.height = height;
	if (ioctl (dev, VIDIOCMCAPTURE, &vid_mmap) == -1) {
		perror ("VIDIOCMCAPTURE");
		munmap (map, vid_buf.size);
		return (NULL);
	}
	if (ioctl (dev, VIDIOCSYNC, &vid_mmap) == -1) {
		perror ("VIDIOCSYNC");
		munmap (map, vid_buf.size);
		return (NULL);
	}
	*size = vid_buf.size;
	return (map);
}

void
put_image_jpeg (char *image, int width, int height, int quality)
{

	FILE *output;
        int y, x, line_width;
        JSAMPROW row_ptr[1];
        struct jpeg_compress_struct cjpeg;
        struct jpeg_error_mgr jerr;
        char *line;

	output = fopen("/tmp/wmwebcam.jpg","w");

	if(!output) return;

        line = malloc (width * 3);
        if (!line)
                return;
        cjpeg.err = jpeg_std_error(&jerr);
        jpeg_create_compress (&cjpeg);
        cjpeg.image_width = width;
        cjpeg.image_height= height;
        cjpeg.input_components = 3;
        cjpeg.in_color_space = JCS_RGB;
        jpeg_set_defaults (&cjpeg);

        jpeg_set_quality (&cjpeg, quality, TRUE);
        cjpeg.dct_method = JDCT_FASTEST;
        jpeg_stdio_dest (&cjpeg, output);

        jpeg_start_compress (&cjpeg, TRUE);

        row_ptr[0] = line;
        line_width = width * 3;
        for ( y = 0; y < height; y++) {
        for (x = 0; x < line_width; x+=3) {
                        line[x]   = image[x+2];
                        line[x+1] = image[x+1];
                        line[x+2] = image[x];
                }
                jpeg_write_scanlines (&cjpeg, row_ptr, 1);
                image += line_width;
        }
        jpeg_finish_compress (&cjpeg);
        jpeg_destroy_compress (&cjpeg);
        free (line);
	fclose (output);
}

void
put_image (char *image, int width, int height)
{

        int x, y, r,g,b, c;
        unsigned char *p = (unsigned char *)image;
        unsigned char *buf = (unsigned char *)image;

        for (y = 0; y < 288; y++ ) {
		c=0;
                for (x = 0; x < 348; x++ ) {
			r = buf[0]; g = buf[1]; b = buf[2]; buf += 3;
			c++;
			if (c == 6) {
				c = 0;
				*p++ = b; *p++ = g; *p++ = r;
			}
                }
        }

        memcpy(draw.mem, image, 352*288*3);
}

int
main (int argc, char *argv[])
{
    XEvent Event;
	char *image, *device = VIDEO_DEV;

	int delay = SENDINGDELAY;
	int width = DEF_WIDTH, height = DEF_HEIGHT, size, dev = -1;
	int max_try = 5;
        int quality = QUAL_DEFAULT;

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open default display\n");
        exit(1);
    }

    if (minirgb_init(display)) {
        fprintf(stderr, "minirgb init failed!\n");
        exit(1);
    }

    new_window("wmwebcam", wmwebcam_mask_bits);
    minirgb_new(&ui, 64, 64);
    minirgb_new(&draw, 352, 288);
    copyXPMArea(0, 0, 64, 64, 0, 0);

while (1) {
	while (max_try) {
		dev = open (device, O_RDWR);
		if (dev == -1) {
			if (!--max_try) {
				fprintf (stderr, "Can't open device %s\n", VIDEO_DEV);
				exit (0);
			}
			sleep (1);
		} else {
			break;
		}
	}
	image = get_image (dev, width, height, &size);
	if (!size) close (dev);

        if (image) {

	if (delay == SENDINGDELAY) { put_image_jpeg(image, width, height, quality); }

	put_image(image, width, height);

        copyXPMArea(1, 1, 54, 47, 5, 5);

        while (XPending(display)) {
            XNextEvent(display, &Event);
            switch (Event.type) {
            case Expose:
                redraw_window();
                break;
            case DestroyNotify:
                XCloseDisplay(display);
                exit(0);
                break;
            }
        }
        redraw_window();

		if (size) {
			munmap (image, size);
			close (dev);
		} else if (image) {
			free (image);
		}

	// close device first before starting to send the image

	if (delay == SENDINGDELAY) { system (CUSTOMSCRIPT); delay = 0; }

	delay++;
	sleep (1);

	} else {
		fprintf (stderr, "Error: Can't get image\n"); exit(0);
	}
}
	return (0);
}

