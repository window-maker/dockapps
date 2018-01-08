/*     WMGlobe 1.3  -  All the Earth on a WMaker Icon
 *     copyright (C) 1998,99,2000,01 Jerome Dumonteil <jerome.dumonteil@linuxfr.org>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ***************************************************************************/
/* 
 * I used many functions  of wmgeneral.c ("openXwindow")
 * for the main function of wmglobe.c 
 * wmgeneral.c was taken from wmaker applet wmtune-1.0 :
 * Author: Martijn Pieterse (pieterse@xs4all.nl)
 *
 * wmglobe.c uses functions of : Xglobe, Xearth, wmgeneral, wmaker/wrlib 
 ***************************************************************************/

#include "wmglobe.h"

#include "cadre0.xbm"
#include "cadre1.xbm"
#include "cadre2.xbm"

#ifdef DEFMAPOK
#include "defmap.xpm"
#include "defnimap.xpm"
#endif





int main(int argc, char *argv[])
{
    unsigned int borderwidth = 1;
    XClassHint classHint;
    char *wname = argv[0];
    XTextProperty name;

    XGCValues gcv;
    unsigned long gcm;
    XWindowAttributes attributes;
    XColor color;

    RContext *ctx;

    XSizeHints mysizehints;
    XWMHints mywmhints;
    Pixel back_pix, fore_pix;
    char Geometry[256];

    char *rond_bits;

    int dummy = 0;
    int ok, redoaction, wait_release, move_lat_flag;
    int xx, yy;

/** initialisation *********************/

    xx = 0;
    yy = 0;
    ok = FALSE;
    move_lat_flag = FALSE;
    redoaction = 0;
    wait_release = 0;

    setlocale(LC_TIME, "");

#ifdef DEBUG
    fprintf(stdout, "%s\n", setlocale(LC_TIME, ""));
#endif

    set_defaults();

    cmdline(argc, argv);

#if WITH_MARKERS
    if (nb_marker) {
	int i;
	for (i = 0; i < nb_marker; i++)
	    if (i != moon_marker && i != sun_marker) {
		marker[i][0] = marker[i][0] * PI / 180.;
		marker[i][1] = marker[i][1] * PI / 180.;
		transform_marker(i);
	    }
    }
#endif

    switch (typecadre) {
    case 1:
	rond_bits = cadre1_bits;
	break;
    case 2:
	rond_bits = cadre2_bits;
	break;
    default:
	rond_bits = cadre0_bits;
    }
    if (p_type == PTRANDOM) {
	dlat = 0;
	dlong = 0;
    }

/* setup long term mem allocation */
    initmyconvert();


    tdelay.tv_sec = (int) floor(delay);
    tdelay.tv_usec = (int) ((delay - tdelay.tv_sec) * 1000000);
    aml = (int) floor(ambient_light * 256);


/* ctx initialization */

    if (!(dpy = XOpenDisplay(dpy_name))) {
	fprintf(stderr, "%s: can't open display \"%s\"\n",
		wname, XDisplayName(dpy_name));
	exit(1);
    }
    ctx = myRCreateContext(dpy, DefaultScreen(dpy), NULL);

    if (ctx->attribs->use_shared_memory) {
#ifdef DEBUG
	fprintf(stdout, "remove flags use_shared_memory\n");
#endif
	ctx->attribs->flags ^= RC_UseSharedMemory;
	ctx->attribs->use_shared_memory = FALSE;
	ctx->flags.use_shared_pixmap = 0;
    }
#ifdef DEBUG
    fprintf(stdout, "depth %d\n", ctx->depth);
    fflush(stdout);
#endif

/*
 * loading maps .............
 */

    if (dayfile != NULL) {
	map = RLoadImage(ctx, dayfile, 0);
	if (!use_default_nightmap)
	    use_nightmap = FALSE;
	ripalpha(map);
	if (!map) {
	    fprintf(stdout, "pb map ! file not found ?\n");
	    exit(1);
	}
    } else {
#ifdef DEFMAPOK
	map = RGetImageFromXPMData(ctx, defmap_xpm);
	use_default_nightmap = TRUE;
	ripalpha(map);
	if (!map) {
	    fprintf(stdout, "pb def map ! file not found ?\n");
	    exit(1);
	}
    }
#else
	fprintf(stdout, "need a map !\n");
	exit(1);
    }
