#define MARGIN  14
#define WINWIDTH 300
#define WINHEIGHT 400

Display *display;
WMScreen *screen;

WMButton *Button; 
WMWindow *win;
WMSize ButtonsetSize;

WMBox *box;
WMText *text;
WMColor *color;

char textbuf[40];

void closeAll(WMWidget *self,void *data){
  WMDestroyWidget(self);
  fprintf(stderr,"I've been used!\n");
  exit(0);
}

static void selectFiles(void *self, void *data){
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
     WMMoveWidget(box, size.width-ButtonsetSize.width, size.height-ButtonsetSize.height);
     WMResizeWidget(text, size.width-MARGIN -10, size.height-80);
}


int main (int argc, char **argv){
 
 WMInitializeApplication("FourthWindow", &argc, argv);
 if (!(display = XOpenDisplay(""))){
   fprintf(stderr,"err: cannot open display");
   exit(-1);
 }
 screen = WMCreateScreen(display, DefaultScreen(display));

      /*    window    */
   win = WMCreateWindow(screen, "");
   WMResizeWidget(win, WINWIDTH, WINHEIGHT);
   WMSetWindowCloseAction(win, closeAll, NULL);

   color = WMCreateRGBColor(screen, 124<<9,206<<8,162<<8, False);
   WMSetWidgetBackgroundColor((WMWidget *)win, color);
 
   WMCreateEventHandler(WMWidgetView(win), ButtonPressMask,handleEvents, win);
   WMSetViewNotifySizeChanges(WMWidgetView(win), True);
   WMAddNotificationObserver(resizeHandler, NULL, WMViewSizeDidChangeNotification, WMWidgetView(win));

        /* Text area */

    text = WMCreateText(win);
    WMResizeWidget(text, WINWIDTH-MARGIN, WINHEIGHT -80);
    WMMoveWidget(text, 10, 10);
    WMSetTextHasVerticalScroller(text, True);
    WMSetTextEditable(text, False);
    WMSetTextIgnoresNewline(text, False);

      /* box with buttons */
   box=WMCreateBox(win);   
   WMSetBoxBorderWidth(box, MARGIN);
   WMSetWidgetBackgroundColor((WMWidget *)box, color);
   WMSetBoxHorizontal(box, True);  

 
   Button =WMCreateButton(box,WBTMomentaryPush);
   WMSetWidgetBackgroundColor((WMWidget *)Button, color);
   WMSetButtonText (Button, "Files");
   WMSetButtonAction (Button, selectFiles, NULL);    
   WMMapWidget(Button);
   ButtonsetSize = WMGetViewSize(WMWidgetView(Button));       

  WMAddBoxSubview(box, WMWidgetView(Button), True, False, 60, 1000, MARGIN);
 
   Button =WMCreateButton(box,WBTMomentaryPush);
   WMSetWidgetBackgroundColor((WMWidget *)Button, color);
   WMSetButtonText (Button, "Quit");
   WMSetButtonAction (Button, closeAll, NULL);
   WMMapWidget(Button);

  WMAddBoxSubview(box, WMWidgetView(Button), True,False, 60, 1000, 0); 
  WMResizeWidget(box, 4*MARGIN+2*ButtonsetSize.width,2*MARGIN+ButtonsetSize.height);
  ButtonsetSize =WMGetViewSize(WMWidgetView(box));
  resizeHandler(NULL,NULL);
  /* end of box and buttons setup */ 

  WMMapWidget(win);

  WMMapSubwidgets(win);
  WMRealizeWidget(win);

  WMScreenMainLoop(screen);

return 0;
}
