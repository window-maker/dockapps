/*
 * configfile.c -- Parsing the configuration file
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
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>

#if STDC_HEADERS
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif

#include <pwd.h>

#include "common.h"
#include "wmnotify.h"
#include "configfile.h"


#define LINE_BUFFER_LEN 256

/* Name of configuration file in user's home directory. */
const static char default_config_filename[] = ".wmnotifyrc";
const static char delimiter_single_arg[] = " \n";
const static char delimiter_multiple_arg[] = "#\n";


static void
CreateDefaultConfigurationFile( char *file )
{
  int status;
  FILE *fp;
  
  fp = fopen( file, "w" );
  if( fp == NULL ) {
    fprintf( stderr, "%s: Can't create file \"%s\"\n", PACKAGE, file );
    exit( EXIT_FAILURE );
  }

  /* Changing permissions so that only the user can read/modify the file. */
  status = chmod( file, S_IRUSR | S_IWUSR );
  if( status < 0 ) {
    fprintf( stderr, "%s: Can't set permission bits on file \"%s\"\n", PACKAGE, file );
    exit( EXIT_FAILURE );
  }

  fprintf( fp, "# ~/.wmnotifyrc -- Default configuration file for wmnotify\n\n" );
  fprintf( fp, "# Replace all 'xxxxxxxx' fields with your own settings.\n\n" );
  fprintf( fp, "# Parameters preceded by a '#' character are optional.\n" );
  fprintf( fp, "# You can set their values by removing the leading '#'.\n\n" );
  fprintf( fp, "# Mail Protocol: POP3 or IMAP4.\n" );
  fprintf( fp, "protocol POP3\n\n" );
  fprintf( fp, "# Use SSL encrytion: 0=disable, 1=enable (optional, default is "
	   "disabled).\n" );
  fprintf( fp, "use_ssl 0\n\n" );
  fprintf( fp, "# Mail Server Name.\n" );
  fprintf( fp, "server xxxxxxxx\n\n" );
  fprintf( fp, "# Mail Server Port Number (optional, default is 110).\n" );
  fprintf( fp, "port 110\n\n" );
  fprintf( fp, "# Username.\n" );
  fprintf( fp, "username xxxxxxxx\n\n" );
  fprintf( fp, "# Password.\n" );
  fprintf( fp, "password xxxxxxxx\n\n" );
  fprintf( fp, "# IMAP folder name (optional, default is INBOX).\n" );
  fprintf( fp, "# folder INBOX.some_folder\n\n" );
  fprintf( fp, "# Mail Check Interval (in minutes, default is 5 minutes).\n" );
  fprintf( fp, "#mailcheckdelay 5\n\n" );
  fprintf( fp, "# Default mail client (optional).\n" );
  fprintf( fp, "#mailclient sylpheed\n\n" );
  fprintf( fp, "# Audio notification, 0=disable, 1=enable (optional, default is "
	   "disabled).\n" );
  fprintf( fp, "enablebeep 0\n\n" );
  fprintf( fp, "# Location of sound file for audio notification. If no sound file is\n" );
  fprintf( fp, "# specified, the console beep will be used instead.\n" );
  fprintf( fp, "audiofile /usr/local/share/sounds/halmsgs.wav\n\n" );
  fprintf( fp, "# Volume (0 to 100%%).\n" );
  fprintf( fp, "volume 100\n" );

  fprintf( stderr, "%s: A default configuration file has been created in your "
	   "home directory: \"%s\"\n", PACKAGE, file );
  fprintf( stderr, "You must edit it before running %s.\n", PACKAGE );
  
  status = fclose( fp );
  if( status != EXIT_SUCCESS ) {
    fprintf( stderr, "%s: Error closing file \"%s\"\n", PACKAGE, file );
  }
}


static void
ParseCommand( char *line, /*@out@*/ char *argv[] )
{
  int argc = 0;

  while( *line != '\0' ) {       /* if not the end of line ....... */
    while( *line == ' ' || *line == '\t' || *line == '\n' ) {
      *line++ = '\0';     /* replace white spaces with 0    */
    }
    *argv++ = line;          /* save the argument position     */
    while( *line != '\0' && *line != ' ' && *line != '\t' && *line != '\n' ) {
      line++;             /* skip the argument until ...    */
    }

    argc++;

    if( argc == ARGV_LIMIT ) {
      fprintf( stderr, "%s: Too much arguments for external command\n",
	       PACKAGE );
      exit( EXIT_FAILURE );
    }
  }
  
  *argv = NULL; /* mark the end of argument list */
}


