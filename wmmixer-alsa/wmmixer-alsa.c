#include "wmmixer-alsa.h"

int main(int argc, char **argv)
{
	XGCValues gcv;
	unsigned long gcm;
	int exact,left,right,device_index,i;
	XpmAttributes xpmattr;
	XpmColorSymbol xpmcsym[4];
	elementinfo *e;

	scanArgs(argc, argv);
	initXWin(argc, argv);
	exact=left=right=device_index=-1;

	init_mixer();
	cure=element;
	e=element;
	while(e)
	{
		if(!strcasecmp(e->info.eid.name,"Master Volume"))
			e->icon=0;
		else if(!strcasecmp(e->info.eid.name,"PCM Volume"))
			e->icon=1;
		else if(!strcasecmp(e->info.eid.name,"MIC Volume"))
			e->icon=5;
		else if(!strcasecmp(e->info.eid.name,"Line Volume"))
			e->icon=4;
		else if(!strcasecmp(e->info.eid.name,"CD Volume"))
			e->icon=3;
		else if(!strcasecmp(e->info.eid.name,"Synth Volume"))
			e->icon=2;
		else if(!strcasecmp(e->info.eid.name,"PC Speaker Volume"))
			e->icon=6;
		/*
		 * bass = 7
		 * treble = 8
		 */
		else
			e->icon=9;
		e=e->next;
	}

	gcm=GCGraphicsExposures;
	gcv.graphics_exposures=0;
	gc_gc=XCreateGC(d_display, w_root, gcm, &gcv);

	color[0]=mixColor(ledcolor, 0, backcolor, 100);
	color[1]=mixColor(ledcolor, 100, backcolor, 0);
	color[2]=mixColor(ledcolor, 60, backcolor, 40);
	color[3]=mixColor(ledcolor, 25, backcolor, 75);

	xpmcsym[0].name="back_color";
	xpmcsym[0].value=NULL;;
	xpmcsym[0].pixel=color[0];
	xpmcsym[1].name="led_color_high";
	xpmcsym[1].value=NULL;;
	xpmcsym[1].pixel=color[1];
	xpmcsym[2].name="led_color_med";
	xpmcsym[2].value=NULL;;
	xpmcsym[2].pixel=color[2];
	xpmcsym[3].name="led_color_low";
	xpmcsym[3].value=NULL;;
	xpmcsym[3].pixel=color[3];
	xpmattr.numsymbols=4;
	xpmattr.colorsymbols=xpmcsym;
	xpmattr.exactColors=0;
	xpmattr.closeness=40000;
	xpmattr.valuemask=XpmColorSymbols | XpmExactColors | XpmCloseness;
	XpmCreatePixmapFromData(d_display, w_root, wmmixer_xpm, &pm_main, &pm_mask, &xpmattr);
	XpmCreatePixmapFromData(d_display, w_root, tile_xpm, &pm_tile, NULL, &xpmattr);
	XpmCreatePixmapFromData(d_display, w_root, icons_xpm, &pm_icon, NULL, &xpmattr);
	pm_disp=XCreatePixmap(d_display, w_root, 64, 64, DefaultDepth(d_display, DefaultScreen(d_display)));

	if(wmaker || ushape || astep)
		XShapeCombineMask(d_display, w_activewin, ShapeBounding, winsize/2-32, winsize/2-32, pm_mask, ShapeSet);
	else
		XCopyArea(d_display, pm_tile, pm_disp, gc_gc, 0, 0, 64, 64, 0, 0);

	XSetClipMask(d_display, gc_gc, pm_mask);
	XCopyArea(d_display, pm_main, pm_disp, gc_gc, 0, 0, 64, 64, 0, 0);
	XSetClipMask(d_display, gc_gc, None);

	if(count==0)
		fprintf(stderr,"%s : Sorry, no supported channels found.\n", NAME);
	else
	{
		int done=0;
		XEvent xev;

		checkVol();
		XSelectInput(d_display, w_activewin, ExposureMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask);
		XMapWindow(d_display, w_main);

		while(!done)
		{
			while(XPending(d_display))
			{
				XNextEvent(d_display, &xev);
				switch(xev.type)
				{
					case Expose:
						repaint();
						break;
					case ButtonPress:
						pressEvent(&xev.xbutton);
						break;
					case ButtonRelease:
						releaseEvent(&xev.xbutton);
						break;
					case MotionNotify:
						motionEvent(&xev.xmotion);
						break;
					case ClientMessage:
						if(xev.xclient.data.l[0]==deleteWin)
							done=1;
						break;
				}
			}

			if(btnstate & (BTNPREV | BTNNEXT))
			{
				rpttimer++;
				if(rpttimer>=RPTINTERVAL)
				{
					if(btnstate & BTNNEXT)
					{
						cure=cure->next;
						if(!cure)
							cure=element;
					}
					else
					{
						cure=cure->prev;
						if(!cure)
						{
							elementinfo *e;
							e=element;
							while(e->next)
								e=e->next;
							cure=e;
						}
					}
					checkVol();
					rpttimer=0;
				}
			}
			else
				checkVol();
			XFlush(d_display);
			usleep(50000);
		}
	}
	XFreeGC(d_display, gc_gc);
	XFreePixmap(d_display, pm_main);
	XFreePixmap(d_display, pm_tile);
	XFreePixmap(d_display, pm_disp);
	XFreePixmap(d_display, pm_mask);
	XFreePixmap(d_display, pm_icon);
	freeXWin();
	return 0;
}

