///////////////////////////////////////////////////////////////////////////////
// wmail.c
// email indicator tool designed as docklet for Window Maker
// main c source-file
//
// Copyright 2000-2002, Sven Geisenhainer <sveng@informatik.uni-jena.de>.
// Copyright 2016-2017, Doug Torrance <dtorrance@piedmont.edu>.
// Copyright 2019, Jeremy Sowden <jeremy@azazel.net>.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions, and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions, and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


///////////////////////////////////////////////////////////////////////////////
// includes

#ifdef HAVE_CONFIG_H
#ifndef CONFIG_H_INCLUDED
#include "../config.h"
#define CONFIG_H_INCLUDED
#endif
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <signal.h>
#include <utime.h>
#include <fnmatch.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <X11/Xlib.h>
#ifdef HAVE_LIBDOCKAPP_DOCKAPP_H
#include <libdockapp/dockapp.h>
#else
#include <dockapp.h>
#endif

#include "common.h"
#include "config.h"

// pixmaps
#ifdef USE_DELT_XPMS
#include "xpm_delt/main.xpm"
#include "xpm_delt/symbols.xpm"
#include "xpm_delt/numbers.xpm"
#include "xpm_delt/button.xpm"
#include "xpm_delt/chars.xpm"
#else
#include "xpm/main.xpm"
#include "xpm/symbols.xpm"
#include "xpm/numbers.xpm"
#include "xpm/button.xpm"
#include "xpm/chars.xpm"
#endif


///////////////////////////////////////////////////////////////////////////////
// typedefs

typedef enum
{
    FLAG_INITIAL = 0,
    FLAG_READ    = 1
} flag_t;

typedef struct _name_t
{
    char *name;
    unsigned long checksum;
    flag_t flag;
    bool visited;
    struct _name_t *next;
} name_t;

typedef enum
{
    STATE_NOMAIL,
    STATE_NEWMAIL,
    STATE_READMAIL
} mail_state_t;

typedef enum
{
    STATE_ADDRESS,
    STATE_QUOTED_ADDRESS,
    STATE_FULLNAME,
    STATE_QUOTED_FULLNAME,
    STATE_ENCODED_FULLNAME,
    STATE_COMMENT
} parse_state_t;


///////////////////////////////////////////////////////////////////////////////
// data

static char *configFile;
static unsigned long lastTimeOut;
static sig_atomic_t caughtSig;
static mail_state_t state;
static unsigned numMails;
static bool namesChanged;
static bool buttonPressed;
static bool readConfigFile;
static bool isMaildir;
static bool forceRead;
static bool forceRedraw = true;
static time_t lastModifySeconds;
static time_t lastAccessSeconds;
static Pixmap mainPixmap;
static Pixmap mainPixmap_mask;
static Pixmap symbolsPixmap;
static Pixmap charsPixmap;
static Pixmap numbersPixmap;
static Pixmap buttonPixmap;
static Pixmap outPixmap;
static GC tickerGC;
static XFontStruct *tickerFS;
static name_t *names;
static name_t *curTickerName;

enum
{
      OPT_INDEX_DISPLAY,
      OPT_INDEX_COMMAND,
      OPT_INDEX_INTERVAL,
      OPT_INDEX_FAMILY_NAME,
      OPT_INDEX_FRAMES,
      OPT_INDEX_SHORT_NAME,
      OPT_INDEX_SYMBOL_COLOR,
      OPT_INDEX_FONT_COLOR,
      OPT_INDEX_BACK_COLOR,
      OPT_INDEX_OFF_COLOR,
      OPT_INDEX_BACKGROUND,
      OPT_INDEX_NO_SHAPE,
      OPT_INDEX_NEW,
      OPT_INDEX_MAILBOX,
      OPT_INDEX_EXECUTE,
      OPT_INDEX_STATUS_FIELD,
      OPT_INDEX_READ_STATUS,
      OPT_INDEX_TICKER_FONT,
      OPT_INDEX_CONFIG_FILE,
};

