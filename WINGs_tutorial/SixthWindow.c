#include <WINGs/WINGs.h>
#include <WINGs/WINGsP.h>

#define WINWIDTH 200
#define WINHEIGHT 300

Display *display;
WMScreen *screen;
WMPixmap* pixmap;

void closeAction(WMWidget *self,void *data){
 WMDestroyWidget(self);
 exit(0);
}

void drawProcedure(XEvent *event, void *data){
 WMDrawPixmap(pixmap, W_VIEW_DRAWABLE(WMWidgetView(data)),30,30);XFlush(display);
}


int main (int argc, char **argv){
WMColor *color;
WMWindow * win;
RImage *image;

RColor one, two={0xaf, 0x0f,0xff,0x33};
one.red=0x20;
one.green=0x20;
one.blue=0x20;
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


image=RCreateImage( 100,100,.8);
RFillImage(image, &two);
RDrawLine(image, 50,10,90,90,&one);
RDrawLine(image, 10,90,50,10,&one);
RDrawLine(image, 10,90,90,90,&one);

WMRealizeWidget(win);

pixmap=WMCreatePixmapFromRImage(screen, image,1);
WMCreateEventHandler(WMWidgetView(win), ExposureMask,drawProcedure,win);

WMMapWidget(win);
WMScreenMainLoop(screen);
}
