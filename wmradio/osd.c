/*
 * Copyright (C) 12 Jun 2003 Tomas Cermak
 *
 * This file is part of wmradio preogram.
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
#include "osd.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_XOSD
#include <xosd.h>
xosd *osd = NULL;
#endif

int osd_init(char *progname, char *font, char *color,
	     int x, int y, int shadow, int timeout) {

#ifdef HAVE_XOSD
    /*
    osd = xosd_init(! font ? font : "fixed" ,
		    ! color ? color : "LawnGreen",
		    timeout,
		    XOSD_top,
		    x,
		    shadow,
		    1);
    if(!osd) {
    	fprintf(stderr, "%s: OSD error\n", progname);
	return 0;
    }
    */

    if (!(osd = xosd_create(1))) {
	fprintf(stderr, "%s: OSD error\n", progname);
	return 0;
    }

    if (font) {
	if (xosd_set_font(osd, font)) {
	    fprintf(stderr, "%s: OSD font init error\n", progname);
            xosd_set_font(osd, "fixed");
	}
    }
    else xosd_set_font(osd, "fixed");

    if (color) {
	if (xosd_set_colour(osd, color)) {
	    fprintf(stderr, "%s: OSD color init error\n", progname);
	    xosd_set_colour(osd, "LawnGreen");
	}
    }
    else xosd_set_colour(osd, "LawnGreen");

    xosd_set_vertical_offset(osd, y);
    xosd_set_horizontal_offset(osd, x);
    xosd_set_shadow_offset(osd, shadow);
    xosd_set_timeout(osd, timeout);

    return 1;
#else /* HAVE_XOSD */
    printf("wmradio: warning - compiled without osd\n");
    return 0;
#endif /* HAVE_XOSD */
}

void osd_print(char *string) {
#ifdef HAVE_XOSD
    xosd_display (osd, 0, XOSD_string, string);
#endif /* HAVE_XOSD */
}

void osd_close() {
#ifdef HAVE_XOSD
    xosd_destroy (osd);
#endif /* HAVE_XOSD */
}
