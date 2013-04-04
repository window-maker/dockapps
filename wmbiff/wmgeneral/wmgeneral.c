/*
	Best viewed with vim5, using ts=4

	wmgeneral was taken from wmppp.

	It has a lot of routines which most of the wm* programs use.

	------------------------------------------------------------

	Author: Martijn Pieterse (pieterse@xs4all.nl)

	---
	CHANGES:
    ---
    14/09/1998 (Dave Clark, clarkd@skyia.com)
        * Updated createXBMfromXPM routine
        * Now supports >256 colors
	11/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Removed a bug from parse_rcfile. You could
		  not use "start" in a command if a label was 
		  also start.
		* Changed the needed geometry string.
		  We don't use window size, and don't support
		  negative positions.
	03/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added parse_rcfile2
	02/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added -geometry support (untested)
	28/08/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Added createXBMfromXPM routine
		* Saves a lot of work with changing xpm's.
	02/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* changed the read_rc_file to parse_rcfile, as suggested by Marcelo E. Magallon
		* debugged the parse_rc file.
	30/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
		* Ripped similar code from all the wm* programs,
		  and put them in a single file.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>

#include <X11/Xlib.h>
#ifdef HAVE_X11_XPM_H
#include <X11/xpm.h>
#endif
#ifdef HAVE_XPM_H
#include <xpm.h>
#endif
#include <X11/Xutil.h>			/* needed for Region on solaris? */
#include <X11/extensions/shape.h>

#include "wmgeneral.h"

  /*****************/
 /* X11 Variables */
/*****************/

Window Root;
int screen;
int x_fd;
int d_depth;
XSizeHints mysizehints;
XWMHints mywmhints;
Pixel back_pix, fore_pix;
// static const char *Geometry = "";
Window iconwin, win;
GC NormalGC;
XpmIcon wmgen_bkg;
XpmIcon wmgen_src;
Pixmap pixmask;

  /*****************/
 /* Mouse Regions */
/*****************/

typedef struct {
	int enable;
	int top;
	int bottom;
	int left;
	int right;
} MOUSE_REGION;

MOUSE_REGION mouse_region[MAX_MOUSE_REGION];

  /***********************/
 /* Function Prototypes */
/***********************/

static void GetXPM(XpmIcon *, const char **);
Pixel GetColor(const char *);
void RedrawWindow(void);
int CheckMouseRegion(int, int);

/*******************************************************************************\
|* parse_rcfile																   *|
\*******************************************************************************/

void parse_rcfile(const char *filename, rckeys * keys)
{

	char *p, *q;
	char temp[128];
	const char *tokens = " :\t\n";
	FILE *fp;
	int i, key;

	fp = fopen(filename, "r");
	if (fp) {
		while (fgets(temp, 128, fp)) {
			key = 0;
			q = strdup(temp);
			q = strtok(q, tokens);
			while (key >= 0 && keys[key].label) {
				if ((!strcmp(q, keys[key].label))) {
					p = strstr(temp, keys[key].label);
					p += strlen(keys[key].label);
					p += strspn(p, tokens);
					if ((i = strcspn(p, "#\n")))
						p[i] = 0;
					free(*keys[key].var);
					*keys[key].var = strdup(p);
					key = -1;
				} else
					key++;
			}
			free(q);
		}
		fclose(fp);
	}
}

/*******************************************************************************\
|* parse_rcfile2															   *|
\*******************************************************************************/

void parse_rcfile2(const char *filename, rckeys2 * keys)
{

	char *p;
	char temp[128];
	const char *tokens = " :\t\n";
	FILE *fp;
	int i, key;
	char *family = NULL;

	fp = fopen(filename, "r");
	if (fp) {
		while (fgets(temp, 128, fp)) {
			key = 0;
			while (key >= 0 && keys[key].label) {
				if ((p = strstr(temp, keys[key].label))) {
					p += strlen(keys[key].label);
					p += strspn(p, tokens);
					if ((i = strcspn(p, "#\n")))
						p[i] = 0;
					free(*keys[key].var);
					*keys[key].var = strdup(p);
					key = -1;
				} else
					key++;
			}
		}
		fclose(fp);
	}
	free(family);
}


/*******************************************************************************\
|* GetXPM																	   *|
\*******************************************************************************/

