/*     WMGlobe 0.5  -  All the Earth on a WMaker Icon
 *     copyright (C) 1998,99 Jerome Dumonteil <jerome.dumonteil@capway.com>
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
#include "zapnum.xpm"
#include "zaptxt.xpm"
#include "zapnum.h"
#include "scrpos.xpm"
#include "scrtime.xpm"
#include "scrdiv.xpm"

static void move_earth(double vla, double vlo);
static int flush_expose(Window w);
static void mqparam();
static double getdegre(char *val);
static void screen_1();
static void screen_2();
static void screen_3();
static void screen_4();
static void screen_5();
static void screen_6();
static void screen_7();
static void write_icon(char *txt, int x, int y);
static void chiffre(int ch, int xx, int yy);
static int lettre(char c, int xx, int yy);
static void display_pos(double la, double lo);
static void display_date(double la, double lo);
static void release_but(int ckm);
static void press_but(int ckm);
static void display_zoom();
static void display_light();
static void display_accel();
static void display_dlat();
static void display_type();
static void move_delay(int factor);
static void move_zoom(int factor);
static void move_light(int factor);
static void move_dawn(int factor);
static void move_accel(int factor);
static void move_dlat(int factor);
static void move_dlong(int factor);
/* ------------------------------------------------------------------------ */
/*
 *       TIME FUNCTIONS
 */
/* ------------------------------------------------------------------------ */
struct timeval diftimev(struct timeval t1, struct timeval t2)
{
	t1.tv_usec -= t2.tv_usec;
	if (t1.tv_usec < 0) {
		t1.tv_usec += 1000000;
		t1.tv_sec--;
	}
	t1.tv_sec -= t2.tv_sec;
	return t1;
}
/* ------------------------------------------------------------------------ */
struct timeval addtimev(struct timeval t1, struct timeval t2)
{
	t1.tv_usec += t2.tv_usec;
	if (t1.tv_usec >= 1000000) {
		t1.tv_usec -= 1000000;
		t1.tv_sec++;
	}
	t1.tv_sec += t2.tv_sec;
	return t1;
}
/* ------------------------------------------------------------------------ */
struct timeval getimev()
{
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	return tv;
}
/* ------------------------------------------------------------------------ */
/*
 *     INIT FUNCTIONS
 *
 *  defaults, commandline
 *  init of pixmap for screenpos and numbers displaying
 */
/* ------------------------------------------------------------------------ */
void loadxpm(Window draw)
{
	XpmCreatePixmapFromData(dpy, draw, scrpos_xpm, &(screenpos.pixmap),
			     &(screenpos.mask), &(screenpos.attributes));
	XpmCreatePixmapFromData(dpy, draw, scrtime_xpm, &(scrdate.pixmap),
				&(scrdate.mask), &(scrdate.attributes));
	XpmCreatePixmapFromData(dpy, draw, zapnum_xpm, &(numpix.pixmap),
				&(numpix.mask), &(numpix.attributes));
	XpmCreatePixmapFromData(dpy, draw, zaptxt_xpm, &(txtpix.pixmap),
				&(txtpix.mask), &(txtpix.attributes));
	XpmCreatePixmapFromData(dpy, draw, scrdiv_xpm, &(scrdiv.pixmap),
				&(scrdiv.mask), &(scrdiv.attributes));
	return;
}
/* ------------------------------------------------------------------------ */
void set_defaults()
{
	firstTime = TRUE;
	dayfile = NULL;
	use_nightmap = FALSE;
	nightfile = NULL;
	oknimap = DEFAULT_NIGHTMAP;
	p_type = PTSUNREL;
	delay = DEFAULT_DELAY;
	tini = getimev();
	tbase = tini;
	tsunpos = 0;
	tlast.tv_sec = 0;
	tlast.tv_usec = 0;
	do_something = TRUE;
	time_multi = 1.0;
	v_lat = DEFAULT_V_LAT / 180. * PI;
	v_long = DEFAULT_V_LONG / 180. * PI;
	dv_lat = DEFAULT_V_LAT;
	dv_long = DEFAULT_V_LONG;
	old_dvlat = DEFAULT_V_LAT;
	old_dvlong = DEFAULT_V_LONG;
	iop = 0;
	dlat = 0.0;
	dlong = 0.0;
	addlat = 0.0;
	addlong = 0.0;
	radius = 1000.0;
	sun_long = 0.0;
	sun_lat = 0.0;
	fov = 0.5 * PI / 180.;
	sens = DEFAULT_SENS;
	zoom = DEFAULT_ZOOM;
	ambient_light = DEFAULT_LIGHT;
	dawn = DEFAULT_DAWN;
	typecadre = DEFAULT_BORDER;
	fun = FALSE;
	funx = 0;
	funy = 0;
	dpy_name = NULL;
	onlyshape = TRUE;
	option_iw = WithdrawnState;
	stoprand = FALSE;
	solution = FALSE;
	proj_dist = DIAMETRE / tan(fov);
	noir.red = 0;
	noir.green = 0;
	noir.blue = 0;
	noir.alpha = 255;

	md[0] = (MPO *) malloc(sizeof(MPO));
	md[1] = (MPO *) malloc(sizeof(MPO));
	md[2] = (MPO *) malloc(sizeof(MPO));
	md[3] = (MPO *) malloc(sizeof(MPO));
	mn[0] = (MPO *) malloc(sizeof(MPO));
	mn[1] = (MPO *) malloc(sizeof(MPO));
	mn[2] = (MPO *) malloc(sizeof(MPO));
	mn[3] = (MPO *) malloc(sizeof(MPO));
#ifdef DEBUG
	minhz = 1.;
#endif
	return;
}
/* ------------------------------------------------------------------------ */
static double getdegre(char *val)
{
	double d, m, s;
	if (strchr(val, '°') != NULL || strchr(val, 'd') != NULL
	    || strchr(val, 'D') != NULL || strchr(val, ':') != NULL) {
		d = m = s = 0;
		sscanf(val, "%lf%*c%lf%*c%lf", &d, &m, &s);
		if (d >= 0.)
			return d + m / 60. + s / 3600.;
		else
			return d - m / 60. - s / 3600.;
	} else {
		return atof(val);
	}
}
/* ------------------------------------------------------------------------ */
static void mqparam()
{
	printf("error in parameters\n");
	exit(1);
}

/* ------------------------------------------------------------------------ */
void cmdline(int argc, char *argv[])
{
	int i, j;

	if (argc > 0) {
		for (i = 1; i < argc; i++) {
			j = 0;

/*                      ---------------------------------------------------- */
			if (strcasecmp(argv[i], "-v") == 0
			    || strcasecmp(argv[i], "-version") == 0) {
				printf("%s\n", WMGVERSION);
				exit(0);
			}
/*                      ---------------------------------------------------- */
			if (strcasecmp(argv[i], "-w") == 0
			    || strcasecmp(argv[i], "-wmaker") == 0) {
				option_iw = WithdrawnState;
				j = 1;
			}
/*                      ---------------------------------------------------- */
			if (strcasecmp(argv[i], "-i") == 0
			    || strcasecmp(argv[i], "-iconic") == 0) {
				option_iw = IconicState;
				j = 1;
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-s") == 0 ||
				   strcasecmp(argv[i], "-shape") == 0)) {
				onlyshape = TRUE;
				j = 1;
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-d") == 0
			      || strcasecmp(argv[i], "-display") == 0)) {
				i++;
				if (i < argc) {
					j = 1;
					dpy_name = argv[i];
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-rand") == 0
			       || strcasecmp(argv[i], "-random") == 0)) {
				p_type = PTRANDOM;
				srandom(((int) time(NULL)) + ((int) getpid()));
				j = 1;
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-oz") == 0
			      || strcasecmp(argv[i], "-austral") == 0)) {
				sens = -1;
				j = 1;
			}
