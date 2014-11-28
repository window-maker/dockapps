/*
 * Copyright (c) 2002 Alban G. Hertroys
 *
 * libDockapp example - Usage of action rectangles
 *
 * Some clickable and draggable areas for you to play
 * with.
 *
 * There's a bit much in here...
 */

/* The cursor font - stamdard cursor glyphs. */
#include <X11/cursorfont.h>

/* Required because we don't use a pixmap for the shape (using a DASetShape
 * variation). Instead we use the XLib call "XShapeCombineRegion".
 * Window shapes are an extension (since X11R5).
 */
#include <X11/extensions/shape.h>

#include <dockapp.h>

/* already includes Xlib, Xresources, XPM, stdlib and stdio */

/*
 * Type definitions
 */

/* I like to keep my graphic contexts (GCs) together */
struct Colors {
	GC white;       /* foreground color from X-resource, or default */
	GC black;       /* background color, idem */
	GC lightGray;   /* Some GC's we'll have to define for colors */
	GC darkGray;
	GC slider;      /* draw-color when not highlighted,
			 * dark-color when highlighted
			 */
	GC sliderLight; /* draw-color when highlighted */
	GC sliderDark;  /* dark-color when not highlighted */
};


/*
 * Global variables
 */
Pixmap pixmap;          /* pixmap pixmap */
DARect *buttonDown = NULL;
struct Colors *colors;  /* our colors */
DAActionRect **actionRects;
float sliderPos = 0.7;
int mouseIn = 0;
Cursor pointer;

/*
 * Prototypes for local functions
 */
struct Colors *setGCs(Drawable d);
unsigned long adjustColor(unsigned long color, signed int adjustment);

/* drawing routines */
void createBtn(DARect rect);
void drawRaisedFrame(DARect rect);
void drawSunkenFrame(DARect rect);
void createSquare(DARect rect);
void drawSquare(DARect rect, int button);
void createSlider(DARect rect);
void drawSlider(DARect rect);
void setPointerColor(GC);

/* utility functions */
DAActionRect setRectAction(DARect rect, DARectCallback action);


/* event handlers functions */
void destroy(void);
void buttonPress(int button, int state, int x, int y);
void buttonRelease(int button, int state, int x, int y);
void mouseMove(int x, int y);
void mouseEnter(void);
void mouseLeave(void);

/* what to do for a specific event for every 'item' in the dockapp */
/*	Button that can be pressed "down" and jumps back "up" again */
void btnDown(int x, int y, DARect rect, void *data);
void btnUp(int x, int y, DARect rect, void *data);
void btnLeave(int x, int y, DARect rect, void *data);

/*	Square that tells which button was pressed (number) */
void squareDown(int x, int y, DARect rect, void *data);

/*	A draggable slider that highlights when the mouse is over it */
void sliderDown(int x, int y, DARect rect, void *data);
void sliderUp(int x, int y, DARect rect, void *data);
void sliderMove(int x, int y, DARect rect, void *data);
void sliderEnter(int x, int y, DARect rect, void *data);
void sliderLeave(int x, int y, DARect rect, void *data);


/*
 * M A I N
 */

