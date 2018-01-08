/*   wmisdn - an ISDN monitor applet for windowmaker/afterstep
 *   Copyright (c) 2000-2001 Tasho Statev Kaletha
 *   tasho.kaletha@gmx.de
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 */


/* these defaults can be changed by command line options. */
#define WINDOWMAKER false
#define USESHAPE false
#define NAME "wmisdn"
#define CLASS "WMIsdn"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <syslog.h>
#include <asm/errno.h> /* for ENOTCONN */
#include <errno.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include <linux/isdn.h>
#include <linux/isdnif.h>
#include <time.h>

#include "regions.h"
#include "optics/optics.h"

static Pixmap coverPixmap;
static Pixmap unknownPixmap;
static Pixmap dialingPixmap;
static Pixmap offPixmap;
static Pixmap disabledPixmap;
static Pixmap statusPixmaps[6];
static Pixmap incomingPixmap, outgoingPixmap, bundlingPixmap, bundledPixmap, slavePixmap;


static Pixmap ledsPixmap;		/* a row of led symbols as shown by the var led_text below */
static Pixmap lampsPixmap;	/* a row of lamp images - green off, green on, yellow off, yellow on, red off, red on */
static Pixmap infoSWPixmap;	/* a row of arrows - left off, left on, right off, right on */
static Pixmap lampsSWPixmap;	/* a row of arrows - up off, up on, down off, down on */

#include "xpm/unknown.xpm"
#include "xpm/disabled.xpm"
#include "xpm/off.xpm"
#include "xpm/offline.xpm"
#include "xpm/dialing.xpm"
#include "xpm/incoming.xpm"
#include "xpm/outgoing.xpm"
#include "xpm/bundling.xpm"
#include "xpm/bundled.xpm"
#include "xpm/slave.xpm"
#include "xpm/raw.xpm"
#include "xpm/modem.xpm"
#include "xpm/online.xpm"
#include "xpm/voice.xpm"
#include "xpm/fax.xpm"

#include "xpm/cover.xpm"
#include "xpm/leds.xpm"
#include "xpm/lamps.xpm"
#include "xpm/lamps_sw.xpm"
#include "xpm/info_sw.xpm"

/* Runtime pixmaps */
static Pixmap disp_info;	/* double buffer for the info panel */
static Pixmap disp;			/* for the main window */
static Pixmap dmsk;			/* clip mask for the main window */

/* For command line arguments */
#define MAX_ARG_LEN		256
static bool wmaker = WINDOWMAKER;
static bool ushape = USESHAPE;
static char txtdpy[MAX_ARG_LEN] = "";
static char txtfont[MAX_ARG_LEN] = "";
static int  dialmode = ISDN_NET_DM_AUTO;
static char devices[ISDN_MAX_CHANNELS][MAX_ARG_LEN];
static int selected_device=-1;		/* selected device, points to an element of devices[] */
static char scriptpath[MAX_ARG_LEN] = "/etc/isdn";
static int scriptmode = 0;
static bool usescripts = false;
static int maxscriptmode = 0;
static char** scriptmodestrings = NULL;
static char *slave_pending = NULL;

/* atoms for deleting window */
static Atom _XA_GNUSTEP_WM_FUNC;
static Atom WM_DELETE_WINDOW;

/* global variables */
Display *dpy;
Window Win[3];				/* 0 - main win, 1 - icon win (for wmaker), 2 - info panel */
Window Root;
GC WinGC;
int activeWin;
XFontStruct *textFont=NULL;

static char led_text[] = "0123456789?/\\!@#$%^&*()_+-=\"~<>[]{}:. abcdefghijklmnopqrstuvwxyz";


static int rootUID, rootGID;
static bool infoPanelActive = false;
static bool lampsActive = false;

/* Der scriptmode wird als 2. Argument den Start-/Stopskripten uebergeben.
   Aenderung mit mittlerer Maustaste auf InfoSw bei offenem Infopanel. */

#define ACTIVE			1	/* values are not only symbolc, but important for calculations! */
#define INACTIVE		0
#define ID_LAMP_GREEN	0
#define ID_LAMP_YELLOW	1
#define ID_LAMP_RED		2

#define ID_SWITCH_INFO	10
#define ID_SWITCH_LAMPS	11
#define ID_SWITCH_STATUS 19
#define ID_DEVICE		20

#define UPDATE_INTERVAL 20  /* how many 50 milisec intervalls to wait between updates */
#define STATUS_WARNING_SAT 5 /* how many times to display a warning upon failing to retrieve device stats */

#define INCOMING		0
#define OUTGOING		1
#define STAT_DISABLED	1000
#define STAT_OFF		1001
#define STAT_DIALING	1002
#define STAT_UNKNOWN	2001
#define STAT_UNINITIALIZED 2002

#define SCRIPT_UP		"wmisdn-up"
#define SCRIPT_DOWN		"wmisdn-down"
#define SCRIPT_MODES	"wmisdn-scriptmodes"

typedef enum mpppModeType { none, master, slave };

struct isdnStatus
{
	int usage;
	int direction;
	bool bundled;
	char peerPhone[100];
	mpppModeType  mpppMode;
	char mpppPartner[100];
} curStatus = { STAT_UNINITIALIZED, INCOMING, false, "", none, "" };



/* text i/o routines */
bool scanArgs(int argc, char *argv[]);
void printUsage( char *prog_name );
void printHeader();
void parseDeviceNames( char *name_list );
void readScriptModes();


/* init/done routines */
void initXWin(int argc, char *argv[]);
void freeXWin();
void createMainWin( Window *win );
void createInfoPanel( Window *win );
void loadPixmaps();
void freePixmaps();
void createRegions();


/* window routines */
void alignInfoPanel();
void getWindowPosition( Window win, int *x, int *y );
void getWindowDimension( Window win, int *w, int *h );

/* graphic routines */
unsigned long getColor(const char *colorname);
void createPixmap(char *data[], Pixmap *image, Pixmap *mask );
void loadLeds(char *data[], Pixmap *image, const char *leds_color, const char *back_color );
void drawText( char *text, Pixmap dst, int x, int y, const char *color=InfoTextColor );
void drawLamp( int index, int active );
void drawDevice( int active = INACTIVE );
unsigned long mixColor( const char *colorname1, int prop1, const char *colorname2, int prop2);

/* interaction routines */
bool timeToUpdate();
void update();
void fullRepaint();
void repaint( Window win, int x, int y, int w, int h );
void setStatusPixmap();
void updateInfoPanel();
void blankMainWin( int x=0, int y=0, int w=MainWinDim.w, int h=MainWinDim.h );
void pressLamp( int lamp, int button );
void pressStatusSw();
void activateLamps( bool active );
void drawSwitches();

/* region event handlers */
void mouseInLamp( int id );
void mouseOutLamp( int id );
void mouseClickLamp( int id, unsigned int button );
void mouseInInfoSw( int id );
void mouseOutInfoSw( int id );
void mouseClickInfoSw( int id, unsigned int button );
void mouseInLampsSw( int id );
void mouseOutLampsSw( int id );
void mouseClickLampsSw( int id, unsigned int button );
void mouseInDevice( int id );
void mouseOutDevice( int id );
void mouseClickDevice( int id, unsigned int button );
void mouseInStatusSw( int id );
void mouseOutStatusSw( int id );
void mouseClickStatusSw( int id, unsigned int button );


