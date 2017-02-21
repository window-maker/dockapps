/* wmswallow.c */

/* #define DEBUG 1 */
          /* Sometimes i want to get quick access to this flag :-)*/

/* Time-stamp: <00/05/15 23:13:43 friedel> */

/* Copyright 2000 Friedrich Delgado Friedrichs */

/* Swallow applications in the Windowmaker dock */
/* Originally i started with asbeats 0.2 (by iznogood@bohemians.org) (simply
   the smallest WindowMaker dockapp i could find, since there was no proper
   Documentation to find on how to make an app dockable), which i stripped
   down to have a basic dockable app, and then stole code from fvwm2 to
   swallow an Application */
/* Man, this was easy! Why did nobody implement this before? (That's me,
   looking  astonished at the first working version at Mon Apr 17 02:16:31
   CEST 2000 after only 4 hours of mostly learning how to program the X-Environment and
   hacking and guessing a little :-) */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <time.h>
#include <X11/Xatom.h>
#include <string.h>

#include "version.h"

/* That's all we need from xpm.h */
#if ! defined(_XtIntrinsic_h) && ! defined(PIXEL_ALREADY_TYPEDEFED)
typedef unsigned long Pixel;    /* Index into colormap */
# define PIXEL_ALREADY_TYPEDEFED
#endif
/* Now we got rid of that stupid libXpm dependency :-) */

#define WIDTH 55
#define HEIGHT 57
/* 55x57 seems to be the default size for a WindowMaker dockapp */
/* settable by "-geometry" switch */

Display *dpy;
Window Root;
Window iconwin,win;
Window swallowed;
XSizeHints mysizehints;

#define MW_EVENTS	(ExposureMask | ButtonPressMask |\
                         StructureNotifyMask |\
                         ButtonReleaseMask |\
                         EnterWindowMask|LeaveWindowMask)
#define SW_EVENTS       (PropertyChangeMask | StructureNotifyMask |\
			 ResizeRedirectMask | SubstructureNotifyMask)
#define FIND_EVENTS     (SubstructureNotifyMask | StructureNotifyMask)
#define READY_EVENTS     FIND_EVENTS
#define FALSE 0
#define TRUE (!FALSE)

Pixel GetColor(char *name);
void FlushWindow();
void usage (char *progname);
int parseargs(int argc, char *argv[]);
Window findnamedwindow (char *class);
Window findnamedwindowacc (char *class, Window window);
int checkwindow (Window window, char *class);
int execstuff(int argc, char *oldargv[]);
Window startandfind(int argc, char *oldargv[], char* class);
int printlist(FILE * stream, char * string, char **stringlist);
int flush_expose (Window w);
void stealshape (Window w);
int sendexpose (Window w);
void waitformap (Window win);
/* int softenwindow (Window w); */ /* won't work, kept for historical reasons */

/* Parameters that can be customized via commandline switches */
char *execstring=NULL;
char *geometry=NULL;
int getclick=FALSE;
int shape=TRUE;
int focus=FALSE;
int unmanaged=FALSE;
int winid=0;
char *display_name=NULL;

