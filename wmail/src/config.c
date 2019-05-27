///////////////////////////////////////////////////////////////////////////////
// config.c
// configuration file parser, part of wmail
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "common.h"
#include "config.h"


///////////////////////////////////////////////////////////////////////////////
// wmailrc file format
//
// # - comment-lines
// Window.Display = "string"
// Window.NonShaped = On|Off
// Window.Button.Command = "string"
// Mail.MailBox = "string"
// Mail.ChecksumFile = "string"
// Mail.CheckIntervall = number
// Mail.ShowOnlyNew = On|Off
// Mail.SkipSender = "string"
// Mail.OnNew.Command = "string"
// Mail.UseStatusField = On|Off
// Ticker.Mode = Address|NickName|FamilyName
// Ticker.Frames = number
// Ticker.BoldFont = On|Off
// Ticker.X11Font = "string"
// Colors.Symbols = "string"
// Colors.Font = "string"
// Colors.Backlight = "string"
// Colors.OffLight = "string"
// Colors.NonShapedFrame = "string"



///////////////////////////////////////////////////////////////////////////////
// typedefs


// list of enum-identifiers and their associated values
typedef struct { char *id; int value; } enumList_t;



///////////////////////////////////////////////////////////////////////////////
// local prototypes


bool ReadString( const char *from, unsigned int line, char **to );
bool ReadEnum( const char *from, unsigned int line, int *to, const enumList_t *enumList );
bool ReadBool( const char *from, unsigned int line, bool *to );
bool ReadInt( const char *from, unsigned int line, int *to );
bool IsWhiteSpace( const char *chr );
const char *SkipWhiteSpaces( const char *str );

// current configuration
config_t config = {
    NULL,
    NULL, // use $MAIL environment-variable
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    WMAIL_CHECK_INTERVAL,
    WMAIL_FPS,
    false,
    false,
    TICKER_ADDRESS,
    NULL,
    NULL,
    false,
    NULL,
    0,
    0
};

// enumeration names for ticker mode
enumList_t tickerEnum[] =
{
    { "address", TICKER_ADDRESS },
    { "familyname", TICKER_FAMILYNAME },
    { "nickname", TICKER_NICKNAME },
    { NULL, 0 }
};

bool Tokenize( const char *line, const char **id, const char **value )
{
    int len;
    const char *token1, *token2;

    if( line != NULL )
    {
	token1 = SkipWhiteSpaces( line );

	if(( len = strlen( token1 )) != 0 && token1[0] != '#' )
	{
	    token2 = strchr( token1, '=' );
	    if( token2 != NULL )
		token2 = SkipWhiteSpaces( token2+1 );

	    if( !IsWhiteSpace( token2 ))
	    {
		*id = token1;
		*value = token2;

		return true;
	    }
	}
    }

    return false;
}

void AddSenderToSkipList( char *sender  )
{
    int numNames, i;
    char **skipName, **newList;

    for( skipName = config.skipNames, numNames = 0;
	 skipName != NULL && *skipName != NULL; skipName++ )
    {
	if( !strcmp( *skipName, sender ))
	    return;

	numNames++;
    }

    TRACE( "adding \"%s\" to skip-list of currently %d names\n", sender, numNames );
    newList = malloc( sizeof(char *) * (numNames + 2) );

    for( i = 0; i < numNames; ++i )
	newList[i] = config.skipNames[i];

    newList[i] = strdup( sender );
    newList[i+1] = NULL;
    free( config.skipNames );
    config.skipNames = newList;
}

