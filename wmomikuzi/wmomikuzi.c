/*
 *
 * Copyright (c) 2000 Makoto Sugano
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <libdockapp/dockapp.h>

#include "XPM/back.xpm"
#include "XPM/button_pressed.xpm"

#include "XPM/daikichi.xpm"
#include "XPM/chukichi.xpm"
#include "XPM/shoukichi.xpm"
#include "XPM/suekichi.xpm"
#include "XPM/kyou.xpm"
#include "XPM/daikyou.xpm"

#define USLEEP 30000
#define OFF 0
#define ON 1

char *displayName = "";
Pixmap pixmap;
Pixmap back_pixmap, back_mask;
Pixmap button_pressed_pixmap;
GC gc;

int i, j, animation = ON;

static void
  buttonReleaseCallback (int button, int state, int x, int y)
{
   if (button == 1 && 0 < x && x < 64 && 44 < y && y < 64){
     srand(time(0L) * (getpid()));
     i = 0;
     j = rand() / (RAND_MAX / 6 + 1) + 18;
     animation = ON;
   }
   XCopyArea(DADisplay, back_pixmap, pixmap, gc, 0, 44, 64, 64, 0, 44);
   DASetPixmap(pixmap);
}

static void
  buttonPressCallback (int button, int state, int x, int y)
{
  if (button == 1 && 0 < x && x < 64 && 44 < y && y < 64){
    XCopyArea(DADisplay, button_pressed_pixmap, pixmap, gc, 0, 44, 64, 64, 0, 44);
    DASetPixmap(pixmap);
   }
}

static DAProgramOption options[] = {
   {"-d", "--displayname", "display to use.", DOString, False, {&displayName}},
};

main(int argc, char **argv)
{
   Pixmap daikichi_pixmap;
   Pixmap chukichi_pixmap;
   Pixmap shoukichi_pixmap;
   Pixmap suekichi_pixmap;
   Pixmap kyou_pixmap;
   Pixmap daikyou_pixmap;

   int w = 64, h = 64;

   DACallbacks callbacks = {NULL,buttonPressCallback
	,buttonReleaseCallback,NULL,NULL,NULL,NULL};

   DAParseArguments(argc, argv, options,
		    sizeof(options)/sizeof(DAProgramOption),
		    "dockapp that predict your luck", "wmomikuzi 0.122");

   DAInitialize(displayName, "wmomikuzi", 64, 64, argc, argv);   
   pixmap = DAMakePixmap();
   
   /* making pixmaps for the panel */
   DAMakePixmapFromData(back_xpm, &back_pixmap, &back_mask, &w, &h);
   DAMakePixmapFromData(button_pressed_xpm, &button_pressed_pixmap, NULL, &w, &h);

   DAMakePixmapFromData(daikichi_xpm, &daikichi_pixmap, NULL, &w, &h);
   DAMakePixmapFromData(chukichi_xpm, &chukichi_pixmap, NULL, &w, &h);
   DAMakePixmapFromData(shoukichi_xpm, &shoukichi_pixmap, NULL, &w, &h);
   DAMakePixmapFromData(suekichi_xpm, &suekichi_pixmap, NULL, &w, &h);
   DAMakePixmapFromData(kyou_xpm, &kyou_pixmap, NULL, &w, &h);
   DAMakePixmapFromData(daikyou_xpm, &daikyou_pixmap, NULL, &w, &h);

   /* setting up the mask for the panel */
   DASetShape(back_mask);
   DASetPixmap(back_pixmap);

   /* setting up the graphic context */
   gc = DefaultGC(DADisplay, DefaultScreen(DADisplay));

   DASetCallbacks(&callbacks);
   DAShow();

   srand(time(0L) * (getpid()));
   i = 0;
   j = rand() / (RAND_MAX / 6 + 1) + 18;
   
   /* draws the button part */
   XCopyArea(DADisplay, back_pixmap, pixmap, gc, 0, 44, 64, 64, 0, 44);

   while (1) {
      XEvent ev;

      /* draws the display part */
      XCopyArea(DADisplay, back_pixmap, pixmap, gc, 0, 0, 64, 44, 0, 0);

      if ((i % 6) == 1){
	 XCopyArea(DADisplay, daikichi_pixmap, pixmap, gc, 0, 0, 64, 39, 5, 5);
      } else if ((i % 6) == 2){
	 XCopyArea(DADisplay, chukichi_pixmap, pixmap, gc, 0, 0, 64, 39, 5, 5);
      } else if ((i % 6) == 3){
	 XCopyArea(DADisplay, shoukichi_pixmap, pixmap, gc, 0, 0, 64, 39, 5, 5);
      } else if ((i % 6) == 4){
	 XCopyArea(DADisplay, suekichi_pixmap, pixmap, gc, 0, 0, 64, 39, 5, 5);
      } else if ((i % 6) == 5){
	 XCopyArea(DADisplay, kyou_pixmap, pixmap, gc, 0, 0, 64, 39, 5, 5);
      } else if ((i % 6) == 0){
	 XCopyArea(DADisplay, daikyou_pixmap, pixmap, gc, 0, 0, 64, 39, 5, 5);
      }
      DASetPixmap(pixmap);
      
      usleep(USLEEP);

      /* j is the destination number */
      if (i <= j){
	i++;
      } else {
  	animation = OFF;
      }

      while (animation == OFF) {

	/* handle all pending X events */
	while (XPending(DADisplay)) {
	  XNextEvent(DADisplay, &ev);
	  DAProcessEvent(&ev);
	}
        usleep(USLEEP);
      }
   }
   return 0;
}
