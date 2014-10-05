/*****************************************************************************/
/*                                                                           */
/*  wmtz.c                                                                   */
/*  Shows local time in different timezones + JD + Sidereal time +           */
/*  internet time + local date and time.                                     */
/*                                                                           */
/*  Jan Lindblom <99jl@home.se> (http://www.geocities.com/~jl1n/)            */
/*                                                                           */
/*  wmtz.c was derived from:                                                 */
/*                                                                           */
/*   wminet.c                                                                */
/*                                                                           */
/*   Multi-function system monitor                                           */
/*   Dave Clark (clarkd@skynet.ca) (http://www.neotokyo.org/illusion)        */
/*   Martijn Pieterse (pieterse@xs4all.nl) (http://windowmaker.mezaway.org)  */
/*   and Antoine Nulle (warp@xs4all.nl) (http://windowmaker.mezaway.org)     */
/*                                                                           */
/*  This software is licensed through the GNU General Public Licence.        */
/*  Read the COPYING file for details.                                       */
/*                                                                           */
/*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"
#include "wmtz.xpm"
#include "wmtz_mono.xpm"


/*
 * Defines
 */
#define WMTZ_VERSION "0.7"
#define CHAR_WIDTH 6
#define STRSIZE 10
#define LMST 1
#define GMST 0
#define ABOUT "xmessage -center -buttons \"Close\" \"WMTZ - Window Maker Time Zone dockapp v0.7
http://www.geocities.com/jl1n/wmtz/wmtz.html\""


/*
 * Typedefs
 */
typedef struct
{
    char label[10];    /* Time zone designation */
    int  diff;         /* Time zone diff. from UT in hours */
    double epoch;      /* Epoch (for JD) */
    char   tz[256];     /* TZ environment variable string */
} timezone_t;          /* ...numbers of days to subtract from JD */


/*
 * Global variables
 */
timezone_t    zone[6];
double        longitude = 0.0;
double        latitude = 0.0;
static        struct tm *clk;
char          *pname;
char          *uconfig_file;
char          *config_file;
char          *defedit;
char          *month[12];
char          *week_day[7];
char          wmtz_mask_bits[64*64];
int           wmtz_mask_width = 64;
int           wmtz_mask_height = 64;
int           mono = 0;
extern char   *tzname[2];
static char   originalTZ[64];

/*
 * Function declarations
 */
void   usage(void);
void   printversion(void);
void   BlitString(char *name, int x, int y);
void   wmtz_routine(int, char **);
int    ReadConfigInt(FILE *fp, char *setting, int *value);
int    ReadConfigString(FILE *fp, char *setting, char *value);
int    Read_Config_File(char *filename);
void   range(double *val, double ran);
void   siderealTime(double jde, int *result, int mode);
double julianDay(int year, int month, double day, double hour,
                 double minute, double second, int julian);
int    calendarDate(double jd, int *year, int *month, double *day);
int    handleJD(void);
void   errH(int printErrno, int exitCode, const char *msg, ...);
void   handleTheMenu(int but_stat);
double jdn(time_t curtime);