static char *
GetArguments( char *parameter, bool single_argument )
{
  char *token;

  if( single_argument ) {
    token = strtok( NULL, delimiter_single_arg );
  }
  else {
    /* We search for a string terminated by either a '#' character (the rest of
       the line is then a comment, which is simply ignored ), or the end of line
       character '\n'. */
    token = strtok( NULL, delimiter_multiple_arg );
  }
  
  if( token == NULL ) {
    fprintf( stderr, "%s: Missing argument for \"%s\" parameter in "
	     "configuration file.\n", PACKAGE, parameter );
    exit( EXIT_FAILURE );
  }
  
  return token;
}


static int
GetNumber( char *token, char *parameter )
{
  char temp[32]; /* Check size ??? */
  
  if( sscanf( token, "%[0123456789]", temp ) == 0 ) {
    fprintf( stderr, "%s: Invalid argument for \"%s\" parameter in "
	     "configuration file.\n", PACKAGE, parameter );
    exit( EXIT_FAILURE );
  }
  
  return atoi( temp );
}


static void
ParseConfigurationFile( FILE *file )
{
  char line[LINE_BUFFER_LEN];
  char *token;
  bool protocol_found = false;
  bool server_found = false;
  bool username_found = false;
  bool password_found = false;
  const char *err_string = NULL;

  /* Default values for optional parameters. */
  strcpy( wmnotify_infos.imap_folder, "INBOX"); /* Default IMAP folder. */
  wmnotify_infos.port = 110;
  wmnotify_infos.mail_check_interval = 60; /* 1 minute interval. */
  wmnotify_infos.audible_notification = false; /* Disabled. */
  wmnotify_infos.use_ssl = false; /* Disabled. */
  wmnotify_infos.mail_client_argv[0] = NULL; /* No default command. */
  wmnotify_infos.audiofile[0] = '\0'; /* No default audio file. */
  wmnotify_infos.volume = 100; /* 100% volume. */

  /* Reading one line of data from the configuration file. */
  /* char *fgets(char *s, int size, FILE *stream);
     Reading stops after an EOF or a newline.  If a newline is read, it is
     stored into the buffer.  A '\0'  is  stored after the last character in
     the buffer. */
  while( fgets( line, LINE_BUFFER_LEN, file ) != NULL ) {
    token = strtok( line, delimiter_single_arg );
    
    if( ( token == NULL ) || ( token[0] == '#' ) ) {
      continue; /* Next iteration of the while() loop (next line). */
    }
    
    if( STREQ( token, "protocol" ) ) {
      token = GetArguments( "protocol", true );
      if( STREQ( token, "POP3" ) == true ) {
	wmnotify_infos.protocol = POP3_PROTOCOL;
      }
      else if( STREQ( token, "IMAP4" ) == true ) {
	wmnotify_infos.protocol = IMAP4_PROTOCOL;
      }
      else {
	fprintf( stderr, "%s: protocol must be POP3 or IMAP4.\n", PACKAGE );
	exit( EXIT_FAILURE );
      }

      protocol_found = true;
    }
    else if( STREQ( token, "imap_folder" ) ) {
      token = GetArguments( "imap_folder", true );
      /* Should check size before using strcpy(), or use strncopy() instead. */
      strcpy( wmnotify_infos.imap_folder, token );
    }
    else if( STREQ( token, "use_ssl" ) ){
      int number;

      token = GetArguments( "use_ssl", true );
      number = GetNumber( token, "use_ssl" );
      if( number == 0 ) {
	wmnotify_infos.use_ssl = false;
      }
      else if( number == 1 ) {
#if HAVE_SSL
	wmnotify_infos.use_ssl = true;
#else
	fprintf( stderr, "%s error: You must compile %s with SSL support to\n" \
		 "set parameter 'use_ssl' to true in configuration file\n", PACKAGE, PACKAGE );
	exit( EXIT_FAILURE );
#endif
      }
      else {
	fprintf( stderr, "%s: Invalid value for parameter 'use_ssl' in\n" \
		 "configuration file (must be 0 or 1): %d\n", PACKAGE, number );
	exit( EXIT_FAILURE );
      }
    }
    else if( STREQ( token, "server" ) ) {
      token = GetArguments( "server", true );
      strncpy( wmnotify_infos.server_name, token, MAX_STR_LEN );
      server_found = true;
    }
    else if( STREQ( token, "port" ) ) {
      token = GetArguments( "port", true );
      wmnotify_infos.port = (u_int16_t) GetNumber( token, "port" );
    }
    
    else if( STREQ( token, "username" ) ) {
      token = GetArguments( "username", true );
      strncpy( wmnotify_infos.username, token, MAX_STR_LEN );
      username_found = true;
    }
    else if( STREQ( token, "password" ) ) {
      token = GetArguments( "password", true );
      strncpy( wmnotify_infos.password, token, MAX_STR_LEN );
      password_found = true;
    }
    else if( STREQ( token, "mailcheckdelay" ) ) {
      int delay; /* delay in minutes. */
      
      token = GetArguments( "mailcheckdelay", true );
      /* GetNumber() will exit if a negative number is entered. */
      delay = GetNumber( token, "mailcheckdelay" );
      if( delay == 0 ) {
	fprintf( stderr, "%s: Mail check interval must be greater than '0'\n",
		 PACKAGE );
	exit( EXIT_FAILURE );
      }
      wmnotify_infos.mail_check_interval = (unsigned int) delay * 60;
    }
    else if( STREQ( token, "mailclient" ) ) {
      token = GetArguments( "mailclient", false ); /* Multiple arguments */
      strcpy( wmnotify_infos.mail_client_command, token );
      ParseCommand( wmnotify_infos.mail_client_command,
		    wmnotify_infos.mail_client_argv );
    }
    else if( STREQ( token, "enablebeep" ) ){
      int number;

      token = GetArguments( "enablebeep", true );
      number = GetNumber( token, "enablebeep" );
      if( number == 0 ) {
	wmnotify_infos.audible_notification = false;
      }
      else if( number == 1 ) {
	wmnotify_infos.audible_notification = true;
      }
      else {
	fprintf( stderr, "%s: Invalid value for for parameter 'enablebeep' in\n" \
		 "configuration file (must be 0 or 1): %d\n", PACKAGE, number );
	exit( EXIT_FAILURE );
      }
    }
    else if( STREQ( token, "audiofile" ) ) {
      token = GetArguments( "audiofile", true );
      /* Should check size before using strcpy(), or use strncopy() instead. */
      strcpy( wmnotify_infos.audiofile, token );
    }
    else if( STREQ( token, "volume" ) ) {
      token = GetArguments( "volume", true );
      wmnotify_infos.volume = GetNumber( token, "volume" );
    }
    else {
      fprintf( stderr, "%s: invalid parameter in configuration file: %s\n", PACKAGE,
	       token );
      exit( EXIT_FAILURE );
    }
    
    token = strtok( NULL, delimiter_single_arg );
    if( ( token != NULL ) && ( token[0] != '#' ) ) {
      fprintf( stderr, "%s: Garbage at end of line in configuration file: %s\n", PACKAGE,
	       token );
      exit( EXIT_FAILURE );
    }
  }

  if( protocol_found == false ) {
    err_string = "protocol";
  }
  else if( server_found == false ) {
    err_string = "server";
  }
  else if( username_found == false ) {
    err_string = "username";
  }
  else if( password_found == false ) {
    err_string = "password";
  }
  else {
    return; /* success */
  }
  
  /* Failure. */
  fprintf( stderr, "%s: Mandatory parameter \"%s\" missing from configuration "
	   "file.\n", PACKAGE, err_string );
  exit( EXIT_FAILURE );
}