/*                      ---------------------------------------------------- */
			if (!j && strcasecmp(argv[i], "-map") == 0) {

				i++;
				if (i < argc) {
					j = 1;
					free(dayfile);
					if ((dayfile = (char *) malloc(strlen(argv[i]) + 1)) == NULL) {
						fprintf(stderr, "erreur memoire options map\n");
						exit(1);
					};
					strcpy(dayfile, argv[i]);
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-nimap") == 0
				 || strcasecmp(argv[i], "-nightmap") == 0
				|| strcasecmp(argv[i], "-night") == 0)) {

				i++;
				if (i < argc) {
					j = 1;
					free(nightfile);
					if ((nightfile = (char *) malloc(strlen(argv[i]) + 1)) == NULL) {
						fprintf(stderr, "erreur memoire options nimap\n");
						exit(1);
					};
					strcpy(nightfile, argv[i]);
					use_nightmap = TRUE;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-accel") == 0
				   || strcasecmp(argv[i], "-accell") == 0
				   || strcasecmp(argv[i], "-acc") == 0
				|| strcasecmp(argv[i], "-multi") == 0)) {
				i++;
				if (i < argc) {
					j = 1;
					time_multi = atof(argv[i]);
					if (time_multi < 1.0)
						time_multi = 1.0;
					if (time_multi > MAX_MULTI_COEF)
						time_multi = MAX_MULTI_COEF;
					if (time_multi > 24.0)
						time_multi = floor(time_multi);
					else
						time_multi = floor(time_multi * 10.0) / 10.0;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-nonimap") == 0
			       || strcasecmp(argv[i], "-nonightmap") == 0
				   || strcasecmp(argv[i], "-nonight") == 0
				|| strcasecmp(argv[i], "-nomap") == 0)) {
				j = 1;
				use_nightmap = FALSE;
				oknimap = FALSE;
			}
