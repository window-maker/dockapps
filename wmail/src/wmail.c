///////////////////////////////////////////////////////////////////////////////
// wmail.c
// email indicator tool designed as docklet for Window Maker
// main c source-file
//
// wmail version 2.0
//
// Copyright 2000~2002, Sven Geisenhainer <sveng@informatik.uni-jena.de>.
// All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <utime.h>
#include <fnmatch.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <X11/Xlib.h>
#include <libdockapp/dockapp.h>
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

typedef enum {
    FLAG_INITIAL = 0,
    FLAG_READ    = 1
} flag_t;

typedef struct _name_t {
    char *name;
    unsigned long checksum;
    flag_t flag;
	bool visited;
    struct _name_t *next;
} name_t;

typedef enum {
    STATE_NOMAIL,
    STATE_NEWMAIL,
    STATE_READMAIL
} mail_state_t;

typedef enum {
    STATE_ADDRESS,
    STATE_QUOTED_ADDRESS,
    STATE_FULLNAME,
    STATE_QUOTED_FULLNAME,
    STATE_ENCODED_FULLNAME,
	STATE_COMMENT
} parse_state_t;


///////////////////////////////////////////////////////////////////////////////
// data

mail_state_t state = STATE_NOMAIL;
int numMails = 0;
bool namesChanged = false;
bool buttonPressed = false;
bool readConfigFile = false;
bool isMaildir = false;
bool forceRead = false;
bool forceRedraw = true;
time_t lastModifySeconds = 0;
time_t lastAccessSeconds = 0;
Pixmap mainPixmap;
Pixmap mainPixmap_mask;
Pixmap symbolsPixmap;
Pixmap charsPixmap;
Pixmap numbersPixmap;
Pixmap buttonPixmap;
Pixmap outPixmap;
GC tickerGC;
XFontStruct *tickerFS = NULL;
name_t *names = NULL;
name_t *curTickerName = NULL;

static DAProgramOption options[] = {
    {"-display", NULL, "display to use", DOString, False, {&config.display}},
    {"-c", "--command", "cmd to run on btn-click (\"xterm -e mail\" is default)",
     DOString, False, {&config.runCmd} },
    {"-i", "--intervall",
     "number of secs between mail-status updates (1 is default)", DONatural,
     False, {&config.checkInterval} },
    {"-f", "--familyname", "tickers the family-name if available", DONone,
     False, {NULL} },
    {"-fps", "--frames", "ticker frames per second", DONatural,
     False, {&config.fps} },
    {"-s", "--shortname", "tickers the nickname (all before the '@')", DONone,
     False, {NULL} },
    {"-sc", "--symbolcolor", "symbol color-name",
     DOString, False, {&config.symbolColor} },
    {"-fc", "--fontcolor", "ticker-font color-name",
     DOString, False, {&config.fontColor} },
    {"-bc", "--backcolor", "backlight color-name",
     DOString, False, {&config.backColor} },
    {"-oc", "--offcolor", "off-light color-name",
     DOString, False, {&config.offlightColor} },
    {"-bg", "--background", "frame-background for non-shaped window",
     DOString, False, {&config.backgroundColor} },
    {"-ns", "--noshape", "make the dockapp non-shaped (combine with -w)",
     DONone, False, {NULL} },
    {"-n", "--new", "forces wmail to show new mail exclusively", DONone, False, {NULL} },
    {"-mb", "--mailbox", "specify another mailbox ($MAIL is default)", DOString, False, {&config.mailBox} },
    {"-e", "--execute", "command to execute when receiving a new mail", DOString, False, {&config.cmdOnMail} },
    {"-sf", "--statusfield", "consider the status-field of the mail header to distinguish unread mails", DONone, False, {NULL} },
	{"-rs", "--readstatus", "status field content that your client uses to mark read mails", DOString, False, {&config.readStatus} },
	{"-fn", "--tickerfont", "use specified X11 font to draw the ticker", DOString, False, {&config.useX11Font} }
};


///////////////////////////////////////////////////////////////////////////////
// prototypes

void PreparePixmaps( bool freeThemFirst );
void TimerHandler( int dummy );
void CheckMBox();
void CheckMaildir();
int TraverseDirectory( const char *name, bool isNewMail );
name_t *GetMail( unsigned long checksum );
void UpdatePixmap( bool flashMailSymbol );
void ParseMBoxFile( struct stat *fileStat );
void ParseMaildirFile( const char *fileName, unsigned long checksum,
					   struct stat *fileStat, bool isNewMail );