/*****************************************************************************\
|* main   	        						     *|
\*****************************************************************************/
int main(int argc, char *argv[]) {

  int		i;
  char    *envbuf;

  /* Store away the executable name for error messages */
  if ( (pname = strrchr( argv[0], '/' )) == NULL )
      pname = argv[0];
  else
      pname++;

  /* Store away the TZ environment variable, if set */
  if ( (envbuf = getenv("TZ")) != NULL )
  {
    //Write TZ=envbuf into originalTZ
    (void) sprintf(originalTZ, "TZ=%s", envbuf);
  }
  else
  {
    // Set originalTZ to TZ erase TZ env.
    (void) sprintf(originalTZ, "TZ");
  }

  /* Parse Command Line */
  for (i=1; i<argc; i++) {
    char *arg = argv[i];

    if (*arg=='-') {
      switch (arg[1]) {
      case 'd' :
	if (strcmp(arg+1, "display")) {
	  usage();
	  exit(1);
	}
	break;
      case 'g' :
	if (strcmp(arg+1, "geometry")) {
	  usage();
	  exit(1);
	}
	break;
      case 'j' :
	if (strcmp(arg+1, "jd")) {
	  usage();
	  exit(1);
	}
        handleJD();
        exit(0);
	break;
      case 'm' :
	  mono = 1;
	  break;
      case 'v' :
	printversion();
	exit(0);
	break;
      case 'c' :
	if (argc > (i+1))
	  {
	    uconfig_file = strdup(argv[i+1]);
            if (uconfig_file == NULL)
                errH(1, 1, "strdup");
	    i++;
	  }
	break;
      case 'e' :
	if (argc > (i+1))
	  {
	    defedit = strdup(argv[i+1]);
            if (defedit == NULL)
                errH(1, 1, "strdup");
	    i++;
	  }
	break;
      default:
	usage();
	exit(0);
	break;
      }
    }
  }

  month[0] = "JAN";  month[1] = "FEB";  month[2] = "MAR";
  month[3] = "APR";  month[4] = "MAY";  month[5] = "JUN";
  month[6] = "JUL";  month[7] = "AUG";  month[8] = "SEP";
  month[9] = "OUT";  month[10] = "NOV";  month[11] = "DEC";

  week_day[0] = "SUNDAY   ";
  week_day[1] = "MONDAY   ";
  week_day[2] = "TUESDAY  ";
  week_day[3] = "WEDNESDAY";
  week_day[4] = "THURSDAY ";
  week_day[5] = "FRIDAY   ";
  week_day[6] = "SATURDAY ";

  wmtz_routine(argc, argv);

  return 0;
}


