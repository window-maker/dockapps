/*
 *
 *  	wmCalendar (c)2003 Matthias Laabs mattlaabs@users.sourceforge.net
 * 
 *  		A Dockapp Calendar
 * 
 *
 *
 *      This code is based on:
 *      wmCalClock-1.25 (C) 1998, 1999 Mike Henderson (mghenderson@lanl.gov)
 * 
 *
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation; either version 2, or (at your option)
 * 	any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program (see the file COPYING); if not, write to the
 * 	Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 *      Boston, MA  02111-1307, USA
 *
 *
*/



/* defines */
#ifndef	_WMCALENDAR_H_
#define	_WMCALENDAR_H_

#define _GNU_SOURCE

#define COUNTER 2000

#define ARROW_OPEN 3
#define ARROW_CLOSE 4
#define MOON 6
#define CALENDAR 8
#define APPLAUNCH 0
#define SETTINGS 5
#define MAXBUTTON 14

#define BT_BACK 20
#define BT_FORWARD 21
#define BT_OPENCLOSE 22
#define BT_MOON 23
#define BT_SETTINGS 24
#define BT_CALTYPE 25
#define BT_APP 26

#define DBLCLKTIME 400

#define GREGORIAN 0
#define PERSIAN 1
#define ISLAMIC 2
#define CALTYPES 3



/* includes  */
#include "settings.h"
#include "calendar.h"
#include "dockapp.h"
#include "calendarfunc.h"
#include "wmCalendar_master.xpm"
#include "wmCalendar_master2.xpm"



/* functions */
void ParseCMDLine(int argc, char *argv[]);
void buttonPress(int btype, int bx, int by, long etime);
void print_usage();
void initValues();
void getTime();

void draw();
void blankScreen();
void drawMonthYear();
void drawDays();
void drawButtons();
int  drawButton(int type, int xoff, int yoff);
void drawNumber(int number, int yoff, int dayOfWeek, int type, int today);

int  processXEvents();



/* definitions of images for months, numbers and icons */
int	xsMonth[6][12] = {{ 3, 27, 52, 77, 102, 130, 162, 188, 216, 241, 266, 293 },//greg. engl.
			  { 2, 24, 49, 74, 89, 115, 140, 168, 197, 223, 250, 275 },//pers. engl.(not yet)
			  { 3, 32, 53, 89, 128, 162, 200, 220, 257, 287, 322, 361},//,isl. eng.
			  { 2, 47, 86, 126, 166, 187, 223, 255, 289, 338, 374, 416 },//greg. farsi.
			  { 2, 42, 84, 123, 149, 188, 227, 258, 286, 311, 336, 371 },//pers. fars.
			  { 3, 32, 53, 89, 128, 162, 200, 220, 257, 287, 322, 361}};//isl. arab.(not yet)

int	xeMonth[6][12] = {{ 23, 48, 74, 98, 126, 150, 177, 211, 237, 262, 289, 315 },//greg. engl.
			  { 20, 44, 70, 85, 111, 135, 164, 193, 218, 245, 270, 289 },//pers. engl.(not yet)
			  { 27, 49, 84, 123, 157, 195, 217, 253, 282, 317, 357, 394},//,isl. eng.
			  { 34, 76, 117, 155, 178, 214, 247, 276, 325, 362, 406, 451 },//greg. fars.
			  { 34, 72, 114, 138, 179, 223, 248, 274, 300, 326, 361, 393 },//pers. farsi.
			  { 27, 49, 84, 123, 157, 195, 217, 253, 282, 317, 357, 394}};//isl. arab.(not yet)


int	xdMonth[6][12];
int	yMonth[6] = {17, 32, 45, 63, 79, 45};
int	ydMonth = 11;

int	xeNumbers[2][10] = {{119, 123, 127, 131, 135, 139, 143, 147, 151, 155 },
			    {69, 73, 77, 81, 85, 89, 93, 97, 101, 105}};
int   xdNumbers = 5;
int	yNumbers = 2;
int	ydNumbers = 7;
int   xsYear[2][10] = {{ 3, 14, 23, 33, 43, 53, 63, 73, 83, 93},
			 { 103, 114, 123, 133, 143, 153, 163, 173, 183, 193}};
int   xeYear[2][10] = {{ 9, 17, 29, 39, 49, 59, 69, 79, 89, 99},
			 { 109, 117, 129, 139, 149, 159, 169, 179, 189, 199}};
int   xdYear[2][10];
int   yYear = 4;
int   ydYear = 9;

int   xsButton[14] = {129, 138, 147, 162, 167, 156, 165, 174, 129, 138, 147, 156};
int   xeButton[14] = {136, 145, 154, 165, 170, 160, 172, 181, 136, 145, 154, 163};
int   yButton[14]  = {80, 80, 80, 80, 80, 80, 70, 70, 88, 88, 88, 88};
int   xdButton[14];
int   ydButton = 7;
int   xsMoon[4] = {128, 137, 146, 155};
int   yMoon = 70;
int   xdMoon = 8;
int   ydMoon = 7;

int 	monthOffset;       /* difference to actual month */
struct  icaltimetype t, told, tgr, today, mark;


Pixmap  bg[2];
Pixmap  pixmap, mask;
XEvent  event;



int   calendartype = 0;  /* defines calendar type 0:greg 1:persian 2:islamic */
int   showmoon = 0;      /* sets whether moonphase is shown (1) or not (0) */
int   showbuttons = 1;   /* sets wheter buttons on the bottom are visible */

int   appDoubleClick = 0;/* this should be changed to gtk doubleclick asap*/
long  appClickTime = 0;



#endif
