#define NORMSIZE    64
#define ASTEPSIZE   56
#define NAME        "wmmixer"
#define CLASS       "WMMixer"

#define BACKCOLOR   "#282828"
#define LEDCOLOR    "LightSeaGreen"

#define CARD_NUM 0
#define MIXER_NUM 0

#define VERSION "0.6"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <sys/asoundlib.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

Pixmap pm_main;
Pixmap pm_tile;
Pixmap pm_disp;
Pixmap pm_mask;
Pixmap pm_icon;

#include "XPM/wmmixer.xpm"
#include "XPM/tile.xpm"
#include "XPM/icons.xpm"

int wmaker;
int ushape;
int astep;
char display[32];
char position[32];
int winsize;

char backcolor[32];
char ledcolor[32];

Atom _XA_GNUSTEP_WM_FUNC;
Atom deleteWin;
Display *d_display;
Window w_icon;
Window w_main;
Window w_root;
Window w_activewin;

GC gc_gc;
unsigned long color[4];

snd_mixer_t *mixer_handle;
snd_mixer_elements_t elements;

typedef struct elementinfoStruct {
	snd_mixer_element_info_t info;
	snd_mixer_element_t element;
	int icon;
	struct elementinfoStruct *next;
	struct elementinfoStruct *prev;
} elementinfo;

elementinfo *element;
elementinfo *cure;

int curchannel;
int curleft;
int curright;

int btnstate;
#define BTNNEXT  1
#define BTNPREV  2
#define BTNREC   4
#define RPTINTERVAL   5

int rpttimer;
int dragging;
int count;
int channel[80];

void initXWin(int argc, char **argv);
void freeXWin();
void createWin(Window *win, int x, int y);
unsigned long getColor(char *colorname);
unsigned long mixColor(char *colorname1, int prop1, char *colorname2, int prop2);
void scanArgs(int argc, char **argv);

void checkVol(void);
void setVol(int left, int right);
void callbacks(void);
int convert_range(int val, int omin, int omax, int nmin, int nmax);
void pressEvent(XButtonEvent *xev);
void releaseEvent(XButtonEvent *xev);
void motionEvent(XMotionEvent *xev);
void repaint();
void update();
void drawLeft();
void drawRight();
void drawBtns(int btns);
void drawBtn(int x, int y, int w, int h, int down);

void init_mixer(void);
void init_elements(void);
elementinfo *add_element(void);
