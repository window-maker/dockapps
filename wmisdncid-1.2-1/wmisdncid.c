
/***********************************************************************\

wmisdncid - WindowMaker isdn monitor
Jul 09th 2000  Release 1.2-1
Copyright (C) 1999-2000  Carl Eike Hofmeister <wmisdncid@carl-eike-hofmeister.de>
			 and may others (see section CREDITS in README)
This software comes with ABSOLUTELY NO WARRANTY
This software is free software, and you are welcome to redistribute it
under certain conditions
See the COPYING file for details.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

\***********************************************************************/

#define WINDOWMAKER 0
#define USESHAPE 0
#define NAME "wmisdncid"
#define CLASS "WMisdnCID"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <term.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <linux/isdn.h>


Pixmap back;
Pixmap tile;
Pixmap icons;
Pixmap disp;
Pixmap mask;

#define BACKCOLOR "#282828"
#define LEDCOLOR "LightSeaGreen"

#include "wmisdncid.xpm"
#include "wmisdncidicons.xpm"
#include "tile.xpm"

/* ISDN Data */

#define ISDNINFO "/dev/isdninfo"

char *isdninfo = ISDNINFO;

FILE *isdninfofile;

struct isdndatastruct
{
  char phone[32];
  char phone_last[32];
  int usage;
  int usage_last;
  int changed;
};

int showlast = 0;

struct isdndatastruct isdndata[2];

/* Defaults. Altered by command line options. */

int wmaker = WINDOWMAKER;
int ushape = USESHAPE;
char txtdpy[256] = "";
char bckcol[256] = BACKCOLOR;
char ledcol[256] = LEDCOLOR;

Atom _XA_GNUSTEP_WM_FUNC;
Atom deleteWin;

Display *dpy;
Window Win[2];
Window Root;
GC WinGC;
int activeWin;

// XWMHints *hints;

/* Standard dock-app stuff */

void initXWin (int argc, char *argv[]);
void freeXWin ();
void createWin (Window * win);
unsigned long getColor (char *colorName, float dim);

/* Custom dock-app stuff */

void scanArgs (int argc, char *argv[]);
int checkISDNopen ();
void checkISDNclose ();
void checkISDN (int forced);
void pressEvent (XButtonEvent * xev);
void releaseEvent (XButtonEvent * xev);
void repaint ();
void update (int forced);

void newSystem (char *command);

/***********************************************************************/

#define MAXISDNDF 3

int isdndf[MAXISDNDF] =
{-1, -1, -1};
int isdndfs = 0;

int
isdnttyI_open (char *ttyIname, char *msn)
{
  if (isdndfs >= MAXISDNDF)
  {
    printf ("too many -t options used\n");
    return (1);
  }

  isdndf[isdndfs] = open (ttyIname, O_RDWR | O_NDELAY, 0);

  if (isdndf[isdndfs] >= 0)
  {
    write (isdndf[isdndfs], "AT\rAT&e", 7);
    write (isdndf[isdndfs], msn, strlen (msn));
    write (isdndf[isdndfs], "\rATS18=7\r", 9);
    isdndfs++;
  }
  else
    return (1);

  return (0);
}

void
isdnttyI_read ()
{
  int i;

  for (i = 0; i < isdndfs; i++)
    if (isdndf[i] >= 0)
    {
      int numchar;

      ioctl (isdndf[i], FIONREAD, &numchar);
      if (numchar > 0)
        tcflush (isdndf[i], TCIFLUSH);
    }
}

void
isdnttyI_close ()
{
  int i;

  for (i = 0; i < isdndfs; i++)
    if (isdndf[i] >= 0)
      close (isdndf[i]);
}

/***********************************************************************/

