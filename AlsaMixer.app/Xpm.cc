//
//  Mixer.app
//
//  Copyright (c) 1998-2002 Per Liden
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307,
//  USA.
//

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "Mixer.h"
#include "Xpm.h"

using namespace std;

Xpm::Xpm(Display* display, Window root, char** data)
{
   int error;

   mDisplay = display;

   mAttributes.valuemask = 0;
   error = XpmCreatePixmapFromData(mDisplay, root, data, &mImage, &mMask, &mAttributes);

   switch (error) {
   case XpmColorError:
      cerr << APPNAME << ": xpm image loaded but did not get all colors needed" << endl;
      break;

   case XpmColorFailed:
      cerr << APPNAME << ": could not load xpm image (not enough colors available)" << endl;
      exit(0);
      break;

   case XpmNoMemory:
      cerr << APPNAME << ": could not load xpm image (not enough memory available)" << endl;
      exit(0);
      break;

   case XpmOpenFailed:
   case XpmFileInvalid:
      cerr << APPNAME << ": could not load xpm image (image broken or corrupt)" << endl;
      exit(0);
      break;

   case XpmSuccess:
   default:
      // Image loaded ok
      break;
   }
}

Xpm::~Xpm()
{
   if (mImage) {
      XFreePixmap(mDisplay, mImage);
   }

   if (mMask) {
      XFreePixmap(mDisplay, mMask);
   }
}

void Xpm::setWindowPixmap(Window win)
{
   XResizeWindow(mDisplay, win, mAttributes.width, mAttributes.height);
   XSetWindowBackgroundPixmap(mDisplay, win, mImage);
}

void Xpm::setWindowPixmapShaped(Window win)
{
   XResizeWindow(mDisplay, win, mAttributes.width, mAttributes.height);
   XSetWindowBackgroundPixmap(mDisplay, win, mImage);
   XShapeCombineMask(mDisplay, win, ShapeBounding, 0, 0, mMask, ShapeSet);
}

void Xpm::drawString(int x, int y, char* text)
{
   Font      font;
   GC        gc;
   XGCValues gcv;

   font = XLoadFont(mDisplay, LABEL_FONT);
   gcv.font = font;
   gcv.foreground = WhitePixel(mDisplay, DefaultScreen(mDisplay));
   gc = XCreateGC(mDisplay, mImage, GCFont | GCForeground, &gcv);
   XDrawString(mDisplay, mImage, gc, x, y, text, strlen(text));
   XFreeGC(mDisplay, gc);
   XUnloadFont(mDisplay, font);
}