static void GetXPM(XpmIcon * wmgen_local, const char *pixmap_bytes[])
{

	XWindowAttributes attributes;
	int err;

	/* For the colormap */
	XGetWindowAttributes(display, Root, &attributes);
	/* despite the comment, I still don't understand... 
	   attributes is subsequently unused in this function -ns 11/2002 */

	wmgen_local->attributes.valuemask |=
		(XpmReturnPixels | XpmReturnExtensions);

	err = XpmCreatePixmapFromData(display, Root, (char **) pixmap_bytes,
								  &(wmgen_local->pixmap),
								  &(wmgen_local->mask),
								  &(wmgen_local->attributes));

	if (err != XpmSuccess) {
		fprintf(stderr,
				"Not enough free colorcells to create pixmap from data (err=%d).\n",
				err);
		exit(1);
	}
}

/*******************************************************************************\
|* GetColor																	   *|
\*******************************************************************************/

Pixel GetColor(const char *name)
{

	XColor color;
	XWindowAttributes attributes;

	XGetWindowAttributes(display, Root, &attributes);

	color.pixel = 0;
	if (!XParseColor(display, attributes.colormap, name, &color)) {
		fprintf(stderr, "wm.app: GetColor() can't parse %s.\n", name);
	} else if (!XAllocColor(display, attributes.colormap, &color)) {
		fprintf(stderr, "wm.app: GetColor() can't allocate %s.\n", name);
	}
	return color.pixel;
}

/*******************************************************************************\
|* flush_expose																   *|
\*******************************************************************************/

static int flush_expose(Window w)
{

	XEvent dummy;
	int i = 0;

	while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
		i++;

	return i;
}

/*******************************************************************************\
|* RedrawWindow																   *|
\*******************************************************************************/

void RedrawWindow(void)
{

	flush_expose(iconwin);
	XCopyArea(display, wmgen_bkg.pixmap, iconwin, NormalGC,
			  0, 0, wmgen_bkg.attributes.width,
			  wmgen_bkg.attributes.height, 0, 0);
	flush_expose(win);
	XCopyArea(display, wmgen_bkg.pixmap, win, NormalGC,
			  0, 0, wmgen_bkg.attributes.width,
			  wmgen_bkg.attributes.height, 0, 0);
}

/*******************************************************************************\
|* RedrawWindowXY															   *|
\*******************************************************************************/

void RedrawWindowXY(int x, int y)
{

	flush_expose(iconwin);
	XCopyArea(display, wmgen_bkg.pixmap, iconwin, NormalGC,
			  x, y, wmgen_bkg.attributes.width,
			  wmgen_bkg.attributes.height, 0, 0);
	flush_expose(win);
	XCopyArea(display, wmgen_bkg.pixmap, win, NormalGC,
			  x, y, wmgen_bkg.attributes.width,
			  wmgen_bkg.attributes.height, 0, 0);
}

/*******************************************************************************\
|* AddMouseRegion															   *|
\*******************************************************************************/

void AddMouseRegion(unsigned int region_idx, int left, int top, int right,
					int bottom)
{

	if (region_idx < MAX_MOUSE_REGION) {
		mouse_region[region_idx].enable = 1;
		mouse_region[region_idx].top = top;
		mouse_region[region_idx].left = left;
		mouse_region[region_idx].bottom = bottom;
		mouse_region[region_idx].right = right;
	}
}

/*******************************************************************************\
|* CheckMouseRegion															   *|
\*******************************************************************************/

int CheckMouseRegion(int x, int y)
{

	int i;
	int found;

	found = 0;

	for (i = 0; i < MAX_MOUSE_REGION && !found; i++) {
		if (mouse_region[i].enable &&
			x <= mouse_region[i].right &&
			x >= mouse_region[i].left &&
			y <= mouse_region[i].bottom && y >= mouse_region[i].top)
			found = 1;
	}
	if (!found)
		return -1;
	return (i - 1);
}

/*******************************************************************************\
|* createXBMfromXPM															   *|
\*******************************************************************************/
void createXBMfromXPM(char *xbm, const char **xpm, int sx, int sy)
{

	int i, j, k;
	int width, height, numcol, depth;
	int zero = 0;
	unsigned char bwrite;
	int bcount;
	int curpixel;

	sscanf(*xpm, "%d %d %d %d", &width, &height, &numcol, &depth);


	for (k = 0; k != depth; k++) {
		zero <<= 8;
		zero |= xpm[1][k];
	}

	for (i = numcol + 1; i < numcol + sy + 1; i++) {
		bcount = 0;
		bwrite = 0;
		for (j = 0; j < sx * depth; j += depth) {
			bwrite >>= 1;

			curpixel = 0;
			for (k = 0; k != depth; k++) {
				curpixel <<= 8;
				curpixel |= xpm[i][j + k];
			}

			if (curpixel != zero) {
				bwrite += 128;
			}
			bcount++;
			if (bcount == 8) {
				*xbm = bwrite;
				xbm++;
				bcount = 0;
				bwrite = 0;
			}
		}
	}
}

