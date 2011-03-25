/*
 * dockapp.h
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

#ifndef DOCKAPP_H
#define DOCKAPP_H 1

#include <X11/xpm.h>

typedef struct XpmIcon
{
  XpmAttributes attributes;
  Pixmap shapemask;
  Pixmap image;
} XpmIcon;

typedef struct dockapp_t
{
  Display *display;
  Window  root_win;
  int     screen;
  int     d_depth;
  Pixel   back_pix;
  Pixel   fore_pix;
  Window  iconwin;
  Window  win;
  GC      NormalGC;
  XpmIcon xpm_icon;
} dockapp_t;

void
InitDockAppWindow( int argc, char *argv[], char *pixmap_data[],
		   char *display_arg, char *geometry_arg );

void
RedrawWindow( void );

void
copyXPMArea( int x, int y, unsigned int sx, unsigned int sy, int dx, int dy );

/* Exported variables */
#undef _SCOPE_
#ifdef DOCKAPP_M
#define _SCOPE_ /**/
#else
#define _SCOPE_ extern
#endif

_SCOPE_ dockapp_t dockapp;

#endif /* DOCKAPP_H */
