/* wmclock.c: a dockable clock applet for Window Maker
 * created 1999-Apr-09 jmk
 *
 * by Jim Knoble <jmknoble@pobox.com>
 * Copyright (C) 1999 Jim Knoble
 *
 * Significant portions of this software are derived from asclock by
 * Beat Christen <spiff@longstreet.ch>.  Such portions are copyright
 * by Beat Christen and the other authors of asclock.
 *
 * Disclaimer:
 *
 * The software is provided "as is", without warranty of any kind,
 * express or implied, including but not limited to the warranties of
 * merchantability, fitness for a particular purpose and
 * noninfringement. In no event shall the author(s) be liable for any
 * claim, damages or other liability, whether in an action of
 * contract, tort or otherwise, arising from, out of or in connection
 * with the software or the use or other dealings in the software.
 */

#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "dynlist.h"

/**********************************************************************/
#define ONLY_SHAPED_WINDOW	1

#define NUM_TIME_POSITIONS	5
#define NUM_X_POSITIONS		11
#define NUM_Y_POSITIONS		4

#define DIGIT_1_X_POS		0
#define DIGIT_2_X_POS		1
#define DIGIT_3_X_POS		3
#define DIGIT_4_X_POS		4
#define DIGIT_Y_POS		0
#define LED_NUM_Y_OFFSET	0
#define LED_THIN_1_X_OFFSET	13
#define LED_NUM_WIDTH		9
#define LED_NUM_HEIGHT		11
#define LED_THIN_1_WIDTH	5

#define COLON_X_POS		2
#define COLON_Y_POS		DIGIT_Y_POS
#define COLON_X_OFFSET		90
#define COLON_Y_OFFSET		0
#define BLANK_X_OFFSET		119
#define BLANK_Y_OFFSET		COLON_Y_OFFSET
#define COLON_WIDTH		3
#define COLON_HEIGHT		11

#define AMPM_X_POS		5
#define AM_X_OFFSET		94
#define AM_Y_OFFSET		5
#define PM_X_OFFSET		107
#define PM_Y_OFFSET		5
#define AM_WIDTH		12
#define AM_HEIGHT		6
#define PM_WIDTH		11
#define PM_HEIGHT		6

#define MONTH_X_POS		10
#define MONTH_Y_POS		3
#define MONTH_X_OFFSET		0
#define MONTH_WIDTH		22
#define MONTH_HEIGHT		6

#define DATE_LEFT_X_POS		7
#define DATE_CENTER_X_POS	8
#define DATE_RIGHT_X_POS	9
#define DATE_Y_POS		2
#define DATE_Y_OFFSET		0
#define DATE_NUM_WIDTH		9
#define DATE_NUM_HEIGHT		13

#define WEEKDAY_X_POS		6
#define WEEKDAY_Y_POS		1
#define WEEKDAY_X_OFFSET	0
#define WEEKDAY_WIDTH		21
#define WEEKDAY_HEIGHT		6

#define OUR_WINDOW_EVENTS	(ExposureMask | ButtonPressMask | StructureNotifyMask)

#define LED_XPM_BRIGHT_LINE_INDEX	3
#define LED_XPM_BRIGHT_CHAR		'+'
#define LED_XPM_DIM_LINE_INDEX		4
#define LED_XPM_DIM_CHAR		'@'

#define DEFAULT_XPM_CLOSENESS	40000

#define DIM_NUMERATOR	5
#define DIM_DENOMINATOR	10
#define makeDimColor(c)	(((c) * DIM_NUMERATOR) / DIM_DENOMINATOR)

/**********************************************************************/
#ifndef ONLY_SHAPED_WINDOW
# include "clk.xpm"
#endif /* !ONLY_SHAPED_WINDOW */
#include MONTH_XPM
#include WEEKDAY_XPM
#include "xpm/date.xpm"
#include "xpm/led.xpm"
#include "xpm/mask.xbm"
#include "xpm/mask.xpm"

typedef struct _XpmIcon {
    Pixmap        pixmap;
    Pixmap        mask;
    XpmAttributes attributes;
} XpmIcon;

void showUsage(void);
void showVersion(void);
int buildCommand(char *, char **, int *, int *);
void executeCommand(char *);
void showError(const char *, const char*);
void showFatalError(const char *, const char*);
void GetXpms(void);
int flushExposeEvents(Window);
void redrawWindow(XpmIcon *);
Pixel GetColor(const char *);
int mytime(void);
void showYear(void);
void showTime12(void);
void showTime24(void);
void showTime(void);
char* extractProgName(char *);
int processArgs(int, char **);