int
main(int argc, char **argv)
{
	/* define the event handlers for the events */
	DACallbacks eventCallbacks = {
		destroy, /* destroy */
		buttonPress, /* buttonPress */
		buttonRelease, /* buttonRelease */
		mouseMove, /* motion (mouse) */
		mouseEnter, /* mouse enters window */
		mouseLeave, /* mouse leaves window */
		NULL    /* timeout */
	};

	/* define regions (x, y, width, height) that need event-handling */
	Region clipRegion = XCreateRegion();

	DARect btn = {0, 0, 16, 16},
	       square = {0, 25, 22, 22},
	       slider = {24, 0, 23, 48};

	/* define what to do if an event occurs in a rectangle */
	DAActionRect *buttonPressRects, *buttonReleaseRects,
	*mouseMoveRects, *mouseEnterRects, *mouseLeaveRects;

	buttonPressRects = malloc(3 * sizeof(DAActionRect));
	buttonPressRects[0] = setRectAction(btn, btnDown);
	buttonPressRects[1] = setRectAction(square, squareDown);
	buttonPressRects[2] = setRectAction(slider, sliderDown);

	buttonReleaseRects = malloc(2 * sizeof(DAActionRect));
	buttonReleaseRects[0] = setRectAction(btn, btnUp);
	buttonReleaseRects[1] = setRectAction(slider, sliderUp);

	mouseMoveRects = malloc(sizeof(DAActionRect));
	mouseMoveRects[0] = setRectAction(slider, sliderMove);

	mouseEnterRects = malloc(sizeof(DAActionRect));
	mouseEnterRects[0] = setRectAction(slider, sliderEnter);

	mouseLeaveRects = malloc(2 * sizeof(DAActionRect));
	mouseLeaveRects[0] = setRectAction(btn, btnLeave);
	mouseLeaveRects[1] = setRectAction(slider, sliderLeave);


	/* XXX: make action rectangles available outside main()
	 * ...libDockapp should be able to do this... (reminder)
	 */
	actionRects = malloc(6 * sizeof(DAActionRect *));
	actionRects[0] = buttonPressRects;
	actionRects[1] = buttonReleaseRects;
	actionRects[2] = mouseMoveRects;
	actionRects[3] = mouseEnterRects;
	actionRects[4] = mouseLeaveRects;
	actionRects[5] = NULL;

	/* provide standard command-line options */
	DAParseArguments(
		argc, argv, /* Where the options come from */
		NULL, 0, /* Our list with options - none as you can see */
		"This is the help text for the rectangle example of how to "
		"use libDockapp.\n",
		"Rectangle example version 1.0");

	/* Tell libdockapp what version we expect it to be, so that you can use
	 * older programs with newer versions of libdockapp with less risc for
	 * compatibility problems.
	 */
	DASetExpectedVersion(20030126);

	/* Initialize a dockapp */
	DAInitialize(
		"",             /* Use default display */
		"daRectangleExample",   /* WM_CLASS hint; don't use chars in [.?*: ] */
		48, 48,         /* geometry of dockapp internals */
		argc, argv      /* (needed internally) */
		);

	/* Create a pixmap to draw on, and to display */
	pixmap = DAMakePixmap(); /* size == dockapp geometry (48,48) */

	colors = setGCs(pixmap);

	XFillRectangle(DADisplay, pixmap, DAClearGC, 0, 0, 48, 48);
	XClearWindow(DADisplay, DAWindow);

	/* Make a "Region" from the shapes we have */
	XUnionRectWithRegion(&btn, clipRegion, clipRegion);
	XUnionRectWithRegion(&square, clipRegion, clipRegion);
	XUnionRectWithRegion(&slider, clipRegion, clipRegion);

	/* Make this region a window shape mask */
	XShapeCombineRegion(DADisplay, DAWindow, ShapeBounding,
			    0, 0, clipRegion, ShapeSet);

	/* We don't need the region anymore (it is copied by XShapeCombineRegion).
	 * XXX: That's not certain, it is not documented. XSetRegion does so,
	 * though, so it is a likely assumption that it does copy.
	 */
	XDestroyRegion(clipRegion);

	/* The cursor we want to use.
	 * Specify 'None' for the default,
	 * or one from X11/cursorfont.h
	 */
	pointer = XCreateFontCursor(DADisplay, XC_based_arrow_up);
	XDefineCursor(DADisplay, DAWindow, pointer);

	/* a square with an image that changes when clicked (A button). */
	createBtn(btn);

	/* a square that shows the number of the mouse-button pressed on click. */
	createSquare(square);

	/* a slider a using two dashed line GC's. */
	createSlider(slider);

	/* Tell libdockapp this is the pixmap that we want to show */
	DASetPixmap(pixmap);

	/* Process events every 100ms */
	DASetTimeout(100);

	/* set event callbacks */
	DASetCallbacks(&eventCallbacks);

	/* Display the pixmap we said it to show */
	DAShow();

	/* Process events and keep the dockapp running */
	DAEventLoop();

	return 0;
}


