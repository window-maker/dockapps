void closeAll(WMWidget *self,void *data){
  fprintf(stderr, "I've been used!\n");
  WMDestroyWidget(self);
  exit(0);
}

static void
handleEvents(XEvent *event, void *data)
{
    WMWidget *widget = (WMWidget*)data;
    switch (event->type) {
    case ButtonPress:
        closeAll(widget,NULL);
        break;
    }
}

int main (int argc, char **argv){

 Display *display;
 WMScreen *screen;

 WMWindow *win;
 WMColor *color;

 WMInitializeApplication("ThirdWindow", &argc, argv);

 if (!(display = XOpenDisplay(""))){
   fprintf(stderr,"error: cannot open display\n");
   exit(1);
 }
 screen = WMCreateScreen(display, DefaultScreen(display));

  win = WMCreateWindow(screen, "");
  WMSetWindowCloseAction(win, closeAll, NULL);
  WMCreateEventHandler(WMWidgetView(win), ButtonPressMask,handleEvents, win);
  color = WMCreateRGBColor(screen, 124<<9,206<<8,162<<8, False);
  WMSetWidgetBackgroundColor((WMWidget *)win, color);

  WMMapWidget(win);
  WMRealizeWidget(win);

  WMScreenMainLoop(screen);

return 0;
}
