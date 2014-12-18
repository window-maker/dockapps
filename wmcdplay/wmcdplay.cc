// wmcdplay - A cd player designed for WindowMaker
// 05/09/98  Release 1.0 Beta1
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
#define NAME        "wmcdplay"
#define CLASS       "WMCDPlay"

// User defines - custom
#define SYSARTDIR   "/usr/X11R6/lib/X11/wmcdplay/"
#define CDDEV       "/dev/cdrom"
#define BACKCOLOR   "#282828"
#define LEDCOLOR    "green"
#define POSRELABS 0            // 0=relative position, 1=absolute position
#define UINTERVAL_N 1          // 20ths of a second
#define UINTERVAL_E 20         // 20ths of a second

// Includes - standard
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Includes - custom
#include "cdctl.h"

// X-Windows includes - standard
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

// Pixmaps - standard
Pixmap pm_tile;
Pixmap pm_disp;
Pixmap pm_mask;

// Pixmaps - artwork
Pixmap pm_cd;
Pixmap pm_cdmask;
Pixmap pm_sym;
Pixmap pm_symmask;
Pixmap pm_led;
Pixmap pm_sled;
Pixmap pm_tled;

// Xpm images - standard
#include "XPM/tile.xpm"

// Xpm images - artwork
#include "XPM/standard.art"

// Variables for command-line arguments - standard
bool wmaker=WINDOWMAKER;
bool ushape=USESHAPE;
bool astep=AFTERSTEP;
char display[256]="";
char position[256]="";
int winsize;

// Variables for command-line arguments - custom
char cddev[256]=CDDEV;
char backcolor[256]=BACKCOLOR;
char ledcolor[256]=LEDCOLOR;
bool artwrk=false;
char artwrkf[256]="";
int tsel=1;
int vol=-1;         // -1 means don't set volume
int uinterval_e=UINTERVAL_E;

// X-Windows basics - standard
Atom _XA_GNUSTEP_WM_FUNC;
Atom deleteWin;
Display *d_display;
Window w_icon;
Window w_main;
Window w_root;
Window w_activewin;

// X-Windows basics - custom
GC gc_gc, gc_bitgc;
unsigned long color[4];


// Misc custom global variables 
// ----------------------------

// For artwork loading
int **art_btnlist;
int *art_btnptr;
int *art_actptr;
int art_symsize[2];
int art_ledsize[6];

int mode=-1, track=-1, pos=-1;
int tdisplay=POSRELABS;
int ucount=0;
char trackstr[8]="";
char timestr[8]="";
char chrset[]="00112233445566778899  DDAATTNNOOCC--PPEE:;_";

CDCtl *cdctl;


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
void checkStatus(bool forced);
void pressEvent(XButtonEvent *xev);
void repaint();
void update();
void drawText(int x, int y, char *text);

// Procedures and functions - artwork basics
bool readArtwork(char *artfilen);
char *readBlock(FILE *dfile);
int arrayItems(char *buf);
void readArrayInt(char *buf, int *array, int n);
void readArrayBool(char *buf, bool *array, int n);

// Procedures and functions - artwork specials
void createPixmap(char **data, char *buf, Pixmap *image, Pixmap *mask, int *width, int *height);
void setBtnList(int *bset);
bool inPolygon(int *points, int px, int py);


// Implementation
// --------------