#endif
    if (!oknimap) {
	use_nightmap = FALSE;
	use_default_nightmap = FALSE;
    }
    if (use_nightmap) {
	if (nightfile != NULL) {
	    mapnight = RLoadImage(ctx, nightfile, 0);
	    ripalpha(mapnight);
	    if (!mapnight) {
		fprintf(stdout, "pb map night! file not found ?\n");
		exit(1);
	    }
	} else {
#ifdef DEFMAPOK
	    if (use_default_nightmap) {
		mapnight = RGetImageFromXPMData(ctx, defnimap_xpm);
		ripalpha(mapnight);
		if (!mapnight) {
		    fprintf(stdout,
			    "pb def map night ! file not found ?\n");
		    exit(1);
		}
	    }
	}
#else
	    use_nightmap = FALSE;
	}
#endif
    }

    use_nmap_ini = use_nightmap;

/* we need a night map of same size as day map */
    if (mapnight) {
	if ((mapnight->width != map->width)
	    || (mapnight->height != map->height)) {
	    RImage *tmp;
	    tmp = mapnight;
	    mapnight = RScaleImage(tmp, map->width, map->height);
	    RReleaseImage(tmp);
	    ripalpha(mapnight);
	}
    }

/* some other init ..................................... */
    ratiox = (double) map->width / (2 * PI);
    ratioy = (double) map->height / PI;
    mratiox = (int) floor(ratiox * 256);
    mratioy = (int) floor(ratioy * 256);
    loadxpm(ctx->drawable);

    small = RCreateImage(DIAMETRE, DIAMETRE, 0);

    calcDistance();

/*
 *  first rendering of the earth
 */
    rotation_terre(DIAMETRE / 2, DIAMETRE / 2, 0);
    recalc(0);
    do_something = FALSE;

/*************************************************************************
 * well, here the problems begin : this code is a merge from wmgeneral and
 * some stuff of wmaker, should be rewritten ...
 ************************************************************************/

    XGetWindowAttributes(dpy, ctx->drawable, &attributes);

    if (!RConvertImage(ctx, small, &pix)) {
	fprintf(stdout, "error small->&pix\n");
	puts(RMessageForError(RErrorCode));
	exit(1);
    }
    wmg.pixmap = pix;
    wmg.mask = pix;

    mysizehints.flags = USSize | USPosition;
    mysizehints.x = 0;
    mysizehints.y = 0;

    color.pixel = 0;
    if (!XParseColor(dpy, attributes.colormap, "white", &color)) {
	fprintf(stdout, "wmglobe: can't parse white\n");
    } else if (!XAllocColor(dpy, attributes.colormap, &color)) {
	fprintf(stdout, "wmglobe: can't allocate white\n");
    }
    back_pix = color.pixel;

    XGetWindowAttributes(dpy, ctx->drawable, &attributes);

    color.pixel = 0;
    if (!XParseColor(dpy, attributes.colormap, "black", &color)) {
	fprintf(stdout, "wmglobe: can't parse black\n");
    } else if (!XAllocColor(dpy, attributes.colormap, &color)) {
	fprintf(stdout, "wmglobe: can't allocate black\n");
    }
    fore_pix = color.pixel;


    XWMGeometry(dpy, ctx->screen_number, Geometry, NULL, borderwidth,
		&mysizehints, &mysizehints.x, &mysizehints.y,
		&mysizehints.width, &mysizehints.height, &dummy);
    mysizehints.width = DIAMETRE;
    mysizehints.height = DIAMETRE;

    win =
	XCreateSimpleWindow(dpy, ctx->drawable, mysizehints.x,
			    mysizehints.y, mysizehints.width,
			    mysizehints.height, borderwidth, fore_pix,
			    back_pix);

    iconwin = XCreateSimpleWindow(dpy, win, mysizehints.x, mysizehints.y,
				  mysizehints.width, mysizehints.height,
				  borderwidth, fore_pix, back_pix);

    /* Activate hints */
    XSetWMNormalHints(dpy, win, &mysizehints);
    classHint.res_name = wname;
    classHint.res_class = wname;
    XSetClassHint(dpy, win, &classHint);

    XSelectInput(dpy, win,
		 ButtonPressMask | ExposureMask | ButtonReleaseMask |
		 PointerMotionMask | StructureNotifyMask);
    XSelectInput(dpy, iconwin,
		 ButtonPressMask | ExposureMask | ButtonReleaseMask |
		 PointerMotionMask | StructureNotifyMask);

    if (XStringListToTextProperty(&wname, 1, &name) == 0) {
	fprintf(stdout, "%s: can't allocate window name\n", wname);
	exit(1);
    }
    XSetWMName(dpy, win, &name);

    /* Create GC for drawing */

    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = fore_pix;
    gcv.background = back_pix;
    gcv.graphics_exposures = 0;
    NormalGC = XCreateGC(dpy, ctx->drawable, gcm, &gcv);

    /* ONLYSHAPE ON */
    if (onlyshape) {
	pixmask =
	    XCreateBitmapFromData(dpy, win, rond_bits, DIAMETRE, DIAMETRE);
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, pixmask,
			  ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, pixmask,
			  ShapeSet);
    }
    /* ONLYSHAPE OFF */

    mywmhints.initial_state = option_iw;
    mywmhints.icon_window = iconwin;
    mywmhints.icon_x = mysizehints.x;
    mywmhints.icon_y = mysizehints.y;
    mywmhints.window_group = win;
    mywmhints.flags =
	StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;

    XSetWMHints(dpy, win, &mywmhints);

    XSetCommand(dpy, win, argv, argc);
    XMapWindow(dpy, win);


    XCopyArea(dpy, wmg.pixmap, win, NormalGC, 0, 0, DIAMETRE, DIAMETRE, 0,
	      0);

    RedrawWindowXYWH(0, 0, DIAMETRE, DIAMETRE);

