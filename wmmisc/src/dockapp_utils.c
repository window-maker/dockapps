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

#include <bits/types/time_t.h>
#include <paths.h>
#include <stdio.h>
#include <sys/stat.h>
#include <utmp.h>

#include "dockapp_utils.h"

static dockapp_proc_t dockapp_proc;

dockapp_proc_t
dockapp_utils_get_proc( void )
{
   FILE* pla_buf = NULL;
   int p_load[2];
   int p_run = 0;
   int p_total = 0;
   int count;
   FILE* put_buf = NULL;
   float p_uptime = 0.00;

   dockapp_utils_get_users();

   pla_buf = fopen( "/proc/loadavg", "r" );

   if ( NULL != pla_buf )
   {
      count = fscanf( pla_buf,
	      "%d.%d %*s %*s %d/%d %*s",
	      &p_load[0],
	      &p_load[1],
	      &p_run,
	      &p_total );
      fclose( pla_buf );

      if (count == 4) {
	      dockapp_proc.total = p_total;
	      dockapp_proc.running = p_run;
	      dockapp_proc.load[0] = p_load[0];
	      dockapp_proc.load[1] = p_load[1];
      } else {
	      dockapp_proc.total = 0;
	      dockapp_proc.running = 0;
	      dockapp_proc.load[0] = 0;
	      dockapp_proc.load[1] = 0;
      }
   }
   else
   {
      dockapp_proc.total = 0;
      dockapp_proc.running = 0;
      dockapp_proc.load[0] = 0;
      dockapp_proc.load[1] = 0;
   }

   put_buf = fopen( "/proc/uptime", "r" );

  if ( NULL != put_buf )
  {
     count = fscanf( put_buf, "%f %*s", &p_uptime );
     fclose( put_buf );

     if (count == 1) {
	     dockapp_proc.jiffies = p_uptime;
	     dockapp_proc.seconds = GET_SECS( p_uptime );
	     dockapp_proc.minutes = GET_MINS( p_uptime );
	     dockapp_proc.hours = GET_HRS( p_uptime );
	     dockapp_proc.days = GET_DAYS( p_uptime );
	     dockapp_proc.weeks = GET_WEEKS( p_uptime );
     } else {
	     dockapp_proc.jiffies = 0.00;
	     dockapp_proc.seconds = 0;
	     dockapp_proc.minutes = 0;
	     dockapp_proc.hours = 0;
	     dockapp_proc.days = 0;
	     dockapp_proc.weeks = 0;
     }

#if 0
     printf( "Uptime: %.2d:%.2d:%.2d, %d day%c, %.3d week%c\n",
	     dockapp_proc.hours,
	     dockapp_proc.minutes,
	     dockapp_proc.seconds,
	     dockapp_proc.days,
	     ( ( dockapp_proc.days > 1 || !dockapp_proc.days ) ? 's' : '\0' ),
	     dockapp_proc.weeks,
	     ( ( dockapp_proc.weeks > 1 || !dockapp_proc.weeks ) ? 's' : '\0' ) );
#endif
  }
  else
  {
     dockapp_proc.jiffies = 0.00;
     dockapp_proc.seconds = 0;
     dockapp_proc.minutes = 0;
     dockapp_proc.hours = 0;
     dockapp_proc.days = 0;
     dockapp_proc.weeks = 0;
  }

  return dockapp_proc;
}

/*
 * Get the number of users logged onto the system.
 *
 * This code was taken from gkrellm and modified for
 * this program.
 */

void
dockapp_utils_get_users( void )
{
   struct utmp* ut;
   struct stat stt;
   static time_t utmp_mtime;
   int p_users = 0;

   if ( 0 == stat( _PATH_UTMP, &stt ) && stt.st_mtime != utmp_mtime )
   {
      setutent();

      ut = getutent();

      while ( NULL != ut )
      {
	 if ( USER_PROCESS == ut->ut_type && '\0' != ut->ut_name[0] )
	 {
	    ++p_users;
	 }

	 ut = getutent();
      }

      endutent();

      utmp_mtime = stt.st_mtime;
      dockapp_proc.users = p_users;
   }
}

int
dockapp_utils_get_char( char d_char )
{
   int n_char = 0;

   switch ( d_char )
   {
      case 'a':
      case 'A':
      {
	 n_char = 0;
	 break;
      }

      case 'b':
      case 'B':
      {
	 n_char = 1;
	 break;
      }

      case 'c':
      case 'C':
      {
	 n_char = 2;
	 break;
      }

      case 'd':
      case 'D':
      {
	 n_char = 3;
	 break;
      }

      case 'e':
      case 'E':
      {
	 n_char = 4;
	 break;
      }

      case 'f':
      case 'F':
      {
	 n_char = 5;
	 break;
      }

      case 'g':
      case 'G':
      {
	 n_char = 6;
	 break;
      }

      case 'h':
      case 'H':
      {
	 n_char = 7;
	 break;
      }

      case 'i':
      case 'I':
      {
	 n_char = 8;
	 break;
      }

      case 'j':
      case 'J':
      {
	 n_char = 9;
	 break;
      }

      case 'k':
      case 'K':
      {
	 n_char = 10;
	 break;
      }

      case 'l':
      case 'L':
      {
	 n_char = 11;
	 break;
      }

      case 'm':
      case 'M':
      {
	 n_char = 12;
	 break;
      }

      case 'n':
      case 'N':
      {
	 n_char = 13;
	 break;
      }

      case 'o':
      case 'O':
      {
	 n_char = 14;
	 break;
      }

      case 'p':
      case 'P':
      {
	 n_char = 15;
	 break;
      }

      case 'q':
      case 'Q':
      {
	 n_char = 16;
	 break;
      }

      case 'r':
      case 'R':
      {
	 n_char = 17;
	 break;
      }

      case 's':
      case 'S':
      {
	 n_char = 18;
	 break;
      }

      case 't':
      case 'T':
      {
	 n_char = 19;
	 break;
      }

      case 'u':
      case 'U':
      {
	 n_char = 20;
	 break;
      }

      case 'v':
      case 'V':
      {
	 n_char = 21;
	 break;
      }

      case 'w':
      case 'W':
      {
	 n_char = 22;
	 break;
      }

      case 'x':
      case 'X':
      {
	 n_char = 23;
	 break;
      }

      case 'y':
      case 'Y':
      {
	 n_char = 24;
	 break;
      }

      case 'z':
      case 'Z':
      {
	 n_char = 25;
	 break;
      }

      default:
      {
	 n_char = 26; /* Blank. */
	 break;
      }
   }

   return n_char;
}
