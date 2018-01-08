#include <string.h>
#include <X11/Xlib.h>
#include "regions.h"

struct region
{
	int id;
	int x, y, w, h;
	bool enabled;

	void (*mouse_in)( int );
	void (*mouse_out)( int );
	void (*mouse_click)( int, unsigned int );

	struct region *next;
};

struct regioned_window
{
	Window win;
	struct region *first_reg;
	struct regioned_window *next;
};


static struct regioned_window *regioned_windows=NULL;
static Display *dpy=NULL;


static struct region *region_locate( Window win, int x, int y );
static struct region *region_find( Window win, int id );
static struct regioned_window *region_get_win( Window win );

void region_init( Display *_dpy )
{
	dpy = _dpy;
}

void region_add( Window win, int id, int x, int y, int w, int h,
				void (*mouse_in)(int), void (*mouse_out)(int), void (*mouse_click)(int, unsigned int) )
{
	/* cerate the region and set its fields */
	struct region *reg = new region;
	reg->id = id;
	reg->x = x;
	reg->y = y;
	reg->w = w;
	reg->h = h;
	reg->enabled = true;
	reg->mouse_in = mouse_in;
	reg->mouse_out = mouse_out;
	reg->mouse_click = mouse_click;

	/* find the regioned_win and insert the new region into its list of regions */
	struct regioned_window *reg_win = region_get_win(win);
	reg->next = reg_win->first_reg;
	reg_win->first_reg = reg;
}

void region_enable( Window win, int id )
{
	struct region *reg = region_find( win, id );
	if( reg != NULL )
		reg->enabled = true;
}

void region_disable( Window win, int id )
{
	struct region *reg = region_find( win, id );
	if( reg != NULL )
		reg->enabled = false;
}


bool region_in( Window win, int x, int y )
{
	return (region_locate(win,x,y) != NULL);
}

void region_mouse_motion( Window win, int x, int y )
{
	struct region *reg;
	static struct region *last_active_reg=NULL;
	reg = region_locate( win, x, y );
	if( reg == last_active_reg )
		return;
	if( last_active_reg != NULL )
	{
		last_active_reg->mouse_out( last_active_reg->id );
		XUngrabPointer( dpy, CurrentTime );
	}
	if( reg != NULL )
	{
		reg->mouse_in( reg->id) ;
		XGrabPointer( dpy, win, true, PointerMotionMask|ButtonPress, GrabModeAsync, GrabModeAsync, None, None, CurrentTime );
	}
	last_active_reg = reg;
}

void region_mouse_click( Window win, int x, int y, unsigned int button )
{
	struct region *reg;
	region_mouse_motion( win, x, y );
	if( (reg=region_locate(win,x,y)) != NULL )
		reg->mouse_click(reg->id, button);
}

static struct region *region_locate( Window win, int x, int y )
{
	struct region *reg = region_get_win(win)->first_reg;

	while( reg != NULL )
	{
		if( (x >= reg->x) && (x <= reg->x+reg->w) &&
		    (y >= reg->y) && (y <= reg->y+reg->h ) &&
		    reg->enabled )
			return reg;
		reg = reg->next;
	}
	return NULL;
}

static struct region *region_find( Window win, int id )
{
	struct region *reg = region_get_win(win)->first_reg;

	while( reg != NULL )
	{
		if( reg->id == id )
			return reg;
		reg = reg->next;
	}
	return NULL;
}

static struct regioned_window *region_get_win( Window win )
{
	struct regioned_window *reg_win = regioned_windows;

	while( reg_win != NULL )
	{
		if( reg_win->win == win )
			return reg_win;
		reg_win = reg_win->next;
	}

	reg_win = new regioned_window;
	reg_win->win = win;
	reg_win->first_reg = NULL;
	reg_win->next = regioned_windows;
	regioned_windows = reg_win;
	return reg_win;
}