void selectNextDevice();

/* event handlers */
void exposeEvent( XExposeEvent *xev);
void pressEvent(XButtonEvent *xev);
void motionEvent( XMotionEvent *xev );

/* ISDN routines */
void getStatus( char *device, isdnStatus *stat );
void getLocalIP( int *a, int *b, int *c, int *d );
inline void getRemoteIP( int *a, int *b, int *c, int *d );
inline void translateIP( struct sockaddr *addr, int *a, int *b, int *c, int *d );
int isdn_ioctl( int func, void *arg, const char *errmsg, const char *filename="/dev/isdnctrl" );
void isdnInitDefaultDialmode();
int getIpppNum( char *name );

inline void set_slave_pending();
inline void clear_slave_pending();
inline bool is_slave_pending();
inline void manage_slave();

/* -------- Implementation ----------- */

int main(int argc, char *argv[])
{

	rootUID = geteuid(); rootGID = getegid();
	seteuid(getuid()); setegid(getgid());

	printHeader();
	if( !scanArgs(argc, argv) )
	{
		printUsage( argv[0] );
		return 1;
	}
	readScriptModes();

	initXWin(argc, argv);

	loadPixmaps();
	createRegions();
	disp = XCreatePixmap(dpy, Root, MainWinDim.w, MainWinDim.h, DefaultDepth(dpy,DefaultScreen(dpy)));
	disp_info = XCreatePixmap(dpy, Root, InfoWinDim.w, InfoWinDim.h, DefaultDepth(dpy,DefaultScreen(dpy)));

	XGCValues gcv;
	unsigned long gcm;
	gcm = GCGraphicsExposures|GCBackground;
	gcv.graphics_exposures = True;
	gcv.background = getColor( WindowBackgroundColor );
	if( strlen(txtfont) != 0 )
	{
		textFont = XLoadQueryFont( dpy, txtfont );
		if( textFont != NULL )
		{
			gcm |= GCFont;
			gcv.font = textFont->fid;
		} else
			syslog( LOG_NOTICE, "Couldn't load specified font" );
	}
	WinGC = XCreateGC(dpy, Root, gcm, &gcv);

	blankMainWin();
	drawDevice(INACTIVE);
	activateLamps( lampsActive );
	drawSwitches();
	update();

	if(!(wmaker || ushape))
		XSetClipMask(dpy, WinGC, dmsk);
	else
		XShapeCombineMask(dpy, Win[activeWin], ShapeBounding, 0, 0, dmsk, ShapeSet);

	XSetClipOrigin(dpy, WinGC, 0, 0);
	XSetClipMask(dpy, WinGC, None);

	XEvent event;
	XSelectInput(dpy, Win[activeWin], PointerMotionMask | ButtonPress | ExposureMask);
	XSelectInput(dpy, Win[2], ExposureMask );
	XMapWindow(dpy, Win[0]);

	bool finished=false;
	while(!finished){
		while(XPending(dpy)){
			XNextEvent(dpy,&event);
			switch(event.type){
				case ButtonPress : pressEvent(&event.xbutton); break;
				case MotionNotify : motionEvent(&event.xmotion); break;

				case ClientMessage :
					if((Atom)event.xclient.data.l[0]==WM_DELETE_WINDOW)
						finished=true;
					break;

				case Expose : exposeEvent( &event.xexpose ); break;
			}
		}
		if( timeToUpdate() )
		{
			update();
			manage_slave();
		}
		usleep(50000);
	}

	if( textFont != NULL )
		XFreeFont( dpy, textFont );
	XFreeGC(dpy, WinGC);
	freePixmaps();
	/* Free runtime pixmaps */
	XFreePixmap(dpy, disp_info);
	XFreePixmap(dpy, disp);
	XFreePixmap(dpy, dmsk);
	/* Finish with X stuff */
	freeXWin();
	return 0;
}

void initXWin(int argc, char *argv[])
{
	if( (dpy=XOpenDisplay(txtdpy)) == NULL )
	{
		fprintf(stderr,"cannot open display!\n");
		exit(1);
	}
	_XA_GNUSTEP_WM_FUNC = XInternAtom(dpy, "_GNUSTEP_WM_FUNCTION", false);
	WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
	Root=DefaultRootWindow(dpy);
	createMainWin(&Win[0]);
	createMainWin(&Win[1]);
	createInfoPanel( &Win[2] );
	XWMHints hints;
	XSizeHints shints;
	hints.window_group = Win[0];
	shints.min_width=64;
	shints.min_height=64;
	shints.max_width=64;
	shints.max_height=64;
	shints.x=0;
	shints.y=0;
	if(wmaker)
	{
		hints.initial_state = WithdrawnState;
		hints.icon_window = Win[1];
		hints.flags = WindowGroupHint | StateHint | IconWindowHint;
		shints.flags = PMinSize | PMaxSize | PPosition;
		activeWin=1;
	}
	else {
		hints.initial_state = NormalState;
		hints.flags = WindowGroupHint | StateHint;
		shints.flags = PMinSize | PMaxSize;
		activeWin=0;
	}
	XSetWMHints(dpy, Win[0], &hints);
	XSetWMNormalHints(dpy, Win[0], &shints);
	XSetCommand(dpy, Win[0], argv, argc);
	XStoreName(dpy, Win[0], NAME);
	XSetIconName(dpy, Win[0], NAME);
	XSetWMProtocols(dpy, Win[activeWin], &WM_DELETE_WINDOW, 1);
}

void freeXWin()
{
	XDestroyWindow(dpy, Win[0]);
	XDestroyWindow(dpy, Win[1]);
	XDestroyWindow(dpy, Win[2]);
	XCloseDisplay(dpy);
}

void loadPixmaps()
{
	createPixmap(cover_xpm, &coverPixmap, &dmsk );
	createPixmap(dialing_xpm, &dialingPixmap, NULL );
	createPixmap(unknown_xpm, &unknownPixmap, NULL );
	createPixmap(disabled_xpm, &disabledPixmap, NULL );
	createPixmap(off_xpm, &offPixmap, NULL );

	createPixmap(incoming_xpm, &incomingPixmap, NULL );
	createPixmap(outgoing_xpm, &outgoingPixmap, NULL );
	createPixmap(bundling_xpm, &bundlingPixmap, NULL );
	createPixmap(bundled_xpm, &bundledPixmap, NULL );
	createPixmap(slave_xpm, &slavePixmap, NULL );

	createPixmap(offline_xpm, &statusPixmaps[ISDN_USAGE_NONE], NULL );
	createPixmap(raw_xpm, &statusPixmaps[ISDN_USAGE_RAW], NULL );
	createPixmap(modem_xpm, &statusPixmaps[ISDN_USAGE_MODEM], NULL );
	createPixmap(online_xpm, &statusPixmaps[ISDN_USAGE_NET], NULL );
	createPixmap(voice_xpm, &statusPixmaps[ISDN_USAGE_VOICE], NULL );
	createPixmap(fax_xpm, &statusPixmaps[ISDN_USAGE_FAX], NULL );

	createPixmap(lamps_xpm, &lampsPixmap, NULL );
	createPixmap(info_sw_xpm, &infoSWPixmap, NULL );
	createPixmap(lamps_sw_xpm, &lampsSWPixmap, NULL );
	loadLeds( leds_xpm, &ledsPixmap, InfoTextColor, WindowBackgroundColor );
}

