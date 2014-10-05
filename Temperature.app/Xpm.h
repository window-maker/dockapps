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

#ifndef _XPM_H_
#define _XPM_H_

#include <X11/Xlib.h>
#include <X11/xpm.h>

class Xpm
{
public:
   Xpm(Display* display, Window root, char** data);
   virtual ~Xpm();
   void setWindowPixmap(Window win);
   void setWindowPixmapShaped(Window win);
   void drawString(int pos, char* font, char* str);
   void drawComposedString(int pos, char* font1, char* str1, char* font2, char* str2);

private:
   Display*      mDisplay;
   XpmAttributes mAttributes;
   Pixmap        mImage;
   Pixmap        mMask;
};

#endif
