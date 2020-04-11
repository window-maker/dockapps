#include <WINGs/WINGs.h>
#include <WINGs/WINGsP.h>

#define HOFF 40
#define VOFF 160
#define WINWIDTH 180
#define WINHEIGHT 300

Display *display;
WMScreen *screen;
WMPixmap* pixmap;

struct _pict{
   Drawable dwin;
   XSegment segments[40];
   int seglen;
 } pic;

GC gc, g3;

void closeAction(WMWidget *self,void *data){
 WMDestroyWidget(self);
 exit(0);
}

void drawProcedure(XEvent *event, void *data){

  WMDrawPixmap(pixmap, ((struct _pict*)data)->dwin,HOFF,30);
  XDrawRectangle(display,((struct _pict*)data)->dwin,g3, HOFF,VOFF,100,100);
  XFillRectangle(screen->display, ((struct _pict*)data)->dwin  , WMColorGC(screen->white),  HOFF, VOFF, 100, 100);
  XDrawSegments(display,  ((struct _pict*)data)->dwin, WMColorGC(screen->black),  ((struct _pict*)data)->segments,  ((struct _pict*)data)->seglen);
  XFlush(display);
  return;
}


int main (int argc, char **argv){
int i,j;
WMColor *color;
WMWindow * win;
RImage *image;
struct _pict pict;
Drawable de;

RColor one, two={0xaf, 0x0f,0xff,0x33};
one.red=247;
one.green=251;
one.blue=107;
one.alpha=0xff;


WMInitializeApplication("DrawWin", &argc, argv);
display = XOpenDisplay("");
screen = WMCreateScreen(display, DefaultScreen(display));
win = WMCreateWindow(screen, "");
WMResizeWidget(win, WINWIDTH, WINHEIGHT);
WMSetWindowCloseAction(win, closeAction, NULL);
WMSetWindowTitle(win,"Graphics");
color = WMCreateRGBColor(screen,124<<9,206<<8,162<<8, False);
WMSetWidgetBackgroundColor((WMWidget *)win, color);
       /* end setup main window */

image=RCreateImage( 100,100,0.5);
RFillImage(image, &two);
RDrawLine(image, 50,10,90,90,&one);
RDrawLine(image, 10,90,50,10,&one);
RDrawLine(image, 10,90,90,90,&one);

g3=WMColorGC(screen->gray);
XSetLineAttributes(display,g3,3,LineSolid,CapButt,JoinMiter);

pict.segments[1].x1= pict.segments[0].x1=HOFF;
pict.segments[0].x2=HOFF;
pict.segments[0].y1=VOFF;
pict.segments[1].y2=  pict.segments[0].y2=VOFF;
pict.segments[1].x2= HOFF+10;
pict.segments[1].y1=VOFF+10;
pict.seglen=2;
for (i=9;i>0;i--){
 j=2*(10-i);
 pict.segments[j+1].x1= pict.segments[j].x1=HOFF;
 pict.segments[j+1].y2= pict.segments[j].y2=VOFF;
 pict.segments[j].x2= i+pict.segments[j-1].x2;
 pict.segments[j].y1=i+pict.segments[j-1].y1;
 pict.segments[j+1].x2= i+pict.segments[j].x2;
 pict.segments[j+1].y1=i+pict.segments[j].y1;
 pict.seglen+=2;
};


WMRealizeWidget(win);

pict.dwin=W_VIEW_DRAWABLE(WMWidgetView(win));
pixmap=WMCreatePixmapFromRImage(screen, image,1);

WMCreateEventHandler(WMWidgetView(win), ExposureMask,drawProcedure,&pict);

WMMapWidget(win);
WMScreenMainLoop(screen);
}
