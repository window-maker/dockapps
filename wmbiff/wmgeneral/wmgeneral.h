#ifndef WMGENERAL_H_INCLUDED
#define WMGENERAL_H_INCLUDED

  /***********/
 /* Defines */
/***********/

#define MAX_MOUSE_REGION (40)

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

extern Display *display;

  /***********************/
 /* Function Prototypes */
/***********************/

void AddMouseRegion(unsigned int rgn_index, int left, int top, int right,
					int bottom);
int CheckMouseRegion(int x, int y);

void openXwindow(int argc, const char *argv[], const char **,
				 const char **, char *, int, int, int);
void RedrawWindow(void);
void RedrawWindowXY(int x, int y);

void createXBMfromXPM(char *, const char **, int, int);
void copyXPMArea(int, int, int, int, int, int);
void copyXBMArea(int, int, int, int, int, int);
void setMaskXY(int, int);

void parse_rcfile(const char *, rckeys *);

/* for wmbiff */
int loadFont(const char *fontname);	/* -1 on fail, 0 success. */
void drawString(int dest_x, int dest_y, const char *string,
				const char *colorname, const char *bgcolorname,
				int right_justify);
void eraseRect(int x, int y, int x2, int y2, const char *bgcolorname);
/* end wmbiff */

#endif
