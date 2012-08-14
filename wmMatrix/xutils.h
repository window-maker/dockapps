#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED



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
Display		*display;
Window          Root;
Window          iconwin, win;
int             screen; 
int             DisplayDepth;





/*
 *   Function Prototypes
 */
void 		AddMouseRegion(int, int, int, int, int);
int  		CheckMouseRegion(int, int);
void 		openXwindow(int, char **, char **, char *, int, int);
void 		initXwindow(int, char **);
void 		RedrawWindow(void);
void 		RedrawWindowXY(int, int);
void  		copyXPMArea(int, int, int, int, int, int);
void  		copyXBMArea(int, int, int, int, int, int);
void  		setMaskXY(int, int);
unsigned long 	getColor(char *, float, int *, int *, int *);
unsigned long 	getBlendedColor(char *, float, int, int, int);
void 		RedrawWindow(void);
void 		uusleep( unsigned long );
void 		short_uusleep( unsigned long );



#endif