void freePixmaps()
{
	XFreePixmap(dpy, coverPixmap);
	XFreePixmap(dpy, dialingPixmap);
	XFreePixmap(dpy, unknownPixmap);
	XFreePixmap(dpy, disabledPixmap);
	XFreePixmap(dpy, offPixmap);

	XFreePixmap(dpy, incomingPixmap);
	XFreePixmap(dpy, outgoingPixmap);
	XFreePixmap(dpy, bundlingPixmap);
	XFreePixmap(dpy, bundledPixmap);
	XFreePixmap(dpy, slavePixmap);

	for( int i=ISDN_USAGE_NONE; i < ISDN_USAGE_FAX; i++ )
		XFreePixmap( dpy, statusPixmaps[i] );

	XFreePixmap(dpy, ledsPixmap);
	XFreePixmap(dpy, lampsPixmap);
	XFreePixmap(dpy, infoSWPixmap);
	XFreePixmap(dpy, lampsSWPixmap);
}

void createMainWin( Window *win )
{
	*win = XCreateSimpleWindow(dpy, Root, 10, 10, MainWinDim.w, MainWinDim.h,0,0,0);

	XClassHint classHint;
	classHint.res_name = NAME;
	classHint.res_class = CLASS;
	XSetClassHint(dpy, *win, &classHint);
}

void createInfoPanel( Window *win )
{
	*win = XCreateSimpleWindow(dpy, Root, 10, 10, InfoWinDim.w, InfoWinDim.h,0,0,0);

	XSizeHints shints;
	shints.flags = PPosition;
	XSetWMNormalHints( dpy, *win, &shints );

	XClassHint classHint;
	classHint.res_name = "Info";
	classHint.res_class = CLASS;
	XSetClassHint(dpy, *win, &classHint);
}

void createRegions()
{
	region_init(dpy);

	region_add( Win[activeWin], ID_LAMP_GREEN, LampsRect[ID_LAMP_GREEN].pos.x, LampsRect[ID_LAMP_GREEN].pos.y, LampsRect[ID_LAMP_GREEN].dim.w, LampsRect[ID_LAMP_GREEN].dim.h, mouseInLamp, mouseOutLamp, mouseClickLamp );
	region_add( Win[activeWin], ID_LAMP_YELLOW, LampsRect[ID_LAMP_YELLOW].pos.x, LampsRect[ID_LAMP_YELLOW].pos.y, LampsRect[ID_LAMP_YELLOW].dim.w, LampsRect[ID_LAMP_YELLOW].dim.h, mouseInLamp, mouseOutLamp, mouseClickLamp );
	region_add( Win[activeWin], ID_LAMP_RED, LampsRect[ID_LAMP_RED].pos.x, LampsRect[ID_LAMP_RED].pos.y, LampsRect[ID_LAMP_RED].dim.w, LampsRect[ID_LAMP_RED].dim.h, mouseInLamp, mouseOutLamp, mouseClickLamp );

	region_add( Win[activeWin], ID_SWITCH_INFO, InfoSWRect.pos.x, InfoSWRect.pos.y, InfoSWRect.dim.w, InfoSWRect.dim.h, mouseInInfoSw, mouseOutInfoSw, mouseClickInfoSw );
	region_add( Win[activeWin], ID_SWITCH_LAMPS, LampsSWRect.pos.x, LampsSWRect.pos.y, LampsSWRect.dim.w, LampsSWRect.dim.h, mouseInLampsSw, mouseOutLampsSw, mouseClickLampsSw );

	region_add( Win[activeWin], ID_DEVICE, DeviceRect.pos.x, DeviceRect.pos.y, DeviceRect.dim.w, DeviceRect.dim.h, mouseInDevice, mouseOutDevice, mouseClickDevice );
	region_add( Win[activeWin], ID_SWITCH_STATUS, StatusPixmapRect.pos.x, StatusPixmapRect.pos.y, StatusPixmapRect.dim.w, StatusPixmapRect.dim.h, mouseInStatusSw, mouseOutStatusSw, mouseClickStatusSw );
}

bool validIppp( char *name )
{
	if( strlen(name) < 5 )
		return false;
	if( strncmp( name, "ippp", 4 ) != 0 )
		return false;
	for( char *p=name+4; *p != '\x0'; p++ )
		if( !isdigit(*p) )
			return false;
	if( getIpppNum(name) >= ISDN_MAX_CHANNELS )
		return false;
	return true;
}

int getIpppNum( char *name )
{
	return atoi( name + 4 );
}

void selectNextDevice()
{
	selected_device++;
	if( devices[selected_device][0] == 0 )
		selected_device = 0;
	drawDevice();
	update();
}

void printUsage( char *prog_name )
{
	fprintf( stderr, "usage:\n\n   %s [options]\n\noptions:\n\n", prog_name );
	fprintf( stderr, "   -h | -help | --help    display this help screen\n");
	fprintf( stderr, "   -w                     use WithdrawnState (for WindowMaker)\n" );
	fprintf( stderr, "   -s                     shaped window\n" );
	fprintf( stderr, "   -display display       select target display (see X manual pages)\n" );
	fprintf( stderr, "   -font font             select the font for displaying status information\n" );
	fprintf( stderr, "   -dialmode mode         select dial mode for offline mode (auto or manual)\n" );
	fprintf( stderr, "   -device device         select ippp devices to monitor\n" );
	fprintf( stderr, "                           (a list of comma-separated device names is expected containing __no blanks__)\n" );
	fprintf( stderr, "   -lamps                 activate the line control switches upon startup\n" );
	fprintf( stderr, "   -usescripts            use user scripts for dialing/hanging up instead of direct ioctl calls\n" );
	fprintf( stderr, "   -path path             select directory with the up-/down-scripts\n\n" );
}

void printHeader()
{
	fprintf( stderr, "wmisdn v1.8 (C) 1999-2001 Tasho Statev Kaletha (kaletha@informatik.uni-bonn.de).\n\n" );
}

void parseDeviceNames( char *name_list )
{
	char *ptr1, *ptr2;
	int i;

	ptr1 = name_list;

	for( i = 0; i < ISDN_MAX_CHANNELS; i++ )
	{
		ptr2 = strchr(ptr1,',');
		if( ptr2 == NULL )
			ptr2 = &name_list[strlen(name_list)];
		strncpy( devices[i], ptr1, ptr2-ptr1 );
		devices[i][ptr2-ptr1] = 0;
		devices[i+1][0] = 0;
		if( !validIppp(devices[i]) )
			fprintf( stderr, "Warning : \"%s\" doesn't seem to be a valid ippp device. wmisdn may not work properly\n", devices[i] );
		if( ptr2[0] == 0 )
			return;
		ptr1 = ptr2+1;
	}
}


