/* $Id: wmpager.c,v 1.4 2002/08/16 17:22:26 essmann Exp $
 *
 * Copyright (c) 2001 Bruno Essmann <essmann@users.sourceforge.net>
 * All rights reserved.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buttons.xpm"
#include "screen.xpm"

/*
 * Constants
 */

#define AUTHOR "Bruno Essmann <essmann@users.sourceforge.net>"
#define APPLICATION "wmpager"
#define VERSION "1.2"
#define BUILD_REV "$Revision: 1.4 $"
#define BUILD_DATE "$Date: 2002/08/16 17:22:26 $"

#define XA_WIN_WORKSPACE "_WIN_WORKSPACE"
#define XA_WIN_WORKSPACE_NAMES "_WIN_WORKSPACE_NAMES"

#define WMPAGER_ENV "WMPAGER"
#define WMPAGER_DEFAULT_INSTALL_DIR "/usr/local/share/wmpager/"
#define WMPAGER_USER_DIR ".wmpager/"

#define TOOLTIP_SUPPORT 1
#define TOOLTIP_FONT "-*-helvetica-bold-r-normal-*-12-*-*-*-*-*-*-*"
#define TOOLTIP_OUTSIDE 0
#define TOOLTIP_SHOW_DELAY 750
#define TOOLTIP_RESHOW_DELAY 1500

#define TOOLTIP_SPACE 12
#define TOOLTIP_TOP	0
#define TOOLTIP_BOTTOM	1
#define TOOLTIP_LEFT	0
#define TOOLTIP_RIGHT	2

/*
 * Prototypes
 */

void usage (int bVerbose);
void info ();
void setVerbose (int bVerbose);
int isVerbose ();

void initApplicationName (char* szApplicationName);
char* getApplicationName ();

void initDisplay (char* szDisplay);
void destroyDisplay ();
Display* getDisplay ();

Pixel getWhitePixel ();
Pixel getBlackPixel ();
int getDefaultScreen ();
int getDefaultDepth ();
void initWindow (int nArgc, char** szArgv);
void destroyWindow ();
GC getWindowGraphics ();
GC getMainGraphics ();
void initWindowMask (char* szInstallDir, char* szButtonTheme);
void redrawWindow ();
void getWindowOrigin (Window w, int* nX, int* nY);

void loop (long nTooltipShowDelay, long nTooltipReshowDelay);

void initButtons (int nButtons, int nColumns, int nRows);
int getButtonCount ();
int getButtonRowCount ();
int getButtonColumnCount ();
int getButtonWidth ();
int getButtonHeight ();
int getButtonAt (int nLocationX, int nLocationY);
void getButtonLocation (int nButton, int* nLocationX, int* nLocationY);

void initTime ();
long currentTimeMillis ();

void initScreens ();
int getScreenCount ();
char* getScreenName (int nScreen);
int getCurrentScreen ();
void setCurrentScreen (int nCurrentScreen);
void gotoScreen (int nWorkspace);

void initTooltip (int bTooltipSupport, char* szFontName, int bTooltipOutside);
void destroyTooltip ();
int hasTooltipSupport ();
void showTooltip (int nButton, int nMouseX, int nMouseY);
void hideTooltip ();
int hasTooltip ();
void drawTooltipBalloon (Pixmap pix, GC gc, int x, int y, int w, int h, int side);
Pixmap createTooltipPixmap (int width, int height, int side, Pixmap *mask);

/*
 * Main
 */