/*****************************************************************************\
|* wmtz_routine	- Creates the wmtz window.				     *|
\*****************************************************************************/
void wmtz_routine(int argc, char **argv)
{
    int			j = 0,k = 0, hour = 0, i = 0;
    int                 sid[2], clicked = 0, but_stat = -1;
    double              jd = 0;
    float               swatch_beats;
    time_t		curtime;
    time_t		prevtime;
    char                *home;
    char                buf[64];
    char                blitstr[STRSIZE];
    XEvent              Event;

    if (mono)
    {
	fprintf(stderr, "Starting monochrome version.\n");
	createXBMfromXPM(wmtz_mask_bits, wmtz_mono_xpm,
			 wmtz_mask_width, wmtz_mask_height);

	openXwindow(argc, argv, wmtz_mono_xpm, wmtz_mask_bits,
		    wmtz_mask_width, wmtz_mask_height);
    }
    else
    {
	createXBMfromXPM(wmtz_mask_bits, wmtz_master_xpm,
			 wmtz_mask_width, wmtz_mask_height);

	openXwindow(argc, argv, wmtz_master_xpm, wmtz_mask_bits,
		    wmtz_mask_width, wmtz_mask_height);
    }

    memset(&zone, 0, sizeof(zone));

    AddMouseRegion(0, 5, 6, 58, 16);
    AddMouseRegion(1, 5, 16, 58, 26);
    AddMouseRegion(2, 5, 26, 58, 36);
    AddMouseRegion(3, 5, 36, 58, 46);
    AddMouseRegion(4, 5, 46, 58, 56);

    /* Read config file */
    if (uconfig_file != NULL)
    {
        /* user-specified config file */
        fprintf(stderr, "Using user-specified config file '%s'.\n",
                uconfig_file);
        config_file = strdup(uconfig_file);
        free(uconfig_file);
        Read_Config_File(config_file);
    }
    else
    {
        home = getenv("HOME");
        config_file = malloc( strlen(home) + 9 );
        if (config_file == NULL)
            errH(1, 1, "malloc");

        sprintf(config_file, "%s/.wmtzrc", home);

        if (!Read_Config_File(config_file))
        {
            /* Fall back to /etc/wmtzrc */
            free(config_file);
            config_file = malloc( 12 );
            if (config_file == NULL)
                errH(1, 1, "malloc");

            sprintf(config_file, "/etc/wmtzrc");

            fprintf(stderr, "Using /etc/wmtzrc as config file.\n");

            Read_Config_File(config_file);
        }
    }

    RedrawWindow();
    prevtime = time(0) - 1;

    while (1)
    {
	waitpid(0, NULL, WNOHANG);

        /* If wmtz have been mouse-clicked, show menu */
        if ( clicked )
	{
            BlitString("MENU:    ", 5, (11*(0)) + 5);
            BlitString(" WMTZ    ", 5, (11*(1)) + 5);
            BlitString(" CONFIG  ", 5, (11*(2)) + 5);
            BlitString(" RESTART ", 5, (11*(3)) + 5);
            BlitString(" QUIT    ", 5, (11*(4)) + 5);
	}
        else if ( (curtime = time(0)) > prevtime)
        {
	  prevtime = curtime;

          /* Update the display */
          for (j=1; j<6; j++)
          {
            /* Display empty line */
            if (strncmp( zone[j].label, "xxx", 3) == 0 )
	    {
              BlitString("        ", 5, (11*(j-1)) + 5);
	    }

	    /* Display local day/mon/year */
            else if (strncmp( zone[j].label, "DATE", 4) == 0 )
	    {
	      clk = localtime(&curtime);
	      while(clk->tm_year>99) clk->tm_year-=100;
	      snprintf(blitstr, STRSIZE,"%s %02d.%02d",
                       month[clk->tm_mon],clk->tm_mday,clk->tm_year);
              BlitString(blitstr, 5, (11*(j-1)) + 5);
	    }

	    /* Display local weekday */
            else if (strncmp( zone[j].label, "WDAY", 4) == 0 )
	    {
	      clk = localtime(&curtime);
	      snprintf(blitstr, STRSIZE,"%s",week_day[clk->tm_wday]);
	      BlitString(blitstr, 4, (11*(j-1)) + 5);
	    }

	    /* Display more precise internet time */
            else if (strncmp( zone[j].label,"@", 1) == 0 )
            {
              /* Calculate Internet time */
              swatch_beats = (float)(((curtime+3600)%86400)/86.4);
              snprintf (blitstr, STRSIZE, "@:%7.3f", swatch_beats);
              BlitString (blitstr, 5, (11*(j-1)) + 5);
            }

            /* Display Julian Day Number */
            else if (strncmp( zone[j].label, "JDN", 3) == 0 )
	    {
 	      clk = gmtime(&curtime);
	      /* Calculate Julin Day Number */
	      jd = jdn(curtime) - zone[j].epoch;
	      snprintf(blitstr, STRSIZE, "%10f", jd );
	      BlitString(blitstr, 5, (11*(j-1)) + 5);
	    }

            /* Display Local Mean Sidereal Time */
            else if (strncmp( zone[j].label, "LMST", 3) == 0 )
	    {
              clk = gmtime(&curtime);
              jd = jdn(curtime);
              siderealTime( jd, sid, LMST );
              snprintf(blitstr, STRSIZE, "%s%02i.%02i","LST:", sid[0], sid[1]);
              BlitString(blitstr, 5, (11*(j-1)) + 5);
            }

            /* Display Greenwich Mean Sidereal Time */
            else if (strncmp( zone[j].label, "GMST", 3) == 0 )
	    {
              clk = gmtime(&curtime);
              jd = jdn(curtime);
              siderealTime( jd, sid, GMST );
              snprintf(blitstr, STRSIZE, "%s%02i.%02i","GST:", sid[0], sid[1]);
              BlitString(blitstr, 5, (11*(j-1)) + 5);
            }

            /* Display local time */
	    else if (strncmp( zone[j].label, "LOCAL", 5) == 0 )
	    {
	      clk = localtime(&curtime);
	      strncpy(buf, tzname[0], 3);

              for (k=0; k<3; k++)
                if (buf[k] == 0)
                  buf[k] = ' ';

              buf[3] = ':';
              buf[4] = 0;
	      hour = clk->tm_hour;

	      /* Print Label */
              snprintf(blitstr, STRSIZE, "%s%02i.%02i",buf,hour,clk->tm_min);
              BlitString(blitstr, 5, (11*(j-1)) + 5);
	    }

            /* Display time in specified time zone */
	    else if (strncmp( zone[j].label, "TZONE", 4) == 0 )
	    {
	      putenv(zone[j].tz);
	      tzset();
	      clk = localtime(&curtime);

	      strncpy(buf, tzname[0], 3);

              for (k=0; k<3; k++)
                if (buf[k] == 0)
                  buf[k] = ' ';

              buf[3] = ':';
              buf[4] = 0;

              snprintf(blitstr, STRSIZE, "%s%02i.%02i", buf,
		       clk->tm_hour, clk->tm_min);
              BlitString(blitstr, 5, (11*(j-1)) + 5);

	      /* Reset TZ environment variable to old value */
	      putenv(originalTZ);
	    }

            /* Display time in specified time zone without TZ env. var. */
            else
            {
 	      clk = gmtime(&curtime);
              strncpy(buf, zone[j].label, 3);

              for (k=0; k<3; k++)
                if (buf[k] == 0)
                  buf[k] = ' ';

              buf[3] = ':';
              buf[4] = 0;

              hour = clk->tm_hour - zone[j].diff;
              if (hour > 23 )
		hour -= 24;
              else if (hour < 0 )
		hour += 24;

	      /* Print Label */
              snprintf(blitstr, STRSIZE, "%s%02i.%02i",buf,hour,clk->tm_min);
              BlitString(blitstr, 5, (11*(j-1)) + 5);
	    }
          }
        }
        RedrawWindow();

        /* X Events */
        while (XPending(display))
        {
          XNextEvent(display, &Event);
          switch (Event.type)
          {
            case Expose:
                   RedrawWindow();
                   break;
            case DestroyNotify:
                   XCloseDisplay(display);
                   exit(0);
                   break;
            case ButtonPress:
                   i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
                   but_stat = i;
                   break;
            case ButtonRelease:
                   i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
                   if (but_stat == i && but_stat >= 0)
                   {
		     if ( clicked ) /* The menu is up */
		     {
		         handleTheMenu(but_stat);
		         clicked = 0;
		     }
                     else /* Show the menu instead of time */
		     {
			 clicked = 1;
		     }
                   }
		   but_stat = -1;
                   break;
	    default:
	  }
        }

        usleep(10000);
    }
}


