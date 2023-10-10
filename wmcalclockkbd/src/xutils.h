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
int LoadKbImg(char *names);
void CreateKbTranImgs(void);
void ShowGroupImage(int group);


#endif
