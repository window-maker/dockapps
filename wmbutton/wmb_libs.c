/* wmb_libs.c - Edward H. Flora - ehf_dockapps@cox.net */
/* Last Modified: 4/3/04 */
/*
 * These functions are designed to work with wmbutton.
 */

/* PORTABILITY:
******************
* Coded in ANSI C, using ANSI prototypes.
*/

/****** Include Files *************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wmbutton.h"

/****** ToolTip Globals ***********************************************/

static struct timeval _tStart;

extern struct Config_t Config;
int _bTooltip = 0;
XFontStruct* _fTooltip;
int _nFontHeight, _nFontY;
int _nScreenWidth, _nScreenHeight;
GC _gcMono = 0;
Window _wTooltip;

/****** Parse Command Line ********************************************/
void parseargs(int argc, char **argv)
{
	int current;
	char *Home = getenv("HOME");

	while (-1 != (current = getopt(argc, argv, "vhnmsF:b:g:d:f:"))) {
		switch (current) {
		case 'v':
			Config.Verbose = 1;
			break;
		case '?':
		case 'h':
			show_usage();
			break;
		case 'm':
			Config.mmouse = 1;
			break;
		case 'n':
			Config.bTooltipDisable = 1;
			break;
		case 's':
			Config.bTooltipSwapColors = 1;
			break;
		case 'g':
			Config.Geometry_str = strdup(optarg);
			break;
		case 'd':
			Config.Display_str = strdup(optarg);
			break;
		case 'f':
			Config.configfile = strdup(optarg);
			break;
		case 'F':
			Config.szTooltipFont = strdup(optarg);
			break;
		case 'b':
			Config.buttonfile = strdup(optarg);
			break;
		}
	}

	if (!Config.configfile) {
		if (Home != NULL) {
			Config.configfile = malloc(strlen(Home) +
						   strlen(CONFFILENAME) + 1);
			sprintf(Config.configfile, "%s%s", Home, CONFFILENAME);
		}
	}

	if (!Config.buttonfile) {
		if (Home != NULL) {
			Config.buttonfile = malloc(strlen(Home) +
						   strlen(BUTTONFILENAME) + 1);
			sprintf(Config.buttonfile, "%s%s", Home, BUTTONFILENAME);
		}
	}

	if (!Config.Geometry_str)
		Config.Geometry_str = "64x64+0+0";

	if (!Config.Display_str)
		Config.Display_str = "";

	if (!Config.szTooltipFont)
		Config.szTooltipFont = TOOLTIP_FONT;

	if (!Config.bTooltipDisable)
		Config.bTooltipDisable = !TOOLTIP_SUPPORT;

}

/****** Show Usage Information ****************************************/
void show_usage(void)
{
	extern char *app_name;

	fprintf(stderr, "\n");
	fprintf(stderr, "usage: %s [-g geom] [-d dpy] [-f cfgfile] [-b btnfile] "\
		"[-F <font>] [-v] [-s] [-n]\n",app_name);
	fprintf(stderr, "\n");
	fprintf(stderr, " wmbutton version %s\n", PACKAGE_VERSION);
	fprintf(stderr, "\n");
	fprintf(stderr, "-g <geometry>  Window Geometry - ie: 64x64+10+10\n");
	fprintf(stderr, "-d <display>   Display - ie: 127.0.0.1:0.0\n");
	fprintf(stderr, "-f <filename>  Full path to configuration file.\n");
	fprintf(stderr, "-b <filename>  Full path to button xpm.\n");
	fprintf(stderr, "-F <font>      Custom tooltip font (e.g. -b\\&h-lucidatypewriter-medium-*-*-*-12-*)\n");
	fprintf(stderr, "-v             Verbose Mode.\n");
	fprintf(stderr, "-h             Help. This message.\n");
	fprintf(stderr, "-m             Disable Middle Mouse functionality.\n");
	fprintf(stderr, "-s             Swap tooltip colors.\n");
	fprintf(stderr, "-n             Turn off tooltips.\n");
	fprintf(stderr, "\n");
	exit(0);
}
/***********************************************************************/