/*****************************************************************************\
|* handleTheMenu                                                             *|
\*****************************************************************************/
void handleTheMenu(int but_stat)
{
    char                *editor;
    char                *ed;

    switch( but_stat )
      {
	case 0:
	    execCommand(ABOUT);
	    break;
	case 1:
	    break;
	case 2:
	    /* Figure out what editor to use */
	    if ( defedit == NULL )
		{
		    ed = getenv("XEDITOR");
		    if ( ed == NULL )
			ed = "xedit";
		}
	    else
		{
		    ed = defedit;
		}
	    editor = malloc( strlen(ed)+strlen(config_file)+2 );
	    if ( editor == NULL )
		errH(1, 1, "malloc");
	    sprintf(editor, "%s %s", ed, config_file);
	    waitpid( execCommand(editor), NULL, 0 );
	    Read_Config_File(config_file);
	    free(editor);
	    break;
	case 3:
	    Read_Config_File(config_file);
	    break;
	case 4:
	    exit(0);
	    break;
	default:
      }
    return;
}


/*****************************************************************************\
|* ReadConfigSetting                                                         *|
\*****************************************************************************/
int ReadConfigString(FILE *fp, char *setting, char *value)
{
    char str[1024];
    char buf[1024];
    int i;
    int len;
    int slen;
    char *p=NULL;

    if (!fp)
    {
        return 0;
    }

    snprintf(str,1024, "%s=", setting);
    slen = strlen(str);

    fseek(fp, 0, SEEK_SET);

    while ( !feof(fp) )
    {

        if (!fgets(buf, 512, fp))
	  break;

        len = strlen(buf);

        /* strip linefeed */
        for (i=0; i!=len; i++)
	  if (buf[i] == '\n')
	    buf[i] = 0;

        /* printf("Scanning '%s'...\n", buf); */
        if ( strncmp(buf, str, strlen(str)) == 0)
        {
            /* found our setting */

	  for(i=0; i!=slen; i++)
	    if ( buf[i] == '=' )
	      {
		p=buf+i+1;
		strcpy(value, p);
		return 1;
	      }
        }
    }
    return 0;
}


