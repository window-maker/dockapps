
#ifndef _REGIONS_H
#define _REGIONS_H

#include <X11/X.h>

void region_init( Display *dpy );

void region_add( Window win, int id, int x, int y, int w, int h,
				void (*mouse_in)(int), void (*mouse_out)(int), void (*mouse_click)(int, unsigned int) );

void region_enable( Window win, int id );
void region_disable( Window win, int id );
bool region_in( Window win, int x, int y );
void region_mouse_motion( Window win, int x, int y );
void region_mouse_click( Window win, int x, int y, unsigned int button );

#endif