int main (int nArgc, char** szArgv) {
	char* szDisplay= NULL;
	char* szTheme= NULL;
	char* szInstallDir= NULL;
	char* szTooltipFont= TOOLTIP_FONT;
	long nTooltipShowDelay= TOOLTIP_SHOW_DELAY;
	long nTooltipReshowDelay= TOOLTIP_RESHOW_DELAY;
	int nWorkspaces= -1;
	int bVerbose= 0;
	int nSizeX= -1, nSizeY= -1;
	int bTooltipSupport= TOOLTIP_SUPPORT;
	int bTooltipOutside= TOOLTIP_OUTSIDE;
	int i;
	initApplicationName(szArgv[0]);
	/* we no longer use the WMPAGER environment variable 
	 * szInstallDir= (char*) getenv(WMPAGER_ENV);
	 * instead we simply use a default installation directory
	 */
	szInstallDir= WMPAGER_DEFAULT_INSTALL_DIR;
	i= 1;
	while (i < nArgc) {
		if (strcmp("-h", szArgv[i]) == 0 || strcmp("--help", szArgv[i]) == 0) {
			usage(1);
		} else if (strcmp("-v", szArgv[i]) == 0 || strcmp("--verbose", szArgv[i]) == 0) {
			bVerbose= 1;
		} else if (strcmp("-w", szArgv[i]) == 0 || strcmp("--workspaces", szArgv[i]) == 0) {
			i+= 1;
			if (i < nArgc) {
				sscanf(szArgv[i], "%d", &nWorkspaces);
				if (nWorkspaces <= 0 || nWorkspaces > 9) {
					fprintf(stderr, "%s: illegal number of workspaces '%s' for option '%s' (has to be 1-9)\n\n", getApplicationName(), szArgv[i], szArgv[i-1]);
					usage(0);
				}
			} else {
				fprintf(stderr, "%s: workspace count expected for '%s'\n\n", getApplicationName(), szArgv[i-1]);
				usage(0);
			}
		} else if (strcmp("-s", szArgv[i]) == 0 || strcmp("--size", szArgv[i]) == 0) {
			i+= 1;
			if (i < nArgc) {
				sscanf(szArgv[i], "%dx%d", &nSizeX, &nSizeY);
				if (nSizeX <= 0 || nSizeX > 3 || nSizeY <= 0 || nSizeY > 3) {
					fprintf(stderr, "%s: illegal size '%s' for option '%s' (has to be 1x1 .. 3x3)\n\n", getApplicationName(), szArgv[i], szArgv[i-1]);
					usage(0);
				}
			} else {
				fprintf(stderr, "%s: size argument expected for '%s'\n\n", getApplicationName(), szArgv[i-1]);
				usage(0);
			}
		} else if (strcmp("-i", szArgv[i]) == 0 || strcmp("--installdir", szArgv[i]) == 0) {
			i+= 1;
			if (i < nArgc) {
				struct stat buf;
				szInstallDir= szArgv[i];
				if (stat(szInstallDir, &buf) != 0) {
					fprintf(stderr, "%s: cannot access installation directory '%s'\n\n", getApplicationName(), szArgv[i]);
					usage(0);
				}
			} else {
				fprintf(stderr, "%s: display argument expected for '%s'\n\n", getApplicationName(), szArgv[i-1]);
				usage(0);
			}
		} else if (strcmp("--disable-tooltips", szArgv[i]) == 0) {
			bTooltipSupport= 0;
		} else if (strcmp("--tooltip-outside", szArgv[i]) == 0) {
			bTooltipOutside= 1;
		} else if (strcmp("--tooltip-font", szArgv[i]) == 0) {
			i+= 1;
			if (i < nArgc) {
				szTooltipFont= szArgv[i];
			} else {
				fprintf(stderr, "%s: font argument expected for '%s'\n\n", getApplicationName(), szArgv[i-1]);
				usage(0);
			}
		} else if (strcmp("--tooltip-delay", szArgv[i]) == 0) {
			i+= 1;
			if (i < nArgc) {
				long nDelay= -1;
				sscanf(szArgv[i], "%ld", &nDelay);
				if (nDelay < 0) {
					fprintf(stderr, "%s: illegal tooltip delay '%s' for option '%s'\n\n", getApplicationName(), szArgv[i], szArgv[i-1]);
					usage(0);
				}
				nTooltipShowDelay= nDelay;
			} else {
				fprintf(stderr, "%s: delay argument expected for '%s'\n\n", getApplicationName(), szArgv[i-1]);
				usage(0);
			}
		} else if (strcmp("--tooltip-reshow", szArgv[i]) == 0) {
			i+= 1;
			if (i < nArgc) {
				long nDelay= -1;
				sscanf(szArgv[i], "%ld", &nDelay);
				if (nDelay < 0) {
					fprintf(stderr, "%s: illegal tooltip delay '%s' for option '%s'\n\n", getApplicationName(), szArgv[i], szArgv[i-1]);
					usage(0);
				}
				nTooltipReshowDelay= nDelay;
			} else {
				fprintf(stderr, "%s: delay argument expected for '%s'\n\n", getApplicationName(), szArgv[i-1]);
				usage(0);
			}
		} else if (strcmp("-d", szArgv[i]) == 0 || strcmp("--display", szArgv[i]) == 0) {
			i+= 1;
			if (i < nArgc) {
				szDisplay= szArgv[i];
			} else {
				fprintf(stderr, "%s: display argument expected for '%s'\n\n", getApplicationName(), szArgv[i-1]);
				usage(0);
			}
		} else if (strcmp("-t", szArgv[i]) == 0 || strcmp("--theme", szArgv[i]) == 0) {
			i+= 1;
			if (i < nArgc) {
				szTheme= szArgv[i];
			} else {
				fprintf(stderr, "%s: theme argument expected for '%s'\n\n", getApplicationName(), szArgv[i-1]);
				usage(0);
			}
		} else {
			fprintf(stderr, "%s: unknown option '%s'\n\n", getApplicationName(), szArgv[i]);
			usage(0);
		}
		i+= 1;
	}
	setVerbose(bVerbose);
	if (isVerbose()) {
		char* szRealDisplay= (szDisplay == NULL) ? (char*) getenv("DISPLAY") : szDisplay;
		if (szRealDisplay == NULL) {
			szRealDisplay= "localhost:0.0";
		}
		info();
		fprintf(
			stdout,
			"[        ] startup options:\n" \
			"[        ] - verbose= true\n" \
			"[        ] - display= '%s'\n" \
			"[        ] - installdir= '%s'\n" \
			"[        ] - theme= '%s'\n" \
			"[        ] - workspaces= '%d'\n" \
			"[        ] - size= '%dx%d'\n" \
			"[        ] - tooltip support= %s\n" \
			"[        ] - tooltip font= %s\n" \
			"[        ] - tooltip show delay= %ld\n" \
			"[        ] - tooltip reshow delay= %ld\n" \
			"[        ] - tooltip outside= %s\n",
			szRealDisplay,
			szInstallDir == NULL ? "<undefined>" : szInstallDir,
			szTheme == NULL ? "<built-in>" : szTheme,
			nWorkspaces,
			nSizeX, nSizeY,
			bTooltipSupport ? "true" : "false", 
			szTooltipFont,
			nTooltipShowDelay, nTooltipReshowDelay,
			bTooltipOutside ? "true" : "false"
		);
	}
	initTime();
	initDisplay(szDisplay);
	initWindow(nArgc, szArgv);
	initScreens();
	initButtons(nWorkspaces, nSizeX, nSizeY);
	initWindowMask(szInstallDir, szTheme);
	initTooltip(bTooltipSupport, szTooltipFont, bTooltipOutside);
	loop(nTooltipShowDelay, nTooltipReshowDelay);
	return 0;
}

/*
 * Verbose
 */

static int _bVerbose;

void setVerbose (int bVerbose) {
	_bVerbose= bVerbose;
}

int isVerbose () {
	return _bVerbose;
}

/*
 * Usage
 */

#define USAGE \
	"usage: %s [options]\n\n" \
	"where options include:\n" \
	"  -h --help           display usage and version information\n" \
	"  -v --verbose        verbose message output\n" \
	"  -d --display        <name> the display to use (defaults to the\n" \
	"                      'DISPLAY' environment variable)\n" \
	"  -s --size           <w>x<h> number of buttons (default depends on the\n" \
	"                      number of workspaces you have, i.e. 2x2 for 4\n" \
	"                      workspaces, 2x3 for 6, maximum is 3x3)\n" \
	"  -w --workspaces     <count> number of workspace buttons to display\n" \
	"                      (default is the number of workspaces you have,\n" \
	"                      maximum is 9)\n" \
	"  -t --theme          <theme.xpm> the button theme to use, extension\n" \
	"                      '.xpm' is optional, for more information about\n" \
	"                      themes see docu (default is the built-in theme)\n" \
	"  -i --installdir     <dir> specifies the installation directory location,\n" \
	"                      this location is automatically searched for themes\n" \
	"                      (defaults to the '/usr/local/share/wmpager/'\n" \
	"                      and the user specific '~/.wmpager' directory)\n" \
	"  --disable-tooltips  do not display any tooltip windows\n" \
	"  --tooltip-font      <font> use the specified font as tooltip font\n" \
	"                      (default is helvetica, bold, roman, 12 point)\n" \
	"  --tooltip-delay     <millis> set the delay before the tooltip window\n" \
	"                      is popped up (default is 750 milliseconds)\n" \
	"  --tooltip-reshow    <millis> set the tooltip reshow delay (triggered\n" \
	"                      when moving from button to button (default is\n" \
	"                      1500 milliseconds)\n" \
	"  --tooltip-outside   display tooltip window outside of docklet\n"