/* Create our GC's to draw colored lines and such */
struct Colors *
setGCs(Drawable d)
{
	struct Colors *colors;
	XGCValues gcv;
	unsigned long origColor;
	char dashList[2] = {3, 1};

	colors = malloc(sizeof(struct Colors));
	if (colors == NULL)
		return NULL;

	/* Get the GC-values of the default GC */
	XGetGCValues(DADisplay, DAGC,
		     GCForeground | GCBackground | GCGraphicsExposures, &gcv);

	origColor = gcv.foreground;

	/* don't send expose events */
	gcv.graphics_exposures = False;

	/* GC for white color */
	gcv.foreground = WhitePixel(DADisplay, DefaultScreen(DADisplay));
	colors->white = XCreateGC(DADisplay, d,
				  GCForeground | GCGraphicsExposures, &gcv);

	/* GC for dark blue color */
#if 1
	gcv.foreground = DAGetColor("navy");
#else
	gcv.foreground = 0;
#endif
	colors->black = XCreateGC(DADisplay, d,
				  GCForeground | GCGraphicsExposures, &gcv);

	/* GC for light borders */
	gcv.foreground = DAGetColor("lightGray");
	colors->lightGray = XCreateGC(DADisplay, d,
				      GCForeground | GCGraphicsExposures, &gcv);

	/* GC for dark borders (note re-use of gcv-values) */
	gcv.foreground = DAGetColor("#222222");
	colors->darkGray =  XCreateGC(DADisplay, d,
				      GCForeground | GCGraphicsExposures, &gcv);

	/* GC for the un-/highlighted colors and dashed line of the slider */
	gcv.foreground = origColor;
	gcv.line_width = 9;
	gcv.line_style = LineOnOffDash;

	colors->slider = XCreateGC(DADisplay, d,
				   GCForeground | GCBackground | GCGraphicsExposures |
				   GCLineWidth | GCLineStyle, &gcv);

	XSetDashes(DADisplay, colors->slider, 1, dashList, 2);

	/* light slider GC */
	gcv.foreground = adjustColor(origColor, +0x40);

	colors->sliderLight = XCreateGC(DADisplay, d,
					GCForeground | GCBackground | GCGraphicsExposures |
					GCLineWidth | GCLineStyle, &gcv);

	XSetDashes(DADisplay, colors->sliderLight, 1, dashList, 2);

	/* dark slider GC */
	gcv.foreground = adjustColor(origColor, -0x40);

	colors->sliderDark = XCreateGC(DADisplay, d,
				       GCForeground | GCBackground | GCGraphicsExposures |
				       GCLineWidth | GCLineStyle, &gcv);

	XSetDashes(DADisplay, colors->sliderDark, 1, dashList, 2);

	return colors;
}


/* Make a (GC) color lighter or darker */
unsigned long
adjustColor(unsigned long color, signed int adjustment)
{
	signed long r, g, b;

	r = color >> 16;
	g = (color - (r << 16)) >> 8;
	b = (color - (g << 8) - (r << 16));

	r += adjustment;
	g += adjustment;
	b += adjustment;

	if (r > 0xff)
		r = 0xff;

	if (g > 0xff)
		g = 0xff;

	if (b > 0xff)
		b = 0xff;

	if (r < 0)
		r = 0;

	if (g < 0)
		g = 0;

	if (b < 0)
		b = 0;

	return ((unsigned short)r << 16) +
	       ((unsigned short)g << 8) +
	       (unsigned short)b;
}