void ResetConfigStrings( void )
{
    if( !( config.givenOptions & CL_MAILBOX )) {
	free( config.mailBox );
	config.mailBox = NULL;
    }

    if( !( config.givenOptions & CL_RUNCMD )) {
	free( config.runCmd );
	config.runCmd = NULL;
    }

    if( !( config.givenOptions & CL_SYMBOLCOLOR )) {
	free( config.symbolColor );
	config.symbolColor = NULL;
    }

    if( !( config.givenOptions & CL_FONTCOLOR )) {
	free( config.fontColor );
	config.fontColor = NULL;
    }

    if( !( config.givenOptions & CL_BACKCOLOR )) {
	free( config.backColor );
	config.backColor = NULL;
    }

    if( !( config.givenOptions & CL_OFFLIGHTCOLOR )) {
	free( config.offlightColor );
	config.offlightColor = NULL;
    }

    if( !( config.givenOptions & CL_BACKGROUNDCOLOR )) {
	free( config.backgroundColor );
	config.backgroundColor = NULL;
    }

    if( !( config.givenOptions & CL_CHECKSUMFILENAME )) {
	free( config.checksumFileName );
	config.checksumFileName = NULL;
    }

    if( !( config.givenOptions & CL_CMDONMAIL )) {
	free( config.cmdOnMail );
	config.cmdOnMail = NULL;
    }

    if( !( config.givenOptions & CL_USEX11FONT )) {
	free( config.useX11Font );
	config.useX11Font = NULL;
    }
}

void PostProcessConfiguration( void )
{
    if( config.display == NULL )
	config.display = strdup( WMAIL_DISPLAY );

    if( config.runCmd == NULL )
	config.runCmd = strdup( WMAIL_CLIENT_CMD );

    if( config.mailBox == NULL )
    {
	char *envMBox = getenv( "MAIL" );
	if( envMBox != NULL )
	    config.mailBox = strdup( envMBox );
    }
}

