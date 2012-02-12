#ifndef WMGENERAL_X11_H_INCLUDED
#define WMGENERAL_X11_H_INCLUDED

#include <X11/xpm.h>

  /************/
 /* Typedefs */
/************/

typedef struct {
	Pixmap			pixmap;
	Pixmap			mask;
	XpmAttributes	attributes;
} XpmIcon;

  /*******************/
 /* Global variable */
/*******************/

Display		*display;

  /***********************/
 /* Function Prototypes */
/***********************/

void openDockWindow(int argc, char *argv[], char **, char *, int, int);
void RedrawWindow(void);
void RedrawWindowXY(int x, int y);

Pixel GetColor(char *);
void GetXPM(XpmIcon *, char **);
void createXBMfromXPM(char *, char **, int, int);
void copyPixmapArea(int, int, int, int, int, int);
void copyMaskArea(int, int, int, int, int, int);
void setMaskXY(int, int);

#endif
