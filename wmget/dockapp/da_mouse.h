#ifndef I_DOCKAPP_DA_MOUSE_H
#define I_DOCKAPP_DA_MOUSE_H
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
    dockapp/da_mouse.h - Mouse/clickregion handling defs

    Private header file.
*/

#ifndef DOCKAPP_EXPOSE_INTERNALS
#   error da_mouse.h #included... this is internal to the dockapp lib...
#endif

void da_mouse_button_down (
        int xpos,
        int ypos,
        int state);

dockapp_rv_t da_mouse_button_up (
        int xpos,
        int ypos,
        int state);


#endif /* I_DOCKAPP_DA_MOUSE_H */