void
setPointerColor(GC color)
{
	XGCValues gcv;
	XColor fcolor, bcolor;

	XGetGCValues(DADisplay, color, GCForeground, &gcv);

	fcolor.pixel = gcv.foreground;
	fcolor.flags = DoRed | DoGreen | DoBlue;

	bcolor.red = 0;
	bcolor.green = 0;
	bcolor.blue = 0;

	XRecolorCursor(DADisplay, pointer, &fcolor, &bcolor);
}


/* event handlers functions */
void destroy(void)
{
}

void buttonPress(int button, int state, int x, int y)
{
	int *data = malloc(sizeof(int *));

	*data = button;

	DAProcessActionRects(x, y, actionRects[0], 3, (void *)data);

	free(data);
}

void buttonRelease(int button, int state, int x, int y)
{
	DAProcessActionRects(x, y, actionRects[1], 2, NULL);

}

void
mouseMove(int x, int y)
{
	DAProcessActionRects(x, y, actionRects[2], 1, NULL);
}

void
mouseEnter(void)
{
	mouseIn = 1;

	drawSlider(actionRects[3][0].rect);
}


void
mouseLeave(void)
{
	mouseIn = 0;

	/* mouse pointer left the dockapp window */
	DAProcessActionRects(0, 0, actionRects[4], 2, NULL);

	/* if the button is still depressed, make it go up again. */
/* TODO: Use data in actionRects[4] here instead of below check */
	if (buttonDown != NULL)
		btnUp(0, 0, *buttonDown, NULL);

	drawSlider(actionRects[4][1].rect);
}

/* what to do for a specific event for every 'item' in the dockapp */
/*	Button that can be pressed "down" and jumps back "up" again */
void
btnDown(int x, int y, DARect rect, void *data)
{
	buttonDown = &rect;
	drawSunkenFrame(rect);
}


void
btnUp(int x, int y, DARect rect, void *data)
{
	buttonDown = NULL;
	drawRaisedFrame(rect);
}


void
btnLeave(int x, int y, DARect rect, void *data)
{
	if (buttonDown == NULL)
		return;

	drawRaisedFrame(rect);
}


/*	Square that tells which button was pressed (number) */
void
squareDown(int x, int y, DARect rect, void *data)
{
	int button;

	if (data) {
		int *tmp = (int *)data;

		button = *tmp;
	} else
		button = 0;

	drawSquare(rect, button);
}


/*	A draggable slider that highlights when the mouse is over it */
void
sliderDown(int x, int y, DARect rect, void *data)
{
	buttonDown = &rect;
	setPointerColor(colors->sliderDark);
}


void
sliderUp(int x, int y, DARect rect, void *data)
{
	buttonDown = NULL;
	setPointerColor(colors->black);
}


void
sliderMove(int x, int y, DARect rect, void *data)
{
	if (buttonDown == NULL /* ||
				  rect.x != buttonDown->x ||
				  rect.y != buttonDown->y ||
				  rect.width != buttonDown->width ||
				  rect.height != buttonDown->height */)
		return;

	sliderPos = (float)(rect.height - y) / (float)rect.height;
	if (sliderPos > 1.0)
		sliderPos = 1.0;

	if (sliderPos < 0.0)
		sliderPos = 0.0;

	drawSlider(rect);
}

void sliderEnter(int x, int y, DARect rect, void *data)
{
}
void sliderLeave(int x, int y, DARect rect, void *data)
{
}



/*
 * Drawing functions
 */
void
createBtn(DARect rect)
{
	/* fill square excluding borders */
	XFillRectangle(DADisplay, pixmap, colors->lightGray,
		       rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 2);

	drawRaisedFrame(rect);
}


