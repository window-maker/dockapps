
/*     
 *      wmAppKill v0.2 - S.Rozange
 * 
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation; either version 2, or (at your option)
 * 	any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program (see the file COPYING); if not, write to the
 * 	Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 *      Boston, MA  02111-1307, USA
 *
 */


#include <unistd.h>
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/xpm.h> 
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <glibtop.h>
#include <glibtop/proclist.h>
#include <glibtop/procstate.h>
#include <glibtop/xmalloc.h>

#include <time.h>
#include <signal.h>

#include "fond.xbm"  
#include "wmAppKill.xpm" 
#include "wmAppKill.h"

Display *dpy;  			       /* xlib global vars */
GC gc;
Window win;
Window iconWin;
Pixmap XPM;     
int screen;

_zone *fZone = NULL;
_desc *pList = NULL;		       /* double linked list */
_desc *posProc = NULL; 		       /* lower proc showed on the dock */
pid_t tabNoProc[NB_LINE];    	       /* array containing each line's pid */
pid_t lastProcPid;		       /* oldest proc before refreshment */
char *procBaseName = PROC_DEF;         /* procBase is NULL if we want all the proc to be listed */
int procBasePos = 0;		       /* procbase's pos in linked list */
pid_t procBasePid = 1;                 /* procbase's pid */
unsigned int gNbProc = 0;	       /* proc number starting from procBase */
unsigned int gNbProcTotal = 0;	       /* total proc number */
struct timeval timev;		       /* to detect double-click */

void ZoneCreate(int x, int y, int width, int height, char no)
{
   _zone *last;
     
   if (!fZone) {
      fZone = (_zone *)malloc(sizeof(_zone));
      last = fZone;
   } else {
      for (last = fZone; last -> next; last = last -> next);
      last -> next = (_zone *)malloc(sizeof(_zone));
      last = last -> next;
   }

   last -> x = x;
   last -> y = y;
   last -> width = width;
   last -> height = height;
   last -> no = no;
   last -> next = NULL;
}

char CheckZone(void)
{
   int x, y,popo;
   unsigned int mask;
   Window root_ret, child_ret;
   _zone *curseur = fZone;
   
   XQueryPointer(dpy, iconWin, &root_ret, &child_ret, &popo, &popo, &x, &y, &mask); /* mouse position */
   
   do {
      if ((x >= curseur -> x) && (x <= curseur -> x + curseur -> width) &&  (y >= curseur -> y) && (y <= curseur -> y + curseur -> height))
	return curseur -> no;

   }
   while ((curseur = curseur -> next));
   
   return 0;
}
   
void GarbageCollector(_desc *garb)
{
   _desc *next;
   
   while (garb)  {
      next = garb -> next;
      free(garb);
      garb = next;
   }
}

int CheckProc(pid_t pid)
{
   glibtop_proclist bof;
   unsigned int *n;
   
   if ((n = glibtop_get_proclist (&bof, GLIBTOP_KERN_PROC_PID , (int64_t)pid)) == NULL) {
      glibtop_free(n);
      return -1;
   }
   
   glibtop_free(n);
   return 0;
}