/*****************************************************************************\
|* ReadConfigInt                                                             *|
\*****************************************************************************/
int ReadConfigInt(FILE *fp, char *setting, int *value)
{
    char buf[1024];

    if (ReadConfigString(fp, setting, (char *) &buf))
    {
        *value = atoi(buf);
        return 1;
    }

    return 0;
}


/*****************************************************************************\
|* ReadConfigDouble                                                          *|
\*****************************************************************************/
int ReadConfigDouble(FILE *fp, char *setting, double *value)
{
    char buf[1024];

    if (ReadConfigString(fp, setting, (char *) &buf))
    {
        *value = atof(buf);
        return 1;
    }

    return 0;
}


/*****************************************************************************\
|* Read_Config_File                                                          *|
\*****************************************************************************/
int Read_Config_File( char *filename )
{
    FILE *fp;
    char temp[253];

    fp = fopen(filename, "r");
    if (fp)
    {
        ReadConfigString(fp, "time1", zone[1].label);
        ReadConfigString(fp, "time2", zone[2].label);
        ReadConfigString(fp, "time3", zone[3].label);
        ReadConfigString(fp, "time4", zone[4].label);
        ReadConfigString(fp, "time5", zone[5].label);
        ReadConfigInt(fp, "utdiff1", &zone[1].diff);
        ReadConfigInt(fp, "utdiff2", &zone[2].diff);
        ReadConfigInt(fp, "utdiff3", &zone[3].diff);
        ReadConfigInt(fp, "utdiff4", &zone[4].diff);
        ReadConfigInt(fp, "utdiff5", &zone[5].diff);
        ReadConfigDouble(fp, "utdiff1", &zone[1].epoch);
        ReadConfigDouble(fp, "utdiff2", &zone[2].epoch);
        ReadConfigDouble(fp, "utdiff3", &zone[3].epoch);
        ReadConfigDouble(fp, "utdiff4", &zone[4].epoch);
        ReadConfigDouble(fp, "utdiff5", &zone[5].epoch);

        ReadConfigString(fp, "utdiff1", temp);
	sprintf(zone[1].tz, "TZ=%s", temp);
        ReadConfigString(fp, "utdiff2", temp);
	sprintf(zone[2].tz, "TZ=%s", temp);
        ReadConfigString(fp, "utdiff3", temp);
	sprintf(zone[3].tz, "TZ=%s", temp);
        ReadConfigString(fp, "utdiff4", temp);
	sprintf(zone[4].tz, "TZ=%s", temp);
        ReadConfigString(fp, "utdiff5", temp);
	sprintf(zone[5].tz, "TZ=%s", temp);

        ReadConfigDouble(fp, "longitude", &longitude);
        /* ReadConfigDouble(fp, "latitude", &latitude); */

        fclose(fp);
        return 1;
    }
    else
    {
        errH(1, 0, "Unable to open %s", filename);
        return 0;
    }
}


