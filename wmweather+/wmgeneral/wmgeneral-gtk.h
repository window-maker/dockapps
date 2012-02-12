#ifndef WMGENERAL_GTK_H_INCLUDED
#define WMGENERAL_GTK_H_INCLUDED

#include <gtk/gtk.h>

  /***********************/
 /* Function Prototypes */
/***********************/

void openDockWindow(int argc, char *argv[], char **, char *, int, int);
void RedrawWindow(void);
void RedrawWindowXY(int x, int y);
void setClickCallback(void (*func)(GdkEventButton *ev));

void createXBMfromXPM(char *, char **, int, int);
void copyPixmapArea(int, int, int, int, int, int);
void copyMaskArea(int, int, int, int, int, int);
void setMaskXY(int, int);

#endif