_desc *GetProcList(void) 		       /* create a double linked list */
{
   glibtop_proclist buf;
   unsigned int *n;
   unsigned int nbPr;
   int i;
   _desc *lastOne;
   _desc *glump;
   _desc *res;
   
   if ((n = glibtop_get_proclist (&buf, GLIBTOP_KERN_PROC_UID, (int64_t)getuid())) == NULL) {
      fprintf(stderr, "Problem using libgtop\n");
      exit(1);
   };
   
   nbPr = (int)buf.number;
    
   glump = (_desc *)malloc(sizeof(_desc));
   res = glump;
   lastOne = NULL;
   
   for (i = nbPr; i; i--){
      char *bof;
      glibtop_proc_state buf;
      glump -> previous = lastOne;
      glump -> next = (_desc *)malloc(sizeof(_desc));
      
      glibtop_get_proc_state(&buf, glump -> pid = n[i - 1]);
      strcpy(glump -> name, bof = buf.cmd);
      if (strlen(glump -> name) > MAX_CHAR) 
	glump -> name[MAX_CHAR] = 0;
      
      lastOne = glump;
      glump = glump -> next;
      
      if (procBaseName && !strcmp(bof, procBaseName)) {
	 procBasePos = i - 1;
	 procBasePid = n[i - 1];
	 break; 
      }
   }
   
   lastOne -> next = NULL;
   lastProcPid = n[nbPr - 1]; 
   glibtop_free(n);
   
   if (procBaseName && i) gNbProc = nbPr - i + 1;   /* procBase has been found */
   else {			       /* procBaseName is null or hasn't been found */
      procBaseName = NULL;
      procBasePos = 0;
      procBasePid = n[0];
      gNbProc = nbPr;
   }
   
   gNbProcTotal = nbPr;
  
   return res;
}

int CheckProcToRemove(unsigned int *procList, unsigned int procListSize)
{
   _desc *curseur = pList, *temp;
   int nbProcRemoved = 0, i;
   
   while (curseur) {
      for (i = procListSize; i; i--)
	if (curseur -> pid == procList[i - 1]) break;
            
      temp = curseur;
      curseur = curseur -> next;
      
	   if (!i) {  /* we didn't find it in proclist, let's remove it */
	      RemoveProc(temp); 
	      gNbProc--;
	      nbProcRemoved++;
	   }
	}
   
   return nbProcRemoved;
}

int CheckProcToAdd(int pos, unsigned int *procList, unsigned int procListSize)
{
   _desc *glump;
   int i, compteur = 0;
   glibtop_proc_state buf;
   
   for (i = pos; i < procListSize ; i++){
      
      compteur++;
      glump = (_desc *)malloc(sizeof(_desc));
      usleep(20000); /* libgtop seems to need a little bit of time */
      if (CheckProc(procList[i])) continue; /* checking if the process isn't already dead */
            
      glibtop_get_proc_state(&buf, glump -> pid = procList[i]);
      strcpy(glump -> name, buf.cmd);
      if (strlen(glump -> name) > MAX_CHAR) 
	glump -> name[MAX_CHAR] = 0;
              
      pList -> previous = glump;
      glump -> next = pList;
      glump -> previous = NULL;
      if (posProc == pList) posProc = glump;
      pList = glump;
      gNbProc++;     
      gNbProcTotal++;
     
      lastProcPid = glump -> pid;
   }

   return compteur;
}
   
int CheckProcChange(void)
{
   glibtop_proclist buf;
   unsigned int *n;
   unsigned int nbPr;
   int diffNbProc;
   
   if ((n = glibtop_get_proclist (&buf, GLIBTOP_KERN_PROC_UID, (int64_t)getuid())) == NULL) return -1;
   nbPr = (int)buf.number;
   
   if ((nbPr == gNbProcTotal) && (n[nbPr - 1] == lastProcPid)) return 0; /* nothing changed */
   
   if (procBaseName && (n[procBasePos] != procBasePid))       /* some proc killed before the baseproc (=oldest proc) */
     {  
	if (CheckProc(procBasePid)) {   /*  baseproc doesn't exist anymore */
	   GarbageCollector(pList);
	   pList = GetProcList(); 	       /* so we create a whole new list */
	   posProc = pList;
	   return 1;
	}
	else 
	  while (n[--procBasePos] != procBasePid); /* here we find what's the new pos. of baseproc */
     }
   
     
   diffNbProc = (nbPr - procBasePos) - gNbProc; /* nb of changes after baseproc */
   
   if (diffNbProc == 0 && (n[nbPr - 1] == lastProcPid)){  /* only changes before baseproc */
      gNbProcTotal = nbPr;
      glibtop_free(n);
      return 0;
   }
   
   if (diffNbProc > 0 && n[nbPr - diffNbProc - 1] == lastProcPid)  /* only proc to add */
     CheckProcToAdd(nbPr - diffNbProc, n, nbPr);
     
   else {  /* to remove [and to add] */
      int nb;
      nb = CheckProcToRemove(n, nbPr);
      if (nb != -diffNbProc) 
	CheckProcToAdd(nbPr - diffNbProc - 1, n, nbPr);
   }

   glibtop_free(n);
   return 1;
}