void initXWin(int argc, char **argv)
{
	int pos;
	XWMHints wmhints;
	XSizeHints shints;

	winsize=astep ? ASTEPSIZE : NORMSIZE;

	if((d_display=XOpenDisplay(display))==NULL)
	{
		fprintf(stderr,"%s : Unable to open X display '%s'.\n", NAME, XDisplayName(display));
		exit(1);
	}
	_XA_GNUSTEP_WM_FUNC=XInternAtom(d_display, "_GNUSTEP_WM_FUNCTION", 0);
	deleteWin=XInternAtom(d_display, "WM_DELETE_WINDOW", 0);

	w_root=DefaultRootWindow(d_display);

	shints.x=0;
	shints.y=0;
	shints.flags=0;
	pos=(XWMGeometry(d_display, DefaultScreen(d_display), position, NULL, 0, &shints, &shints.x, &shints.y, &shints.width, &shints.height, &shints.win_gravity) & (XValue | YValue));
	shints.min_width=winsize;
	shints.min_height=winsize;
	shints.max_width=winsize;
	shints.max_height=winsize;
	shints.base_width=winsize;
	shints.base_height=winsize;
	shints.flags=PMinSize | PMaxSize | PBaseSize;

	createWin(&w_main, shints.x, shints.y);

	if(wmaker || astep || pos)
		shints.flags |= USPosition;
	if(wmaker)
	{
		wmhints.initial_state=WithdrawnState;
		wmhints.flags=WindowGroupHint | StateHint | IconWindowHint;
		createWin(&w_icon, shints.x, shints.y);
		w_activewin=w_icon;
		wmhints.icon_window=w_icon;
	}
	else
	{
		wmhints.initial_state=NormalState;
		wmhints.flags=WindowGroupHint | StateHint;
		w_activewin=w_main;
	}
	wmhints.window_group=w_main;
	XSetWMHints(d_display, w_main, &wmhints);
	XSetWMNormalHints(d_display, w_main, &shints);
	XSetCommand(d_display, w_main, argv, argc);
	XStoreName(d_display, w_main, NAME);
	XSetIconName(d_display, w_main, NAME);
	XSetWMProtocols(d_display, w_activewin, &deleteWin, 1);
}

void freeXWin()
{
	XDestroyWindow(d_display, w_main);
	if(wmaker)
		XDestroyWindow(d_display, w_icon);
	XCloseDisplay(d_display);
}

void createWin(Window *win, int x, int y)
{
	XClassHint classHint;
	*win=XCreateSimpleWindow(d_display, w_root, x, y, winsize, winsize, 0, 0, 0);
	classHint.res_name=NAME;
	classHint.res_class=CLASS;
	XSetClassHint(d_display, *win, &classHint);
}

unsigned long mixColor(char *colorname1, int prop1, char *colorname2, int prop2){
	XColor color, color1, color2;
	XWindowAttributes winattr;
	XGetWindowAttributes(d_display, w_root, &winattr);
	XParseColor(d_display, winattr.colormap, colorname1, &color1);
	XParseColor(d_display, winattr.colormap, colorname2, &color2);
	color.pixel=0;
	color.red=(color1.red*prop1+color2.red*prop2)/(prop1+prop2);
	color.green=(color1.green*prop1+color2.green*prop2)/(prop1+prop2);
	color.blue=(color1.blue*prop1+color2.blue*prop2)/(prop1+prop2);
	color.flags=DoRed | DoGreen | DoBlue;
	XAllocColor(d_display, winattr.colormap, &color);
	return color.pixel;
}

