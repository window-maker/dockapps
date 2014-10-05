// wmsmixer - A mixer designed for WindowMaker with scrollwheel support
// Copyright (C) 2003  Damian Kramer <psiren@hibernaculum.net>
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the README file for a more complete notice.


// Defines, includes and global variables
// --------------------------------------

// User defines - standard
#define WINDOWMAKER false
#define USESHAPE    false
#define AFTERSTEP   false
#define NORMSIZE    64
#define ASTEPSIZE   56
#define NAME        "wmsmixer"
#define CLASS       "Wmsmixer"

#define VERSION "0.5.1"

// User defines - custom
#define MIXERDEV    "/dev/mixer"
#define BACKCOLOR   "#202020"
#define LEDCOLOR    "#00c9c1"

#undef CLAMP
#define CLAMP(x, l, h) (((x) > (h)) ? (h) : (((x) < (l)) ? (l) : (x)))

// Includes - standard
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// Includes - custom
#include "mixctl.h"

// X-Windows includes - standard
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

// Pixmaps - standard
Pixmap pm_main;
Pixmap pm_tile;
Pixmap pm_disp;
Pixmap pm_mask;

// Pixmaps - custom
Pixmap pm_icon;
Pixmap pm_digits;
Pixmap pm_chars;

// Xpm images - standard
#include "XPM/wmsmixer.xpm"
#include "XPM/tile.xpm"

// Xpm images - custom
#include "XPM/icons.xpm"
#include "XPM/digits.xpm"
#include "XPM/chars.xpm"

// Variables for command-line arguments - standard
bool wmaker=WINDOWMAKER;
bool ushape=USESHAPE;
bool astep=AFTERSTEP;
char display[256]="";
char position[256]="";
int winsize;
bool no_volume_display = 0;

// Variables for command-line arguments - custom
char mixdev[256]=MIXERDEV;
char backcolor[256]=BACKCOLOR;
char ledcolor[256]=LEDCOLOR;

// X-Windows basics - standard
Atom _XA_GNUSTEP_WM_FUNC;
Atom deleteWin;
Display *d_display;
Window w_icon;
Window w_main;
Window w_root;
Window w_activewin;

// X-Windows basics - custom
GC gc_gc;
unsigned long color[4];

int text_counter = 0;

// Misc custom global variables
// ----------------------------

// Current state information
int curchannel=0;
int curleft;
int curright;

// For buttons
int btnstate=0;
#define BTNNEXT  1
#define BTNPREV  2

// For repeating next and prev buttons
#define RPTINTERVAL   5
int rpttimer=0;

// For draggable volume control
bool dragging=false;

int channels=0;
int channel[25];
int icon[25]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
char *small_labels[25] = {"vol ", "bass", "trbl", "synt", "pcm ", "spkr", "line",
			  "mic ", "cd  ", "mix ", "pcm2", "rec ", "igai", "ogai",
			  "lin1", "lin2", "lin3", "dig1", "dig2", "dig3", "phin",
			  "phou", "vid ", "rad ", "mon "};

MixCtl *mixctl;


// Procedures and functions
// ------------------------

// Procedures and functions - standard
void initXWin(int argc, char **argv);
void freeXWin();
void createWin(Window *win, int x, int y);
unsigned long getColor(char *colorname);
unsigned long mixColor(char *colorname1, int prop1, char *colorname2, int prop2);

// Procedures and functions - custom
void scanArgs(int argc, char **argv);
void readFile();
void checkVol(bool forced);
void pressEvent(XButtonEvent *xev);
void releaseEvent(XButtonEvent *xev);
void motionEvent(XMotionEvent *xev);
void repaint();
void update();
void drawLeft();
void drawRight();
void drawMono();
void drawVolLevel();
void drawText(char *text);
void drawBtns(int btns);
void drawBtn(int x, int y, int w, int h, bool down);


// Implementation
// --------------