static DAProgramOption options[] =
{
    [OPT_INDEX_DISPLAY] =
    {
	.shortForm   = "-display",
	.description = "display to use",
	.type        = DOString,
	.value       = { .string = &config.display }
    },
    [OPT_INDEX_COMMAND] =
    {
	.shortForm   = "-c",
	.longForm    = "--command",
	.description = "cmd to run on btn-click (\"xterm -e mail\" is default)",
	.type        = DOString,
	.value       = { .string = &config.runCmd }
    },
    [OPT_INDEX_INTERVAL] =
    {
	.shortForm   = "-i",
	.longForm    = "--interval",
	.description = "number of secs between mail-status updates (1 is default)",
	.type        = DONatural,
	.value       = { .integer = &config.checkInterval }
    },
    [OPT_INDEX_FAMILY_NAME] =
    {
	.shortForm   = "-f",
	.longForm    = "--familyname",
	.description = "tickers the family-name if available",
    },
    [OPT_INDEX_FRAMES] =
    {
	.shortForm   = "-fps",
	.longForm    = "--frames",
	.description = "ticker frames per second",
	.type        = DONatural,
	.value       = { .integer = &config.fps }
    },
    [OPT_INDEX_SHORT_NAME] =
    {
	.shortForm   = "-s",
	.longForm    = "--shortname",
	.description = "tickers the nickname (all before the '@')",
    },
    [OPT_INDEX_SYMBOL_COLOR] =
    {
	.shortForm   = "-sc",
	.longForm    = "--symbolcolor",
	.description = "symbol color-name",
	.type        = DOString,
	.value       = { .string = &config.symbolColor }
    },
    [OPT_INDEX_FONT_COLOR] =
    {
	.shortForm   = "-fc",
	.longForm    = "--fontcolor",
	.description = "ticker-font color-name",
	.type        = DOString,
	.value       = { .string = &config.fontColor }
    },
    [OPT_INDEX_BACK_COLOR] =
    {
	.shortForm   = "-bc",
	.longForm    = "--backcolor",
	.description = "backlight color-name",
	.type        = DOString,
	.value       = { .string = &config.backColor }
    },
    [OPT_INDEX_OFF_COLOR] =
    {
	.shortForm   = "-oc",
	.longForm    = "--offcolor",
	.description = "off-light color-name",
	.type        = DOString,
	.value       = { .string = &config.offlightColor }
    },
    [OPT_INDEX_BACKGROUND] =
    {
	.shortForm   = "-bg",
	.longForm    = "--background",
	.description = "frame-background for non-shaped window",
	.type        = DOString,
	.value       = { .string = &config.backgroundColor }
    },
    [OPT_INDEX_NO_SHAPE] =
    {
	.shortForm   = "-ns",
	.longForm    = "--noshape",
	.description = "make the dockapp non-shaped (combine with -w)",
    },
    [OPT_INDEX_NEW] =
    {
	.shortForm   = "-n",
	.longForm    = "--new",
	.description = "forces wmail to show new mail exclusively",
    },
    [OPT_INDEX_MAILBOX] =
    {
	.shortForm   = "-mb",
	.longForm    = "--mailbox",
	.description = "specify another mailbox ($MAIL is default)",
	.type        = DOString,
	.value       = { .string = &config.mailBox }
    },
    [OPT_INDEX_EXECUTE] =
    {
	.shortForm   = "-e",
	.longForm    = "--execute",
	.description = "command to execute when receiving a new mail",
	.type        = DOString,
	.value       = { .string = &config.cmdOnMail }
    },
    [OPT_INDEX_STATUS_FIELD] =
    {
	.shortForm   = "-sf",
	.longForm    = "--statusfield",
	.description = "consider the status-field of the mail header to distinguish unread mails",
    },
    [OPT_INDEX_READ_STATUS] =
    {
	.shortForm   = "-rs",
	.longForm    = "--readstatus",
	.description = "status field content that your client uses to mark read mails",
	.type        = DOString,
	.value       = { .string = &config.readStatus }
    },
    [OPT_INDEX_TICKER_FONT] =
    {
	.shortForm   = "-fn",
	.longForm    = "--tickerfont",
	.description = "use specified X11 font to draw the ticker",
	.type        = DOString,
	.value       = { .string = &config.useX11Font }
    },
    [OPT_INDEX_CONFIG_FILE] =
    {
	.shortForm   = "-rc",
	.longForm    = "--rcfile",
	.description = "specify another rc-file ($HOME/.wmailrc is default)",
	.type        = DOString,
	.value       = { .string = &configFile }
    }
};


///////////////////////////////////////////////////////////////////////////////
// prototypes

static int  PreparePixmaps( bool freeMem );
static void ExitHandler( int sig );
static void TimedOut( void );
static void CheckTimeOut( bool force );
static void CheckMBox( void );
static void CheckMaildir( void );
static int TraverseDirectory( const char *name, bool isNewMail );
static name_t *GetMail( unsigned long checksum );
static void UpdatePixmap( bool flashMailSymbol );
static void ParseMBoxFile( struct stat *fileStat );
static void ParseMaildirFile( const char *fileName, unsigned long checksum,
			      struct stat *fileStat, bool isNewMail );
static char *ParseFromField( char *buf );
static bool SkipSender( char *address );
static int  InsertName( char *name, unsigned long checksum, flag_t flag );
static void RemoveLastName( void );
static void ClearAllNames( void );
static void DrawTickerX11Font( void );
static void DrawTickerBuildinFont( void );
static void ButtonPressed( int button, int state, int x, int y );
static void ButtonReleased( int button, int state, int x, int y );
static int  XpmColorLine( const char *colorName, char **colorLine,
			   bool disposeLine );
static void ReadChecksumFile( void );
static void WriteChecksumFile( bool writeAll );
static void UpdateChecksum( unsigned long *checksum, const char *buf );
static void RemoveChecksumFile( void );
static void SetMailFlags( flag_t flag );
static void MarkName( unsigned long checksum );
static void DetermineState( void );
static void UpdateConfiguration( void );
static void CleanupNames( void );
static bool HasTickerWork( void );


///////////////////////////////////////////////////////////////////////////////
// implementation


