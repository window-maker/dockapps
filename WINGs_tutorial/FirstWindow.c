int main (int argc, char **argv){

 Display *display;
 WMScreen *screen;
 WMWindow *win;
  
   WMInitializeApplication("FirstWindow", &argc, argv);

   display = XOpenDisplay("");
   screen = WMCreateScreen(display, DefaultScreen(display));
   win = WMCreateWindow(screen, "");
 
   WMRealizeWidget(win);
   WMMapWidget(win);

   WMScreenMainLoop(screen);
}
