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

#ifndef __dockapp_wmgeneral_h
#define __dockapp_wmgeneral_h

#define MAX_MOUSE_REGION ( 16 )

typedef struct
{
      Pixmap pixmap;
      Pixmap mask;
      XpmAttributes attributes;
} xpm_icon;

void get_xpm( xpm_icon*, char** );

Pixel get_color_by_name( const char* );

int flush_expose( Window );

void redraw_window( void );

void redraw_window_coords( int, int );

void add_mouse_region( int, int, int, int, int );

int check_mouse_region( int, int );

void create_xbm_from_xpm( char*, char**, int, int );

void copy_xpm_area( int, int, unsigned int, unsigned int, int, int );

void copy_xbm_area( int, int, unsigned int, unsigned int, int, int );

void set_mask_coords( int, int );

void open_window( int, char**, char**, char*, int, int );

#endif /* !__dockapp_wmgeneral_h */