void usage (int bVerbose) {
	if (bVerbose) {
		info();
		fprintf(stdout, USAGE, getApplicationName());
		exit(0);
	} else {
		fprintf(stderr, USAGE, getApplicationName());
		exit(-1);
	}
}

void info () {
	char* szRev= strdup(BUILD_REV);
	char* szDate= strdup(BUILD_DATE);
	szRev= &szRev[11];
	szRev[strlen(szRev) - 2]= '\0';
	szDate= &szDate[7];
	szDate[strlen(szDate) - 2]= '\0';
	fprintf(stdout, "%s %s (build %s, %s)\n\n", APPLICATION, VERSION, szRev, szDate);
}

/*
 * Application
 */

static char* _szApplicationName;

char* getApplicationName () {
	return _szApplicationName;
}

void initApplicationName (char* szApplicationName) {
	if (szApplicationName == NULL) {
		_szApplicationName= APPLICATION;
	} else {
		_szApplicationName= strdup(szApplicationName);
	}
}

/*
 * Display
 */

static Display* _display;

Display* getDisplay () {
	return _display;
}

void destroyDisplay () {
	XCloseDisplay(getDisplay());
}

void initDisplay (char* szDisplay) {
	if (szDisplay == NULL && ((char*) getenv("DISPLAY")) == NULL) {
		szDisplay= ":0.0";
	}
	if (isVerbose()) {
		char* szRealDisplay= (szDisplay == NULL) ? (char*) getenv("DISPLAY") : szDisplay;
		if (szRealDisplay == NULL) {
			szRealDisplay= "localhost:0.0";
		}
		fprintf(stdout, "[%8ld] initializing display '%s'\n", currentTimeMillis(), szRealDisplay);
	}
	_display= XOpenDisplay(szDisplay);
	if (_display == NULL) {
		fprintf(
			stderr, 
			"%s: couldn't open display '%s'.\n",
			getApplicationName(),
			(szDisplay == NULL) ? ((char*) getenv("DISPLAY")) : szDisplay
		);
		exit(-1);
	}
}

/*
 * Window
 */

static int _nDefaultScreen, _nDefaultDepth;
static Window _wRoot, _wMain, _wIcon;
static GC _gcMain, _gcWindow;
static XpmAttributes _attrButtonTheme;
static Pixmap _pButtonTheme, _pButtonThemeMask;
static XpmAttributes _attrWindow;
static Pixmap _pWindow, _pWindowMask;
static Pixel _pWhite, _pBlack;

Pixel getWhitePixel () {
	return _pWhite;
}

Pixel getBlackPixel () {
	return _pBlack;
}

int getDefaultScreen () {
	return _nDefaultScreen;
}

int getDefaultDepth () {
	return _nDefaultDepth;
}

Window getRootWindow () {
	return _wRoot;
}

Window getMainWindow () {
	return _wMain;
}

Window getIconWindow () {
	return _wIcon;
}

GC getMainGraphics () {
	return _gcMain;
}

GC getWindowGraphics () {
	return _gcWindow;
}

void initWindow (int nArgc, char** szArgv) {
	char* szApplicationName= getApplicationName();
	Display* display= getDisplay();
	XSizeHints *xsizehints;
	XWMHints* xwmhints;
	XClassHint* xclasshint;
	XTextProperty* xtApplication;
	XGCValues xgcMain;
	
	if (isVerbose()) {
		fprintf(stdout, "[%8ld] initializing application window\n", currentTimeMillis());
	}

	_nDefaultScreen= DefaultScreen(display);
	_nDefaultDepth= DefaultDepth(display, _nDefaultScreen);
	_wRoot= RootWindow(display, _nDefaultScreen);

	XSelectInput(display, _wRoot, PropertyChangeMask);
	
	_pWhite= WhitePixel(display, _nDefaultScreen);
	_pBlack= BlackPixel(display, _nDefaultScreen);

	xsizehints= XAllocSizeHints();
	xsizehints->flags= USSize | USPosition;
	xsizehints->width= xsizehints->height= 64;
	
	_wMain= XCreateSimpleWindow(display, _wRoot, 0, 0, 64, 64, 5, _pWhite, _pBlack);
	if (_wMain == 0) {
		fprintf(stderr, "Cannot create main window.\n");
		exit(-1);
	}
	
	_wIcon= XCreateSimpleWindow(display, _wMain, 0, 0, 64, 64, 5, _pWhite, _pBlack);
	if (_wIcon == 0) {
		fprintf(stderr, "Cannot create icon window.\n");
		exit(-1);
	}
	
	xwmhints= XAllocWMHints();
	xwmhints->flags= WindowGroupHint | IconWindowHint | StateHint;    
 	xwmhints->icon_window= _wIcon;
	xwmhints->window_group= _wMain;
	xwmhints->initial_state= WithdrawnState;
	XSetWMHints(display, _wMain, xwmhints);
	
	xclasshint= XAllocClassHint();
	xclasshint->res_name= APPLICATION;
	xclasshint->res_class= APPLICATION;
	XSetClassHint(display, _wMain, xclasshint);

	XSetWMNormalHints(display, _wMain, xsizehints);

	xtApplication= (XTextProperty*) malloc(sizeof(XTextProperty));
	if (XStringListToTextProperty(&szApplicationName, 1, xtApplication) == 0) {
		fprintf(stderr, "Cannot set window title.\n");
		exit(-1);
	}
	XSetWMName(display, _wMain, xtApplication);
	
	_gcMain= XCreateGC(display, _wMain, (GCForeground | GCBackground), &xgcMain);
	if (_gcMain == NULL) {
		fprintf(stderr, "Cannot create graphics context.\n");
		exit(-1);
	}

	XSelectInput(display, _wMain, ExposureMask | ButtonPressMask | PointerMotionMask | StructureNotifyMask | LeaveWindowMask);
	XSelectInput(display, _wIcon, ExposureMask | ButtonPressMask | PointerMotionMask | StructureNotifyMask | LeaveWindowMask);
	
	XSetCommand(display, _wMain, szArgv, nArgc);
	
	XMapWindow(display, _wMain);
}