bool scanArgs(int argc, char *argv[])
{
	bool dialmode_set = false;

	for(int i=1;i<argc;i++)
	{
		if(strcmp(argv[i],"-h")==0 || strcmp(argv[i],"-help")==0 || strcmp(argv[i],"--help")==0)
			return false;

		else if(strcmp(argv[i],"-w")==0)
			wmaker=true;
		else if(strcmp(argv[i],"-s")==0)
			ushape=true;
		else if(strcmp(argv[i],"-lamps")==0)
			lampsActive=true;
		else if(strcmp(argv[i],"-usescripts")==0)
			usescripts=true;

		else if(strcmp(argv[i],"-display")==0)
		{
			if(i<argc-1)
			{
				i++;
				if( strlen(argv[i]) > MAX_ARG_LEN-1 )
				{
					fprintf( stderr, "Argument for -display option too long\n" );
					return false;
				}
				sprintf(txtdpy,"%s",argv[i]);
			}
			continue;
		}
		else if(strcmp(argv[i],"-font")==0)
		{
			if(i<argc-1)
			{
				i++;
				if( strlen(argv[i]) > MAX_ARG_LEN-1 )
				{
					fprintf( stderr, "Argument for -font option too long\n" );
					return false;
				}
				sprintf(txtfont,"%s",argv[i]);
			}
			continue;
		}
		else if(strcmp(argv[i],"-dialmode")==0)
		{
			if(i<argc-1)
			{
				i++;
				if( strcmp(argv[i], "auto")==0 )
					dialmode = ISDN_NET_DM_AUTO;
				else if( strcmp(argv[i], "manual")==0 )
					dialmode = ISDN_NET_DM_MANUAL;
				else {
					fprintf( stderr, "Unknown dial mode \"%s\"\n", argv[i] );
					return false;
				}
				dialmode_set = true;
			}
		}
		else if(strcmp(argv[i],"-device")==0)
		{
			if(i<argc-1)
			{
				i++;
				if( strlen(argv[i]) > MAX_ARG_LEN-1 )
				{
					fprintf( stderr, "Argument for -device option too long\n" );
					return false;
				}
				parseDeviceNames( argv[i] );
				selected_device = 0;
			}
		}
		else if(strcmp(argv[i],"-path")==0)
		{
			if(i<argc-1)
			{
				i++;
				if( strlen(argv[i]) > MAX_ARG_LEN-1 )
				{
					fprintf( stderr, "Argument for -path option too long\n" );
					return false;
				}
				strcpy( scriptpath, argv[i] );
			}
		}
		else {
			fprintf( stderr, "Unknown option \"%s\"\n", argv[i] );
			return false;
		}

	}
	if( !dialmode_set )
		isdnInitDefaultDialmode();
	if( selected_device == -1 )
	{
		strcpy( devices[0], "ippp0" );
		devices[1][0] = 0;
		selected_device = 0;
	}

	return true;
}

/* Reads the string representations of the scriptmode parameter given to the up/down scripts
 * and initializes the coresponding variables */
void readScriptModes()
{
	char filename[1000];
	char buf[1000];
	sprintf( filename, "%s/%s", scriptpath, SCRIPT_MODES );
	FILE *f = fopen( filename, "r" );
	/* init one default string if reading fails */
	if( f == NULL || (fgets(buf,sizeof(buf),f) == NULL) || !usescripts )
	{
		maxscriptmode = 0;
		if( usescripts )
			syslog( LOG_NOTICE, "Couldn't read script mode strings: %m" );
		scriptmodestrings = (char **)malloc( sizeof(char*) );
		scriptmodestrings[0] = (char *)malloc( sizeof("go online") );
		strcpy(scriptmodestrings[0], "go online" );
		if( f != NULL )
			fclose(f);
		return;
	}
	maxscriptmode = -1; /* the first iteration sets it to 0 - first array index */

	/* read the strings and put them into the scriptmodestrings array */
	do
	{
		maxscriptmode++;
		scriptmodestrings = (char **)realloc( scriptmodestrings, (maxscriptmode+1)*sizeof(char*) );
		scriptmodestrings[maxscriptmode] = (char *)malloc( strlen(buf)+1 );
		while( strchr(buf,'\n') != NULL )
			*strchr(buf,'\n') = '\0';
		strcpy( scriptmodestrings[maxscriptmode], buf );
	} while( fgets(buf,sizeof(buf),f) != NULL );
	fclose(f);
}

void advanceScriptMode()
{
	scriptmode++;
	if(scriptmode > maxscriptmode)
		scriptmode = 0;
	update();
}

/* press event
 * - if a lamp is pressed then the corresponding actions are taken.
 * - outside a lamp the extended view is turned on or off
 */
void pressEvent(XButtonEvent *xev)
{
	if( region_in( xev->window, xev->x, xev->y ) )
		region_mouse_click( xev->window, xev->x, xev->y, xev->button );
}

/* pointer motion
 * - draws a lamp in an active state if the pointer passes above it
 */
void motionEvent( XMotionEvent *xev )
{
	region_mouse_motion( xev->window, xev->x, xev->y );
}

void exposeEvent( XExposeEvent *xev )
{
	repaint( xev->window, xev->x, xev->y, xev->width, xev->height );
}

void alignInfoPanel()
{
	/* get the position of the main win */
	int win_x, win_y, screen_w, screen_h, panel_x, panel_y;
	getWindowPosition( Win[activeWin], &win_x, &win_y );
	getWindowDimension( Root, &screen_w, &screen_h );
	/* find a suitable position for the info panel */
	if( win_x - InfoWinDim.w > 0 )
		panel_x = win_x - InfoWinDim.w;
	else
		panel_x = win_x + MainWinDim.w;
	panel_y = win_y;
	/* move the panel */
	XMoveWindow( dpy, Win[2], panel_x, panel_y );
}

void mouseInLamp( int id )
{
	drawLamp( id, ACTIVE );
	for( int i=0; i < 3; i++ )
		repaint( Win[activeWin], LampsRect[i].pos.x, LampsRect[i].pos.y, LampsRect[i].dim.w, LampsRect[i].dim.h );
}

void mouseOutLamp( int id )
{
	drawLamp( id, INACTIVE );
	for( int i=0; i < 3; i++ )
		repaint( Win[activeWin], LampsRect[i].pos.x, LampsRect[i].pos.y, LampsRect[i].dim.w, LampsRect[i].dim.h );
}

void mouseClickLamp( int id, unsigned int button )
{
	pressLamp( id, button );
	update();
}

inline void drawInfoSwitch( int active )
{
	int pixmap_index = (infoPanelActive ? 2:0) + active;
	int offset_x = pixmap_index * InfoSWRect.dim.w;
	XCopyArea( dpy, infoSWPixmap, disp, WinGC, offset_x, 0, InfoSWRect.dim.w, InfoSWRect.dim.h, InfoSWRect.pos.x, InfoSWRect.pos.y );
	repaint( Win[activeWin], InfoSWRect.pos.x, InfoSWRect.pos.y, InfoSWRect.dim.w, InfoSWRect.dim.h );
}

void mouseInInfoSw( int id )
{
	drawInfoSwitch(ACTIVE);
}

void mouseOutInfoSw( int id )
{
	drawInfoSwitch(INACTIVE);
}