/**********************************************************************/
int enable12HourClock = 0;	/* default value is 24h format */
int enableShapedWindow = 1;	/* default value is noshape */
int enableBlinking = 1;		/* default is blinking */
int startIconified = 0;		/* default is not iconified */
int enableYearDisplay = 0;	/* default is to show time, not year */
unsigned int blinkInterval = 2;          /* default is a 2-second blink cycle */

int timePos12[NUM_TIME_POSITIONS]  = { 5, 14, 24, 28, 37 };
int timePos24[NUM_TIME_POSITIONS]  = { 4,  8, 17, 22, 31 };
/* with shape */
int xPosShaped[NUM_X_POSITIONS] = { 0, 0, 0, 0, 0, 40, 17, 17, 22, 27, 15 };
int yPosShaped[NUM_Y_POSITIONS] = { 3, 21, 30, 45 };

#ifndef ONLY_SHAPED_WINDOW
/* no shape */
int xPosUnshaped[NUM_X_POSITIONS] = { 5, 5, 5, 5, 5, 45, 21, 21, 26, 31, 19 };
int yPosUnshaped[NUM_Y_POSITIONS] = { 7, 25, 34, 49 };
#endif /* !ONLY_SHAPED_WINDOW */

int xPos[NUM_X_POSITIONS];
int yPos[NUM_Y_POSITIONS];

Display    *dpy;
Window     rootWindow;
int        screen;
int        xFd;
fd_set     xFdSet;
int        displayDepth;
XSizeHints sizeHints;
XWMHints   wmHints;
Pixel      bgPixel, fgPixel;
GC         normalGC;
Window     iconWin, win;

char *progName;
char *className = "WMClock";
char *geometry = "";
char *ledColor = "LightSeaGreen";

char *commandToExec = NULL;
char *commandBuf = NULL;
int   commandLength = 0;
int   commandIndex = 0;

char *errColorCells = "not enough free color cells or xpm not found\n";

char *userClockXpm;
char *userMonthXpm;
char *userWeekdayXpm;
int   useUserClockXpm = 0;
int   useUserMonthXpm = 0;
int   useUserWeekdayXpm = 0;

XpmIcon clockBg, led, months, dateNums, weekdays;
XpmIcon visible;

time_t actualTime;
long   actualMinutes;

static struct tm *localTime;

char *usageText[] = {
"Options:",
"    -12                     show 12-hour time (am/pm)",
"    -24                     show 24-hour time",
"    -year                   show year instead of time",
"    -noblink                don't blink",
"    -interval <seconds>     set blink interval",
"    -exe <command>          start <command> on mouse click",
"    -led <color>            use <color> as color of led",
#ifndef ONLY_SHAPED_WINDOW
"    -clockxpm <filename>    get clock background from pixmap in <filename>",
#endif /* !ONLY_SHAPED_WINDOW */
"    -monthxpm <filename>    get month names from pixmap in <filename>",
"    -weekdayxpm <filename>  get weekday names from pixmap in <filename>",
"    -version                display the version",
NULL
};

char *version = VERSION;

/**********************************************************************/
/* Display usage information */
void showUsage(void)
{
   char **cpp;

   fprintf(stderr, "Usage:  %s [option [option ...]]\n\n", progName);
   for (cpp = usageText; *cpp; cpp++)
    {
       fprintf(stderr, "%s\n", *cpp);
    }
   fprintf(stderr,"\n");
   exit(1);
}

/* Display the program version */
void showVersion()
{
   fprintf(stderr, "%s version %s\n", progName, version);
   exit(1);
}

/* Build the shell command to execute */
int buildCommand(char *command, char **buf, int *buf_len, int *i)
{
   int status;

   status = append_string_to_buf(buf, buf_len, i, command);
   if (APPEND_FAILURE == status)
    {
       return (0);
    }
   status = append_string_to_buf(buf, buf_len, i, " &");
   return ((APPEND_FAILURE == status) ? 0 : 1);
}

/* Execute the given shell command */
void executeCommand(char *command)
{
   int status;

   if (NULL == command)
    {
       return;
    }
   status = system(command);

   if (-1 == status)
    {
       perror("system");
    }
}

/* Display an error message */
void showError(const char *message, const char *data)
{
   fprintf(stderr,"%s: can't %s %s\n", progName, message, data);
}

/* Display an error message and exit */
void showFatalError(const char *message, const char *data)
{
   showError(message, data);
   exit(1);
}