void destroyWindow () {
	XFreeGC(getDisplay(), getWindowGraphics());
	XFreeGC(getDisplay(), getMainGraphics());
	XDestroyWindow(getDisplay(), getMainWindow());
	XDestroyWindow(getDisplay(), getIconWindow());
}

void initWindowMask (char* szInstallDir, char* szButtonTheme) {
	Display* display= getDisplay();
	GC gc;
	Window wRoot= getRootWindow();
	Window wMain= getMainWindow();
	Window wIcon= getIconWindow();
	XGCValues xgc, xgcWindow;
	Pixmap pOpaque, pTransparent, pMask;
	char* mask= (char*) malloc(512);
	int i;

	if (isVerbose()) {
		fprintf(stdout, "[%8ld] initializing window mask\n", currentTimeMillis());
	}
	for (i= 0; i < 512; i++) {
		mask[i]= 0x00;
	}
	pTransparent= XCreateBitmapFromData(display, wRoot, mask, 64, 64);
	if (pTransparent == 0) {
		fprintf(stderr, "%s: couldn't create window mask (transparent).\n", getApplicationName());
		exit(-1);
	}
	pMask= XCreateBitmapFromData(display, wRoot, mask, 64, 64);
	if (pMask == 0) {
		fprintf(stderr, "%s: couldn't create window mask (mask buffer).\n", getApplicationName());
		exit(-1);
	}

	for (i= 0; i < 512; i++) {
		mask[i]= 0xff;
	}
	pOpaque= XCreateBitmapFromData(display, wRoot, mask, 64, 64);
	if (pOpaque == 0) {
		fprintf(stderr, "%s: couldn't create window mask (opaque).\n", getApplicationName());
		exit(-1);
	}

	gc= XCreateGC(display, pMask, (GCForeground | GCBackground), &xgc);
	if (gc == NULL) {
		fprintf(stderr, "%s: couldn't create window mask (mask graphics).\n", getApplicationName());
		exit(-1);
	}
	for (i= 0; i < getButtonCount(); i++) {
		int nButtonX, nButtonY;
		getButtonLocation(i, &nButtonX, &nButtonY);
		XCopyArea(display, pOpaque, pMask, gc, nButtonX, nButtonY, getButtonWidth(), getButtonHeight(), nButtonX, nButtonY);
	}
	
	XFreePixmap(display, pOpaque);
	XFreePixmap(display, pTransparent);
	XFreeGC(display, gc);

	XShapeCombineMask(display, wMain, ShapeBounding, 0, 0, pMask, ShapeSet);
	XShapeCombineMask(display, wIcon, ShapeBounding, 0, 0, pMask, ShapeSet);

	if (isVerbose()) {
		fprintf(stdout, "[%8ld] initializing button theme '%s'\n", currentTimeMillis(), 
			szButtonTheme == NULL ? "<built-in>" : szButtonTheme);
	}

	_attrButtonTheme.valuemask|= (XpmReturnPixels | XpmReturnExtensions);
	if (szButtonTheme == NULL) {
		if (
			XpmCreatePixmapFromData(
				display, wRoot, buttons_xpm, &_pButtonTheme, &_pButtonThemeMask, &_attrButtonTheme
			) != XpmSuccess
		) {
			fprintf(stderr, "%s: couldn't create button theme.\n", getApplicationName());
			exit(-1);
		}
	} else {
		int bCheckAgain= 0;
		struct stat buf;
		/* check for absolute button theme pathname */
		if (stat(szButtonTheme, &buf) == -1) {
			char* szNewTheme= (char*) malloc(strlen(szButtonTheme) + 4);
			strcpy(szNewTheme, szButtonTheme);
			strcat(szNewTheme, ".xpm");
			if (isVerbose()) {
				fprintf(stdout, "[%8ld] theme file '%s' not found, trying '%s'\n", currentTimeMillis(), szButtonTheme, szNewTheme);
			}
			/* check for absolute button theme pathname (with .xpm added) */
			if (stat(szNewTheme, &buf) == 0) {
				szButtonTheme= szNewTheme;
				if (isVerbose()) {
					fprintf(stdout, "[%8ld] initializing button theme '%s'\n", currentTimeMillis(), szButtonTheme);
				}
			} else {
				bCheckAgain= 1;
				free(szNewTheme);
			}
		}
		if (bCheckAgain && szInstallDir != NULL) {
			char* szNewTheme= (char*) malloc(strlen(szInstallDir) + strlen(szButtonTheme) + 5);
			strcpy(szNewTheme, szInstallDir);
			if (szNewTheme[strlen(szNewTheme) - 1] != '/') {
				strcat(szNewTheme, "/");
			}
			strcat(szNewTheme, szButtonTheme);
			if (stat(szNewTheme, &buf) == 0) {
				bCheckAgain= 0;
				szButtonTheme= szNewTheme;
				if (isVerbose()) {
					fprintf(stdout, "[%8ld] initializing button theme '%s'\n", currentTimeMillis(), szButtonTheme);
				}
			} else {
				strcat(szNewTheme, ".xpm");
				if (stat(szNewTheme, &buf) == 0) {
					bCheckAgain= 0;
					szButtonTheme= szNewTheme;
					if (isVerbose()) {
						fprintf(stdout, "[%8ld] initializing button theme '%s'\n", currentTimeMillis(), szButtonTheme);
					}
				}
			}
		}
		if (bCheckAgain) {
			/* as a goody check the ~/.wmpager directory if it exists */
			char* szHome= (char*) getenv("HOME");
			if (szHome) {
				/* one really shouldn't copy&paste but hey this is a q&d tool */
				char* szNewTheme= (char*) malloc(strlen(szHome) + strlen(szButtonTheme) + strlen(WMPAGER_USER_DIR) + 5);
				strcpy(szNewTheme, szHome);
				if (szNewTheme[strlen(szNewTheme) - 1] != '/') {
					strcat(szNewTheme, "/");
				}
				strcat(szNewTheme, WMPAGER_USER_DIR);
				strcat(szNewTheme, szButtonTheme);
				if (stat(szNewTheme, &buf) == 0) {
					bCheckAgain= 0;
					szButtonTheme= szNewTheme;
					if (isVerbose()) {
						fprintf(stdout, "[%8ld] initializing button theme '%s'\n", currentTimeMillis(), szButtonTheme);
					}
				} else {
					strcat(szNewTheme, ".xpm");
					if (stat(szNewTheme, &buf) == 0) {
						bCheckAgain= 0;
						szButtonTheme= szNewTheme;
						if (isVerbose()) {
							fprintf(stdout, "[%8ld] initializing button theme '%s'\n", currentTimeMillis(), szButtonTheme);
						}
					}
				}
			}
		}
		if (
			XpmReadFileToPixmap(
				display, wRoot, szButtonTheme, &_pButtonTheme, &_pButtonThemeMask, &_attrButtonTheme
			) != XpmSuccess
		) {
			fprintf(stderr, "%s: couldn't read button theme '%s'.\n", getApplicationName(), szButtonTheme);
			exit(-1);
		}
	}

	if (isVerbose()) {
		fprintf(stdout, "[%8ld] initializing screen buffer\n", currentTimeMillis());
	}

	_attrWindow.valuemask|= (XpmReturnPixels | XpmReturnExtensions);
	if (
		XpmCreatePixmapFromData(
			display, wRoot, screen_xpm, &_pWindow, &_pWindowMask, &_attrWindow
		) != XpmSuccess
	) {
		fprintf(stderr, "%s: couldn't create screen buffer.\n", getApplicationName());
		exit(-1);
	}

	_gcWindow= XCreateGC(_display, _pWindow, (GCForeground | GCBackground), &xgcWindow);
	if (gc == NULL) {
		fprintf(stderr, "%s: couldn't create screen buffer graphics.\n", getApplicationName());
		exit(-1);
	}
}

