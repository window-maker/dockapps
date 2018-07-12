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

#ifndef __dockapp_main_h
#define __dockapp_main_h

#define DOCKAPP_AUTHOR "Jesse S."
#define DOCKAPP_AUTHOR_EMAIL "luxorfalls@sbcglobal.net"

#define DOCKAPP_NAME "wmmisc"

static char wmmisc_mask_bits[64 * 64];
static int wmmisc_mask_width = 64;
static int wmmisc_mask_height = 64;

/*
 * Function prototypes.
 */

void dockapp_show_help( const char* );

void dockapp_show_version( void );

void dockapp_exit( int );

void dockapp_crash( int );

#endif /* !__dockapp_main_h */