char *ParseFromField( char *buf );
bool SkipSender( char *address );
void InsertName( char *name, unsigned long checksum, flag_t flag );
void RemoveLastName();
void ClearAllNames();
void DrawTickerX11Font();
void DrawTickerBuildinFont();
void ButtonPressed( int button, int state, int x, int y );
void ButtonReleased( int button, int state, int x, int y );
char *XpmColorLine( const char *colorName, char *colorLine, bool disposeLine );
void ReadChecksumFile();
void WriteChecksumFile( bool writeAll );
void UpdateChecksum( unsigned long *checksum, const char *buf );
void RemoveChecksumFile();
void SetMailFlags( flag_t flag );
void MarkName( unsigned long checksum );
void DetermineState();
void UpdateConfiguration();
void CleanupNames();
char *FileNameConcat( const char *path, const char *fileName );
bool HasTickerWork();


///////////////////////////////////////////////////////////////////////////////
// implementation


void SetTimer()
{
    struct itimerval timerVal;

    timerVal.it_interval.tv_sec = 0;
    timerVal.it_interval.tv_usec = 1000000/config.fps;
    timerVal.it_value.tv_sec = 0;
    timerVal.it_value.tv_usec = 1000000/config.fps;

    setitimer( ITIMER_REAL, &timerVal, NULL );
}

int main( int argc, char **argv )
{
    char *usersHome;
    struct sigaction sa;
    int ret;
	struct stat fileStat;
	XTextProperty windowName;
	char *name = argv[0];
    DACallbacks callbacks = { NULL, &ButtonPressed, &ButtonReleased,
							  NULL, NULL, NULL, NULL };

    // read the config file and overide the default-settings
    ReadConfigFile( false );

    if( config.checksumFileName == NULL ) {
		if(( usersHome = getenv( "HOME" )) == NULL ) {
			WARNING( "HOME environment-variable is not set, placing %s in current directory!\n", WMAIL_CHECKSUM_FILE );
			config.checksumFileName = WMAIL_CHECKSUM_FILE;
		} else
			config.checksumFileName = MakePathName( usersHome, WMAIL_CHECKSUM_FILE );
    }

    TRACE( "using checksum-file \"%s\"\n", config.checksumFileName );

    // parse cmdline-args and overide defaults and cfg-file settings
    DAParseArguments( argc, argv, options,
					  sizeof(options) / sizeof(DAProgramOption),
					  WMAIL_NAME, WMAIL_VERSION );

	if( options[0].used )
		config.givenOptions |= CL_DISPLAY;
	if( options[1].used )
		config.givenOptions |= CL_RUNCMD;
	if( options[2].used )
		config.givenOptions |= CL_CHECKINTERVAL;
	if( options[3].used ) {
		config.givenOptions |= CL_TICKERMODE;
		config.tickerMode = TICKER_FAMILYNAME;
	}
	if( options[4].used )
		config.givenOptions |= CL_FPS;
	if( options[5].used ) {
		config.givenOptions |= CL_TICKERMODE;
		config.tickerMode = TICKER_NICKNAME;
	}
	if( options[6].used )
		config.givenOptions |= CL_SYMBOLCOLOR;
	if( options[7].used )
		config.givenOptions |= CL_FONTCOLOR;
	if( options[8].used )
		config.givenOptions |= CL_BACKCOLOR;
	if( options[9].used )
		config.givenOptions |= CL_OFFLIGHTCOLOR;
	if( options[10].used )
		config.givenOptions |= CL_BACKGROUNDCOLOR;
	if( options[11].used ) {
		config.givenOptions |= CL_NOSHAPE;
		config.noshape = true;
	}
	if( options[12].used ) {
		config.givenOptions |= CL_NEWMAILONLY;
		config.newMailsOnly = true;
	}
	if( options[13].used )
		config.givenOptions |= CL_MAILBOX;
	if( options[14].used )
		config.givenOptions |= CL_CMDONMAIL;
    if( options[15].used ) {
		config.givenOptions |= CL_CONSIDERSTATUSFIELD;
		config.considerStatusField = true;
	}
	if( options[16].used )
		config.givenOptions |= CL_READSTATUS;
	if( options[17].used )
		config.givenOptions |= CL_USEX11FONT;

    if( config.mailBox == NULL )
		ABORT( "no mailbox specified - please define at least your $MAIL environment-variable!\n" );
	else if( stat( config.mailBox, &fileStat ) == 0 )
		isMaildir = S_ISDIR( fileStat.st_mode ) != 0;

	TRACE( "mailbox is of type %s\n", isMaildir ? "maildir" : "mbox" );

    // dockapp size hard wired - sorry...
    DAInitialize( config.display, "wmail", 64, 64, argc, argv );

	outPixmap = DAMakePixmap();
    PreparePixmaps( false );

    DASetCallbacks( &callbacks );
    DASetTimeout( -1 );

    sa.sa_handler = TimerHandler;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;
    ret = sigaction( SIGALRM, &sa, 0 );

    if( ret ) {
        perror( "wmail error: sigaction" );
        exit( 1 );
    }

	XStringListToTextProperty( &name, 1, &windowName );
	XSetWMName( DADisplay, DAWindow, &windowName );

    UpdatePixmap( false );
    DAShow();
    SetTimer();

    DAEventLoop();

    return 0;
}