void redrawWindow () {
	XEvent event;
	int i;
	int w= getButtonWidth();
	int h= getButtonHeight();
	for (i= 0; i < getButtonCount(); i++) {
		int x, y, xoff, yoff;
		xoff= 51;
		yoff= 10;
		if (i == getCurrentScreen()) {
			xoff= 0;
			yoff= 0;
		}
		getButtonLocation(i, &x, &y);
		XSetClipMask(_display, _gcWindow, _pWindowMask);
		XSetClipOrigin(_display, _gcWindow, 0, 0);
		XCopyArea(_display, _pButtonTheme, _pWindow, _gcWindow, xoff + x - 7, y - 7, w, h, x, y);
		XCopyArea(_display, _pButtonTheme, _pWindow, _gcWindow, xoff, 0, w, 1, x, y);
		XCopyArea(_display, _pButtonTheme, _pWindow, _gcWindow, xoff, 0, 1, h, x, y);
		XCopyArea(_display, _pButtonTheme, _pWindow, _gcWindow, xoff + 50, 0, 1, h, x + w - 1, y);
		XCopyArea(_display, _pButtonTheme, _pWindow, _gcWindow, xoff, 50, w, 1, x, y + h - 1);
		XSetClipMask(_display, _gcWindow, _pButtonThemeMask);
		XSetClipOrigin(_display, _gcWindow, (x + (w - 10) / 2) - i * 10, -51 - yoff + y + (h - 10) / 2);
		XCopyArea(_display, _pButtonTheme, _pWindow, _gcWindow, i * 10, 51 + yoff, 10, 10, x + (w - 10) / 2, y + (h - 10) / 2);
	}
	while (XCheckTypedWindowEvent(_display, _wMain, Expose, &event));
	XCopyArea(_display, _pWindow, _wMain, _gcMain, 0, 0, 64, 64, 0, 0);
	while (XCheckTypedWindowEvent(_display, _wIcon, Expose, &event));
	XCopyArea(_display, _pWindow, _wIcon, _gcMain, 0, 0, 64, 64, 0, 0);
}

void getWindowOrigin (Window w, int* nX, int* nY) {
	Window wWindow, wParent, wRoot;
	Window* wChildren;
	unsigned int nChildren;
	unsigned int ww, wh, wb, wd;
	int wx, wy;

	wParent= w;
	do {
		wWindow= wParent;
		if (!XQueryTree(getDisplay(), wParent, &wRoot, &wParent, &wChildren, &nChildren)) {
			return;
		}
		if (wChildren) {
			XFree(wChildren);
		}
	} while (wParent != wRoot);

	if (XGetGeometry(getDisplay(), wWindow, &wRoot, &wx, &wy, &ww, &wh, &wb, &wd)) {
		if (nX) {
			*nX= wx;
		}
		if (nY) {
			*nY= wy;
		}
	}
}

/*
 * Event Loop
 */

void loop (long nTooltipShowDelay, long nTooltipReshowDelay) {
	Display* display= getDisplay();
	XEvent event;
	long nTooltipTimer= -1, nTooltipHideTimer= -1, nNow;
	int nTooltipButton, nTooltipX, nTooltipY;

	if (isVerbose()) {
		fprintf(stdout, "[%8ld] starting event loop\n", currentTimeMillis());
	}
	for (;;) {
		while (XPending(display) || (nTooltipTimer == -1)) {
			XNextEvent(display, &event);
			switch (event.type) {
				case Expose:
					if (event.xexpose.count == 0) {
						redrawWindow();
					}
					break;
				case ConfigureNotify:
					redrawWindow();
					break;
				case MotionNotify:
					if (hasTooltipSupport()) {
						if (!hasTooltip()) {
							nTooltipTimer= currentTimeMillis();
							nTooltipX= event.xbutton.x;
							nTooltipY= event.xbutton.y;
							nTooltipButton= getButtonAt(event.xbutton.x, event.xbutton.y);
						} else {
							int nButton= getButtonAt(event.xbutton.x, event.xbutton.y);
							if (nButton != nTooltipButton) {
								hideTooltip();
								nTooltipTimer= -1;
								nTooltipX= event.xbutton.x;
								nTooltipY= event.xbutton.y;
								nTooltipButton= nButton;
								showTooltip(nTooltipButton, nTooltipX, nTooltipY);
							}
						}
					}
					break;
				case LeaveNotify:
					if (hasTooltip()) {
						hideTooltip();
						nTooltipHideTimer= currentTimeMillis();
					}
					nTooltipTimer= -1;
					break;
				case ButtonPress:
					{
						int nButton= getButtonAt(event.xbutton.x, event.xbutton.y);
						if (isVerbose()) {
							fprintf(stdout, "[%8ld] button %d pressed\n", currentTimeMillis(), nButton);
						}
						if (nButton != -1) {
							if (hasTooltip()) {
								hideTooltip();
								nTooltipHideTimer= currentTimeMillis();
							}
							setCurrentScreen(nButton);
							gotoScreen(nButton);
						}
					}
					break;
				case PropertyNotify:
					if (strcmp(XA_WIN_WORKSPACE, XGetAtomName(getDisplay(), event.xproperty.atom)) == 0) {
						setCurrentScreen(-1);
						if (isVerbose()) {
							fprintf(stdout, "[%8ld] new current workspace (%d= %s)\n", 
								currentTimeMillis(), getCurrentScreen(), getScreenName(getCurrentScreen()));
						}
						redrawWindow();
					}
					break;
				case DestroyNotify:
					if (isVerbose()) {
						fprintf(stdout, "[%8ld] quit application\n", currentTimeMillis());
					}
					destroyTooltip();
					destroyWindow();
					destroyDisplay();
					exit(0);
					break;
			}
		}
		usleep(50000);
		nNow= currentTimeMillis();
		if (
			nTooltipTimer != -1 && 
			(
				(nNow > nTooltipTimer + nTooltipShowDelay) ||
				(nNow < nTooltipHideTimer + nTooltipReshowDelay)
			)
		)	{
			showTooltip(nTooltipButton, nTooltipX, nTooltipY);
			nTooltipTimer= -1;
		}
	}
}

