//WindowMaker DVB frontend monitor dock applet version 1. MaxCompress
 #define _GNU_SOURCE
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 #include <getopt.h>
 #include <unistd.h>
 #include <time.h>
 #include <stdint.h>
 #include <unistd.h>

 #include <X11/X.h>
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 #include <X11/extensions/shape.h>
 #include <X11/xpm.h>

 #include <fcntl.h> 
 #include <sys/ioctl.h>

 #include <linux/dvb/frontend.h>
 #define FRONTENDDEVICE "/dev/dvb/adapter%d/frontend%d"

static Atom delete_win;
Font f1,f2;
Window W1,Wicon,W2;
Window W2=0;
Display *XD;
GC xgc;
unsigned int d[7][4]={{22, 2,19,9},{2, 2,19,9},{2,12,19,9},
		      {2,22,19,9},{22,12,19,9},{2,32,19,9},{32,32,42,9}};

// #define DEBUG

unsigned long getcolor(char *color_name)
{
/* getcolor("darkblue") veya getcolor("rgb:52/55/52") olabilir. */
XColor color;

if (!XParseColor(XD, DefaultColormap(XD, DefaultScreen(XD)), color_name, &color))
   fprintf(stderr, "can't parse color %s\n", color_name), exit(1);

if (!XAllocColor(XD, DefaultColormap(XD, DefaultScreen(XD)), &color)) {
   fprintf(stderr, "can't allocate color %s. Using black\n", color_name);
   return BlackPixel(XD, DefaultScreen(XD)); }

return color.pixel;
}

void xmon(int fefd)
{
fe_status_t status;
uint16_t snr, signal, level;
//uint32_t ber, uncorrected_blocks;
struct dvb_frontend_parameters dfp;
XWindowAttributes xwattr;
char a[250];
XTextItem xti;
int freq;
	      
ioctl(fefd, FE_READ_STATUS, &status);
ioctl(fefd, FE_READ_SIGNAL_STRENGTH, &signal);
ioctl(fefd, FE_READ_SNR, &snr);
//ioctl(fefd, FE_READ_BER, &ber);
//ioctl(fefd, FE_READ_UNCORRECTED_BLOCKS, &uncorrected_blocks);
ioctl(fefd, FE_GET_FRONTEND, &dfp);
if (dfp.frequency+10600000 >= 11700000) freq=dfp.frequency+10600000;
                    		   else freq=dfp.frequency+ 9750000;
XGetWindowAttributes(XD,Wicon,&xwattr);

level=(signal)*(xwattr.width-1)/(0xffff);
XSetForeground(XD,xgc,getcolor("gray70"));
XFillRectangle(XD,Wicon,xgc,0,0,xwattr.width,xwattr.height);
if (status & FE_HAS_LOCK) XSetForeground(XD,xgc,getcolor("rgb:ff/00/00"));
		     else XSetForeground(XD,xgc,getcolor("wheat4"));
XFillRectangle(XD,Wicon,xgc,0,15,level,20);
level=(snr)*(xwattr.width-1)/(0xffff);
XFillRectangle(XD,Wicon,xgc,0,38,level,20);

XSetForeground(XD,xgc,getcolor("white"));
XDrawRectangle(XD,Wicon,xgc,0,14,59,21);
XDrawRectangle(XD,Wicon,xgc,0,37,59,21);
XSetForeground(XD,xgc,getcolor("black"));
XDrawLine(XD,Wicon,xgc,0,14,59,14); XDrawLine(XD,Wicon,xgc,0,14,0,35);
XDrawLine(XD,Wicon,xgc,0,37,59,37); XDrawLine(XD,Wicon,xgc,0,37,0,58);

level=(signal)*100/(0xffff);
sprintf(a,"sig %%%d",level);
XSetForeground(XD,xgc,getcolor("black"));
/* ascent font un dikey uzunlugu imis. */
xti.chars=a;
xti.nchars=strlen(a);
xti.delta=0;
xti.font=f2;
XDrawText(XD,Wicon,xgc,2,29,&xti,1);
level=(snr)*100/(0xffff);
sprintf(a,"snr %%%d",level);
xti.chars=a;
xti.nchars=strlen(a);
xti.delta=0;
xti.font=f2;
XDrawText(XD,Wicon,xgc,2,52,&xti,1);
//sprintf(a,"stat %02x|sig %04x|snr %04x|ber %08x|unc %08x|if %08d|sym %d|f %d", status, signal, snr, ber, uncorrected_blocks, dfp.frequency,dfp.u.qpsk.symbol_rate,freq);
sprintf(a,"f %d", freq);
xti.chars=a;
xti.nchars=strlen(a);
xti.delta=0;
xti.font=f1;
XDrawText(XD,Wicon,xgc,1,11,&xti,1);
XFlush(XD);	/*Flush etmeden ekrana bok gibi yaziyor.*/
}