int main( int argc, char **argv )
{
    char *usersHome = getenv( "HOME" );
    struct sigaction sa = { .sa_handler = ExitHandler };
    struct stat fileStat;
    XTextProperty windowName;
    char *name = argv[0];
    DACallbacks callbacks = { NULL, &ButtonPressed, &ButtonReleased,
			      NULL, NULL, NULL, &TimedOut };

    // parse cmdline-args
    DAParseArguments( argc, argv, options, sizeof options / sizeof *options,
		      PACKAGE_NAME, PACKAGE_STRING );

    if( options[OPT_INDEX_DISPLAY].used )
	config.givenOptions |= CL_DISPLAY;
    if( options[OPT_INDEX_COMMAND].used )
	config.givenOptions |= CL_RUNCMD;
    if( options[OPT_INDEX_INTERVAL].used )
	config.givenOptions |= CL_CHECKINTERVAL;
    if( options[OPT_INDEX_FAMILY_NAME].used )
    {
	config.givenOptions |= CL_TICKERMODE;
	config.tickerMode = TICKER_FAMILYNAME;
    }
    if( options[OPT_INDEX_FRAMES].used )
	config.givenOptions |= CL_FPS;
    if( options[OPT_INDEX_SHORT_NAME].used )
    {
	config.givenOptions |= CL_TICKERMODE;
	config.tickerMode = TICKER_NICKNAME;
    }
    if( options[OPT_INDEX_SYMBOL_COLOR].used )
	config.givenOptions |= CL_SYMBOLCOLOR;
    if( options[OPT_INDEX_FONT_COLOR].used )
	config.givenOptions |= CL_FONTCOLOR;
    if( options[OPT_INDEX_BACK_COLOR].used )
	config.givenOptions |= CL_BACKCOLOR;
    if( options[OPT_INDEX_OFF_COLOR].used )
	config.givenOptions |= CL_OFFLIGHTCOLOR;
    if( options[OPT_INDEX_BACK_COLOR].used )
	config.givenOptions |= CL_BACKGROUNDCOLOR;
    if( options[OPT_INDEX_NO_SHAPE].used )
    {
	config.givenOptions |= CL_NOSHAPE;
	config.noshape = true;
    }
    if( options[OPT_INDEX_NEW].used )
    {
	config.givenOptions |= CL_NEWMAILONLY;
	config.newMailsOnly = true;
    }
    if( options[OPT_INDEX_MAILBOX].used )
	config.givenOptions |= CL_MAILBOX;
    if( options[OPT_INDEX_EXECUTE].used )
	config.givenOptions |= CL_CMDONMAIL;
    if( options[OPT_INDEX_STATUS_FIELD].used )
    {
	config.givenOptions |= CL_CONSIDERSTATUSFIELD;
	config.considerStatusField = true;
    }
    if( options[OPT_INDEX_READ_STATUS].used )
	config.givenOptions |= CL_READSTATUS;
    if( options[OPT_INDEX_TICKER_FONT].used )
	config.givenOptions |= CL_USEX11FONT;

    if( configFile == NULL)
    {
	if( usersHome == NULL)
	{
	    WARNING( "HOME environment-variable is not set, looking for %s in current directory!\n",
		     WMAIL_RC_FILE );
	    configFile = strdup( WMAIL_RC_FILE );
	}
	else
	    configFile = MakePathName( usersHome, WMAIL_RC_FILE );

	if( configFile == NULL )
	{
	    WARNING( "Cannot allocate config file-name.\n");
	    exit( EXIT_FAILURE );
	}

    }

    TRACE( "%s: configFile = %s\n", __func__, configFile );

    // read the config file
    ReadConfigFile( configFile, false );

    if( config.checksumFileName == NULL )
    {
	if( usersHome == NULL )
	{
	    WARNING( "HOME environment-variable is not set, placing %s in current directory!\n",
		     WMAIL_CHECKSUM_FILE );
	    config.checksumFileName = strdup( WMAIL_CHECKSUM_FILE );
	}
	else
	    config.checksumFileName = MakePathName( usersHome, WMAIL_CHECKSUM_FILE );

	if( config.checksumFileName == NULL )
	{
	    WARNING( "Cannot allocate checksum file-name.\n");
	    exit( EXIT_FAILURE );
	}
    }

    TRACE( "using checksum-file \"%s\"\n", config.checksumFileName );

    if( config.mailBox == NULL )
	ABORT( "no mailbox specified - please define at least your $MAIL environment-variable!\n" );
    else if( stat( config.mailBox, &fileStat ) == 0 )
	isMaildir = S_ISDIR( fileStat.st_mode ) != 0;

    TRACE( "mailbox is of type %s\n", isMaildir ? "maildir" : "mbox" );

    // dockapp size hard wired - sorry...
    DAInitialize( config.display, "wmail", 64, 64, argc, argv );

    outPixmap = DAMakePixmap();
    if( PreparePixmaps( false ) < 0 )
    {
	WARNING( "Cannot allocate color.\n" );
	exit( EXIT_FAILURE );
    }

    if( sigaction( SIGINT, &sa, NULL ) == -1 )
    {
	perror( "wmail error: sigaction" );
	exit( EXIT_FAILURE );
    }

    if( sigaction( SIGTERM, &sa, NULL ) == -1 )
    {
	perror( "wmail error: sigaction" );
	exit( EXIT_FAILURE );
    }

    DASetCallbacks( &callbacks );
    DASetTimeout( 1000 / config.fps );

    XStringListToTextProperty( &name, 1, &windowName );
    XSetWMName( DADisplay, DAWindow, &windowName );

    UpdatePixmap( false );
    DAShow();

    DAEventLoop();

    return 0;
}

