/*
 *  dockapp.c - dockapp part
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
#include <stdlib.h>
#include <signal.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/shapeconst.h>
#include <X11/cursorfont.h>

#include "wmmemfree.h"
#include "options.h"

#include "xpm/bg.xpm"
#include "xpm/on.xpm"
#include "xpm/off.xpm"
#include "xpm/numbers.xpm"
#include "xpm/panel.xpm"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 64

Display *display;
int      screen;
Window   iconwindow, window, mapwindow;
Colormap colormap;
GC       gc;
Pixmap   background, backgroundmask, on, off, numbers, buffer, panel;
Atom     wm_delete_window;
Atom     _motif_wm_hints;
int      moving, oldx, oldy;
int      screenwidth, screenheight;
Cursor   fleur;

void free_stuff()
{
 XUnmapWindow(display, mapwindow);
 XFreeGC(display, gc);
 XDestroyWindow(display, iconwindow);
 XDestroyWindow(display, window);
 XFreePixmap(display, buffer);
 XFreePixmap(display, numbers);
 XFreePixmap(display, off);
 XFreePixmap(display, on);
 XFreePixmap(display, backgroundmask);
 XFreePixmap(display, background);
 XCloseDisplay(display);
}

void handle_signal(int sig)
{
 switch(sig)
 {
  case SIGINT:
   fprintf(stderr, "Interrupt received\n");
   break;
  case SIGQUIT:
   fprintf(stderr, "Quit signal received\n");
   break;
  case SIGTERM:
   fprintf(stderr, "Terminate signal received\n");
  default:
   fprintf(stderr, "Got signal %d\n", sig);
 }
 stop_timer();
 free_stuff();
 exit(0);
}

void make_window()
{
 Window                rootwindow;
 XpmAttributes         xpmattributes;
 XSetWindowAttributes  windowattributes;
 int                   shapeevent, shapeerror;
 XWMHints             *wmhints;
 XClassHint           *classhint;
 XSizeHints           *sizehints;
 XGCValues             gcvalues;
 unsigned int          depth;
 struct
 {
  long flags;
  long functions;
  long decorations;
  long input_mode;
  long unknown;
 } mwmhints;
 

 display = XOpenDisplay(opt_display);
 if(!display)
 {
  fprintf(stderr, "Could not open display %s\n", opt_display);
  exit(1);
 }
 screen = DefaultScreen(display);
 screenwidth = DisplayWidth(display, screen);
 screenheight = DisplayHeight(display, screen);
 rootwindow = RootWindow(display, screen);
 colormap = DefaultColormap(display, screen);
 depth = DefaultDepth(display, screen);
 xpmattributes.valuemask = XpmColormap | XpmCloseness;
 xpmattributes.colormap = colormap;
 xpmattributes.closeness = 40000;
 XpmCreatePixmapFromData(display, rootwindow, bg_xpm,
                         &background, &backgroundmask, &xpmattributes);
 XpmCreatePixmapFromData(display, rootwindow, on_xpm,
                         &on, None, &xpmattributes);
 XpmCreatePixmapFromData(display, rootwindow, off_xpm,
                         &off, None, &xpmattributes);
 XpmCreatePixmapFromData(display, rootwindow, numbers_xpm,
                         &numbers, None, &xpmattributes);
 buffer = XCreatePixmap(display, rootwindow,
                        WINDOW_WIDTH, WINDOW_HEIGHT, depth);
 windowattributes.background_pixmap = background;
 windowattributes.event_mask = ExposureMask |
                               ButtonPressMask |
			       ButtonReleaseMask |
			       PointerMotionMask |
			       PropertyChangeMask;
 windowattributes.colormap = colormap;
 window = XCreateWindow(display, rootwindow,
                        0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
			CopyFromParent,
			InputOutput,
			CopyFromParent,
			CWBackPixmap | CWEventMask | CWColormap,
			&windowattributes);
 iconwindow = XCreateWindow(display, rootwindow,
                            0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0,
			    CopyFromParent,
			    InputOutput,
			    CopyFromParent,
			    CWBackPixmap | CWEventMask,
			    &windowattributes);
 fleur = XCreateFontCursor(display, XC_fleur);
 if(XShapeQueryExtension(display, &shapeevent, &shapeerror) && opt_shape)
 {
  XShapeCombineMask(display, window, ShapeBounding,
                    0, 0, backgroundmask, ShapeSet);
  XShapeCombineMask(display, iconwindow, ShapeBounding,
                    0, 0, backgroundmask, ShapeSet);
 }
 else
 {  
  XpmCreatePixmapFromData(display, rootwindow, panel_xpm,
                          &panel, None, &xpmattributes);
  gcvalues.function = GXcopy;
  gcvalues.graphics_exposures = False;
  gcvalues.clip_mask = backgroundmask;
  gcvalues.clip_x_origin = 0;
  gcvalues.clip_y_origin = 0;
  gc = XCreateGC(display, rootwindow,
                 GCFunction |
                 GCGraphicsExposures |
                 GCClipMask |
		 GCClipXOrigin |
		 GCClipYOrigin,
		 &gcvalues);
  XCopyArea(display, background, panel, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
  gcvalues.clip_mask = None;
  XChangeGC(display, gc, GCClipMask, &gcvalues);
  XCopyArea(display, panel, background, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
  XFreeGC(display, gc);
  XFreePixmap(display, panel);
 }

 mapwindow = opt_window ? iconwindow : window;
 wmhints = XAllocWMHints();
 wmhints -> flags = InputHint | WindowGroupHint | IconWindowHint | StateHint;
 wmhints -> input = True;
 wmhints -> window_group = window;
 wmhints -> icon_window = iconwindow;
 wmhints -> initial_state = WithdrawnState;
 XSetWMHints(display, window, wmhints);
 XFree(wmhints);

 classhint = XAllocClassHint();
 classhint -> res_name = OPT_CLASS_NAME;
 classhint -> res_class = OPT_CLASS_CLASS;
 XSetClassHint(display, mapwindow, classhint);
 XFree(classhint);

 sizehints = XAllocSizeHints();
 sizehints -> flags = USSize | PSize | PMinSize | PMaxSize | PBaseSize;
 sizehints -> width = WINDOW_WIDTH;
 sizehints -> height = WINDOW_HEIGHT;
 sizehints -> min_width = WINDOW_WIDTH;
 sizehints -> min_height = WINDOW_HEIGHT;
 sizehints -> max_width = WINDOW_WIDTH;
 sizehints -> max_height = WINDOW_HEIGHT;
 sizehints -> base_width = WINDOW_WIDTH;
 sizehints -> base_height = WINDOW_HEIGHT;
 XSetWMNormalHints(display, mapwindow, sizehints);
 XFree(sizehints);

 XStoreName(display, window, OPT_WINDOW_NAME);
 XStoreName(display, iconwindow, OPT_WINDOW_NAME); /* For other wms */

 gcvalues.graphics_exposures = False;
 gcvalues.function = GXcopy;
 gc = XCreateGC(display, rootwindow,
                GCGraphicsExposures | GCFunction,
		&gcvalues);
 XCopyArea(display, background, buffer, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
 XSetCommand(display, window, argv, argc);
 wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
 XSetWMProtocols(display, mapwindow, &wm_delete_window, 1);
 if(opt_window)
 {
  _motif_wm_hints = XInternAtom(display, "_MOTIF_WM_HINTS", 0);
  mwmhints.flags = 2;
  mwmhints.functions = 0x71; /* WTF IS THIS ? */
  mwmhints.decorations = 0;
  XChangeProperty(display, mapwindow,
                  _motif_wm_hints, _motif_wm_hints,
                  32, PropModeReplace, (unsigned char *)&mwmhints, 5);
 }
 XMapWindow(display, mapwindow);
 signal(SIGINT, handle_signal);
 signal(SIGQUIT, handle_signal);
 signal(SIGTERM, handle_signal);
}

