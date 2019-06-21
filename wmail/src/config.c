///////////////////////////////////////////////////////////////////////////////
// config.c
// configuration file parser, part of wmail
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


#ifdef HAVE_CONFIG_H
#ifndef CONFIG_H_INCLUDED
#include "../config.h"
#define CONFIG_H_INCLUDED
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
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
// Mail.CheckInterval = number
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


static bool ReadString( const char *from, unsigned int line, char **to );
static bool ReadEnum( const char *from, unsigned int line, int *to,
		      const enumList_t *enumList );
static bool ReadBool( const char *from, unsigned int line, bool *to );
static bool ReadInt( const char *from, unsigned int line, int *to );
static bool IsWhiteSpace( const char *chr );
static const char *SkipWhiteSpaces( const char *str );

// current configuration
config_t config = {
    .checkInterval = WMAIL_CHECK_INTERVAL,
    .fps = WMAIL_FPS,
    .tickerMode = TICKER_ADDRESS
};

// enumeration names for ticker mode
static enumList_t tickerEnum[] =
{
    { "address", TICKER_ADDRESS },
    { "familyname", TICKER_FAMILYNAME },
    { "nickname", TICKER_NICKNAME },
    { NULL, 0 }
};

static bool Tokenize( const char *line, const char **id, const char **value )
{
    size_t len;
    const char *token1, *token2;

    if( line != NULL )
    {
	token1 = SkipWhiteSpaces( line );

	if(( len = strlen( token1 )) != 0 && token1[0] != '#' )
	{
	    token2 = strchr( token1, '=' );
	    if( token2 != NULL )
		token2 = SkipWhiteSpaces( token2 + 1 );

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

static void AddSenderToSkipList( char *sender )
{
    size_t numNames;
    char **skipName, **newList;

    for( skipName = config.skipNames, numNames = 0;
	 skipName != NULL && *skipName != NULL; skipName++ )
    {
	if( !strcmp( *skipName, sender ))
	    return;

	numNames++;
    }

    TRACE( "adding \"%s\" to skip-list of currently %d names\n", sender, numNames );
    newList = realloc( config.skipNames, sizeof *config.skipNames * (numNames + 2) );

    if( newList == NULL )
    {
	WARNING( "Cannot allocate memory for skip list.\n");
	return;
    }

    config.skipNames = newList;
    config.skipNames[numNames++] = sender;
    config.skipNames[numNames++] = NULL;
}

void ResetConfigStrings( void )
{
    if( !( config.givenOptions & CL_MAILBOX ))
    {
	free( config.mailBox );
	config.mailBox = NULL;
    }

    if( !( config.givenOptions & CL_RUNCMD ))
    {
	free( config.runCmd );
	config.runCmd = NULL;
    }

    if( !( config.givenOptions & CL_SYMBOLCOLOR ))
    {
	free( config.symbolColor );
	config.symbolColor = NULL;
    }

    if( !( config.givenOptions & CL_FONTCOLOR ))
    {
	free( config.fontColor );
	config.fontColor = NULL;
    }

    if( !( config.givenOptions & CL_BACKCOLOR ))
    {
	free( config.backColor );
	config.backColor = NULL;
    }

    if( !( config.givenOptions & CL_OFFLIGHTCOLOR ))
    {
	free( config.offlightColor );
	config.offlightColor = NULL;
    }

    if( !( config.givenOptions & CL_BACKGROUNDCOLOR ))
    {
	free( config.backgroundColor );
	config.backgroundColor = NULL;
    }

    /*
     * No corresponding command-line option.
     */
    free( config.checksumFileName );
    config.checksumFileName = NULL;

    if( !( config.givenOptions & CL_CMDONMAIL ))
    {
	free( config.cmdOnMail );
	config.cmdOnMail = NULL;
    }

    if( !( config.givenOptions & CL_USEX11FONT ))
    {
	free( config.useX11Font );
	config.useX11Font = NULL;
    }

    /*
     * No corresponding command-line option.
     */
    if( config.skipNames != NULL )
    {
	char **n;
	for( n = config.skipNames; *n; ++n )
	    free( *n );
	free( config.skipNames );
	config.skipNames = NULL;
    }
}

static void PostProcessConfiguration( void )
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

void ReadConfigFile( const char *configFile, bool resetConfigStrings )
{
    // free all config strings and reset their pointers if required
    if( resetConfigStrings )
	ResetConfigStrings();

    FILE *f = fopen( configFile, "r" );
    if( f != NULL )
    {
	char buf[1024];
	int line = 1;

	for( ; !feof( f ); ++line )
	{
	    const char *id, *value;
	    size_t len;

	    if( fgets( buf, sizeof buf, f ) == NULL )
		break;

	    // first eliminate the trailing whitespaces
	    for( len = strlen( buf ); len > 0 && IsWhiteSpace(buf + (--len)); )
		*(buf + len) = '\0';

	    if( !Tokenize( buf, &id, &value ))
		continue;

	    if( PREFIX_MATCHES( id, "Window.Display", false ))
	    {
		if( !( config.givenOptions & CL_DISPLAY ))
		    ReadString( value, line, &config.display );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Window.NonShaped", false ))
	    {
		if( !( config.givenOptions & CL_NOSHAPE ))
		    ReadBool( value, line, &config.noshape );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Window.Button.Command", false ))
	    {
		if( !( config.givenOptions & CL_RUNCMD ))
		    ReadString( value, line, &config.runCmd );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Mail.MailBox", false ))
	    {
		if( !( config.givenOptions & CL_MAILBOX ))
		    ReadString( value, line, &config.mailBox );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Mail.ChecksumFile", false ))
	    {
		/*
		 * No corresponding command-line option.
		 */
		ReadString( value, line, &config.checksumFileName );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Mail.CheckInterval", false ))
	    {
		if( !( config.givenOptions & CL_CHECKINTERVAL ))
		    ReadInt( value, line, &config.checkInterval );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Mail.ShowOnlyNew", false ))
	    {
		if( !( config.givenOptions & CL_NEWMAILONLY ))
		    ReadBool( value, line, &config.newMailsOnly );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Ticker.Mode", false ))
	    {
		if( !( config.givenOptions & CL_TICKERMODE ))
		    ReadEnum( value, line, (int *)&config.tickerMode, tickerEnum );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Ticker.Frames", false ))
	    {
		if( !( config.givenOptions & CL_FPS ))
		    ReadInt( value, line, &config.fps );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Colors.Symbols", false ))
	    {
		if( !( config.givenOptions & CL_SYMBOLCOLOR ))
		    ReadString( value, line, &config.symbolColor );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Colors.Font", false ))
	    {
		if( !( config.givenOptions & CL_FONTCOLOR ))
		    ReadString( value, line, &config.fontColor );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Colors.Backlight", false ))
	    {
		if( !( config.givenOptions & CL_BACKCOLOR ))
		    ReadString( value, line, &config.backColor );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Colors.OffLight", false ))
	    {
		if( !( config.givenOptions & CL_OFFLIGHTCOLOR ))
		    ReadString( value, line, &config.offlightColor );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Colors.NonShapedFrame", false ))
	    {
		if( !( config.givenOptions & CL_NOSHAPE ))
		    ReadString( value, line, &config.backgroundColor );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Ticker.X11Font", false ))
	    {
		if( !( config.givenOptions & CL_USEX11FONT ))
		    ReadString( value, line, &config.useX11Font );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Mail.SkipSender", false ))
	    {
		/*
		 * No corresponding command-line option.
		 */
		char *skip;
		if( ReadString( value, line, &skip ))
		    AddSenderToSkipList( skip );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Mail.OnNew.Command", false ))
	    {
		if( !( config.givenOptions & CL_CMDONMAIL ))
		    ReadString( value, line, &config.cmdOnMail );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Mail.UseStatusField", false ))
	    {
		if( !( config.givenOptions & CL_CONSIDERSTATUSFIELD ))
		    ReadBool( value, line, &config.considerStatusField );
		continue;
	    }

	    if( PREFIX_MATCHES( id, "Mail.ReadStatus", false ))
	    {
		if( !( config.givenOptions & CL_READSTATUS ))
		    ReadString( value, line, &config.readStatus );
		continue;
	    }

	    WARNING( "cfg-file(%i): unrecognized: \"%s\"\n", line, buf );
	}

	fclose( f );
    }
    else
	TRACE( "unable to open config-file \"%s\"\n", configFile );

    PostProcessConfiguration();
}

static bool ReadString( const char *from, unsigned int line, char **to )
{
    if( *from++ == '"' )
    {
	const char *trailingQuote;

	for( trailingQuote = strchr( from, '"' );
	     trailingQuote != NULL;
	     trailingQuote = strchr( trailingQuote, '"' ))
	{
	    if( *(trailingQuote-1) != '\\' )
		break;

	    ++trailingQuote;
	}

	if( trailingQuote != NULL )
	{
	    // valid string found, copy and translate escape sequences
	    const char *c;
	    char *to_c;

	    // disposing of "to" is up to the caller...
	    *to = malloc( trailingQuote - from + 1 );
	    to_c = *to;

	    for( c = from; c != trailingQuote; ++c )
	    {
		if( *c == '\\' )
		{
		    switch( *(++c) )
		    {
		    case 'n':  *to_c = '\n'; break;
		    case 'b':  *to_c = '\b'; break;
		    case '\\': *to_c = '\\'; break;
		    case 'r':  *to_c = '\r'; break;
		    case 't':  *to_c = '\t'; break;
		    case '"':  *to_c = '"';  break;
		    default:
			{
			    int value, i;
			    for( i = 0, value = 0; i < 3; ++i )
			    {
				if( c + i == NULL || *(c + i) < '0' || *(c + i) > '9' )
				    break;
				value = value * 10 + *(c + i) - '0';
			    }
			    if( value == 0 )
				WARNING( "cfg-file(%i): '\\0' in string or unknown escape sequence found\n",
					 line );
			    else
			    {
				*to_c = (char)value;
				c += i - 1;
			    }
			}
		    }
		}
		else
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

static bool ReadBool( const char *from, unsigned int line, bool *to )
{
    if( !strcasecmp( from, "on" ) || !strcasecmp( from, "yes" ) || !strcasecmp( from, "true" ))
	*to = true;
    else if( !strcasecmp( from, "off" ) || !strcasecmp( from, "no" ) || !strcasecmp( from, "false" ))
	*to = false;
    else
    {
	WARNING( "cfg-file(%i): invalid boolean value: \"%s\"\n", line, from );
	return false;
    }

    TRACE( "ReadBool read \"%s\"\n", *to ? "True" : "False" );

    return true;
}

static bool ReadInt( const char *from, unsigned int line, int *to )
{
    int value = 0;

    if( *from == '0' && (*(from + 1) == 'x' || *(from + 1) == 'X'))
	for( from += 2; *from != '\0' && !IsWhiteSpace( from ); ++from )
	{
	    if( value > (INT_MAX - 0xf) / 0x10 )
	    {
		WARNING( "cfg-file(%i): hexadecimal-number too large: \">%x\"\n", line, INT_MAX );
		return false;
	    }

	    if( *from >= '0' && *from <= '9')
		value = value * 16 + *from - '0';
	    else if( *from >= 'a' && *from >= 'f' )
		value = value * 16 + *from - 'a' + 10;
	    else if( *from >= 'A' && *from >= 'F' )
		value = value * 16 + *from - 'A' + 10;
	    else
	    {
		WARNING( "cfg-file(%i): invalid hex-digit: \"%c\"\n", line, *from );
		return false;
	    }
	}
    else
	for( ; *from != '\0' && !IsWhiteSpace( from ); ++from )
	{
	    if( value > (INT_MAX - 9) / 10 )
	    {
		WARNING( "cfg-file(%i): decimal-number too large: \">%i\"\n",
			 line, INT_MAX );
		return false;
	    }
	    if( *from >= '0' && *from <= '9' )
		value = value * 10 + *from - '0';
	    else
	    {
		WARNING( "cfg-file(%i): invalid decimal-digit: \"%c\"\n",
			 line, *from );
		return false;
	    }
	}

    *to = value;

    TRACE( "ReadInt read \"%i\"\n", *to );

    return true;
}

static bool ReadEnum( const char *from, unsigned int line, int *to,
		      const enumList_t *enumList )
{
    int index;

    for( index = 0; enumList[index].id != NULL; ++index )
	if( !strcasecmp( enumList[index].id, from ))
	{
	    *to = enumList[index].value;

	    TRACE( "ReadEnum read \"%i\"\n", *to );

	    return true;
	}

    WARNING( "cfg-file(%i): unknown modifier: \"%s\"\n", line, from );

    return false;
}

static bool IsWhiteSpace( const char *chr )
{
    return chr != NULL && ( *chr == ' ' || *chr == '\t' || *chr == '\n' );
}

static const char *SkipWhiteSpaces( const char *str )
{
    const char *c;

    for( c = str; IsWhiteSpace( c ); ++c )
	;

    return c;
}