/****** Error Handler Routine *****************************************/
void err_mess(int err, char *str)
{
	switch (err) {
	case FAILDISP:
		fprintf(stderr, "Fail: XOpenDisplay for %s\n", str);
		exit(err);
	case FAILSWIN:
		fprintf(stderr, "Fail: XCreateSimpleWindow\n");
		exit(err);
	case FAILICON:
		fprintf(stderr, "Fail: XCreateSimpleWindow\n");
		exit(err);
	case FAILXPM:
		fprintf(stderr, "Fail: XCreateBitmapFromData\n");
		break;
	case FAILWNAM:
		fprintf(stderr, "%s: Can't set up window name\n", str);
		exit(err);
	case FAILGC:
		fprintf(stderr, "Fail: XCreateGC\n");
		exit(err);
	case FAILCONF:
		fprintf(stderr, "Fail: Can't Find user or system configuration file.\n");
		fprintf(stderr, "Fail: User Config: '%s'\n", str);
		fprintf(stderr, "Fail: System Config: '%s'\n", CONFIGGLOBAL);
		exit(err);
	case FAILTMPL:
		fprintf(stderr, "Fail: Can't Create 'template' Pixmap\n");
		exit(err);
	case FAILVIS:
		fprintf(stderr, "Fail: Can't Create 'visible' Pixmap\n");
		exit(err);
	case FAILBUT:
		fprintf(stderr, "Fail: Can't Create 'buttons' Pixmap\n");
		exit(err);
	default:
		fprintf(stderr, "Fail: UnSpecified Error: %d\n", err);
		fprintf(stderr, "Fail: %s\n", str);
		exit(err);
	}
}
/***********************************************************************/

/***********************************************************************
 * RunAppN(int app)
 *
 * Run the command given in the configuration file 'configfile'
 ***********************************************************************/
void RunAppN(int app)
{
	char *cmndstr;
	extern struct Config_t Config;

	cmndstr = Parse(app); /* Get command to pass to system */

	if (Config.Verbose)
		fprintf(stderr, "Command String: %s", cmndstr);

	if (cmndstr != NULL) {
		system(cmndstr); /* if there's a command, run it */
		free(cmndstr);
	}
}
/***********************************************************************/

/***********************************************************************
 * canOpenFile(const char *path)
 *
 * Check if the file at a given path can be opened.
 ***********************************************************************/
int canOpenFile(const char *path)
{
	FILE *fp;

	if ((fp = fopen(path, "r")) == NULL)
		return 0;
	else {
		fclose(fp);
		return 1;
	}
}

/***********************************************************************
 * Parse(int app)
 *
 * Parses the file 'configfile' for command to execute.
 ***********************************************************************/
char *Parse(int app)
{
	FILE *fp;
	char Buf[BUFFER_SIZE];
	char *Ptr;

	if ((fp = fopen(Config.configfile, "r")) == NULL)
		if ((fp = fopen(CONFIGGLOBAL, "r")) == NULL)
			err_mess(FAILCONF,Config.configfile);

	while ((Ptr = fgets(Buf, BUFFER_SIZE, fp))) {
		if (atoi(Buf) == app)
			break;
	}

	fclose(fp);

	if (!Ptr)
		return Ptr;

	Ptr = strchr(Buf, '\t');        /* find first tab */
	if (Ptr == NULL)
		Ptr = strchr(Buf, ' '); /* or space charater */

	if (Ptr == NULL)
		return(NULL);

	Ptr++;
	Ptr = strdup(Ptr);

	return Ptr;
}
/**********************************************************************/

/***********************************************************************
 * initTime
 *
 * Copyright (c) 2001 Bruno Essmann <essmann@users.sourceforge.net>
 ***********************************************************************/

