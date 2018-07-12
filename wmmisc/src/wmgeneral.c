/*
 *  wmmisc - WindowMaker Dockapp for monitoring misc. information.
 *  Copyright (C) 2003-2006 Jesse S. (luxorfalls@sbcglobal.net)
 *
 *  wmmisc is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  wmmisc is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with wmmisc; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "wmgeneral.h"

static Display *da_display;
static Window da_root_window;
static int da_screen;
static int da_x_fd;
static int da_display_depth;
static XSizeHints da_size_hints;
static XWMHints da_wm_hints;
static Pixel da_bg_pixel;
static Pixel da_fg_pixel;
static char* da_geometry = NULL;
static Window da_icon_window;
static Window da_window;
static GC da_normal_gc;
static xpm_icon da_window_icon;
static Pixmap da_pixmask;

typedef struct
{
      int enable;
      int top;
      int bottom;
      int left;
      int right;
} mouse_region_t;

mouse_region_t mouse_region[MAX_MOUSE_REGION];

void
get_xpm( xpm_icon* window_icon, char** pixmap_bytes )
{
   XWindowAttributes attributes;
   int err;

   XGetWindowAttributes( da_display, da_root_window, &attributes );

   window_icon->attributes.valuemask |= ( XpmReturnPixels | XpmReturnExtensions );

   err = XpmCreatePixmapFromData( da_display,
				  da_root_window,
				  pixmap_bytes,
				  &window_icon->pixmap,
				  &window_icon->mask,
				  &window_icon->attributes );

   if ( XpmSuccess != err )
   {
      fprintf( stderr, "Not enough free color cells.\n" );
      exit( 1 );
   }
}

Pixel
get_color_by_name( const char* color_name )
{
   XColor color;
   XWindowAttributes attributes;

   XGetWindowAttributes( da_display, da_root_window, &attributes );

   color.pixel = 0;

   if ( 0 == XParseColor( da_display, attributes.colormap, color_name, &color ) )
   {
      fprintf( stderr, "wmmisc: Can't parse color name: '%s'\n", color_name );
      return 0;
   }

   if ( 0 == XAllocColor( da_display, attributes.colormap, &color ))
   {
      fprintf( stderr, "wmmisc: Can't allocate memory for color: '%s'\n", color_name );
      return 0;
   }

  return color.pixel;
}

int
flush_expose( Window window )
{
   XEvent dummy;
   int i = 0;

   while ( 0 != XCheckTypedWindowEvent( da_display, window, Expose, &dummy ) )
   {
      ++i;
   }

   return i;
}

void
redraw_window( void )
{
   flush_expose( da_icon_window );

   XCopyArea( da_display,
	      da_window_icon.pixmap,
	      da_icon_window,
	      da_normal_gc,
	      0,
	      0,
	      da_window_icon.attributes.width,
	      da_window_icon.attributes.height,
	      0,
	      0 );

  flush_expose( da_window );

  XCopyArea( da_display,
	     da_window_icon.pixmap,
	     da_window,
	     da_normal_gc,
	     0,
	     0,
	     da_window_icon.attributes.width,
	     da_window_icon.attributes.height,
	     0,
	     0 );
}

void
redraw_window_coords( int x, int y )
{
   flush_expose( da_icon_window );

   XCopyArea( da_display,
	      da_window_icon.pixmap,
	      da_icon_window,
	      da_normal_gc,
	      x,
	      y,
	      da_window_icon.attributes.width,
	      da_window_icon.attributes.height,
	      0,
	      0 );

   flush_expose( da_window );

   XCopyArea( da_display,
	      da_window_icon.pixmap,
	      da_window,
	      da_normal_gc,
	      x,
	      y,
	      da_window_icon.attributes.width,
	      da_window_icon.attributes.height,
	      0,
	      0 );
}

void
add_mouse_region( int m_index,
		  int m_left,
		  int m_top,
		  int m_right,
		  int m_bottom )
{
   if ( MAX_MOUSE_REGION > m_index )
   {
      mouse_region[m_index].enable = 1;
      mouse_region[m_index].top = m_top;
      mouse_region[m_index].left = m_left;
      mouse_region[m_index].bottom = m_bottom;
      mouse_region[m_index].right = m_right;
   }
}

int
check_mouse_region( int x, int y )
{
   int i;
   int found;

   found = 0;

   for ( i = 0; MAX_MOUSE_REGION > i && 0 == found; ++i )
   {
      if ( 0 != mouse_region[i].enable &&
	   x <= mouse_region[i].right &&
	   x >= mouse_region[i].left &&
	   y <= mouse_region[i].bottom &&
	   y >= mouse_region[i].top )
      {
	 found = 1;
      }
   }

  if ( 0 == found )
  {
     return -1;
  }

  return --i;
}

void
create_xbm_from_xpm( char* xbm, char** xpm, int sx, int sy )
{
   int i;
   int j;
   int k;
   int width;
   int height;
   int numcol;
   int depth;
   int zero = 0;
   unsigned char bwrite;
   int bcount;
   int curpixel;

   sscanf( *xpm, "%d %d %d %d", &width, &height, &numcol, &depth );

   for ( k = 0; k != depth; ++k )
   {
      zero <<= 8;
      zero |= xpm[2][k];
   }

   for ( i = numcol + 1; i < ( numcol + sy + 1 ); ++i )
   {
      bcount = 0;
      bwrite = 0;

      for ( j = 0; j < sx * depth; j += depth )
      {
	 bwrite >>= 1;

	 curpixel = 0;

	 for ( k = 0; k != depth; ++k )
	 {
	    curpixel <<= 8;
	    curpixel |= xpm[i][j + k];
	 }

	 if ( curpixel != zero )
	 {
	    bwrite += 128;
	 }

	 bcount++;

	 if ( bcount == 8 )
	 {
	    *xbm = bwrite;
	    ++xbm;
	    bcount = 0;
	    bwrite = 0;
	 }
      }
   }
}

void
copy_xpm_area( int x,
	       int y,
	       unsigned int sx,
	       unsigned int sy,
	       int dx,
	       int dy )
{
   XCopyArea( da_display,
	      da_window_icon.pixmap,
	      da_window_icon.pixmap,
	      da_normal_gc,
	      x,
	      y,
	      sx,
	      sy,
	      dx,
	      dy );
}

void
copy_xbm_area( int x,
	       int y,
	       unsigned int sx,
	       unsigned int sy,
	       int dx,
	       int dy )
{
   XCopyArea( da_display,
	      da_window_icon.mask,
	      da_window_icon.pixmap,
	      da_normal_gc,
	      x,
	      y,
	      sx,
	      sy,
	      dx,
	      dy );
}

void
set_mask_coords( int x, int y )
{
   XShapeCombineMask( da_display,
		      da_window,
		      ShapeBounding,
		      x,
		      y,
		      da_pixmask,
		      ShapeSet );
   XShapeCombineMask( da_display,
		      da_icon_window,
		      ShapeBounding,
		      x,
		      y,
		      da_pixmask,
		      ShapeSet );
}

void
open_window( int argc,
	     char** argv,
	     char** pixmap_bytes,
	     char* pixmask_bits,
	     int pixmask_width,
	     int pixmask_height )
{
   unsigned int borderwidth = 1;
   XClassHint classHint;
   char* display_name = NULL;
   char* wname = NULL;
   XTextProperty name;
   XGCValues gcv;
   unsigned long gcm;
   char* geometry = NULL;
   int dummy = 0;
   int i;
   int wx;
   int wy;

   wname = argv[0];

   for ( i = 1; i < argc; ++i )
   {
      if ( 0 == strcmp( argv[i], "-display" ) )
      {
	 display_name = argv[++i];
      }

      if ( 0 == strcmp( argv[i], "-geometry" ) )
      {
	 geometry = argv[++i];
      }
   }

   da_display = XOpenDisplay( display_name );

   if ( NULL == da_display )
   {
      fprintf( stderr,
	       "%s: Can't open display: '%s'\n",
	       wname,
	       XDisplayName( display_name ) );
      exit( 1 );
   }

   da_screen = DefaultScreen( da_display );
   da_root_window = RootWindow( da_display, da_screen );
   da_display_depth = DefaultDepth( da_display, da_screen );
   da_x_fd = XConnectionNumber( da_display );

   /* Convert XPM to XImage */
   get_xpm( &da_window_icon, pixmap_bytes );

   /* Create a window to hold the stuff */
   da_size_hints.flags = USSize | USPosition;
   da_size_hints.x = 0;
   da_size_hints.y = 0;

   da_bg_pixel = get_color_by_name( "white" );
   da_fg_pixel = get_color_by_name( "black" );

   XWMGeometry( da_display,
		da_screen,
		da_geometry,
		NULL,
		borderwidth,
		&da_size_hints,
		&da_size_hints.x,
		&da_size_hints.y,
		&da_size_hints.width,
		&da_size_hints.height,
		&dummy );

   da_size_hints.width = 64;
   da_size_hints.height = 64;

   da_window = XCreateSimpleWindow( da_display,
				    da_root_window,
				    da_size_hints.x,
				    da_size_hints.y,
				    da_size_hints.width,
				    da_size_hints.height,
				    borderwidth,
				    da_fg_pixel,
				    da_bg_pixel );

   da_icon_window = XCreateSimpleWindow( da_display,
					 da_window,
					 da_size_hints.x,
					 da_size_hints.y,
					 da_size_hints.width,
					 da_size_hints.height,
					 borderwidth,
					 da_fg_pixel,
					 da_bg_pixel );

   /* Activate hints */
   XSetWMNormalHints( da_display, da_window, &da_size_hints );
   classHint.res_name = wname;
   classHint.res_class = wname;
   XSetClassHint( da_display, da_window, &classHint );
   XSelectInput( da_display,
		 da_window,
		 ( ButtonPressMask |
		   ExposureMask |
		   ButtonReleaseMask |
		   PointerMotionMask |
		   StructureNotifyMask ) );
   XSelectInput( da_display,
		 da_icon_window,
		 ( ButtonPressMask |
		   ExposureMask |
		   ButtonReleaseMask |
		   PointerMotionMask |
		   StructureNotifyMask ) );

   if ( 0 == XStringListToTextProperty( &wname, 1, &name ) )
   {
      fprintf( stderr, "%s: can't allocate window name\n", wname );
      exit( 1 );
   }

   XSetWMName( da_display, da_window, &name );

   /* Create GC for drawing */
   gcm = GCForeground | GCBackground | GCGraphicsExposures;
   gcv.foreground = da_fg_pixel;
   gcv.background = da_bg_pixel;
   gcv.graphics_exposures = 0;
   da_normal_gc = XCreateGC( da_display, da_root_window, gcm, &gcv );

   /* ONLYSHAPE ON */
   da_pixmask = XCreateBitmapFromData( da_display,
				       da_window,
				       pixmask_bits,
				       pixmask_width,
				       pixmask_height );

   XShapeCombineMask( da_display,
		      da_window,
		      ShapeBounding,
		      0,
		      0,
		      da_pixmask,
		      ShapeSet );
   XShapeCombineMask( da_display,
		      da_icon_window,
		      ShapeBounding,
		      0,
		      0,
		      da_pixmask,
		      ShapeSet );

   /* ONLYSHAPE OFF */
   da_wm_hints.initial_state = WithdrawnState;
   da_wm_hints.icon_window = da_icon_window;
   da_wm_hints.icon_x = da_size_hints.x;
   da_wm_hints.icon_y = da_size_hints.y;
   da_wm_hints.window_group = da_window;
   da_wm_hints.flags = ( StateHint |
			 IconWindowHint |
			 IconPositionHint |
			 WindowGroupHint );

   XSetWMHints( da_display, da_window, &da_wm_hints );
   XSetCommand( da_display, da_window, argv, argc );
   XMapWindow( da_display, da_window );

   if ( NULL != geometry )
    {
       if ( 2 != sscanf( geometry, "+%d+%d", &wx, &wy ) )
	{
	   fprintf( stderr, "Bad geometry string.\n" );
	   exit( 1 );
	}

       XMoveWindow( da_display, da_window, wx, wy );
    }

   if ( NULL != display_name )
   {
      free( display_name );
   }
}
