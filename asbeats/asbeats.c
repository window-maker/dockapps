#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <time.h>
#include <X11/Xatom.h>
#include <math.h>

#include "led.xpm"
#include "mask.xbm"
#include "mask.xpm"

int posx[4]={8,17,26,35};
static struct tm *clk;

Display *dpy;
Window Root;
int screen;
int d_depth;
XSizeHints mysizehints;
XWMHints mywmhints;
Pixel back_pix,fore_pix;
GC NormalGC;
Window iconwin,win;
char *LedColor="LightSeaGreen";
char *ERR_colorcells="not enough free color cells\n";
typedef struct _XpmIcon {
	Pixmap pixmap;
	Pixmap mask;
	XpmAttributes attributes;
} XpmIcon;

XpmIcon wmclock,led;
XpmIcon visible;
time_t actualtime;

#define MW_EVENTS	(ExposureMask | ButtonPressMask | StructureNotifyMask)
#define FALSE 0
void GetXPM(void);
static void CreatePixmap(Display *dpy, Window Root, char **data, XpmIcon* icon);
Pixel GetColor(char *name);
void RedrawWindow( XpmIcon *v);
void InsertTime();

int main(int argc,char *argv[])
{
	int i;
	unsigned int borderwidth;
	char *display_name=NULL;
	char *wname="asbeats";
	XGCValues gcv;
	unsigned long gcm;
	XEvent Event;
	XTextProperty name;
	XClassHint classHint;
	Pixmap pixmask;
	char *Geometry="";
	if (!(dpy = XOpenDisplay(display_name)))
	{
		fprintf(stderr,"asbeats: can't open display %s\n",
		XDisplayName(display_name));
		exit (1);
	}
	screen=DefaultScreen(dpy);
	Root=RootWindow(dpy,screen);
	d_depth=DefaultDepth(dpy,screen);
	XConnectionNumber(dpy);
	GetXPM();
	mysizehints.flags=USSize|USPosition;
	mysizehints.x=0;
	mysizehints.y=0;
	back_pix=GetColor("white");
	fore_pix=GetColor("black");
	XWMGeometry(dpy,screen,Geometry,NULL,(borderwidth =1),
		&mysizehints,&mysizehints.x,&mysizehints.y,
		&mysizehints.width,&mysizehints.height,&i);
	mysizehints.width=wmclock.attributes.width;
	mysizehints.height=wmclock.attributes.height;
	win=XCreateSimpleWindow(dpy,Root,mysizehints.x,mysizehints.y,
		mysizehints.width,mysizehints.height,borderwidth,
		fore_pix,back_pix);
	iconwin=XCreateSimpleWindow(dpy,win,mysizehints.x,mysizehints.y,
		mysizehints.width,mysizehints.height,borderwidth,fore_pix,
		back_pix);
	XSetWMNormalHints(dpy,win,&mysizehints);
	classHint.res_name="asbeats";
	classHint.res_class="ASBeats";
	XSetClassHint(dpy,win,&classHint);
	XSelectInput(dpy,win,MW_EVENTS);
	XSelectInput(dpy,iconwin,MW_EVENTS);
	if(XStringListToTextProperty(&wname,1,&name)==0)
	{
		fprintf(stderr,"asbeats: can't allocate window name\n");
		exit(-1);
	}
	XSetWMName(dpy, win, &name);
	gcm = GCForeground|GCBackground|GCGraphicsExposures;
	gcv.foreground = fore_pix;
	gcv.background = back_pix;
	gcv.graphics_exposures = FALSE;
	NormalGC = XCreateGC(dpy, Root, gcm, &gcv);
	pixmask = XCreateBitmapFromData(dpy, win, mask_bits, mask_width,
	mask_height);
	XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
	XShapeCombineMask(dpy, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);
	mywmhints.initial_state = WithdrawnState;
	mywmhints.icon_window = iconwin;
	mywmhints.icon_x = mysizehints.x;
	mywmhints.icon_y = mysizehints.y;
	mywmhints.window_group = win;
	mywmhints.flags = StateHint | IconWindowHint |
		IconPositionHint | WindowGroupHint;
	XSetWMHints(dpy, win, &mywmhints);
	XSetCommand(dpy, win, argv, argc);
	XMapWindow(dpy,win);
	InsertTime();
	RedrawWindow(&visible);
	while(1)
	{
		if (actualtime != time(0))
		{
			InsertTime();
			RedrawWindow(&visible);

		}
		while (XPending(dpy))
		{
			XNextEvent(dpy,&Event);
			switch(Event.type)
			{
				case Expose:
					if(Event.xexpose.count == 0 )
					RedrawWindow(&visible);
					break;
				case DestroyNotify:
					XCloseDisplay(dpy);
					exit(0);
				default:
					break;
			}
		}
		XFlush(dpy);
		usleep(50000L);
	}
	return 0;
}

