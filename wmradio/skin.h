/*
 * Copyright (C) 12 Jun 2003 Tomas Cermak
 * 
 * This file is part of wmradio program. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _SKIN_H
#define _SKIN_H
#include <X11/xpm.h>

#define DEFAULTICONSIZE 56
#define SKIN_ON 0
#define SKIN_OFF 1

typedef struct {
    int x,y,w,h;
} region;

typedef struct {
    int x,y,w,h;
    char *name;
} text_skin_item;

typedef struct {
    int action;
    unsigned int
        destx,desty,
        srcx,srcy,
        width,height;
    unsigned char status;
} ButtonInfo;

#define BS_RELEASED 1
#define BS_PRESSED  2
#define BS_SELECTED 4

typedef struct {
    unsigned char
        destx,desty,
        srcx,srcy,
        w[12],
        h;
} DigitsInfo;

typedef struct {
    unsigned int
        destx,desty,
        srcx,srcy,
        w[26], /* a - z */
        h;
} LettersInfo;

typedef struct { Pixmap pixmap; Pixmap mask; XpmAttributes
        attributes; } XpmIcon;

int in_region(int x, int y, int btx,int bty,int btw,int bth);
int xor_string(char *s);
void create_skin(char *skin_def_file, Display *display, Drawable drawable);
int find_action(int x, int y);
void skin_to_window(Display *display, Window win, GC gc, int freq, char stereo);
int skin_mouse_event(int x, int y, int button, int press);
void skin_unselect_button(void);
void skin_select_button(int action);
void skin_switch_radio(char status);
int skin_width(void);
int skin_height(void);
void skin_select_station(int station);

#endif
