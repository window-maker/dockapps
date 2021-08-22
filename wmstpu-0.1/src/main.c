/* Created by Anjuta version 1.2.2 */
/*	This file will not be overwritten */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdio.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/signal.h>
#include <time.h>

#include "wmgeneral.h"
#include "../pixmaps/stpu2.xpm"
#include "../pixmaps/stpu_mask2.xbm"

#define START 0
#define MIN 1
#define SEC 2
#define STD 3
#define SMALL 106
#define NORMAL 81
#define BIG 93
#define MAX_TIME 13*3600

struct __stpuTimer{
	time_t ende;
	int running;
	int interval;
	int timeReached;
};
int faded=0;
typedef struct __stpuTimer _stpuTimer;

/* Prototypes */
static void print_usage(void);
static void ParseCMDLine(int argc, char *argv[]);

static void print_usage(void)
{
	printf("\nHello Dock App version: %s\n", VERSION);
	printf("\nTODO: Write This.\n\n");
}
void handleMousePressed(int button,_stpuTimer* stt)
{
	faded=0;
	/*switch(button)
	{
		case -1:
			break;
		case UP:
			copyXPMArea(112,68, 10,9, 48,5);
			break;
		case DOWN:
			copyXPMArea(112,80, 10,9, 48,34);
			break;
		case START:
			if((*stt).running)
			{
				copyXPMArea(66,43,53,11,5,48);
			}
			else
			{
				copyXPMArea(66,17,53,11,5,48);
			}
			//run
			break;
		case FADE:
			break;
		default:
			printf("[ERR unknown Button]");
	}*/
			
}
void handleMouseReleased(int button,_stpuTimer* stt)
{
	
	switch(button)
	{
		case -1:
			break;
		case MIN:
			if(faded == 0)
			(*stt).interval +=60;
			break;
		case SEC:
			if(faded == 0)
			(*stt).interval +=1;

			break;
		case START:
			if((*stt).running)
			{
				//stop
				copyXPMArea(11,68, 12,21, 27,34);
				(*stt).timeReached =0;
				(*stt).running =0;

			}
			else
			{
				//start
				//printf("timer: %d\n",(*stt).interval);
				if((*stt).interval>0)
				{
					(*stt).ende = time(NULL)+ (*stt).interval;
					(*stt).running =1;
				}
			}
			break;
		case STD:
			if(faded == 0)
				(*stt).interval +=3600;
			break;
		default:
			printf("[ERR unknown Button]");
	}
			
}

void handleMouseMove(int button, XEvent event,_stpuTimer *stt)
{
	static int yLast = -1;
	if( yLast <0 )
		yLast = event.xmotion.y_root;
	if(button != -1)
	{
		faded = 1;
	}
	switch(button)
	{
		case -1:
			break;
		case MIN:
			(*stt).interval+= ((yLast - event.xmotion.y_root) > 0)? 60: -60;
			break;
		case SEC:

			(*stt).interval+= ((yLast - event.xmotion.y_root) > 0)? 1: -1;
			break;
		case STD:
			
			(*stt).interval+= ((yLast - event.xmotion.y_root) > 0)? 3600: -3600;
			break;
		case START:
			;
		default:
			printf("UNKNOWN BUTTON!!!!");
	}
	yLast = event.xmotion.y_root;
	if((*stt).interval < 0)
	{
		(*stt).interval=0;
	}
	if((*stt).interval > MAX_TIME)
	{
		(*stt).interval=MAX_TIME;
	}
}

void ParseCMDLine(int argc, char *argv[])
{
	int	i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-display")) {
			++i; /* -display is used in wmgeneral */
		/*} else if (!strcmp(argv[i], "-option_with_param")) {
			strcpy(param, argv[++i]);
		} else if (!strcmp(argv[i], "-option_wo_param")) {
			param = 1;*/
		} else {
			print_usage();
			exit(1);
		}
	}
}
void paintNumberAt(int num, int dx, int dy, int size)
{
	int sx,sy,y;
	int x=1;
	switch(size)
	{
		case BIG:
			y= 112;
			sx= 12;
			sy=15;
			break;
		case SMALL:
			y= 106;
			sx= 4;
			sy =5;
			break;
		case NORMAL:
			y= 81;
			sx= 5;
			sy=7;
			break;
		default:
			y= 69;
			sx= 5;
			sy= 7;
	}
	//printf("num:%d \n",num);
	copyXPMArea((num/10)*sx+x,y, sx,sy, dx,dy);
	copyXPMArea((num%10)*sx+x,y, sx,sy, dx+sx,dy);
}