void
drawRaisedFrame(DARect rect)
{
	/* left border */
	XDrawLine(DADisplay, pixmap, colors->white,
		  rect.x, rect.y, rect.x, rect.y + rect.height - 2);
	/* top border */
	XDrawLine(DADisplay, pixmap, colors->white,
		  rect.x + 1, rect.y, rect.width - 1, rect.y);
	/* bottom border */
	XDrawLine(DADisplay, pixmap, colors->darkGray,
		  rect.x,                 rect.y + rect.height - 1,
		  rect.x + rect.width - 1, rect.y + rect.height - 1);
	/* right border */
	XDrawLine(DADisplay, pixmap, colors->darkGray,
		  rect.x + rect.width - 1, rect.y + 1,
		  rect.x + rect.width - 1, rect.y + rect.height - 2);

	DASetPixmap(pixmap);
}


void
drawSunkenFrame(DARect rect)
{
	/* left border */
	XDrawLine(DADisplay, pixmap, colors->darkGray,
		  rect.x, rect.y, rect.x, rect.y + rect.height - 2);
	/* top border */
	XDrawLine(DADisplay, pixmap, colors->darkGray,
		  rect.x + 1, rect.y, rect.width - 1, rect.y);
	/* bottom border */
	XDrawLine(DADisplay, pixmap, colors->white,
		  rect.x,                 rect.y + rect.height - 1,
		  rect.x + rect.width - 1, rect.y + rect.height - 1);
	/* right border */
	XDrawLine(DADisplay, pixmap, colors->white,
		  rect.x + rect.width - 1, rect.y + 1,
		  rect.x + rect.width - 1, rect.y + rect.height - 2);

	DASetPixmap(pixmap);
}


void
createSquare(DARect rect)
{
	/* fill square excluding borders */
	XFillRectangle(DADisplay, pixmap, colors->black,
		       rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 2);

	XDrawRectangle(DADisplay, pixmap, colors->lightGray,
		       rect.x, rect.y, rect.width - 1, rect.height - 1);

	drawSquare(rect, 0);
}


void
drawSquare(DARect rect, int button)
{
	char label[3];

	XFillRectangle(DADisplay, pixmap, colors->black,
		       rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 2);

	snprintf(label, 3, "%2d", button);
	XDrawString(DADisplay, pixmap, colors->white,
		    rect.x + 3, rect.y + rect.height - 5, label, 2);

	DASetPixmap(pixmap);
}


void
createSlider(DARect rect)
{
	/* fill square excluding borders */
	XFillRectangle(DADisplay, pixmap, colors->black,
		       rect.x + 1, rect.y + 1, rect.width - 2, rect.height - 2);

	drawSunkenFrame(rect);

	drawSlider(rect);
}


void
drawSlider(DARect rect)
{
	GC highColor, lowColor; /* determine colors to use */

	if (mouseIn) {
		highColor = colors->sliderLight;
		lowColor = colors->slider;
	} else {
		highColor = colors->slider;
		lowColor = colors->sliderDark;
	}

	/* Draw two lines from bottom to sliderPos fraction of height */
	if (sliderPos < 1.0) {
		XDrawLine(DADisplay, pixmap, highColor,
			  rect.x + 6, rect.y + rect.height - 2,
			  rect.x + 6, rect.y + (1.0 - sliderPos) * (rect.height - 2));

		XDrawLine(DADisplay, pixmap, highColor,
			  rect.x + 17, rect.y + rect.height - 2,
			  rect.x + 17, rect.y + (1.0 - sliderPos) * (rect.height - 2));
	}

	if (sliderPos > 0.0) {
		XDrawLine(DADisplay, pixmap, lowColor,
			  rect.x + 6, rect.y + 1,
			  rect.x + 6, rect.y + (1.0 - sliderPos) * (rect.height - 2));

		XDrawLine(DADisplay, pixmap, lowColor,
			  rect.x + 17, rect.y + 1,
			  rect.x + 17, rect.y + (1.0 - sliderPos) * (rect.height - 2));
	}

	DASetPixmap(pixmap);
}


DAActionRect
setRectAction(DARect rect, DARectCallback action)
{
	DAActionRect ar;

	ar.rect     = rect;
	ar.action   = action;

	return ar;
}