int main(int argc, char **argv)
{
  scanArgs(argc, argv);
  initXWin(argc, argv);

  XGCValues gcv;
  unsigned long gcm;
  gcm=GCGraphicsExposures;
  gcv.graphics_exposures=false;
  gc_gc=XCreateGC(d_display, w_root, gcm, &gcv);

  color[0]=mixColor(ledcolor, 0, backcolor, 100);
  color[1]=mixColor(ledcolor, 100, backcolor, 0);
  color[2]=mixColor(ledcolor, 60, backcolor, 40);
  color[3]=mixColor(ledcolor, 25, backcolor, 75);

  XpmAttributes xpmattr;
  XpmColorSymbol xpmcsym[4]={{"back_color",     NULL, color[0]},
			     {"led_color_high", NULL, color[1]},
			     {"led_color_med",  NULL, color[2]},
			     {"led_color_low",  NULL, color[3]}};
  xpmattr.numsymbols=4;
  xpmattr.colorsymbols=xpmcsym;
  xpmattr.exactColors=false;
  xpmattr.closeness=40000;
  xpmattr.valuemask=XpmColorSymbols | XpmExactColors | XpmCloseness;
  XpmCreatePixmapFromData(d_display, w_root, wmsmixer_xpm, &pm_main, &pm_mask, &xpmattr);
  XpmCreatePixmapFromData(d_display, w_root, tile_xpm, &pm_tile, NULL, &xpmattr);
  XpmCreatePixmapFromData(d_display, w_root, icons_xpm, &pm_icon, NULL, &xpmattr);
  XpmCreatePixmapFromData(d_display, w_root, digits_xpm, &pm_digits, NULL, &xpmattr);
  XpmCreatePixmapFromData(d_display, w_root, chars_xpm, &pm_chars, NULL, &xpmattr);
  pm_disp=XCreatePixmap(d_display, w_root, 64, 64, DefaultDepth(d_display, DefaultScreen(d_display)));


  if(wmaker || ushape || astep)
    XShapeCombineMask(d_display, w_activewin, ShapeBounding, winsize/2-32, winsize/2-32, pm_mask, ShapeSet);
  else
    XCopyArea(d_display, pm_tile, pm_disp, gc_gc, 0, 0, 64, 64, 0, 0);

  XSetClipMask(d_display, gc_gc, pm_mask);
  XCopyArea(d_display, pm_main, pm_disp, gc_gc, 0, 0, 64, 64, 0, 0);
  XSetClipMask(d_display, gc_gc, None);

  mixctl=new MixCtl(mixdev);

  if(!mixctl->openOK())
    fprintf(stderr,"%s : Unable to open mixer device '%s'.\n", NAME, mixdev);
  else{
    for(int i=0;i<mixctl->getNrDevices();i++){
      if(i==25){
	fprintf(stderr,"%s : Sorry, can only use channels 0-24\n", NAME);
	break;
      }
      if(mixctl->getSupport(i)){
	channel[channels]=i;
	channels++;
      }
    }
  }

  readFile();

  if(channels==0)
    fprintf(stderr,"%s : Sorry, no supported channels found.\n", NAME);
  else{
    checkVol(true);

    XEvent xev;
    XSelectInput(d_display, w_activewin, ExposureMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
    XMapWindow(d_display, w_main);

    bool done=false;
    while(!done){
      while(XPending(d_display)){
	XNextEvent(d_display, &xev);
	switch(xev.type){
	case Expose:
	  repaint();
	  break;
	case ButtonPress:
	  pressEvent(&xev.xbutton);
	  break;
	case ButtonRelease:
	  releaseEvent(&xev.xbutton);
	  break;
	case MotionNotify:
	  motionEvent(&xev.xmotion);
	  break;
	case ClientMessage:
	  if(xev.xclient.data.l[0]==deleteWin)
	    done=true;
	  break;
	}
      }

      if(btnstate & (BTNPREV | BTNNEXT)){
	rpttimer++;
	if(rpttimer>=RPTINTERVAL){
	  if(btnstate & BTNNEXT)
	    curchannel++;
	  else
	    curchannel--;
	  if(curchannel<0)
	    curchannel=channels-1;
	  if(curchannel>=channels)
	    curchannel=0;
	  checkVol(true);
	  rpttimer=0;
	}
      }
      else
	checkVol(false);

      if(text_counter) {
	text_counter--;
	if(!text_counter) {
	  drawVolLevel();
	  repaint();
	}
	//	printf("%c", text_counter);
      }

      XFlush(d_display);

      usleep(50000);
    }
  }
  XFreeGC(d_display, gc_gc);
  XFreePixmap(d_display, pm_main);
  XFreePixmap(d_display, pm_tile);
  XFreePixmap(d_display, pm_disp);
  XFreePixmap(d_display, pm_mask);
  XFreePixmap(d_display, pm_icon);
  XFreePixmap(d_display, pm_digits);
  XFreePixmap(d_display, pm_chars);
  freeXWin();
  delete mixctl;
  return 0;
}

void initXWin(int argc, char **argv)
{
  winsize=astep ? ASTEPSIZE : NORMSIZE;

  if((d_display=XOpenDisplay(display))==NULL){
    fprintf(stderr,"%s : Unable to open X display '%s'.\n", NAME, XDisplayName(display));
    exit(1);
  }
  _XA_GNUSTEP_WM_FUNC=XInternAtom(d_display, "_GNUSTEP_WM_FUNCTION", false);
  deleteWin=XInternAtom(d_display, "WM_DELETE_WINDOW", false);

  w_root=DefaultRootWindow(d_display);

  XWMHints wmhints;
  XSizeHints shints;
  shints.x=0;
  shints.y=0;
  shints.flags=0;
  bool pos=(XWMGeometry(d_display, DefaultScreen(d_display), position, NULL, 0, &shints, &shints.x, &shints.y,
			&shints.width, &shints.height, &shints.win_gravity) & (XValue | YValue));
  shints.min_width=winsize;
  shints.min_height=winsize;
  shints.max_width=winsize;
  shints.max_height=winsize;
  shints.base_width=winsize;
  shints.base_height=winsize;
  shints.flags=PMinSize | PMaxSize | PBaseSize;

  createWin(&w_main, shints.x, shints.y);

  if(wmaker || astep || pos)
    shints.flags |= USPosition;
  if(wmaker){
    wmhints.initial_state=WithdrawnState;
    wmhints.flags=WindowGroupHint | StateHint | IconWindowHint;
    createWin(&w_icon, shints.x, shints.y);
    w_activewin=w_icon;
    wmhints.icon_window=w_icon;
  }
  else{
    wmhints.initial_state=NormalState;
    wmhints.flags=WindowGroupHint | StateHint;
    w_activewin=w_main;
  }
  wmhints.window_group=w_main;
  XSetWMHints(d_display, w_main, &wmhints);
  XSetWMNormalHints(d_display, w_main, &shints);
  XSetCommand(d_display, w_main, argv, argc);
  XStoreName(d_display, w_main, NAME);
  XSetIconName(d_display, w_main, NAME);
  XSetWMProtocols(d_display, w_activewin, &deleteWin, 1);
}

void freeXWin()
{
  XDestroyWindow(d_display, w_main);
  if(wmaker)
    XDestroyWindow(d_display, w_icon);
  XCloseDisplay(d_display);
}

void createWin(Window *win, int x, int y)
{
  XClassHint classHint;
  *win=XCreateSimpleWindow(d_display, w_root, x, y, winsize, winsize, 0, 0, 0);
  classHint.res_name=NAME;
  classHint.res_class=CLASS;
  XSetClassHint(d_display, *win, &classHint);
}

unsigned long getColor(char *colorname)
{
  XColor color;
  XWindowAttributes winattr;
  XGetWindowAttributes(d_display, w_root, &winattr);
  color.pixel=0;
  XParseColor(d_display, winattr.colormap, colorname, &color);
  color.flags=DoRed | DoGreen | DoBlue;
  XAllocColor(d_display, winattr.colormap, &color);
  return color.pixel;
}

unsigned long mixColor(char *colorname1, int prop1, char *colorname2, int prop2)
{
  XColor color, color1, color2;
  XWindowAttributes winattr;
  XGetWindowAttributes(d_display, w_root, &winattr);
  XParseColor(d_display, winattr.colormap, colorname1, &color1);
  XParseColor(d_display, winattr.colormap, colorname2, &color2);
  color.pixel=0;
  color.red=(color1.red*prop1+color2.red*prop2)/(prop1+prop2);
  color.green=(color1.green*prop1+color2.green*prop2)/(prop1+prop2);
  color.blue=(color1.blue*prop1+color2.blue*prop2)/(prop1+prop2);
  color.flags=DoRed | DoGreen | DoBlue;
  XAllocColor(d_display, winattr.colormap, &color);
  return color.pixel;
}

void scanArgs(int argc, char **argv)
{
  for(int i=1;i<argc;i++){
    if(strcmp(argv[i], "-h")==0 || strcmp(argv[i], "--help")==0) {
      fprintf(stderr, "wmsmixer - A mixer designed for WindowMaker with scrollwheel support\n");
      fprintf(stderr, "Copyright (C) 2003  Damian Kramer <psiren@hibernaculum.net>\n");
      fprintf(stderr, "Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>\n");
      fprintf(stderr, "This software comes with ABSOLUTELY NO WARRANTY\n");
      fprintf(stderr, "This software is free software, and you are welcome to redistribute it\n");
      fprintf(stderr, "under certain conditions\n");
      fprintf(stderr, "See the README file for a more complete notice.\n\n");
      fprintf(stderr, "usage:\n\n   %s [options]\n\noptions:\n\n",argv[0]);
      fprintf(stderr, "   -h | --help            display this help screen\n");
      fprintf(stderr, "   -v | --version         display the version\n");
      fprintf(stderr, "   -w                     use WithdrawnState    (for WindowMaker)\n");
      fprintf(stderr, "   -s                     shaped window\n");
      fprintf(stderr, "   -a                     use smaller window    (for AfterStep Wharf)\n");
      fprintf(stderr, "   -l led_color           use the specified color for led display\n");
      fprintf(stderr, "   -b back_color          use the specified color for backgrounds\n");
      fprintf(stderr, "   -d mix_device          use specified device  (rather than /dev/mixer)\n");
      fprintf(stderr, "   -position position     set window position   (see X manual pages)\n");
      fprintf(stderr, "   -display display       select target display (see X manual pages)\n\n");
      exit(0);
    }
    if(strcmp(argv[i], "-v")==0 || strcmp(argv[i], "--version")==0) {
      fprintf(stderr, "wmsmixer version %s\n", VERSION);
      exit(0);
    }
    if(strcmp(argv[i], "-w")==0)
      wmaker=!wmaker;
    if(strcmp(argv[i], "-s")==0)
      ushape=!ushape;
    if(strcmp(argv[i], "-a")==0)
      astep=!astep;
    if(strcmp(argv[i], "-novol")==0)
      no_volume_display = 1;
    if(strcmp(argv[i], "-d")==0){
      if(i<argc-1){
	i++;
	sprintf(mixdev, "%s", argv[i]);
      }
      continue;
    }
    if(strcmp(argv[i], "-l")==0){
      if(i<argc-1){
	i++;
	sprintf(ledcolor, "%s", argv[i]);
      }
      continue;
    }
    if(strcmp(argv[i], "-b")==0){
      if(i<argc-1){
	i++;
	sprintf(backcolor, "%s", argv[i]);
      }
      continue;
    }
    if(strcmp(argv[i], "-position")==0){
      if(i<argc-1){
	i++;
	sprintf(position, "%s", argv[i]);
      }
      continue;
    }
    if(strcmp(argv[i], "-display")==0){
      if(i<argc-1){
	i++;
	sprintf(display, "%s", argv[i]);
      }
      continue;
    }
  }
}

void readFile()
{
  FILE *rcfile;
  char rcfilen[256];
  char buf[256];
  int done;
  int current=-1;
  sprintf(rcfilen, "%s/.wmsmixer", getenv("HOME"));
  if((rcfile=fopen(rcfilen, "r"))!=NULL){
    channels=0;
    do{
      fgets(buf, 250, rcfile);
      if((done=feof(rcfile))==0){
	buf[strlen(buf)-1]=0;
	if(strncmp(buf, "addchannel ", strlen("addchannel "))==0){
	  sscanf(buf, "addchannel %i", &current);
	  if(current>=mixctl->getNrDevices() || mixctl->getSupport(current)==false){
	    fprintf(stderr,"%s : Sorry, this channel (%i) is not supported.\n", NAME, current);
	    current=-1;
	  }
	  else{
	    channel[channels]=current;
	    channels++;
	  }
	}
	if(strncmp(buf, "setchannel ", strlen("setchannel "))==0){
	  sscanf(buf, "setchannel %i", &current);
	  if(current>=mixctl->getNrDevices() || mixctl->getSupport(current)==false){
	    fprintf(stderr,"%s : Sorry, this channel (%i) is not supported.\n", NAME, current);
	    current=-1;
	  }
	}
	if(strncmp(buf, "setname ", strlen("setname "))==0){
	  if(current==-1)
	    fprintf(stderr,"%s : Sorry, no current channel.\n", NAME);
	  else {
	    small_labels[current] = (char *)malloc(sizeof(char)*5);
	    sscanf(buf, "setname %4s", small_labels[current]);
	  }
	}
	if(strncmp(buf, "setmono ", strlen("setmono "))==0){
	  if(current==-1)
	    fprintf(stderr,"%s : Sorry, no current channel.\n", NAME);
	  else{
	    int value;
	    sscanf(buf, "setmono %i", &value);
	    mixctl->setLeft(current, value);
	    mixctl->setRight(current, value);
	    mixctl->writeVol(current);
	  }
	}
	if(strncmp(buf, "setleft ", strlen("setleft "))==0){
	  if(current==-1)
	    fprintf(stderr, "%s : Sorry, no current channel.\n", NAME);
	  else{
	    int value;
	    sscanf(buf, "setleft %i", &value);
	    mixctl->setLeft(current, value);
	    mixctl->writeVol(current);
	  }
	}
	if(strncmp(buf, "setright ", strlen("setright "))==0){
	  if(current==-1)
	    fprintf(stderr, "%s : Sorry, no current channel.\n", NAME);
	  else{
	    int value;
	    sscanf(buf, "setleft %i", &value);
	    mixctl->setRight(current, value);
	    mixctl->writeVol(current);
	  }
	}
      }
    }  while(done==0);
    fclose(rcfile);
  }
}

void checkVol(bool forced=true)
{
  mixctl->readVol(channel[curchannel], true);
  int nl=mixctl->readLeft(channel[curchannel]);
  int nr=mixctl->readRight(channel[curchannel]);
  if(forced){
    curleft=nl;
    curright=nr;
    update();
    repaint();
  }
  else{
    if(nl!=curleft || nr!=curright){
      if(nl!=curleft){
	curleft=nl;
	if(mixctl->getStereo(channel[curchannel]))
	  drawLeft();
	else
	  drawMono();
      }
      if(nr!=curright){
	curright=nr;
	if(mixctl->getStereo(channel[curchannel]))
	  drawRight();
	else
	  drawMono();
      }
      if(!no_volume_display)
	drawVolLevel();
      repaint();
    }
  }
}

void pressEvent(XButtonEvent *xev)
{
  if(xev->button == Button4 || xev->button == Button5) {
    int inc;
    if(xev->button == Button4) inc = 4;
    else inc = -4;

    mixctl->readVol(channel[curchannel], false);
    mixctl->setLeft(channel[curchannel],
		    CLAMP(mixctl->readLeft(channel[curchannel]) + inc, 0, 100));
    mixctl->setRight(channel[curchannel],
		     CLAMP(mixctl->readRight(channel[curchannel]) + inc, 0, 100));
    mixctl->writeVol(channel[curchannel]);
    checkVol(false);
    return;
  }

  int x=xev->x-(winsize/2-32);
  int y=xev->y-(winsize/2-32);
  if(x>=5 && y>=47 && x<=17 && y<=57){
    curchannel--;
    if(curchannel<0)
      curchannel=channels-1;
    btnstate |= BTNPREV;
    rpttimer=0;
    drawBtns(BTNPREV);
    checkVol(true);
    return;
  }
  if(x>=18 && y>=47 && x<=30 && y<=57){
    curchannel++;
    if(curchannel>=channels)
      curchannel=0;
    btnstate|=BTNNEXT;
    rpttimer=0;
    drawBtns(BTNNEXT);
    checkVol(true);
    return;
  }
  if(x>=37 && x<=56 && y>=8 && y<=56){
    int v=((60-y)*100)/(2*25);
    dragging=true;
    if(x<=50)
      mixctl->setLeft(channel[curchannel], v);
    if(x>=45)
      mixctl->setRight(channel[curchannel], v);
    mixctl->writeVol(channel[curchannel]);
    checkVol(false);
    return;
  }
  if(x>=5 && y>=21 && x<=30 && y<=42) {
    drawText(small_labels[channel[curchannel]]);
    return;
  }

}

void releaseEvent(XButtonEvent *xev)
{
  dragging=false;
  btnstate &= ~(BTNPREV | BTNNEXT);
  drawBtns(BTNPREV | BTNNEXT);
  repaint();
}

void motionEvent(XMotionEvent *xev)
{
  int x=xev->x-(winsize/2-32);
  int y=xev->y-(winsize/2-32);
  if(x>=37 && x<=56 && y>=8 && dragging){
    int v=((60-y)*100)/(2*25);
    if(v<0)
      v=0;
    if(x<=50)
      mixctl->setLeft(channel[curchannel], v);
    if(x>=45)
      mixctl->setRight(channel[curchannel], v);
    mixctl->writeVol(channel[curchannel]);
    checkVol(false);
  }
}

void repaint()
{
  XCopyArea(d_display, pm_disp, w_activewin, gc_gc, 0, 0, 64, 64, winsize/2-32, winsize/2-32);
  XEvent xev;
  while(XCheckTypedEvent(d_display, Expose, &xev));
}

void update()
{
  drawText(small_labels[channel[curchannel]]);

  XCopyArea(d_display, pm_icon, pm_disp, gc_gc, icon[channel[curchannel]]*26, 0, 26, 24, 5, 19);
  if(mixctl->getStereo(channel[curchannel])) {
    drawLeft();
    drawRight();
  }
  else {
    drawMono();
  }
}

void drawText(char *text)
{
  char *p = text;
  char p2;

  for(int i=0; i<4; i++, p++) {
    p2 = toupper(*p);
    if(p2 >= 'A' && p2 <= 'Z') {
      XCopyArea(d_display, pm_chars, pm_disp, gc_gc, 6*((int)p2-65), 0, 6, 9, 5+(i*6), 5);
    }
    else if(p2 >= '0' && p2 <= '9') {
      XCopyArea(d_display, pm_digits, pm_disp, gc_gc, 6*((int)p2-48), 0, 6, 9, 5+(i*6), 5);
    }
    else {
      if(p2 == '\0')
	p--;
      XCopyArea(d_display, pm_digits, pm_disp, gc_gc, 60, 0, 6, 9, 5+(i*6), 5);
    }
  }
  if(!no_volume_display)
    text_counter = 10;
}

void drawVolLevel()
{
  int digits[4];

  int vol = (mixctl->readLeft(channel[curchannel]) +
	     mixctl->readRight(channel[curchannel])) / 2;

  digits[0] = (vol/100) ? 1 : 10;
  digits[1] = (vol/10) == 10 ? 0 : (vol/10);
  digits[2] = vol%10;
  digits[3] = 10;

  for(int i=0; i<4; i++) {
    XCopyArea(d_display, pm_digits, pm_disp, gc_gc, 6*digits[i], 0, 6, 9, 5+(i*6), 5);
  }
}

void drawLeft()
{
  XSetForeground(d_display, gc_gc, color[0]);
  XFillRectangle(d_display, pm_disp, gc_gc, 46, 7, 2, 49);

  XSetForeground(d_display, gc_gc, color[1]);
  for(int i=0;i<25;i++) {
    if(i==(curleft*25)/100)
      XSetForeground(d_display, gc_gc, color[3]);
    XFillRectangle(d_display, pm_disp, gc_gc, 37, 55-2*i, 9, 1);
  }
}

void drawRight()
{
  XSetForeground(d_display, gc_gc, color[0]);
  XFillRectangle(d_display, pm_disp, gc_gc, 46, 7, 2, 49);

  XSetForeground(d_display, gc_gc, color[1]);
  for(int i=0;i<25;i++) {
    if(i==(curright*25)/100)
      XSetForeground(d_display, gc_gc, color[3]);
    XFillRectangle(d_display, pm_disp, gc_gc, 48, 55-2*i, 9, 1);
  }
}

void drawMono()
{
  XSetForeground(d_display, gc_gc, color[1]);
  for(int i=0;i<25;i++){
    if(i==(curright*25)/100)
      XSetForeground(d_display, gc_gc, color[3]);
    XFillRectangle(d_display, pm_disp, gc_gc, 37, 55-2*i, 20, 1);
  }
}


void drawBtns(int btns)
{
  if(btns & BTNPREV)
    drawBtn(5, 47, 13, 11, (btnstate & BTNPREV));
  if(btns & BTNNEXT)
    drawBtn(18, 47, 13, 11, (btnstate & BTNNEXT));
}

void drawBtn(int x, int y, int w, int h, bool down)
{
  if(!down)
    XCopyArea(d_display, pm_main, pm_disp, gc_gc, x, y, w, h, x, y);
  else {
    XCopyArea(d_display, pm_main, pm_disp, gc_gc, x, y, 1, h-1, x+w-1, y+1);
    XCopyArea(d_display, pm_main, pm_disp, gc_gc, x+w-1, y+1, 1, h-1, x, y);
    XCopyArea(d_display, pm_main, pm_disp, gc_gc, x, y, w-1, 1, x+1, y+h-1);
    XCopyArea(d_display, pm_main, pm_disp, gc_gc, x+1, y+h-1, w-1, 1, x, y);
  }
}