int main(int argc,char *argv[])
{
  int screen;
  int d_depth;
  XWMHints mywmhints;
  Pixel back_pix,fore_pix;

  int i;
  unsigned int borderwidth;
  char *wname="wmswallow";

  int remainarg, remainargc;

  XEvent Event;
  XTextProperty name;
  XClassHint classHint;

  remainarg=parseargs(argc, argv); /* remainarg > 0 afterwards */
  remainargc=argc-remainarg;
#ifdef DEBUG
  fprintf(stderr, "remainarg: %d, remainargc: %d, argc: %d\n", remainarg,
	  remainargc,argc);
  fflush(stderr);
#endif

  if (!(dpy = XOpenDisplay(display_name))) {
    fprintf(stderr,"wmswallow: can't open display %s\n",
	    XDisplayName(display_name));
    exit (1);
  }
  screen=DefaultScreen(dpy);
  Root=RootWindow(dpy, screen);

  /* So, now we've got everything we need to get Events from the XServer */
  if (remainargc>1) {
    winid=startandfind(remainargc-1, argv+remainarg+1, argv[remainarg]);
    if (winid==0) {
      perror("wmswallow: startandfind failed");
      /* Real error handling in execstuff()*/
      exit (1);
    }
  }

  d_depth=DefaultDepth(dpy, screen);
  /*   XConnectionNumber(dpy); */ /* useless */
  mysizehints.flags=USSize|USPosition;
  mysizehints.x=0;
  mysizehints.y=0;
  back_pix=GetColor("white");
  fore_pix=GetColor("black");
  XWMGeometry(dpy, screen, geometry, NULL, (borderwidth =1),
	      &mysizehints, &mysizehints.x, &mysizehints.y,
	      &mysizehints.width, &mysizehints.height, &i);
  mysizehints.width=WIDTH;
  mysizehints.height=HEIGHT;
  if (geometry!=NULL) {
#ifdef DEBUG
    fprintf(stderr,"Setting geometry to: %s\n",geometry);
    fflush(stderr);
#endif
    XParseGeometry(geometry, &mysizehints.x, &mysizehints.y,
		   &mysizehints.width, &mysizehints.height);
  }

  win=XCreateSimpleWindow(dpy, Root, mysizehints.x, mysizehints.y,
			  mysizehints.width, mysizehints.height, borderwidth,
			  fore_pix, back_pix);
  iconwin=XCreateSimpleWindow(dpy, win, mysizehints.x, mysizehints.y,
			      mysizehints.width, mysizehints.height, borderwidth,
			      fore_pix, back_pix);
  XSetWMNormalHints(dpy, win, &mysizehints);
  classHint.res_name="wmswallow";
  classHint.res_class="WMswallow";
  XSetClassHint(dpy, win, &classHint);
  XSelectInput(dpy, win, MW_EVENTS);
  XSelectInput(dpy, iconwin, MW_EVENTS);
  if(XStringListToTextProperty(&wname, 1, &name)==0)
    {
      fprintf(stderr, "wmswallow: can't allocate window name\n");
      exit(-1);
    }
  XSetWMName(dpy, win, &name);
  mywmhints.initial_state = WithdrawnState;
  mywmhints.icon_window = iconwin;
  mywmhints.icon_x = mysizehints.x;
  mywmhints.icon_y = mysizehints.y;
  mywmhints.window_group = win;
  mywmhints.flags = StateHint | IconWindowHint |
    IconPositionHint | WindowGroupHint;
  XSetWMHints(dpy, win, &mywmhints);
  XSetCommand(dpy, win, argv, argc);

  if (winid==0) {
    swallowed=findnamedwindow(argv[remainarg]); /* Find which window to
						   swallow*/
#ifdef DEBUG
    fprintf(stderr,"%s has Window-id 0x%lx\n", argv[remainarg], swallowed);
    fflush(stderr);
#endif
  }
  else
    swallowed=winid;


  /* "Swallow" it */
  XReparentWindow(dpy, swallowed, iconwin, 0, 0);
  if (getclick) {
    /* softenwindow (swallowed); */ /* Change some attributes */
    XSelectInput(dpy, swallowed, SW_EVENTS|ButtonPressMask);
  }
  else {
    XSelectInput(dpy, swallowed, SW_EVENTS); /* Workaround for apps like
						perfmeter that don't let us
						get their mouseclicks :-( */
  }
  XSetWindowBorderWidth(dpy, swallowed,0);
  XMoveResizeWindow(dpy, swallowed, 0, 0,
		    mysizehints.width, mysizehints.height);

  /* Now we do some special juju for shaped windows: */

  /* ...tell the window to repaint itself, please! */
  if (shape) {
    sendexpose(swallowed);

    /* ... ok, window should be repainted and a shaped window should have updated
       its mask accordingly! (-: End of shape-juju :-) */

    /* Now steal the shape of the Window we just swallowed! */
    stealshape(swallowed); }
  XMapWindow(dpy,win);
  XMapSubwindows(dpy,win);
  FlushWindow();

  while(1)
    {
      while (XPending(dpy))
	{
	  XNextEvent(dpy,&Event);
	  switch(Event.type)
	    {
	    case ButtonPress:
#ifdef DEBUG
	      fprintf (stderr, "wmswallow: Got ButtonPress Event\n");
	      fflush(stderr);
#endif
	      if (getclick)
		system(execstring);
	      break;
	    case Expose:
	      if(Event.xexpose.count == 0 ) {
#ifdef DEBUG
		fprintf (stderr, "wmswallow: Got Expose Event, count==0\n");
		fflush(stderr);
#endif
		if (shape)
		  stealshape(swallowed); /* Oclock changes its shape! That's why
					    we have to steal it *again* */
		FlushWindow();
	        XMapRaised(dpy,swallowed);
		/* the following Produces "focus-flicker" */
		/* XMapSubwindows(dpy,win); */
		/* XMapWindow(dpy,win); */
	      }
	      break;
	    case EnterNotify:
	      if (focus)
		XSetInputFocus(dpy, swallowed, RevertToPointerRoot,
		CurrentTime);
	      break;
	    case LeaveNotify:
	      if (focus)
		XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
		CurrentTime);
	      break;

	    case DestroyNotify:
	      XCloseDisplay(dpy);
	      exit(0);
	    default:
#ifdef DEBUG
	      /* fprintf (stderr, "wmswallow: Got Some Other Event\n");
			      fflush(stderr); */
#endif
	      break;
	    }
	}
      XFlush(dpy);
      usleep(50000L);
    }
  return 1;
}