int main(int argc, char **argv)
{
   scanArgs(argc, argv);
   initXWin(argc, argv);

   color[0]=mixColor(ledcolor, 0, backcolor, 100);
   color[1]=mixColor(ledcolor, 100, backcolor, 0);
   color[2]=mixColor(ledcolor, 60, backcolor, 40);
   color[3]=mixColor(ledcolor, 25, backcolor, 75);

   if(artwrk)
      artwrk=readArtwork(artwrkf);
   if(!artwrk){
      int w, h;
      createPixmap(cdplayer_xpm, NULL, &pm_cd, &pm_cdmask, NULL, NULL);
      createPixmap(symbols_xpm, NULL, &pm_sym, &pm_symmask, &w, &h);
      art_symsize[0]=(w+1)/11-1;
      art_symsize[1]=h;
      createPixmap(led_xpm, NULL, &pm_led, NULL, &w, &h);
      art_ledsize[0]=(w+1)/strlen(chrset)-1;
      art_ledsize[1]=h;
      createPixmap(ledsym_xpm, NULL, &pm_sled, NULL, &w, &h);
      art_ledsize[2]=(w+1)/6-1;
      art_ledsize[3]=h;
      createPixmap(ledtsel_xpm, NULL, &pm_tled, NULL, &w, &h);
      art_ledsize[4]=(w+1)/5-1;
      art_ledsize[5]=h;
      art_btnptr=art_btns;
      art_actptr=art_actions;
   }
   setBtnList(art_btnptr);
   createPixmap(tile_xpm, NULL, &pm_tile, NULL, NULL, NULL);
   pm_disp = XCreatePixmap(d_display, w_root, 64, 64, DefaultDepth(d_display, DefaultScreen(d_display)));
   pm_mask = XCreatePixmap(d_display, w_root, 64, 64, 1);

   XGCValues gcv;
   unsigned long gcm;
   gcm=GCGraphicsExposures;
   gcv.graphics_exposures=false;
   gc_gc=XCreateGC(d_display, w_root, gcm, &gcv);
   gc_bitgc=XCreateGC(d_display, pm_mask, gcm, &gcv);

   cdctl=new CDCtl(cddev);

   if(!cdctl->openOK())
      fprintf(stderr, "%s : Unable to open cdrom device '%s'.\n", NAME, cddev);
   else{
      if(vol!=-1)
         cdctl->setVolume(vol, vol);
      int tsels[] = { tsNone, tsNext, tsRepeat, tsRepeatCD, tsRandom };
      cdctl->setTrackSelection(tsels[tsel]);

      checkStatus(true);

      XEvent xev;
      XSelectInput(d_display, w_activewin, ButtonPress | ExposureMask);
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
             case ClientMessage:
                if(xev.xclient.data.l[0]==deleteWin)
                   done=true;
             break;
            }
         }
         ucount++;
         if(ucount>=((mode==ssNoCD || mode==ssTrayOpen) ? uinterval_e : UINTERVAL_N))
            checkStatus(false);
         XFlush(d_display);
         usleep(50000);
      }
   }
   XFreeGC(d_display, gc_gc);
   XFreeGC(d_display, gc_bitgc);
   XFreePixmap(d_display, pm_tile);
   XFreePixmap(d_display, pm_disp);
   XFreePixmap(d_display, pm_mask);
   XFreePixmap(d_display, pm_cd);
   XFreePixmap(d_display, pm_cdmask);
   XFreePixmap(d_display, pm_sym);
   XFreePixmap(d_display, pm_symmask);
   XFreePixmap(d_display, pm_led);
   XFreePixmap(d_display, pm_sled);
   XFreePixmap(d_display, pm_tled);
   freeXWin();
   if(artwrk){
      free(art_btnptr);
      free(art_actptr);
   }
   free(art_btnlist);
   delete cdctl;
   return 0;
}

