#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED


typedef struct {
	Pixmap		pixmap;
	Pixmap		mask;
	XpmAttributes	attributes;
} XpmIcon;

/* Global variable */
Display		*display;
Window          Root;
GC              NormalGC;
XpmIcon         wmgen;
XpmIcon		wmnumbers;
XpmIcon		wmempty;

/* Function Prototypes */
void openXwindow(int argc, char *argv[], char **, char **, char *, int, int);
void RedrawWindow(void);						
void copyXPMArea(int, int, int, int, int, int);			
void cleanXPMArea();					

#endif