void initTime(void)
{
	extern struct Config_t Config;

	if (Config.Verbose)
		fprintf(stdout, "[        ] initializing time\n");

	gettimeofday(&_tStart, NULL);
}
/**********************************************************************/

long currentTimeMillis(void)
{
	struct timeval tNow, tElapsed;

	gettimeofday(&tNow, NULL);

	if (_tStart.tv_usec > tNow.tv_usec) {
		tNow.tv_usec += 1000000;
		tNow.tv_sec--;
	}

	tElapsed.tv_sec = tNow.tv_sec - _tStart.tv_sec;
	tElapsed.tv_usec = tNow.tv_usec - _tStart.tv_usec;
	return (tElapsed.tv_sec * 1000) + (tElapsed.tv_usec / 1000);
}
/**********************************************************************/

void getWindowOrigin(Window w, int *nX, int *nY)
{
	extern Display *display;
	Window wWindow, wParent, wRoot;
	Window* wChildren;
	unsigned int nChildren, ww, wh, wb, wd;
	int wx, wy;

	wParent = w;
	do {
		wWindow = wParent;
		if (!XQueryTree(display, wParent, &wRoot, &wParent, &wChildren, &nChildren))
			return;

		if (wChildren)
			XFree(wChildren);

	} while (wParent != wRoot);

	if (XGetGeometry(display, wWindow, &wRoot, &wx, &wy, &ww, &wh, &wb, &wd)) {
		if (nX)
			*nX = wx;

		if (nY)
			*nY = wy;
	}
}
/**********************************************************************/

/***********************************************************************
 * getButtonLocation
 *
 * compute location for each button's tooltip (not perfect)
 ***********************************************************************/
void getButtonLocation (int nButton, int *nLocationX, int *nLocationY)
{
	*nLocationX = 0;
	*nLocationY = 8;

	while (nButton > BUTTON_COLS) {
		*nLocationY += BUTTON_SIZE;
		nButton -= BUTTON_COLS;
	}

	while (nButton > 0) {
		*nLocationX += BUTTON_SIZE - 1;
		nButton--;
	}
}
/**********************************************************************/

/* SkipWord & SkipSpaces: utility functions for getNicenedString */
char *SkipWord(char *Text) {
	char *Result = Text;

	while ((*Result != ' ') && (*Result != '\t') &&
	       (*Result != '\n') && (*Result != 0x00))
		Result++;

	return Result;
}

char *SkipSpaces(char *Text) {
	char *Result = Text;

	while ((*Result == ' ') || (*Result == '\t') || (*Result == '\n'))
		Result++;

	return Result;
}

/***********************************************************************
 * getNicenedString
 *
 * nicen the parsed command from the .wmbutton config file
 *  - cut if too long
 *  - remove parameters, whitespace and the '&'...
 ***********************************************************************/
char *getNicenedString(char *old, int andAddSeparator)
{
	char *WorkStr, *WorkStrEnd, *StartPtr, *EndPtr, *RetStr;

	if (!old) {
		if (andAddSeparator)
			return strdup("-- | ");
		else
			return strdup("--");
	}

	RetStr = malloc(strlen(old) + 3 + 1); /* 3 for Seperator */
	*RetStr = 0x00;

	WorkStr = strdup(old);
	WorkStrEnd = strchr(WorkStr, 0x00);
	StartPtr = WorkStr;

	while (StartPtr < WorkStrEnd) {
		StartPtr = SkipSpaces(StartPtr);
		EndPtr = SkipWord(StartPtr);
		*EndPtr = 0x00;

		if ((*StartPtr == '&') || (*StartPtr == '-'))
			break;

		strcat(RetStr, StartPtr);
		strcat(RetStr, " ");
		StartPtr = EndPtr + 1;
	}

	free(WorkStr);

	if (andAddSeparator)
		strcat(RetStr, "| ");

	return RetStr;
}