/* int softenwindow (Window w) { */
/*   XSetWindowAttributes attributes; */

/*   attributes.override_redirect=FALSE; */
/*   attributes.event_mask=SW_EVENTS|MW_EVENTS; */
/*   attributes.do_not_propagate_mask=0; */

/*   XChangeWindowAttributes(dpy, w, */
/*   CWOverrideRedirect|CWEventMask|CWDontPropagate, */
/* 			  &attributes); */
/*   return TRUE; */
/* } */

int sendexpose (Window w) {
  XExposeEvent xexp;
  XEvent Event;
  int retval;

  xexp.type=Expose;
  xexp.serial=0;
  xexp.send_event=TRUE;
  xexp.display=dpy;
  xexp.window=w;
  xexp.x=0;
  xexp.y=0;
  xexp.width=mysizehints.width;
  xexp.height=mysizehints.height;
  xexp.count=0;
  Event.xexpose=xexp;
  retval=XSendEvent(dpy, w, FALSE, ExposureMask, &Event);
  /*   XFlush(dpy); */ /* ... send all queued Events  */
  /*   usleep(5000L); */ /* ... take a tiny doze */
  XSync(dpy, FALSE); /* I doubt if this is really better than Flushing and
			pausing */
  return retval;
}

void stealshape(Window w) {
  XShapeCombineShape (dpy, iconwin, ShapeBounding, 0, 0, w,
		      ShapeBounding, ShapeSet);
  /* XShapeCombineShape (dpy, win, ShapeBounding, 0, 0, w,  */
  /* 		      ShapeBounding, ShapeSet); */
  /*Re-read specs! */
  /*   XShapeCombineShape (dpy, win, ShapeClip, 0, 0, w,  */
  /* 		      ShapeClip, ShapeSet); */
  /*   XShapeCombineShape (dpy, iconwin, ShapeClip, 0, 0, w, */
  /* 		      ShapeClip, ShapeSet); */
}

void nocolor(char *a, char *b)
{
  fprintf(stderr,"wmswallow: can't %s %s\n", a,b);
}


int flush_expose (Window w)
{
  XEvent dummy;
  int i=0;
  while(XCheckTypedWindowEvent(dpy,w,Expose,&dummy))
    i++;
  return i;
}

void FlushWindow()
{
  flush_expose(swallowed);
  flush_expose (iconwin);
  flush_expose(win);
}

Pixel GetColor(char *name)
{
  XColor color;
  XWindowAttributes attributes;
  XGetWindowAttributes(dpy,Root,&attributes);
  color.pixel=0;
  if (!XParseColor(dpy,attributes.colormap,name,&color))
    nocolor("parse",name);
  else if(!XAllocColor (dpy,attributes.colormap,&color))
    nocolor("alloc",name);
  return color.pixel;
}

void usage(char *progname){
  printf(
	 "wmswallow Version %s\n"
	 "               by Friedrich Delgado Friedrichs (c) 2000\n"
	 "\n"
	 "Usage:\n"
	 "  %s [<flags>] [windowname [command [args ...]]]\n"
	 "  Will swallow the first X-Window it finds with a WM_NAME or\n"
	 "           WM_CLASS matching <windowname>\n"
	 "\n"
	 "        Flags:\n"
	 " -h:                  prints this message and exits\n"
	 " -geometry <string>:  use specified geometry for swallowed\n"
	 "                      window\n"
	 " -display <string>:   connect to specified display\n"
	 " -shape:              use the shape extension (default)\n"
	 " -noshape:            don't use the shape extension\n"
	 " -focus:              Window should take focus\n"
	 " -nofocus:            Window shouldn't take focus(default)\n"
	 " -managed:            Assume window is managed by the\n"
	 "                      windowmanager (default)\n"
	 " -unmanaged:          Assume window is not managed by the\n"
	 "                      windowmanager\n"
	 " -getclick <string>:  on mouseclick, execute <string>\n"
	 "                      instead of passing the Event to the\n"
	 "                      swallowed window.\n"
	 " -id [0x]<hexnumber>: swallow window with id <hexnumber>\n"
	 "  The command with args will be executed, before swallowing.\n",
	 VERSION, progname);
}