void RemoveProc(_desc *cible)
{
   _desc *temp1, *temp2;
   
   _desc *curseur;
   int i;
   
   temp1 = cible -> previous;
   temp2 = cible -> next;
      
   for (curseur = cible, i = 0; curseur && i != NB_LINE + 1; curseur = curseur -> next, ++i);  
   
   if (!(gNbProc - 1 < NB_LINE) && (i == 2 || i == 3)) { 	       /* the killed proc is near the start of the list */
      for (--i, curseur = posProc; i && curseur -> previous; curseur = curseur -> previous, --i);
      posProc = curseur;
   }
   else if ((cible == posProc) && (cible -> previous)) {
      posProc = cible -> previous;
   }
   else if ((cible == posProc) && (cible -> next)) {  
      posProc = cible -> next;
   }
   
   if (temp1) temp1 -> next = temp2; 
   else {
      pList = temp2;
      temp2 -> previous = NULL;
      gNbProcTotal--;
      lastProcPid = temp2 -> pid;
      free(cible);
      return;
   }
   
   if (temp2) temp2 -> previous = temp1;
   else 
      temp1 -> next = NULL;
      
   free(cible);
   gNbProcTotal--;
   
}

void ShowString (int x, int y, char *doudou)
{
   int i = 0;
   char c;

   while ((c = tolower(doudou[i++]))){
      if (c >= 'a' && c <= 'z') {   
	 XCopyArea(dpy,XPM, win,gc, 1 + (c - 'a') * 6, 10, 5, 8, x, y) ;
	 XCopyArea(dpy,XPM, iconWin,gc, 1 + (c - 'a') * 6, 10, 5, 8, x, y) ;
	 x += 6;
      }
   }
}

void DoExp(){
   
   XClearWindow(dpy, win);
   XClearWindow(dpy, iconWin);
   
   XCopyArea(dpy, XPM ,win, gc, 1 + (26) * 6, 10, 6, 8, 5, 51);
   XCopyArea(dpy, XPM, iconWin, gc, 1 + (26) * 6, 10, 6, 8, 5, 51);
   XCopyArea(dpy, XPM, win, gc, 1 + (27) * 6, 10, 6, 8, 53, 51);
   XCopyArea(dpy, XPM, iconWin, gc, 1 + (27) * 6, 10, 6, 8, 53, 51);

   XCopyArea(dpy, XPM, win, gc, 106, 0, 28, 8, 17, 50);
   XCopyArea(dpy, XPM, iconWin, gc, 106, 0, 28, 8, 17, 50);
   
   DoExpose();
}

void DoExpose() {
  
   int i;
   _desc *curseur;

   for (i = NB_LINE; i; i--){
            XCopyArea(dpy, XPM, win, gc, 2, 22, 53, 9, X_PROC + 1, Y_PROC + ((i - 1) * 10));
            XCopyArea(dpy, XPM, iconWin, gc, 2, 22, 53, 9, X_PROC + 1, Y_PROC + ((i - 1) * 10));
   }

   if (gNbProc < NB_LINE) {
      int y;
      i = gNbProc;
      for (y = NB_LINE; y != gNbProc; y--) tabNoProc[y - 1] = -1;
   }
   else i = NB_LINE;
      
   for (curseur = posProc; i; curseur = curseur -> next, i--)
     {
	ShowString(X_PROC + 1, Y_PROC + (i - 1) * 10, curseur -> name);
	tabNoProc[i - 1] = curseur -> pid;
     }
}