static int PreparePixmaps( bool freeMem )
{
    // simple recoloring of the raw xpms befor creating Pixmaps of them
    // this works as long as you don't "touch" the images...

    bool freeSymColor = freeMem && ( config.colorsUsed & SYM_COLOR );
    bool freeFntColor = freeMem && ( config.colorsUsed & FNT_COLOR );
    bool freeBckColor = freeMem && ( config.colorsUsed & BCK_COLOR );
    bool freeOffColor = freeMem && ( config.colorsUsed & OFF_COLOR );
    bool freeBgrColor = freeMem && ( config.colorsUsed & BGR_COLOR );

#if DA_VERSION < 20030126
    unsigned dummy;
#else
    unsigned short dummy;
#endif

    XGCValues values;

    /*
     * Symbol color?
     */
    if( config.symbolColor != NULL )
    {
	if( XpmColorLine( config.symbolColor, &symbols_xpm[2], freeSymColor) < 0 )
	    return -1;

	config.colorsUsed |= SYM_COLOR;
    }
    else
    {
	if( XpmColorLine( "#20B2AA", &symbols_xpm[2], freeSymColor) < 0 )
	    return -1;

	config.colorsUsed |= SYM_COLOR;
    }

    /*
     * Font color?
     */
    if( config.fontColor != NULL )
    {
	if( XpmColorLine( config.fontColor, &chars_xpm[3], freeFntColor) < 0 )
	    return -1;

	if( XpmColorLine( config.fontColor, &numbers_xpm[3], freeFntColor) < 0 )
	    return -1;

	config.colorsUsed |= FNT_COLOR;
    }
    else
    {
	if( XpmColorLine( "#D3D3D3", &chars_xpm[3], freeFntColor) < 0 )
	    return -1;

	if( XpmColorLine( "#D3D3D3", &numbers_xpm[3], freeFntColor) < 0 )
	    return -1;

	config.colorsUsed |= FNT_COLOR;
    }

    /*
     * Backlight color?
     */
    if( config.backColor != NULL )
    {
	if( XpmColorLine( config.backColor, &main_xpm[3], freeBckColor) < 0 )
	    return -1;

	if( XpmColorLine( config.backColor, &symbols_xpm[3], freeBckColor) < 0 )
	    return -1;

	if( XpmColorLine( config.backColor, &chars_xpm[2], freeBckColor) < 0 )
	    return -1;

	if( XpmColorLine( config.backColor, &numbers_xpm[2], freeBckColor) < 0 )
	    return -1;

	config.colorsUsed |= BCK_COLOR;
    }
    else
    {
	if( XpmColorLine( "#282828", &main_xpm[3], freeBckColor) < 0 )
	    return -1;

	if( XpmColorLine( "#282828", &symbols_xpm[3], freeBckColor) < 0 )
	    return -1;

	if( XpmColorLine( "#282828", &chars_xpm[2], freeBckColor) < 0 )
	    return -1;

	if( XpmColorLine( "#282828", &numbers_xpm[2], freeBckColor) < 0 )
	    return -1;

	config.colorsUsed |= BCK_COLOR;
    }

    /*
     * Off-light color?
     */
    if( config.offlightColor != NULL )
    {
	if( XpmColorLine( config.offlightColor, &main_xpm[2], freeOffColor) < 0 )
	    return -1;

	if( XpmColorLine( config.offlightColor, &numbers_xpm[4], freeOffColor) < 0 )
	    return -1;

	config.colorsUsed |= OFF_COLOR;
    }
    else
    {
	if( XpmColorLine( "#000000", &main_xpm[2], freeOffColor) < 0 )
	    return -1;

	if( XpmColorLine( "#000000", &numbers_xpm[4], freeOffColor) < 0 )
	    return -1;

	config.colorsUsed |= OFF_COLOR;
    }

    /*
     * Window-frame background (only seen if nonshaped)?
     */
    if( config.backgroundColor != NULL )
    {
	if( XpmColorLine( config.backgroundColor, &main_xpm[1], freeBgrColor) < 0 )
	    return -1;

	config.colorsUsed |= BGR_COLOR;
    }

    if( freeMem )
    {
	XFreePixmap( DADisplay, mainPixmap );
	XFreePixmap( DADisplay, mainPixmap_mask );
	XFreePixmap( DADisplay, symbolsPixmap );
	XFreePixmap( DADisplay, charsPixmap );
	XFreePixmap( DADisplay, numbersPixmap );
	XFreePixmap( DADisplay, buttonPixmap );

	if( tickerGC != NULL )
	{
	    XFreeGC( DADisplay, tickerGC );
	    tickerGC = NULL;
	    if( tickerFS != NULL )
	    {
		XFreeFont( DADisplay, tickerFS );
		tickerFS = NULL;
	    }
	}
    }

    DAMakePixmapFromData( main_xpm, &mainPixmap, &mainPixmap_mask, &dummy,  &dummy );
    DAMakePixmapFromData( symbols_xpm, &symbolsPixmap, NULL, &dummy, &dummy );
    DAMakePixmapFromData( chars_xpm, &charsPixmap, NULL, &dummy, &dummy );
    DAMakePixmapFromData( numbers_xpm, &numbersPixmap, NULL, &dummy, &dummy );
    DAMakePixmapFromData( button_xpm, &buttonPixmap, NULL, &dummy, &dummy );

    if( config.useX11Font != NULL )
    {
	XRectangle clipRect;

	if( config.fontColor != NULL )
	    values.foreground = DAGetColor( config.fontColor );
	else
	    values.foreground = DAGetColor( "#D3D3D3" );

	tickerFS = XLoadQueryFont( DADisplay, config.useX11Font );
	if( tickerFS == NULL )
	    ABORT( "Cannot load font \"%s\"", config.useX11Font );

	values.font = tickerFS->fid;
	tickerGC = XCreateGC( DADisplay, DAWindow, GCForeground | GCFont,
			      &values );
	clipRect.x = 6;
	clipRect.y = 19;
	clipRect.width = 52;
	clipRect.height = 23;

	XSetClipRectangles( DADisplay, tickerGC, 0, 0, &clipRect, 1, Unsorted );
    }

    if( config.noshape ) // non-shaped dockapp ?
	DASetShape( None );
    else
	DASetShape( mainPixmap_mask );

    return 0;
}

static void MarkName( unsigned long checksum )
{
    name_t *name;

    for( name = names; name != NULL; name = name->next )
    {
	if( name->checksum == checksum )
	{
	    name->flag |= FLAG_READ;
	    if( config.newMailsOnly )
		numMails--;
	    break;
	}
    }
}

static void DetermineState( void )
{
    name_t *name;

    for( name = names; name != NULL; name = name->next )
	if(!( name->flag & FLAG_READ ))
	{
	    state = STATE_NEWMAIL;

	    if( config.cmdOnMail != NULL )
	    {
		int ret = system( config.cmdOnMail );

		if( ret == 127 || ret == -1 )
		    WARNING( "execution of command \"%s\" failed.\n", config.cmdOnMail );
	    }

	    break;
	}
}

static void ReadChecksumFile( void )
{
    FILE *f = fopen( config.checksumFileName, "rb" );
    if( f != NULL )
	while( !feof( f ))
	{
	    unsigned long checksum;
	    if( fread( &checksum, sizeof(long), 1, f ) != 1 )
		continue;

	    MarkName( checksum );
	}
    else
	return;

    fclose( f );
}

static void WriteChecksumFile( bool writeAll )
{
    FILE *f;
    TRACE( "writing checksums:" );

    if(( f = fopen( config.checksumFileName, "wb" )) != NULL )
    {
	name_t *name;
	for( name = names; name != NULL; name = name->next )
	{
	    if( writeAll || (name->flag & FLAG_READ))
	    {
		fwrite( &name->checksum, sizeof(long), 1, f );
		TRACE( " %X", name->checksum );
	    }
	}
    }
    else
	return;

    TRACE( "\n" );

    fclose( f );
}

static void UpdateChecksum( unsigned long *checksum, const char *buf )
{
    if( buf != NULL )
    {
	size_t i, len = strlen( buf );

	for( i = 0; i < len; ++i )
	    *checksum += buf[i] << (( i % sizeof(long) ) * 8 );
    }
}

static void RemoveChecksumFile( void )
{
    TRACE( "removing checksum-file\n" );
    remove( config.checksumFileName );
}