/* Konvertiere XPMIcons nach Pixmaps */
void GetXpms(void)
{
   static char     **clock_xpm;
   XColor            color;
   XWindowAttributes attributes;
   char              ledBright[64];
   char              ledDim[64];
   int               status;

#ifdef ONLY_SHAPED_WINDOW
   clock_xpm = mask_xpm;
#else /* !ONLY_SHAPED_WINDOW */
   clock_xpm = enableShapedWindow ? mask_xpm : clk_xpm;
#endif /* ONLY_SHAPED_WINDOW */

   /* for the colormap */
   XGetWindowAttributes(dpy, rootWindow, &attributes);

   /* get user-defined color */
   if (!XParseColor(dpy, attributes.colormap, ledColor, &color))
    {
       showError("parse color", ledColor);
    }

   sprintf(ledBright, "%c      c #%04X%04X%04X", LED_XPM_BRIGHT_CHAR,
	   color.red, color.green, color.blue);
   led_xpm[LED_XPM_BRIGHT_LINE_INDEX] = &ledBright[0];

   color.red   = makeDimColor(color.red);
   color.green = makeDimColor(color.green);
   color.blue  = makeDimColor(color.blue);
   sprintf(&ledDim[0], "%c      c #%04X%04X%04X", LED_XPM_DIM_CHAR,
	   color.red, color.green, color.blue);
   led_xpm[LED_XPM_DIM_LINE_INDEX] = &ledDim[0];

   clockBg.attributes.closeness = DEFAULT_XPM_CLOSENESS;
   clockBg.attributes.valuemask |=
      (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);

   if (useUserClockXpm)
    {
       status = XpmReadFileToPixmap(dpy, rootWindow, userClockXpm,
				    &clockBg.pixmap, &clockBg.mask,
				    &clockBg.attributes);
    }
   else
    {
       status = XpmCreatePixmapFromData(dpy, rootWindow, clock_xpm,
					&clockBg.pixmap, &clockBg.mask,
					&clockBg.attributes);
    }
   if (XpmSuccess != status)
    {
       showFatalError("create clock pixmap:", errColorCells);
    }

#ifdef ONLY_SHAPED_WINDOW
   visible.attributes.depth  = displayDepth;
   visible.attributes.width  = clockBg.attributes.width;
   visible.attributes.height = clockBg.attributes.height;
   visible.pixmap = XCreatePixmap(dpy, rootWindow, visible.attributes.width,
				  visible.attributes.height,
				  visible.attributes.depth);
#else /* !ONLY_SHAPED_WINDOW */
   visible.attributes.closeness = DEFAULT_XPM_CLOSENESS;
   visible.attributes.valuemask |=
      (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);
   status = XpmCreatePixmapFromData(dpy, rootWindow, clk_xpm,
				    &visible.pixmap, &visible.mask,
				    &visible.attributes);
#endif /* ONLY_SHAPED_WINDOW */

   led.attributes.closeness = DEFAULT_XPM_CLOSENESS;
   led.attributes.valuemask |=
      (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);
   status = XpmCreatePixmapFromData(dpy, rootWindow, led_xpm,
				    &led.pixmap, &led.mask,
				    &led.attributes);
   if (XpmSuccess != status)
    {
       showFatalError("create led pixmap:", errColorCells);
    }

   months.attributes.closeness = DEFAULT_XPM_CLOSENESS;
   months.attributes.valuemask |=
      (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);
   if (useUserMonthXpm)
    {
       status = XpmReadFileToPixmap(dpy, rootWindow, userMonthXpm,
				    &months.pixmap, &months.mask,
				    &months.attributes);
    }
   else
    {
       status = XpmCreatePixmapFromData(dpy, rootWindow, month_xpm,
					&months.pixmap, &months.mask,
					&months.attributes);
    }
   if (XpmSuccess != status)
    {
       showFatalError("create month pixmap:", errColorCells);
    }

   dateNums.attributes.closeness = DEFAULT_XPM_CLOSENESS;
   dateNums.attributes.valuemask |=
      (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);
   status = XpmCreatePixmapFromData(dpy, rootWindow, date_xpm,
				    &dateNums.pixmap, &dateNums.mask,
				    &dateNums.attributes);
   if (XpmSuccess != status)
    {
       showFatalError("create date pixmap:", errColorCells);
    }

   weekdays.attributes.closeness = DEFAULT_XPM_CLOSENESS;
   weekdays.attributes.valuemask |=
      (XpmReturnPixels | XpmReturnExtensions | XpmCloseness);
   if (useUserWeekdayXpm)
    {
       status = XpmReadFileToPixmap(dpy, rootWindow, userWeekdayXpm,
				    &weekdays.pixmap, &weekdays.mask,
				    &weekdays.attributes);
    }
   else
    {
       status = XpmCreatePixmapFromData(dpy, rootWindow, weekday_xpm,
					&weekdays.pixmap, &weekdays.mask,
					&weekdays.attributes);
    }
   if (XpmSuccess != status)
    {
       showFatalError("create weekday pixmap:", errColorCells);
    }
}