/*******************************************************************************\
|* copyXPMArea																   *|
\*******************************************************************************/

void copyXPMArea(int src_x, int src_y, int width, int height, int dest_x,
				 int dest_y)
{

	XCopyArea(display, wmgen_src.pixmap, wmgen_bkg.pixmap, NormalGC, src_x,
			  src_y, width, height, dest_x, dest_y);

}

/*******************************************************************************\
|* copyXBMArea																   *|
\*******************************************************************************/

void copyXBMArea(int src_x, int src_y, int width, int height, int dest_x,
				 int dest_y)
{

	XCopyArea(display, wmgen_src.mask, wmgen_bkg.pixmap, NormalGC, src_x,
			  src_y, width, height, dest_x, dest_y);
}


/* added for wmbiff */
XFontStruct *f;
int loadFont(const char *fontname)
{
	if (display != NULL) {
		f = XLoadQueryFont(display, fontname);
		if (f) {
			XSetFont(display, NormalGC, f->fid);
			return 0;
		} else {
			printf("couldn't set font!\n");
		}
	}
	return -1;
}

void drawString(int dest_x, int dest_y, const char *string,
				const char *colorname, const char *bgcolorname,
				int right_justify)
{
	int len = strlen(string);
	assert(colorname != NULL);
	XSetForeground(display, NormalGC, GetColor(colorname));
	XSetBackground(display, NormalGC, GetColor(bgcolorname));
	if (right_justify)
		dest_x -= XTextWidth(f, string, len);
	XDrawImageString(display, wmgen_bkg.pixmap, NormalGC, dest_x, dest_y,
					 string, len);
}

void eraseRect(int x, int y, int x2, int y2, const char *bgcolorname)
{
	XSetForeground(display, NormalGC, GetColor(bgcolorname));
	XFillRectangle(display, wmgen_bkg.pixmap, NormalGC, x, y, x2 - x,
				   y2 - y);
}

/* end wmbiff additions */

/*******************************************************************************\
|* setMaskXY																   *|
\*******************************************************************************/

void setMaskXY(int x, int y)
{

	XShapeCombineMask(display, win, ShapeBounding, x, y, pixmask,
					  ShapeSet);
	XShapeCombineMask(display, iconwin, ShapeBounding, x, y, pixmask,
					  ShapeSet);
}

