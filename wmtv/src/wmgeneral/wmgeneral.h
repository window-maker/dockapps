#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED

  /***********/
 /* Defines */
/***********/

#define MAX_MOUSE_REGION (16)

  /************/
 /* Typedefs */
/************/

typedef struct _rckeys rckeys;

struct _rckeys {
	const char	*label;
	char		**var;
};

typedef struct _rckeys2 rckeys2;

struct _rckeys2 {
	const char	*family;
	const char	*label;
	char		**var;
};

typedef struct {
	Pixmap			pixmap;
	Pixmap			mask;
	XpmAttributes	attributes;
} XpmIcon;

  /*******************/
 /* Global variable */
/*******************/

extern Display		*display;

  /***********************/
 /* Function Prototypes */
/***********************/

void AddMouseRegion(int index, int left, int top, int right, int bottom);
int CheckMouseRegion(int x, int y);

void openXwindow(int argc, char *argv[], char **, char *, int, int);
void RedrawWindow(void);
void RedrawWindowXYWH(int x, int y, int w, int h);

void createXBMfromXPM(char *, char **, int, int);
void copyXPMArea(int, int, int, int, int, int);
void copyXBMArea(int, int, int, int, int, int);
void setMaskXY(int, int);

extern Window win, iconwin;
extern char *exef;
extern char *display_name;
extern char *geometry;
extern char *exe;
#endif