int
main (int argc, char *argv[])
{
  scanArgs (argc, argv);

  if (checkISDNopen ())
    return (1);

  initXWin (argc, argv);

  {
    XGCValues gcv;
    unsigned long gcm;

    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.graphics_exposures = 0;
    WinGC = XCreateGC (dpy, Root, gcm, &gcv);
  }

  {
    XpmAttributes pixatt;
    XpmColorSymbol ledcols[4] =
    {
      {"led_color_high", NULL, 0},
      {"led_color_med", NULL, 0},
      {"led_color_low", NULL, 0},
      {"back_color", NULL, 0}
    };

    ledcols[0].pixel = getColor (ledcol, 1.00);
    ledcols[1].pixel = getColor (ledcol, 1.75);
    ledcols[2].pixel = getColor (ledcol, 2.30);
    ledcols[3].pixel = getColor (bckcol, 1.00);
    pixatt.numsymbols = 4;
    pixatt.colorsymbols = ledcols;
    pixatt.exactColors = 0;
    pixatt.closeness = 40000;
    pixatt.valuemask = XpmColorSymbols | XpmExactColors | XpmCloseness;
    XpmCreatePixmapFromData (dpy, Root, wmisdncid_xpm, &back, &mask, &pixatt);
    XpmCreatePixmapFromData (dpy, Root, wmisdncidicons_xpm, &icons, NULL, &pixatt);
    XpmCreatePixmapFromData (dpy, Root, tile_xpm, &tile, NULL, &pixatt);
    disp = XCreatePixmap (dpy, Root, 64, 64, DefaultDepth (dpy, DefaultScreen (dpy)));

    /* Install mask or copy background tile */
    if (wmaker || ushape)
      XShapeCombineMask (dpy, Win[activeWin], ShapeBounding, 0, 0, mask, ShapeSet);
    else
      XCopyArea (dpy, tile, disp, WinGC, 0, 0, 64, 64, 0, 0);

    /* Copy background */
    XSetClipMask (dpy, WinGC, mask);
    XCopyArea (dpy, back, disp, WinGC, 0, 0, 64, 64, 0, 0);
    XSetClipMask (dpy, WinGC, None);
  }

  {
    int finished = 0;
    XEvent event;

    XSelectInput (dpy, Win[activeWin], ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow (dpy, Win[0]);

    checkISDN (1);

    while (!finished)
    {
      while (XPending (dpy))
      {
        XNextEvent (dpy, &event);
        switch (event.type)
        {
        case Expose:
          repaint ();
          break;
        case ButtonPress:
          pressEvent (&event.xbutton);
          break;
        case ButtonRelease:
          releaseEvent (&event.xbutton);
          break;
        case ClientMessage:
          if (event.xclient.data.l[0] == deleteWin)
            finished = 1;
          break;
        }
      }

      isdnttyI_read ();
      checkISDN (0);
      XFlush (dpy);

      /* usleep (50000L); *//* select sleeps */
    }
  }

  XFreeGC (dpy, WinGC);
  XFreePixmap (dpy, disp);
  XFreePixmap (dpy, mask);
  XFreePixmap (dpy, icons);
  XFreePixmap (dpy, tile);
  XFreePixmap (dpy, back);
  freeXWin ();
  checkISDNclose ();
  isdnttyI_close ();
  return (0);
}

void
initXWin (int argc, char *argv[])
{
  if ((dpy = XOpenDisplay (txtdpy)) == NULL)
  {
    fprintf (stderr, "wmisdncid: could not open display %s\n", txtdpy);
    exit (1);
  }
  _XA_GNUSTEP_WM_FUNC = XInternAtom (dpy, "_GNUSTEP_WM_FUNCTION", 0);
  deleteWin = XInternAtom (dpy, "WM_DELETE_WINDOW", 0);
  Root = DefaultRootWindow (dpy);
  createWin (&Win[0]);
  createWin (&Win[1]);

  {
    XWMHints hints;
    XSizeHints shints;

    hints.window_group = Win[0];
    shints.min_width = 64;
    shints.min_height = 64;
    shints.max_width = 64;
    shints.max_height = 64;
    shints.x = 0;
    shints.y = 0;
    if (wmaker)
    {
      hints.initial_state = WithdrawnState;
      hints.icon_window = Win[1];
      hints.flags = WindowGroupHint | StateHint | IconWindowHint;
      shints.flags = PMinSize | PMaxSize | PPosition;
      activeWin = 1;
    }
    else
    {
      hints.initial_state = NormalState;
      hints.flags = WindowGroupHint | StateHint;
      shints.flags = PMinSize | PMaxSize;
      activeWin = 0;
    }
    XSetWMHints (dpy, Win[0], &hints);
    XSetWMNormalHints (dpy, Win[0], &shints);
  }

  XSetCommand (dpy, Win[0], argv, argc);
  XStoreName (dpy, Win[0], NAME);
  XSetIconName (dpy, Win[0], NAME);
  XSetWMProtocols (dpy, Win[activeWin], &deleteWin, 1);
}


void
freeXWin ()
{
  XDestroyWindow (dpy, Win[0]);
  XDestroyWindow (dpy, Win[1]);
  XCloseDisplay (dpy);
}

void
createWin (Window * win)
{
  XClassHint classHint;

  *win = XCreateSimpleWindow (dpy, Root, 10, 10, 64, 64, 0, 0, 0);
  classHint.res_name = NAME;
  classHint.res_class = CLASS;
  XSetClassHint (dpy, *win, &classHint);
}

unsigned long
getColor (char *colorName, float dim)
{
  XColor Color;
  XWindowAttributes Attributes;

  XGetWindowAttributes (dpy, Root, &Attributes);
  Color.pixel = 0;

  XParseColor (dpy, Attributes.colormap, colorName, &Color);
  Color.red = (unsigned short) (Color.red / dim);
  Color.blue = (unsigned short) (Color.blue / dim);
  Color.green = (unsigned short) (Color.green / dim);
  Color.flags = DoRed | DoGreen | DoBlue;
  XAllocColor (dpy, Attributes.colormap, &Color);

  return Color.pixel;
}

void
scanArgs (int argc, char *argv[])
{
  int i;

  for (i = 1; i < argc; i++)
  {
    if (strcmp (argv[i], "-w") == 0)
      wmaker = 1;
    else if (strcmp (argv[i], "-s") == 0)
      ushape = 1;
    else if (strcmp (argv[i], "-b") == 0)
    {
      if (i < argc - 1)
      {
        i++;
        sprintf (bckcol, "%s", argv[i]);
      }
    }
    else if (strcmp (argv[i], "-l") == 0)
    {
      if (i < argc - 1)
      {
        i++;
        sprintf (ledcol, "%s", argv[i]);
      }
    }
    else if (strcmp (argv[i], "-t") == 0)
    {
      if (i < argc - 2)
      {
        i += 2;
        if (isdnttyI_open (argv[i - 1], argv[i]))
          printf ("failed opening %s\n", argv[i - 1]);
      }
      else
        printf ("-t requires /dev/ttyIx and phone number\n");
    }
    else if (strcmp (argv[i], "-display") == 0)
    {
      if (i < argc - 1)
      {
        i++;
        sprintf (txtdpy, "%s", argv[i]);
      }
    }
    else if ((!strcmp (argv[i], "-help")) ||
             (!strcmp (argv[i], "--help")) ||
             (!strcmp (argv[i], "-h")) ||
             (!strcmp (argv[i], "-?")))
    {
      fprintf (stderr, "wmisdncid - the WindowMaker isdn monitor\nJul 09th 2000  Release 1.2-1\n");
      fprintf (stderr, "Copyright (C) 1999-2000 Carl Eike Hofmeister <wmisdncid@carl-eike-hofmeister.de>\n");
      fprintf (stderr, "                        and many others (see section CREDITS in README\n");
      fprintf (stderr, "This software comes with ABSOLUTELY NO WARRANTY\n");
      fprintf (stderr, "This software is free software, and you are welcome to redistribute it\n");
      fprintf (stderr, "under certain conditions\n");
      fprintf (stderr, "See the README file for a more complete notice.\n\n");
      fprintf (stderr, "usage:\n\n   %s [options]\n\noptions:\n\n", argv[0]);
      fprintf (stderr, "   -h | -help | --help    display this help screen\n");
      fprintf (stderr, "   -ttyI <ttyI><MSN>      set ttyI to MSN and wait for any incoming call (makes\n");
      fprintf (stderr, "                          readttyIforever obsolete) may be used 3 times\n");
      fprintf (stderr, "   -w                     use WithdrawnState    (for WindowMaker)\n");
      fprintf (stderr, "   -s                     shaped window\n");
      fprintf (stderr, "   -b back_color          use the specified color for background of display\n");
      fprintf (stderr, "   -l led_color           use the specified color for display\n");
      fprintf (stderr, "   -display display       select target display (see X manual pages)\n\n");
      exit (0);
    }

  }
}

void
pressEvent (XButtonEvent * xev)
{
  showlast = !showlast;
  update (1);

/*
 * int x = xev->x;
 * int y = xev->y;
 * if (x >= 33 && y >= 47 && x <= 45 && y <= 57)
 * {
 * 
 * }
 *
 */
}

void
releaseEvent (XButtonEvent * xev)
{
//  btnstate &= ~(btnPrev | btnNext);
  //  drawBtns (btnPrev | btnNext);
  //  repaint ();
}

void
repaint ()
{
  XEvent xev;

  XCopyArea (dpy, disp, Win[activeWin], WinGC, 0, 0, 64, 64, 0, 0);
  while (XCheckTypedEvent (dpy, Expose, &xev)) ;
}

void
update_num (char *num, int x, int y)
{
  char s[16];                   /* 6 + 9 + terminating zero */
  int i;

  if (strlen (num) > 15)
    num[15] = 0;
  sprintf (s, "%15s", num);

  for (i = 0; i < 6; i++)
    if ((s[i] >= '0') && (s[i] <= '9'))
      XCopyArea (dpy, icons, disp, WinGC, (s[i] - ('0' - 1)) * 5, 0, 5, 7, 18 + x + (6 * i), y);
    else
      XCopyArea (dpy, icons, disp, WinGC, 0, 0, 5, 7, 18 + x + (6 * i), y);
  for (i = 6; i < 15; i++)
    if ((s[i] >= '0') && (s[i] <= '9'))
      XCopyArea (dpy, icons, disp, WinGC, (s[i] - ('0' - 1)) * 5, 0, 5, 7, x + (6 * (i - 6)), y + 14);
    else
      XCopyArea (dpy, icons, disp, WinGC, 0, 0, 5, 7, x + (6 * (i - 6)), y + 14);
}

void
update_state (char state, int x, int y)
{
  if ((state < 0) || (state > 3))
    state = 0;
  XCopyArea (dpy, icons, disp, WinGC, state * 8, 7, 7, 7, x, y);
}

void
update_usage (int usage, int x, int y)
{
  switch (usage)
  {
  case ISDN_USAGE_RAW:
    usage = 1;
    break;
  case ISDN_USAGE_MODEM:
    usage = 2;
    break;
  case ISDN_USAGE_NET:
    usage = 3;
    break;
  case ISDN_USAGE_VOICE:
    usage = 4;
    break;
  case ISDN_USAGE_FAX:
    usage = 5;
    break;
  default:
    usage = 0;
  }
  XCopyArea (dpy, icons, disp, WinGC, usage * 5, 2 * 7, 5, 7, x, y);
}

void
update (int forced)
{
  int somethingchanged = 0;

  if (((isdndata[0].changed) ||
       (isdndata[1].changed)) && (showlast))
  {
    forced = 1;
    showlast = 0;
  }

  if (showlast)
  {
    update_num (isdndata[0].phone_last, 5, 6);
    update_num (isdndata[1].phone_last, 5, 35);
    update_state (3, 5, 6);
    update_state (3, 5, 35);
    update_usage (isdndata[0].usage_last & 7, 5 + 8, 6);
    update_usage (isdndata[1].usage_last & 7, 5 + 8, 35);
    somethingchanged = 1;
  }
  else
  {
    if (forced || isdndata[0].changed)
    {
      update_num (isdndata[0].phone, 5, 6);
      update_state ((1 << (!!(isdndata[0].usage & ISDN_USAGE_OUTGOING))) & (3 * (!!(isdndata[0].usage & 7))), 5, 6);
      update_usage (isdndata[0].usage & 7, 5 + 8, 6);
      isdndata[0].changed = 0;
      somethingchanged = 1;
    }
    if (forced || isdndata[1].changed)
    {
      update_num (isdndata[1].phone, 5, 35);
      update_state ((1 << (!!(isdndata[1].usage & ISDN_USAGE_OUTGOING))) & (3 * (!!(isdndata[1].usage & 7))), 5, 35);
      update_usage (isdndata[1].usage & 7, 5 + 8, 35);
      isdndata[1].changed = 0;
      somethingchanged = 1;
    }
  }
  if (somethingchanged)
    repaint ();
}

/* ISDNINFO - Reading */

int
checkISDNopen ()
{
  memset (isdndata, 0, sizeof (isdndata));
  if (isdninfofile == NULL)
  {
    if ((isdninfofile = fopen (isdninfo, "rt")) == NULL)
    {
      printf ("wmisdncid: error: could not open %s\n", isdninfo);
      return (1);
    }
  }
  return (0);
}

void
checkISDNclose ()
{
  if (isdninfofile != NULL)
    fclose (isdninfofile);
  isdninfofile = NULL;
}

void
checkISDN (int forced)
{
  char buf[6][4096], prefix[2][64], data[2][64];
  int i, j, dataint[2];

  if (isdninfofile != NULL)
  {

    if (!forced)
    {
      fd_set fdset;
      struct timeval timeout;

      FD_ZERO (&fdset);
      FD_SET (fileno (isdninfofile), &fdset);
      timeout.tv_sec = 0;
      timeout.tv_usec = 50000L;
      if (select (fileno (isdninfofile) + 1, &fdset, NULL, NULL, &timeout) <= 0)
        return;
    }

    for (i = 0; i < 6; i++)
      fgets (buf[i], sizeof (buf[i]) - 1, isdninfofile);

    sscanf (buf[3], "%s %i %i", &(prefix[0][0]), &dataint[0], &dataint[1]);
    sscanf (buf[5], "%s %s %s", &(prefix[1][0]), &(data[0][0]), &(data[1][0]));

    if ((!strcmp (prefix[0], "usage:")) &&
        (!strcmp (prefix[1], "phone:")))
    {
      for (j = 0; j < 2; j++)
      {
        if (strcmp (isdndata[j].phone, data[j]) || (isdndata[j].usage != dataint[j]) || forced)
        {
          if ((!(isdndata[j].usage & ISDN_USAGE_OUTGOING)) && (!dataint[j]))
          {
            strncpy (isdndata[1].phone_last, isdndata[0].phone_last, sizeof (isdndata[1].phone_last) - 1);
            isdndata[1].usage_last = isdndata[0].usage_last;
            strncpy (isdndata[0].phone_last, isdndata[j].phone, sizeof (isdndata[j].phone_last) - 1);
            isdndata[0].usage_last = isdndata[j].usage;
          }

          strncpy (isdndata[j].phone, data[j], sizeof (isdndata[j].phone) - 1);
          isdndata[j].usage = dataint[j];
          isdndata[j].changed = 1;
        }
      }
    }
    else
    {
      printf ("wmisdncid: error reading %s\n", ISDNINFO);
    }
    update (forced);
  }
}

void
newSystem (char *command)
{
  int pid = fork ();
  int status;

  if (pid == 0)
  {
    char *argv[4] =
    {
      "sh", "-c", command, 0
    };

    execv ("/bin/sh", argv);
    exit (127);
  }
  do
  {
    if (waitpid (pid, &status, 0) != -1)
      return;
  }
  while (1);
}
