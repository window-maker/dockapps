/*
 * options.c -- functions for processing command-line options and arguments
 *
 * Copyright (C) 2003 Hugo Villeneuve <hugo@hugovil.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#if STDC_HEADERS
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif

#include "common.h"
#include "wmnotify.h"
#include "options.h"


/*******************************************************************************
 * Display the help message and exit
 ******************************************************************************/
static void
DisplayUsage( void )
{
  printf( "Usage: %s [OPTIONS]...\n", PACKAGE );
  printf( "Email notification for single POP3 or IMAP4 account.\n\n" );
  printf( "  -c <config-file>          use alternate configuration file\n" );
  printf( "  -d                        Display debugging messages.\n" );
  printf( "  -display <host:display>   X display name\n" );
  printf( "  -geometry +XPOS+YPOS      initial window position\n" );
  printf( "  -h                        display this help and exit\n" );
  printf( "  -version                  display version information and exit\n");
  printf( "\n" );
}


/*******************************************************************************
 * Display version information and exit
 ******************************************************************************/
static void
DisplayVersion( void )
{
  printf( "\n" );
  printf( "  %s, version %s\n", PACKAGE, VERSION );
  printf( "  Written by Hugo Villeneuve\n\n" );
}


static void
InvalidOption( const char *message, /*@null@*/ const char *string )
{
  if( string == NULL ) {
    fprintf(stderr, "%s: %s\n", PACKAGE, message );
  }
  else {
    fprintf(stderr, "%s: %s %s\n", PACKAGE, message, string );
  }

  fprintf(stderr, "Try `%s -h' for more information.\n", PACKAGE );

  exit( EXIT_FAILURE );
}


/*******************************************************************************
 * Initializes the different options passed as arguments on the command line.
 ******************************************************************************/
void
ParseCommandLineOptions( int argc, char *argv[] )
{
  int i;
  char *token;
  bool config_file_on = false;
  bool display_on     = false;
  bool geometry_on    = false;

  /* Default values. */
  wmnotify_infos.debug = false;

  for( i = 1; i < argc; i++ ) {
    token = argv[i];
    switch( token[0] ) {
    case '-':
      /* Processing options names */
      switch( token[1] ) {
      case 'c':
	if( strlen( &token[1] ) == 1 ) {
	  config_file_on = true;
	}
	else {
	  InvalidOption( "invalid option", token );
	}
	break;
      case 'd':
	if( STREQ( "display", &token[1] ) ) {
	  display_on = true;
	}
	else if( strlen( &token[1] ) == 1 ) {
	  wmnotify_infos.debug = true;
	}
	break;
      case 'g':
	if( STREQ( "geometry", &token[1] ) ) {
	  geometry_on = true;
	}
	else {
	  InvalidOption( "invalid option", token );
	}
	break;
      case 'h':
	if( strlen( &token[1] ) == 1 ) {
	  DisplayUsage();
	  exit( EXIT_SUCCESS );
	}
	InvalidOption( "invalid option", token );
	break;
      case 'v' :
	if( STREQ( "version", &token[1] ) ) {
	  DisplayVersion();
	  exit( EXIT_SUCCESS );
	}
	else {
	  InvalidOption( "invalid option", token );
	}
	break;
      default:
	InvalidOption( "invalid option", token );
	break;
      } /* end switch( token[1] ) */
      break;
    default:
      /* Processing options arguments */
      if( config_file_on != false ) {
	wmnotify_infos.optional_config_file = token;
	/*strcpy( config_file_name, token );*/
	config_file_on = false;
      }
      else if( display_on != false ) {
	display_on = false;
	wmnotify_infos.display_arg = token;
      }
      else if( geometry_on != false ) {
	geometry_on = false;
	wmnotify_infos.geometry_arg = token;
      }
      else {
	InvalidOption( "invalid option", token );
      }
      break;
    } /* end switch( token[0] ) */
  } /* end for */

  if( config_file_on != false ) {
    InvalidOption( "missing configuration file parameter", NULL );
  }
  else if( display_on != false ) {
    InvalidOption( "missing display parameter", NULL );
  }
  else if( geometry_on != false ) {
    InvalidOption( "missing geometry parameter", NULL );
  }
}