void mouseClickInfoSw( int id, unsigned int button )
{
	if( !infoPanelActive )
	{
		alignInfoPanel();
		XMapWindow( dpy, Win[2] );
	} else
		XUnmapWindow( dpy, Win[2] );
	infoPanelActive = !infoPanelActive;
	mouseInInfoSw( ID_SWITCH_INFO );
	fullRepaint();
}

inline void drawLampsSwitch( int active )
{
	int pixmap_index = (lampsActive ? 2:0) + active;
	int offset_x = pixmap_index * LampsSWRect.dim.w;
	XCopyArea( dpy, lampsSWPixmap, disp, WinGC, offset_x, 0, LampsSWRect.dim.w, LampsSWRect.dim.w, LampsSWRect.pos.x, LampsSWRect.pos.y );
	repaint( Win[activeWin], LampsSWRect.pos.x, LampsSWRect.pos.y, LampsSWRect.dim.w, LampsSWRect.dim.h );
}

void mouseInLampsSw( int id )
{
	drawLampsSwitch(ACTIVE);
}

void mouseOutLampsSw( int id )
{
	drawLampsSwitch(INACTIVE);
}

void mouseClickLampsSw( int id, unsigned int button )
{
	activateLamps( !lampsActive );
	mouseInLampsSw( ID_SWITCH_LAMPS );
}

void activateLamps( bool active )
{
	if( active )
	{
		drawLamp( 0, INACTIVE );
		drawLamp( 1, INACTIVE );
		drawLamp( 2, INACTIVE );
		region_enable( Win[activeWin], ID_LAMP_GREEN );
		region_enable( Win[activeWin], ID_LAMP_YELLOW );
		region_enable( Win[activeWin], ID_LAMP_RED );
	} else {
		for( int i=0; i < 3; i++ )
			blankMainWin( LampsRect[i].pos.x, LampsRect[i].pos.y, LampsRect[i].dim.w, LampsRect[i].dim.h );
		region_disable( Win[activeWin], ID_LAMP_GREEN );
		region_disable( Win[activeWin], ID_LAMP_YELLOW );
		region_disable( Win[activeWin], ID_LAMP_RED );
	}
	lampsActive = active;
	fullRepaint();
}

void drawSwitches()
{
	drawInfoSwitch( INACTIVE );
	drawLampsSwitch( INACTIVE );
}

void mouseInDevice( int id )
{
	drawDevice(ACTIVE);
	repaint( Win[activeWin], 0, 0, MainWinDim.w, MainWinDim.h );
}

void mouseOutDevice( int id )
{
	drawDevice(INACTIVE);
	repaint( Win[activeWin], 0, 0, MainWinDim.w, MainWinDim.h );
}

void mouseClickDevice( int id, unsigned int button )
{
	selectNextDevice();
	mouseInDevice( ID_DEVICE );
}


void drawDevice( int active )
{
	char *color = active == ACTIVE ? DeviceColorHigh : DeviceColorLow;
	drawText( devices[selected_device], disp, DeviceRect.pos.x, DeviceRect.pos.y, color );
}

void mouseInStatusSw( int id )
{
       /* drawStatusSw( ACTIVE ); */
}

void mouseOutStatusSw( int id )
{
       /* drawStatusSw( INACTIVE ); */
}

void mouseClickStatusSw( int id, unsigned int button )
{
       if(button == 2)
               pressStatusSw();
}

/* void drawStatusSw( int active ) */

void getWindowPosition( Window win, int *x, int *y )
{
	XWindowAttributes winAttr;
	Window dummy;

	XGetWindowAttributes( dpy, win, &winAttr );
	XTranslateCoordinates( dpy, win, winAttr.root,
						  -winAttr.border_width, -winAttr.border_width,
						  x, y, &dummy );
}

void getWindowDimension( Window win, int *w, int *h )
{
	XWindowAttributes winAttr;
	XGetWindowAttributes( dpy, win, &winAttr );
	*w = winAttr.width;
	*h = winAttr.height;
}
void repaint( Window win, int x, int y, int w, int h )
{
	//bad code start
	Pixmap src;
	if( win == Win[activeWin] )
		src = disp;
	else if( win == Win[2] )
		src = disp_info;
	else {
		syslog( LOG_DEBUG, "Oops! Unknown window given to repaint\n" );
		return;
	}
	//bade code end

	XCopyArea( dpy, src, win, WinGC, x, y, w, h, x, y );
	XFlush(dpy);
}

void fullRepaint()
{
	repaint( Win[activeWin], 0, 0, MainWinDim.w, MainWinDim.h );
	if( infoPanelActive )
		repaint( Win[2], 0, 0, InfoWinDim.w, InfoWinDim.h );
}

bool timeToUpdate()
{
	static int ticker = 0;
	if( ticker++ > UPDATE_INTERVAL )
	{
		ticker = 0;
		return true;
	}
	return false;
}

/* get ISDN device status and update windows as needed */
void update()
{
	isdnStatus stat;
	getStatus( devices[selected_device], &stat );
	if( memcmp(&curStatus, &stat, sizeof(stat)) != 0 )
	{
		memcpy( &curStatus, &stat, sizeof(stat) );
		setStatusPixmap();
//		drawDevice();
		repaint( Win[activeWin], 0, 0, MainWinDim.w, MainWinDim.h );
	}
	updateInfoPanel();
	repaint( Win[2], 0, 0, InfoWinDim.w, InfoWinDim.h );
}

/* set the appropriate pixmap on the main window */
void setStatusPixmap()
{
	Pixmap statusPixmap, directionPixmap;

	if( curStatus.usage > ISDN_USAGE_NONE && curStatus.usage <= ISDN_USAGE_FAX )
	{
		statusPixmap = statusPixmaps[curStatus.usage];
		switch( curStatus.mpppMode )
		{
			case slave	: directionPixmap = slavePixmap; break;
			case master	: 	if( curStatus.bundled == true ) { directionPixmap = bundledPixmap; break; }
							if( is_slave_pending() ) { directionPixmap = bundlingPixmap; break; }
			case none	: directionPixmap = curStatus.direction == INCOMING ? incomingPixmap : outgoingPixmap; break;
			default : syslog( LOG_DEBUG, "Ooops! curStatus.direction has an invalid value\n" ); directionPixmap = 0;
		}
	}
	else {
		switch( curStatus.usage )
		{
			case STAT_OFF 		: statusPixmap = offPixmap; break;
			case ISDN_USAGE_NONE: statusPixmap = statusPixmaps[ISDN_USAGE_NONE]; break;
			case STAT_DISABLED 	: statusPixmap = disabledPixmap; break;
			case STAT_DIALING	: statusPixmap = dialingPixmap; break;
			case STAT_UNKNOWN	: statusPixmap = unknownPixmap; break;
			default : syslog( LOG_DEBUG, "Ooops! curStatus.usage has an invalid value\n" ); statusPixmap = 0;
		}
		directionPixmap = 0;
	}
	if( statusPixmap != 0 )
		XCopyArea(dpy, statusPixmap, disp, WinGC, StatusPixmapRect.pos.x, StatusPixmapRect.pos.y, StatusPixmapRect.dim.w, StatusPixmapRect.dim.h, StatusPixmapRect.pos.x, StatusPixmapRect.pos.y);
	if( directionPixmap != 0 )
		XCopyArea(dpy, directionPixmap, disp, WinGC, DirectionPixmapRect.pos.x, DirectionPixmapRect.pos.y, DirectionPixmapRect.dim.w, DirectionPixmapRect.dim.h, DirectionPixmapRect.pos.x, DirectionPixmapRect.pos.y);
}

