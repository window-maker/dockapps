/*
 * wmnotify.c -- POP3 E-mail notification program
 *
 * Copyright (C) 2003 Hugo Villeneuve (hugo@hugovil.com)
 * based on WMPop3 by Scott Holden (scotth@thezone.net)
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

/* Define filename_M */
#define WMNOTIFY_M 1

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "common.h"
#include "dockapp.h"
#include "pop3.h"
#include "imap.h"
#include "network.h"
#include "xevents.h"
#include "options.h"
#include "configfile.h"
#if defined(HAVE_SNDFILE)
#  include "sound.h"
#endif
#include "wmnotify.xpm"
#include "wmnotify.h"


/* Set in DoubleClick() to stop the new mail animation when the mail client is
   opened. */
static bool animation_stop = false;

static int animation_image = MAILBOX_FULL;

/* Set in response to signal sent by SingleClick() to force mail check. Also set to true at
 * startup to force initial check. */
static bool manual_check = true;

/* Used to signal TimerThread to quit. Inactive for now. */
static bool quit = false;

static int double_click_notif = false;

/* TimerThread ID */
static pthread_t timer_thread;


inline void
ErrorLocation( const char *file, int line )
{
  fprintf( stderr, "  Error in file \"%s\" at line #%d\n", file, line );
}


void *
xmalloc( size_t size, const char *filename, int line_number )
{
  void *value;

  value = malloc( size );
  
  if( value == NULL ) {
    perror( PACKAGE );
    ErrorLocation( filename, line_number );
    exit( EXIT_FAILURE );
  }

  return value;
}


static void
DisplayOpenedEmptyMailbox( void )
{
  /* Opened and empty mailbox image */
  copyXPMArea( MAILBOX_OPENED_EMPTY_SRC_X, MAILBOX_OPENED_EMPTY_SRC_Y,
	       MAILBOX_SIZE_X, MAILBOX_SIZE_Y, MAILBOX_DEST_X, MAILBOX_DEST_Y );
  RedrawWindow();
}


static void
DisplayOpenedFullMailbox( void )
{
  /* Full mailbox image */
  copyXPMArea( MAILBOX_OPENED_FULL_SRC_X, MAILBOX_OPENED_FULL_SRC_Y,
	       MAILBOX_SIZE_X, MAILBOX_SIZE_Y,
	       MAILBOX_DEST_X, MAILBOX_DEST_Y );
  RedrawWindow();
}


static void
DisplayClosedMailbox( void )
{
  /* Opened mailbox image */
  copyXPMArea( MAILBOX_CLOSED_SRC_X, MAILBOX_CLOSED_SRC_Y,
	       MAILBOX_SIZE_X, MAILBOX_SIZE_Y,
	       MAILBOX_DEST_X, MAILBOX_DEST_Y );
  RedrawWindow();
}


static void
DisplayExecuteCommandNotification( void )
{
  /* Visual notification that the double-click was catched. */
  copyXPMArea( EXEC_CMD_IMG_SRC_X, EXEC_CMD_IMG_SRC_Y,
	       MAILBOX_SIZE_X, MAILBOX_SIZE_Y, MAILBOX_DEST_X, MAILBOX_DEST_Y );
  RedrawWindow();
}