void PreparePixmaps( bool freeMem )
{
    // simple recoloring of the raw xpms befor creating Pixmaps of them
    // this works as long as you don't "touch" the images...

    unsigned dummy;
	XGCValues values;

    if( config.symbolColor != NULL ) { // symbol color ?
		symbols_xpm[2] = XpmColorLine( config.symbolColor, symbols_xpm[2],
									   freeMem && ( config.colorsUsed & SYM_COLOR ));
		config.colorsUsed |= SYM_COLOR;
    } else {
		symbols_xpm[2] = XpmColorLine( "#20B2AA", symbols_xpm[2],
									   freeMem && ( config.colorsUsed & SYM_COLOR ));
		config.colorsUsed |= SYM_COLOR;
	}

    if( config.fontColor != NULL ) { // font color ?
		chars_xpm[3] = XpmColorLine( config.fontColor, chars_xpm[3],
									 freeMem && ( config.colorsUsed & FNT_COLOR ));
		numbers_xpm[3] = XpmColorLine( config.fontColor, numbers_xpm[3],
									   freeMem && ( config.colorsUsed & FNT_COLOR ));
		config.colorsUsed |= FNT_COLOR;
    } else {
		chars_xpm[3] = XpmColorLine( "#D3D3D3", chars_xpm[3],
									 freeMem && ( config.colorsUsed & FNT_COLOR ));
		numbers_xpm[3] = XpmColorLine( "#D3D3D3", numbers_xpm[3],
									   freeMem && ( config.colorsUsed & FNT_COLOR ));
		config.colorsUsed |= FNT_COLOR;
	}

    if( config.backColor != NULL ) { // backlight color ?
		main_xpm[3] = XpmColorLine( config.backColor, main_xpm[3],
									freeMem && ( config.colorsUsed & BCK_COLOR ));
		symbols_xpm[3] = XpmColorLine( config.backColor, symbols_xpm[3],
									   freeMem && ( config.colorsUsed & BCK_COLOR ));
		chars_xpm[2] = XpmColorLine( config.backColor, chars_xpm[2],
									 freeMem && ( config.colorsUsed & BCK_COLOR ));
		numbers_xpm[2] = XpmColorLine( config.backColor, numbers_xpm[2],
									   freeMem && ( config.colorsUsed & BCK_COLOR ));
		config.colorsUsed |= BCK_COLOR;
    } else {
		main_xpm[3] = XpmColorLine( "#282828", main_xpm[3],
									freeMem && ( config.colorsUsed & BCK_COLOR ));
		symbols_xpm[3] = XpmColorLine( "#282828", symbols_xpm[3],
									   freeMem && ( config.colorsUsed & BCK_COLOR ));
		chars_xpm[2] = XpmColorLine( "#282828", chars_xpm[2],
									 freeMem && ( config.colorsUsed & BCK_COLOR ));
		numbers_xpm[2] = XpmColorLine( "#282828", numbers_xpm[2],
									   freeMem && ( config.colorsUsed & BCK_COLOR ));
		config.colorsUsed |= BCK_COLOR;
	}

    if( config.offlightColor != NULL ) { // off-light color ?
		main_xpm[2] = XpmColorLine( config.offlightColor, main_xpm[2],
									freeMem && ( config.colorsUsed & OFF_COLOR ));
		numbers_xpm[4] = XpmColorLine( config.offlightColor, numbers_xpm[4],
									   freeMem && ( config.colorsUsed & OFF_COLOR ));
		config.colorsUsed |= OFF_COLOR;
    } else {
		main_xpm[2] = XpmColorLine( "#000000", main_xpm[2],
									freeMem && ( config.colorsUsed & OFF_COLOR ));
		numbers_xpm[4] = XpmColorLine( "#000000", numbers_xpm[4],
									   freeMem && ( config.colorsUsed & OFF_COLOR ));
		config.colorsUsed |= OFF_COLOR;
	}

    if( config.backgroundColor != NULL ) { // window-frame background (only seen if nonshaped) ?
		main_xpm[1] = XpmColorLine( config.backgroundColor, main_xpm[1],
									freeMem && ( config.colorsUsed & BGR_COLOR ));
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
			if( tickerFS != NULL ) {
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
}

void MarkName( unsigned long checksum )
{
    name_t *name;

    for( name = names; name != NULL; name = name->next ) {
		if( name->checksum == checksum ) {
			name->flag |= FLAG_READ;
			if( config.newMailsOnly )
				numMails--;
			break;
		}
    }
}

void DetermineState()
{
    name_t *name;

    for( name = names; name != NULL; name = name->next )
		if(!( name->flag & FLAG_READ )) {
			state = STATE_NEWMAIL;

			if( config.cmdOnMail != NULL ) {
				int ret = system( config.cmdOnMail );

				if( ret == 127 || ret == -1 )
					WARNING( "execution of command \"%s\" failed.\n", config.cmdOnMail );
			}

			break;
		}
}

void ReadChecksumFile()
{
    FILE *f = fopen( config.checksumFileName, "rb" );
    if( f != NULL ) while( !feof( f )) {
		unsigned long checksum;
		if( fread( &checksum, sizeof(long), 1, f ) != 1 )
			continue;

		MarkName( checksum );
    } else
		return;

    fclose( f );
}

void WriteChecksumFile( bool writeAll )
{
    FILE *f;
    TRACE( "writing checksums:" );

    if(( f = fopen( config.checksumFileName, "wb" )) != NULL ) {
		name_t *name;
		for( name = names; name != NULL; name = name->next ) {
			if( writeAll || (name->flag & FLAG_READ)) {
				fwrite( &name->checksum, sizeof(long), 1, f );
				TRACE( " %X", name->checksum );
			}
		}
    } else
		return;

    TRACE( "\n" );

    fclose( f );
}

void UpdateChecksum( unsigned long *checksum, const char *buf )
{
    if( buf != NULL ) {
		unsigned int i, len = strlen( buf );

		for( i = 0; i < len; ++i )
			*checksum += buf[i] << (( i % sizeof(long) ) * 8 );
    }
}

void RemoveChecksumFile()
{
    TRACE( "removing checksum-file\n" );
    remove( config.checksumFileName );
}

// dummy needed because this func is a signal-handler
void TimerHandler( int dummy )
{
    static int checkMail = 0;

    if( readConfigFile ) {
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

void CheckMBox()
{
	struct stat fileStat;

	// error retrieving file-stats -> no/zero-size file and no new/read mails
	// available
	if( stat( config.mailBox, &fileStat ) == -1 || fileStat.st_size == 0 ) {
		if( state != STATE_NOMAIL ) {
			state = STATE_NOMAIL;
			ClearAllNames();
			RemoveChecksumFile();
			forceRedraw = true;
		}
	} else {
		// file has changed -> new mails arrived or some mails removed
		if( lastModifySeconds != fileStat.st_mtime || forceRead ) {
			forceRead = false;
			ParseMBoxFile( &fileStat );
			stat( config.mailBox, &fileStat );
			forceRedraw = true;
			// file has accessed (read) -> mark all mails as "read", because
			// it cannot be decided which mails the user has read...
		} else if( lastAccessSeconds != fileStat.st_atime ) {
			state = STATE_READMAIL;
			WriteChecksumFile( true );
			if( config.newMailsOnly ) {
				numMails = 0;
				SetMailFlags( FLAG_READ );
			}
			forceRedraw = true;
		}

		lastModifySeconds = fileStat.st_mtime;
		lastAccessSeconds = fileStat.st_atime;
	}
}

void CheckMaildir()
{
	DIR *dir = NULL;
	int lastState = state;
	int lastMailCount = numMails;

	if( forceRead ) {
		forceRead = false;
		ClearAllNames();
		TRACE( "all names cleared\n" );
	}

	state = STATE_NOMAIL;

	if(( dir = opendir( config.mailBox )) != NULL )
	{
		struct dirent *dirEnt = NULL;
		//bool writeChecksums = false;
		name_t *name;

		for( name = names; name != NULL; name = name->next )
			name->visited = false;

		while(( dirEnt = readdir( dir )) != NULL )
		{
			char *fullName = FileNameConcat( config.mailBox, dirEnt->d_name );
			struct stat fileStat;

			if( !stat( fullName, &fileStat ) == 0 ) {
				WARNING( "Can't stat file/path \"%s\"\n", fullName );
				free( fullName );
				continue;
			}

			if(	S_ISDIR( fileStat.st_mode ))
			{
				if( strcmp( dirEnt->d_name, "new" ) == 0 ) {
					if( TraverseDirectory( fullName, true ) > 0 )
						state = STATE_NEWMAIL;
				}
				else if( strcmp( dirEnt->d_name, "cur" ) == 0 ) {
					if( TraverseDirectory( fullName, false ) > 0 )
						if( state != STATE_NEWMAIL )
							state = STATE_READMAIL;
				}
				// directories ".", ".." and "tmp" discarded
			}
		}
		closedir( dir );
		CleanupNames();
	} else
		WARNING( "can't open directory \"%s\"\n", config.mailBox );

	if( lastState != state || lastMailCount != numMails )
		forceRedraw = true;
}

int TraverseDirectory( const char *name, bool isNewMail )
{
	DIR *dir = NULL;
	int mails = 0;

	if(( dir = opendir( name )) != NULL )
	{
		struct dirent *dirEnt = NULL;

		while(( dirEnt = readdir( dir )) != NULL )
		{
			char *fullName = FileNameConcat( name, dirEnt->d_name );
			struct stat fileStat;
			unsigned long checksum = 0;
			name_t *name;

			if( !stat( fullName, &fileStat ) == 0 ) {
				WARNING( "Can't stat file/path \"%s\"\n", fullName );
				free( fullName );
				continue;
			}

			if(	!S_ISDIR( fileStat.st_mode ))
			{
				TRACE( "found email-file \"%s\"\n", fullName );
				UpdateChecksum( &checksum, dirEnt->d_name );

				if(( name = GetMail( checksum )) == NULL )
				{
					TRACE( "-> new file - parsing it\n" );
					ParseMaildirFile( fullName, checksum, &fileStat, isNewMail );
				}
				else {
					name->flag = isNewMail ? FLAG_INITIAL : FLAG_READ;
					name->visited = true;
				}
				++mails;
			}
		}
	}

	closedir( dir );

	return mails;
}

char *FileNameConcat( const char *path, const char *fileName )
{
	int len1 = strlen( path );
	int len2 = strlen( fileName );
	char *buf;

	if( path[len1-1] == '/' )
		--len1;

	buf = (char *)malloc( len1 + len2 + 2 );

	memcpy( buf, path, len1 );
	buf[len1] = '/';
	memcpy( &buf[len1+1], fileName, len2 );
	buf[len1+len2+1] = '\0';

	return buf;
}

name_t *GetMail( unsigned long checksum )
{
	name_t *name;

    for( name = names; name != NULL; name = name->next )
		if( name->checksum == checksum )
			return name;

    return NULL;
}

void UpdatePixmap( bool flashMailSymbol )
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
    } else
		drawCount = numMails;

    for( i = 0; i < 3; ++i, drawCount /= 10 )
	{
		XCopyArea( DADisplay, numbersPixmap, outPixmap, DAGC,
				   (drawCount%10)*5, 0, 5, 9, 24-i*6, 49 );
		if( drawCount <= 9 )
			break;
    }

    if( buttonPressed )
	{
		XCopyArea( DADisplay, buttonPixmap, outPixmap, DAGC,
				   0, 0, 23, 11, 36, 48 );
	}

    switch( state ) {
    case STATE_NEWMAIL:
		if( flashMailSymbol )
			XCopyArea( DADisplay, symbolsPixmap, outPixmap, DAGC,
					   13, 0, 37, 12, 20, 7 );
    case STATE_READMAIL:
		XCopyArea( DADisplay, symbolsPixmap, outPixmap, DAGC,
				   0, 0, 13, 12, 7, 7 );

		if( config.useX11Font == NULL )
			DrawTickerBuildinFont();
		else
			DrawTickerX11Font();
    default: // make compiler happy
		;
    }

    DASetPixmap( outPixmap );
}

void ParseMBoxFile( struct stat *fileStat )
{
    char buf[1024];
    struct utimbuf timeStruct;
    int fromFound = 0;
    FILE *f = fopen( config.mailBox, "rt" );
    unsigned long checksum;

    state = STATE_READMAIL;
    ClearAllNames();

    numMails = 0;

    if( f == NULL ) {
		WARNING( "can't open mbox \"%s\"\n", config.mailBox );
		return;
    }

    while( fgets( buf, 1024, f ) != NULL )
	{
		if( strncmp( buf, "From ", 5 ) == 0 ) {
			fromFound = 1;
			checksum = 0;
			continue;
		}

		if( fromFound )
			UpdateChecksum( &checksum, buf );

		if( fromFound && strncasecmp( buf, "from: ", 6 ) == 0 )
		{
			if( SkipSender( buf+6 ))
				goto NEXTMAIL;

			InsertName( ParseFromField( buf+6 ), checksum, FLAG_INITIAL );

			++numMails;
			fromFound = 0;
			checksum = 0;
		} else if( config.considerStatusField && strncasecmp( buf, "status: ", 8 ) == 0 &&
				   strstr( buf+8, config.readStatus ) == NULL )
		{
			RemoveLastName();
			--numMails;
		}
    NEXTMAIL:
		;
    }

    fclose( f );
    ReadChecksumFile();

    DetermineState();

    timeStruct.actime = fileStat->st_atime;
    timeStruct.modtime = fileStat->st_mtime;
    utime( config.mailBox, &timeStruct );
}

void ParseMaildirFile( const char *fileName, unsigned long checksum,
					   struct stat *fileStat, bool isNewMail )
{
	char buf[1024];
	struct utimbuf timeStruct;
	FILE *f = fopen( fileName, "rt" );

	if( f == NULL )
	{
		WARNING( "can't open maildir file \"%s\"\n", fileName );
		return;
	}

	while( fgets( buf, 1024, f ) != NULL )
	{
		if( strncasecmp( buf, "from: ", 6 ) == 0 )
		{
			if( SkipSender( buf+6 ))
				break;

			InsertName( ParseFromField( buf+6 ), checksum,
						isNewMail ? FLAG_INITIAL : FLAG_READ );

			//++numMails;
		}
	}

	fclose( f );

	timeStruct.actime = fileStat->st_atime;
    timeStruct.modtime = fileStat->st_mtime;
    utime( fileName, &timeStruct );
}

char *ParseFromField( char *buf )
{
	parse_state_t state = STATE_FULLNAME;
	int fullNameEncoded = 0;
	int saveAtCharPos = -1;
	char *fullName;
	char *addressName;
	char *atChar = NULL;
	char *c;
	int maxLen = strlen( buf ) + 1;
	char *comment;

	// FIXME: Uhm, those mallocs might fail...

	fullName = malloc( maxLen );
	addressName = malloc( maxLen );
	comment = malloc( maxLen );

	memset( fullName, '\0', maxLen );
	memset( addressName, '\0', maxLen );
	memset( comment, '\0', maxLen );

	// FIXME: Don't do that "encoded" dance.  It's not intended by
	// RFC2047, and it's better to just do it in the end.
	// Cleaner.

	for( c = buf; *c != '\0'; ++c )
	{
		switch( state ) {
		case STATE_FULLNAME:

			switch( *c ) {
			case '"':
				state = STATE_QUOTED_FULLNAME;
				continue;
			case '<':
				if( fullName[0] != '\0' &&
					fullName[ strlen( fullName ) - 1 ] == ' ' )
					fullName[ strlen( fullName ) - 1 ] = '\0';
				state = STATE_ADDRESS;
				continue;
			case '@':
				saveAtCharPos = strlen( fullName );
				fullName[ saveAtCharPos ] = *c;
				continue;
			case '(':
				state = STATE_COMMENT;
				continue;
			case '=':
				if( *(c+1) == '?' ) {
					++c;
					fullNameEncoded = 1;
					state = STATE_ENCODED_FULLNAME;
					continue;
				} // else do the default action
			default:
				fullName[ strlen( fullName ) ] = *c;
			}
			continue;

		case STATE_QUOTED_FULLNAME:

			switch( *c ) {
			case '\\':
				fullName[ strlen( fullName ) ] = *(++c);
				continue;
			case '"':
				state = STATE_FULLNAME;
				continue;
			default:
				fullName[ strlen( fullName ) ] = *c;
			}
			continue;

		case STATE_ENCODED_FULLNAME:

			switch( *c ) {
			case '?':
				if( *(c+1) == '=' ) {
					++c;
					state = STATE_FULLNAME;
					continue;
				}
			default:
				; // do nothing... COMING SOON: decode at least latin1
			}
			continue;

		case STATE_ADDRESS:

			switch( *c ) {
			case '"':
				state = STATE_QUOTED_ADDRESS;
			case '>':
			case '\n':
				// FIXME: Shouldn't it break here?
				// Since the address is finished?
				continue;
			case '@':
				atChar = &addressName[ strlen( addressName ) ];
				*atChar = *c;
				continue;
			default:
				addressName[ strlen( addressName ) ] = *c;
			}
			continue;

		case STATE_QUOTED_ADDRESS:

			switch( *c ) {
			case '"':
				state = STATE_ADDRESS;
				continue;
			case '\\':
				addressName[ strlen( addressName ) ] = *(++c);
				continue;
			default:
				addressName[ strlen( addressName ) ] = *c;
			}
			continue;
			case STATE_COMMENT:
				switch( *c ) {
				case ')':
					state = STATE_FULLNAME;
					continue;
				default:
					comment[ strlen( comment ) ] = *c;
					continue;
				}
				continue;
		}
	}

	if (*comment) {
		//WARNING("Comment seen: %s\nIn: %s\nFullname: %s\nAddress: %s\n", comment, buf, fullName, addressName);
		// Comment seen: if there's an address, append to
		// fullname.  If no address, copy fullname to address
		// and comment to fullname.
		if (*addressName) {
			strcat(fullName, "(");
			strcat(fullName, comment);
			strcat(fullName, ")");
		} else {
			strcpy(addressName, fullName);
			strcpy(fullName, comment);
		}
	}

	//WARNING("Fullname: %s\nAddress: %s\n", fullName, addressName);

	// what name should be tickered
	if( config.tickerMode == TICKER_FAMILYNAME && fullName[0] != '\0' && !fullNameEncoded ) {
		free( addressName );
		return fullName;
	} else {
		if( state == STATE_FULLNAME ) {
			strcpy( addressName, fullName );
			if( saveAtCharPos != -1 )
				atChar = &addressName[saveAtCharPos];
		}
		if( config.tickerMode == TICKER_NICKNAME ) {
			if( atChar != NULL )
				*atChar = '\0';
		}
		free( fullName );
		return addressName;
	}
}

bool SkipSender( char *address )
{
	char **skipName;
	int len = strlen( address );

	// remove trailing '\n' got from fgets
	if( address[len-1] == '\n' )
		address[len-1] = '\0';

	for( skipName = config.skipNames;
		 skipName != NULL && *skipName != NULL; skipName++ )
	{
		TRACE( "comparing \"%s\" and \"%s\"\n", *skipName, address );

		// call libc-fnmatch (wildcard-match :-) !
		if( !fnmatch( *skipName, address, 0 )) {
			TRACE( "skipping sender \"%s\"\n", *skipName );
			return true;
		}
	}

	return false;
}

void InsertName( char *name, unsigned long checksum, flag_t flag )
{
    name_t *item;

    TRACE( "insertName: %X, \"%s\"\n", checksum, name );
    item = (name_t *)malloc( sizeof( name_t ));
    item->name = name; /*strdup( name );*/
    item->checksum = checksum;
    item->flag = flag;
	item->visited = true;
    item->next = names;
    names = item;

    namesChanged = true;
}

void RemoveLastName()
{
    if( names != NULL ) {
		name_t *name = names;
		names = names->next;
		free( name );
    }
}

void ClearAllNames()
{
    name_t *name, *nextName;

    for( name = names; name != NULL; name = nextName ) {
		nextName = name->next;

		free( name->name );
		free( name );
    }

    names = NULL;
    numMails = 0;

    namesChanged = true;
}

void SetMailFlags( flag_t flag )
{
    name_t *name;

    for( name = names; name != NULL; name = name->next )
		name->flag |= flag;
}

void DrawTickerX11Font()
{
	// 49x21+7+20 out-drawable size

	static int insertAt;

	if( curTickerName == NULL || namesChanged )
	{
		for( curTickerName = names;
			 curTickerName != NULL && config.newMailsOnly && ( curTickerName->flag & FLAG_READ );
			 curTickerName = curTickerName->next );

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
		do {
			curTickerName = curTickerName->next;
		} while( curTickerName != NULL && config.newMailsOnly && ( curTickerName->flag & FLAG_READ ));

		if( curTickerName != NULL ) {
			insertAt = 54;
		}
	}
}

void DrawTickerBuildinFont()
{
    // 49x21+7+20 out-drawable size
    // 14x21 font-character size

    static int insertAt;
    static int takeItFrom;

    int leftSpace;
    int drawTo;
    unsigned char *currentChar;

    if( names == NULL )
		return;

    if( curTickerName == NULL || namesChanged ) {

		for( curTickerName = names;
			 curTickerName != NULL && config.newMailsOnly && ( curTickerName->flag & FLAG_READ );
			 curTickerName = curTickerName->next );

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
		int charWidth = 57-drawTo >= 14 ? 14 - leftSpace : 57-drawTo;

		XCopyArea( DADisplay, charsPixmap, outPixmap, DAGC,
				   (outChar-32)*14+leftSpace, 0, charWidth, 21, drawTo, 20 );

        leftSpace = 0;
        drawTo += charWidth;

        if( drawTo > 57 )
            break;
    }

    if( --insertAt < 7 ) {
		insertAt = 7;
		takeItFrom++;

		if( takeItFrom/14 >= strlen( curTickerName->name )) {

			do {
				curTickerName = curTickerName->next;
			} while( curTickerName != NULL && config.newMailsOnly && ( curTickerName->flag & FLAG_READ ));

			if( curTickerName != NULL ) {
				takeItFrom = 0;
				insertAt = 57;
			}
		}
    }
}

void ButtonPressed( int button, int state, int x, int y )
{
    if( x >= 35 && x <= 59 && y >= 47 && y <= 59 ) {
		buttonPressed = true;
		forceRedraw = true;
    } else
		// reread the config file
		readConfigFile = true;
}

void ButtonReleased( int button, int state, int x, int y )
{
    buttonPressed = false;
	forceRedraw = true;

    if( x >= 35 && x <= 59 && y >= 47 && y <= 59 ) {
		int ret = system( config.runCmd );

		if( ret == 127 || ret == -1 )
			WARNING( "execution of command \"%s\" failed.\n", config.runCmd );
    }
}

void GetHexColorString( const char *colorName, char *xpmLine )
{
    XColor color;

    if( XParseColor( DADisplay,
					 DefaultColormap( DADisplay, DefaultScreen( DADisplay )),
					 colorName, &color ))
    {
		sprintf( xpmLine, "%02X%02X%02X", color.red>>8, color.green>>8,
				  color.blue>>8 );
    } else
		WARNING( "unknown colorname: \"%s\"\n", colorName );
}

char *XpmColorLine( const char *colorName, char *colorLine, bool disposeLine )
{
    char *newLine = strdup( colorLine );
    char *from = strrchr( newLine, '#' );

    if( from == NULL && !strcasecmp( &colorLine[ strlen( colorLine ) - 4 ], "none" ))  {
		// if no # found, it should be a None-color line
		free( newLine );
		newLine = malloc( 12 );
		strcpy( newLine, " \tc #" );
		newLine[11] = '\0';
		from = newLine + 4;
    }

    if( disposeLine )
		free( colorLine );

    GetHexColorString( colorName, from+1 );

    return newLine;
}

void UpdateConfiguration()
{
	struct stat fileStat;

    TRACE( "reading configuration file...\n" );

    ReadConfigFile( true );

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

    PreparePixmaps( true );

    SetTimer();
}

void CleanupNames()
{
	name_t *name, *last = NULL, *nextName;

	numMails = 0;

	for( name = names; name != NULL; name = nextName )
	{
		nextName = name->next;

		if( !name->visited ) {
			if( last == NULL )
				names = name->next;
			else
				last->next = name->next;

			free( name );
		} else {
			last = name;

			if( !config.newMailsOnly || (name->flag & FLAG_READ) == 0 )
				++numMails;
		}
	}
}

bool HasTickerWork()
{
	name_t *nextTickerName;

	if( names == NULL )
		return false;

	if( curTickerName == NULL || namesChanged ) {

		for( nextTickerName = names;
			 nextTickerName != NULL && ( config.newMailsOnly && ( nextTickerName->flag & FLAG_READ ));
			 nextTickerName = nextTickerName->next );

		if( nextTickerName == NULL )
			return false;
	}

	return true;
}
