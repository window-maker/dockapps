/*
 * dockapp.c -- routines for managing dockapp windows and icons.
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

/* Define filename_M */
#define DOCKAPP_M 1

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "common.h"
#include "dockapp.h"


#define XLIB_FAILURE 0
#define XLIB_SUCCESS 1

/* Specifies the border width */
#define BWIDTH 1

/* Width and height in pixels of Window Maker icons. */
#define ICON_SIZE 64


static void
CreateIconFromXpmData( char *pixmap_data[] )
{
  int status;

  dockapp.xpm_icon.attributes.valuemask |=
    ( XpmReturnPixels | XpmReturnExtensions );
  
  /* Using the XPM library to read XPM data from the array in the included XPM
     file. The 'shapemask' Pixmap variable is set to an additional 1-bit deep
     pixmap that can then be used as a shape mask for the XShapeCombineMask()
     function. */
  status = XpmCreatePixmapFromData( dockapp.display, dockapp.root_win,
				    pixmap_data, &dockapp.xpm_icon.image,
				    &dockapp.xpm_icon.shapemask,
				    &dockapp.xpm_icon.attributes );
  if( status != XpmSuccess ) {
    fprintf( stderr, "%s: XpmCreatePixmapFromData() failed\n", PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
}


static Pixel
GetColor( char *name )
{
  int status;
  bool res;
  XColor color;
  XWindowAttributes attributes;

  status = XGetWindowAttributes( dockapp.display, dockapp.root_win,
				 &attributes );
  if( status == XLIB_FAILURE ) {
    fprintf( stderr, "%s: XGetWindowAttributes() failed\n", PACKAGE );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }

  color.pixel = 0;
  res = (bool) XParseColor( dockapp.display, attributes.colormap, name,
			    &color );
  if( res == false ) {
    fprintf( stderr, "%s: Can't parse %s.\n", PACKAGE, name );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  res = (bool) XAllocColor( dockapp.display, attributes.colormap, &color );
  if( res == false ) {
    fprintf( stderr, "%s: Can't allocate %s.\n", PACKAGE, name );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  return color.pixel;
}


static void
flush_expose( Window win )
{
  XEvent dummy;
  bool res = true;

  while( res != false ) {
    res = (bool) XCheckTypedWindowEvent( dockapp.display, win, Expose, &dummy );
  }
}


void
RedrawWindow( void )
{
  flush_expose( dockapp.iconwin );
  
  (void) XCopyArea( dockapp.display, dockapp.xpm_icon.image, dockapp.iconwin,
		    dockapp.NormalGC, 0, 0, dockapp.xpm_icon.attributes.width,
		    dockapp.xpm_icon.attributes.height, 0, 0 );
  
  flush_expose( dockapp.win );
  
  (void) XCopyArea( dockapp.display, dockapp.xpm_icon.image, dockapp.win,
		    dockapp.NormalGC, 0, 0, dockapp.xpm_icon.attributes.width,
		    dockapp.xpm_icon.attributes.height, 0, 0 );
}


void
copyXPMArea( int x, int y, unsigned int sx, unsigned int sy, int dx, int dy )
{
  (void) XCopyArea( dockapp.display, dockapp.xpm_icon.image,
		    dockapp.xpm_icon.image, dockapp.NormalGC, x, y, sx, sy,
		    dx, dy );
}


/*******************************************************************************
 * New window creation and initialization for a Dockable Application
 ******************************************************************************/
void
InitDockAppWindow( int argc, char *argv[], char *pixmap_data[],
		   char *display_arg, char *geometry_arg )
{
  XGCValues  gcv;
  XSizeHints size_hints;
  XWMHints   wm_hints;
  int status;
  int gravity = 0; /* Used to store the gravity value returned by XWMGeometry,
		      but not used. */
  
  /* Opening a connection to the X server. */
  dockapp.display = XOpenDisplay( display_arg );
  if( dockapp.display == NULL ) {
    fprintf( stderr, "%s: Can't open display: %s\n", PACKAGE,
	     XDisplayName( display_arg ) );
    ErrorLocation( __FILE__, __LINE__ );
    exit( EXIT_FAILURE );
  }
  
  dockapp.screen   = DefaultScreen( dockapp.display );
  dockapp.root_win = RootWindow( dockapp.display, dockapp.screen );
  dockapp.d_depth  = DefaultDepth( dockapp.display, dockapp.screen );
  
  /* Create a window to hold the stuff */
  size_hints.flags = USSize | USPosition;
  size_hints.x = 0;
  size_hints.y = 0;
  
  /* Constructing window's geometry information. */
  /* XWMGeometry() returns an 'int', but Xlib documentation doesn't explain
     it's meaning. */
  XWMGeometry( dockapp.display, dockapp.screen, geometry_arg, NULL, BWIDTH,
	       &size_hints, &size_hints.x, &size_hints.y, &size_hints.width,
	       &size_hints.height, &gravity );
  
  size_hints.width  = ICON_SIZE;
  size_hints.height = ICON_SIZE;
  dockapp.back_pix = GetColor("white");
  dockapp.fore_pix = GetColor("black");
  
  dockapp.win = XCreateSimpleWindow( dockapp.display, dockapp.root_win,
				     size_hints.x, size_hints.y,
				     (unsigned int) size_hints.width,
				     (unsigned int) size_hints.height,
				     BWIDTH, dockapp.fore_pix,
				     dockapp.back_pix );
  
  dockapp.iconwin = XCreateSimpleWindow( dockapp.display, dockapp.win,
					 size_hints.x, size_hints.y,
					 (unsigned int) size_hints.width,
					 (unsigned int) size_hints.height,
					 BWIDTH, dockapp.fore_pix,
					 dockapp.back_pix );
  
  /* Configuring Client to Window Manager Communications. */
  
  /* WM_NORMAL_HINTS property: size hints for a window in it's normal state. */
  /* Replaces the size hints for the WM_NORMAL_HINTS property on the specified
     window. */
  XSetWMNormalHints( dockapp.display, dockapp.win, &size_hints );
  
  /* Setting the WM_CLASS property. */
  {
    char *app_name = argv[0];
    XClassHint wm_class;
    
    /* The res_name member contains the application name.
       The res_class member contains the application class. */
    /* The name set in this property may differ from the name set as WM_NAME.
       That is, WM_NAME specifies what should be displayed in the title bar and,
       therefore, can contain temporal information (for example, the name of a
       file currently in an editor's buffer). On the other hand, the name
       specified as part of WM_CLASS is the formal name of the application that
       should be used when retrieving the application's resources from the
       resource database. */
    wm_class.res_name = app_name;
    wm_class.res_class = app_name;
    (void) XSetClassHint( dockapp.display, dockapp.win, &wm_class );
  }
  
  /* Setting the WM_NAME property.
     This specifies what should be displayed in the title bar (usually the
     application name). */
  {
    XTextProperty text_prop;
   
    char *app_name = argv[0];
    const int string_count = 1;
  
    status = XStringListToTextProperty( &app_name, string_count, &text_prop );
    if( status == 0 ) {
      fprintf( stderr, "%s: XStringListToTextProperty() failed\n", PACKAGE );
      ErrorLocation( __FILE__, __LINE__ );
      exit( EXIT_FAILURE );
    }
    
    XSetWMName( dockapp.display, dockapp.win, &text_prop );

    /* Freing the storage for the value field. */
    (void) XFree( text_prop.value );
  }

  /* WM_HINTS --> Additional hints set by the client for use by the Window
     Manager. */
  /* XWMHints wm_hints; */

  /* WithdrawnState, NormalState or IconicState. Must be set to WithdrawnState
     for DockApp. */
  wm_hints.flags = StateHint | IconWindowHint | IconPositionHint |
    WindowGroupHint;
  wm_hints.initial_state = WithdrawnState; /* Withdrawn, Normal */
  wm_hints.icon_window = dockapp.iconwin;
  wm_hints.icon_x = size_hints.x;
  wm_hints.icon_y = size_hints.y;
  wm_hints.window_group = dockapp.win;
  (void) XSetWMHints( dockapp.display, dockapp.win, &wm_hints );
  
  /* Sets the WM_COMMAND property. This sets the command and arguments used to
     invoke the application. */
  (void) XSetCommand( dockapp.display, dockapp.win, argv, argc );
  
  /* ... */
  (void) XSelectInput( dockapp.display, dockapp.win,
		       ButtonPressMask | ExposureMask | ButtonReleaseMask |
		       PointerMotionMask | StructureNotifyMask );
  
  (void) XSelectInput( dockapp.display, dockapp.iconwin,
		       ButtonPressMask | ExposureMask | ButtonReleaseMask |
		       PointerMotionMask | StructureNotifyMask );
  
  /* Create GC for drawing */
  gcv.foreground = dockapp.fore_pix;
  gcv.background = dockapp.back_pix;
  gcv.graphics_exposures = 0;
  dockapp.NormalGC = XCreateGC( dockapp.display, dockapp.root_win,
				GCForeground | GCBackground |
				GCGraphicsExposures, &gcv );
  
  /* Convert XPM data to XImage */
  CreateIconFromXpmData( pixmap_data );

  XShapeCombineMask( dockapp.display, dockapp.win, ShapeBounding, 0, 0,
		     dockapp.xpm_icon.shapemask, ShapeSet );
  
  XShapeCombineMask( dockapp.display, dockapp.iconwin, ShapeBounding, 0, 0,
		     dockapp.xpm_icon.shapemask, ShapeSet );
  
  /* Making the new window visible. */
  (void) XMapWindow( dockapp.display, dockapp.win );
}