/* Remove expose events for a specific window from the queue */
int flushExposeEvents(Window w)
{
   XEvent dummy;
   int    i = 0;

   while (XCheckTypedWindowEvent(dpy, w, Expose, &dummy))
    {
       i++;
    }
   return(i);
}

/* (Re-)Draw the main window and the icon window */
void redrawWindow(XpmIcon *v)
{
   flushExposeEvents(iconWin);
   XCopyArea(dpy, v->pixmap, iconWin, normalGC,
	     0, 0, v->attributes.width, v->attributes.height, 0, 0);
   flushExposeEvents(win);
   XCopyArea(dpy, v->pixmap, win, normalGC,
	     0, 0, v->attributes.width, v->attributes.height, 0, 0);
}

/* Get a Pixel for the given color name */
Pixel GetColor(const char *colorName)
{
   XColor            color;
   XWindowAttributes attributes;

   XGetWindowAttributes(dpy, rootWindow, &attributes);
   color.pixel = 0;
   if (!XParseColor(dpy, attributes.colormap, colorName, &color))
    {
       showError("parse color", colorName);
    }
   else if (!XAllocColor(dpy, attributes.colormap, &color))
    {
       showError("allocate color", colorName);
    }
   return(color.pixel);
}

/* Fetch the system time and time zone */
int mytime(void)
{
   struct timeval  tv;
   struct timezone tz;

   gettimeofday(&tv, &tz);

   return(tv.tv_sec);
}

/* Display the current year in the LED display */
void showYear(void)
{
   int year;
   int digitXOffset;
   int digitYOffset;

   year = localTime->tm_year + 1900;

   digitYOffset = LED_NUM_Y_OFFSET;
   digitXOffset = LED_NUM_WIDTH * (year / 1000);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset , digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_1_X_POS], yPos[DIGIT_Y_POS]);
   digitXOffset = LED_NUM_WIDTH * ((year / 100) % 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset , digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_2_X_POS], yPos[DIGIT_Y_POS]);
   digitXOffset = LED_NUM_WIDTH * ((year / 10) % 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset , digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_3_X_POS], yPos[DIGIT_Y_POS]);
   digitXOffset = LED_NUM_WIDTH * (year % 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset , digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_4_X_POS], yPos[DIGIT_Y_POS]);
}

/* Display time in twelve-hour mode, with am/pm indicator */
void showTime12(void)
{
   int digitXOffset;
   int digitYOffset;
   int localHour = localTime->tm_hour % 12;

   if (0 == localHour)
    {
       localHour = 12;
    }
   if (localTime->tm_hour < 12)
    {
       /* AM */
       XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
		 AM_X_OFFSET, AM_Y_OFFSET, AM_WIDTH, AM_HEIGHT,
		 xPos[AMPM_X_POS], yPos[DIGIT_Y_POS] + AM_Y_OFFSET);
    }
   else
    {
       /* PM */
       XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
		 PM_X_OFFSET, PM_Y_OFFSET, PM_WIDTH, PM_HEIGHT,
		 xPos[AMPM_X_POS], yPos[DIGIT_Y_POS] + PM_Y_OFFSET);
    }

   digitYOffset = LED_NUM_Y_OFFSET;
   if (localHour > 9)
    {
       digitXOffset = LED_THIN_1_X_OFFSET;
       XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
		 digitXOffset, digitYOffset, LED_THIN_1_WIDTH, LED_NUM_HEIGHT,
		 xPos[DIGIT_1_X_POS], yPos[DIGIT_Y_POS]);
    }
   digitXOffset = LED_NUM_WIDTH * (localHour % 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset, digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_2_X_POS], yPos[DIGIT_Y_POS]);
   digitXOffset = LED_NUM_WIDTH * (localTime->tm_min / 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset, digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_3_X_POS], yPos[DIGIT_Y_POS]);
   digitXOffset = LED_NUM_WIDTH * (localTime->tm_min % 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset, digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_4_X_POS], yPos[DIGIT_Y_POS]);
}

/* Display time in 24-hour mode, without am/pm indicator */
void showTime24(void)
{
   int digitXOffset;
   int digitYOffset;

   digitYOffset = LED_NUM_Y_OFFSET;
   digitXOffset = LED_NUM_WIDTH * (localTime->tm_hour / 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset, digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_1_X_POS], yPos[DIGIT_Y_POS]);
   digitXOffset = LED_NUM_WIDTH * (localTime->tm_hour % 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset, digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_2_X_POS], yPos[DIGIT_Y_POS]);
   digitXOffset = LED_NUM_WIDTH * (localTime->tm_min / 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset, digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_3_X_POS], yPos[DIGIT_Y_POS]);
   digitXOffset = LED_NUM_WIDTH * (localTime->tm_min % 10);
   XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
	     digitXOffset, digitYOffset, LED_NUM_WIDTH, LED_NUM_HEIGHT,
	     xPos[DIGIT_4_X_POS], yPos[DIGIT_Y_POS]);
}