/*****************************************************************************\
|* BlitString - Blits a string at given coordinates.                         *|
\*****************************************************************************/
void BlitString(char *name, int x, int y)
{
    int		i;
    int		c;
    int		k;

    k = x;
    for (i=0; name[i]; i++)
    {

        c = toupper(name[i]);
	if (c >= 'A' && c <= 'Z')
	{   /* its a letter */
	  c -= 'A';
	  copyXPMArea(c * CHAR_WIDTH, 74, CHAR_WIDTH, 8, k, y);
	  k += CHAR_WIDTH;
        }
        else if ( c >= '0' && c <= ':')
        {
	  c -= '0';
	  copyXPMArea(c * CHAR_WIDTH, 64, CHAR_WIDTH, 8, k, y);
	  k += CHAR_WIDTH;
	}
	else if (c == ';') /* used as a slim ':' */
	{
	  copyXPMArea(60, 64, CHAR_WIDTH, 8, k, y);
	  k += 4;
	}
	else if (c=='.')
	  {
	  copyXPMArea(115, 64, 4, 8, k, y);
	  k += 4;
	}
	else if (c=='@')
	{
	  copyXPMArea(108, 64, CHAR_WIDTH, 8, k, y);
	  k += CHAR_WIDTH;
	}
	else /* print a ' ' */
	{
	  copyXPMArea(120, 64, CHAR_WIDTH, 8, k, y);
	  k += CHAR_WIDTH;
	}
    }
}


/*****************************************************************************\
|* usage                                                                     *|
\*****************************************************************************/
void usage(void)
{
  fprintf(stderr, "\nwmtz - shows local time around the world and more.\n");
  fprintf(stderr, "See ~/.wmtzrc or /etc/wmtzrc for configuration.\n\n");
  fprintf(stderr, "Usage:\n");
  fprintf(stderr, "    -display <display name>\n");
  fprintf(stderr, "    -e <editor>               use specified editor\n");
  fprintf(stderr, "    -geometry +XPOS+YPOS      initial window position\n");
  fprintf(stderr, "    -jd                       Julian<->Date conversion\n");
  fprintf(stderr, "    -c <filename>             use specified config file\n");
  fprintf(stderr, "    -m                        start monochrome version\n");
  fprintf(stderr, "    -h                        this help screen\n");
  fprintf(stderr, "    -v                        print the version number\n");
  fprintf(stderr, "\n");
}


/*****************************************************************************\
|* printversion								     *|
\*****************************************************************************/
void printversion(void)
{
    fprintf(stderr, "wmtz v%s\n", WMTZ_VERSION);
}


/*****************************************************************************\
|* range - Put val in 0<->ran interval.                                      *|
\*****************************************************************************/
void range (double *val, double ran)
{
      *val -= ran*floor(*val/ran);

      if (*val < 0)
          *val += ran;
}


/*****************************************************************************\
|* jdn - converts a time_t to Julian Day                                     *|
\*****************************************************************************/
double jdn(time_t curtime)
{
    return (curtime/86400.0 + 2440587.5);
}

/*****************************************************************************\
|* siderealTime - Gives sidereal time from JD.                               *|
\*****************************************************************************/
void siderealTime( double jde, int *result, int mode )
{
   double t, t2, t3, ts;

   t = (jde - 2451545.0)/36525.0;
   t2 = t*t;
   t3 = t2*t;

   /* Expression from "Astronomical Algorithms" by J. Meeus */
   ts = 280.46061837 + 360.98564736629 * ( jde - 2451545.0 )
        + 0.000387933 * t2 - t3/38710000.0;

   range( &ts, 360.0 );
   ts /= 15.0;

   /* If local time add one hour for every 15 degree in longitude */
   if ( mode == LMST )
   {
     ts += longitude/15.0;
   }

   range( &ts, 24.0 );
   result[0] = (int)ts;
   result[1] = (int)(60 *(ts - result[0]));
}