void DoClick(XEvent ev)
{
   unsigned char doubleClick = 0;
   static unsigned char firstClick = 0;
   _desc *curseur;
   char zone, i;
   
   zone = CheckZone();
      
   if (ev.xbutton.button == CLICK_TWO) { 
      DoExpose() ;
   }

   /* Mouse wheel patch by Mathieu Cuny */
   
   if (ev.xbutton.button == WHEEL_UP && gNbProc > NB_LINE) {
      for (i = NB_LINE, curseur = posProc; i; curseur = curseur -> next, i--);
      if (curseur) posProc = posProc -> next;
      DoExpose();
   }
   
   if (ev.xbutton.button == WHEEL_DOWN && posProc -> previous && gNbProc > NB_LINE) {
      	   posProc = posProc -> previous;
	   DoExpose();
   }
   
   /* Mouse wheel patch end */
   
   if (ev.xbutton.button == CLICK_ONE) {
      
      struct timeval temp;
      long long nms1;
                 
      gettimeofday(&temp, NULL);
      nms1 = temp.tv_sec - timev.tv_sec; /* nb sec since last click */
      
      if  ((!nms1 || nms1 == 1)){
	 long long yop = (nms1 * 1000000L) + (temp.tv_usec - timev.tv_usec); /* nb mlsec since last click */
	 if (firstClick && (yop < DOUBLE_CLICK_DELAY)){  /* we got double click */
	    doubleClick = 1;
	    firstClick = 0;
	 } else firstClick = 1;
      } else firstClick = 1;
      
       timev = temp;
           
      if (zone == UP && !doubleClick && gNbProc > NB_LINE)
	{
	   for (i = NB_LINE, curseur = posProc; i; curseur = curseur -> next, i--);
	   if (curseur) posProc = posProc -> next;
	   DoExpose();
	}
      
      else if (zone == DOWN && posProc -> previous && !doubleClick && gNbProc > NB_LINE)
	{
	   posProc = posProc -> previous;
	   DoExpose();
	}
      
      else if (zone == UP && doubleClick && gNbProc > NB_LINE)   
	{

	   for (curseur = pList; curseur -> next; curseur = curseur -> next);    /* curseur = end of list */
	   
	   for (i = NB_LINE - 1; i; curseur = curseur -> previous, i--);
	   posProc = curseur;
	   DoExpose();
	}
      
      else if (zone == DOWN && doubleClick && gNbProc > NB_LINE) 
	{
	   posProc = pList;
	   DoExpose();
	}
      
      else if (zone > 0 && zone <= NB_LINE && doubleClick && tabNoProc[zone - 1] != -1)
	{
	   kill(tabNoProc[zone - 1], SIGKILL); /* let's kill the mofo */
	   waitpid(tabNoProc[zone - 1], NULL, 0);
	}
      
      if (doubleClick) doubleClick = 0;
      
   }
}

void DoEvents() {

   unsigned long long compteur = 0;
   
   XEvent ev ; 

   for (;;){
      if (!compteur){
	 if (CheckProcChange()) DoExpose();
	 compteur = UPDATE_NB * DELAY;
      }
      while(XPending(dpy)){   
	 XNextEvent(dpy,&ev); 
	 switch(ev.type) {
	  case Expose : DoExp(); break;
	  case ButtonPress : DoClick(ev); break;
	 }
      }
      usleep(DELAY);
      compteur -= DELAY;
   }
}

void PrintUsage(void)
{
   printf("usage: wmAppKill [-a] [-n <name>] [-h]\n");
   printf("\t-a\t\tSelect all processes\n");
   printf("\t-n <name>\tDo not select processes older than <name> (default: wmaker)\n");
   printf("\t-h\t\tDisplay help screen\n");
}