/*
 * Button Helpers
 */

static int _nButtons;
static int _nButtonRows, _nButtonColumns;
static int _nButtonWidth, _nButtonHeight;

int getButtonCount () {
	return _nButtons;
}

int getButtonRowCount () {
	return _nButtonRows;
}

int getButtonColumnCount () {
	return _nButtonColumns;
}

int getButtonWidth () {
	return _nButtonWidth;
}

int getButtonHeight () {
	return _nButtonHeight;
}

int getButtonAt (int nLocationX, int nLocationY) {
	int i, nButtonX, nButtonY;
	for (i= 0; i < _nButtons; i++) {
		getButtonLocation(i, &nButtonX, &nButtonY);
		if (
			nLocationX >= nButtonX && nLocationX < nButtonX + _nButtonWidth &&
			nLocationY >= nButtonY && nLocationY < nButtonY + _nButtonHeight
		) {
			return i;
		}
	}
	return -1;
}

void getButtonLocation (int nButton, int* nLocationX, int* nLocationY) {
	if (nButton < 0 || nButton > _nButtons) {
		*nLocationX= *nLocationY= 0;
	} else {
		*nLocationX= *nLocationY= 7;
		while (nButton >= _nButtonColumns) {
			*nLocationY+= _nButtonHeight + 3;
			nButton-= _nButtonColumns;
		}
		while (nButton > 0) {
			*nLocationX+= _nButtonWidth + 3;
			nButton--;
		}
	}
}

void initButtons (int nButtons, int nColumns, int nRows) {
	if (nButtons != -1) {
		_nButtons= nButtons;
	} else {
		_nButtons= getScreenCount();
		if (_nButtons > 9) {
			/* only handle nine screens at most */
			_nButtons= 9;
		}
	}
	if (nColumns == -1 || nRows == -1) {
		switch (_nButtons) {
			case 1:
				_nButtonRows= _nButtonColumns= 1;
				break;
			case 2:
				_nButtonColumns= 1;
				_nButtonRows= 2;
				break;
			case 3:
				/* fallthrough */
			case 4:
				_nButtonColumns= _nButtonRows= 2;
				break;
			case 5:
				/* fallthrough */
			case 6:
				_nButtonColumns= 2;
				_nButtonRows= 3;
				break;
			default:
				_nButtonColumns= _nButtonRows= 3;
				break;
		}
	} else {
		_nButtonColumns= nColumns;
		_nButtonRows= nRows;
	}
	if (_nButtons > _nButtonColumns * _nButtonRows) {
		_nButtons= _nButtonColumns * _nButtonRows;
	}
	if (isVerbose()) {
		fprintf(stdout, "[%8ld] initializing buttons\n", currentTimeMillis());
		fprintf(stdout, "[%8ld] - %d workspace buttons\n", currentTimeMillis(), _nButtons);
		fprintf(stdout, "[%8ld] - button layout %dx%d\n", currentTimeMillis(), _nButtonColumns, _nButtonRows);
	}
	
	if (_nButtonColumns == 1) {
		_nButtonWidth= 51;
	} else if (_nButtonColumns == 2) {
		_nButtonWidth= 24;
	} else {
		_nButtonWidth= 15;
	}

	if (_nButtonRows == 1) {
		_nButtonHeight= 51;
	} else if (_nButtonRows == 2) {
		_nButtonHeight= 24;
	} else {
		_nButtonHeight= 15;
	}

}

/*
 * Time
 */

static struct timeval _tStart;

void initTime () {
	if (isVerbose()) {
		fprintf(stdout, "[        ] initializing time\n");
	}
	gettimeofday(&_tStart, NULL);
}

long currentTimeMillis () {
	struct timeval tNow;
	struct timeval tElapsed;

	gettimeofday(&tNow, NULL);

	if (_tStart.tv_usec > tNow.tv_usec) {
		tNow.tv_usec+= 1000000;
		tNow.tv_sec--;
	}
	tElapsed.tv_sec= tNow.tv_sec - _tStart.tv_sec;
	tElapsed.tv_usec= tNow.tv_usec - _tStart.tv_usec;
	return (tElapsed.tv_sec * 1000) + (tElapsed.tv_usec / 1000);
}

/*
 * Screen Handling
 */

static Atom _xaWinWorkspace;
static Atom _xaWinWorkspaceNames;
static int _nScreens;
static char** _szScreenNames;
static int _nLastScreen;

int getScreenCount () {
	return _nScreens;
}

char* getScreenName (int nScreen) {
	if (nScreen < 0 || nScreen >= _nScreens) {
		return NULL;
	}
	return _szScreenNames[nScreen];
}

void gotoScreen (int nWorkspace) {
	XEvent event;
	event.type= ClientMessage;
	event.xclient.type= ClientMessage;
	event.xclient.window= getRootWindow();
	event.xclient.message_type= _xaWinWorkspace;
	event.xclient.format= 32;
	event.xclient.data.l[0]= nWorkspace;
	event.xclient.data.l[1]= currentTimeMillis();
	XSendEvent(getDisplay(), getRootWindow(), False, SubstructureNotifyMask, (XEvent*) &event);
}

