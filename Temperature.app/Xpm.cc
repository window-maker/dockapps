//
//  Temperature.app
// 
//  Copyright (c) 2000-2002 Per Liden
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
#include <stdlib.h>
#include <string.h>
#include "Temperature.h"
#include "Xpm.h"

Xpm::Xpm(Display* display, Window root, char** data) 
{
   int error;

   mDisplay = display;

   mAttributes.valuemask = 0;
   error = XpmCreatePixmapFromData(mDisplay, root, data, &mImage, &mMask, &mAttributes);

   switch (error) {
   case XpmColorError:
      std::cerr << APPNAME << ": xpm image loaded but did not get all colors needed" << std::endl;
      break;

   case XpmColorFailed:
      std::cerr << APPNAME << ": could not load xpm image (not enough colors available)" << std::endl;
      exit(0);
      break;

   case XpmNoMemory:
      std::cerr << APPNAME << ": could not load xpm image (not enough memory available)" << std::endl;
      exit(0);
      break;

   case XpmOpenFailed:
   case XpmFileInvalid:
      std::cerr << APPNAME << ": could not load xpm image (image broken or corrupt)" << std::endl;
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
   XClearWindow(mDisplay, win);
}

void Xpm::setWindowPixmapShaped(Window win) 
{
   XResizeWindow(mDisplay, win, mAttributes.width, mAttributes.height);
   XSetWindowBackgroundPixmap(mDisplay, win, mImage);
   XShapeCombineMask(mDisplay, win, ShapeBounding, 0, 0, mMask, ShapeSet);
   XClearWindow(mDisplay, win);
}

void Xpm::drawString(int pos, char* font, char* str)
{
   XFontStruct* fontStruct;
   GC           gc;
   XGCValues    gcv;

   if ((fontStruct = XLoadQueryFont(mDisplay, font)) == 0) {
      cerr << APPNAME << ": could not load font '" << font << "'" << endl;
      exit(0);
   }
   
   gcv.foreground = WhitePixel(mDisplay, DefaultScreen(mDisplay));
   gc = XCreateGC(mDisplay, mImage, GCForeground, &gcv);

   int strLength = strlen(str);
   int strWidth = XTextWidth(fontStruct, str, strLength);
   
   int x = (64 / 2) - (strWidth / 2);
   XSetFont(mDisplay, gc, fontStruct->fid);
   XDrawString(mDisplay, mImage, gc, x, pos, str, strLength);

   XFreeGC(mDisplay, gc);
   XFreeFont(mDisplay, fontStruct);
}

void Xpm::drawComposedString(int pos, char* font1, char* str1, char* font2, char* str2)
{
   XFontStruct* fontStruct1;
   XFontStruct* fontStruct2;
   GC           gc;
   XGCValues    gcv;

   if ((fontStruct1 = XLoadQueryFont(mDisplay, font1)) == 0) {
      cerr << APPNAME << ": could not load font '" << font1 << "'" << endl;
      exit(0);
   }

   if ((fontStruct2 = XLoadQueryFont(mDisplay, font2)) == 0) {
      cerr << APPNAME << ": could not load font '" << font2 << "'" << endl;
      exit(0);
   }
   
   gcv.foreground = WhitePixel(mDisplay, DefaultScreen(mDisplay));
   gc = XCreateGC(mDisplay, mImage, GCForeground, &gcv);

   int str1Length = strlen(str1);
   int str1Width = XTextWidth(fontStruct1, str1, str1Length);
   int str2Length = strlen(str2);
   int str2Width = XTextWidth(fontStruct2, str2, str2Length);
   
   int x = (64 / 2) - ((str1Width + str2Width) / 2);
   XSetFont(mDisplay, gc, fontStruct1->fid);
   XDrawString(mDisplay, mImage, gc, x, pos, str1, str1Length);

   x += str1Width;
   XSetFont(mDisplay, gc, fontStruct2->fid);
   XDrawString(mDisplay, mImage, gc, x, pos, str2, str2Length);

   XFreeGC(mDisplay, gc);
   XFreeFont(mDisplay, fontStruct1);
   XFreeFont(mDisplay, fontStruct2);
}
