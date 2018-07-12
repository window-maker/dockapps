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

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include <libdockapp/wmgeneral.h>
#include "dockapp_draw.h"
#include "dockapp_utils.h"

static dockapp_proc_t dockapp_proc;

void
dockapp_draw_big_digit( unsigned int digit_value,
			int digit_zero_count,
			int digit_draw_location_x,
			int digit_draw_location_y )
{
   int digit_value_of_ten = 1;
   int digit_maximum_value_of_ten = 0;
   int digit_new_draw_location_x = 0;

   if ( ( ( DOCKAPP_WIDTH - DOCKAPP_BIG_CHAR_WIDTH ) < digit_draw_location_x
	  || ( DOCKAPP_HEIGHT - DOCKAPP_BIG_CHAR_HEIGHT ) < digit_draw_location_y )
	|| ( 0 > digit_draw_location_x || 0 > digit_draw_location_y ) )
   {
      fprintf( stderr,
	       "%s: Invalid x,y position: %d,%d\n",
	       __func__,
	       digit_draw_location_x,
	       digit_draw_location_y );
      return;
   }

   for ( digit_maximum_value_of_ten = 0;
	 digit_maximum_value_of_ten < digit_zero_count;
	 ++digit_maximum_value_of_ten )
   {
      digit_value_of_ten *= 10;
   }

   digit_new_draw_location_x = digit_draw_location_x;

   do
   {
      copyXPMArea( ( ( ( digit_value / digit_value_of_ten ) % 10 ) *
		       DOCKAPP_BIG_CHAR_WIDTH + DOCKAPP_BIG_DIGIT_X ),
		     DOCKAPP_BIG_DIGIT_Y,
		     DOCKAPP_BIG_CHAR_WIDTH,
		     DOCKAPP_BIG_CHAR_HEIGHT,
		     digit_new_draw_location_x,
		     digit_draw_location_y );

      digit_value_of_ten /= 10;
      digit_new_draw_location_x += DOCKAPP_BIG_CHAR_WIDTH;
   } while ( 0 < digit_value_of_ten );

  RedrawWindow();
}

void
dockapp_draw_small_digit( unsigned int d_value,
			  int d_zero,
			  int digit_draw_location_x,
			  int digit_draw_location_y )
{
   int digit_value_of_ten = 1;
   int digit_maximum_value_of_ten = 0;
   int digit_new_draw_location_x = 0;

   if ( ( ( DOCKAPP_WIDTH - DOCKAPP_BIG_CHAR_WIDTH ) < digit_draw_location_x
	  || ( DOCKAPP_HEIGHT - DOCKAPP_BIG_CHAR_HEIGHT ) < digit_draw_location_y )
	|| ( 0 > digit_draw_location_x || 0 > digit_draw_location_y ) )
   {
      fprintf( stderr,
	       "%s: Invalid x,y position: %d,%d\n",
	       __func__,
	       digit_draw_location_x,
	       digit_draw_location_y );
      return;
   }

   for ( digit_maximum_value_of_ten = 0;
	 digit_maximum_value_of_ten < d_zero;
	 ++digit_maximum_value_of_ten )
   {
      digit_value_of_ten *= 10;
   }

   digit_new_draw_location_x = digit_draw_location_x;

   do
   {
      copyXPMArea( ( ( ( d_value / digit_value_of_ten) % 10) *
		       DOCKAPP_SMALL_CHAR_WIDTH + DOCKAPP_SMALL_DIGIT_X ),
		     DOCKAPP_SMALL_DIGIT_Y,
		     DOCKAPP_SMALL_CHAR_WIDTH,
		     DOCKAPP_SMALL_CHAR_HEIGHT,
		     digit_new_draw_location_x,
		     digit_draw_location_y );

      digit_value_of_ten /= 10;
      digit_new_draw_location_x += DOCKAPP_SMALL_CHAR_WIDTH;
   } while ( 0 < digit_value_of_ten );

  RedrawWindow();
}

void
dockapp_draw_big_str( const char* string_to_draw,
		      int digit_draw_location_x,
		      int digit_draw_location_y )
{
   int string_length = 0;
   int string_character_position = 0;
   int string_character = 0;
   int digit_new_draw_location_x = 0;

   string_length = strlen( string_to_draw );

   if ( ( ( DOCKAPP_WIDTH - DOCKAPP_BIG_CHAR_WIDTH ) < digit_draw_location_x
	  || ( DOCKAPP_HEIGHT - DOCKAPP_BIG_CHAR_HEIGHT ) < digit_draw_location_y )
	|| ( 0 > digit_draw_location_x || 0 > digit_draw_location_y ) )
    {
       fprintf( stderr,
		"%s: Invalid x,y position: %d,%d\n",
		__func__,
		digit_draw_location_x,
		digit_draw_location_y );

       return;
    }
   else if ( 0 == string_length )
   {
      fprintf( stderr, "%s: Draw string is empty!\n", __func__ );
      return;
   }
   else if ( ( DOCKAPP_WIDTH / DOCKAPP_BIG_CHAR_WIDTH ) < string_length )
   {
      fprintf( stderr, "%s: Draw string is too long!", __func__ );
      return;
   }

   digit_new_draw_location_x = digit_draw_location_x;

   for ( string_character_position = 0;
	 string_character_position < string_length;
	 ++string_character_position )
   {
      string_character = dockapp_utils_get_char( string_to_draw[string_character_position] );

      copyXPMArea( string_character * DOCKAPP_BIG_CHAR_WIDTH + DOCKAPP_BIG_LETTER_X,
		     DOCKAPP_BIG_LETTER_Y,
		     DOCKAPP_BIG_CHAR_WIDTH,
		     DOCKAPP_BIG_CHAR_HEIGHT,
		     digit_new_draw_location_x,
		     digit_draw_location_y );

      digit_new_draw_location_x += DOCKAPP_BIG_CHAR_WIDTH;
   }

  RedrawWindow();
}

