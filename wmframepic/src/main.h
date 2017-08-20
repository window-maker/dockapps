#ifndef MAIN_H_
#define MAIN_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>

#include <stdio.h>
#include <stdlib.h>

#include "draw_text.h"
#include "frame_mask.xbm"
#include "core_mask.xbm"

#define WINDOWED_SIZE_W 64
#define WINDOWED_SIZE_H 64
#define SIZE 58

Display	*display;
Window 	root, win, iconwin;
//long	fgcolor, bgcolor;
int 	screen;
int 	depth=1;
int 	eventmask = ButtonPressMask|ExposureMask;
Bool show_pic = True;

XSizeHints	sizehints;
XEvent	event;
Pixmap pic_pixmap;
GC 		gc_core, gc_border;

void mainloop(void);
#endif /* MAIN_H_ */