void xmon_W2(int fefd)
{
fe_status_t status;
uint16_t snr, signal, level;
uint32_t ber, uncorrected_blocks;
struct dvb_frontend_parameters dfp;
XWindowAttributes xwattr;
char a[250];
XTextItem xti;
int freq;
	      
ioctl(fefd, FE_READ_STATUS, &status);
ioctl(fefd, FE_READ_SIGNAL_STRENGTH, &signal);
ioctl(fefd, FE_READ_SNR, &snr);
ioctl(fefd, FE_READ_BER, &ber);
ioctl(fefd, FE_READ_UNCORRECTED_BLOCKS, &uncorrected_blocks);
ioctl(fefd, FE_GET_FRONTEND, &dfp);
if (dfp.frequency+10600000 >= 11700000) freq=dfp.frequency+10600000;
                    		   else freq=dfp.frequency+ 9750000;
XGetWindowAttributes(XD,W2,&xwattr);

level=(signal)*(xwattr.width-1)/(0xffff);
XSetForeground(XD,xgc,getcolor("gray70"));
XFillRectangle(XD,W2,xgc,0,0,xwattr.width,xwattr.height);
if (status & FE_HAS_LOCK) XSetForeground(XD,xgc,getcolor("rgb:ff/00/00"));
		     else XSetForeground(XD,xgc,getcolor("wheat4"));
XFillRectangle(XD,W2,xgc,0,15,level,20);
level=(snr)*(xwattr.width-1)/(0xffff);
XFillRectangle(XD,W2,xgc,0,38,level,20);

XSetForeground(XD,xgc,getcolor("white"));
XDrawRectangle(XD,W2,xgc,0,14,xwattr.width-1,21);
XDrawRectangle(XD,W2,xgc,0,37,xwattr.width-1,21);
XSetForeground(XD,xgc,getcolor("black"));
XDrawLine(XD,W2,xgc,0,14,xwattr.width-1,14); XDrawLine(XD,Wicon,xgc,0,14,0,35);
XDrawLine(XD,W2,xgc,0,37,xwattr.width-1,37); XDrawLine(XD,Wicon,xgc,0,37,0,58);

level=(signal)*100/(0xffff);
sprintf(a,"sig %%%d",level);
XSetForeground(XD,xgc,getcolor("black"));
/* ascent font un dikey uzunlugu imis. */
xti.chars=a;
xti.nchars=strlen(a);
xti.delta=0;
xti.font=f2;
XDrawText(XD,W2,xgc,2,29,&xti,1);
level=(snr)*100/(0xffff);
sprintf(a,"snr %%%d",level);
xti.chars=a;
xti.nchars=strlen(a);
xti.delta=0;
xti.font=f2;
XDrawText(XD,W2,xgc,2,52,&xti,1);
sprintf(a,"stat %02x|sig %04x|snr %04x|ber %08x|unc %08x|if %08d|sym %d|f %d", status, signal, snr, ber, uncorrected_blocks, dfp.frequency,dfp.u.qpsk.symbol_rate,freq);
//sprintf(a,"f %d", freq);
xti.chars=a;
xti.nchars=strlen(a);
xti.delta=0;
xti.font=f1;
XDrawText(XD,W2,xgc,1,11,&xti,1);
XFlush(XD);	/*Flush etmeden ekrana bok gibi yaziyor.*/
}