/***********************************************************************
 * getButtonAppNames
 *
 *returns the 1..3 application names / commands to be shown in tooltip
 ***********************************************************************/
char *getButtonAppNames(int nButton)
{
	char *tmp1, *tmp2, *str = NULL;

	if (!( nButton < 0 || nButton > 9 )) {

		/* FIXME: _Might_ overflow, but it's unlikely.
		 * Perhaps one should fix this sometime ;) */
		str = (char*) calloc (sizeof(char), BUFFER_SIZE);

		tmp1 = Parse(nButton + LMASK);
		tmp2 = getNicenedString(tmp1, 1);
		strcat(str, tmp2);
		free(tmp1);
		free(tmp2);

		tmp1 = Parse(nButton + MMASK);
		tmp2 = getNicenedString(tmp1, 1);
		strcat(str, tmp2);
		free(tmp1);
		free(tmp2);

		tmp1 = Parse(nButton + RMASK);
		tmp2 = getNicenedString(tmp1, 0);
		strcat(str, tmp2);
		free(tmp1);
		free(tmp2);
	}

	return str;
}
/**********************************************************************/


int hasTooltipSupport(void)
{
	return !Config.bTooltipDisable;
}
/**********************************************************************/

void showTooltip (int nButton, int nMouseX, int nMouseY)
{
	Pixmap pixmap, mask;
	int nMainWinX, nMainWinY;
	int nButtonX = 0, nButtonY = 0, nButtonWidth = 0, nButtonHeight = 0;
	int nTextY, nX, nY, nWidth, nHeight, nSide;
	char* szText;
	extern struct Config_t Config;
	extern Window iconwin;
	extern Pixel bg_pixel, fg_pixel;
	extern Display *display;
	extern GC gc;

	if (Config.bTooltipDisable || nButton == -1)
		return;

	if (_bTooltip)
		hideTooltip();

	if (Config.Verbose)
		fprintf(stdout,
			"[%8ld] showing tooltip for button %d at %d, %d\n",
			currentTimeMillis(), nButton, nMouseX, nMouseY);

	szText = getButtonAppNames(nButton);
	if(!szText)
		return;

	_bTooltip = 1;

	nWidth = XTextWidth(_fTooltip, szText, strlen(szText)) + 16;
	nHeight = _nFontHeight + 4;
	if (nHeight < 16)
		nHeight = 16;

	if (nWidth < nHeight)
		nWidth = nHeight;

	if (Config.Verbose)
		fprintf(stdout, "[%8ld] tooltip size: %d, %d\n",
			currentTimeMillis(), nWidth, nHeight);

	getWindowOrigin(iconwin, &nMainWinX, &nMainWinY);
	getButtonLocation(nButton, &nButtonX, &nButtonY);
	nButtonX += nMainWinX;
	nButtonY += nMainWinY;
	nButtonWidth = BUTTON_SIZE;
	nButtonHeight = BUTTON_SIZE;

	if (nButtonX + nWidth > _nScreenWidth) {
		nSide = TOOLTIP_RIGHT;
		nX = nButtonX - nWidth + nButtonWidth / 2;
		if (nX < 0)
			nX = 0;
	} else {
		nSide = TOOLTIP_LEFT;
		nX = nButtonX + nButtonWidth / 2;
	}

	if (nX + nWidth > _nScreenWidth)
		nX = _nScreenWidth - nWidth;

	if (nButtonY - (nHeight + TOOLTIP_SPACE) < 0) {
		nSide |= TOOLTIP_TOP;
		nY = nButtonY + nButtonHeight - 1;
		nTextY = TOOLTIP_SPACE;
	} else {
		nSide |= TOOLTIP_BOTTOM;
		nY = nButtonY - (nHeight + TOOLTIP_SPACE);
		nTextY = 0;
	}

	pixmap = createTooltipPixmap(nWidth, nHeight, nSide, &mask);

	XSetForeground(display, gc, Config.bTooltipSwapColors ? fg_pixel : bg_pixel);
	XSetFont(display, gc, _fTooltip->fid);
	XDrawString(display, pixmap, gc,
		    8, nTextY + (nHeight - _nFontHeight) / 2 + _nFontY,
		    szText, strlen(szText));

	XSetWindowBackgroundPixmap(display, _wTooltip, pixmap);

	XResizeWindow(display, _wTooltip, nWidth, nHeight + TOOLTIP_SPACE);
	XShapeCombineMask(display, _wTooltip, ShapeBounding, 0, 0, mask, ShapeSet);
	XFreePixmap(display, mask);
	XMoveWindow(display, _wTooltip, nX, nY);
	XMapRaised(display, _wTooltip);
	XFreePixmap(display, pixmap);

	free(szText);
}
/**********************************************************************/

