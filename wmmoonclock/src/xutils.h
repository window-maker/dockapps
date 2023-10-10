#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

/*
 *   Typedefs
 */
typedef struct {
	Pixmap		pixmap;
	Pixmap		mask;
	XpmAttributes	attributes;
} XpmIcon;




/*
 *   Global variable
 */
extern Display		*display;
extern Window          Root;
extern Window          iconwin, win;
extern int             screen;
extern int             DisplayDepth;





/*
 *   Function Prototypes
 */
void 		AddMouseRegion(unsigned, int, int, int, int);
int  		CheckMouseRegion(int, int);
void 		openXwindow(int, char **, char **, char *, int, int, char *, char *, char *);
void 		initXwindow(int, char **);
void 		RedrawWindow(void);
void 		RedrawWindowXY(int, int);
void  		copyXPMArea(int, int, int, int, int, int);
void  		copyXBMArea(int, int, int, int, int, int);
void  		setMaskXY(int, int);
unsigned long 	getColor(char *, float);
void 		RedrawWindow(void);


#endif