static void ExitHandler( int sig )
{
    (void) sig;
    caughtSig = 1;
}

static void TimedOut( void )
{
    if( caughtSig )
    {
	ClearAllNames();
	ResetConfigStrings();
	if( !options[OPT_INDEX_CONFIG_FILE].used )
	    free( configFile );
	exit( EXIT_SUCCESS );
    }

    CheckTimeOut( true );
}

static void CheckTimeOut( bool force )
{
    static int checkMail = 0;

    struct timeval now;
    gettimeofday(&now, NULL);

    unsigned long nowMs = now.tv_sec * 1000UL + now.tv_usec / 1000UL;

    if( !force && nowMs - lastTimeOut < 1000UL / config.fps )
	return;

    lastTimeOut = nowMs;

    if( readConfigFile )
    {
	readConfigFile = false;
	UpdateConfiguration();
	checkMail = 0;
	forceRead = true;
    }

    if( checkMail == 0 )
    {
	TRACE( "checking for new mail...\n" );

	if( isMaildir )
	    CheckMaildir();
	else
	    CheckMBox();
    }

    UpdatePixmap( checkMail % config.fps < config.fps/2 );

    if( ++checkMail >= config.fps * config.checkInterval )
	checkMail = 0;
}

static void CheckMBox( void )
{
    struct stat fileStat;

    if( stat( config.mailBox, &fileStat ) == -1 || fileStat.st_size == 0 )
    {
	/*
	 * Error retrieving file-stats or file is empty -> no new/read mails
	 * available.
	 */
	if( state != STATE_NOMAIL )
	{
	    state = STATE_NOMAIL;
	    ClearAllNames();
	    RemoveChecksumFile();
	    forceRedraw = true;
	}
    }
    else
    {
	if( lastModifySeconds != fileStat.st_mtime || forceRead )
	{
	    /*
	     * File has been updated -> new mails arrived or some mails removed
	     */
	    forceRead = false;
	    ParseMBoxFile( &fileStat );
	    stat( config.mailBox, &fileStat );
	    forceRedraw = true;
	}
	else if( lastAccessSeconds != fileStat.st_atime )
	{
	    /*
	     * File has been accessed (read) -> mark all mails as "read", because it
	     * cannot be decided which mails the user has read...
	     */
	    state = STATE_READMAIL;
	    WriteChecksumFile( true );
	    if( config.newMailsOnly )
	    {
		numMails = 0;
		SetMailFlags( FLAG_READ );
	    }
	    forceRedraw = true;
	}

	lastModifySeconds = fileStat.st_mtime;
	lastAccessSeconds = fileStat.st_atime;
    }
}

static void CheckMaildir( void )
{
    DIR *dir = NULL;
    mail_state_t lastState = state;
    unsigned lastMailCount = numMails;

    if( forceRead )
    {
	forceRead = false;
	ClearAllNames();
	TRACE( "all names cleared\n" );
    }

    state = STATE_NOMAIL;

    if(( dir = opendir( config.mailBox )) != NULL )
    {
	struct dirent *dirEnt = NULL;
	name_t *name;

	for( name = names; name != NULL; name = name->next )
	    name->visited = false;

	while(( dirEnt = readdir( dir )) != NULL )
	{
	    char *fullName = MakePathName( config.mailBox, dirEnt->d_name );
	    struct stat fileStat;

	    if( fullName == NULL )
	    {
		WARNING( "Cannot allocate file/path\n" );
		break;
	    }

	    if( stat( fullName, &fileStat ) == -1 )
		WARNING( "Can't stat file/path \"%s\"\n", fullName );
	    else if( S_ISDIR( fileStat.st_mode ))
	    {
		if( strcmp( dirEnt->d_name, "new" ) == 0 )
		{
		    if( TraverseDirectory( fullName, true ) > 0 )
			state = STATE_NEWMAIL;
		}
		else if( strcmp( dirEnt->d_name, "cur" ) == 0 )
		{
		    if( TraverseDirectory( fullName, false ) > 0 )
			if( state != STATE_NEWMAIL )
			    state = STATE_READMAIL;
		}
		// directories ".", ".." and "tmp" discarded
	    }
	    free( fullName );
	}

	closedir( dir );
	CleanupNames();
    }
    else
	WARNING( "can't open directory \"%s\"\n", config.mailBox );

    if( lastState != state || lastMailCount != numMails )
	forceRedraw = true;
}

static int TraverseDirectory( const char *name, bool isNewMail )
{
    DIR *dir = NULL;
    int mails = 0;

    if(( dir = opendir( name )) != NULL )
    {
	struct dirent *dirEnt = NULL;

	while(( dirEnt = readdir( dir )) != NULL )
	{
	    char *fullName = MakePathName( name, dirEnt->d_name );
	    struct stat fileStat;
	    unsigned long checksum = 0;
	    name_t *name;

	    if( fullName == NULL )
	    {
		WARNING( "Cannot allocate file/path\n" );
		break;
	    }

	    if( stat( fullName, &fileStat ) == -1 )
		WARNING( "Can't stat file/path \"%s\"\n", fullName );
	    else if( !S_ISDIR( fileStat.st_mode ))
	    {
		TRACE( "found email-file \"%s\"\n", fullName );
		UpdateChecksum( &checksum, dirEnt->d_name );

		if(( name = GetMail( checksum )) == NULL )
		{
		    TRACE( "-> new file - parsing it\n" );
		    ParseMaildirFile( fullName, checksum, &fileStat, isNewMail );
		}
		else
		{
		    name->flag = isNewMail ? FLAG_INITIAL : FLAG_READ;
		    name->visited = true;
		}
		++mails;
	    }
	    free( fullName );
	}

	closedir( dir );
    }

    return mails;
}

static name_t *GetMail( unsigned long checksum )
{
    name_t *name;

    for( name = names; name != NULL; name = name->next )
	if( name->checksum == checksum )
	    return name;

    return NULL;
}