/* update the info panel */
void updateInfoPanel()
{
	XSetForeground( dpy, WinGC, getColor(WindowBackgroundColor) );
	XFillRectangle( dpy, disp_info, WinGC, 0, 0, InfoWinDim.w, InfoWinDim.h );
	char line[100];
	if( (curStatus.usage > ISDN_USAGE_NONE && curStatus.usage <= ISDN_USAGE_FAX) || curStatus.usage == STAT_DIALING )
	{
		sprintf( line, "peer phone: %s", curStatus.peerPhone );
		drawText( line, disp_info, 5, 5 );
	}
	if( (curStatus.usage == ISDN_USAGE_NET) && (curStatus.mpppMode != slave) )
	{
		int a, b, c, d;
		getLocalIP( &a, &b, &c, &d );
		sprintf( line, "local ip : %d.%d.%d.%d", a, b, c, d );
		drawText( line, disp_info, 5, 20 );
		getRemoteIP( &a, &b, &c, &d );
		sprintf( line, "remote ip: %d.%d.%d.%d", a, b, c, d );
		drawText( line, disp_info, 5, 35 );
	}
	else if( curStatus.usage == STAT_OFF )
			drawText( "dialing disabled", disp_info, 5, 5 );
	else if( curStatus.usage == STAT_DISABLED )
		drawText( "device disabled", disp_info, 5, 5 );
	else if( curStatus.usage == ISDN_USAGE_NONE )
	{
		drawText( "not connected", disp_info, 5, 5 );
		sprintf( line, "action: %s", scriptmodestrings[scriptmode] );
		drawText( line, disp_info, 5, 20 );
	}
	switch( curStatus.mpppMode )
	{
		case none : sprintf( line, "bundling: none" ); break;
		case master : sprintf( line, "bundling: master of %s", curStatus.mpppPartner ); break;
		case slave : sprintf( line, "bundling: slave of %s", curStatus.mpppPartner ); break;
	}
	drawText( line, disp_info, 5, 50 );
}

void blankMainWin( int x, int y, int w, int h )
{
	XCopyArea(dpy, coverPixmap, disp, WinGC, x, y, w, h, x, y);
}

unsigned long getColor( const char *colorname )
{
	XColor color;
	XWindowAttributes winattr;
	XGetWindowAttributes(dpy, Root, &winattr);
	color.pixel=0;
	XParseColor(dpy, winattr.colormap, colorname, &color);
	color.flags=DoRed | DoGreen | DoBlue;
	XAllocColor(dpy, winattr.colormap, &color);
	return color.pixel;
}

void createPixmap(char *data[], Pixmap *image, Pixmap *mask )
{
	XpmAttributes pixatt;

	pixatt.exactColors=false;
	pixatt.closeness=40000;
	pixatt.valuemask=XpmExactColors | XpmCloseness | XpmSize;
	XpmCreatePixmapFromData(dpy, Root, data, image, mask, &pixatt);
}

void loadLeds( char *data[], Pixmap *image, const char *led_color, const char *back_color)
{
	XpmAttributes pixatt;
	unsigned long color[4];

	color[0] = mixColor(led_color, 0, back_color, 100);
	color[1] = mixColor(led_color, 100, back_color, 0);
	color[2] = mixColor(led_color, 60, back_color, 40);
	color[3] = mixColor(led_color, 25, back_color, 75);

	XpmColorSymbol xpmcsym[4]={{"led_color_back",     NULL, color[0] },
	                           {"led_color_high", NULL, color[1]},
	                           {"led_color_med",  NULL, color[2]},
	                           {"led_color_low",  NULL, color[3]}};


	pixatt.numsymbols = 4;
	pixatt.colorsymbols = xpmcsym;
	pixatt.exactColors = false;
	pixatt.closeness = 40000;
	pixatt.valuemask = XpmColorSymbols | XpmExactColors | XpmCloseness | XpmSize;
	XpmCreatePixmapFromData(dpy, Root, data, image, NULL, &pixatt);
}

unsigned long mixColor( const char *colorname1, int prop1, const char *colorname2, int prop2 )
{
	XColor color, color1, color2;
	XWindowAttributes winattr;
	XGetWindowAttributes(dpy, Root, &winattr);
	XParseColor(dpy, winattr.colormap, colorname1, &color1);
	XParseColor(dpy, winattr.colormap, colorname2, &color2);
	color.pixel=0;
	color.red=(color1.red*prop1+color2.red*prop2)/(prop1+prop2);
	color.green=(color1.green*prop1+color2.green*prop2)/(prop1+prop2);
	color.blue=(color1.blue*prop1+color2.blue*prop2)/(prop1+prop2);
	color.flags=DoRed | DoGreen | DoBlue;
	XAllocColor(dpy, winattr.colormap, &color);
	return color.pixel;
}

/* draws text on dst using the led symbols from the leds pixmap */
void leds_drawText( char *text, Pixmap dst, int x, int y, const char *color )
{
	Pixmap leds_pixmap;
	loadLeds( leds_xpm, &leds_pixmap, color, WindowBackgroundColor );

	x -= 3;
	char *led_ptr;
	while( *text != 0 )
	{
		led_ptr = strchr( led_text, tolower(*text) );
		if( led_ptr == NULL )
		{
			/*syslog( LOG_DEBUG, "Oops! Internal bug in drawText: No led symbol for char %c\n", *text );*/
			led_ptr = strchr( led_text, '?' );
		}
		XCopyArea( dpy, leds_pixmap, dst, WinGC, (led_ptr-led_text)*LedDim.w, 0, LedDim.w, LedDim.h, x, y );
		x += LedDim.w;
		text++;
	}
	XFreePixmap( dpy, leds_pixmap );
}

/* draws text on dst using the X-Font from textFont */
void font_drawText( char *text, Pixmap dst, int x, int y, const char *color )
{
	XSetForeground( dpy, WinGC, getColor(color) );
	XDrawImageString( dpy, dst, WinGC, x, y+textFont->ascent/2, text, strlen(text) );
}

void drawText( char *text, Pixmap dst, int x, int y, const char *color )
{
	if( textFont == NULL )
		leds_drawText( text, dst, x, y, color );
	else
		font_drawText( text, dst, x, y, color );
}

/* draws the lamp in the specified state */
void drawLamp( int lamp, int active )
{
	int disp_x, disp_y, lamp_x=0;

	disp_x = LampsRect[lamp].pos.x;
	disp_y = LampsRect[lamp].pos.y;

	/* find the offset of the lamp pixmap in the pixmap of all lamps */
	for( int i=0; i < lamp; i++ )
		lamp_x += LampsRect[i].dim.w*2;
	lamp_x += active*LampsRect[lamp].dim.w;

	XCopyArea( dpy, lampsPixmap, disp, WinGC, lamp_x, 0, LampsRect[lamp].dim.w, LampsRect[lamp].dim.h, disp_x, disp_y );
}