void hideTooltip(void)
{
	extern struct Config_t Config;
	extern Display *display;

	if (Config.bTooltipDisable)
		return;

	if (_bTooltip) {
		if (Config.Verbose)
			fprintf(stdout, "[%8ld] hiding tooltip\n", currentTimeMillis());

		XUnmapWindow(display, _wTooltip);
		_bTooltip = 0;
	}
}
/**********************************************************************/

int hasTooltip(void)
{
	if (Config.bTooltipDisable)
		return 0;

	return _bTooltip;
}
/**********************************************************************/

void initTooltip(void)
{
	XSetWindowAttributes attribs;
	unsigned long vmask;
	extern Display *display;
	extern char *app_name;
	extern int screen;
	extern Window rootwin, win;

	if (Config.bTooltipDisable) {
		if (Config.Verbose)
			fprintf(stdout, "[%8ld] initializing tooltips (disabled)\n",
				currentTimeMillis());

		return;
	}
	if (Config.Verbose)
		fprintf(stdout, "[%8ld] initializing tooltips\n", currentTimeMillis());

	_fTooltip = XLoadQueryFont(display, Config.szTooltipFont);
	if (!_fTooltip) {
		fprintf(stderr, "%s: couldn't allocate font '%s'.\n", app_name, Config.szTooltipFont);
		if (!strcmp(Config.szTooltipFont, TOOLTIP_FONT))
			fprintf(stderr, "%s: Use option -F <font>\n", app_name);
		exit(-1);
	}

	_nFontHeight = _fTooltip->ascent + _fTooltip->descent;
	_nFontY = _fTooltip->ascent;
	_nScreenWidth = WidthOfScreen(ScreenOfDisplay(display, screen));
	_nScreenHeight = HeightOfScreen(ScreenOfDisplay(display, screen));
	if (Config.Verbose)
		fprintf(stdout, "[%8ld] configuring tooltip font:\n" \
			"[%8ld] - '%s'\n" \
			"[%8ld] - font-height= %d, font-ascent= %d\n" \
			"[%8ld] configuring screen size: %dx%d\n",
			currentTimeMillis(),
			currentTimeMillis(), Config.szTooltipFont,
			currentTimeMillis(), _nFontHeight, _nFontY,
			currentTimeMillis(), _nScreenWidth, _nScreenHeight);

	vmask = CWSaveUnder | CWOverrideRedirect | CWBorderPixel;
	attribs.save_under = True;
	attribs.override_redirect = True;
	attribs.border_pixel = 0;
	_wTooltip = XCreateWindow(display, rootwin, 1, 1, 10, 10, 1,
				  CopyFromParent, CopyFromParent,
				  CopyFromParent, vmask, &attribs);

	if (win == 0) {
		fprintf(stderr, "Cannot create tooltip window.\n");
		exit(-1);
	}
}
/**********************************************************************/

void destroyTooltip(void)
{
	extern Display *display;

	if (Config.bTooltipDisable)
		return;

	if (_gcMono) {
		XFreeGC(display, _gcMono);
		_gcMono = 0;
	}

	XDestroyWindow(display, _wTooltip);
}
/**********************************************************************/