void showTime(void)
{
   int xOffset;
   int yOffset;

   /* Zeit auslesen */
   actualTime = mytime();
   actualMinutes = actualTime / 60;

   localTime = localtime(&actualTime);

   /* leere clock holen */
   XCopyArea(dpy, clockBg.pixmap, visible.pixmap, normalGC,
	     0, 0, sizeHints.width, sizeHints.height, 0, 0);

   if (enableYearDisplay)
    {
       showYear();
    }
   else if (enable12HourClock)
    {
       showTime12();
    }
   else
    {
       showTime24();
    }

   /* Monat */
   xOffset = MONTH_X_OFFSET;
   yOffset = MONTH_HEIGHT * (localTime->tm_mon);
   XCopyArea(dpy, months.pixmap, visible.pixmap, normalGC,
	     xOffset, yOffset, MONTH_WIDTH, MONTH_HEIGHT,
	     xPos[MONTH_X_POS], yPos[MONTH_Y_POS]);

   /* Datum */
   yOffset = DATE_Y_OFFSET;
   if (localTime->tm_mday > 9)
    {
       xOffset = DATE_NUM_WIDTH * (((localTime->tm_mday / 10) + 9) % 10);
       XCopyArea(dpy, dateNums.pixmap, visible.pixmap, normalGC,
		 xOffset, yOffset, DATE_NUM_WIDTH, DATE_NUM_HEIGHT,
		 xPos[DATE_LEFT_X_POS], yPos[DATE_Y_POS]);
       xOffset = DATE_NUM_WIDTH * (((localTime->tm_mday % 10) + 9) % 10);
       XCopyArea(dpy, dateNums.pixmap, visible.pixmap, normalGC,
		 xOffset, yOffset, DATE_NUM_WIDTH, DATE_NUM_HEIGHT,
		 xPos[DATE_RIGHT_X_POS], yPos[DATE_Y_POS]);
    }
   else
    {
       xOffset = DATE_NUM_WIDTH * (localTime->tm_mday - 1);
       XCopyArea(dpy, dateNums.pixmap, visible.pixmap, normalGC,
		 xOffset, yOffset, DATE_NUM_WIDTH, DATE_NUM_HEIGHT,
		 xPos[DATE_CENTER_X_POS], yPos[DATE_Y_POS]);
    }

   /* Wochentag */
   xOffset = WEEKDAY_X_OFFSET;
   yOffset = WEEKDAY_HEIGHT * ((localTime->tm_wday + 6) % 7);
   XCopyArea(dpy, weekdays.pixmap, visible.pixmap, normalGC,
	     xOffset, yOffset, WEEKDAY_WIDTH, WEEKDAY_HEIGHT,
	     xPos[WEEKDAY_X_POS], yPos[WEEKDAY_Y_POS]);

   if ((!enableBlinking) && (!enableYearDisplay))
    {
       /* Sekunden Doppelpunkt ein */
       xOffset = COLON_X_OFFSET;
       yOffset = COLON_Y_OFFSET;
       XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
		 xOffset, yOffset, COLON_WIDTH, COLON_HEIGHT,
		 xPos[COLON_X_POS], yPos[COLON_Y_POS]);
    }
}

/* Extract program name from the zeroth program argument */
char *extractProgName(char *argv0)
{
   char *prog_name = NULL;

   if (NULL != argv0)
    {
       prog_name = strrchr(argv0, '/');
       if (NULL == prog_name)
	{
	   prog_name = argv0;
	}
       else
	{
	   prog_name++;
	}
    }
   return (prog_name);
}