void ReadConfigFile( bool resetConfigStrings )
{
    char *usersHome;

    // free all config strings and reset their pointers if required
    if( resetConfigStrings )
	ResetConfigStrings();

    if(( usersHome = getenv( "HOME" )) != NULL )
    {
	char *fileName;
	if(( fileName = MakePathName( usersHome, WMAIL_RC_FILE )) != NULL )
	{
	    FILE *f = fopen( fileName, "rt" );

	    if( f != NULL )
	    {
		char buf[1024];
		int line = 1;

		for( ; !feof( f ); ++line )
		{
		    const char *id, *value;
		    unsigned int len;

		    if( fgets( buf, 1024, f ) == NULL )
			break;

		    // first eliminate the trailing whitespaces
		    for( len = strlen( buf );
			 len > 0 && IsWhiteSpace(buf+(--len)); )
			*(buf+len) = '\0';

		    if( !Tokenize( buf, &id, &value ))
			continue;

		    if( !strncasecmp( id, "Window.Display", 14 )) {
			if( !( config.givenOptions & CL_DISPLAY ))
			    ReadString( value, line, &config.display );
		    } else if( !strncasecmp( id, "Window.NonShaped", 16 )) {
			if( !( config.givenOptions & CL_NOSHAPE ))
			    ReadBool( value, line, &config.noshape );
		    } else if( !strncasecmp( id, "Window.Button.Command", 21 )) {
			if( !( config.givenOptions & CL_RUNCMD ))
			    ReadString( value, line, &config.runCmd );
		    } else if( !strncasecmp( id, "Mail.MailBox", 12 )) {
			if( !( config.givenOptions & CL_MAILBOX ))
			    ReadString( value, line, &config.mailBox );
		    } else if( !strncasecmp( id, "Mail.ChecksumFile", 17 )) // no corresponding cmdline option
			ReadString( value, line, &config.checksumFileName );
		    else if( !strncasecmp( id, "Mail.CheckIntervall", 19 )) {
			if( !( config.givenOptions & CL_CHECKINTERVAL ))
			    ReadInt( value, line, &config.checkInterval );
		    } else if( !strncasecmp( id, "Mail.ShowOnlyNew", 16 )) {
			if( !( config.givenOptions & CL_NEWMAILONLY ))
			    ReadBool( value, line, &config.newMailsOnly );
		    } else if( !strncasecmp( id, "Ticker.Mode", 11 )) {
			if( !( config.givenOptions & CL_TICKERMODE ))
			    ReadEnum( value, line, (int *)&config.tickerMode, tickerEnum );
		    } else if( !strncasecmp( id, "Ticker.Frames", 13 )) {
			if( !( config.givenOptions & CL_FPS ))
			    ReadInt( value, line, &config.fps );
		    } else if( !strncasecmp( id, "Colors.Symbols", 14 )) {
			if( !( config.givenOptions & CL_SYMBOLCOLOR ))
			    ReadString( value, line, &config.symbolColor );
		    } else if( !strncasecmp( id, "Colors.Font", 11 )) {
			if( !( config.givenOptions & CL_FONTCOLOR ))
			    ReadString( value, line, &config.fontColor );
		    } else if( !strncasecmp( id, "Colors.Backlight", 16 )) {
			if( !( config.givenOptions & CL_BACKCOLOR ))
			    ReadString( value, line, &config.backColor );
		    } else if( !strncasecmp( id, "Colors.OffLight", 15 )) {
			if( !( config.givenOptions & CL_OFFLIGHTCOLOR ))
			    ReadString( value, line, &config.offlightColor );
		    } else if( !strncasecmp( id, "Colors.NonShapedFrame", 21 )) {
			if( !( config.givenOptions & CL_NOSHAPE ))
			    ReadString( value, line, &config.backgroundColor );
		    } else if( !strncasecmp( id, "Ticker.X11Font", 14 )) {
			if( !( config.givenOptions & CL_USEX11FONT ))
			    ReadString( value, line, &config.useX11Font );
		    } else if( !strncasecmp( id, "Mail.SkipSender", 15 )) { // no corresponding cmdline options
			char *skip;
			if( ReadString( value, line, &skip ))
			    AddSenderToSkipList( skip );
		    } else if( !strncasecmp( id, "Mail.OnNew.Command", 18 )) {
			if( !( config.givenOptions & CL_CMDONMAIL ))
			    ReadString( value, line, &config.cmdOnMail );
		    } else if( !strncasecmp( id, "Mail.UseStatusField", 19 )) {
			if( !( config.givenOptions & CL_CONSIDERSTATUSFIELD ))
			    ReadBool( value, line, &config.considerStatusField );
		    } else if( !strncasecmp( id, "Mail.ReadStatus", 15 )) {
			if( !( config.givenOptions & CL_READSTATUS ))
			    ReadString( value, line, &config.readStatus );
		    } else
			WARNING( "cfg-file(%i): unrecognized: \"%s\"\n", line, buf );
		}

		fclose( f );
	    } else {
		TRACE( "unable to open config-file \"%s\"\n", fileName );
	    }

	    free( fileName );
	} else {
	    TRACE( "unable to allocate config-file\n" );
	}
    } else {
	TRACE( "no $HOME defined - config-file not read\n" );
    }

    PostProcessConfiguration();
}

bool ReadString( const char *from, unsigned int line, char **to )
{
    if( *from++ == '"' ) {
	const char *trailingQuote;

	for( trailingQuote = strchr( from, '"' );
	     trailingQuote != NULL;
	     trailingQuote = strchr( trailingQuote, '"' ))
	{
	    if( *(trailingQuote-1) != '\\' )
		break;

	    ++trailingQuote;
	}

	if( trailingQuote != NULL ) {
	    // valid string found, copy and translate escape sequences
	    const char *c;
	    char *to_c;

	    // disposing of "to" is up to the caller...
	    *to = malloc( trailingQuote - from + 1 );
	    to_c = *to;

	    for( c = from; c != trailingQuote; ++c ) {
		if( *c == '\\' ) {
		    switch( *(++c) ) {
		    case 'n': *to_c = '\n'; break;
		    case 'b': *to_c = '\b'; break;
		    case '\\': *to_c = '\\'; break;
		    case 'r': *to_c = '\r'; break;
		    case 't': *to_c = '\t'; break;
		    case '"': *to_c = '"'; break;
		    default: {
			int value, i;
			for( i = 0, value = 0; i < 3; ++i ) {
			    if( c+i == NULL || *(c+i) < '0' || *(c+i) > '9' )
				break;
			    value = value * 10 + *(c+i) - '0';
			}
			if( value == 0 )
			    WARNING( "cfg-file(%i): '\\0' in string or unknown escape sequence found\n", line );
			else {
			    *to_c = (char)value;
			    c += i-1;
			}
		    }
		    }
		} else
		    *to_c = *c;

		++to_c;
	    }

	    *to_c = '\0';
	    TRACE( "ReadString read \"%s\"\n", *to );

	    return true;
	}
    }

    WARNING( "cfg-file(%i): invalid string\n" );
    return false;
}

