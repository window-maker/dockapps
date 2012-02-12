#include "config.h"

/*  Copyright (C) 2002  Brad Jorsch <anomie@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "font.h"

#include "wmgeneral/wmgeneral-x11.h"
extern XpmIcon wmgen;
extern GC NormalGC;
extern Window Root;

#include "characters.xpm"

static Pixmap fonts[3]={ None, None, None };
static char *colors[3][4]={
    { "#000000", "#0C4E66", "#127599", "#1EC3FF" },
    { "#000000", "#664D0B", "#997411", "#FFC21D" },
    { "#000000", "#662B31", "#99414A", "#FF6D7B" }
};

void init_font(int i){
    XpmIcon x;
    XpmColorSymbol cols[4]={
        {"Background", NULL, 0},
        {"Low", NULL, 0},
        {"Mid", NULL, 0},
        {"High", NULL, 0}
    };
    int j;

    if(fonts[i]!=None) return;
    for(j=0; j<4; j++){
        cols[j].pixel=GetColor(colors[i][j]);
    }
    x.attributes.numsymbols=5;
    x.attributes.colorsymbols=cols;
    x.attributes.exactColors=False;
    x.attributes.closeness=40000;
    x.attributes.valuemask=(XpmColorSymbols | XpmExactColors | XpmCloseness);
    GetXPM(&x, characters_xpm);
    fonts[i]=x.pixmap;
    XFreePixmap(display, x.mask);
}

int DrawString(int x, int y, char *str, int font){
    int w;
    char *c;

    w=0;
    for(c=str; *c!='\0'; c++){
        w+=DrawChar(x+w, y, *c, font);
        w++;
    }

    return w-1;
}

int GetStringWidth(char *str){
    int w;
    char *c;

    w=0;
    for(c=str; *c!='\0'; c++){
        w+=DrawChar(-1, -1, *c, -1);
        w++;
    }

    return w-1;
}

int DrawNumber(int x, int y, int n, int font){
    int w;
    int flag=0;
    char c;

    if(n<0){
        flag=1;
        n=-n;
    }

    w=0;
    do {
        w+=3;
        c='0'+(n%10);
        DrawChar(x-w, y, c, font);
        n/=10;
        w++;
    } while(n>0);
    if(flag){
        w+=2;
        DrawChar(x-w, y, '-', font);
        w++;
    }

    return w-1;
}

int DrawChar(int x, int y, char c, int font){
    int sx, w;

    c=toupper(c);
    w=3;
    if(c>='A' && c<='Z'){
        sx=(c-'A')*4+1;
        if(c=='M'){ w=4; sx=149; }
        if(c=='N'){ w=4; sx=154; }
        if(c=='W'){ w=4; sx=159; }
    } else if(c>='0' && c<='9') sx=(c-'0')*4+105;
    else if(c==':'){ w=1; sx=164; }
    else if(c=='('){ w=2; sx=171; }
    else if(c==')'){ w=2; sx=174; }
    else if(c=='%') sx=89;
    else if(c=='-'){ w=2; sx=168; }
    else if(c=='.'){ w=1; sx=166; }
    else if(c=='<') sx=49;
    else if(c=='>') sx=53;
    else if(c=='/') sx=145;
    else if(c=='\''){ w=1; sx=177; }
    else return 0;

    if(x>=0 && y>=0 && x+w<192 && y<174 && font>=0 && font<3){
        init_font(font);
        XCopyArea(display, fonts[font], wmgen.pixmap, NormalGC, sx, 1, w, 5, x, y);
    }
    return w;
}