/*                      ---------------------------------------------------- */
			if (!j && strcasecmp(argv[i], "-zoom") == 0) {
				i++;
				if (i < argc) {
					j = 1;
					zoom = atof(argv[i]);
					if (zoom < ZOOM_MIN)
						zoom = ZOOM_MIN;
					if (zoom > ZOOM_MAX)
						zoom = ZOOM_MAX;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && strcasecmp(argv[i], "-dawn") == 0) {
				i++;
				if (i < argc) {
					j = 1;
					dawn = atof(argv[i]);
					dawn = MAX(0.0, dawn);
					dawn = MIN(1.0, dawn);

					dawn = (1.0 - dawn / 2.0);
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && strcasecmp(argv[i], "-delay") == 0) {
				i++;
				if (i < argc) {
					j = 1;
					delay = atof(argv[i]);
					if (delay < 0)
						delay = 0;
					if (delay > MAX_DELAY_SEC)
						delay = MAX_DELAY_SEC;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-dlat") == 0
				   || strcasecmp(argv[i], "-lat") == 0
			    || strcasecmp(argv[i], "-dlatitude") == 0)) {
				i++;
				if (i < argc) {
					j = 1;
					dlat = getdegre(argv[i]);
					if (dlat < -MAX_DELTA_LONG)
						dlat = -MAX_DELTA_LONG;
					if (dlat > MAX_DELTA_LONG)
						dlat = MAX_DELTA_LONG;
					p_type = PTFIXED;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-dlong") == 0
				   || strcasecmp(argv[i], "-long") == 0
			   || strcasecmp(argv[i], "-dlongitude") == 0)) {
				i++;
				if (i < argc) {
					j = 1;
					dlong = getdegre(argv[i]);
					if (dlong < -MAX_DELTA_LONG)
						dlong = -MAX_DELTA_LONG;
					if (dlong > MAX_DELTA_LONG)
						dlong = MAX_DELTA_LONG;
					p_type = PTFIXED;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && strcasecmp(argv[i], "-light") == 0) {
				i++;
				if (i < argc) {
					j = 1;
					ambient_light = atof(argv[i]);
					if (ambient_light < 0.)
						ambient_light = 0.;
					if (ambient_light > 1.)
						ambient_light = 1.;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && strcasecmp(argv[i], "-time") == 0) {
				i++;
				if (i < argc) {
					j = 1;
					tbase.tv_sec = atoi(argv[i]);
					tbase.tv_usec = 0;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-bord") == 0
			       || strcasecmp(argv[i], "-border") == 0)) {
				i++;
				if (i < argc) {
					j = 1;
					typecadre = atoi(argv[i]);
					if (typecadre < 0)
						typecadre = 0;
					if (typecadre > 2)
						typecadre = 2;
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && strcasecmp(argv[i], "-fun") == 0) {
				fun = TRUE;
				i++;
				if (i < argc) {
					funx = atoi(argv[i]);

					i++;
					if (i < argc) {
						j = 1;
						funy = atoi(argv[i]);
					} else {
						mqparam();
						i--;
					}
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */
			if (!j && (strcasecmp(argv[i], "-pos") == 0
				 || strcasecmp(argv[i], "-position") == 0
				|| strcasecmp(argv[i], "-fixed") == 0)) {
				p_type = PTFIXED;
				i++;
				if (i < argc) {
					addlat = getdegre(argv[i]);
					i++;
					if (i < argc) {
						j = 1;
						addlong = getdegre(argv[i]);
					} else {
						mqparam();
						i--;
					}
				} else {
					mqparam();
					i--;
				}
			}
/*                      ---------------------------------------------------- */

			if (!j && argv[i][0] == '-') {
				printf("%s\n", WMGVERSION);
				printf("\n");
				printf("-v                  : version         -h : this help message !\n");
				printf("-zoom   zoom_value  : changing apparent size in icon\n");
				printf("-pos latitude long. : fixed initial position (default=follow sun)\n");
				printf("-rand               : random position at every refresh\n");
				printf("-map    map_file    : use this map for rendering\n");
				printf("-nimap  night_file  : and this one for the dark side of earth\n");
				printf("-nonimap            : don't use the default night map\n");
				printf("-delay  seconds     : time between refresh of image\n");
				printf("-dlat   delta_lat   : latitude speed of point of view (default=follow sun)\n");
				printf("-dlong  delta_long  : the same for longitude\n");
				printf("-light  light_value : level of light for dark side of earth\n");
				printf("-dawn   dawn_value  : level of continuity for dawn limit\n");
				printf("-bord   border_num  : 0 1 or 2 , type of icon border.\n");
				printf("-accel  time_multi  : time accelerator\n");
				printf("-time   seconds     : time to display in seconds since 01-01-1970\n");
				printf("-oz                 : start in \"austral\" mode (for \"down under\" people)\n");
				printf("-fun    dx  dy      : offset of vision... almost useless\n");
				printf("-w  -shape          : set by default (WMaker dockable application)\n");
				printf("-d      display     : display (WindowMaker not needed on the server side)\n");
				printf("left button         : change longitude, with shift key, change latitude too\n");
				printf("middle button       : zoom in, shift + middle button : zoom out\n");
				printf("right button        : access to a few screens of parameters\n");
				exit(0);
			}
/* ------------------------------------------------------------------------ */
		}
	}
	return;
}

/****************************************************************************
 * X functions, mouse selection
 *
 */

/* ------------------------------------------------------------------------ */
static int flush_expose(Window w)
{
	XEvent dummy;
	int i = 0;

	while (XCheckTypedWindowEvent(dpy, w, Expose, &dummy))
		i++;
	return i;
}
/* ------------------------------------------------------------------------ */
void RedrawWindowXYWH(int x, int y, int w, int h)
{
	flush_expose(iconwin);
	XCopyArea(dpy, wmg.pixmap, iconwin, NormalGC, x, y, w, h, x, y);
	flush_expose(win);
	XCopyArea(dpy, wmg.pixmap, win, NormalGC, x, y, w, h, x, y);
	return;
}
/* ------------------------------------------------------------------------ */
void AddMouseRegion(int index, int left, int top, int right, int bottom)
{
	if (index < MAX_MOUSE_REGION) {
		mouse_region[index].enable = 1;
		mouse_region[index].top = top;
		mouse_region[index].left = left;
		mouse_region[index].bottom = bottom;
		mouse_region[index].right = right;
	}
	return;
}
/* ------------------------------------------------------------------------ */
int CheckMouseRegion(int x, int y)
{
	int i;
	int found;

	found = 0;

	for (i = 0; i < MAX_MOUSE_REGION && !found; i++) {
		if (mouse_region[i].enable &&
		    x <= mouse_region[i].right &&
		    x >= mouse_region[i].left &&
		    y <= mouse_region[i].bottom &&
		    y >= mouse_region[i].top)
			found = 1;
	}
	if (!found)
		return -1;
	return (i - 1);
}
/* ------------------------------------------------------------------------ */
/*
 * GRAPHIC : pixmap writing of letters & numbers
 *
 */
/* ------------------------------------------------------------------------ */
static void chiffre(int ch, int xx, int yy)
{
	XCopyArea(dpy, numpix.pixmap, wmg.pixmap, NormalGC,
		  zapnum[ch][0], zapnum[ch][1], zapnum[ch][2], zapnum[ch][3], xx, yy);

	return;
}
/* ------------------------------------------------------------------------ */
static int lettre(char c, int xx, int yy)
{
	int i;
	if (c == '°')
		i = 288;
	else
		switch (toupper(c)) {
		case '0':
			i = 0;
			break;
		case '1':
			i = 6;
			break;
		case '2':
			i = 12;
			break;
		case '3':
			i = 18;
			break;
		case '4':
			i = 24;
			break;
		case '5':
			i = 30;
			break;
		case '6':
			i = 36;
			break;
		case '7':
			i = 42;
			break;
		case '8':
			i = 48;
			break;
		case '9':
			i = 54;
			break;
		case '+':
			i = 60;
			break;
		case 'A':
			i = 66;
			break;
		case 'B':
			i = 72;
			break;
		case 'C':
			i = 78;
			break;
		case 'D':
			i = 84;
			break;
		case 'E':
			i = 90;
			break;
		case 'F':
			i = 96;
			break;
		case 'G':
			i = 102;
			break;
		case 'H':
			i = 108;
			break;
		case 'I':
			i = 114;
			break;
		case 'J':
			i = 120;
			break;
		case 'K':
			i = 126;
			break;
		case 'L':
			i = 132;
			break;
		case 'M':
			i = 138;
			break;
		case 'N':
			i = 144;
			break;
		case 'O':
			i = 150;
			break;
		case 'P':
			i = 156;
			break;
		case 'Q':
			i = 162;
			break;
		case 'R':
			i = 168;
			break;
		case 'S':
			i = 174;
			break;
		case 'T':
			i = 180;
			break;
		case 'U':
			i = 186;
			break;
		case 'V':
			i = 192;
			break;
		case 'W':
			i = 198;
			break;
		case 'X':
			i = 204;
			break;
		case 'Y':
			i = 210;
			break;
		case 'Z':
			i = 216;
			break;
		case ' ':
			i = 222;
			break;
		case ':':
			i = 228;
			break;
		case '/':
			i = 234;
			break;
		case '!':
			i = 240;
			break;
		case '*':
			i = 246;
			break;
		case '-':
			i = 252;
			break;
		case '.':
			i = 258;
			break;
		case '=':
			i = 264;
			break;
		case '#':
			i = 270;
			break;
		case '<':
			i = 276;
			break;
		case '>':
			i = 282;
			break;
		case '"':
			i = 294;
			break;
		case '\'':
			i = 300;
			break;
		case ',':
			i = 306;
			break;
		case '@':
			i = 312;
			break;
		case '(':
			i = 318;
			break;
		case ')':
			i = 324;
			break;
		case '&':
			i = 330;
			break;
		case '~':
			i = 336;
			break;
		case '£':
			i = 342;
			break;
		case 'µ':
			i = 348;
			break;
		case '{':
			i = 354;
			break;
		case '}':
			i = 360;
			break;
		case '[':
			i = 366;
			break;
		case ']':
			i = 372;
			break;
		case '\\':
			i = 378;
			break;
		case '|':
			i = 384;
			break;
		case '?':
			i = 390;
			break;
		case '%':
			i = 396;
			break;
		default:
			i = 222;
			break;
		}

	XCopyArea(dpy, txtpix.pixmap, wmg.pixmap, NormalGC,
		  i, 0, 6, 10, xx, yy);
	return xx + 6;
}
/* ------------------------------------------------------------------------ */
static void write_icon(char *txt, int x, int y)
{
	int p, ok;
	p = 0;

#ifdef DEBUG
	fprintf(stdout, "%s_\n", txt);
#endif
	ok = TRUE;
	while (ok) {
		if (txt[p] == '\0' || x > DIAMETRE - 5) {
			ok = FALSE;
		} else {
			x = lettre(txt[p], x, y);
			p++;
		}
	}
	return;
}
/* ------------------------------------------------------------------------ */
static void press_but(int ckm)
{
	switch (ckm) {
	case 5:
		XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
			  0, 38, 20, 9, 2, 52);
		RedrawWindowXYWH(2, 52, 20, 9);
		break;
	case 6:
		XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
			  20, 38, 20, 9, 22, 52);
		RedrawWindowXYWH(22, 52, 20, 9);
		break;
	case 7:
		XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
			  40, 38, 20, 9, 42, 52);
		RedrawWindowXYWH(42, 52, 20, 9);
		break;
	default:
		break;
	}
	return;
}
/* ------------------------------------------------------------------------ */
static void release_but(int ckm)
{
	switch (ckm) {
	case 5:
		XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
			  0, 28, 20, 9, 2, 52);
		RedrawWindowXYWH(2, 52, 20, 9);
		break;
	case 6:
		XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
			  20, 28, 20, 9, 22, 52);
		RedrawWindowXYWH(22, 52, 20, 9);
		break;
	case 7:
		XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
			  40, 28, 20, 9, 42, 52);
		RedrawWindowXYWH(42, 52, 20, 9);
		break;
	default:
		break;
	}
	return;
}
/* ------------------------------------------------------------------------ */
/*
 * MENU SELECTION & ACTIONS
 */