bool ReadBool( const char *from, unsigned int line, bool *to )
{
    if( !strcasecmp( from, "on" ) || !strcasecmp( from, "yes" ) || !strcasecmp( from, "true" ))
	*to = true;
    else if( !strcasecmp( from, "off" ) || !strcasecmp( from, "no" ) || !strcasecmp( from, "false" ))
	*to = false;
    else {
	WARNING( "cfg-file(%i): invalid boolean value: \"%s\"\n", line, from );
	return false;
    }

    TRACE( "ReadBool read \"%s\"\n", *to ? "True" : "False" );

    return true;
}

bool ReadInt( const char *from, unsigned int line, int *to )
{
    int value = 0;

    if( *from == '0' && (*(from+1) == 'x' || *(from+1) == 'X') ) {
	for( from += 2; *from != '\0' && !IsWhiteSpace( from ); ++from )
	{
	    if( value > (INT_MAX - 0xf) / 0x10 ) {
		WARNING( "cfg-file(%i): hexadecimal-number too large: \">%x\"\n", line, INT_MAX );
		return false;
	    }

	    if( *from >= '0' && *from <= '9')
		value = value * 16 + *from - '0';
	    else if( *from >= 'a' && *from >= 'f' )
		value = value * 16 + *from - 'a' + 10;
	    else if( *from >= 'A' && *from >= 'F' )
		value = value * 16 + *from - 'A' + 10;
	    else {
		WARNING( "cfg-file(%i): invalid hex-digit: \"%c\"\n", line, *from );
		return false;
	    }
	}
    } else for( ; *from != '\0' && !IsWhiteSpace( from ); ++from ) {
	if( value > (INT_MAX - 9) / 10 ) {
	    WARNING( "cfg-file(%i): decimal-number too large: \">%i\"\n", line, INT_MAX );
	    return false;
	}
	if( *from >= '0' && *from <= '9' )
	    value = value * 10 + *from - '0';
	else {
	    WARNING( "cfg-file(%i): invalid decimal-digit: \"%c\"\n", line, *from );
	    return false;
	}
    }

    *to = value;

    TRACE( "ReadInt read \"%i\"\n", *to );

    return true;
}

bool ReadEnum( const char *from, unsigned int line, int *to, const enumList_t *enumList )
{
    int index;

    for( index = 0; enumList[index].id != NULL; ++index )
	if( !strcasecmp( enumList[index].id, from )) {
	    *to = enumList[index].value;

	    TRACE( "ReadEnum read \"%i\"\n", *to );

	    return true;
	}

    WARNING( "cfg-file(%i): unknown modifier: \"%s\"\n", line, from );

    return false;
}

bool IsWhiteSpace( const char *chr )
{
    return ( chr != NULL && ( *chr == ' ' || *chr == '\t' || *chr == '\n' )) ? true : false;
}

const char *SkipWhiteSpaces( const char *str )
{
    const char *c;

    for( c = str; IsWhiteSpace( c ); ++c )
	;

    return c;
}
