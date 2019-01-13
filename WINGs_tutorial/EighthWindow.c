#include "editmenu.h"

#define WINWIDTH 300
#define WINHEIGHT 400
#define MENUWIDTH 80
#define MENITEMHT 21

struct datacouple{WMWindow *window;
  WEditMenu *menu;
} datacouple;

void closeAll(WMWidget *self,void *data){
  WMDestroyWidget(self);
  exit(0);
}

void getMenu(WMWidget *self, void *data){
  WMPoint position;
  struct datacouple *tmp=(struct datacouple *)data;
  if(WMGetButtonSelected(self)){
  position=WMGetViewScreenPosition(WMWidgetView(tmp->window)); 
  WEditMenuShowAt(tmp->menu,(position.x>MENUWIDTH)?position.x-MENUWIDTH:0, position.y+MENITEMHT,tmp->window);
  }else
    WEditMenuHide(tmp->menu);
}

int main (int argc, char **argv){

Display *display;
WMScreen *screen;
WMWindow *win;
WEditMenu *submenu, *menu;
WEditMenuItem * menuitem;
struct datacouple Mainmenu;
 WMButton *Button;

WMInitializeApplication("MenuWindow", &argc, argv);
display = XOpenDisplay("");
screen = WMCreateScreen(display, DefaultScreen(display));
win = WMCreateWindow(screen, "Menu");
WMResizeWidget(win, WINWIDTH, WINHEIGHT);
WMSetWindowCloseAction(win, closeAll, NULL);

 submenu=WCreateEditMenu(screen,"Submenu");
 menuitem =WAddMenuItemWithTitle(submenu,"Submenu item"); 
 menu=WCreateEditMenu(screen,"Main menu");
 menuitem = WAddMenuItemWithTitle(menu,"To submenu"); 
 WSetEditMenuSubmenu(menu, menuitem , submenu);
 menuitem = WAddMenuItemWithTitle(menu,"Main item"); 

Mainmenu.window=win;
Mainmenu.menu=menu;
 
 Button =WMCreateButton(win,WBTPushOnPushOff);
 WMSetButtonText (Button, "Menu");
 WMSetButtonAction (Button, getMenu, &Mainmenu);    
 WMMoveWidget(Button, 1,1);

WMRealizeWidget(win); 
WMRealizeWidget(Button);
WMRealizeWidget(menu);
WMRealizeWidget(submenu);

WMMapSubwidgets(win);
WMMapWidget(win);

WMScreenMainLoop(screen);

return 0;
}
