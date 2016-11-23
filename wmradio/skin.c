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

#include "skin.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include "stationnames.h"

#define ACTION_COUNT 16
#ifndef SKIN_DIR
#define SKIN_DIR "/usr/local/lib/wmradio/"
#endif

ButtonInfo buttons[ACTION_COUNT];
int button_count = 0;
DigitsInfo digits;
LettersInfo letters;
XpmIcon RadioSkin;
ButtonInfo StereoInfo;
char radio_is_off = 0;
int icon_width = DEFAULTICONSIZE;
int icon_height = DEFAULTICONSIZE;

int in_region(int x, int y, int btx,int bty,int btw,int bth)
{
    if( (x>=btx) && (x < btx + btw) && (y>=bty) && (y<bty+bth) ) return 1;
    return 0;
}

int xor_string(char *s)
{
    int a,r;

    r = 0;
    for(a=0; a<strlen(s); a++) {
        r = r << 1;
        if( r>0xFFFF ) r = (r & 0xFFFF) + 1;
        r ^= s[a];
    }
    return r;
}

/*  void add_action(char *name, region reg) */
/*  { */
/*    if( action_count < ACTION_COUNT ) { */
/*      actions[action_count].action = xor_string(name); */
/*      actions[action_count].r = reg; */
/*      action_count++; */
/*    } */
/*  } */

int skin_read_num(char *p)
{
    char *q;
    int res;

    q = strstr(p,",");
    if(q) {
        *q = '\000';
        q++;
    }
    res = atoi(p);
    if(q) strcpy(p,q);
    return res;
}

void skin_def_line(char *line, Display *display, Drawable drawable, char *skin_desc_dir)
{
    char *p,*w;
    int x,i;
    char buffer[256];

    w = line;
    p = strstr(w,"=");
    if(!p) return;
    *p = '\000';
    p++;
    x = xor_string(w);
    switch(x) {
    case 5829: /* preset1-6 */
        buttons[button_count].status = BS_SELECTED;
    case 5830:
    case 5831:
    case 5824:
    case 5825:
    case 5826:
    case 1457: /* tune+ */
    case 1463: /* tune- */
    case 278: /* off */
    case 696: /* scan */
    case 4507: /* display */
        buttons[button_count].action = x;
        buttons[button_count].destx = skin_read_num(p);
        buttons[button_count].desty = skin_read_num(p);
        buttons[button_count].srcx = skin_read_num(p);
        buttons[button_count].srcy = skin_read_num(p);
        buttons[button_count].width = skin_read_num(p);
        buttons[button_count].height = skin_read_num(p);
        button_count++;
        break;
    case 365: /* xpm */
        buffer[0] = '\000';
        if( ! strstr(p,"/") ) {
            strcpy(buffer,skin_desc_dir);
        }
        strcat(buffer,p);
        if(XpmReadFileToPixmap(display,drawable,buffer,
			       &(RadioSkin.pixmap),
			       &(RadioSkin.mask),
			       &(RadioSkin.attributes)) != XpmSuccess) {
	    printf("wmradio: can't load pixmap %s\n",buffer);
	}
        break;
    case 2071: /* digits */
        digits.destx = skin_read_num(p);
        digits.desty = skin_read_num(p);
        digits.srcx = skin_read_num(p);
        digits.srcy = skin_read_num(p);
        for(i=0; i<12; i++) {
            digits.w[i] = skin_read_num(p);
        }
        digits.h = skin_read_num(p);
        break;
    case 4675: /* letters */
        letters.destx = skin_read_num(p);
        letters.desty = skin_read_num(p);
        letters.srcx = skin_read_num(p);
	letters.srcy = skin_read_num(p);
        for(i=0; i<26; i++) {
            letters.w[i] = skin_read_num(p);
        }
        letters.h = skin_read_num(p);
        break;	
    case 2917: /* stereo */
        StereoInfo.destx = skin_read_num(p);
        StereoInfo.desty = skin_read_num(p);
        StereoInfo.srcx = skin_read_num(p);
        StereoInfo.srcy = skin_read_num(p);
        StereoInfo.width = skin_read_num(p);
        StereoInfo.height = skin_read_num(p);
        break;
    case 19368: /* iconwidth */
	icon_width = atoi(p);
    case 37920: /* iconheight */
	icon_height = atoi(p);
    }
}

