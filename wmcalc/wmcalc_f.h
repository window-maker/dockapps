/*  File:     wmcalc_f.h
 *  Author:   Edward H. Flora <ehflora@access1.net>
 *  Version:  0.2
 *
 *  Description:
 *  This file contains the function prototypes for functional
 *  components of the wmcalc program.
 *
 *  Change History:
 *  Date       Modification
 *  01/17/01   Updated whichKey() function to take a KeySym
 *  11/09/00   Removed function clrmem() as this was merged into clrallmem()
 *             Also add function whichKey, in wmcalc.c to handle Keyboard 
 *             events
 *  10/25/00   Original file creation, extracted from wmcalc.h
 */

#ifndef WMCALC_F_H
#define WMCALC_F_H

#include <stdio.h>
#include "wmcalc_x.h"
#include "wmcalc_t.h"

void ExecFunc(int val);     // function to run app N as found in conf file
void redraw(void);                 
void getPixmaps(void);
int  whichButton(int x, int y);  // determine which button has been pressed
int  whichKey(KeySym keysym);    // determine which key has been pressed
int  flush_expose(Window w);
void show_usage(void);           // show usage message to stderr
char *readln(FILE *fp);          // read line from file, return pointer to it
void defineButtonRegions(void);  // Define boundaries for each button
void displaystr(void);
void displaychar(char ch, int location);
ButtonArea getboundaries(char ch);
int read_config(void);
int write_config(void);
void error_handler(int err_code, char *err_string);

/* Calculator Specific functions */

void clearcalc(void);
void clearnum(void);
void addnums(void);
void subtnums(void);
void multnums(void);
void divnums(void);
void chgsignnum(void);
void sqrtnum(void);
void sqrnum(void);
void charkey(char ch);
void equalfunc(void);
void stormem(int mem_loc);
void clrallmem(void);
void startcalc(void);
void recallmem(int mem_loc);

/* Future functions yet to be implemented */

//void scinotation(void);
//void clearmem(void);
//void userdef201(void);
//void userdef205(void);
//void userdef206(void);
//void userdef210(void);
//void userdef211(void);
//void userdef215(void);
//void userdef218(void);
//void userdef220(void);
//void userdef301(void);
//void userdef305(void);
//void userdef306(void);
//void userdef310(void);
//void userdef315(void);
//void userdef318(void);
//void userdef320(void);


#endif