static void UpdatePixmap( bool flashMailSymbol )
{
    int drawCount, i;

    if( !forceRedraw && !HasTickerWork() )
	return;

    forceRedraw = false;

    XCopyArea( DADisplay, mainPixmap, outPixmap, DAGC,
	       0, 0, 64, 64, 0, 0 );

    if( numMails > 999 )
    {
	XCopyArea( DADisplay, numbersPixmap, outPixmap, DAGC,
		   50, 0, 5, 9, 6, 49 );
	drawCount = 999;
    }
    else
	drawCount = numMails;

    for( i = 0; i < 3; ++i, drawCount /= 10 )
    {
	XCopyArea( DADisplay, numbersPixmap, outPixmap, DAGC,
		   (drawCount%10)*5, 0, 5, 9, 24-i*6, 49 );
	if( drawCount <= 9 )
	    break;
    }

    if( buttonPressed )
	XCopyArea( DADisplay, buttonPixmap, outPixmap, DAGC,
		   0, 0, 23, 11, 36, 48 );

    switch( state )
    {
    case STATE_NEWMAIL:
	if( flashMailSymbol )
	    XCopyArea( DADisplay, symbolsPixmap, outPixmap, DAGC,
		       13, 0, 37, 12, 20, 7 );
	/* fall through */
    case STATE_READMAIL:
	XCopyArea( DADisplay, symbolsPixmap, outPixmap, DAGC,
		   0, 0, 13, 12, 7, 7 );

	if( config.useX11Font == NULL )
	    DrawTickerBuildinFont();
	else
	    DrawTickerX11Font();
	break;
    default: // make compiler happy
	break;
    }

    DASetPixmap( outPixmap );
}

static void ParseMBoxFile( struct stat *fileStat )
{
    char buf[1024];
    struct utimbuf timeStruct;
    int fromFound = 0;
    FILE *f = fopen( config.mailBox, "r" );
    unsigned long checksum;

    state = STATE_READMAIL;
    ClearAllNames();

    numMails = 0;

    if( f == NULL )
    {
	WARNING( "can't open mbox \"%s\"\n", config.mailBox );
	return;
    }

    while( fgets( buf, sizeof buf, f ) != NULL )
    {
	if( PREFIX_MATCHES( buf, "From ", true ))
	{
	    fromFound = 1;
	    checksum = 0;
	    continue;
	}

	if( fromFound )
	    UpdateChecksum( &checksum, buf );

	if( fromFound && PREFIX_MATCHES( buf, "From:", false ))
	{
	    char *addr = buf + sizeof "From:";

	    if( SkipSender( addr ))
		continue;

	    char *name;
	    if(( name = ParseFromField( addr )) == NULL )
	    {
		WARNING( "Could not parse From field\n" );
		break;
	    }
	    if ( InsertName( name, checksum, FLAG_INITIAL ) < 0 )
	    {
		WARNING( "Could not allocate name\n" );
		break;
	    }

	    ++numMails;
	    fromFound = 0;
	    checksum = 0;
	}
	else if( config.considerStatusField &&
		   PREFIX_MATCHES( buf, "Status:", false ) &&
		   strstr( buf + sizeof "Status:", config.readStatus ) == NULL )
	{
	    RemoveLastName();
	    --numMails;
	}
    }

    fclose( f );
    ReadChecksumFile();

    DetermineState();

    timeStruct.actime = fileStat->st_atime;
    timeStruct.modtime = fileStat->st_mtime;
    utime( config.mailBox, &timeStruct );
}

static void ParseMaildirFile( const char *fileName, unsigned long checksum,
			      struct stat *fileStat, bool isNewMail )
{
    char buf[1024];
    struct utimbuf timeStruct;
    FILE *f = fopen( fileName, "r" );

    if( f == NULL )
    {
	WARNING( "can't open maildir file \"%s\"\n", fileName );
	return;
    }

    while( fgets( buf, sizeof buf, f ) != NULL )
    {
	if( PREFIX_MATCHES( buf, "From:", false ))
	{
	    char *addr = buf + sizeof "From:";

	    if( SkipSender( addr ))
		break;

	    char *name;
	    if(( name = ParseFromField( addr )) == NULL )
	    {
		WARNING( "Could not parse From field\n" );
		break;
	    }
	    if ( InsertName( name, checksum,
			     isNewMail ? FLAG_INITIAL : FLAG_READ ) < 0 )
	    {
		WARNING( "Could not allocate name\n" );
		break;
	    }

	    //++numMails;
	}
    }

    fclose( f );

    timeStruct.actime = fileStat->st_atime;
    timeStruct.modtime = fileStat->st_mtime;
    utime( fileName, &timeStruct );
}

