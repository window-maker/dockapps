#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>


/***********/
 /* Defines */
/***********/

#define MAX_MOUSE_REGION (16)

/************/
 /* Typedefs */
/************/

typedef struct _rckeys rckeys;

struct _rckeys {
	const char *label;
	char **var;
};

typedef struct _rckeys2 rckeys2;

struct _rckeys2 {
	const char *family;
	const char *label;
	char **var;
};

typedef struct {
	Pixmap pixmap;
	Pixmap mask;
	XpmAttributes attributes;
} XpmIcon;

/*******************/
 /* Global variable */
/*******************/

Display *display;
Window Root, iconwin, win;
XpmIcon wmgen;
XpmIcon wmfont;

/***********************/
 /* Function Prototypes */
/***********************/

void AddMouseRegion(int index, int left, int top, int right, int bottom);
int CheckMouseRegion(int x, int y);

void openXwindow(int argc, char *argv[], char **, char *, int, int);
void RedrawWindow(void);
void RedrawWindowXY(int x, int y);

void createXBMfromXPM(char *, char **, int, int);
void copyXPMArea(int, int, int, int, int, int);
void copyXBMArea(int, int, int, int, int, int);
void setMaskXY(int, int);

void parse_rcfile(const char *, rckeys *);

void font_init();
void draw_char(char c, int x, int y);
void draw_string(char *s, int x, int y);

#endif