int getCurrentScreen () {
	return _nLastScreen;
}

void setCurrentScreen (int nCurrentScreen) {
	if (nCurrentScreen == -1) {
		long nScreen= _nLastScreen;
		Atom xaType;
		int nFormat;
		unsigned long nItems, nBytesAfter;
		unsigned char* data;
		
		XGetWindowProperty(
			getDisplay(), getRootWindow(), _xaWinWorkspace,
			0, 8192, False, XA_CARDINAL, &xaType, &nFormat, &nItems, &nBytesAfter, &data
		);
		if ((nFormat == 32) && (nItems == 1) && (nBytesAfter == 0)) {
			nScreen= *(long*) data;
		}
		if (xaType != None) {
			XFree(data);
		}
		_nLastScreen= nScreen;
	} else {
		_nLastScreen= nCurrentScreen;
	}
}

void initScreens () {
	XTextProperty tp;
	
	if (isVerbose()) {
		fprintf(stdout, "[%8ld] initializing window maker communication\n", currentTimeMillis());
	}
	_xaWinWorkspace= XInternAtom(getDisplay(), XA_WIN_WORKSPACE, False);
	_xaWinWorkspaceNames= XInternAtom(getDisplay(), XA_WIN_WORKSPACE_NAMES, False);
	XGetTextProperty(getDisplay(), getRootWindow(), &tp, _xaWinWorkspaceNames);
	XTextPropertyToStringList(&tp, &_szScreenNames, &_nScreens);
	_nLastScreen= -1;
	setCurrentScreen(-1);
	if (_nLastScreen == -1) {
		fprintf(
			stderr, 
			"%s: couldn't determine current workspace.\n" \
			"Make sure your WindowMaker has Gnome support enabled!\n", 
			getApplicationName()
		);
		setCurrentScreen(0);
	}
	if (isVerbose()) {
		int i;
		fprintf(stdout, "[%8ld] - %d worspaces found\n", currentTimeMillis(), getScreenCount());
		for (i= 0; i < getScreenCount(); i++) {
			fprintf(stdout, "[%8ld] - workspace %d: %s\n", currentTimeMillis(), i, getScreenName(i));
		}
		fprintf(stdout, "[%8ld] - current workspace is %d (%s)\n", currentTimeMillis(), 
			getCurrentScreen(), getScreenName(getCurrentScreen()));
	}
}

/*
 * Tooltip
 */

int _bTooltip= 0, _bTooltipSupport, _bTooltipOutside;
XFontStruct* _fTooltip;
int _nFontHeight, _nFontY;
int _nScreenWidth, _nScreenHeight;
GC _gcMono= 0;
Window _wTooltip;

int hasTooltipSupport () {
	return _bTooltipSupport;
}

void showTooltip (int nButton, int nMouseX, int nMouseY) {
	Pixmap pixmap, mask;
	int nMainWinX, nMainWinY;
	int nButtonX, nButtonY, nButtonWidth, nButtonHeight;
	int nTextY, nX, nY, nWidth, nHeight, nSide;
	char* szText;

	if (!_bTooltipSupport) {
		return;
	}
	if (_bTooltip) {
		hideTooltip();
	}
	_bTooltip= 1;
	if (isVerbose()) {
		fprintf(stdout, "[%8ld] showing tooltip for button %d (%s) at %d, %d\n", currentTimeMillis(), 
			nButton, getScreenName(nButton), nMouseX, nMouseY);
	}

	szText= strdup(getScreenName(nButton));
	nWidth= XTextWidth(_fTooltip, szText, strlen(szText)) + 16;
	nHeight= _nFontHeight + 4;
	if (nHeight < 16) {
		nHeight= 16;
	}
	if (nWidth < nHeight) {
		nWidth= nHeight;
	}
	if (isVerbose()) {
		fprintf(stdout, "[%8ld] tooltip size: %d, %d\n", currentTimeMillis(), nWidth, nHeight);
	}
	
	getWindowOrigin(getIconWindow(), &nMainWinX, &nMainWinY);
	if (_bTooltipOutside) {
		nButtonX= nMainWinX;
		nButtonY= nMainWinY;
		nButtonWidth= 64;
		nButtonHeight= 64;
	} else {
		getButtonLocation(nButton, &nButtonX, &nButtonY);
		nButtonX+= nMainWinX;
		nButtonY+= nMainWinY;
		nButtonWidth= getButtonWidth();
		nButtonHeight= getButtonHeight();
	}
	
	if (nButtonX + nWidth > _nScreenWidth) {
		nSide= TOOLTIP_RIGHT;
		nX= nButtonX - nWidth + nButtonWidth / 2;
		if (nX < 0) {
			nX= 0;
		}
	} else {
		nSide= TOOLTIP_LEFT;
		nX= nButtonX + nButtonWidth / 2;
	}
	if (nX + nWidth > _nScreenWidth) {
		nX= _nScreenWidth - nWidth;
	}

	if (nButtonY - (nHeight + TOOLTIP_SPACE) < 0) {
		nSide|= TOOLTIP_TOP;
		nY= nButtonY + nButtonHeight - 1;
		nTextY= TOOLTIP_SPACE;
	} else {
		nSide|= TOOLTIP_BOTTOM;
		nY= nButtonY - (nHeight + TOOLTIP_SPACE);
		nTextY= 0;
	}
	
	pixmap= createTooltipPixmap(nWidth, nHeight, nSide, &mask);
	
	XSetForeground(getDisplay(), getMainGraphics(), getBlackPixel());
	XSetFont(getDisplay(), getMainGraphics(), _fTooltip->fid);
	XDrawString(getDisplay(), pixmap, getMainGraphics(), 
		8, nTextY + (nHeight - _nFontHeight) / 2 + _nFontY, szText, strlen(szText));
		
	XSetWindowBackgroundPixmap(getDisplay(), _wTooltip, pixmap);

	XResizeWindow(getDisplay(), _wTooltip, nWidth, nHeight + TOOLTIP_SPACE);
	XShapeCombineMask(getDisplay(), _wTooltip, ShapeBounding, 0, 0, mask, ShapeSet);
	XFreePixmap(getDisplay(), mask);
	XMoveWindow(getDisplay(), _wTooltip, nX, nY);
	XMapRaised(getDisplay(), _wTooltip);
	XFreePixmap(getDisplay(), pixmap);

	free(szText);
}

