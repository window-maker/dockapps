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

#ifndef __dockapp_draw_h
#define __dockapp_draw_h

#define DOCKAPP_WIDTH 64
#define DOCKAPP_HEIGHT 64

#define DOCKAPP_BIG_CHAR_WIDTH 6
#define DOCKAPP_BIG_CHAR_HEIGHT 8
#define DOCKAPP_SMALL_CHAR_WIDTH 5
#define DOCKAPP_SMALL_CHAR_HEIGHT 6

#define DOCKAPP_BIG_DIGIT_X 3
#define DOCKAPP_BIG_DIGIT_Y 65
#define DOCKAPP_SMALL_DIGIT_X 3
#define DOCKAPP_SMALL_DIGIT_Y 86

#define DOCKAPP_BIG_LETTER_X 3
#define DOCKAPP_BIG_LETTER_Y 75
#define DOCKAPP_SMALL_LETTER_X 3
#define DOCKAPP_SMALL_LETTER_Y 94

#define DOCKAPP_DOT_X 64
#define DOCKAPP_DOT_Y 85

#define DOCKAPP_DOT_WIDTH 2
#define DOCKAPP_DOT_HEIGHT 6

#if 0
#define DOCKAPP_BAR_WIDTH 55
#define DOCKAPP_BAR_HEIGHT 2

#define DOCKAPP_BAR_ON_X 82
#define DOCKAPP_BAR_ON_Y 71
#define DOCKAPP_BAR_OFF_X 82
#define DOCKAPP_BAR_OFF_Y 69
#endif

/*
 * Function protoypes.
 */

void dockapp_draw_small_digit( unsigned int, int, int, int );

void dockapp_draw_big_digit( unsigned int, int, int, int );

void dockapp_draw_big_str( const char*, int, int );

void dockapp_draw_small_str( const char*, int, int );

#if 0
void dockapp_draw_bar( int, int, int, int, int );

void dockapp_draw_bar_calculate( float, int, int );
#endif

void dockapp_draw_data( void );

#endif /* !__dockapp_draw_h */