void paintRunner()
{
	static int index =0;
	int x=23,y=68;
	int sx=12, sy=21;
	copyXPMArea(index*sx+x,y, sx,sy, 27,34);
	index = (index+1)%5;
}

void paintJumper()
{
	static int index =0;
	int x=98,y=68;
	int sx=12, sy=21;
	copyXPMArea(index*sx+x,y, sx,sy, 27,34);
	index = (index+1)%6;
}

void paintTimer(long interval)
{
	int min=0;
	int std=0;
	int sec=0;
	int z1 =0;
	
	std = abs(interval / 3600);
	min = abs((interval % 3600)/60);
	sec = interval % 60;
	paintNumberAt(min,4,12,BIG);
	paintNumberAt(sec,35,12,BIG);
	
	//draw std 
	copyXPMArea(2,92, 54,6, 5,5);
	for(z1 =0; z1< std; z1++)
	{
		copyXPMArea(2,67, 4,6, z1*4+5,5);
	}
	//printf("%d:%d sec:%d\n",std,min,sec);
}

int main(int argc, char *argv[])
{
	XEvent	event;
	__useconds_t usleepTime=100000;
	_stpuTimer stpuTimer;
	stpuTimer.ende=0;
	stpuTimer.running =0;
	stpuTimer.interval =0;
	stpuTimer.timeReached =0;
	int button = -1;
	
	ParseCMDLine(argc, argv);
	openXwindow(argc, argv, stpu2_xpm, stpu_mask2_bits , stpu_mask2_width, stpu_mask2_height);
	AddMouseRegion(START,4,31,59, 59);
	AddMouseRegion(MIN,4,12,28,27);
	AddMouseRegion(SEC,34,12,59,27);
	AddMouseRegion(STD,5,5,59,11);
	
	/* Loop Forever */
	while (1) {
		usleepTime=50000;
		//printf("endlos\n");
		if(stpuTimer.running && !stpuTimer.timeReached)
		{
			long t = stpuTimer.ende - time(NULL);
			if(t <= 0)
			{
				t=0;
				stpuTimer.timeReached =1;
			}
			paintRunner();
			paintTimer(t);
		}
		else if(stpuTimer.timeReached && stpuTimer.running )
		{
			paintJumper();
		}
		else
		{
			paintTimer(stpuTimer.interval);
		}
		//printf("run:%d reached:%d\n",stpuTimer.running,stpuTimer.timeReached);
		/* Process any pending X events. */
		while (XPending(display)) {
			if(XEventsQueued(display,QueuedAlready))
			{
			XNextEvent(display, &event);
				switch (event.type) {
					case Expose:
						RedrawWindow();
						break;
					case ButtonPress:
						//printf("button pressed x:%d y:%d\n",event.xbutton.x,event.xbutton.y);
						button = CheckMouseRegion(event.xbutton.x,event.xbutton.y);
						handleMousePressed(button,&stpuTimer);
						RedrawWindow();
						break;
					case ButtonRelease:
						//printf("button released\n");
						handleMouseReleased(button,&stpuTimer);
						RedrawWindow();
						usleepTime=30000;
						button = -1;
						break;
					case MotionNotify:
						//mouse move event
						handleMouseMove(button,event,&stpuTimer);
						usleepTime=30000;
						break;
					default:
					;
						//printf("default Event %d\n",event.type);
				}
			}
		}
		RedrawWindow();
		//XFlush(display);
		//nanosleep(&delay
		usleep(usleepTime);
	}

	/* we should never get here */
	return (0);
}