/* Process program arguments and set corresponding options */
int processArgs(int argc, char **argv)
{
   int i;

   for (i = 1; i < argc; i++)
    {
       if (0 == strcmp(argv[i], "--"))
	{
	   break;
	}
       else if ((0 == strcmp(argv[i], "-12")) ||
		(0 == strcmp(argv[i], "-1")) ||
		(0 == strcmp(argv[i], "--12")))
	{
	   enable12HourClock = 1;
	}
       else if ((0 == strcmp(argv[i], "-24")) ||
		(0 == strcmp(argv[i], "-2")) ||
		(0 == strcmp(argv[i], "--24")))
	{
	   enable12HourClock = 0;
	}
       else if ((0 == strcmp(argv[i], "-exe")) ||
		(0 == strcmp(argv[i], "-e")) ||
		(0 == strcmp(argv[i], "--exe")))
	{
	   if (++i >= argc)
	    {
	       showUsage();
	    }
	   commandToExec = argv[i];
	}
       else if ((0 == strcmp(argv[i], "-led")) ||
		(0 == strcmp(argv[i], "-l")) ||
		(0 == strcmp(argv[i], "--led")))
	{
	   if (++i >= argc)
	    {
	       showUsage();
	    }
	   ledColor = argv[i];
	}
       else if ((0 == strcmp(argv[i], "-clockxpm")) ||
		(0 == strcmp(argv[i], "-c")) ||
		(0 == strcmp(argv[i], "--clockxpm")))
	{
#ifndef ONLY_SHAPED_WINDOW
	   if (++i >= argc)
	    {
	       showUsage();
	    }
	   userClockXpm = argv[i];
	   useUserClockXpm = 1;
#endif /* !ONLY_SHAPED_WINDOW */
	}
       else if ((0 == strcmp(argv[i], "-monthxpm")) ||
		(0 == strcmp(argv[i], "-m")) ||
		(0 == strcmp(argv[i], "--monthxpm")))
	{
	   if (++i >= argc)
	    {
	       showUsage();
	    }
	   userMonthXpm = argv[i];
	   useUserMonthXpm = 1;
	}
       else if ((0 == strcmp(argv[i], "-weekdayxpm")) ||
		(0 == strcmp(argv[i], "-w")) ||
		(0 == strcmp(argv[i], "--weekdayxpm")))
	{
	   if (++i >= argc)
	    {
	       showUsage();
	    }
	   userWeekdayXpm = argv[i];
	   useUserWeekdayXpm = 1;
	}
       else if ((0 == strcmp(argv[i], "-noblink")) ||
		(0 == strcmp(argv[i], "-n")) ||
		(0 == strcmp(argv[i], "--noblink")))
	{
	   enableBlinking = 0;
	}
       else if ((0 == strcmp(argv[i], "-year")) ||
		(0 == strcmp(argv[i], "-y")) ||
		(0 == strcmp(argv[i], "--year")))
	{
	   enableYearDisplay = 1;
	}
       else if ((0 == strcmp(argv[i], "-position")) ||
		(0 == strcmp(argv[i], "-p")) ||
		(0 == strcmp(argv[i], "--position")))
	{
#ifndef ONLY_SHAPED_WINDOW
	   if (++i >= argc)
	    {
	       showUsage();
	    }
	   geometry = argv[i];
#endif /* !ONLY_SHAPED_WINDOW */
	}
       else if ((0 == strcmp(argv[i], "-shape")) ||
		(0 == strcmp(argv[i], "-s")) ||
		(0 == strcmp(argv[i], "--shape")))
	{
	   enableShapedWindow = 1;
	}
       else if ((0 == strcmp(argv[i], "-iconic")) ||
		(0 == strcmp(argv[i], "-i")) ||
		(0 == strcmp(argv[i], "--iconic")))
	{
#ifndef ONLY_SHAPED_WINDOW
	   startIconified = 1;
#endif /* !ONLY_SHAPED_WINDOW */
	}
       else if ((0 == strcmp(argv[i], "-version")) ||
		(0 == strcmp(argv[i], "-V")) ||
		(0 == strcmp(argv[i], "--version")))
	{
	   showVersion();
	}
       else if ((0 == strcmp(argv[i], "-help")) ||
		(0 == strcmp(argv[i], "-h")) ||
		(0 == strcmp(argv[i], "--help")))
	{
	   showUsage();
	}
       else if ((0 == strcmp(argv[i], "-interval")) ||
		(0 == strcmp(argv[i], "--interval")))
	{
	   if (++i >= argc)
	    {
	       showUsage();
	    }
	   blinkInterval = atoi(argv[i]);
	}

       else
	{
	   fprintf(stderr, "%s: unrecognized option `%s'\n",
		   progName, argv[i]);
	   showUsage();
	}
    }
   return (i);
}