void show_params(void)
{
	fprintf(stdout,"wmmixer-alsa %s\t\tby Sam Hawker\n"
			"\t\t\tAdded support for ALSA by\n"
			"\t\t\tMartin Dahl <dahlm@vf.telia.no>\n\n"
			"Usage: wmmixer-alsa [options]\n\n"
			"\t-h\t\tDisplay this help screen\n"
			"\t-w\t\tUse WithdrawnState\n"
			"\t-s\t\tShaped window\n"
			"\t-a\t\tUse smaller window\n"
			"\t-l <color>\t\tColor for led display\n"
			"\t-b <color>\t\tColor for background\n"
			"\t-p <position>\t\tWindow position\n"
			"\t-d <display>\t\tTarget display\n",VERSION);
	exit(0);
}

void scanArgs(int argc, char **argv)
{
	int x;

	memset(backcolor,0,32);
	memset(ledcolor,0,32);
	memset(display,0,32);
	memset(position,0,32);
	strncpy(ledcolor,LEDCOLOR,32);
	strncpy(backcolor,BACKCOLOR,32);
	strcpy(display,"");
	strcpy(position,"");
	wmaker=ushape=astep=0;
	btnstate=rpttimer=0;
	dragging=count=0;

	for(x=1;x<argc;x++)
	{
		if(argv[x][0]!='-') show_params();
		else switch(argv[x][1])
		{
			case 'w':
				wmaker=1;
				break;
			case 's':
				ushape=1;
				break;
			case 'a':
				astep=1;
				break;
			case 'l':
				strncpy(ledcolor,argv[x+1],32);
				x++;
				break;
			case 'b':
				strncpy(backcolor,argv[x+1],32);
				x++;
				break;
			case 'p':
				strncpy(position,argv[x+1],32);
				x++;
				break;
			case 'd':
				strncpy(display,argv[x+1],32);
				x++;
				break;
			default:
				show_params();
				break;
		}
	}
	return;
}

void checkVol(void)
{
	int nl=0,nr=0;
	nl=convert_range(cure->element.data.volume1.pvoices[0],cure->info.data.volume1.prange[0].min,cure->info.data.volume1.prange[0].max,0,100);
	nr=convert_range(cure->element.data.volume1.pvoices[1],cure->info.data.volume1.prange[1].min,cure->info.data.volume1.prange[1].max,0,100);

	if(1)
	{
		curleft=nl;
		curright=nr;
	}
	else
	{
		if(nl!=curleft||nr!=curright)
		{
			if(nl!=curleft)
			{
				curleft=nl;
				drawLeft();
			}
			if(nr!=curright)
			{
				curright=nr;
				drawRight();
			}
		}
	}
	update();
	repaint();
}

void setVol(int left, int right)
{
	int err;
	callbacks();
	cure->element.data.volume1.pvoices[0]=convert_range(left,0,100,cure->info.data.volume1.prange[0].min,cure->info.data.volume1.prange[0].max);
	if(cure->element.data.volume1.pvoices[0]>cure->info.data.volume1.prange[0].max)
		cure->element.data.volume1.pvoices[0]=cure->info.data.volume1.prange[0].max;
	if(cure->element.data.volume1.pvoices[0]<cure->info.data.volume1.prange[0].min)
		cure->element.data.volume1.pvoices[0]=cure->info.data.volume1.prange[0].min;
	cure->element.data.volume1.pvoices[1]=convert_range(right,0,100,cure->info.data.volume1.prange[1].min,cure->info.data.volume1.prange[1].max);
	if(cure->element.data.volume1.pvoices[1]>cure->info.data.volume1.prange[1].max)
		cure->element.data.volume1.pvoices[1]=cure->info.data.volume1.prange[1].max;
	if((err=snd_mixer_element_write(mixer_handle,&cure->element))<0)
		fprintf(stderr,"Mixer element write error: %s\n",snd_strerror(err));
	return;
}

void callbacks(void)
{
	int err;
	snd_mixer_callbacks_t callbacks;
	memset(&callbacks,0,sizeof(callbacks));
	callbacks.rebuild=NULL;
	callbacks.element=NULL;
	callbacks.group=NULL;
	if((err=snd_mixer_read(mixer_handle,&callbacks))<0)
	{
		fprintf(stderr,"Callbacks error: %s\n",snd_strerror(err));
		return;
	}
	return;
}

int convert_range(int val, int omin, int omax, int nmin, int nmax)
{
	int orange=omax-omin, nrange=nmax-nmin;
	if(orange==0)
		return 0;
	return rint((((double)nrange*((double)val-(double)omin))+((double)orange/2.0))/(double)orange+(double)nmin);
}

