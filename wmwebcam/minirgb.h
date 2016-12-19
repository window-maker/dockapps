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
#ifndef _MINIRGB_H_
#define _MINIRGB_H_
#include <X11/Xlib.h>

typedef struct _MiniRGB MiniRGB;

struct _MiniRGB {
    unsigned char *mem;				/* rgb buffer */
    unsigned int width;				/* buffer width */
    unsigned int height;			/* buffer height */
    unsigned int rowstride;			/* row stride */
};

void minirgb_draw(Window drawable,
		  GC gc,
		  int x,
		  int y,
		  int width,
		  int height,
		  MiniRGB *rgb_image);

int minirgb_init (Display *display);

void minirgb_setpixel(MiniRGB *rgb_image,
		      unsigned char *color,
		      unsigned int x,
		      unsigned int y);

void minirgb_copy    (MiniRGB *from,
		      MiniRGB *to,
		      unsigned int x,
		      unsigned int y,
		      unsigned int w,
		      unsigned int h,
		      unsigned int dx,
		      unsigned int dy);

void minirgb_new     (MiniRGB *image,
		      unsigned int width,
		      unsigned int height);

#endif /* !_MINIRGB_H_ */
