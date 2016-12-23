/*
 * $Id: wmfortune.c,v 0.24 2000/03/28 01:49:58 sugano Exp $
 * wmfortune Copyright (c) 2000 Makoto Sugano
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
#define BUFF_SIZE 1024
#define SCROLL_DEFAULT_SPEED 1000

#include <unistd.h>
#include <stdio.h>
#include "dockapp.h"
#include "XPM/panel.xpm"
#include "XPM/panel_button_pressed.xpm"
#include "XPM/panel_window.xpm"

/* WINDOW: window for showing the messages */
#define WINDOW_X 4
#define WINDOW_Y 4
#define WINDOW_WIDTH 56
#define WINDOW_HEIGHT 22

/* BUTTON: button for the new fortune message */
#define BUTTON_X 6
#define BUTTON_Y 32
#define BUTTON_WIDTH 52
#define BUTTON_HEIGHT 28

/* 6 pixels are used to draw a letter */
#define PIXELS_PER_LETTER 6

#define STRING_Y 18

/*
 * pixel lag between the last letter of the previous message
 * and the first letter of the next message.
 * if set to 0, the messages appear quickly after
 * the previous messages ends.
 */
#define MESSAGE_LAG 5

char *displayName = "";
int speed = SCROLL_DEFAULT_SPEED;

/* global variables for main() and callbacks */
FILE *fortune;
Pixmap pixmap;
Pixmap panel_button_pressed_pixmap;
Pixmap panel_pixmap, panel_mask;
GC gc;
int c;
char buff[BUFF_SIZE];

/*
 * i: used to count the number of letters in fill_buff().
 * j: the number that points the message starting pixel.
 */
int i, j;

/* fill the buffer with the fortune message. */
static void
fill_buff (void)
{
  i = 0;

  memset(buff, '\0', BUFF_SIZE);
  if (!(fortune = popen("fortune -s", "r")))
    {
      perror("fortune");
      exit(0);
    }

  while (( c = fgetc(fortune)) != EOF)
    {
      if (c == '\n')
	{
	  buff[i++] = ' ';
	}
      else if (c == '\t')
	{
	  buff[i++] = ' ';
	  buff[i++] = ' ';
	  buff[i++] = ' ';
	}
      else
	{
	  buff[i++] = c;
	}
    }

  j = i + MESSAGE_LAG;
  pclose(fortune);
}

static DAProgramOption options[] = {
  {"-d", "--displayname", "display to use.", DOString, False, {&displayName}},
  {"-s", "--speed", "scrolling speed. (default 1000 dot/ms)", DOInteger, False, {&speed}},
};

static void
buttonReleaseCallback (int button, int state, int x, int y)
{
  if (button == 1 && BUTTON_X < x &&
      x < (BUTTON_X + BUTTON_WIDTH) &&
      BUTTON_Y < y && y < (BUTTON_Y + BUTTON_HEIGHT))
    {
      fill_buff();
    }
  XCopyArea(DADisplay, panel_pixmap, pixmap, gc,
	    BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
	    BUTTON_X, BUTTON_Y);
}

static void
buttonPressCallback (int button, int state, int x, int y)
{
  if (button == 1 && BUTTON_X < x &&
      x < (BUTTON_X + BUTTON_WIDTH) &&
      BUTTON_Y < y && y < (BUTTON_Y + BUTTON_HEIGHT))
    {
      XCopyArea(DADisplay, panel_button_pressed_pixmap, pixmap, gc,
		BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
		BUTTON_X, BUTTON_Y);
    }
}

int
main(int argc, char **argv)
{
   Pixmap panel_window_pixmap;

   int w = 64, h = 64;

   DACallbacks callbacks = {NULL,buttonPressCallback
	,buttonReleaseCallback,NULL,NULL,NULL,NULL};

   DAParseArguments(argc, argv, options,
		    sizeof(options)/sizeof(DAProgramOption),
		    "dockapp that shows the messages from fortune command.", "$Id: wmfortune.c,v 0.24 2000/03/28 01:49:58 sugano Exp $");

   fill_buff();

   DAInitialize(displayName, "wmfortune", 64, 64, argc, argv);
   pixmap = DAMakePixmap();

   /* making pixmap for the panel */
   DAMakePixmapFromData(panel_xpm, &panel_pixmap,
			&panel_mask, &w, &h);
   DAMakePixmapFromData(panel_button_pressed_xpm,
			&panel_button_pressed_pixmap, NULL, &w, &h);
   DAMakePixmapFromData(panel_window_xpm,
			&panel_window_pixmap, NULL, &w, &h);

   /* setting up the mask for the panel */
   DASetShape(panel_mask);
   DASetPixmap(panel_pixmap);

   /* setting up the graphic context */
   gc = DefaultGC(DADisplay, DefaultScreen(DADisplay));

   DASetCallbacks(&callbacks);
   DAShow();

   /* drawing the button */
   XCopyArea(DADisplay, panel_pixmap, pixmap, gc,
	     BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
	     BUTTON_X, BUTTON_Y);

   while (1)
     {
       XEvent ev;

       /* sets the foreground color green */
       XSetForeground(DADisplay, gc, DAGetColor("green"));
       XCopyArea(DADisplay, panel_window_pixmap, pixmap, gc,
		 WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT,
		 WINDOW_X, WINDOW_Y);
       XDrawString(DADisplay, pixmap, gc, j, STRING_Y, buff, strlen(buff));

       DASetPixmap(pixmap);

       /* scroll the message by a pixel to left */
       i--;
       j = i + MESSAGE_LAG;

       /*
	* starts scrolling the messages from the beginning
	* if the message ends.
        */
       if (j == - (PIXELS_PER_LETTER) * strlen(buff))
	 {
	   /* if not 64, message suddenly appears in the window */
	   i = 64;
	   j = i + MESSAGE_LAG;
	 }

       /* handle all pending X events */
       while (XPending(DADisplay))
	 {
	   XNextEvent(DADisplay, &ev);
	   DAProcessEvent(&ev);
	 }
       usleep(speed);
     }
   return 0;
}