void create_skin(char *skin_def_file, Display *display, Drawable drawable)
{
    FILE *f;
    char buffer[256], *p, dir[256];

    buffer[0] = '\000';
    memset(&letters,0,sizeof(letters));

    if(!strstr(skin_def_file,"/")) {
        strcpy(buffer,SKIN_DIR);
    }
    strcat(buffer,skin_def_file);
    strncpy(dir,buffer,sizeof(dir));
    p = &dir[strlen(dir)-1];
    while( *p != '/' ) p--;
    p[1] = '\000';
    
    f = fopen(buffer,"r");
    if(! f ){
        printf("wmradio: Skin %s not found\n", skin_def_file);
	if( strcmp(skin_def_file,"default.skin") != 0 ) {
	    create_skin("default.skin",display,drawable);
	}
        return;
    }
    while(!feof(f)) {
        fgets(buffer,sizeof(buffer),f);
        p = strstr(buffer,"#");
        if(p) *p = '\000';
        p = strstr(buffer,"\n");
        if(p) *p = '\000';
        while(buffer[0] == ' ') strcpy(buffer,&buffer[1]);
        skin_def_line(buffer,display,drawable,dir);
    }
    fclose(f);
}

int find_action(int x, int y)
{
    int a;

    for(a=0; a< button_count; a++) {
        if(
            in_region(x,y,
                buttons[a].destx,
                buttons[a].desty,
                buttons[a].width,
                buttons[a].height
            )
        ) return buttons[a].action;
    }
    return 0;
}

int skin_button_index(int x, int y)
{
    int i;
    for(i=0; i< button_count; i++) {
        if( in_region(x,y, 
		      buttons[i].destx, 
		      buttons[i].desty, 
		      buttons[i].width, 
		      buttons[i].height) ) {
            return i;
        }
    }
    return -1;
}

void skin_unselect_button(void)
{
    int a;

    for(a=0; a< ACTION_COUNT; a++) {
        if(buttons[a].status == BS_SELECTED) buttons[a].status = BS_RELEASED;
    }
}

int skin_mouse_event(int x, int y, int mousebutton, int press)
{
    int i,r;
    static int last_press = -1;

    r=0;
    if(press) {
        last_press = skin_button_index(x,y);
        buttons[last_press].status = BS_PRESSED;
    } else {
        /* release */
        if(last_press == skin_button_index(x,y)) {
            r = buttons[last_press].action;
            buttons[last_press].status = BS_RELEASED;
            if( r/100 == 58 ) {
                for(i = 0; i< button_count; i++) buttons[i].status = BS_RELEASED;
                buttons[last_press].status = BS_SELECTED;
            }
        } else {
            if(last_press >= 0) {
                buttons[last_press].status = BS_RELEASED;
            }
            last_press = -1;
        }
    }
    return r;
}

int digit_index(char digit) {
    int i = 10;

    if(isdigit(digit)) i = digit - '0';
    if(digit == ' ') i = 10;
    if(digit == '.') i = 11;
    return i;
}

int digit_source_x(int index)
{
    int i,x;

    x = digits.srcx;
    for(i=0; i<index; i++) {
        x += digits.w[i];
    }
    return x;
}

int letter_index(char letter) {

    if(isalpha(letter) && isascii(letter)) return tolower(letter) - 'a';
    return -1;
}

int letter_source_x(int index)
{
    int i,x;

    x = letters.srcx;
    for(i=0; i<index; i++) {
        x += letters.w[i];
    }
    return x;
}

int have_letters(void)
{
    return letters.h != 0;
}

