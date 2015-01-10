/*  File:     wmcalc_g.h
 *  Author:   Edward H. Flora <ehflora@access1.net>
 *  Version:  0.2
 *
 *  Description:
 *  This file contains the global variables used by the wmcalc program.
 *
 *  Change History:
 *  Date       Modification
 *  10/25/00   Original file creation, extracted from wmcalc.h
 *  11/05/00   Added MemLock global array for locking values into Memory
 *  11/05/00   Added Entries for "Locked" memory values in config CfgVarList
 */

#ifndef WMCALC_G_H
#define WMCALC_G_H

#include "wmcalc_x.h"
#include "wmcalc_c.h"


/* X11 Globals */
Display *display;
int screen;
Window rootwin, win, iconwin;
GC gc;
int depth;
Pixel bg_pixel, fg_pixel;

XSizeHints xsizehints;
XWMHints   *xwmhints;
XClassHint xclasshint;
char configfile[CONFIGFILEMAX];
char tempfile[CONFIGFILEMAX];

Pixmap pixmask;

ButtonArea dispchar;
XpmIcon template, visible, buttons, charmap;


/* Configuration File Information */
char *CfgVarList[MAX_LABEL] = {"Mem0",
			       "Mem1",
			       "Mem2",
			       "Mem3",
			       "Mem4",
			       "Mem5",
			       "Mem6",
			       "Mem7",
			       "Mem8",
			       "Mem9",
			       "MEM0",
			       "MEM1",
			       "MEM2",
			       "MEM3",
			       "MEM4",
			       "MEM5",
			       "MEM6",
			       "MEM7",
			       "MEM8",
			       "MEM9",
			       "ImagChar",
			       "CalcStart"};

char ImagChar = 'j';
char SysCalcCmd[CALC_CMD_SIZE] = "xcalc &";


double MemArray[NUM_MEM_CELLS];
int    MemLock [NUM_MEM_CELLS];

#endif