void update_window()
{
 XCopyArea(display, buffer, iconwindow, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
 XCopyArea(display, buffer, window, gc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0);
}

void process_events()
{
 XEvent event;
 int winx, winy;

 XNextEvent(display, &event);
 switch(event.type)
 {
  case Expose:
   update_window();
   break;
  case ButtonPress:
   if(opt_window && (event.xbutton.button == 1))
   {
    XDefineCursor(display, mapwindow, fleur);
    moving = 1;
    oldx = event.xbutton.x;
    oldy = event.xbutton.y;
   }
   break;
  case MotionNotify:
   winx = event.xmotion.x_root - oldx;
   winy = event.xmotion.y_root - oldy;
   if(winx < 0) winx = 0;
   if(winy < 0) winy = 0;
   if(winx > (screenwidth - WINDOW_WIDTH))
    winx = screenwidth - WINDOW_WIDTH;
   if(winy > (screenheight - WINDOW_HEIGHT))
    winy = screenheight - WINDOW_HEIGHT;
   if(moving)
    XMoveWindow(display, mapwindow, winx, winy);
   break;
  case ButtonRelease:
   if(opt_window)
   {
    moving = 0;
    XUndefineCursor(display, mapwindow);
   }
   break;
  case ClientMessage:
   if(event.xclient.data.l[0] == wm_delete_window)
    exitloop = 1;
   break;
 }
}
