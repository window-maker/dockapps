#define MARGIN  14
#define WINWIDTH 300
#define WINHEIGHT 400

Display *display;
WMScreen *screen;

WMWindow *win;
WMSize ButtonsetSize;

WMText *text;
WMColor *color;
WMFrame *controlframe;

char textbuf[40];

void closeAll(WMWidget *self,void *data){
  WMDestroyWidget(self);
  fprintf(stderr,"I've been used!\n");
  exit(0);
}

static void selectFiles(void *self, void *data){
  int i=0;
     WMOpenPanel *oPanel;
     oPanel = WMGetOpenPanel(screen);
     if (WMRunModalFilePanelForDirectory(oPanel, NULL, "/tmp",
					    "Search..", NULL) == True){
     snprintf(textbuf,39,"%s\n-", WMGetFilePanelFileName(oPanel));
     WMFreezeText(text);
     WMAppendTextStream(text,textbuf);
     WMThawText(text);
     }
    return ;
}

static void handleEvents(XEvent *event, void *data){
   WMWidget *widget = (WMWidget*)data;
  switch (event->type) {
  case ButtonPress:
    snprintf(textbuf,39,"Button down at (%i,%i) \n-",event->xbutton.x,event->xbutton.y);
    WMFreezeText(text);
    WMAppendTextStream(text,textbuf);
    WMThawText(text);
    break;
  }
}

static void resizeHandler(void *self, WMNotification *notif){
     WMSize size = WMGetViewSize(WMWidgetView(win));   
     WMMoveWidget(controlframe, size.width-ButtonsetSize.width, size.height-ButtonsetSize.height);
     WMResizeWidget(text, size.width-MARGIN -10, size.height-80);
}


int main (int argc, char **argv){
 
WMButton *Button; 

 WMInitializeApplication("FifthWindow", &argc, argv);
 if (!(display = XOpenDisplay(""))){
   fprintf(stderr,"err: cannot open display");
   exit(1);
 }
 screen = WMCreateScreen(display, DefaultScreen(display));

      /*    window    */
   win = WMCreateWindow(screen, "");
   WMResizeWidget(win, WINWIDTH, WINHEIGHT);
   WMSetWindowCloseAction(win, closeAll, NULL);
   WMCreateEventHandler(WMWidgetView(win), ButtonPressMask,handleEvents, win);
   color = WMCreateRGBColor(screen, 124<<9,206<<8,162<<8, False);
   WMSetWidgetBackgroundColor((WMWidget *)win, color);
   WMSetViewNotifySizeChanges(WMWidgetView(win), True);
   WMAddNotificationObserver(resizeHandler, NULL, WMViewSizeDidChangeNotification, WMWidgetView(win));

        /* Text area */

    text = WMCreateText(win);
    WMResizeWidget(text, WINWIDTH-MARGIN, WINHEIGHT -80);
    WMMoveWidget(text, 10, 10);
    WMSetTextHasVerticalScroller(text, True);
    WMSetTextEditable(text, False);
  
       /* frame and two buttons */
	
   controlframe=WMCreateFrame(win);
   WMSetWidgetBackgroundColor((WMWidget *)controlframe, color);
   WMSetFrameRelief(controlframe,WRFlat);

   Button =WMCreateButton(controlframe,WBTMomentaryPush);
   WMSetWidgetBackgroundColor((WMWidget *)Button, color);
   WMSetButtonText (Button, "Files");
   WMSetButtonAction (Button, selectFiles, NULL);    
   ButtonsetSize = WMGetViewSize(WMWidgetView(Button));       
   WMMoveWidget(Button,  MARGIN, MARGIN);
 
   Button =WMCreateButton(controlframe,WBTMomentaryPush);
   WMSetWidgetBackgroundColor((WMWidget *)Button, color);
   WMSetButtonText (Button, "Quit");
   WMSetButtonAction (Button, closeAll, NULL);
   WMMoveWidget(Button,2*MARGIN+ButtonsetSize.width, MARGIN);
   ButtonsetSize.width = 3*MARGIN+2*ButtonsetSize.width;
   ButtonsetSize.height=2*MARGIN+ButtonsetSize.height;
   WMResizeWidget(controlframe,ButtonsetSize.width,ButtonsetSize.height);
  
   WMMapSubwidgets(controlframe);
   resizeHandler(NULL,NULL);
     /* end of frame and buttons setup */ 


  WMMapSubwidgets(win);
  WMMapWidget(win);
  WMRealizeWidget(win);

  WMScreenMainLoop(screen);

return 0;
}