/*******************************************************************************
 * Read and parse the configuration file in the user's home directory
 ******************************************************************************/
void
ConfigurationFileInit( void )
{
  FILE *fp;
  int status;
  size_t len;
  
  /* Check if an optional configuration file was specified on the command
     line. */
  if( wmnotify_infos.optional_config_file != NULL ) {
    /* Trying to open the file. */
    fp = fopen( wmnotify_infos.optional_config_file, "r" );
    if( fp == NULL ) {
      perror( PACKAGE );
      ErrorLocation( __FILE__, __LINE__ );
      exit( EXIT_FAILURE );
    }
  }
  else {
    /* Using the default configuration file. */
    char *home_dir;
    char *default_config_file;
    
    home_dir = getenv("HOME");
    if( home_dir == NULL ) {
      /* We're trying to expand ~/, but HOME isn't set. */
      struct passwd *pw = getpwuid( getuid() );
      
      if( pw != NULL ) {
	home_dir = pw->pw_dir;
      }
      else {
	fprintf( stderr, "%s: Couldn't determine user's home directory path\n",
		 PACKAGE );
	exit( EXIT_FAILURE );
      }
    }
    
    /* We add 1 to the length for the terminating character '\0'. */
    len = strlen( home_dir ) + strlen( "/" ) + strlen( default_config_filename )
      + 1;
    default_config_file = xmalloc( len, __FILE__, __LINE__ );
    
    sprintf( default_config_file, "%s/%s", home_dir, default_config_filename );
  
    fp = fopen( default_config_file, "r" );
    if( fp == NULL ) {
      /* If we cannot open the default configuration file, it probably means
	 it is missing, so we create it. */
      CreateDefaultConfigurationFile( default_config_file );
      free( default_config_file );
      exit( EXIT_FAILURE );
    }
    
    free( default_config_file );
  }

  ParseConfigurationFile( fp );
  
  status = fclose( fp );
  if( status != EXIT_SUCCESS ) {
    fprintf( stderr, "%s: Error closing configuration file.\n", PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
}