/*****************************************************************************\
|* julianDay - Gives JD from date.                                           *|
\*****************************************************************************/
double julianDay( int year, int month, double day, double hour,
                  double minute, double second, int julian )
{
   int a, b, c, d;
   double jd;

   day = day + hour/24.0 + minute/1440.0 + second/86400.0;

   if ( month < 3 )
   {
      year -= 1;
      month += 12;
   }

   /* If the date is a Julian calendar date, set julian to TRUE */
   if ( julian )
   {
      b = 0;
   }
   else /* If Gregorian calendar date, julian should be FALSE */
   {
      a = year/100;
      b = 2 - a + a/4;
   }
   c = 365.25 * (year + 4716);
   d = 30.6001 * (month + 1);
   jd = c + d + day + b - 1524.5;

   return( jd );
}


/*****************************************************************************\
|* calendarDate - Gives date from JD. Only Gregorian calendar dates.         *|
\*****************************************************************************/
int calendarDate( double jd, int *year, int *month, double *day )
{
   double a, b, frac, ij, alfa, beta, c, d, e, f;

   if ( jd < 0 )
      return 0;

   jd += 0.5;
   ij = floor(jd);
   frac = jd - ij;

   if ( ij < 2299161 )
   {
      a = ij;
   }
   else
   {
      alfa = floor((ij - 1867216.25)/36524.25);
      beta = floor(alfa/4);
      a = ij + 1 + alfa - beta;
   }

   b = a + 1524;
   c = floor((b - 122.1)/365.25);
   d = floor(365.25 * c);
   e = floor((b - d)/30.6001);
   f = floor(30.6001 * e);

   *day = b - d - f + frac;

   if (e < 14)
       *month = e - 1;
   else if (e == 14 || e == 15)
       *month = e - 13;
   else
       return 0;

   if (*month > 2)
       *year = c - 4716;
   else if (*month == 1 || *month == 2)
       *year = c - 4715;
   else
       return 0;

   return 1;
}


/*****************************************************************************\
|* handleJD                                                                  *|
\*****************************************************************************/
int handleJD( void )
{
  int conv, y, m, d, h, min, sec;
  double day, jd;

  printf(" 1 : Date to JD.\n 2 : JD to date.\n");
  printf("Choose conversion (1 or 2): ");
  scanf("%d", &conv);

  if (conv == 1 )
  {
     printf("Enter UT date with time (YYYY,MM,DD,hh:mm:ss): ");
     scanf("%d,%d,%d,%d:%d:%d", &y, &m, &d, &h, &min, &sec);
     printf("\nJulian Day: %f\n", julianDay( y, m, d, h, min, sec, 0 ) );
  }
  else if (conv == 2)
  {
     printf("Enter Julian Day Number: ");
     scanf("%lf", &jd );
     if ( !calendarDate( jd, &y, &m, &day ) )
     {
       printf("Conversion error! Negative JD not allowed.\n");
       return 0;
     }

     printf("\nGregorian date: %d-%2.2d-%2.4f\n", y, m, day);
  }
  else
  {
    printf("Invalid choice! Try again, please...\n");
    handleJD();
  }
  return 1;
}

/*****************************************************************************\
|* errH                                                                      *|
\*****************************************************************************/
void errH(int printErrno, int exitCode, const char *msg, ...)
{
    int error = errno;
    va_list arg;
    char buf[2048];

    /* Put the name of the program first */
    buf[0] = 0;
    strcat(buf, pname);
    strcat(buf, ": ");

    va_start(arg, msg);
    vsprintf(buf+strlen(buf), msg, arg);
    if (printErrno)
        sprintf(buf+strlen(buf), ": %s", strerror(error));
    strcat(buf, "\n");
    fflush(stdout);
    fputs(buf, stderr);
    fflush(NULL);
    va_end(arg);

    if ( exitCode )
         exit(exitCode);

    return;
}