/* Parse commandline args, returns first non-switch argnumber */
int parseargs(int argc, char *argv[]){
  int argnum;
  int lastarg=1;
  if (argc<2) {
    usage(argv[0]);
    exit(0);
  }

  for(argnum=1; argnum<argc && *argv[argnum]=='-'; lastarg=++argnum) {
    if (!strncmp(argv[argnum],"-h",2) ||
	!strncmp(argv[argnum],"--",2)) {
      usage(argv[0]);
      exit(0);
    } else if (!strcmp(argv[argnum],"-geometry")||
	       !strcmp(argv[argnum],"-geom"))
      geometry=argv[++argnum];
    else if (!strcmp(argv[argnum],"-display"))
      display_name = argv[++argnum];
    else if (!strcmp(argv[argnum],"-noshape"))
      shape=FALSE;
    else if (!strcmp(argv[argnum],"-shape"))
      shape=TRUE;
    else if (!strcmp(argv[argnum],"-unmanaged"))
      unmanaged=TRUE;
    else if (!strcmp(argv[argnum],"-managed"))
      unmanaged=FALSE;
    else if (!strcmp(argv[argnum],"-nofocus"))
      focus=FALSE;
    else if (!strcmp(argv[argnum],"-focus"))
      focus=TRUE;
    else if (!strcmp(argv[argnum],"-getclick")) {
      execstring=(char *)malloc(strlen(argv[++argnum])+1+2);
      strcpy(execstring, argv[argnum]);
      strcat(execstring, " &");
      getclick=TRUE;
    } else if (!strcmp(argv[argnum],"-id"))
      winid=strtol(argv[++argnum], NULL, 16);
    else {
      usage(argv[0]);
      exit(0);
    }
  }
  return lastarg; /*Return number of first argument, that is neither a switch nor
		      an argument to a switch */
}


/* Print a NULL-terminated list of char* onto file stream */
int printlist(FILE * stream, char * string, char **stringlist) {
  int i=0;

  fprintf(stream, string);
  if (stringlist!=NULL) {
    while (stringlist[i]!=NULL) {
      fprintf(stream, " §");
      fprintf(stream, stringlist[i]);
      fprintf(stream, "§ ");
      ++i;
    }
  } else {
    return(TRUE);
  }
  return(FALSE);
}

/* Select SubstructureNotify on the root-window, start the command, then look
   if we get Create Events for a matching window, set winid */
Window startandfind (int argc, char *oldargv[], char* class) {
  int found=0;
  XEvent Event;
  Window winreturn=(Window)0;
  Window wintmp=(Window)0;

#ifdef DEBUG
  fprintf(stderr, "Checking for name: %s\n", class);
  fflush(stderr);
#endif

  XSelectInput(dpy, Root, FIND_EVENTS);
  if (!execstuff(argc, oldargv))
    return FALSE; /* execstuff failed, should not return, but
		     nevertheless...*/
  while (!found) {
    while (!found && XPending(dpy)) {
      /* FIXME: We hang, when the application starts, but we
	 cannot find the window! */
      XNextEvent(dpy, &Event);
      switch (Event.type) { /* Switch, in case we check for more than one
			       Event-Type one day */
	/* We're waiting for the wm to reparent the window! */
      case ReparentNotify:
#ifdef DEBUG
	fprintf (stderr, "wmswallow: Got ReparentNotify Event\n");
	fflush(stderr);
#endif
	wintmp=Event.xreparent.window;
	if (checkwindow(wintmp, class)) {
	  winreturn=wintmp;
	  found=TRUE;
	} else if ((winreturn=findnamedwindowacc(class, wintmp))) {
	  found=TRUE;
	}
      	break;
      case CreateNotify: case MapNotify:
	wintmp=Event.xcreatewindow.window;
#ifdef DEBUG
	fprintf (stderr, "wmswallow: Got CreateNotify Event for window "
		 "0x%lx\n", wintmp);
	fflush(stderr);
#endif
	if (unmanaged) {
	  if (checkwindow(wintmp, class)) {
	    winreturn=wintmp;
	    found=TRUE;
	  } else if ((winreturn=findnamedwindowacc(class, wintmp))) {
	    found=TRUE;
	  }
	}
	break;
      default:
	break;
      }
    }
  }
  XSelectInput(dpy, Root, None);
#ifdef DEBUG
  fprintf (stderr, "wmswallow: found it"
	   "0x%lx\n", wintmp);
  fflush(stderr);
#endif

  waitformap(winreturn);
  /* Ok, the window has been created, Reparented by WindowMaker and mapped */
  /* What else can we do to make sure the window was created? */

  sleep(1); /* doze just a sec, should be more than enough in any case */

  return winreturn;
}