void pressEvent(XButtonEvent *xev)
{
	int x=xev->x-(winsize/2-32);
	int y=xev->y-(winsize/2-32);
	if(x>=5 && y>=33 && x<=16 && y<=43)
	{
		cure=cure->prev;
		if(!cure)
		{
			elementinfo *e;
			e=element;
			while(e->next)
				e=e->next;
			cure=e;
		}
		btnstate |= BTNPREV;
		rpttimer=0;
		drawBtns(BTNPREV);
		checkVol();
		return;
	}
	if(x>=17 && y>=33 && x<=28 && y<=43)
	{
		cure=cure->next;
		if(!cure)
			cure=element;
		btnstate|=BTNNEXT;
		rpttimer=0;
		drawBtns(BTNNEXT);
		checkVol();
		return;
	}
	if(x>=37 && x<=56 && y>=8 && y<=56)
	{
		int v=((60-y)*100)/(2*25);
		dragging=1;
		if(x<=50)
			setVol(v,curright);
		if(x>=45)
			setVol(curleft,v);
		checkVol();
		return;
	}
/*	if(x>=5 && y>=47 && x<=28 && y<=57)
	{
		int nl,nr,flags;
		mixer.DeviceSet(channel[curchannel]);
		mixer.Read(&nl,&nr,&flags);
		if(flags & SND_MIXER_DFLG_MUTE)
		{
			btnstate &= ~BTNREC;
			flags &= ~SND_MIXER_DFLG_MUTE;
		}
		else
		{
			btnstate |= BTNREC;
			flags |= SND_MIXER_DFLG_MUTE;
		}
		mixer.Write(nl,nr,flags);
		checkVol();
		return;
	}*/
}

void releaseEvent(XButtonEvent *xev)
{
	dragging=0;
	btnstate &= ~(BTNPREV | BTNNEXT);
	drawBtns(BTNPREV | BTNNEXT);
	repaint();
}

void motionEvent(XMotionEvent *xev)
{
	int x=xev->x-(winsize/2-32);
	int y=xev->y-(winsize/2-32);
	if(x>=37 && x<=56 && y>=8 && dragging)
	{
		int v=((60-y)*100)/(2*25);
		if(v<0)
			v=0;
		if(x<=50)
			setVol(v,curright);
		if(x>=45)
			setVol(curleft,v);
		checkVol();
	}
}

void repaint()
{
	XEvent xev;
	XCopyArea(d_display, pm_disp, w_activewin, gc_gc, 0, 0, 64, 64, winsize/2-32, winsize/2-32);
	while(XCheckTypedEvent(d_display, Expose, &xev));
}

void update()
{
	XCopyArea(d_display, pm_icon, pm_disp, gc_gc, cure->icon*22, 0, 22, 22, 6, 5);
	drawLeft();
	drawRight();
	drawBtns(BTNREC);
}

void drawLeft()
{
	int i;
	XSetForeground(d_display, gc_gc, color[1]);
	for(i=0;i<25;i++)
	{
		if(i==(curleft*25)/100)
			XSetForeground(d_display, gc_gc, color[3]);
		XFillRectangle(d_display, pm_disp, gc_gc, 37, 55-2*i, 9, 1);
	}
}

void drawRight()
{
	int i;
	XSetForeground(d_display, gc_gc, color[1]);
	for(i=0;i<25;i++)
	{
		if(i==(curright*25)/100)
			XSetForeground(d_display, gc_gc, color[3]);
		XFillRectangle(d_display, pm_disp, gc_gc, 48, 55-2*i, 9, 1);
	}
}

void drawBtns(int btns)
{
	if(btns & BTNPREV)
		drawBtn(5, 33, 12, 11, (btnstate & BTNPREV));
	if(btns & BTNNEXT)
		drawBtn(17, 33, 12, 11, (btnstate & BTNNEXT));
	if(btns & BTNREC)
		drawBtn(5, 47, 24, 11, (btnstate & BTNREC));
}

void drawBtn(int x, int y, int w, int h, int down)
{
	if(!down)
		XCopyArea(d_display, pm_main, pm_disp, gc_gc, x, y, w, h, x, y);
	else
	{
		XCopyArea(d_display, pm_main, pm_disp, gc_gc, x, y, 1, h-1, x+w-1, y+1);
		XCopyArea(d_display, pm_main, pm_disp, gc_gc, x+w-1, y+1, 1, h-1, x, y);
		XCopyArea(d_display, pm_main, pm_disp, gc_gc, x, y, w-1, 1, x+1, y+h-1);
		XCopyArea(d_display, pm_main, pm_disp, gc_gc, x+1, y+h-1, w-1, 1, x, y);
	}
}