/*
 *  =================   MAIN LOOP     ==================
 */
    while (1) {
	while (XPending(dpy)) {
	    XNextEvent(dpy, &Event);
	    switch (Event.type) {
	    case Expose:
		RedrawWindowXYWH(0, 0, DIAMETRE, DIAMETRE);
		break;
	    case DestroyNotify:
		XCloseDisplay(dpy);
		exit(0);
		break;
	    case ButtonPress:
/*
 * earth rotate when clic left (1) , zooming when middle (2)
 * change screen to longitude / latitude when (3)
 */
		switch (Event.xbutton.button) {
		case 1:
#ifdef MOUSE_LAT_NO_SHIFT
		    move_lat_flag = TRUE;
#else
		    if (Event.xbutton.state & ShiftMask)
			move_lat_flag = TRUE;
		    else
			move_lat_flag = FALSE;
#endif
		    redoaction = 1;
		    wait_release = 1;
		    break;
		case 2:
		    if (Event.xbutton.state & ShiftMask)
			redoaction = 2;
		    else
			redoaction = 3;
		    wait_release = 1;
		    break;
		case 3:
		    wait_release = 0;
		    redoaction = 0;
		    screen_back();
		    ok = TRUE;
		    break;
		default:
		    break;
		}
		break;
	    case ButtonRelease:
		wait_release = 0;
		redoaction = 0;
		break;
	    default:
		break;
	    }
	}
	if (wait_release) {
	    usleep(2 * VAL_USLEEP_SHORT);
	    if (redoaction == 1)
		rotation_terre(Event.xbutton.x, Event.xbutton.y,
			       move_lat_flag);
	    else
		zooming(Event.xbutton.state & ShiftMask);
	    ok = TRUE;
	}
	if (diftimev(tnext, getimev()).tv_sec < 0 || ok) {
	    ok = FALSE;
	    recalc(redoaction == 1);
	    if (do_something) {
		if (!myRConvertImage(ctx, small, &pix)) {
		    fprintf(stderr, "crash !?\n");
		    fprintf(stderr, RMessageForError(RErrorCode));
		    exit(1);
		}
		wmg.pixmap = pix;
		wmg.mask = pix;
		RedrawWindowXYWH(0, 0, DIAMETRE, DIAMETRE);
#ifdef DEBUG
		fprintf(stdout, "draw\n");
#endif
		do_something = FALSE;
	    }
	}
	usleep(VAL_USLEEP);
    }
    return 0;
}