void char_to_window(Display *display, Window win, GC gc, char c, int *x, int y)
{
    int idx, sourcex;
    
    if(isalpha(c)) {
	idx = letter_index(c);
	sourcex  = letter_source_x(idx);
	XCopyArea(display,RadioSkin.pixmap,win, gc,
		  letters.srcx + sourcex,
		  letters.srcy,
		  letters.w[idx],
		  letters.h,
		  *x,
		  y);
	*x += letters.w[idx];
    } else {
	idx = digit_index(c);
	sourcex  = digit_source_x(idx);
	XCopyArea(display,RadioSkin.pixmap,win, gc,
		  digits.srcx + sourcex,
		  digits.srcy,
		  digits.w[idx],
		  digits.h,
		  *x,
		  y);
	*x += digits.w[idx];
    }
}


void freq_to_window(Display *display, Window win, GC gc, int freq)
{
    char freqs[10], temp[10], *stn_name;
    int x,i;
    
    stn_name = station_get_freq_name(freq);
    if(stn_name && have_letters()) {
	strncpy(freqs,stn_name,4);
	freqs[4] = 0;
	while(strlen(freqs)<3) strcat(freqs," ");
	strcat(freqs,". ");
    } else {
	snprintf(freqs,sizeof(freqs),"%i.%i",freq/100, (freq % 100) / 10);
	while(strlen(freqs)<5) {
	    strcpy(temp," ");
	    strcat(temp,freqs);
	    strcpy(freqs,temp);
	}
    }
    /* freq now contain right text */
    i = 0;
    x = digits.destx;
    while(freqs[i]) {
	char_to_window(display,win,gc,freqs[i],&x,digits.desty);
	i++;
    }
}

void skin_to_window(Display *display, Window win, GC gc, int freq, char stereo)
{
    int i;
    int xs;
    char bs;

    setlocale(LC_ALL,"C");
    XCopyArea(display,RadioSkin.pixmap,win, gc, 0, 0, skin_width(), skin_height(), 0, 0);
    for(i=0; i<button_count; i++) {
        xs = buttons[i].srcx;
	bs = buttons[i].status;
	if( radio_is_off ) bs = BS_PRESSED;
        switch(bs) {
        case BS_RELEASED:
            xs = buttons[i].srcx;
            break;
        case BS_PRESSED:
            xs = buttons[i].srcx + buttons[i].width;
            break;
        case BS_SELECTED:
            xs = buttons[i].srcx + buttons[i].width * 2;
            break;
	}
	if( buttons[i].action != 4507 ) { /* 4507 is display */
	    XCopyArea(display,RadioSkin.pixmap,win, gc,
		  xs, buttons[i].srcy,
		  buttons[i].width,
		  buttons[i].height,
		  buttons[i].destx,
		  buttons[i].desty);
	}
    }
    /* digits */
    if(!radio_is_off){
	freq_to_window(display,win,gc,freq);
	/* stereo */
	xs = StereoInfo.srcx;
	if(stereo) xs += StereoInfo.width;
	XCopyArea(display,RadioSkin.pixmap,win, gc,
		  xs,
		  StereoInfo.srcy,
		  StereoInfo.width,
		  StereoInfo.height,
		  StereoInfo.destx,
		  StereoInfo.desty);
    }
}

void skin_select_button(int action) {
    int a;

    for(a=0; a< ACTION_COUNT; a++) {
        if(buttons[a].action == action) buttons[a].status = BS_SELECTED;
    }
}

void skin_switch_radio(char status)
{
    radio_is_off = status;
}

int skin_width(void) {
    return icon_width;
}

int skin_height(void) {
    return icon_height;
}
    
void skin_select_station(int station)
{
    int i;

    for(i=0; i<ACTION_COUNT; i++) {
	switch(buttons[i].action) {
	case 5829:
            buttons[i].status = station == 0 ? BS_SELECTED : BS_RELEASED;
	    break;
	case 5830:
            buttons[i].status = station == 1 ? BS_SELECTED : BS_RELEASED;
	    break;
	case 5831:
            buttons[i].status = station == 2 ? BS_SELECTED : BS_RELEASED;
	    break;
	case 5824:
            buttons[i].status = station == 3 ? BS_SELECTED : BS_RELEASED;
	    break;
	case 5825:
            buttons[i].status = station == 4 ? BS_SELECTED : BS_RELEASED;
	    break;
	case 5826:
            buttons[i].status = station == 5 ? BS_SELECTED : BS_RELEASED;
	    break;
	}
   }
}