void isdnInitDefaultDialmode()
{
	seteuid( rootUID );
	setegid( rootGID );

	isdn_net_ioctl_cfg cfg;
	strcpy( cfg.name, devices[selected_device] );
	if( isdn_ioctl( IIOCNETGCF, &cfg, NULL ) != -1 )
		dialmode = cfg.dialmode;
	else
		dialmode = ISDN_NET_DM_AUTO;  /* for the sake of cleanness, we'll get an error msg soon anyway */
	if( dialmode == ISDN_NET_DM_OFF )
		dialmode = ISDN_NET_DM_AUTO;  /* use auto as default dialmode if device disabled */

	seteuid( getuid() );
	setegid( getgid() );
}

int isdn_ioctl( int func, void *arg, const char *errmsg, const char *filename )
{
	int fd = fd = open( filename, O_RDONLY );
	if( fd == -1 )
	{
		if( errmsg != NULL )
			syslog( LOG_NOTICE, "Couldn't open %s : %m\n", filename );
		return -1;
	}

	int res = ioctl( fd, func, arg );
	if( res == -1 && errmsg != NULL )
		syslog( LOG_NOTICE, "%s : %m\n", errmsg );

	close(fd);

	return res;
}

inline void isdn_dial()
{
	if( !usescripts )
		isdn_ioctl( IIOCNETDIL, devices[selected_device], "Couldn't dial" );
	else
	{

		int handle;
		char command[MAX_ARG_LEN];

		strcpy(command, scriptpath);
		strcat(command, "/");
		strcat(command, SCRIPT_UP);
		if ((handle = open(command, O_RDONLY)) == -1)
			syslog( LOG_NOTICE, "Couldn't open %s : %m\n", SCRIPT_UP );
		else {
			close(handle);
			sprintf(command, "%s/%s %s %d 2>&1 | logger -t wmisdn.sh &", scriptpath, SCRIPT_UP, devices[selected_device], scriptmode);
			system(command);
		}
	}
	update();
}

inline void isdn_hangup()
{
	if( !usescripts )
		isdn_ioctl( IIOCNETHUP, devices[selected_device], "Couldn't hang up" );
	else
	{
		int handle;
		char command[MAX_ARG_LEN];

		strcpy(command, scriptpath);
		strcat(command, "/");
		strcat(command, SCRIPT_DOWN);

		if ((handle = open(command, O_RDONLY)) == -1)
			syslog( LOG_NOTICE, "Couldn't open %s : %m\n", SCRIPT_DOWN );
		else {
			close(handle);
			sprintf(command, "%s/%s %s %d 2>&1 | logger -t wmisdn.sh &", scriptpath, SCRIPT_DOWN, devices[selected_device], scriptmode);
			system(command);
		}
	}
	update();
}

inline void isdn_enable()
{
	isdn_net_ioctl_cfg cfg;
	strcpy( cfg.name, devices[selected_device] );
	if( isdn_ioctl( IIOCNETGCF, &cfg, "Error enabling dialing. Couldn't get dev cfg" ) != -1 )
	{
		cfg.dialmode = dialmode;
		isdn_ioctl( IIOCNETSCF, &cfg, "Error enabling dialing. Couldn't set dev cfg" );
	}
}

inline void isdn_disable()
{
	isdn_net_ioctl_cfg cfg;
	strcpy( cfg.name, devices[selected_device] );
	if( isdn_ioctl( IIOCNETGCF, &cfg, "Error disabling dialing. Couldn't get dev cfg" ) != -1 )
	{
		cfg.dialmode = ISDN_NET_DM_OFF;
		isdn_ioctl( IIOCNETSCF, &cfg, "Error disabling dialing. Couldn't set dev cfg" );
	}
}

inline void isdn_dial_slave( char *master )
{
	isdn_ioctl( IIOCNETALN, master, "Couldn't fire up slave" );
}

inline void isdn_hangup_slave( char *master )
{
	isdn_ioctl( IIOCNETDLN, master, "Couldn't hang up slave" );
}

inline void set_slave_pending()
{
	slave_pending = devices[selected_device];
}

inline void clear_slave_pending()
{
	slave_pending = NULL;
}

inline bool is_slave_pending()
{
	return slave_pending != NULL;
}

inline void manage_slave()
{
	if( is_slave_pending() )
	{
		if( curStatus.usage == ISDN_USAGE_NET )
		{
			isdn_dial_slave(slave_pending);
			clear_slave_pending();
		}
		if( curStatus.usage == ISDN_USAGE_NONE )
			clear_slave_pending();
	}
}

/* react upon a lamp press
 * - green opens a connection and sets the device in dial_auto mode
 * - yellow ends the connection and sets the device in dial_auto mode
 * - red ends the connection and sets the device in dial_off mode
 */

inline void _pressGreenLamp( int button )
{
	/* middle button - just change the script mode */
	if( button == 2 )
	{
		advanceScriptMode();
		return;
	}
	/* online request of slave - add a channel to the master */
	if( curStatus.mpppMode == slave )
		isdn_dial_slave( curStatus.mpppPartner );
	/* online request of master or non-bundled device */
	else {
		/* additional mppp-link requested - add a slave channel to the master (wait until master online) */
		if( (button == 3) && (curStatus.mpppMode == master) )
			set_slave_pending();
		/* if device is dialing or online - ignore button */
		if( curStatus.usage == STAT_DIALING || curStatus.usage == ISDN_USAGE_NET )
			return;
		if( curStatus.usage == STAT_OFF )
			isdn_enable();
		isdn_dial();
	}
}

inline void _pressYellowLamp( int button )
{
	if( curStatus.usage == ISDN_USAGE_NONE )
		return;
	if( curStatus.usage == STAT_OFF )
		isdn_enable();
	else
	{
		if( (button == 3) || (button == 1) && (curStatus.mpppMode == master) )
			isdn_hangup_slave( devices[selected_device] );
		if( button == 1 )
		{
			if( curStatus.mpppMode == slave )
				isdn_hangup_slave( curStatus.mpppPartner );
			else
				isdn_hangup();
		}
	}
}

inline void _pressRedLamp( int button )
{
	if( (curStatus.usage == STAT_OFF) || (button != 1) )
		return;
	_pressYellowLamp( button );
	isdn_disable();
}

void pressLamp( int lamp_id, int button )
{
	seteuid( rootUID );
	setegid( rootGID );
	switch( lamp_id )
	{
		case ID_LAMP_GREEN	: _pressGreenLamp( button );break;
		case ID_LAMP_YELLOW	: _pressYellowLamp( button ); break;
		case ID_LAMP_RED	: _pressRedLamp( button ); break;
	}
	seteuid( getuid() );
	setegid( getgid() );
}

/* activated when user clicks the status pixmap with the right button - switch online<->offline */
void pressStatusSw()
{
	seteuid( rootUID );
	setegid( rootGID );

	if( curStatus.usage == ISDN_USAGE_NONE )
		_pressGreenLamp(1);
	else if(curStatus.usage == ISDN_USAGE_NET )
		_pressYellowLamp(1);

	seteuid( getuid() );
	setegid( getgid() );
}


/* Get the local/remote IP addresses of the devices[selected_device] */

