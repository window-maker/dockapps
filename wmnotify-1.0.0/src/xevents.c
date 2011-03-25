/*
 * xevents.c -- handling X events, and detecting single-click and double-click
 *              mouse events.
 *
 * Copyright (C) 2009 Hugo Villeneuve <hugo@hugovil.com>
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
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <X11/Xlib.h>

#include "common.h"
#include "dockapp.h"
#include "xevents.h"


/* Maximum time between mouse double-clicks, in milliseconds */
#define DOUBLE_CLICK_MAX_INTERVAL_MS 250


/* Function pointers to handle single and double mouse click events. */
static void (*SingleClickCallback)( void ) = NULL;

static void (*DoubleClickCallback)( void ) = NULL;


void
AudibleBeep( void )
{
  /* The specified volume is relative to the base volume for the keyboard.
     To change the base volume of the keyboard, use XChangeKeyboardControl(). */
  (void) XBell( dockapp.display, 100 ); /* Volume = 100% */
}


/* This function must be called at the beginning of your program to initialize
   the function pointers to handle single and double click mouse events. */
void
ProcessXlibEventsInit( void (*single_click_callback)( void ),
		       void (*double_click_callback)( void ) )
{
  int status;
  
  /* This must be called before any other XLib functions. */
  status = XInitThreads();
  if( status == 0 ) {
    fprintf( stderr, "%s: XInitThreads() initialization failed\n", PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  SingleClickCallback = single_click_callback;
  DoubleClickCallback = double_click_callback;
}


/* Processing of X events */
void
ProcessXlibEvents( void )
{
  bool quit = false;
  bool button1_pressed = false;
  bool check_for_double_click = false;
  XEvent Event;
  
  while( quit == false ) {
    if( ( check_for_double_click != false ) &&
	( XPending( dockapp.display ) == 0 ) ) {
      /* If no other button 1 events are received after the delay, then it is a
	 single-click mouse event. */
      if( SingleClickCallback != NULL ) {
	(*SingleClickCallback)();
      }
      
      check_for_double_click = false;
    }
    /* XNextEvent is a blocking call: it will return only when an event is
       ready to be processed, thus freeing the CPU for other tasks when no
       events are available. */
    (void) XNextEvent( dockapp.display, &Event );
    switch( Event.type ) {
    case Expose:
      /* Window was uncovered... */
      RedrawWindow();
      break;
    case DestroyNotify:
      /* Window was killed... */
      /* Is this necessary ? */
      (void) XCloseDisplay( dockapp.display );
      quit = true;
      break;
    case ClientMessage:
      /* Doesn't seem to work... */
      printf( "Client message received...\n" );
      break;
    case ButtonPress:
      if( Event.xbutton.button == Button1 ) {
	/* Mouse LEFT button pressed. */
	button1_pressed = true;
      }
      break;
    case ButtonRelease:
      if( Event.xbutton.button == Button1 ) {
	/* Mouse LEFT button released. */
	if( button1_pressed != false ) {
	  /* We act only when the button is released */
	  if( check_for_double_click != false ) {
	    /* Double-click */
	    if( DoubleClickCallback != NULL ) {
	      (*DoubleClickCallback)();
	    }
	    check_for_double_click = false;
	  }
	  else {
	    (void) usleep( DOUBLE_CLICK_MAX_INTERVAL_MS * 1000 );
	    check_for_double_click = true;
	  }
	}
      }
      break;
    }
  } /* end while */
}