static void
ExecuteCommand( char *argv[] )
{
  pid_t pid;
  char *msg;

  /* No command defined, this is not an error. */
  if( argv[0] == NULL ) {
    return;
  }

  pid = fork(); /* fork a child process. */
  
  if( pid < 0) {
    perror( PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  else if( pid == 0 ) { /* Child process */
    /* When execvp() is successful, it doesn't return; otherwise, it returns
       -1 and sets errno. */
    (void) execvp( argv[0], argv );
    
    msg = strerror( errno );
    fprintf( stderr, "%s: The external mail program couldn't be started.\n",
	     PACKAGE);
    fprintf( stderr, "Check your path or your configuration file for errors.\n"
	     );
    fprintf( stderr, "%s: \"%s\"\n", msg, argv[0] );
    exit( EXIT_FAILURE );
  }
}


/* single-click --> Checking mail */
static void
SingleClick( void )
{
  int status;

  if( wmnotify_infos.debug ) {
    printf( "%s: SingleClick() Entry\n", PACKAGE );
  }

  /* Sending a signal to awake the TimerThread() thread. */
  status = pthread_kill( timer_thread, SIGUSR1 );
  if( status != EXIT_SUCCESS ) {
    fprintf( stderr, "%s: pthread_kill() error (%d)\n", PACKAGE, status );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }

  if( wmnotify_infos.debug ) {
    printf( "%s: SingleClick() Exit\n", PACKAGE );
  }
}


/* Double-click --> Starting external mail client. */
static void
DoubleClick( void )
{
  int status;

  if( wmnotify_infos.mail_client_argv[0] != NULL ) {
    /* Starting external mail client. */
    ExecuteCommand( wmnotify_infos.mail_client_argv );
    
    double_click_notif = true;

    /* Sending a signal to awake the TimerThread() thread. This was previously
       done with a mutex variable (animation_stop), but this caused a bug when the
       following sequence was encountered:
       -The user double-click to start the external mail client
       -A new E-mail is received shortly after that
       -The user exit the external mail client
       -The user manually check for new E-mail
       -The audio notification sound is played, but no animation image is
       displayed.
       This was because setting the mutex variable 'animation_stop' didn't
       awakened the TimerThread(), but single-clicking awakened it. Since the
       'animation_stop' variable was still set to true, no animation occured. */
    status = pthread_kill( timer_thread, SIGUSR2 );
    if( status != EXIT_SUCCESS ) {
      fprintf( stderr, "%s: pthread_kill() error (%d)\n", PACKAGE, status );
      ErrorLocation( __FILE__, __LINE__ );
      exit( EXIT_FAILURE );
    }

    DisplayExecuteCommandNotification();
    sleep(1);
    DisplayClosedMailbox();

    double_click_notif = false;
  }
  else {
    fprintf( stderr, "%s: Warning: No email-client defined.\n", PACKAGE );
  }
}


static void
CatchChildTerminationSignal( int signal )
{
  switch( signal ) {
  case SIGCHLD:
    /* Wait for Mail Client child process termination. Child enters zombie
       state: process is dead and most resources are released, but process
       descriptor remains until parent reaps exit status via wait. */
    
    /* The WNOHANG option prevents the call to waitpid from suspending execution
       of the caller. */
    (void) waitpid( 0, NULL, WNOHANG );
    break;
  default:
    fprintf( stderr, "%s: Unregistered signal received, exiting.\n", PACKAGE );
    exit( EXIT_FAILURE );
  }
}


static void
CatchTimerSignal( int signal )
{
  switch( signal ) {
  case SIGUSR1:
    /* Catching the signal sent by the SingleClick() function. */
    manual_check = true;
    break;
  case SIGUSR2:
    /* Catching the signal sent by the DoubleClick() function. */
    animation_stop = true;
    break;
  default:
    fprintf( stderr, "%s: CatchTimerSignal(): unknown signal (%d)\n", PACKAGE,
	     signal );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
}


static void
NewMailAnimation( void )
{
  if( animation_image == MAILBOX_FULL ) {
    DisplayOpenedFullMailbox();
    animation_image = MAILBOX_CLOSED;
    if( wmnotify_infos.debug ) {
      printf( "%s: NewMailAnimation() MAILBOX_FULL.\n", PACKAGE );
    }
  }
  else {
    DisplayClosedMailbox();
    animation_image = MAILBOX_FULL;
    if( wmnotify_infos.debug ) {
      printf( "%s: NewMailAnimation() MAILBOX_CLOSED.\n", PACKAGE );
    }
  }
}


/* We display the opened mailbox image only when doing a manual check. */
static int
CheckForNewMail( bool manual_check )
{
  int new_messages;

  if( manual_check == true ) {
    DisplayOpenedEmptyMailbox();
  }

  if( wmnotify_infos.protocol == POP3_PROTOCOL ) {
    new_messages = POP3_CheckForNewMail();
  }
  else if( wmnotify_infos.protocol == IMAP4_PROTOCOL ) {
    new_messages = IMAP4_CheckForNewMail();
  }
  else {
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }

  if( ( manual_check == true ) && ( new_messages > 0 ) ) {
    animation_image = MAILBOX_FULL;
  }
  
  return new_messages;
}


static void *
TimerThread( /*@unused@*/ void *arg )
{
  int new_messages = 0;
  int counter = -1;
  bool animation_running = false;

  /* For catching the signal SIGUSR1. This signal is sent by the main program thread when the
   * user is issuing a single-click to manually check for new mails. */
  (void) signal( SIGUSR1, CatchTimerSignal );
  
  /* For catching the signal SIGUSR2. This signal is sent by the main program thread when the
   * user is issuing a double-click to start ther external  mail client. */
  (void) signal( SIGUSR2, CatchTimerSignal );

  while( quit == false ) {
    if( wmnotify_infos.debug ) {
      printf( "%s: Timer thread iteration.\n", PACKAGE );
    }
    if( ( manual_check == true ) || ( counter == 0 ) ) {
      new_messages = CheckForNewMail( manual_check );
      manual_check = false;
     
      if( wmnotify_infos.debug ) {
	printf( "%s: new messages = %d.\n", PACKAGE, new_messages );
      }
 
      if( new_messages > 0 ) {
	/* Checking if audio notification was already produced. */
	if( animation_running == false ) {
	  /* Audible notification, if requested in configuration file. */
	  if( wmnotify_infos.audible_notification != false ) {
	    if( strlen( wmnotify_infos.audiofile ) != 0 ) {
#if defined(HAVE_SNDFILE)
	      PlayAudioFile( wmnotify_infos.audiofile, wmnotify_infos.volume );
#endif
	    }
	    else {
	      AudibleBeep();
	    }
	  }
	  
	  animation_running = true;
	}
	/* Number of times to execute timer loop before checking again for new mails when the
	 * animation is running (when the animation is running, we sleep for
	 * NEW_MAIL_ANIMATION_DURATION instead of wmnotify_infos.mail_check_interval). We set
	 * the check interval to 30 seconds because we want the new mail condition to be
	 * removed as soon as possible when the new messages are checked. */
	counter = 30 * 1000000 / NEW_MAIL_ANIMATION_DURATION;
      }
    }
    
    if( ( animation_stop == true ) || ( new_messages <= 0 ) ) {
      if( wmnotify_infos.debug ) {
	if( animation_stop != false ) {
	  printf( "%s: animation_stop is true\n", PACKAGE );
	}
      }
      animation_running = false;
      animation_stop = false;
      if( double_click_notif == false ) {
	/* Before exiting, be sure to put NO MAIL image back in place... */
	DisplayClosedMailbox();
      }
    }

    /* If sleep() returns because the requested time has elapsed, the value returned will be
     * 0. If sleep() returns because of premature arousal due to delivery of a signal, the
     * return value will be the "unslept" amount (the requested time minus the time actually
     * slept) in seconds. */
    
    if( animation_running == false ) {
      (void) sleep( wmnotify_infos.mail_check_interval );
      counter = 0;
    }
    else {
      NewMailAnimation();
      (void) usleep( NEW_MAIL_ANIMATION_DURATION );
      counter--;
    }

    if( wmnotify_infos.debug ) {
      printf( "%s: counter = %d\n", PACKAGE, counter );
    }
  } /* end while */
  
  if( wmnotify_infos.debug ) {
    printf( "%s: Error, TimerThread() exited abnormally\n", PACKAGE );
  }

  /* This code is never reached for now, because quit is always false. */
  pthread_exit( NULL );
}


/*******************************************************************************
 * Main function
 ******************************************************************************/
int
main( int argc, char *argv[] )
{
  int status;
  
  /* Initialization */
  ParseCommandLineOptions( argc, argv );

  /* Reading configuration options from configuration file. */
  ConfigurationFileInit();

  /* For catching the termination signal SIGCHLD when the external mail client
     program is terminated, thus permitting removing zombi processes... */
  (void) signal( SIGCHLD, CatchChildTerminationSignal );
  
  /* Initialize callback function pointers. */
  ProcessXlibEventsInit( SingleClick, DoubleClick );

  /* Initializing and creating a DockApp window. */
  InitDockAppWindow( argc, argv, wmnotify_xpm, wmnotify_infos.display_arg,
		     wmnotify_infos.geometry_arg );

  /* Starting thread for periodically checking for new mail. */
  status = pthread_create( &timer_thread, NULL, TimerThread, NULL );
  if( status != 0 ) {
    fprintf( stderr, "%s: Thread creation failed (%d)\n", PACKAGE, status );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }

  /* Main loop, processing X Events */
  ProcessXlibEvents();

  /* This code is never reached for now. */
  fprintf( stderr, "%s: Program exit\n", PACKAGE );
  
  exit( EXIT_SUCCESS );
}