int main(int argc, char **argv)
{
XClassHint classhint;
XWMHints wmhints;
XGCValues gcval;
unsigned int adapter = 0, frontend = 0;
char fedev[128];
int fefd;
int z;
struct dvb_frontend_info fe_info;
int bekle=0, bekle2=0;
/* Komut satiri */
/*
fprintf(stderr,"Option Sayisi=%d\n",argc);
for (sayac2=2;sayac2<=argc;sayac2++)
{
fprintf(stderr,"Option %d=%s\n",sayac2,argv[sayac2-1]);
}
*/
/*
while ((opt=getopt(argc, argv, "h:d:"))!=-1) {
  switch (opt) {
  case 'h':
    fprintf(stderr, "Help: [-h] [-d device] [-t aralik]\n");
  break;
  case 'd':
    dev=strdup(optarg);
  break;
  default:
      fprintf(stderr, "\nUsage: %s %s", argv[0], USAGE);
   exit(1);
  } }
*/

XD = XOpenDisplay("");
if (!XD) { fprintf(stderr, "Default Display acilamadi.\n"); return 1; }
W1 = XCreateSimpleWindow(XD, DefaultRootWindow(XD),0, 0, 64, 64, 1, 50, getcolor("yellow"));
Wicon = XCreateSimpleWindow(XD, W1, 0, 0, 60, 60, 1, 50, getcolor("darkgreen"));

wmhints.initial_state = WithdrawnState;                                     
wmhints.icon_window = Wicon;                                              
wmhints.icon_x = 64;                                               
wmhints.icon_y = 64;                                               
wmhints.window_group = W1;                                                 
wmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
XSetWMHints(XD, W1, &wmhints);                                        

XMapWindow(XD, W1);

classhint.res_name = "wmfemon";
classhint.res_class = "wmfemon";
XSetClassHint(XD, W1, &classhint);
XSelectInput(XD, W1, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask);
XSelectInput(XD, Wicon, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask);
XStoreName(XD, W1, "wmfemon");
XSetIconName(XD, W1, "wmfemon");

gcval.foreground = 50;
gcval.background = 150;
gcval.graphics_exposures = False;
gcval.font = XLoadFont(XD,"-*-fixed-medium-r-normal-*-10-*-*-*-*-*-*-1");
xgc = XCreateGC(XD, W1, GCForeground | GCBackground | GCGraphicsExposures | GCFont, &gcval);

//f1=XLoadFont(XD,"-*-fixed-medium-r-normal-*-10-*-*-*-*-*-*-1");
f1=XLoadFont(XD,"-*-clean-medium-r-*-*-12-*-*-*-*-*-*-*");
f2=XLoadFont(XD,"-*-helvetica-*-r-*-*-12-*-*-*-*-*-*-*");
snprintf(fedev, sizeof(fedev), FRONTENDDEVICE, adapter, frontend);
printf("Device: '%s'\n", fedev);
if ((fefd = open(fedev, O_RDONLY | O_NONBLOCK)) < 0) { perror("opening frontend failed"); return 0;}
z = ioctl(fefd, FE_GET_INFO, &fe_info);
if (z < 0) { perror("ERR ioctl FE_GET_INFO:"); close(fefd); return 0;}

/* main loop */
  while (1)
  {
  XEvent event;
	while (XPending(XD))
	{
	XNextEvent(XD, &event);
	    switch (event.type) 
	    {
	    case Expose:
		/* update */
		fprintf(stderr,"Expose (Update) event geldi.\n");
		fprintf(stderr,"xexpose.window=%ld\n",event.xexpose.window);
		xmon(fefd);
		break;
	    case MotionNotify:
#ifdef DEBUG
		fprintf(stderr,"xmotion x=%d,y=%d\n",event.xmotion.x,event.xmotion.y);
		fprintf(stderr,"xmotion window=%ld\n",event.xmotion.window);
    		fprintf(stderr,"W1 window=%ld\n",W1);
		fprintf(stderr,"Wicon window=%ld\n",Wicon);
		fprintf(stderr,"-----------------------\n");
#endif
		break;
	    case UnmapNotify:
		fprintf(stderr,"UnmapNotify geldi.\n");	        
		break;
	    case DestroyNotify:
		fprintf(stderr,"DestroyNotify geldi.\n");
	        XCloseDisplay(XD);
		exit(0);
		break;
            case ClientMessage: 
            //Kapat tusu icin amma yarak isler lazim ulan. wmclockmon dockapp.c'den alinti.
	    //printf("ClientMessage\n");fflush(stdout);
	        if (event.xclient.data.l[0] == delete_win) {
    	          if (event.xclient.window==W2)
    		  { XDestroyWindow(XD,W2); W2=0;
    		  }
	        }
		break;
	    case ButtonPress:
                //fprintf(stderr,"x=%d,y=%d\n",event.xbutton.x,event.xbutton.y);
		break;
	    case ButtonRelease:
                fprintf(stderr,"x=%d,y=%d\n",event.xbutton.x,event.xbutton.y);
		if (W2==0)
		{ W2 = XCreateSimpleWindow(XD, DefaultRootWindow(XD),100,100,600,60,1,50,getcolor("blue"));
		  if (!W2) { perror("Error: W2"); }
		  delete_win = XInternAtom(XD, "WM_DELETE_WINDOW", False); //DIKKAT: Kapatma tusunun cikmasi icin bu yarak gerekiyor.
		  XSetWMProtocols (XD, W2, &delete_win, 1);
		  XStoreName(XD, W2, "DVB Frontend Monitor for WindowMaker");
		  XSetIconName(XD, W2, "DVB Signal");
		  XMapWindow(XD, W2);
		} else
		{ XDestroyWindow(XD,W2); W2=0;
		}
		break;
	    }
	} 
  if (bekle == 0) xmon(fefd);
  bekle++;
  bekle %= 10; // 10 modulu. W1 1000 ms.
  if (W2) 
  { if (bekle2 == 0) xmon_W2(fefd);
    bekle2++;
    bekle2 %= 3; // 3 modulu. W2 biraz daha hizli gostersin. 300 ms.
  }

  usleep(100000L);
  } //while 
  return 0;
}