static char *ParseFromField( char *buf )
{
    parse_state_t state = STATE_FULLNAME;
    int fullNameEncoded = 0;
    int saveAtCharPos = -1;
    char *fullName;
    char *addressName;
    char *atChar = NULL;
    char *c;
    size_t maxLen = strlen( buf ) + 1;
    char *comment;
    size_t fullNameLen = 0, addressNameLen = 0, commentLen = 0;

    if(( fullName = calloc( maxLen, sizeof *fullName )) == NULL )
	return NULL;
    if(( addressName = calloc( maxLen, sizeof *addressName )) == NULL )
    {
	free( fullName );
	return NULL;
    }
    if(( comment = calloc( maxLen, sizeof *comment )) == NULL )
    {
	free( fullName );
	free( addressName );
	return NULL;
    }

    // FIXME: Don't do that "encoded" dance.  It's not intended by
    // RFC2047, and it's better to just do it in the end.
    // Cleaner.

    for( c = buf; *c != '\0'; ++c )
    {
	switch( state )
	{
	case STATE_FULLNAME:

	    switch( *c )
	    {
	    case '"':
		state = STATE_QUOTED_FULLNAME;
		continue;
	    case '<':
		while( fullNameLen > 0 &&
		       isspace( fullName[ fullNameLen - 1 ] ))
		    fullNameLen--;
		fullName[ fullNameLen ] = '\0';
		state = STATE_ADDRESS;
		continue;
	    case '@':
		saveAtCharPos = fullNameLen;
		fullName[ fullNameLen++ ] = *c;
		continue;
	    case '(':
		state = STATE_COMMENT;
		continue;
	    case '=':
		if( *(c+1) == '?' )
		{
		    ++c;
		    fullNameEncoded = 1;
		    state = STATE_ENCODED_FULLNAME;
		    continue;
		}
		/* else fall through */
	    default:
		if( fullName[0] != '\0' || !isspace( *c ))
		    fullName[ fullNameLen++ ] = *c;
	    }
	    continue;

	case STATE_QUOTED_FULLNAME:

	    switch( *c )
	    {
	    case '\\':
		fullName[ fullNameLen++ ] = *(++c);
		continue;
	    case '"':
		state = STATE_FULLNAME;
		continue;
	    default:
		fullName[ fullNameLen++ ] = *c;
	    }
	    continue;

	case STATE_ENCODED_FULLNAME:

	    switch( *c )
	    {
	    case '?':
		if( *(c+1) == '=' )
		{
		    ++c;
		    state = STATE_FULLNAME;
		    continue;
		}
	    default:
		; // do nothing... COMING SOON: decode at least latin1
	    }
	    continue;

	case STATE_ADDRESS:

	    switch( *c )
	    {
	    case '"':
		state = STATE_QUOTED_ADDRESS;
	    case '>':
	    case '\n':
		// FIXME: Shouldn't it break here?
		// Since the address is finished?
		continue;
	    case '@':
		atChar = addressName + addressNameLen;
		addressName[ addressNameLen++ ] = *c;
		continue;
	    default:
		addressName[ addressNameLen++ ] = *c;
	    }
	    continue;

	case STATE_QUOTED_ADDRESS:

	    switch( *c )
	    {
	    case '"':
		state = STATE_ADDRESS;
		continue;
	    case '\\':
		addressName[ addressNameLen++ ] = *(++c);
		continue;
	    default:
		addressName[ addressNameLen++ ] = *c;
	    }
	    continue;
	case STATE_COMMENT:
	    switch( *c )
	    {
	    case ')':
		    state = STATE_FULLNAME;
		    continue;
	    default:
		    comment[ commentLen++ ] = *c;
	    }
	    continue;
	}
    }

    if( *comment )
    {
	//WARNING("Comment seen: %s\nIn: %s\nFullname: %s\nAddress: %s\n", comment, buf, fullName, addressName);
	// Comment seen: if there's an address, append to
	// fullname.  If no address, copy fullname to address
	// and comment to fullname.
	if( *addressName )
	{
	    strcat(fullName, "(");
	    strcat(fullName, comment);
	    strcat(fullName, ")");
	}
	else
	{
	    strcpy(addressName, fullName);
	    strcpy(fullName, comment);
	}
    }
    free( comment );

    //WARNING("Fullname: %s\nAddress: %s\n", fullName, addressName);

    // what name should be tickered
    if( config.tickerMode == TICKER_FAMILYNAME && fullName[0] != '\0' && !fullNameEncoded )
    {
	free( addressName );
	return fullName;
    }
    else
    {
	if( state == STATE_FULLNAME )
	{
	    strcpy( addressName, fullName );
	    if( saveAtCharPos != -1 )
		atChar = &addressName[saveAtCharPos];
	}
	if( config.tickerMode == TICKER_NICKNAME )
	{
	    if( atChar != NULL )
		*atChar = '\0';
	}
	free( fullName );
	return addressName;
    }
}

static bool SkipSender( char *address )
{
    char **skipName;
    size_t len = strlen( address );

    // remove trailing '\n' got from fgets
    if( address[len - 1] == '\n' )
	address[len - 1] = '\0';

    while( isspace( *address ))
	address++;

    for( skipName = config.skipNames;
	 skipName != NULL && *skipName != NULL; skipName++ )
    {
	TRACE( "comparing \"%s\" and \"%s\"\n", *skipName, address );

	// call libc-fnmatch (wildcard-match :-) !
	if( !fnmatch( *skipName, address, 0 ))
	{
	    TRACE( "skipping sender \"%s\"\n", *skipName );
	    return true;
	}
    }

    return false;
}

static int InsertName( char *name, unsigned long checksum, flag_t flag )
{
    name_t *item;

    TRACE( "insertName: %X, \"%s\"\n", checksum, name );
    if(( item = malloc( sizeof( name_t ))) == NULL )
    {
	free( name );
	return -1;
    }

    item->name = name;
    item->checksum = checksum;
    item->flag = flag;
    item->visited = true;
    item->next = names;
    names = item;

    namesChanged = true;
    return 0;
}

static void RemoveLastName( void )
{
    if( names != NULL )
    {
	name_t *name = names;
	names = names->next;
	free( name->name );
	free( name );
    }
}

static void ClearAllNames( void )
{
    name_t *name, *nextName;

    for( name = names; name != NULL; name = nextName )
    {
	nextName = name->next;

	free( name->name );
	free( name );
    }

    names = NULL;
    numMails = 0;

    namesChanged = true;
}

static void SetMailFlags( flag_t flag )
{
    name_t *name;

    for( name = names; name != NULL; name = name->next )
	name->flag |= flag;
}

static void DrawTickerX11Font( void )
{
    // 49 x 21 + 7 + 20 out-drawable size

    static int insertAt;

    if( curTickerName == NULL || namesChanged )
    {
	for( curTickerName = names;
	     curTickerName != NULL && config.newMailsOnly &&
		 ( curTickerName->flag & FLAG_READ );
	     curTickerName = curTickerName->next )
	    ;

	if( curTickerName == NULL )
	    return;

	namesChanged = false;
	insertAt = 54;
    }

    XDrawString( DADisplay, outPixmap, tickerGC, insertAt,
		 41-tickerFS->max_bounds.descent,
		 curTickerName->name, strlen( curTickerName->name ));

    --insertAt;

    if( insertAt < -XTextWidth( tickerFS, curTickerName->name,
				strlen( curTickerName->name )) + 6 )
    {
	do
	    curTickerName = curTickerName->next;
	while( curTickerName != NULL && config.newMailsOnly &&
		   ( curTickerName->flag & FLAG_READ ));

	if( curTickerName != NULL )
	    insertAt = 54;
    }
}