void initXWin(int argc, char **argv){
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

void freeXWin(){
   XDestroyWindow(d_display, w_main);
   if(wmaker)
      XDestroyWindow(d_display, w_icon);
   XCloseDisplay(d_display);
}

void createWin(Window *win, int x, int y){
   XClassHint classHint;
   *win=XCreateSimpleWindow(d_display, w_root, x, y, winsize, winsize, 0, 0, 0);
   classHint.res_name=NAME;
   classHint.res_class=CLASS;
   XSetClassHint(d_display, *win, &classHint);
}

unsigned long getColor(char *colorname){
   XColor color;
   XWindowAttributes winattr;
   XGetWindowAttributes(d_display, w_root, &winattr);
   color.pixel=0;
   XParseColor(d_display, winattr.colormap, colorname, &color);
   color.flags=DoRed | DoGreen | DoBlue;
   XAllocColor(d_display, winattr.colormap, &color);
   return color.pixel;
}

unsigned long mixColor(char *colorname1, int prop1, char *colorname2, int prop2){
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

void scanArgs(int argc, char **argv){
   for(int i=1;i<argc;i++){
      if(strcmp(argv[i], "-h")==0 || strcmp(argv[i], "-help")==0 || strcmp(argv[i], "--help")==0){
         fprintf(stderr, "wmcdplay - A cd player designed for WindowMaker\n05/09/98  Release 1.0 Beta1\n");
         fprintf(stderr, "Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>\n");
         fprintf(stderr, "This software comes with ABSOLUTELY NO WARRANTY\n");
         fprintf(stderr, "This software is free software, and you are welcome to redistribute it\n");
         fprintf(stderr, "under certain conditions\n");
         fprintf(stderr, "See the README file for a more complete notice.\n\n");
         fprintf(stderr, "usage:\n\n   %s [options]\n\noptions:\n\n",argv[0]);
         fprintf(stderr, "   -h | -help | --help    display this help screen\n");
         fprintf(stderr, "   -w                     use WithdrawnState    (for WindowMaker)\n");
         fprintf(stderr, "   -s                     shaped window\n");
         fprintf(stderr, "   -a                     use smaller window    (for AfterStep Wharf)\n");
         fprintf(stderr, "   -f artwork_file        load the specified artwork file\n");
         fprintf(stderr, "   -t track_selection     set track selection   (between 0 and 4)\n");
         fprintf(stderr, "   -v volume              set the cdrom volume  (between 0 and 255)\n");
         fprintf(stderr, "   -i interval            interval in 1/20 seconds between cd polls when empty\n");
         fprintf(stderr, "   -l led_color           use the specified color for led displays\n");
         fprintf(stderr, "   -b back_color          use the specified color for backgrounds\n");
         fprintf(stderr, "   -d cd_device           use specified device  (rather than /dev/cdrom)\n");
         fprintf(stderr, "   -position position     set window position   (see X manual pages)\n");
         fprintf(stderr, "   -display display       select target display (see X manual pages)\n\n");
         exit(0);
      }
      if(strcmp(argv[i], "-w")==0)
         wmaker=!wmaker;
      if(strcmp(argv[i], "-s")==0)
         ushape=!ushape;
      if(strcmp(argv[i], "-a")==0)
         astep=!astep;
      if(strcmp(argv[i], "-t")==0){
	 if(i<argc-1){
            i++;
            sscanf(argv[i], "%i", &tsel);
	 }
         continue;
      }
      if(strcmp(argv[i], "-v")==0){
         if(i<argc-1){
            i++;
            sscanf(argv[i], "%i", &vol);
         }
         continue;
      }
      if(strcmp(argv[i], "-i")==0){
         if(i<argc-1){
            i++;
            sscanf(argv[i], "%i", &uinterval_e);
         }
         continue;
      }
      if(strcmp(argv[i], "-f")==0){
         artwrk=true;
	 if(i<argc-1){
            i++;
            sprintf(artwrkf, "%s", argv[i]);
         }
         continue;
      }
      if(strcmp(argv[i], "-d")==0){
	 if(i<argc-1){
            i++;
            sprintf(cddev, "%s", argv[i]);
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

void checkStatus(bool forced){
   ucount=0;
   int oldmode=mode;
   int oldpos=pos;
   int oldtrack=track;

   cdctl->doStatus();
   mode=cdctl->getStatusState();
   track=cdctl->getStatusTrack();

   if(mode==ssStopped){
      if(tdisplay==0)
         pos=0;
      if(tdisplay==1)
         pos=cdctl->getTrackStart(track);
   }
   if(mode==ssPlaying || mode==ssPaused){
      if(tdisplay==0)
         pos=cdctl->getStatusPosRel();
      if(tdisplay==1)
         pos=cdctl->getStatusPosAbs();
   }

   bool umode=mode!=oldmode || forced;
   bool utrack=umode || (!(mode==ssNoCD || mode==ssTrayOpen) && track!=oldtrack);
   bool utimer=utrack || ((mode==ssPlaying || mode==ssPaused || mode==ssStopped) && (int)(pos/75)!=(int)(oldpos/75));

   if(utimer){
      if(umode)
         update();
      if(utrack){
         if(mode==ssNoCD || mode==ssTrayOpen)
            sprintf(trackstr, "  ");
         else
            sprintf(trackstr, "%2d", cdctl->getStatusTrack());
         if(art_showled[1])
            drawText(art_ledpos[1][0], art_ledpos[1][1], trackstr);
      }
      if(mode==ssPlaying || mode==ssPaused || mode==ssStopped){
         int remain = 0;
         if(tdisplay==0)
            remain=cdctl->getTrackLen(cdctl->getStatusTrack())-pos;
         if(tdisplay==1)
            remain=cdctl->getCDLen()-pos;
         if(remain<2250)
            sprintf(timestr, " -;%02d", remain/75);
         else
            sprintf(timestr, "%2d:%02d", (pos/75)/60, (pos/75)%60);
      }
      if(art_showled[0])
         drawText(art_ledpos[0][0], art_ledpos[0][1], timestr);
      repaint();
   }
}

void pressEvent(XButtonEvent *xev){
   int x=xev->x-(winsize/2-32);
   int y=xev->y-(winsize/2-32);
   int btn=-1;
   for(int i=0;i<art_nbtns;i++){
      if(inPolygon(&art_btnlist[i][2], x, y))
         btn=i;
   }
   if(btn==-1){
      if(art_showled[3]){
         if(x>=art_ledpos[3][0] && y>=art_ledpos[3][1] && x<=art_ledpos[3][0]+art_ledsize[4] && y<=art_ledpos[3][1]+art_ledsize[5]){ 
            int tsels[] = { tsNone, tsNext, tsRepeat, tsRepeatCD, tsRandom };
            tsel++;
            if(tsel>=5)
               tsel=0;
            cdctl->setTrackSelection(tsels[tsel]);
            XCopyArea(d_display, pm_tled, pm_disp, gc_gc, (art_ledsize[4]+1)*tsel, 0, art_ledsize[4], art_ledsize[5], art_ledpos[3][0], art_ledpos[3][1]);
            repaint();
            return;
         }
      }
      if(art_showled[0]){
         if(x>=art_ledpos[0][0] && y>=art_ledpos[0][1] && x<=art_ledpos[0][0]+(art_ledsize[0]+1)*9-1 && y<=art_ledpos[0][1]+art_ledsize[1]){ 
            tdisplay++;
            if(tdisplay>=2)
               tdisplay=0;
            checkStatus(false);
            return;
         }
      }
   }
   else{
      int action=art_actptr[6*btn+mode];
      int acmds[]={ acStop, acPlay, acPause, acResume, acPrev, acNext, acRewd, acFFwd, acEject, acClose };
      if(action>0){
         int acmd=acmds[action-1];
         cdctl->doAudioCommand(acmd);
         checkStatus(false);
      }
   }
}

void repaint(){
   XCopyArea(d_display, pm_disp, w_activewin, gc_gc, 0, 0, 64, 64, winsize/2-32, winsize/2-32);
   XEvent xev;
   while(XCheckTypedEvent(d_display, Expose, &xev));
}

void update(){
   if(mode==ssData)
      sprintf(timestr, "DA_TA");
   if(mode==ssNoCD)
      sprintf(timestr, "NO;CD");
   if(mode==ssTrayOpen)
      sprintf(timestr, "OP_EN");

   XPoint mply[art_nbtns];
   if(pm_cdmask!=None){
      XSetForeground(d_display, gc_bitgc, 0);
      XCopyArea(d_display, pm_cdmask, pm_mask, gc_bitgc, 0, 0, 64, 64, 0, 0);
      for(int i=0; i<art_nbtns; i++){
         if(art_actptr[6*i+mode]==0 && art_hidebtns){
            for(int k=0;k<art_btnlist[i][2];k++){
               mply[k].x=art_btnlist[i][k*2+3];
               mply[k].y=art_btnlist[i][k*2+4];
            }
            XFillPolygon(d_display, pm_mask, gc_bitgc, (XPoint *)mply, art_btnlist[i][2], Convex, CoordModeOrigin);
         }
      }
      if(!(wmaker || ushape || astep)){
         XCopyArea(d_display, pm_tile, pm_disp, gc_gc, 0, 0, 64, 64, 0, 0);
         XSetClipMask(d_display, gc_gc, pm_mask);
      }
   }
   XCopyArea(d_display, pm_cd, pm_disp, gc_gc, 0, 0, 64, 64, 0, 0);
   if(pm_symmask!=None){
      XSetClipMask(d_display, gc_gc, pm_symmask);
      XSetClipMask(d_display, gc_bitgc, pm_symmask);
   }
   XSetForeground(d_display, gc_bitgc, 1);
   for(int i=0;i<art_nbtns;i++){
      if(!(art_actptr[6*i+mode]==0 && art_hidebtns)){
         int sympos=(art_symsize[0]+1)*(art_actptr[6*i+mode]);
         XSetClipOrigin(d_display, gc_gc, art_btnlist[i][0]-sympos, art_btnlist[i][1]);
         XSetClipOrigin(d_display, gc_bitgc, art_btnlist[i][0]-sympos, art_btnlist[i][1]);
         XCopyArea(d_display, pm_sym, pm_disp, gc_gc, sympos, 0, art_symsize[0], art_symsize[1], art_btnlist[i][0], art_btnlist[i][1]);
         XFillRectangle(d_display, pm_mask, gc_bitgc, art_btnlist[i][0], art_btnlist[i][1], art_symsize[0], art_symsize[1]);
      }
   }
   if(wmaker || ushape || astep)
      XShapeCombineMask(d_display, w_activewin, ShapeBounding, winsize/2-32, winsize/2-32, pm_mask, ShapeSet);
   XSetClipOrigin(d_display, gc_gc, 0, 0);
   XSetClipOrigin(d_display, gc_bitgc, 0, 0);
   XSetClipMask(d_display, gc_gc, None);
   XSetClipMask(d_display, gc_bitgc, None);
   if(art_showled[2])
      XCopyArea(d_display, pm_sled, pm_disp, gc_gc, (art_ledsize[2]+1)*mode, 0, art_ledsize[2], art_ledsize[3], art_ledpos[2][0], art_ledpos[2][1]);
   if(art_showled[3])
      XCopyArea(d_display, pm_tled, pm_disp, gc_gc, (art_ledsize[4]+1)*tsel, 0, art_ledsize[4], art_ledsize[5], art_ledpos[3][0], art_ledpos[3][1]);
}

void drawText(int x, int y, char *text){
   int drawx=x;
   for(int i=0;i<strlen(text);i++){
      char *chrptr=strchr(chrset,text[i]);
      if(chrptr!=NULL){
         int chrindex=chrptr-chrset;
         int chrwidth=art_ledsize[0];
         if(chrset[chrindex+1]==text[i])
            chrwidth=2*art_ledsize[0]+1;
         XCopyArea(d_display, pm_led, pm_disp, gc_gc, chrindex*(art_ledsize[0]+1), 0, chrwidth, art_ledsize[1], drawx, y);
         drawx+=chrwidth+1;
      }
   }
}

bool readArtwork(char *artfilen){
   FILE *artfile;
   char artfilenbuf[256];
   artfile=fopen(artfilen, "r");
   if(artfile==NULL){
      if(strchr(artfilen, '/')!=NULL){
         fprintf(stderr, "%s : Unable to open artwork file '%s'.\n", NAME, artfilen);
         return false;
      }
      sprintf(artfilenbuf, "%s/.wmcdplay/%s", getenv("HOME"), artfilen);
      artfile=fopen(artfilenbuf, "r");
      if(artfile==NULL){
         sprintf(artfilenbuf, "%s%s", SYSARTDIR, artfilen);
         artfile=fopen(artfilenbuf, "r");
         if(artfile==NULL){
            fprintf(stderr,"%s : Tried to find artwork file, but failed.\n", NAME);
            return false;
         }
      }
   }

   char buf[256];
   bool done=false;
   while(!done){
      fgets(buf, 250, artfile);
      done=(feof(artfile)!=0);
      if(!done){

         int keynum=0;
         char *keystr[]={ "int art_nbtns=",
                          "bool art_hidebtns=",
                          "bool art_showled[4]=",
                          "int art_ledpos[4][2]=",
                          "int art_btns[]=",
                          "int art_actions[]=",
                          "/* XPM */" };
         for(int i=0;i<7;i++){
            if(strncmp(buf, keystr[i], strlen(keystr[i]))==0){
               keynum=i+1;
               break;
            }
         }

         if(keynum==1)
            sscanf(buf+strlen(keystr[keynum-1]), "%d", &art_nbtns);

         if(keynum==2)
            art_hidebtns=(strstr(buf+strlen(keystr[keynum-1]), "true")!=NULL);

         if(keynum==3)
            readArrayBool((char *)buf, (bool *)art_showled, 4);

         if(keynum==4)
            readArrayInt((char *)buf, (int *)art_ledpos, 8);

         if(keynum>=5){
            fseek(artfile, -strlen(buf), SEEK_CUR);
            char *block=readBlock(artfile);

            if(keynum==5){
               int items=arrayItems(block);
               art_btnptr=(int *)malloc(sizeof(int)*items);
               readArrayInt(block, art_btnptr, items);
            }

            if(keynum==6){
               int items=arrayItems(block);
               art_actptr=(int *)malloc(sizeof(int)*items);
               readArrayInt(block, art_actptr, items);
            }

            if(keynum==7){

               strncpy(buf, strchr(block+strlen(keystr[keynum-1]), '\n')+1, 250);
               *strchr(buf, '\n')='\0';

               int w,h;
               if(strncmp(buf, "static char * cdplayer_xpm", strlen("static char * cdplayer_xpm"))==0)
                  createPixmap(NULL, block, &pm_cd, &pm_cdmask, NULL, NULL);
               if(strncmp(buf, "static char * symbols_xpm", strlen("static char * symbols_xpm"))==0){
                  createPixmap(NULL, block, &pm_sym, &pm_symmask, &w, &h);
                  art_symsize[0]=(w+1)/11-1;
                  art_symsize[1]=h;
               }
               if(strncmp(buf, "static char * led_xpm", strlen("static char * led_xpm"))==0){
                  createPixmap(NULL, block, &pm_led, NULL, &w, &h);
                  art_ledsize[0]=(w+1)/strlen(chrset)-1;
                  art_ledsize[1]=h;
               }
               if(strncmp(buf, "static char * ledsym_xpm", strlen("static char * ledsym_xpm"))==0){
                  createPixmap(NULL, block, &pm_sled, NULL, &w, &h);
                  art_ledsize[2]=(w+1)/6-1;
                  art_ledsize[3]=h;
               }
               if(strncmp(buf, "static char * ledtsel_xpm", strlen("static char * ledtsel_xpm"))==0){
                  createPixmap(NULL, block, &pm_tled, NULL, &w, &h);
                  art_ledsize[4]=(w+1)/5-1;
                  art_ledsize[5]=h;
               }

            }
            free(block);
         }
      }
   }
   fclose(artfile);
   return true;
}

char *readBlock(FILE *dfile){
   char buf[256];
   long bytes=0;
   char *block=NULL;
   do{
      fgets(buf, 250, dfile);
      int buflen=strlen(buf);
      block=(char *)realloc(block, sizeof(char)*(bytes+buflen+1));
      strcpy(block+bytes, buf);
      bytes+=buflen;
   }  while(strstr(buf, "}")==NULL);
   return block;
}

int arrayItems(char *buf){
   int items=1;
   char *bufptr=buf;
   while((bufptr=strstr(bufptr, ","))!=NULL){
      bufptr++;
      items++;
   };
   return items;
}

void readArrayInt(char *buf, int *array, int n){
   char *bufptr;
   bufptr=strtok(buf, "{,}");
   for(int i=0;i<n;i++){
      bufptr=strtok(NULL, "{,}");
      sscanf(bufptr, "%d", &array[i]);
   }
}

void readArrayBool(char *buf, bool *array, int n){
   char *bufptr;
   bufptr=strtok(buf, "{,}");
   for(int i=0;i<n;i++){
      bufptr=strtok(NULL, "{,}");
      array[i]=(strstr(bufptr, "true")!=NULL);
   }
}

void createPixmap(char **data, char *buf, Pixmap *image, Pixmap *mask, int *width, int *height){
   XpmAttributes xpmattr;
   XpmColorSymbol xpmcsym[4]={{"back_color",     NULL, color[0]},
                              {"led_color_high", NULL, color[1]},
                              {"led_color_med",  NULL, color[2]},
                              {"led_color_low",  NULL, color[3]}};
   xpmattr.numsymbols=4;
   xpmattr.colorsymbols=xpmcsym;
   xpmattr.exactColors=false;
   xpmattr.closeness=40000;
   xpmattr.valuemask=XpmColorSymbols | XpmExactColors | XpmCloseness | XpmSize;
   if(data!=NULL)
      XpmCreatePixmapFromData(d_display, w_root, data, image, mask, &xpmattr);
   else
      XpmCreatePixmapFromBuffer(d_display, w_root, buf, image, mask, &xpmattr);
   if(width!=NULL)
      *width=xpmattr.width;
   if(height!=NULL)
      *height=xpmattr.height;
}

void setBtnList(int *bset){
   // Create a list of pointers to button data.
   // So, for example, data for button 2 can be accessed as art_btnlist[2];
   // Also, the y co-ordinate of its symbol would be art_btnlist[2][1]

   art_btnlist=(int **)malloc(art_nbtns*sizeof(int *));
   int curpos=0;
   for(int i=0;i<art_nbtns;i++){
      art_btnlist[i]=&bset[0+curpos];
      curpos+=2*art_btnlist[i][2]+3;
   }
}

bool inPolygon(int *points, int px, int py){
   int lx=points[1];
   int ly=points[2];
   int x,y;
   for(int i=1;i<=points[0];i++){
      if(i==points[0]){
         x=points[1];
         y=points[2];
      }
      else{
         x=points[i*2+1];
         y=points[i*2+2];
      }
      int a=ly-y;
      int b=x-lx;
      int c=-a*x-b*y;
      if(a*px+b*py+c<0)
         return false;
      lx=x;
      ly=y;
   }
   return true;
}
