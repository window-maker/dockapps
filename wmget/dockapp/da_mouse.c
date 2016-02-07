/*
    wmget - A background download manager as a Window Maker dock app
    Copyright (c) 2001-2003 Aaron Trickey <aaron@amtrickey.net>

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    ********************************************************************
    dockapp/da_mouse.c - Mouse/clickregion handling

    This code keeps track of app-registered ``clickregions'', which
    represent clickable rectangles and button/modifier masks associated
    with app callbacks.
*/

#include <stdio.h>
#include <sys/types.h>
#include "dockapp.h"

#define DOCKAPP_EXPOSE_INTERNALS
#include "da_mouse.h"



typedef struct {
    int x, y, w, h;
    int buttonmask;

    dockapp_rv_t (*cb) (void *, int x, int y);
    void *cbdata;
} da_clickregion;


/* for now we use a simple array of clickregions
 */
#define DA_MAX_CLICKREGIONS 50
static da_clickregion da_clickregions[DA_MAX_CLICKREGIONS];
static size_t da_num_clickregions = 0;


/* a mouse-down simply stores the mouse state here for later processing
 * by a mouse-up
 */
static int da_mouse_down_x;
static int da_mouse_down_y;
static int da_mouse_down_buttons;


void da_mouse_button_down (int x, int y, int buttons)
{
    /* fprintf (stderr, "hello from da_mouse_button_down\n"); */
    da_mouse_down_x = x;
    da_mouse_down_y = y;
    da_mouse_down_buttons = buttons;
}


dockapp_rv_t da_mouse_button_up (int x, int y, int buttons)
{
    da_clickregion *c, *end;

    /*
    fprintf (stderr, "hello from da_mouse_button_up, btns=0x%02x\n",
            buttons);
     */

    end = da_clickregions + da_num_clickregions;

    for (c = da_clickregions; c < end; ++c) {
        if (((buttons & c->buttonmask) == c->buttonmask)
                && x >= c->x && x < c->x + c->w
                && y >= c->y && y < c->y + c->h
                && da_mouse_down_x >= c->x && da_mouse_down_x < c->x + c->w
                && da_mouse_down_y >= c->y && da_mouse_down_y < c->y + c->h) {
            if (c->cb (c->cbdata, x, y) == dockapp_exit)
                return dockapp_exit;
        }
    }

    return dockapp_ok;
}


dockapp_rv_t dockapp_add_clickregion (
        int x, int y, int w, int h,
        int buttonmask,
        dockapp_rv_t (*cb) (void *, int x, int y),
        void *cbdata)
{
    da_clickregion *c;

    if (da_num_clickregions >= DA_MAX_CLICKREGIONS)
        return dockapp_rv_too_many_clickregions;

    c = da_clickregions + da_num_clickregions++;

    c->x = x;
    c->y = y;
    c->w = w;
    c->h = h;
    c->buttonmask = buttonmask;
    c->cb = cb;
    c->cbdata = cbdata;

    return dockapp_ok;
}