/*******************************************************************************\
|* openXwindow																   *|
\*******************************************************************************/
void openXwindow(int argc, const char *argv[],
				 const char *pixmap_bytes_bkg[],
				 const char *pixmap_bytes_src[], char *pixmask_bits,
				 int pixmask_width, int pixmask_height, int notWithdrawn)
{

	unsigned int borderwidth = 1;
	XClassHint classHint;
	const char *display_name = NULL;
	char *wname = strdup(argv[0]);
	XTextProperty name;

	XGCValues gcv;
	unsigned long gcm;

	const char *geometry = NULL;
	char default_geometry[128];

	int dummy = 0;
	int i;

	if (!wname) {
		fprintf(stderr, "Unable to allocate memory for window name!\n");
		abort();
	}

	for (i = 1; argv[i]; i++) {
		if (!strcmp(argv[i], "-display")) {
			display_name = argv[i + 1];
			i++;
		}
		if (!strcmp(argv[i], "-geometry")) {
			geometry = argv[i + 1];
			i++;
		}
	}

	sprintf(default_geometry, "%dx%d+0+0", pixmask_width, pixmask_height);

	if (!(display = XOpenDisplay(display_name))) {
		fprintf(stderr, "%s: can't open display %s\n",
				wname, XDisplayName(display_name));
		exit(1);
	}
	screen = DefaultScreen(display);
	Root = RootWindow(display, screen);
	d_depth = DefaultDepth(display, screen);
	x_fd = XConnectionNumber(display);

	/* Convert XPM to XImage */
	GetXPM(&wmgen_bkg, pixmap_bytes_bkg);
	GetXPM(&wmgen_src, pixmap_bytes_src);

	/* Create a window to hold the stuff */
	mysizehints.flags = USSize | USPosition;
	mysizehints.x = 0;
	mysizehints.y = 0;

	back_pix = GetColor("black");
	fore_pix = GetColor("cyan");

	XWMGeometry(display, screen, geometry, default_geometry, borderwidth,
				&mysizehints, &mysizehints.x, &mysizehints.y,
				&mysizehints.width, &mysizehints.height, &dummy);

	mysizehints.width = pixmask_width;	/* changed 11/2002 for wmbiff non 64x64-ness */
	mysizehints.height = pixmask_height;	/* was statically 64. */

	win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,
							  mysizehints.width, mysizehints.height,
							  borderwidth, fore_pix, back_pix);

	iconwin =
		XCreateSimpleWindow(display, win, mysizehints.x, mysizehints.y,
							mysizehints.width, mysizehints.height,
							borderwidth, fore_pix, back_pix);

	/* Activate hints */
	XSetWMNormalHints(display, win, &mysizehints);
	classHint.res_name = wname;
	classHint.res_class = wname;
	XSetClassHint(display, win, &classHint);

    /* Was PointerMotionMask instead of KeyPressMask, but pointer motion is irrelevant,
       and if the user went to the trouble of giving us keypresses, the least we can do
       is handle em... */
	XSelectInput(display, win,
				 ButtonPressMask | ExposureMask | ButtonReleaseMask |
				 KeyPressMask | StructureNotifyMask);
	XSelectInput(display, iconwin,
				 ButtonPressMask | ExposureMask | ButtonReleaseMask |
				 KeyPressMask | StructureNotifyMask);

	/* wname is argv[0] */
	if (XStringListToTextProperty(&wname, 1, &name) == 0) {
		fprintf(stderr, "%s: can't allocate window name\n", wname);
		exit(1);
	}

	XSetWMName(display, win, &name);
	XFree(name.value);

	/* Create GC for drawing */

	gcm = GCForeground | GCBackground | GCGraphicsExposures;
	gcv.foreground = fore_pix;
	gcv.background = back_pix;
	gcv.graphics_exposures = 0;
	NormalGC = XCreateGC(display, Root, gcm, &gcv);

	/* ONLYSHAPE ON */

	pixmask =
		XCreateBitmapFromData(display, win, pixmask_bits, pixmask_width,
							  pixmask_height);

	XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask,
					  ShapeSet);
	XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask,
					  ShapeSet);

	/* ONLYSHAPE OFF */

	mywmhints.initial_state = WithdrawnState;
	mywmhints.icon_window = iconwin;
	mywmhints.icon_x = mysizehints.x;
	mywmhints.icon_y = mysizehints.y;
	mywmhints.window_group = win;
	mywmhints.flags =
		(notWithdrawn ? 0 : StateHint) | IconWindowHint |
		IconPositionHint | WindowGroupHint;

	XSetWMHints(display, win, &mywmhints);

	XSetCommand(display, win, (char **) argv, argc);
	XMapWindow(display, win);

	if (geometry) {
		/* we'll silently drop width and height as well as negative positions */
		/* mostly because I don't know how to deal with them */
		/*
		   int wx, wy, x, y;
		   int specified = XParseGeometry(geometry, &x, &y, &wx, &wy);
		   printf("%d %d %d %d\n", x, y, wx, wy);
		   if( specified & XNegative ) {
		   x = DisplayWidth(display, DefaultScreen(display)) - x - pixmask_width;
		   }
		   if( specified & YNegative ) {
		   y = DisplayHeight(display, DefaultScreen(display)) - y - pixmask_height;
		   }
		   if( specified & XValue || specified & YValue ) { 
		   XMoveWindow(display, win, x, y);
		   } */

		/*
		   if (sscanf(geometry, "+%d+%d", &wx, &wy) == 2) {
		   XMoveWindow(display, win, wx, wy);
		   } else if (sscanf(geometry, "%dx%d+%d+%d", &x, &y, &wx, &wy) == 4) {
		   XMoveWindow(display, win, wx, wy);
		   } else if (sscanf(geometry, "+%d-%d", &wx, &wy) == 2) {
		   XMoveWindow(display, win, wx, 0 - wy);
		   }  else {
		   fprintf(stderr, "Unsupported geometry string '%s'\n",
		   geometry);
		   exit(1);
		   } */
	}

	if (wname)
		free(wname);
}
