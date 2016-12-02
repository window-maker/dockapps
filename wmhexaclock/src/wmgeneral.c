#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include "wmgeneral.h"

/* X11 Variables */
int		screen;
int		x_fd;
int		d_depth;
XSizeHints	mysizehints;
XWMHints	mywmhints;
Pixel		back_pix, fore_pix;
char		*Geometry = "";
Window		iconwin, win;
Pixmap		pixmask;


/* Function Prototypes */
static void GetXPM(XpmIcon *, char **);
static Pixel GetColor(char *);
void RedrawWindow(void);


static void GetXPM(XpmIcon *wmgen, char *pixmap_bytes[])
{
	XWindowAttributes	attributes;
	int			err;

	/* For the colormap */
	XGetWindowAttributes(display, Root, &attributes);

	wmgen->attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);

	err = XpmCreatePixmapFromData(display, Root, pixmap_bytes, &(wmgen->pixmap),
		&(wmgen->mask), &(wmgen->attributes));

	if (err != XpmSuccess) {
		fprintf(stderr, "Not enough free colorcells.\n");
		exit(1);
	}
}

static Pixel GetColor(char *name)
{
	XColor		color;
	XWindowAttributes	attributes;

	XGetWindowAttributes(display, Root, &attributes);

	color.pixel = 0;
	if (!XParseColor(display, attributes.colormap, name, &color)) {
		fprintf(stderr, "wm.app: can't parse %s.\n", name);
	} else if (!XAllocColor(display, attributes.colormap, &color)) {
		fprintf(stderr, "wm.app: can't allocate %s.\n", name);
	}
	return color.pixel;
}

static int flush_expose(Window w)
{
	XEvent	dummy;
	int		i=0;

	while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
		i++;
	return i;
}

void RedrawWindow(void)
{
	flush_expose(iconwin);
	XCopyArea(display, wmgen.pixmap, iconwin, NormalGC,
		0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
	flush_expose(win);
	XCopyArea(display, wmgen.pixmap, win, NormalGC,
		0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}


void copyXPMArea(int x, int y, int sx, int sy, int dx, int dy)
{
	XCopyArea(display, wmnumbers.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}

void cleanXPMArea()
{
    XCopyArea(display, wmempty.pixmap, wmgen.pixmap, NormalGC, 0,0,64,64,0,0);
}

void openXwindow(int argc, char *argv[], char *pixmap_bytes_numbers[], char *pixmap_bytes_background[], char pixmask_bits[], int pixmask_width, int pixmask_height)
{
	unsigned int	borderwidth = 1;
	XClassHint	classHint;
	char		*display_name = NULL;
	char		*wname = argv[0];
	XTextProperty	name;
	XGCValues	gcv;
	unsigned long	gcm;
	int		dummy = 0;

	if (!(display = XOpenDisplay(display_name))) {
		fprintf(stderr, "%s: can't open display %s\n",
			wname, XDisplayName(display_name));
		exit(1);
	}
	screen  = DefaultScreen(display);
	Root    = RootWindow(display, screen);
	d_depth = DefaultDepth(display, screen);
	x_fd    = XConnectionNumber(display);

	/* Convert XPM to XImage */
	GetXPM(&wmgen, pixmap_bytes_background);
    	GetXPM(&wmempty, pixmap_bytes_background);
    	GetXPM(&wmnumbers,pixmap_bytes_numbers);

	/* Create a window to hold the stuff */
	mysizehints.flags = USSize | USPosition;
	mysizehints.x = 0;
	mysizehints.y = 0;

	back_pix = GetColor("white");
	fore_pix = GetColor("black");

	XWMGeometry(display, screen, Geometry, NULL, borderwidth, &mysizehints,	&mysizehints.x, &mysizehints.y,&mysizehints.width,&mysizehints.height, &dummy);

	mysizehints.width = 64;
	mysizehints.height = 64;

	win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,	mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

	iconwin = XCreateSimpleWindow(display, win, mysizehints.x, mysizehints.y, mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

	/* Activate hints */
	XSetWMNormalHints(display, win, &mysizehints);
	classHint.res_name = wname;
	classHint.res_class = wname;
	XSetClassHint(display, win, &classHint);

	XSelectInput(display, win, ButtonPressMask | ExposureMask | ButtonReleaseMask |	PointerMotionMask | StructureNotifyMask);
	XSelectInput(display, iconwin, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

	if (XStringListToTextProperty(&wname, 1, &name) == 0) {
		fprintf(stderr, "%s: can't allocate window name\n", wname);
		exit(1);
	}

	XSetWMName(display, win, &name);

	/* Create GC for drawing */
	gcm = GCForeground | GCBackground | GCGraphicsExposures;
	gcv.foreground = fore_pix;
	gcv.background = back_pix;
	gcv.graphics_exposures = 0;
	NormalGC = XCreateGC(display, Root, gcm, &gcv);

	/* ONLYSHAPE ON */
	pixmask = XCreateBitmapFromData(display, win, pixmask_bits, pixmask_width, pixmask_height);
	XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
	XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);

	/* ONLYSHAPE OFF */
	mywmhints.initial_state = WithdrawnState;
	mywmhints.icon_window = iconwin;
	mywmhints.icon_x = mysizehints.x;
	mywmhints.icon_y = mysizehints.y;
	mywmhints.window_group = win;
	mywmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;

	XSetWMHints(display, win, &mywmhints);

	XSetCommand(display, win, argv, argc);
	XMapWindow(display, win);
}

