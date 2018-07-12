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
#include <unistd.h>
#include <bits/getopt_core.h>
#include <signal.h>

#ifdef USE_MTRACE
#include <mcheck.h>
#endif /* USE_MTRACE */

#include <libdockapp/wmgeneral.h>

#include "wmmisc-master.xpm"
#include "dockapp_main.h"
#include "dockapp_draw.h"

void
dockapp_show_help( const char* wm_name )
{
   printf( "Usage: %s [OPTION]...\n", wm_name );
   printf( "This is a simple WindowMaker 'DockApp' that will monitor the following:\n" );
   printf( "Number of users logged in, total processes, total number of processes running,\n" );
   printf( "the system's fork count and load average.\n\n" );
   printf( "                -h       display this help and exit\n" );
   printf( "                -v       output version information and exit\n\n" );
   printf( "Report bugs to <" DOCKAPP_AUTHOR_EMAIL ">.\n" );
}

void
dockapp_show_version( void )
{
   printf( "%s %d.%d\n",
	   DOCKAPP_NAME,
	   DOCKAPP_MAJOR_VERSION,
	   DOCKAPP_MINOR_VERSION );
   printf( "Written by %s <%s>\n\n", DOCKAPP_AUTHOR, DOCKAPP_AUTHOR_EMAIL );
   printf( "Copyright (C) 2003-2006 %s\n", DOCKAPP_AUTHOR );
   printf( "This is free software; see the source for copying conditions.  There is NO\n" );
   printf( "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n" );
}

void
dockapp_exit( int signal_received )
{
#ifdef USE_MTRACE
   muntrace();
#endif /* USE_MTRACE */

#ifdef DEBUG
   fprintf( stderr,
	    "%s: Exiting cleanly with signal %d\n",
	    DOCKAPP_NAME,
	    signal_received );
#endif /* DEBUG */

   exit( 0 );
}

void
dockapp_crash( int signal_received )
{
   fprintf( stderr,
	    "%s: got signal %d -- bailing out!\n",
	    DOCKAPP_NAME,
	    signal_received );
   fprintf( stderr,
	    "It appears that %s has encountered an error.\n",
	    DOCKAPP_NAME );
   fprintf( stderr,
	    "If this continues to happen, please run '%s core' from a debugger,\n",
	    DOCKAPP_NAME );
   fprintf( stderr,
	    "such as gdb, and report the output to <%s>.\n",
	    DOCKAPP_AUTHOR_EMAIL );

   abort();
}

int
main( int argc, char** argv )
{
   int opt = 0;

#ifdef USE_MTRACE
   mtrace();
#endif /* USE_MTRACE */

   signal( SIGINT, dockapp_exit );
   signal( SIGSEGV, dockapp_crash );

   opt = getopt( argc, argv, "hv" );

   while ( -1 != opt )
   {
      switch ( opt )
      {
	 case 'h':
	 {
	    dockapp_show_help( argv[0] );
	    exit( 1 );
	    /* Never reached. */
	 }

	 case 'v':
	 {
	    dockapp_show_version();
	    exit( 1 );
	    /* Never reached. */
	 }
      }

      opt = getopt( argc, argv, "hv" );
   }

   createXBMfromXPM( wmmisc_mask_bits,
			wmmisc_master_xpm,
			wmmisc_mask_width,
			wmmisc_mask_height );

   openXwindow( argc,
		argv,
		wmmisc_master_xpm,
		wmmisc_mask_bits,
		wmmisc_mask_width,
		wmmisc_mask_height );

   RedrawWindow();

   dockapp_draw_small_str( "USERS", 4, 4 );
   dockapp_draw_small_str( "PROCS", 4, 11 );
   dockapp_draw_small_str( "ACTIVE", 4, 18 );

   dockapp_draw_small_str( "UP", 4, 28 );
   dockapp_draw_small_str( "DAYS", 4, 37 );
   dockapp_draw_small_str( "WEEKS", 4, 44 );

   dockapp_draw_small_str( "LOAD", 4, 53 );

   do
   {
      dockapp_draw_data();
      usleep( 250000L );
   } while( 1 );

   return 0;
}