void
dockapp_draw_small_str( const char* string_to_draw,
			int digit_draw_location_x,
			int digit_draw_location_y )
{
   int string_length = 0;
   int string_character_position = 0;
   int string_character = 0;
   int digit_new_draw_location_x = 0;

   string_length = strlen( string_to_draw );

   if ( ( ( DOCKAPP_WIDTH - DOCKAPP_BIG_CHAR_WIDTH ) < digit_draw_location_x
	  || ( DOCKAPP_HEIGHT - DOCKAPP_BIG_CHAR_HEIGHT ) < digit_draw_location_y )
	|| ( 0 > digit_draw_location_x || 0 > digit_draw_location_y ) )
    {
       fprintf( stderr,
		"%s: Invalid x,y position: %d,%d\n",
		__func__,
		digit_draw_location_x,
		digit_draw_location_y );
       return;
    }
   else if ( 0 == string_length )
   {
      fprintf( stderr, "%s: Draw string is empty!\n", __func__ );
      return;
   }
   else if ( ( DOCKAPP_WIDTH / DOCKAPP_SMALL_CHAR_WIDTH ) < string_length )
   {
      fprintf( stderr, "%s: Draw string is too long!", __func__ );
      return;
   }

   digit_new_draw_location_x = digit_draw_location_x;

   for (string_character_position = 0;
	string_character_position < string_length;
	++string_character_position )
   {
      string_character = dockapp_utils_get_char( string_to_draw[string_character_position] );

      copyXPMArea( string_character * DOCKAPP_SMALL_CHAR_WIDTH + DOCKAPP_SMALL_LETTER_X,
		     DOCKAPP_SMALL_LETTER_Y,
		     DOCKAPP_SMALL_CHAR_WIDTH,
		     DOCKAPP_SMALL_CHAR_HEIGHT,
		     digit_new_draw_location_x,
		     digit_draw_location_y );

      digit_new_draw_location_x += DOCKAPP_SMALL_CHAR_WIDTH;
   }

  RedrawWindow();
}

#if 0
void
dockapp_draw_bar( int bar_draw_width,
		  int bar_draw_x,
                  int bar_draw_y,
		  int bar_x,
		  int bar_y )
{
   if ( 0 > bar_draw_width || DOCKAPP_BAR_WIDTH < bar_draw_width )
   {
      fprintf( stderr, "%s: Invalid bar width!\n", __func__ );
      return;
   }
   else if ( ( 0 > bar_draw_x || DOCKAPP_WIDTH < bar_draw_x ) ||
	     ( 0 > bar_draw_y || DOCKAPP_WIDTH < bar_draw_y ) )
   {
      fprintf( stderr,
	       "%s: Invalid x,y position: %d,%d\n",
	       __func__,
               bar_draw_x,
	       bar_draw_y );
      return;
   }
   else if ( 0 >= bar_x || 0 >= bar_y )
   {
      fprintf( stderr,
	       "%s: Invalid x,y position: %d,%d\n",
	       __func__,
               bar_x,
	       bar_y );
      return;
   }

  copyXPMArea( bar_x,
		 bar_y,
		 bar_draw_width,
		 DOCKAPP_BAR_HEIGHT,
		 bar_draw_x,
		 bar_draw_y );

  RedrawWindow();
}

void
dockapp_draw_bar_calculate( float draw_size, int bar_draw_x, int bar_draw_y )
{
   float draw_percent_f;
   int draw_percent;

   draw_percent_f = ( GET_HRS_F( draw_size ) / DOCKAPP_BAR_WIDTH ) * 100.0f;
   draw_percent = ( int ) nearbyint( draw_percent_f );

  if ( DOCKAPP_BAR_WIDTH == draw_percent )
  {
     dockapp_draw_bar( draw_percent,
		       bar_draw_x,
		       bar_draw_y,
		       DOCKAPP_BAR_OFF_X,
		       DOCKAPP_BAR_OFF_Y );
  }
  else
  {
     dockapp_draw_bar( draw_percent,
		       bar_draw_x,
		       bar_draw_y,
		       DOCKAPP_BAR_ON_X,
		       DOCKAPP_BAR_ON_Y );
  }
}
#endif

void
dockapp_draw_data( void )
{
   dockapp_proc = dockapp_utils_get_proc();

   dockapp_draw_small_digit( dockapp_proc.users, 2, 45, 4 );
   dockapp_draw_small_digit( dockapp_proc.total, 2, 45, 11 );
   dockapp_draw_small_digit( dockapp_proc.running, 2, 45, 18 );

   dockapp_draw_small_digit( dockapp_proc.hours, 1, 36, 28 );
   dockapp_draw_small_digit( dockapp_proc.minutes, 1, 50, 28 );

   dockapp_draw_small_digit( dockapp_proc.days, 0, 55, 37 );
   dockapp_draw_small_digit( dockapp_proc.weeks, 2, 45, 44 );

   dockapp_draw_small_digit( dockapp_proc.load[0], 1, 36, 53 );
   dockapp_draw_small_digit( dockapp_proc.load[1], 1, 50, 53 );

   RedrawWindow();
}