void GetArg(int argc, char *argv[])
{
   if (argc == 1) return;
   
   else if (argc == 3 && !strcmp(argv[1], "-n") && argv[2][0] != '-')
     procBaseName = strdup(argv[2]);
   
   else if (argc == 2 && !strcmp(argv[1], "-a"))
     procBaseName = NULL;
   
   else if (argc == 2 && !strcmp(argv[1], "-h"))
     {
	PrintUsage();
	exit(1);
     }
   else {
      PrintUsage();
      exit(1);
   }
}
      
void CreateDock(int argc, char *argv[])    /* this part comes from http://www.linuxmag-france.org/ */
{
   Window root;
   XWMHints wmHints; 
   XSizeHints sizeHints;
   XClassHint classHint;
   Pixmap pixmask;
   unsigned long p_blanc;
   unsigned long p_noir;
   unsigned int borderWidth = 2;
   char *wname = argv[0] ;
   
   dpy = XOpenDisplay(NULL) ;
   
   if(dpy == NULL)
	  {
	     fprintf(stderr, "Can't open display\n") ;
	     exit(1) ;
	  }
   
   root = RootWindow(dpy,screen);
   p_blanc = WhitePixel(dpy,screen) ;
   p_noir = BlackPixel(dpy,screen) ;
   gc = XDefaultGC(dpy,screen) ;
   XSetForeground(dpy, gc, p_noir);
   XSetBackground(dpy, gc,p_noir);
   
   sizeHints.x = 0 ;
   sizeHints.y = 0 ;
   sizeHints.width = 64 ;
   sizeHints.height = 64 ;
   
   win = XCreateSimpleWindow(dpy,root,sizeHints.x,sizeHints.y , sizeHints.width, sizeHints.height, borderWidth, p_noir,p_noir) ;
   iconWin = XCreateSimpleWindow(dpy,root,sizeHints.x,sizeHints.y,sizeHints.width, sizeHints.height, borderWidth, p_noir,p_noir ) ;
   
   sizeHints.flags = USSize | USPosition ;
   XSetWMNormalHints(dpy,win,&sizeHints) ;
   wmHints.initial_state = WithdrawnState ;
   wmHints.icon_window = iconWin ;         
   wmHints.icon_x = sizeHints.x ;
   wmHints.icon_y = sizeHints.y ;
   wmHints.window_group = win ;
   wmHints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint ;
   XSetWMHints(dpy, win, &wmHints) ;
   classHint.res_name = wname ;
   classHint.res_class = wname ;
	
   XSetClassHint(dpy, win, &classHint) ;
   XSetCommand(dpy,win, argv, argc) ;
	
   pixmask = XCreateBitmapFromData(dpy,win,fond_bits, fond_width, fond_height) ;
   XShapeCombineMask(dpy,win,ShapeBounding,0,0,pixmask,ShapeSet) ;
   XShapeCombineMask(dpy,iconWin,ShapeBounding, 0, 0, pixmask, ShapeSet) ;
   XpmCreatePixmapFromData(dpy,root,wmAppKill_xpm, &XPM, NULL,NULL) ;
   
   XSelectInput(dpy,win, ExposureMask | ButtonPressMask) ;
   XSelectInput(dpy,iconWin, ExposureMask | ButtonPressMask) ;
	
   XMapWindow(dpy,win) ;
}

int main(int argc, char *argv[])
{
   int i;
   
   GetArg(argc, argv);
   glibtop_init();
   CreateDock(argc, argv);
   gettimeofday(&timev, NULL); 
   
   pList = GetProcList(); 
   posProc = pList;
   
   ZoneCreate(4, 50, 8, 8, UP);
   ZoneCreate(54, 50, 8, 8, DOWN);
   
   for (i = NB_LINE; i; i--)
     ZoneCreate(X_PROC + 1, Y_PROC + (i - 1) * 10 + 2, 48, 7, i);
	
   DoEvents();
	
   return 0;
}
	 