/**********************************************************************/
int main(int argc, char **argv)
{
   int           i;
   unsigned int  borderWidth = 0;
   char          *displayName = NULL;
   XGCValues     gcValues;
   unsigned long gcMask;
   XEvent        event;
   XTextProperty wmName;
   XClassHint    classHint;
   Pixmap        shapeMask;
   struct timeval nextEvent;
   unsigned int   blinkCounter = 0;

   /* Parse command line options */
   progName = extractProgName(argv[0]);
   processArgs(argc, argv);

   /* init led position */
#ifndef ONLY_SHAPED_WINDOW
   for (i = 0; i < NUM_Y_POSITIONS; i++)
    {
       yPos[i] = enableShapedWindow ? yPosShaped[i] : yPosUnshaped[i];
    }
   for (i = 0; i < NUM_X_POSITIONS; i++)
    {
       xPos[i] = enableShapedWindow ? xPosShaped[i] : xPosUnshaped[i];
    }
#else /* ONLY_SHAPED_WINDOW */
   for (i = 0; i < NUM_Y_POSITIONS; i++)
    {
       yPos[i] = yPosShaped[i];
    }
   for (i = 0; i < NUM_X_POSITIONS; i++)
    {
       xPos[i] = xPosShaped[i];
    }
#endif /* !ONLY_SHAPED_WINDOW */
   for (i = 0; i < NUM_TIME_POSITIONS; i++)
    {
      if (enable12HourClock && (!enableYearDisplay))
       {
         xPos[i] += timePos24[i];
       }
      else
       {
         xPos[i] += timePos12[i];
       }
    }

   /* Open the display */
   dpy = XOpenDisplay(displayName);
   if (NULL == dpy)
    {
       fprintf(stderr, "%s: can't open display %s\n", progName,
	       XDisplayName(displayName));
       exit(1);
    }
   screen       = DefaultScreen(dpy);
   rootWindow   = RootWindow(dpy, screen);
   displayDepth = DefaultDepth(dpy, screen);
   xFd          = XConnectionNumber(dpy);

   /* Icon Daten nach XImage konvertieren */
   GetXpms();

   /* Create a window to hold the banner */
   sizeHints.x = 0;
   sizeHints.y = 0;
   sizeHints.min_width  = clockBg.attributes.width;
   sizeHints.min_height = clockBg.attributes.height;
   sizeHints.max_width  = clockBg.attributes.width;
   sizeHints.max_height = clockBg.attributes.height;
   sizeHints.base_width  = clockBg.attributes.width;
   sizeHints.base_height = clockBg.attributes.height;
   sizeHints.flags = USSize | USPosition | PMinSize | PMaxSize | PBaseSize;

   bgPixel = GetColor("white");
   fgPixel = GetColor("black");

   XWMGeometry(dpy, screen, geometry, NULL, borderWidth, &sizeHints,
	       &sizeHints.x, &sizeHints.y, &sizeHints.width, &sizeHints.height,
	       &sizeHints.win_gravity);
   sizeHints.width  = clockBg.attributes.width;
   sizeHints.height = clockBg.attributes.height;

   win = XCreateSimpleWindow(dpy, rootWindow, sizeHints.x, sizeHints.y,
			     sizeHints.width, sizeHints.height,
			     borderWidth, fgPixel, bgPixel);
   iconWin = XCreateSimpleWindow(dpy, win, sizeHints.x, sizeHints.y,
				 sizeHints.width, sizeHints.height,
				 borderWidth, fgPixel, bgPixel);

   /* Hints aktivieren */
   XSetWMNormalHints(dpy, win, &sizeHints);
   classHint.res_name = progName;
   classHint.res_class = className;
   XSetClassHint(dpy, win, &classHint);

   XSelectInput(dpy, win, OUR_WINDOW_EVENTS);
   XSelectInput(dpy, iconWin, OUR_WINDOW_EVENTS);

   if (0 == XStringListToTextProperty(&progName, 1, &wmName))
    {
       fprintf(stderr, "%s: can't allocate window name text property\n",
	       progName);
       exit(-1);
    }
   XSetWMName(dpy, win, &wmName);

   /* Create a GC for drawing */
   gcMask = GCForeground | GCBackground | GCGraphicsExposures;
   gcValues.foreground = fgPixel;
   gcValues.background = bgPixel;
   gcValues.graphics_exposures = False;
   normalGC = XCreateGC(dpy, rootWindow, gcMask, &gcValues);

   if (enableShapedWindow)
    {
       shapeMask = XCreateBitmapFromData(dpy, win, (char *)mask_bits,
					 mask_width, mask_height);
       XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, shapeMask, ShapeSet);
       XShapeCombineMask(dpy, iconWin, ShapeBounding, 0, 0, shapeMask,
			 ShapeSet);
    }

   wmHints.initial_state = WithdrawnState;
   wmHints.icon_window = iconWin;
   wmHints.icon_x = sizeHints.x;
   wmHints.icon_y = sizeHints.y;
   wmHints.window_group = win;
   wmHints.flags = StateHint | IconWindowHint | IconPositionHint |
      WindowGroupHint;
   XSetWMHints(dpy, win, &wmHints);

   XSetCommand(dpy, win, argv, argc);
   XMapWindow(dpy,win);

   showTime();
   redrawWindow(&visible);
   while (1)
    {
       if (actualTime != mytime())
	{
	   actualTime = mytime();
	   if (actualMinutes != (actualTime / 60))
	    {
	       showTime();
	       if (!enableBlinking)
		{
		  redrawWindow(&visible);
		}
	    }
	   if (0 == (actualTime % 2))
	    {
	       /* Clean up zombie processes */
#ifdef DEBUG
	       fprintf(stderr, "%s: cleaning up zombies (time %ld)\n",
		       progName, actualTime);
#endif /* DEBUG */
	       if (NULL != commandToExec)
		{
		   waitpid(0, NULL, WNOHANG);
		}
	    }
	}
       if (enableBlinking && (!enableYearDisplay))
        {
            blinkCounter++;
#ifdef SYSV
            if (blinkCounter >= 20*blinkInterval)
#else
            if (blinkCounter >= 2*blinkInterval)
#endif
                blinkCounter = 0;
            if (blinkCounter == 0)
	     {
                /* Sekunden Doppelpunkt ein */
		XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
			  COLON_X_OFFSET, COLON_Y_OFFSET,
			  COLON_WIDTH, COLON_HEIGHT,
			  xPos[COLON_X_POS], yPos[COLON_Y_POS]);
	     }
#ifdef SYSV
	    if (blinkCounter == 10*blinkInterval)
#else
	    if (blinkCounter == blinkInterval)
#endif
	     {
		    /* Sekunden Doppelpunkt aus */
		    XCopyArea(dpy, led.pixmap, visible.pixmap, normalGC,
			      BLANK_X_OFFSET, BLANK_Y_OFFSET,
			      COLON_WIDTH, COLON_HEIGHT,
			      xPos[COLON_X_POS], yPos[COLON_Y_POS]);
	     }
	    redrawWindow(&visible);
        }

       /* read a packet */
       while (XPending(dpy))
	{
	   XNextEvent(dpy, &event);
	   switch(event.type)
	    {
	     case Expose:
	       if (0 == event.xexpose.count)
		{
		   redrawWindow(&visible);
		}
	       break;
	    case ButtonPress:
	       if (NULL != commandToExec)
		{
		   pid_t fork_pid;

		   if ((NULL == commandBuf) &&
		       (!buildCommand(commandToExec, &commandBuf,
				      &commandLength, &commandIndex)))
		    {
		       break;
		    }
		   fork_pid = fork();
		   switch (fork_pid)
		    {
		     case 0:
		       /* We're the child process;
			* run the command and exit.
			*/
		       executeCommand(commandBuf);
		       /* When the system() call finishes, we're done. */
		       exit(0);
		       break;
		     case -1:
		       /* We're the parent process, but
			* fork() gave an error.
			*/
		       perror("fork");
		       break;
		     default:
		       /* We're the parent process;
			* keep on doing what we normally do.
			*/
		       break;
		    }
		}
	       break;
	    case DestroyNotify:
#if 0
	       XFreeGC(dpy, normalGC);
	       XDestroyWindow(dpy, win);
	       XDestroyWindow(dpy, iconWin);
#endif /* 0 */
#ifdef ONLY_SHAPED_WINDOW
	       XFreePixmap(dpy, visible.pixmap);
#endif /* ONLY_SHAPED_WINDOW */
	       XCloseDisplay(dpy);
	       exit(0);
	     default:
	       break;
	    }
	}
       XFlush(dpy);
#ifdef SYSV
       if (enableYearDisplay)
	{
	   poll((struct poll *) 0, (size_t) 0, 200);	/* 1/5 sec */
	}
       else
	{
	   poll((struct poll *) 0, (size_t) 0, 50);	/* 5/100 sec */
	}
#else
       /* We compute the date of next event, in order to avoid polling */
       if (enableBlinking)
	 {
	   gettimeofday(&nextEvent,NULL);
	   nextEvent.tv_sec = 0;
	   if (nextEvent.tv_usec < 500000)
		   nextEvent.tv_usec = 500000-nextEvent.tv_usec;
	   else
		   nextEvent.tv_usec = 1000000-nextEvent.tv_usec;
	 }
       else
	 {
	   if (enableYearDisplay)
	     {
	       nextEvent.tv_sec = 86400-actualTime%86400;
	       nextEvent.tv_usec = 0;
	     }
	   else
	     {
	       nextEvent.tv_sec = 60-actualTime%60;
	       nextEvent.tv_usec = 0;
	     }
	 }
       FD_ZERO(&xFdSet);
       FD_SET(xFd,&xFdSet);
       select(FD_SETSIZE,&xFdSet,NULL,NULL,&nextEvent);
#endif
    }
   return (0);
}

