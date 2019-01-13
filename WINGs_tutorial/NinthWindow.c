#include "editmenu.h"     /* This must be the MODIFIED .h file   */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#define WINWIDTH 300
#define WINHEIGHT 400
#define MENUWIDTH 85
#define MENITEMHT 21
#define LOGPROGRAM "xconsole"
#define ERRMSGFIFO "/tmp/WINGsWindowfifo"
#define FIFONAMELEN 20
#define NOLOGWINDOW (-2)    /*  value when there is no console window   */
#define FIFOERROR (-1)      /* value when there is a problem w/ console   */
#define FIFOLOWESTPOSS 0


int windowCounter=0;
int fifonr;
int sibpid;
char fifofilename[FIFONAMELEN+5];

struct dataStruct{
 WMWindow *window;    
 WEditMenu *menu;
} dataStruct;


          /*    functions for the message window part:      */

void redirectmsg(int sig){

  // clean up after SIGCHLD, and set fifonr to flag it
  fifonr=NOLOGWINDOW;
  if (!access(fifofilename,F_OK|W_OK))
    unlink(fifofilename); 
  return;
}


int showMessageWindow(){

 sprintf(fifofilename,"%s%i",ERRMSGFIFO,(unsigned short)getpid());

 (void) signal(SIGCHLD,redirectmsg); // clean up if message console is killed

 if(access(fifofilename,F_OK)==-1)
   fifonr=mknod(fifofilename,0640|O_EXCL|S_IFIFO,(dev_t)0);
  else {fifonr=FIFOERROR;
   wwarning("Fifo file already exists\n");
 }
 /* fifonr == FIFOERROR if mknod/mkfifo or access failed, mknod returns -1 on failure  */

if(fifonr!=FIFOERROR){

 sibpid=fork();
 if(sibpid==0){
   execlp(LOGPROGRAM , LOGPROGRAM, "-file",fifofilename,"-geometry","250x400", "-title","Window Messages",(char *)0);
   exit(1);
}else
  fifonr=open(fifofilename,O_WRONLY);
}
  return fifonr;
}

       /*     general and menu handling functions            */

void closeAll(WMWidget *self,void *data){

  WMDestroyWidget(self);
  if(--windowCounter<1){
    if (fifonr>=FIFOLOWESTPOSS) 
      kill(sibpid,SIGTERM);
    if (!access(fifofilename,F_OK|W_OK))
     unlink(fifofilename);
    exit(0);
  }
}


void menuItemAction(void *self, void *data){

  if (fifonr<FIFOLOWESTPOSS)fifonr=showMessageWindow();   // try again in case FIFOERROR
  if (fifonr==FIFOERROR)   // give up and print to stderr
    fprintf(stderr,"%i: %s selected\n", getpid(), WGetEditMenuItemTitle(self));
  else{
  char textbuffer[100];
    snprintf(textbuffer,100, "%i: %s selected\n", getpid(), WGetEditMenuItemTitle(self));
    write(fifonr, textbuffer,strlen(textbuffer));
  }
}


void menuItemCloseAction(void *self, void *data){

  WMPostNotificationName("WMWindowClose", self, NULL);
}


void getMenu(WMWidget *self, void *data){

WMPoint position;
struct dataStruct *tmp=(struct dataStruct *)data;

 if(WMGetButtonSelected(self)){
   position=WMGetViewScreenPosition(WMWidgetView(tmp->window)); 
   WEditMenuShowAt(tmp->menu,(position.x>MENUWIDTH)?position.x-MENUWIDTH:0, position.y+MENITEMHT,tmp->window);
 }else{
   WEditMenuHide(tmp->menu);
   WDeselectItem(tmp->menu);             // remove selection before next pop up
  }
}


static void notificationHandler(void *self, WMNotification *notif){  

if(!strcmp("WMWindowClose",WMGetNotificationName(notif)))
  closeAll(self,NULL);	
if(!strcmp(WMViewSizeDidChangeNotification,WMGetNotificationName(notif))){
  //resize actions
  WMSize size = WMGetViewSize(WMWidgetView(self)); 
 }
}

         /*       main widget creating functions       */

WMWindow * makeMainwindow(Display *display, WMScreen *screen){
WMWindow *window;

 window = WMCreateWindow(screen, "Menu");
 WMResizeWidget(window, WINWIDTH, WINHEIGHT);
 WMSetWindowCloseAction(window, closeAll, NULL);
 WMAddNotificationObserver(notificationHandler, window, "WMWindowClose", WMWidgetView(window));
 WMSetViewNotifySizeChanges(WMWidgetView(window), True);
 WMAddNotificationObserver(notificationHandler, window, WMViewSizeDidChangeNotification, WMWidgetView(window));
 WMAddNotificationObserver(notificationHandler, window, "WMWindowClose", NULL);
 WMRealizeWidget(window); 
 return window;
}


WEditMenu * makeMenus(WMScreen *screen,WEditMenu *menu, WEditMenu *submenu){
WEditMenuItem * menuitem;

 submenu=WCreateEditMenu(screen,"Submenu");
 menuitem =WAddMenuItemWithTitle(submenu,"Submenu item"); 
 WSetEditMenuItemAction( menuitem, menuItemAction);
 menuitem =WAddMenuItemWithTitle(submenu,"2nd submenu item"); 
 WSetEditMenuItemAction( menuitem, menuItemAction);
 menuitem =WAddMenuItemWithTitle(submenu,"3d submenu item"); 
 WSetEditMenuItemAction( menuitem, menuItemAction);
 menu=WCreateEditMenu(screen,"Main menu");
 menuitem = WAddMenuItemWithTitle(menu,"1st main item"); 
 WSetEditMenuItemAction( menuitem, menuItemAction);
 menuitem = WAddMenuItemWithTitle(menu,"2nd main item"); 
 WSetEditMenuItemAction( menuitem, menuItemAction);
 menuitem = WAddMenuItemWithTitle(menu,"To submenu"); 
 WSetEditMenuSubmenu(menu, menuitem , submenu);
 menuitem = WAddMenuItemWithTitle(menu,"Quit"); 
 WSetEditMenuItemAction( menuitem, menuItemCloseAction);
 WMRealizeWidget(submenu);WMRealizeWidget(menu);
 return menu;
}


WMButton * makeButtonsTop( WMWidget *window, void *AppData){
WMButton *Button;

 Button =WMCreateButton(window,WBTPushOnPushOff);
 WMSetButtonText (Button, "Menu");
 WMSetButtonAction (Button, getMenu, AppData);    
 WMMoveWidget(Button, 4,2);
 WMRealizeWidget(Button);
 return Button;
}


int main (int argc, char **argv){

Display *display;
WMScreen *screen;
WMWindow *mainwindow;
WEditMenu *submenu, *menu;
WEditMenuItem * menuitem;
struct dataStruct Mainmenu;
WMButton *menubutton;

fifonr=NOLOGWINDOW;

WMInitializeApplication("MenuWindow", &argc, argv);
display = XOpenDisplay("");
screen = WMCreateScreen(display, DefaultScreen(display));
mainwindow= makeMainwindow(display, screen) ;

menu=makeMenus(screen,menu,submenu);

Mainmenu.window=mainwindow;
Mainmenu.menu=menu;
menubutton=makeButtonsTop(mainwindow, &Mainmenu);

WMMapSubwidgets(mainwindow);
WMMapWidget(mainwindow);

WMScreenMainLoop(screen);
return 0;
}
