/* minirgb -- simple rgb buffer drawing library for 16, 24 and 32 bit displays
 * Copyright (C) 2000 timecop@japan.co.jp
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
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "minirgb.h"

#undef  MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/* local variables */
Display	*minirgb_display;			/* display */
Visual	*minirgb_visual;			/* visual */
int	minirgb_depth;				/* display depth */
int	minirgb_ok = 0;				/* init ok? */

/* local prototypes */
static __inline void rgb_conv_16(unsigned char *ptr, unsigned char r, unsigned char g, unsigned char b, XImage *image);
static __inline void rgb_conv_24(unsigned char *ptr, unsigned char r, unsigned char g, unsigned char b, XImage *image);
static __inline void rgb_conv_32(unsigned char *ptr, unsigned char r, unsigned char g, unsigned char b, XImage *image);

static __inline void rgb_conv_16(unsigned char *ptr, unsigned char r, unsigned char g, unsigned char b, XImage *image)
{
    if (image->green_mask & 0x7e0) /* 565 */
	*(unsigned short int *)ptr = ((r >> 3) << 11) |
	    			     ((g >> 2) << 5) |
				     (b >> 3);
    else /* 555 untested */
	*(unsigned short int *)ptr = ((r >> 3) << 11) |
	    			     ((g >> 3) << 6) |
				     (b >> 3);
}

static __inline void rgb_conv_24(unsigned char *ptr, unsigned char r, unsigned char g, unsigned char b, XImage *image)
{
    /* untested */
    *ptr++ = r;
    *ptr++ = g;
    *ptr = b;
}

static __inline void rgb_conv_32(unsigned char *ptr, unsigned char r, unsigned char g, unsigned char b, XImage *image)
{
    if (image->red_mask & 0xff0000)  /* 0888 */
	*(unsigned int *)ptr = (r << 16) |
	    		       (g << 8) |
			       (b);
    else /* 8880 untested */
	*(unsigned int *)ptr = (r << 24) |
	    		       (g << 16) |
			       (b << 8);
}

int minirgb_init(Display *d)
{
    int screen;

    minirgb_display = d;
    screen = DefaultScreen(minirgb_display);
    minirgb_visual = XDefaultVisual(minirgb_display, screen);
    minirgb_depth = XDefaultDepth(minirgb_display, screen);
    if (minirgb_depth <= 8) {
	fprintf(stderr, "minirgb: No support for 8 bit displays\n");
	minirgb_ok = 0;
	return 1;
    } else {
	minirgb_ok = 1;
	return 0;
    }
}

void minirgb_setpixel(MiniRGB *rgb_image, unsigned char *color, unsigned int x, unsigned int y)
{
    unsigned char *ptr;

    ptr = rgb_image->mem + (y * rgb_image->rowstride * 3) + (x * 3);
    *ptr++ = *color++;
    *ptr++ = *color++;
    *ptr = *color;
}

void minirgb_copy(MiniRGB *from, MiniRGB *to,
	unsigned int sx, unsigned int sy, unsigned int w, unsigned int h,
	unsigned int dx, unsigned int dy)
{
    unsigned int i;
    unsigned char *src, *dest;

#ifdef VERBOSE
    fprintf(stderr, "minirgb: copy %d, %d (%dx%d) -> %d, %d)\n", sx, sy, w, h, dx, dy);
#endif

#if 0
    if (w == 1) { /* special case - dot operations */
	src = from->mem + (sy * from->rowstride * 3) + sx * 3;
	dest = to->mem + (dy * to->rowstride * 3) + dx * 3;
	dest[0] = dest[0] & src[0];
	dest[1] = dest[1] & src[1];
	dest[2] = dest[2] & src[2];
	return;
    }
#endif

    for (i = 0; i < h; i++) {
	src = from->mem + ((sy + i) * from->rowstride * 3) + sx * 3;
	dest = to->mem + ((dy + i) * to->rowstride * 3) + dx * 3;
	memcpy(dest, src, w * 3);
    }
}

void minirgb_new(MiniRGB *image, unsigned int width, unsigned int height)
{
    image->width = width;
    image->height = height;
    image->rowstride = width;
    image->mem = calloc(1, width * height * 3);
}

void minirgb_draw(Window drawable, GC gc, int x, int y, int width,
		       int height, MiniRGB *rgb_image)
{
    XImage *image = NULL;
    unsigned int bpp;
    unsigned int x0, y0;
    unsigned char *ptr, *col;
    void (*conv) (unsigned char *, unsigned char, unsigned char, unsigned char, XImage *) = NULL;

    if (!minirgb_ok)
	return;

    image = XCreateImage(minirgb_display, minirgb_visual, minirgb_depth, ZPixmap, 0, 0, width, height, 32, 0);
    bpp = (image->bits_per_pixel + 7) / 8;
    switch (bpp) {
	case 2:
	    conv = rgb_conv_16;
	    break;
	case 3:
	    conv = rgb_conv_24;
	    break;
	case 4:
	    conv = rgb_conv_32;
	    break;
    }
#ifdef VERBOSE
    printf("minirgb: image %p %dx%d (bpp: %d, bpl: %d)\n", image, width, height, bpp, image->bytes_per_line);
#endif
    image->data = malloc(image->bytes_per_line * height);
    if (!image->data) {
	fprintf(stderr, "minirgb: allocation error\n");
	XDestroyImage(image);
	return;
    }

    for (y0 = 0; y0 < height; y0++) {
	for (x0 = 0; x0 < width; x0++) {
	    col = rgb_image->mem + (y0 * rgb_image->rowstride * 3 + x0 * 3);
	    ptr = image->data + (y0 * image->bytes_per_line + x0 * bpp);
	    conv(ptr, col[0], col[1], col[2], image);
	}
    }
    /* draw image onto drawable */
    XPutImage(minirgb_display, drawable, gc, image, 0, 0, x, y, width, height);
    XDestroyImage(image);
}