void nocolor(char *a, char *b)
{
	fprintf(stderr,"asbeats: can't %s %s\n", a,b);
}

void GetXPM(void)
{
	XColor col;
	XWindowAttributes attributes;
	char led1[22];
	char led2[22];
	int ret;
	XGetWindowAttributes(dpy,Root,&attributes);
	if(!XParseColor(dpy,attributes.colormap,LedColor,&col))
	{
		nocolor("parse",LedColor);
	}
	sprintf(&led1[0],".      c #%4X%4X%4X",col.red,col.green,col.blue);
	for(ret=10;ret<22;ret++)
		if(led1[ret]==' ')
			led1[ret]='0';
	led_xpm[2] = &led1[0];
	col.red=(col.red/10)*3;
	col.green=(col.green/10)*3;
	col.blue=(col.blue/10)*3;
	sprintf(&led2[0],"X      c #%4X%4X%4X",col.red,col.green,col.blue);
	for(ret=10;ret<22;ret++)
		if(led2[ret]==' ')
			led2[ret]='0';
	led_xpm[3]=&led2[0];
	CreatePixmap(dpy,Root,mask_xpm,&wmclock);
	CreatePixmap(dpy,Root,mask_xpm,&visible);
	CreatePixmap(dpy,Root,led_xpm,&led);
}

static void CreatePixmap(Display *dpy, Window Root, char **data, XpmIcon* icon)
{
	int ret;
	icon->attributes.valuemask |= (XpmReturnPixels |
		XpmReturnExtensions | XpmExactColors | XpmCloseness);
	icon->attributes.exactColors=False;
	icon->attributes.closeness=40000;
	ret=XpmCreatePixmapFromData(dpy,Root,data,&(icon->pixmap),
		&(icon->mask),&(icon->attributes));
	if(ret != XpmSuccess)
	{
		fprintf(stderr,"%s",ERR_colorcells);
		exit(1);
	}
}

int flush_expose (Window w)
{
	XEvent dummy;
	int i=0;
	while(XCheckTypedWindowEvent(dpy,w,Expose,&dummy))
		i++;
	return i;
}

void RedrawWindow( XpmIcon *v)
{
	flush_expose (iconwin);
	XCopyArea(dpy,v->pixmap,iconwin,NormalGC,0,0,v->attributes.width,
		v->attributes.height,0,0);
	flush_expose(win);
	XCopyArea(dpy,v->pixmap,win,NormalGC,0,0,v->attributes.width,
		v->attributes.height,0,0);
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

void Beat()
{
	float fTime;

	fTime=(clk->tm_hour*3600+clk->tm_min*60+clk->tm_sec);
	fTime=fTime+timezone+3600;
	if (clk->tm_isdst)
		fTime-=3600;
	fTime=(fTime*1000)/86400;
	if (fTime >= 1000)
		fTime-=1000;
	else
		if (fTime < 0)
			fTime+=1000;

	XCopyArea(dpy,led.pixmap,visible.pixmap,NormalGC,
			90,0,9,11,posx[0],15);
	XCopyArea(dpy,led.pixmap,visible.pixmap,NormalGC,
			9*(((int)fTime)/100),0,9,11,posx[1],15);
	XCopyArea(dpy,led.pixmap,visible.pixmap,NormalGC,
			9*((((int)fTime) / 10) % 10),0,9,11,posx[2],15);
	XCopyArea(dpy,led.pixmap,visible.pixmap,NormalGC,
			9*((int)fTime % 10),0,9,11,posx[3],15);

	fTime = abs(((fTime - abs(fTime)) * 1000) + 0.5);

	XCopyArea(dpy,led.pixmap,visible.pixmap,NormalGC,
			99,0,9,11,posx[0],32);
	XCopyArea(dpy,led.pixmap,visible.pixmap,NormalGC,
			9*(((int)fTime)/100),0,9,11,posx[1],32);
	XCopyArea(dpy,led.pixmap,visible.pixmap,NormalGC,
			9*((((int)fTime) / 10) % 10),0,9,11,posx[2],32);
	XCopyArea(dpy,led.pixmap,visible.pixmap,NormalGC,
			9*((int)fTime % 10),0,9,11,posx[3],32);

	return;
}

void InsertTime()
{
	actualtime=time(0);
	clk=localtime(&actualtime);
	XCopyArea(dpy,wmclock.pixmap,visible.pixmap,NormalGC,
		0,0,mysizehints.width,mysizehints.height,0,0);
	Beat();
	return;
}