/* Execute a command */
int execstuff (int argc, char *oldargv[]) {
  char **argv;
  int i, success, forked;

  argv=(char **)malloc((argc+1)*sizeof(char *));

  for (i=0; i<argc; i++) {
    argv[i]=oldargv[i];
  }
  argv[i]=NULL;

  forked=fork();
  if (forked==-1) {
    perror("Could not fork");
    exit(1);
  }
  if (forked) {
#ifdef DEBUG
    printlist(stderr, "Trying to execute:", argv);
    fprintf(stderr, "\n");
#endif
    success=execvp(argv[0],argv);
    if (success!=0) {
      printlist(stderr, "Could not execute:", argv);
      fprintf(stderr, "\n");
      exit(1);
    }
  } /* Removed the sleep, since it keeps us from getting the Create Event */
  free(argv);
  return(TRUE);
}

void waitformap (Window win) {
  int found=0;
  XEvent Event;

  XSelectInput(dpy, win, READY_EVENTS);
  found=0;
  while (!found && XPending(dpy)) {
    if (XCheckMaskEvent(dpy, READY_EVENTS, &Event))
      if (Event.type==MapNotify)
	if (Event.xmap.window==win)
	  found=TRUE;
  }
#ifdef DEBUG
  fprintf (stderr, "wmswallow: Got MapNotify Event for window 0x%lx\n", win);
  fflush(stderr);
#endif
  while (XCheckTypedWindowEvent(dpy, win, MapNotify, &Event))
    ; /* Flush the map Events */
  XSelectInput(dpy, win, None);
}

/* Find window which matches WM_NAME or WM_CLASS */
Window findnamedwindow (char *class) {
  /* Get All Children of the Root-Window */
  Window result;
  if ((result=(findnamedwindowacc (class, Root)))!=0)
    return result;
  else {
    fprintf(stderr, "Could not find %s\n", class);
    exit (1);
  }
}

/* Recursively walk through all the windows and their children to find the
   first one that matches WM_NAME or WM_CLASS */
/* Only called by findnamedwindow() */
Window findnamedwindowacc (char *class, Window window) {
  Window root_return;
  Window parent_return;
  Window *children_return;
  unsigned int nchildren_return;
  Window runner;
  Window result;
  int i;

  children_return=(Window *)NULL;
  result=(Window)0;

  if (checkwindow(window, class))
    return window;

  if (XQueryTree (dpy, window, &root_return, &parent_return,
		  &children_return, &nchildren_return)&&nchildren_return>0) {
    for
      (i=0,runner=*children_return;i<nchildren_return;
       runner=children_return[++i]) {
      if ((result=findnamedwindowacc(class, runner))!=0)
	break; /* Leave this loop */ /* It's one of the children */
    }
    /* end of for loop*/ /* checked all windows to no avail */
  } /* If the if (XQueryTree...)-part wasn't executed, wmswallow could not get
       Windows (probably no children) */
  if (children_return)
    XFree(children_return);
  return result;
}

/* Checks if the window has WM_NAME or WM_CLASS properties fitting *class */
int checkwindow (Window window, char *class) {
  XClassHint class_hints;
  XTextProperty prop;

  int found=0;

  class_hints.res_name = class_hints.res_class = prop.value =(char *) NULL;

  /* Check WM_CLASS properties name and class */
  if (XGetClassHint(dpy, window, &class_hints)) {
    if (!strcmp(class_hints.res_name, class)  ||
	!strcmp(class_hints.res_class, class)) {
      found = 1; /* It's this window! */
    }
#ifdef DEBUG
    fprintf (stderr, "wmswallow: checkwindow: 0x%lx, class: %s, name: %s\n",
	     win, class_hints.res_class, class_hints.res_name);
    fflush(stderr);
#endif
  }
  /* Check WM_NAME property */
  if (!found && XGetWMName(dpy, window, &prop))
    if (prop.nitems && !strcmp(prop.value, class)) {
      found = 1; /* (-: It's really this window, and we're lucky we guessed its
		    name correctly :-) */
    }
#ifdef DEBUG
  fprintf (stderr, "wmswallow: WM_NAME: %s\n",
	   prop.value);
  fflush(stderr);
#endif

  /* Clean up */
  if (prop.value)
    XFree(prop.value);
  if (class_hints.res_class)
    XFree(class_hints.res_class);
  if (class_hints.res_name)
    XFree(class_hints.res_name);
  return found;
}