void hideTooltip () {
	if (!_bTooltipSupport) {
		return;
	}
	if (_bTooltip) {
		if (isVerbose()) {
			fprintf(stdout, "[%8ld] hiding tooltip\n", currentTimeMillis());
		}
		XUnmapWindow(getDisplay(), _wTooltip);
		_bTooltip= 0;
	}
}

int hasTooltip () {
	if (!_bTooltipSupport) {
		return 0;
	}
	return _bTooltip;
}

void initTooltip (int bTooltipSupport, char* szFontName, int bTooltipOutside) {
	XSetWindowAttributes attribs;
	unsigned long vmask;
	
	_bTooltipSupport= bTooltipSupport;
	if (!_bTooltipSupport) {
		if (isVerbose()) {
			fprintf(stdout, "[%8ld] initializing tooltips (disabled)\n", currentTimeMillis());
		}
		return;
	}
	if (isVerbose()) {
		fprintf(stdout, "[%8ld] initializing tooltips\n", currentTimeMillis());
	}
	_bTooltipOutside= bTooltipOutside;
	_fTooltip= XLoadQueryFont(getDisplay(), szFontName);
	if (!_fTooltip) {
		fprintf(stderr, "%s: couldn't allocate font '%s'.\n", getApplicationName(), szFontName);
		exit(-1);
	}
	_nFontHeight= _fTooltip->ascent + _fTooltip->descent;
	_nFontY= _fTooltip->ascent;
	_nScreenWidth= WidthOfScreen(ScreenOfDisplay(getDisplay(), getDefaultScreen()));
	_nScreenHeight= HeightOfScreen(ScreenOfDisplay(getDisplay(), getDefaultScreen()));
	if (isVerbose()) {
		fprintf(stdout, "[%8ld] configuring tooltip font:\n" \
			"[%8ld] - '%s'\n[%8ld] - font-height= %d, font-ascent= %d\n" \
			"[%8ld] configuring screen size: %dx%d\n",
			currentTimeMillis(), currentTimeMillis(), szFontName, currentTimeMillis(), _nFontHeight, _nFontY,
			currentTimeMillis(), _nScreenWidth, _nScreenHeight
		);
	}

	vmask= CWSaveUnder | CWOverrideRedirect | CWBorderPixel;
	attribs.save_under= True;
	attribs.override_redirect= True;
	attribs.border_pixel= 0;
	_wTooltip= XCreateWindow(getDisplay(), getRootWindow(), 1, 1, 10, 10, 1,
		CopyFromParent, CopyFromParent, CopyFromParent, vmask, &attribs);
	if (_wMain == 0) {
		fprintf(stderr, "Cannot create tooltip window.\n");
		exit(-1);
	}
}

void destroyTooltip () {
	if (!_bTooltipSupport) {
		return;
	}
	if (_gcMono) {
		XFreeGC(getDisplay(), _gcMono);
		_gcMono= 0;
	}
	XDestroyWindow(getDisplay(), _wTooltip);
}

void drawTooltipBalloon (Pixmap pix, GC gc, int x, int y, int w, int h, int side) {
	Display* display= getDisplay();
	int rad = h*3/10;
	XPoint pt[3];

	XFillArc(display, pix, gc, x, y, rad, rad, 90*64, 90*64);
	XFillArc(display, pix, gc, x, y+h-1-rad, rad, rad, 180*64, 90*64);

	XFillArc(display, pix, gc, x+w-1-rad, y, rad, rad, 0*64, 90*64);
	XFillArc(display, pix, gc, x+w-1-rad, y+h-1-rad, rad, rad, 270*64, 90*64);

	XFillRectangle(display, pix, gc, x, y+rad/2, w, h-rad);
	XFillRectangle(display, pix, gc, x+rad/2, y, w-rad, h);

	if (side & TOOLTIP_BOTTOM) {
		pt[0].y = y+h-1;
		pt[1].y = y+h-1+TOOLTIP_SPACE;
		pt[2].y = y+h-1;
	} else {
		pt[0].y = y;
		pt[1].y = y-TOOLTIP_SPACE;
		pt[2].y = y;
	}
	if (side & TOOLTIP_RIGHT) {
		pt[0].x = x+w-h+2*h/16;
		pt[1].x = x+w-h+11*h/16;
		pt[2].x = x+w-h+7*h/16;
	} else {
		pt[0].x = x+h-2*h/16;
		pt[1].x = x+h-11*h/16;
		pt[2].x = x+h-7*h/16;
	}
	XFillPolygon(display, pix, gc, pt, 3, Convex, CoordModeOrigin);
}

Pixmap createTooltipPixmap (int width, int height, int side, Pixmap *mask) {
	Display* display= getDisplay();
	Window wRoot= getRootWindow();
	GC gc= getMainGraphics();
	Pixmap bitmap;
	Pixmap pixmap;
	int x, y;

	bitmap = XCreatePixmap(display, wRoot, width+TOOLTIP_SPACE, height+TOOLTIP_SPACE, 1);

	if (!_gcMono) {
		_gcMono= XCreateGC(display, bitmap, 0, NULL);
	}
	XSetForeground(display, _gcMono, 0); 
	XFillRectangle(display, bitmap, _gcMono, 0, 0, width+TOOLTIP_SPACE, height+TOOLTIP_SPACE);

	pixmap = XCreatePixmap(display, wRoot, width+TOOLTIP_SPACE, height+TOOLTIP_SPACE, getDefaultDepth());
	XSetForeground(display, gc, getBlackPixel());
	XFillRectangle(display, pixmap, gc, 0, 0, width+TOOLTIP_SPACE, height+TOOLTIP_SPACE);

	if (side & TOOLTIP_BOTTOM) {
		y = 0;
	} else {
		y = TOOLTIP_SPACE;
	}
	x = 0;

	XSetForeground(display, _gcMono, 1);
	drawTooltipBalloon(bitmap, _gcMono, x, y, width, height, side);
	XSetForeground(display, gc, getWhitePixel());
	drawTooltipBalloon(pixmap, gc, x+1, y+1, width-2, height-2, side);

	*mask = bitmap;

	return pixmap;
}