void drawTooltipBalloon(Pixmap pix, GC gc, int x, int y, int w, int h, int side)
{
	extern Display *display;
	int rad = h * 3 / 10;
	XPoint pt[3];

	XFillArc(display, pix, gc, x, y, rad, rad, 90 * 64, 90 * 64);
	XFillArc(display, pix, gc, x, y + h - 1 - rad, rad, rad, 180 * 64, 90 * 64);

	XFillArc(display, pix, gc, x + w - 1 - rad, y, rad, rad, 0 * 64, 90 * 64);
	XFillArc(display, pix, gc, x + w - 1 - rad, y + h - 1 - rad, rad, rad, 270 * 64, 90 * 64);

	XFillRectangle(display, pix, gc, x, y + rad / 2, w, h - rad);
	XFillRectangle(display, pix, gc, x + rad / 2, y, w - rad, h);

	if (side & TOOLTIP_BOTTOM) {
		pt[0].y = y + h - 1;
		pt[1].y = y + h - 1 + TOOLTIP_SPACE;
		pt[2].y = y + h - 1;
	} else {
		pt[0].y = y;
		pt[1].y = y-TOOLTIP_SPACE;
		pt[2].y = y;
	}

	if (side & TOOLTIP_RIGHT) {
		pt[0].x = x + w - h +  2 * h / 16;
		pt[1].x = x + w - h + 11 * h / 16;
		pt[2].x = x + w - h +  7 * h / 16;
	} else {
		pt[0].x = x + h -  2 * h /16;
		pt[1].x = x + h - 11 * h /16;
		pt[2].x = x + h -  7 * h /16;
	}

	XFillPolygon(display, pix, gc, pt, 3, Convex, CoordModeOrigin);
}
/**********************************************************************/

Pixmap createTooltipPixmap(int width, int height, int side, Pixmap *mask)
{
	extern Display *display;
	extern GC gc;
	extern Pixel bg_pixel, fg_pixel;
	extern int depth;
	extern Window rootwin;
	Pixmap bitmap, pixmap;
	int x, y;

	bitmap = XCreatePixmap(display, rootwin,
			       width+TOOLTIP_SPACE, height+TOOLTIP_SPACE, 1);

	if (!_gcMono)
		_gcMono = XCreateGC(display, bitmap, 0, NULL);

	XSetForeground(display, _gcMono, 0);
	XFillRectangle(display, bitmap, _gcMono, 0, 0,
		       width+TOOLTIP_SPACE, height+TOOLTIP_SPACE);

	pixmap = XCreatePixmap(display, rootwin, width+TOOLTIP_SPACE,
			       height+TOOLTIP_SPACE, depth);
	XSetForeground(display, gc, Config.bTooltipSwapColors ? fg_pixel : bg_pixel);
	XFillRectangle(display, pixmap, gc, 0, 0, width+TOOLTIP_SPACE,
		       height+TOOLTIP_SPACE);

	if (side & TOOLTIP_BOTTOM)
		y = 0;
	else
		y = TOOLTIP_SPACE;

	x = 0;

	XSetForeground(display, _gcMono, 1);
	drawTooltipBalloon(bitmap, _gcMono, x, y, width, height, side);
	XSetForeground(display, gc, Config.bTooltipSwapColors ? bg_pixel : fg_pixel);
	drawTooltipBalloon(pixmap, gc, x+1, y+1, width-2, height-2, side);

	*mask = bitmap;

	return pixmap;
}
/***********************************************************************/


/***********************************************************************
 * flush_expose
 *
 * Everyone else has one of these... Can't hurt to throw it in.
 ***********************************************************************/
int flush_expose(Window w)
{
	extern Display *display;
	XEvent dummy;
	int i = 0;

	while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
		i++;

	return i;
}
/***********************************************************************/
