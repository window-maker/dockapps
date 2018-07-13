/*
 *  draw.c - drawing code
 *
 *  Copyright (C) 2003 Draghicioiu Mihai <misuceldestept@go.ro>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "dockapp.h"
#include "draw.h"
#include "mem_linux.h"
#include "options.h"

void draw_window()
{
 int n;
 long long int mem, swp;
 int mem_percent, swp_percent;

 mem_getfree();
 mem = mem_used;
 if(!opt_buffers) mem -= mem_buffers;
 if(!opt_cache)   mem -= mem_cached;
 swp = swp_used;
 for(n=0;n<25;n++)
 {
  if(n < (mem * 25 / mem_total))
   XCopyArea(display, on, buffer, gc, 0, 0, 2, 11, 7 + n * 2, 27);
  else
   XCopyArea(display, off, buffer, gc, 0, 0, 2, 11, 7 + n * 2, 27);
  if(n < (swp * 25 / swp_total))
   XCopyArea(display, on, buffer, gc, 0, 0, 2, 11, 7 + n * 2, 47);
  else
   XCopyArea(display, off, buffer, gc, 0, 0, 2, 11, 7 + n * 2, 47);
 }
 mem_percent = mem * 100 / mem_total;
 swp_percent = swp * 100 / swp_total;
 if(mem_percent == 100)
  XCopyArea(display, numbers, buffer, gc, 5, 0, 5, 6, 33, 20);
 else
  XCopyArea(display, numbers, buffer, gc, 50, 0, 5, 6, 33, 20);
 if(mem_percent > 9)
  XCopyArea(display, numbers, buffer, gc, 5*((mem_percent%100)/10), 0, 5, 6, 39, 20);
 else
  XCopyArea(display, numbers, buffer, gc, 50, 0, 5, 6, 39, 20);
 XCopyArea(display, numbers, buffer, gc, 5*(mem_percent%10), 0, 5, 6, 45, 20);
 if(swp_percent == 100)
  XCopyArea(display, numbers, buffer, gc, 5, 0, 5, 6, 33, 40);
 else
  XCopyArea(display, numbers, buffer, gc, 50, 0, 5, 6, 33, 40);
 if(swp_percent > 9)
  XCopyArea(display, numbers, buffer, gc, 5*((swp_percent%100)/10), 0, 5, 6, 39, 40);
 else
  XCopyArea(display, numbers, buffer, gc, 50, 0, 5, 6, 39, 40);
 XCopyArea(display, numbers, buffer, gc, 5*(swp_percent%10), 0, 5, 6, 45, 40);
 update_window();
 XSync(display, False);
}
