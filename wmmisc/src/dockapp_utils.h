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

#ifndef __dockapp_utils_h
#define __dockapp_utils_h

#include <math.h>

typedef struct
{
      unsigned int users;
      unsigned int total;
      unsigned int running;
      int load[2];

      float jiffies;
      int seconds;
      int minutes;
      int hours;
      int days;
      int weeks;
} dockapp_proc_t;

/*
 * Define some math macros.
 */

/* Integer math. */
#define GET_SECS( j ) ( ( int ) nearbyint( j ) % 60 )
#define GET_MINS( j ) ( ( int ) ( nearbyint( j ) / 60 ) % 60 )
#define GET_HRS( j ) ( ( int ) ( nearbyint( j ) / 3600 ) % 24 )
#define GET_DAYS( j ) ( ( int ) ( nearbyint( j ) / 86400 ) % 7 )
#define GET_WEEKS( j ) ( ( int ) nearbyint( j ) / 604800 )

/* Floating-point math. */
#define GET_SECS_F( j ) ( fmodf( j, 60 ) )
#define GET_MINS_F( j ) ( fmodf( ( j / 60 ), 60 ) )
#define GET_HRS_F( j ) ( fmodf( ( j / 3600 ), 24 ) )
#define GET_DAYS_F( j ) ( fmodf( ( j / 86400 ), 7 ) )
#define GET_WEEKS_F( j ) ( ( float ) j / 604800 )

/*
 * Function prototypes.
 */

dockapp_proc_t dockapp_utils_get_proc( void );

void dockapp_utils_get_users( void );

int dockapp_utils_get_char( char );

#endif /* !__dockapp_utils_h */