static void DrawTickerBuildinFont( void )
{
    // 49 x 21 + 7 + 20 out-drawable size
    // 14 x 21 font-character size

    static unsigned insertAt;
    static unsigned takeItFrom;

    unsigned leftSpace;
    unsigned drawTo;
    unsigned char *currentChar;

    if( names == NULL )
	return;

    if( curTickerName == NULL || namesChanged )
    {
	for( curTickerName = names;
	     curTickerName != NULL && config.newMailsOnly && ( curTickerName->flag & FLAG_READ );
	     curTickerName = curTickerName->next )
	    ;

	if( curTickerName == NULL )
	    return;

	insertAt = 57;
	takeItFrom = 0;
	namesChanged = false;
    }

    leftSpace = takeItFrom % 14;

    for( currentChar = (unsigned char *)&curTickerName->name[takeItFrom/14],
	 drawTo = insertAt; *currentChar != '\0'; ++currentChar )
    {
	int outChar = (*currentChar < 32 || *currentChar >= 128) ? '?' :
	    *currentChar;
	int charWidth = 57 - drawTo >= 14 ? 14 - leftSpace : 57 - drawTo;

	XCopyArea( DADisplay, charsPixmap, outPixmap, DAGC,
		   (outChar - 32) * 14 + leftSpace, 0, charWidth, 21, drawTo, 20 );

	leftSpace = 0;
	drawTo += charWidth;

	if( drawTo > 57 )
	    break;
    }

    if( --insertAt < 7 )
    {
	insertAt = 7;
	takeItFrom++;

	if( takeItFrom/14 >= strlen( curTickerName->name ))
	{
	    do
		curTickerName = curTickerName->next;
	    while( curTickerName != NULL && config.newMailsOnly &&
		   ( curTickerName->flag & FLAG_READ ));

	    if( curTickerName != NULL )
	    {
		takeItFrom = 0;
		insertAt = 57;
	    }
	}
    }
}

static void ButtonPressed( int button, int state, int x, int y )
{
    (void) button;
    (void) state;

    if( x >= 35 && x <= 59 && y >= 47 && y <= 59 )
    {
	buttonPressed = true;
	forceRedraw = true;
    }
    else
	// reread the config file
	readConfigFile = true;

    CheckTimeOut( false );
}

static void ButtonReleased( int button, int state, int x, int y )
{
    (void) button;
    (void) state;

    buttonPressed = false;
    forceRedraw = true;

    if( x >= 35 && x <= 59 && y >= 47 && y <= 59 )
    {
	int ret = system( config.runCmd );

	if( ret == 127 || ret == -1 )
	    WARNING( "execution of command \"%s\" failed.\n", config.runCmd );
    }

    CheckTimeOut( false );
}

static void GetHexColorString( const char *colorName, char *xpmLine )
{
    XColor color;

    if( XParseColor( DADisplay,
		     DefaultColormap( DADisplay, DefaultScreen( DADisplay )),
		     colorName, &color ))
	sprintf( xpmLine, "%02X%02X%02X", color.red>>8, color.green>>8,
		  color.blue>>8 );
    else
	WARNING( "unknown colorname: \"%s\"\n", colorName );
}

static int XpmColorLine( const char *colorName, char **colorLine,
			   bool disposeLine )
{
    char *newLine, *from;

    newLine = strdup( *colorLine );
    if ( newLine == NULL )
	return -1;

    from = strrchr( newLine, '#' );
    if( from == NULL &&
	strcasecmp( &(*colorLine)[ strlen( *colorLine ) - 4 ], "none" ) == 0 )
    {
	/*
	 * if no # found, it should be a None-color line
	 */
	free( newLine );
	newLine = malloc( 12 );
	if ( newLine != NULL )
	{
	    strcpy( newLine, " \tc #" );
	    newLine[11] = '\0';
	    from = newLine + 4;
	}
    }

    if( newLine == NULL)
	return -1;

    if( disposeLine )
	free( *colorLine );

    GetHexColorString( colorName, from + 1 );

    *colorLine = newLine;
    return 0;
}

static void UpdateConfiguration( void )
{
    struct stat fileStat;

    TRACE( "reading configuration file...\n" );

    ReadConfigFile( configFile, true );

    // if no path/name to an mbox or maildir inbox directory was given,
    // use the environment
    if( config.mailBox == NULL )
	config.mailBox = getenv( "MAIL" );

    // mbox or maildir ?
    if( config.mailBox != NULL && stat( config.mailBox, &fileStat ) == 0 )
	isMaildir = S_ISDIR( fileStat.st_mode ) != 0;
    else
	isMaildir = false;

    TRACE( "mailbox is of type %s\n", isMaildir ? "maildir" : "mbox" );

    if( PreparePixmaps( true ) < 0 )
	WARNING( "Cannot allocate color.\n" );

    DASetTimeout( 1000 / config.fps );
}

static void CleanupNames( void )
{
    name_t *name, *last = NULL, *nextName;

    numMails = 0;

    for( name = names; name != NULL; name = nextName )
    {
	nextName = name->next;

	if( !name->visited )
	{
	    if( last == NULL )
		names = name->next;
	    else
		last->next = name->next;

	    free( name->name );
	    free( name );

	    namesChanged = true;
	}
	else
	{
	    last = name;

	    if( !config.newMailsOnly || (name->flag & FLAG_READ) == 0 )
		++numMails;
	}
    }
}

static bool HasTickerWork( void )
{
    name_t *nextTickerName;

    if( names == NULL )
	return false;

    if( curTickerName == NULL || namesChanged )
    {

	for( nextTickerName = names;
	     nextTickerName != NULL && config.newMailsOnly &&
		( nextTickerName->flag & FLAG_READ );
	     nextTickerName = nextTickerName->next )
	    ;

	if( nextTickerName == NULL )
	    return false;
    }

    return true;
}