void getLocalIP( int *a, int *b, int *c, int *d )
{
	struct ifreq ifr;
	int fd = socket( AF_INET, SOCK_DGRAM, 0 );

	strcpy( ifr.ifr_ifrn.ifrn_name, devices[selected_device] );
	ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;
	int res = ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	translateIP( &(ifr.ifr_ifru.ifru_addr), a, b, c, d );

	if( res != 0 )
		syslog( LOG_NOTICE, "Oops! Couldn't get local IP of device %s. ioctl() call failed : %m\n", devices[selected_device] );
}

void getRemoteIP( int *a, int *b, int *c, int *d )
{
	struct ifreq ifr;
	int fd = socket( AF_INET, SOCK_DGRAM, 0 );

	strcpy( ifr.ifr_ifrn.ifrn_name, devices[selected_device] );
	ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;
	int res = ioctl( fd, SIOCGIFDSTADDR, &ifr );
	close(fd);
	translateIP( &(ifr.ifr_ifru.ifru_addr), a, b, c, d );

	if( res != 0 )
		syslog( LOG_NOTICE, "Oops! Couldn't get remote IP of device %s. ioctl() call failed : %m\n", devices[selected_device]);
}

/* extract the ip address from the addr struct asuming that it is a valid INET sockaddr */
inline void translateIP( struct sockaddr *addr, int *a, int *b, int *c, int *d )
{
	struct sockaddr_in* inet_addr = (sockaddr_in *)addr;
	unsigned int ip = inet_addr->sin_addr.s_addr;
	*d = (ip >> 24) & 0xFF;
	*c = (ip >> 16) & 0xFF;
	*b = (ip >> 8 ) & 0xFF;
	*a = ip & 0xFF;
}

/* extract the data from the 'key'-line of /dev/isdninfo for all 16 B-Channels */
bool extractIsdnInfoData( const char *all_data, const char *key, char buffer[ISDN_MAX_CHANNELS][100] )
{
	char temp[100]; /* buffer the key string */
	char *ptr;

	ptr = strstr( all_data, key );
	if( ptr == NULL )
	{
		syslog( LOG_NOTICE, "Error getting status info. /dev/isdninfo doesn't contain a '%s' line\n", key );
		return false;
	}
	sscanf( ptr, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
		   temp,
		   buffer[0], buffer[1], buffer[2], buffer[3],
		   buffer[4], buffer[5], buffer[6], buffer[7],
		   buffer[8], buffer[9], buffer[10], buffer[11],
		   buffer[12], buffer[13], buffer[14], buffer[15] );

	return true;
}

bool getPeerPhone( char *ippp, char *phone )
{
	isdn_net_ioctl_phone ippp_phone;
	int res;

	strcpy( ippp_phone.name, ippp );
	res = isdn_ioctl( IIOCNETGPN, &ippp_phone, NULL, "/dev/isdninfo" );
	if( res < 0 )
	{
		if( errno != ENOTCONN ) /* device not connected - no real error */
			syslog( LOG_NOTICE, "Error getting phone number for device %s: %m", ippp );
		return false;
	}
	strcpy( phone, ippp_phone.phone );
	return true;
}

bool findBChannel( char *phone, char all_phones[ISDN_MAX_CHANNELS][100], int &channel )
{
	for( int i=0; i < ISDN_MAX_CHANNELS; i++ )
		if( strcmp( all_phones[i], phone ) == 0 )
		{
			channel = i;
			return true;
		}
	syslog( LOG_NOTICE, "Hmm!!?? That's strange! Device phone number %s couldn't be found in /dev/isdninfo", phone );
	return false;
}

void getMPPPSettings( isdn_net_ioctl_cfg *cfg, isdnStatus *stat )
{
	stat->mpppMode = none;
	if( strlen(cfg->master) != 0 )
	{
		stat->mpppMode = slave;
		strcpy( stat->mpppPartner, cfg->master );
	}
	if( strlen(cfg->slave) != 0 )
	{
		stat->mpppMode = master;
		strcpy( stat->mpppPartner, cfg->slave );
	}
}

/* get the status of the ippp device:
 *
 * - isOff if dialing is disabled
 * - isOffline if dialing is enabled but no connection is established
 * - isOnline if device has established a connection
 * - isDialing if device is dialing the remote but no connection is established
 * - isUnknown if no stat info available
 */
void getStatus( char *device, isdnStatus *stat )
{
	isdn_net_ioctl_cfg cfg;
	int fd, len, res;
	char buf[10000];
	static int warning_count=0;
	int channel, channel_usage;
	char channel_info[ISDN_MAX_CHANNELS][100];


	/* get ippp device config */
	seteuid( rootUID );
	setegid( rootGID );
	strcpy( cfg.name, device );
	res = isdn_ioctl( IIOCNETGCF, &cfg, warning_count < STATUS_WARNING_SAT ? "Error getting status info. Couldn't get device cfg" : (char *)NULL );
	seteuid( getuid() );
	setegid( getgid() );

	stat->usage = STAT_UNKNOWN;
	stat->direction = INCOMING;

	if( res == -1 )
	{
		warning_count++;
		return;
	}
	warning_count = 0;
	if( cfg.dialmode == ISDN_NET_DM_OFF  )
		stat->usage = STAT_OFF;
	else
		stat->usage = ISDN_USAGE_NONE;

	stat->bundled = false;
	getMPPPSettings( &cfg, stat );

	/* read the device flags from /dev/isdninfo */
	fd = open( "/dev/isdninfo", O_RDONLY|O_NDELAY );
	if( fd == -1 )
	{
		syslog( LOG_NOTICE, "Error getting status info. Couldn't open /dev/isdninfo : %m\n" );
		return;
	}
	len = read( fd, buf, sizeof(buf)-1 );
	close(fd);
	if( len == -1 )
	{
		syslog( LOG_NOTICE, "Error getting status info. Couldn't read from /dev/isdninfo : %m\n" );
		return;
	}
	buf[len] = 0; 						/* terminate the string */

	if( !extractIsdnInfoData( buf, "phone:", channel_info ) )
		return;
	if( !getPeerPhone( device, stat->peerPhone ) )
		return;
	if( !findBChannel( stat->peerPhone, channel_info, channel ) )
		return;
	if( !extractIsdnInfoData( buf, "usage:", channel_info ) )
		return;

	channel_usage = atoi(channel_info[channel]);
	if( (channel_usage & ISDN_USAGE_DISABLED) != 0 )
		stat->usage = STAT_DISABLED;
	else
		stat->usage =  channel_usage & ISDN_USAGE_MASK;
	stat->direction = (channel_usage & ISDN_USAGE_OUTGOING) == 0 ? INCOMING : OUTGOING;

	/* check if device is still dialing or already online */
	if( stat->usage == ISDN_USAGE_NET )
	{

		if( !extractIsdnInfoData( buf, "flags:", channel_info ) )
			return;

		if( ((atoi(channel_info[0]) >> channel) & 1) == 0 )
			stat->usage = STAT_DIALING;
	}
	/* check for channel bundling */
	if( stat->mpppMode == master )
	{
		isdnStatus slaveStatus;
		getStatus( stat->mpppPartner, &slaveStatus );
		if( (stat->usage == slaveStatus.usage) && (stat->direction == slaveStatus.direction) &&
			(strcmp(stat->peerPhone,slaveStatus.peerPhone)==0) )
			stat->bundled = true;
	}
}