/* ------------------------------------------------------------------------ */
void screen_back()
{
	AddMouseRegion(0, 0, 0, 0, 0);
	if (p_type == PTRANDOM) {
		stoprand = STOP_RANDOM_FACTOR;
		addlong = dv_long;
		addlat = dv_lat;
	}
	gotoscr = 1;
	while (gotoscr != 0) {
		switch (gotoscr) {
		case 1:
			screen_1();
			break;
		case 2:
			screen_2();
			break;
		case 3:
			screen_3();
			break;
		case 4:
			screen_4();
			break;
		case 5:
			screen_5();
			break;
		case 6:
			screen_6();
			break;
		case 7:
			screen_7();
			break;
		default:
			break;
		}
		XFlush(dpy);
	}
/*
 * put old environment
 */
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);

	AddMouseRegion(0, 5, 5, DIAMETRE - 4, DIAMETRE - 4);
	stoprand = FALSE;
	do_something = TRUE;

	return;
}
/* ------------------------------------------------------------------------ */
static void display_pos(double la, double lo)
{
	int c, i, j, k, l, neg;

	if (la > 0) {
		neg = 10;
	} else {
		neg = 11;
		la *= -1;
	}
	i = floor(la / 10.0);
	la -= 10 * i;
	j = floor(la);
	la -= j;
	la *= 60;
	k = floor(la / 10.0);
	la -= 10 * k;
	l = floor(la);

	if (i == 0) {
		i = neg;
		neg = 10;
	}
	chiffre(neg, platd[1][0], platd[1][1]);
	chiffre(i, platd[2][0], platd[2][1]);
	chiffre(j, platd[3][0], platd[3][1]);
	chiffre(k, platm[0][0], platd[0][1]);
	chiffre(l, platm[1][0], platd[1][1]);

	if (lo > 0) {
		neg = 12;
	} else {
		neg = 11;
		lo *= -1;
	}
	c = floor(lo / 100.0);
	lo -= c * 100;
	if (c > 0) {
		if (neg == 11)
			neg = 13;
	} else {
		if (neg == 11) {
			c = neg;
		} else {
			c = 10;
		}
		neg = 12;
	}
	i = floor(lo / 10.0);
	lo -= 10 * i;
	j = floor(lo);
	lo -= j;
	lo *= 60;
	k = floor(lo / 10.0);
	lo -= 10 * k;
	l = floor(lo);
	if (i == 0 && c > 9) {
		i = c;
		c = 10;
	}
	chiffre(neg, plongd[0][0], plongd[0][1]);
	chiffre(c, plongd[1][0], plongd[1][1]);
	chiffre(i, plongd[2][0], plongd[2][1]);
	chiffre(j, plongd[3][0], plongd[3][1]);
	chiffre(k, plongm[0][0], plongm[0][1]);
	chiffre(l, plongm[1][0], plongm[1][1]);

	RedrawWindowXYWH(0, 0, DIAMETRE, DIAMETRE);
	return;
}
/* ------------------------------------------------------------------------ */
void rotation_terre(int x, int y, int lat_flag)
{
/*
 * earth rotate after (while) a clic
 */
	double mx, my;
	mx = (double) ((double) x - DIAMETRE / 2 + 0.5) / zoom * cos(v_lat) * sens;
	if (lat_flag) {
		my = -(double) ((double) y - DIAMETRE / 2 + 0.5) * sens / zoom;
		if (my > 0.0)
			my += ABS(sin(v_lat) * ((double) x - DIAMETRE / 2 + 0.5) / zoom);
		else
			my -= ABS(sin(v_lat) * ((double) x - DIAMETRE / 2 + 0.5) / zoom);
	} else {
		my = 0;
	}
	if (p_type == PTRANDOM) {
		stoprand = STOP_RANDOM_FACTOR;
		addlong = dv_long;
		addlat = dv_lat;
	}
	addlong += mx * RATIO_ROTATE;
	addlat += my * RATIO_ROTATE;

	do_something = TRUE;
	return;
}
/* ------------------------------------------------------------------------ */
static void move_earth(double vla, double vlo)
{
	addlat += vla;
	addlong += vlo;


	switch (p_type) {
	case PTSUNREL:
		setViewPos(sun_lat * 180. / PI + addlat,
			   sun_long * 180. / PI + addlong);
		break;

	case PTFIXED:
	case PTRANDOM:
		setViewPos(addlat, addlong);
		break;
	default:
		break;
	}
	display_pos(dv_lat, dv_long);
	return;
}
/* ------------------------------------------------------------------------ */
static void screen_1()
{
	int ok, ckm, sensadd, waitrel, not3;
	struct timeval tin;

#ifdef DEBUG
	fprintf(stdout, "scr 1\n");
#endif
	tin = getimev();
	waitrel = 0;
	ckm = 0;
	sensadd = 1;
	not3 = TRUE;

	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, screenpos.mask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, screenpos.mask, ShapeSet);

	XCopyArea(dpy, screenpos.pixmap, wmg.pixmap, NormalGC, 0, 0, DIAMETRE, DIAMETRE, 0, 0);

	AddMouseRegion(1, platd[0][0], platd[0][1], platd[3][0] + 9, platd[3][1] + 12);
	AddMouseRegion(2, platm[0][0], platm[0][1], platm[1][0] + 9, platm[1][1] + 12);
	AddMouseRegion(3, plongd[0][0], plongd[0][1], plongd[3][0] + 9, plongd[3][1] + 12);
	AddMouseRegion(4, plongm[0][0], plongm[0][1], plongm[1][0] + 9, plongm[1][1] + 12);
	AddMouseRegion(5, 1, 51, 21, 61);
	AddMouseRegion(6, 22, 51, 41, 61);
	AddMouseRegion(7, 42, 51, 62, 61);

	display_pos(dv_lat, dv_long);

	ok = TRUE;
	while (ok) {
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
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				waitrel = 1;
				press_but(ckm);
				switch (Event.xbutton.button) {
				case 1:
					not3 = TRUE;
					sensadd = 1;
					break;
				case 2:
					not3 = TRUE;
					sensadd = -1;
					break;
				case 3:
					not3 = FALSE;
					if (ckm < 5)
						gotoscr = 0;
				default:
					break;
				}
				break;
			case ButtonRelease:
				release_but(ckm);
				waitrel = ckm;
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				if (waitrel == ckm) {
					switch (ckm) {
					case 5:
						gotoscr--;
						if (gotoscr == 0)
							gotoscr = NUM_SCREEN;
						break;
					case 6:
						gotoscr++;
						if (gotoscr > NUM_SCREEN)
							gotoscr = 1;
						break;
					case 7:
						gotoscr = 0;
					default:
						break;
					}
				}
				ckm = 0;
				waitrel = 0;
				tin = getimev();
				break;
			default:
				break;

			}
		}
		usleep(VAL_USLEEP_SHORT);
		if (waitrel && not3) {
			if (ckm == 1) {
				move_earth(sens * sensadd * 1.0, 0.0);
				usleep(VAL_USLEEP);
			}
			if (ckm == 2) {
				move_earth(sens * sensadd * 1.0 / 60.0, 0.0);
				usleep(VAL_USLEEP);
			}
			if (ckm == 3) {
				move_earth(0.0, sens * sensadd * 1.0);
				usleep(VAL_USLEEP);
			}
			if (ckm == 4) {
				move_earth(0.0, sens * sensadd * 1.0 / 60.0);
				usleep(VAL_USLEEP);
			}
		}
		if (waitrel == 0 && diftimev(getimev(), tin).tv_sec > SCREEN_WAIT && gotoscr == 1)
			gotoscr = 0;
		if (gotoscr != 1)
			ok = FALSE;
	}
	return;
}
/* ------------------------------------------------------------------------ */
static void display_date(double la, double lo)
{
	char datest[32];
	time_t t;
	int i, j;

	write_icon(" time :", 2, 2);
	RedrawWindowXYWH(2, 2, DIAMETRE - 1, DIAMETRE - 1);

	write_icon(" GMT+long.", 2, 14);
	RedrawWindowXYWH(2, 14, DIAMETRE - 1, DIAMETRE - 1);

	t = trend.tv_sec;
	t += (int) (floor((lo + 7.5) / 15.0) * 3600);
/*** pb near 2038 ***/

	strftime(datest, 30, "%x", gmtime(&t));

	i = 10 - strlen(datest);
	if (i < 0)
		i = 0;
	else
		i = i / 2;

	write_icon(datest, 2 + i * 6, 26);
	RedrawWindowXYWH(2, 26, DIAMETRE - 1, DIAMETRE - 1);

	strftime(datest, 30, "%X", gmtime(&t));

	i = 10 - strlen(datest);
	if (i < 0) {
		i = 0;
		while (datest[i] != '\0') {
			if (datest[i] != ' ') {
				i++;
			} else {
				j = i;
				do {
					datest[j] = datest[j + 1];
					j++;
				}
				while (datest[j - 1] != '\0');
			}
		}
		i = 0;
	} else {
		i = i / 2;
	}

	write_icon(datest, 2 + i * 6, 38);
	RedrawWindowXYWH(2, 38, DIAMETRE - 1, DIAMETRE - 1);

	return;
}
/* ------------------------------------------------------------------------ */
static void screen_2()
{
	int ok, ckm, waitrel;
	struct timeval tin;

#ifdef DEBUG
	fprintf(stdout, "scr 2\n");
#endif
	tin = getimev();
	waitrel = 0;
	ckm = 0;

	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);

	XCopyArea(dpy, scrdate.pixmap, wmg.pixmap, NormalGC, 0, 0, DIAMETRE, DIAMETRE, 0, 0);

	display_date(dv_lat, dv_long);

	ok = TRUE;
	while (ok) {
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
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				waitrel = 1;
				press_but(ckm);
				switch (Event.xbutton.button) {
				case 1:
					break;
				case 2:
					break;
				case 3:
					if (ckm < 5)
						gotoscr = 0;
					break;
				default:
					break;
				}
				break;
			case ButtonRelease:
				release_but(ckm);
				waitrel = ckm;
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				if (waitrel == ckm) {
					switch (ckm) {
					case 5:
						gotoscr--;
						if (gotoscr == 0)
							gotoscr = NUM_SCREEN;
						break;
					case 6:
						gotoscr++;
						if (gotoscr > NUM_SCREEN)
							gotoscr = 1;
						break;
					case 7:
						gotoscr = 0;
					default:
						break;
					}
				}
				ckm = 0;
				waitrel = 0;
				tin = getimev();
				break;
			default:
				break;

			}
		}
		usleep(VAL_USLEEP_SHORT);

		if (waitrel == 0 && diftimev(getimev(), tin).tv_sec > SCREEN_WAIT && gotoscr == 2)
			gotoscr = 0;
		if (gotoscr != 2)
			ok = FALSE;
	}
	return;
}
/* ------------------------------------------------------------------------ */
static void display_zoom()
{
	char datest[32], dstr[32];
	int i;

	write_icon(" delay :", 2, 2);
	RedrawWindowXYWH(2, 2, DIAMETRE - 1, DIAMETRE - 1);

	sprintf(dstr, "%f", delay);
	dstr[8] = '\0';
	i = strlen(dstr) - 1;
	while (i > 0) {
		if (dstr[i] == '0') {
			dstr[i] = '\0';
			i--;
		} else {
			i = 0;
		}
	}
	i = strlen(dstr) - 1;
	if (dstr[i] == '.')
		dstr[i] = '\0';
	sprintf(datest, "%s s", dstr);

	i = 10 - strlen(datest);
	if (i < 0)
		i = 0;
	else
		i = i / 2;
	write_icon(datest, 2 + i * 6, 14);
	RedrawWindowXYWH(2, 14, DIAMETRE - 1, DIAMETRE - 1);

	write_icon(" zoom :", 2, 26);
	RedrawWindowXYWH(2, 26, DIAMETRE - 1, DIAMETRE - 1);

	sprintf(datest, "%.3f", zoom);
	i = 10 - strlen(datest);
	if (i < 0)
		i = 0;
	else
		i = i / 2;
	write_icon(datest, 2 + i * 6, 38);
	RedrawWindowXYWH(2, 38, DIAMETRE - 1, DIAMETRE - 1);

	return;
}
/* ------------------------------------------------------------------------ */
static void move_delay(int factor)
{
	if (factor == 1) {
		if (delay < (double) VAL_USLEEP / 1000000)
			delay = (double) VAL_USLEEP / 1000000;
		else if (delay < 0.1) {
			delay += 0.01;
			if (delay > 0.1)
				delay = 0.2;
		} else if (delay < 1.) {
			delay += 0.1;
			if (delay > 1.0)
				delay = 2.;
		} else if (delay < 60.)
			delay += 1.;
		else if (delay < 300.)
			delay += 10.;
		else if (delay < 3600.)
			delay += 60.;
		else if (delay < MAX_DELAY_SEC)
			delay += 360.;

		if (delay > MAX_DELAY_SEC)
			delay = MAX_DELAY_SEC;
	} else {
		if (delay > 3600.)
			delay -= 360.;
		else if (delay > 300.)
			delay -= 60.;
		else if (delay > 60.)
			delay -= 10.;
		else if (delay > 1.)
			delay -= 1.;
		else if (delay > 0.1)
			delay -= 0.1;
		else if (delay > (double) VAL_USLEEP / 1000000)
			delay -= 0.01;
		else
			delay = 0.0;

		if (delay < (double) VAL_USLEEP / 1000000)
			delay = 0.0;
	}

	tdelay.tv_sec = (int) floor(delay);
	tdelay.tv_usec = (int) ((delay - tdelay.tv_sec) * 1000000);

	XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
		  0, 15, 60, 10, 2, 14);

	display_zoom();
	return;
}
/* ------------------------------------------------------------------------ */
void zooming(int facto)
{
	if (facto)
		zoom /= ZOOM_FACTOR;
	else
		zoom *= ZOOM_FACTOR;
	zoom = MAX(zoom, ZOOM_MIN);
	zoom = MIN(zoom, ZOOM_MAX);
	if (p_type == PTRANDOM) {
		addlong = dv_long;
		addlat = dv_lat;
		stoprand = STOP_RANDOM_FACTOR;
	}
	calcDistance();
	do_something = TRUE;
	return;
}
/* ------------------------------------------------------------------------ */
static void move_zoom(int factor)
{
	if (factor == -1)
		zooming(1);
	else
		zooming(0);
	XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
		  0, 15, 60, 10, 2, 38);
	display_zoom();
	return;
}
/* ------------------------------------------------------------------------ */
static void screen_3()
{
	int ok, ckm, waitrel, sensadd, not3;
	struct timeval tin;

#ifdef DEBUG
	fprintf(stdout, "scr 3\n");
#endif
	tin = getimev();
	waitrel = 0;
	ckm = 0;
	sensadd = 0;
	not3 = TRUE;
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);

	XCopyArea(dpy, scrdate.pixmap, wmg.pixmap, NormalGC, 0, 0, DIAMETRE, DIAMETRE, 0, 0);

	AddMouseRegion(1, 2, 14, DIAMETRE - 2, 25);
	AddMouseRegion(2, 2, 38, DIAMETRE - 2, 49);
	display_zoom();

	ok = TRUE;
	while (ok) {

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
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				waitrel = 1;
				press_but(ckm);
				switch (Event.xbutton.button) {
				case 1:
					not3 = TRUE;
					sensadd = 1;
					break;
				case 2:
					not3 = TRUE;
					sensadd = -1;
					break;
				case 3:
					not3 = FALSE;
					if (ckm < 5)
						gotoscr = 0;
					break;
				default:
					break;
				}
				break;
			case ButtonRelease:
				release_but(ckm);
				waitrel = ckm;
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				if (waitrel == ckm) {
					switch (ckm) {
					case 5:
						gotoscr--;
						if (gotoscr == 0)
							gotoscr = NUM_SCREEN;
						break;
					case 6:
						gotoscr++;
						if (gotoscr > NUM_SCREEN)
							gotoscr = 1;
						break;
					case 7:
						gotoscr = 0;
					default:
						break;
					}
				}
				ckm = 0;
				waitrel = 0;
				tin = getimev();
				break;
			default:
				break;

			}
		}
		usleep(VAL_USLEEP_SHORT);
		if (waitrel && not3) {
			if (ckm == 1) {
				move_delay(sensadd);
				usleep(2 * VAL_USLEEP);
			}
			if (ckm == 2) {
				move_zoom(sensadd);
				usleep(VAL_USLEEP);
			}
		}
		if (waitrel == 0 && diftimev(getimev(), tin).tv_sec > SCREEN_WAIT && gotoscr == 3)
			gotoscr = 0;
		if (gotoscr != 3)
			ok = FALSE;
	}
	return;
}
/* ------------------------------------------------------------------------ */
static void display_light()
{
	char datest[32];
	int i;

	write_icon(" light :", 2, 2);
	RedrawWindowXYWH(2, 2, DIAMETRE - 1, DIAMETRE - 1);

	sprintf(datest, "%.2f", ambient_light);
	i = 10 - strlen(datest);
	if (i < 0)
		i = 0;
	else
		i = i / 2;
	write_icon(datest, 2 + i * 6, 14);
	RedrawWindowXYWH(2, 14, DIAMETRE - 1, DIAMETRE - 1);

	write_icon(" dawn :", 2, 26);
	RedrawWindowXYWH(2, 26, DIAMETRE - 1, DIAMETRE - 1);

	dawn = (1.0 - dawn) * 2.0;
	sprintf(datest, "%.2f", dawn);
	dawn = (1.0 - dawn / 2.0);
	i = 10 - strlen(datest);
	if (i < 0)
		i = 0;
	else
		i = i / 2;
	write_icon(datest, 2 + i * 6, 38);
	RedrawWindowXYWH(2, 38, DIAMETRE - 1, DIAMETRE - 1);

	return;
}
/* ------------------------------------------------------------------------ */
static void move_light(int factor)
{
	ambient_light += factor / 20.0;
	ambient_light = MAX(0.0, ambient_light);
	ambient_light = MIN(1.0, ambient_light);
	aml = (int) floor(ambient_light * 256);

	XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
		  0, 15, 60, 10, 2, 14);

	display_light();
	return;
}
/* ------------------------------------------------------------------------ */
static void move_dawn(int factor)
{
	dawn = (1.0 - dawn) * 2.0;
	dawn += factor / 20.0;
	dawn = MAX(0.0, dawn);
	dawn = MIN(1.0, dawn);
	dawn = (1.0 - dawn / 2.0);
	XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
		  0, 15, 60, 10, 2, 38);

	display_light();
	return;
}
/* ------------------------------------------------------------------------ */
static void screen_4()
{
	int ok, ckm, waitrel, sensadd, not3;
	struct timeval tin;

#ifdef DEBUG
	fprintf(stdout, "scr 4\n");
#endif
	tin = getimev();
	waitrel = 0;
	ckm = 0;
	sensadd = 0;
	not3 = TRUE;
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);

	XCopyArea(dpy, scrdate.pixmap, wmg.pixmap, NormalGC, 0, 0, DIAMETRE, DIAMETRE, 0, 0);

	AddMouseRegion(1, 2, 14, DIAMETRE - 2, 25);
	AddMouseRegion(2, 2, 38, DIAMETRE - 2, 49);
	display_light();

	ok = TRUE;
	while (ok) {

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
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				waitrel = 1;
				press_but(ckm);
				switch (Event.xbutton.button) {
				case 1:
					not3 = TRUE;
					sensadd = 1;
					break;
				case 2:
					not3 = TRUE;
					sensadd = -1;
					break;
				case 3:
					not3 = FALSE;
					if (ckm < 5)
						gotoscr = 0;
					break;
				default:
					break;
				}
				break;
			case ButtonRelease:
				release_but(ckm);
				waitrel = ckm;
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				if (waitrel == ckm) {
					switch (ckm) {
					case 5:
						gotoscr--;
						if (gotoscr == 0)
							gotoscr = NUM_SCREEN;
						break;
					case 6:
						gotoscr++;
						if (gotoscr > NUM_SCREEN)
							gotoscr = 1;
						break;
					case 7:
						gotoscr = 0;
					default:
						break;
					}
				}
				ckm = 0;
				waitrel = 0;
				tin = getimev();
				break;
			default:
				break;
			}
		}
		usleep(VAL_USLEEP_SHORT);
		if (waitrel && not3) {
			if (ckm == 1) {
				move_light(sensadd);
				usleep(VAL_USLEEP);
			}
			if (ckm == 2) {
				move_dawn(sensadd);
				usleep(VAL_USLEEP);
			}
		}
		if (waitrel == 0 && diftimev(getimev(), tin).tv_sec > SCREEN_WAIT && gotoscr == 4)
			gotoscr = 0;
		if (gotoscr != 4)
			ok = FALSE;
	}
	return;
}
/* ------------------------------------------------------------------------ */
static void display_accel()
{
	char datest[32];
	int i;

	write_icon(" accel :", 2, 2);
	RedrawWindowXYWH(2, 2, DIAMETRE - 1, DIAMETRE - 1);

	if (time_multi < 24.0)
		sprintf(datest, "%.1f", time_multi);
	else
		sprintf(datest, "%.0f", time_multi);

	i = 10 - strlen(datest);
	if (i < 0)
		i = 0;
	else
		i = i / 2;
	write_icon(datest, 2 + i * 6, 14);
	RedrawWindowXYWH(2, 14, DIAMETRE - 1, DIAMETRE - 1);

	write_icon("night map:", 2, 26);
	RedrawWindowXYWH(2, 26, DIAMETRE - 1, DIAMETRE - 1);

	if (use_nightmap)
		write_icon("    yes   ", 2, 38);
	else
		write_icon("    no    ", 2, 38);

	RedrawWindowXYWH(2, 38, DIAMETRE - 1, DIAMETRE - 1);

	return;
}
/* ------------------------------------------------------------------------ */
static void move_accel(int factor)
{
	if (factor == 1) {
		if (time_multi < 1.0)
			time_multi = 1.0;
		if (time_multi < 24.0)
			time_multi += 0.2;
		else if (time_multi < 60.0) {
			time_multi = floor(time_multi);
			time_multi += 1.0;
		} else if (time_multi < 120.0) {
			time_multi = floor(time_multi);
			time_multi += 2.0;
		} else if (time_multi < 240.0) {
			time_multi = floor(time_multi);
			time_multi += 4.0;
		} else if (time_multi < 600.0) {
			time_multi = floor(time_multi);
			time_multi += 10.0;
		} else if (time_multi < 1200.0) {
			time_multi = floor(time_multi);
			time_multi += 20.0;
		} else if (time_multi < 2400.0) {
			time_multi = floor(time_multi);
			time_multi += 40.0;
		} else if (time_multi < 6000.0) {
			time_multi = floor(time_multi);
			time_multi += 100.0;
		} else if (time_multi < 12000.0) {
			time_multi = floor(time_multi);
			time_multi += 200.0;
		} else if (time_multi < 24000.0) {
			time_multi = floor(time_multi);
			time_multi += 400.0;
		} else if (time_multi < 60000.0) {
			time_multi = floor(time_multi);
			time_multi += 1000.0;
		} else if (time_multi < 120000.0) {
			time_multi = floor(time_multi);
			time_multi += 2000.0;
		} else if (time_multi < 240000.0) {
			time_multi = floor(time_multi);
			time_multi += 4000.0;
		} else {
			time_multi = floor(time_multi);
			time_multi += 8000.0;
		}
		if (time_multi > MAX_MULTI_COEF)
			time_multi = MAX_MULTI_COEF;
	} else {
		if (time_multi < 24.1)
			time_multi -= 0.2;
		else if (time_multi < 60.2) {
			time_multi = floor(time_multi);
			time_multi -= 1.0;
		} else if (time_multi < 121.0) {
			time_multi = floor(time_multi);
			time_multi -= 2.0;
		} else if (time_multi < 241.0) {
			time_multi = floor(time_multi);
			time_multi -= 4.0;
		} else if (time_multi < 601.0) {
			time_multi = floor(time_multi);
			time_multi -= 10.0;
		} else if (time_multi < 1201.0) {
			time_multi = floor(time_multi);
			time_multi -= 20.0;
		} else if (time_multi < 2401.0) {
			time_multi = floor(time_multi);
			time_multi -= 40.0;
		} else if (time_multi < 6001.0) {
			time_multi = floor(time_multi);
			time_multi -= 100.0;
		} else if (time_multi < 12001.0) {
			time_multi = floor(time_multi);
			time_multi -= 200.0;
		} else if (time_multi < 24001.0) {
			time_multi = floor(time_multi);
			time_multi -= 400.0;
		} else if (time_multi < 60001.0) {
			time_multi = floor(time_multi);
			time_multi -= 1000.0;
		} else if (time_multi < 120001.0) {
			time_multi = floor(time_multi);
			time_multi -= 2000.0;
		} else if (time_multi < 240001.0) {
			time_multi = floor(time_multi);
			time_multi -= 4000.0;
		} else {
			time_multi = floor(time_multi);
			time_multi -= 8000.0;
		}

		if (time_multi > 24.0)
			time_multi = floor(time_multi);
		if (time_multi < 1.0)
			time_multi = 1.0;
	}

	tini = tlast;
	tbase = trend;

	XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
		  0, 15, 60, 10, 2, 14);

	display_accel();
	return;
}
/* ------------------------------------------------------------------------ */
static void screen_5()
{
	int ok, ckm, waitrel, sensadd, not3;
	struct timeval tin;

#ifdef DEBUG
	fprintf(stdout, "scr 5\n");
#endif
	tin = getimev();
	waitrel = 0;
	ckm = 0;
	sensadd = 0;
	not3 = TRUE;
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);

	XCopyArea(dpy, scrdate.pixmap, wmg.pixmap, NormalGC, 0, 0, DIAMETRE, DIAMETRE, 0, 0);

	AddMouseRegion(1, 2, 14, DIAMETRE - 2, 25);
	AddMouseRegion(2, 2, 38, DIAMETRE - 2, 49);
	display_accel();

	ok = TRUE;
	while (ok) {

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
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				waitrel = 1;
				press_but(ckm);
				switch (Event.xbutton.button) {
				case 1:
					not3 = TRUE;
					sensadd = 1;
					break;
				case 2:
					not3 = TRUE;
					sensadd = -1;
					break;
				case 3:
					not3 = FALSE;
					if (ckm < 5)
						gotoscr = 0;
					break;
				default:
					break;
				}
				break;
			case ButtonRelease:
				release_but(ckm);
				waitrel = ckm;
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				if (waitrel == ckm) {
					switch (ckm) {
					case 5:
						gotoscr--;
						if (gotoscr == 0)
							gotoscr = NUM_SCREEN;
						break;
					case 6:
						gotoscr++;
						if (gotoscr > NUM_SCREEN)
							gotoscr = 1;
						break;
					case 7:
						gotoscr = 0;
					default:
						break;
					}
				}
				ckm = 0;
				waitrel = 0;
				tin = getimev();
				break;
			default:
				break;

			}
		}
		usleep(VAL_USLEEP_SHORT);
		if (waitrel && not3) {
			if (ckm == 1) {
				move_accel(sensadd);
				usleep(VAL_USLEEP);
			}
			if (ckm == 2) {
				not3 = FALSE;
				if (use_nightmap) {
					use_nightmap = FALSE;
					display_accel();
				} else {
					if (use_nmap_ini) {
						use_nightmap = TRUE;
						display_accel();
					}
				}
				usleep(VAL_USLEEP);
			}
		}
		if (waitrel == 0 && diftimev(getimev(), tin).tv_sec > SCREEN_WAIT && gotoscr == 5)
			gotoscr = 0;
		if (gotoscr != 5)
			ok = FALSE;
	}
	return;
}
/* ------------------------------------------------------------------------ */
static void display_dlat()
{
	char datest[32];
	int i, d, m, nega, nego;

	write_icon(" dlat :", 2, 2);
	RedrawWindowXYWH(2, 2, DIAMETRE - 1, DIAMETRE - 1);
	if (dlat < 0.0) {
		nega = TRUE;
		dlat *= -1;
	} else
		nega = FALSE;

	m = (int) floor((dlat - floor(dlat)) * 60.0);
	d = (int) floor(dlat);

	if (nega) {
		sprintf(datest, "-%d°%02d'", d, m);
		dlat *= -1;
	} else
		sprintf(datest, "%d°%02d'", d, m);

	i = 10 - strlen(datest);
	if (i < 0)
		i = 0;
	else
		i = i / 2;
	write_icon(datest, 2 + i * 6, 14);
	RedrawWindowXYWH(2, 14, DIAMETRE - 1, DIAMETRE - 1);

	write_icon(" dlong :", 2, 26);
	RedrawWindowXYWH(2, 26, DIAMETRE - 1, DIAMETRE - 1);


	if (dlong < 0.0) {
		nego = TRUE;
		dlong *= -1;
	} else
		nego = FALSE;

	m = (int) floor((dlong - floor(dlong)) * 60.0);
	d = (int) floor(dlong);

	if (nego) {
		sprintf(datest, "-%d°%02d'", d, m);
		dlong *= -1;
	} else
		sprintf(datest, "%d°%02d'", d, m);

	i = 10 - strlen(datest);
	if (i < 0)
		i = 0;
	else
		i = i / 2;
	write_icon(datest, 2 + i * 6, 38);
	RedrawWindowXYWH(2, 38, DIAMETRE - 1, DIAMETRE - 1);

	return;
}
/* ------------------------------------------------------------------------ */
static void move_dlat(int factor)
{
	dlat += factor / 4.0;
	if (dlat > MAX_DELTA_LONG)
		dlat = MAX_DELTA_LONG;
	if (dlat < -MAX_DELTA_LONG)
		dlat = -MAX_DELTA_LONG;
	if (dlat < 0.25 && dlat > -0.25)
		dlat = 0;
	XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
		  0, 15, 60, 10, 2, 14);
	if (dlat != 0.)
		p_type = PTFIXED;
	display_dlat();
	return;
}
/* ------------------------------------------------------------------------ */
static void move_dlong(int factor)
{
	dlong += factor / 4.0;
	if (dlong > MAX_DELTA_LONG)
		dlong = MAX_DELTA_LONG;
	if (dlong < -MAX_DELTA_LONG)
		dlong = -MAX_DELTA_LONG;
	if (dlong < 0.25 && dlong > -0.25)
		dlong = 0;
	XCopyArea(dpy, scrdiv.pixmap, wmg.pixmap, NormalGC,
		  0, 15, 60, 10, 2, 38);
	if (dlong != 0.)
		p_type = PTFIXED;
	display_dlat();
	return;
}
/* ------------------------------------------------------------------------ */
static void screen_6()
{
	int ok, ckm, waitrel, sensadd, not3;
	struct timeval tin;

#ifdef DEBUG
	fprintf(stdout, "scr 6\n");
#endif
	tin = getimev();
	waitrel = 0;
	sensadd = 0;
	ckm = 0;
	not3 = TRUE;
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);

	XCopyArea(dpy, scrdate.pixmap, wmg.pixmap, NormalGC, 0, 0, DIAMETRE, DIAMETRE, 0, 0);
	AddMouseRegion(1, 2, 14, DIAMETRE - 2, 25);
	AddMouseRegion(2, 2, 38, DIAMETRE - 2, 49);

	display_dlat();

	ok = TRUE;
	while (ok) {

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
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				waitrel = 1;
				press_but(ckm);
				switch (Event.xbutton.button) {
				case 1:
					not3 = TRUE;
					sensadd = 1;
					break;
				case 2:
					not3 = TRUE;
					sensadd = -1;
					break;
				case 3:
					not3 = FALSE;;
					if (ckm < 5)
						gotoscr = 0;
					break;
				default:
					break;
				}
				break;
			case ButtonRelease:
				release_but(ckm);
				waitrel = ckm;
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				if (waitrel == ckm) {
					switch (ckm) {
					case 5:
						gotoscr--;
						if (gotoscr == 0)
							gotoscr = NUM_SCREEN;
						break;
					case 6:
						gotoscr++;
						if (gotoscr > NUM_SCREEN)
							gotoscr = 1;
						break;
					case 7:
						gotoscr = 0;
					default:
						break;
					}
				}
				ckm = 0;
				waitrel = 0;
				tin = getimev();
				break;
			default:
				break;

			}
		}
		usleep(VAL_USLEEP_SHORT);
		if (waitrel && not3) {
			if (ckm == 1 && p_type != PTRANDOM) {
				move_dlat(sensadd);
				usleep(VAL_USLEEP);
			}
			if (ckm == 2 && p_type != PTRANDOM) {
				move_dlong(sensadd);
				usleep(VAL_USLEEP);
			}
		}
		if (waitrel == 0 && diftimev(getimev(), tin).tv_sec > SCREEN_WAIT && gotoscr == 6)
			gotoscr = 0;
		if (gotoscr != 6)
			ok = FALSE;
	}
	return;
}
/* ------------------------------------------------------------------------ */
static void display_type()
{
	char c, cc, mess[12];

	write_icon(" view :", 2, 2);
	RedrawWindowXYWH(2, 2, DIAMETRE - 1, DIAMETRE - 1);

	if (p_type == PTFIXED) {
		c = '>';
		cc = '<';
	} else {
		c = ' ';
		cc = ' ';
	}
	sprintf(mess, "%c  move  %c", c, cc);
	write_icon(mess, 2, 14);
	RedrawWindowXYWH(2, 14, DIAMETRE - 1, DIAMETRE - 1);
	if (p_type == PTSUNREL) {
		c = '>';
		cc = '<';
	} else {
		c = ' ';
		cc = ' ';
	}
	sprintf(mess, "%c  sun   %c", c, cc);
	write_icon(mess, 2, 26);
	RedrawWindowXYWH(2, 26, DIAMETRE - 1, DIAMETRE - 1);
	if (p_type == PTRANDOM) {
		c = '>';
		cc = '<';
	} else {
		c = ' ';
		cc = ' ';
	}
	sprintf(mess, "%c random %c", c, cc);
	write_icon(mess, 2, 38);
	RedrawWindowXYWH(2, 38, DIAMETRE - 1, DIAMETRE - 1);

	return;
}
/* ------------------------------------------------------------------------ */
static void screen_7()
{
	int ok, ckm, waitrel, sensadd, not3;
	struct timeval tin;

#ifdef DEBUG
	fprintf(stdout, "scr 7\n");
#endif
	tin = getimev();
	waitrel = 0;
	sensadd = 0;
	ckm = 0;
	not3 = TRUE;

	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, scrdate.mask, ShapeSet);

	XCopyArea(dpy, scrdate.pixmap, wmg.pixmap, NormalGC, 0, 0, DIAMETRE, DIAMETRE, 0, 0);
	AddMouseRegion(1, 2, 14, DIAMETRE - 2, 25);
	AddMouseRegion(2, 2, 26, DIAMETRE - 2, 37);
	AddMouseRegion(3, 2, 38, DIAMETRE - 2, 49);

	display_type(0);

	ok = TRUE;
	while (ok) {

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
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				waitrel = 1;
				press_but(ckm);
				switch (Event.xbutton.button) {
				case 1:
					not3 = TRUE;
					if (ckm < 4 && ckm > 0)
						sensadd = ckm;
					break;
				case 2:
					not3 = TRUE;
					break;
				case 3:
					not3 = FALSE;
					if (ckm < 5)
						gotoscr = 0;
					break;
				default:
					break;
				}
				break;
			case ButtonRelease:
				release_but(ckm);
				waitrel = ckm;
				ckm = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				if (waitrel == ckm) {
					switch (ckm) {
					case 5:
						gotoscr--;
						if (gotoscr == 0)
							gotoscr = NUM_SCREEN;
						break;
					case 6:
						gotoscr++;
						if (gotoscr > NUM_SCREEN)
							gotoscr = 1;
						break;
					case 7:
						gotoscr = 0;
					default:
						break;
					}
				}
				ckm = 0;
				waitrel = 0;
				tin = getimev();
				break;
			default:
				break;
			}
		}
		usleep(VAL_USLEEP_SHORT);
		if (waitrel && not3 && sensadd) {
			if (sensadd == 2) {
				if (p_type != PTSUNREL) {
					p_type = PTSUNREL;
					dlat = 0;
					dlong = 0;
					display_type();
				}
			}
			if (sensadd == 1) {
				if (p_type != PTFIXED) {
					p_type = PTFIXED;
					display_type();
				}
			}
			if (sensadd == 3) {
				if (p_type != PTRANDOM) {
					p_type = PTRANDOM;
					dlat = 0;
					dlong = 0;
					display_type();
				}
			}
		}
		sensadd = 0;
		if (waitrel == 0 && diftimev(getimev(), tin).tv_sec > SCREEN_WAIT && gotoscr == 7)
			gotoscr = 0;
		if (gotoscr != 7)
			ok = FALSE;
	}
	return;
}
/* ------------------------------------------------------------------------ */
