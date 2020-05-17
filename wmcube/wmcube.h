#ifndef _WMCUBE_HPP_
#define _WMCUBE_HPP_

#include "wmapp/wmwidget.h"
#include "wmapp/wmapp.h"
#include "wmapp/wmwindow.h"
#include "wmapp/wmframe.h"
#include "wmapp/wmcanvas.h"
#include "wmapp/wmmeterbar.h"
#include "wmapp/wmtextbar.h"
#include "wmapp/wmcallback.h"

#ifdef DARWIN
#include "cpumoncc/darwin/CpuMonitor.h"
#endif
#ifdef FREEBSD
#include "cpumoncc/freebsd/CpuMonitor.h"
#endif
#ifdef LINUX
#include "cpumoncc/linux/CpuMonitor.h"
#endif
#ifdef NETBSD
#include "cpumoncc/netbsd/CpuMonitor.h"
#endif
#ifdef OPENBSD
#include "cpumoncc/openbsd/CpuMonitor.h"
#endif
#ifdef SOLARIS
#include "cpumoncc/solaris/CpuMonitor.h"
#endif

#define WMCUBE_VERSION "0.99-pre1"
#define WMCUBE_RELDATE "2003-02-28"

typedef struct
{ 
   const int  c; 
   const char *parms;
   const char *desc;
} 
cl_opt;

#define CL_OPT_OBJECT_INT  (int)'o'
#define CL_OPT_CPU_INT     (int)'c'
#define CL_OPT_UPDATE_INT  (int)'u'
#define CL_OPT_WINDOW_INT  (int)'k'
#define CL_OPT_RSPEED_INT  (int)'r'    
#define CL_OPT_NICE_INT    (int)'n'
#define CL_OPT_FGCOLOR_INT (int)'f'
#define CL_OPT_SHADE_INT   (int)'s'    
#define CL_OPT_BGCOLOR_INT (int)'b'
#define CL_OPT_INVERT_INT  (int)'i'    
#define CL_OPT_MODE_INT    (int)'m'    
#define CL_OPT_LSOURCE_INT (int)'l'
#define CL_OPT_WHEEL_INT   (int)'w'
#define CL_OPT_HELP_INT   (int)'h'

const cl_opt CL_OPT_OBJECT =	{ CL_OPT_OBJECT_INT,"filename/directory","load wmc-object or scan a directory for objects"};
const cl_opt CL_OPT_CPU	= 		{ CL_OPT_CPU_INT    ,"X","monitor cpu X (starts with 0, default average over all)" };
const cl_opt CL_OPT_UPDATE = 	{ CL_OPT_UPDATE_INT ,"X","read cpu load every X centiseconds (default 50)" };
const cl_opt CL_OPT_WINDOW =	{ CL_OPT_WINDOW_INT,"","start in classic mode" };
const cl_opt CL_OPT_RSPEED =	{ CL_OPT_RSPEED_INT ,"X","rotating speed 0-9 (default 3)" };
const cl_opt CL_OPT_NICE =		{ CL_OPT_NICE_INT, "","exclude nice processes" };
const cl_opt CL_OPT_FGCOLOR =	{ CL_OPT_FGCOLOR_INT,"0xXXXXXX","solid/wireframe color (default CYANish color)" };
const cl_opt CL_OPT_SHADE =		{ CL_OPT_SHADE_INT,"X","amount of shading on solid objects 0-100 (default 80)" };
const cl_opt CL_OPT_BGCOLOR =	{ CL_OPT_BGCOLOR_INT,"0xXXXXXX","background color (default 0x202020)" };
const cl_opt CL_OPT_INVERT =	{ CL_OPT_INVERT_INT, "","invert rotating speed/cpu load relationship" };
const cl_opt CL_OPT_MODE =		{ CL_OPT_MODE_INT,"wire/solid/solidwire","default mode for objects with multiple modes" };
const cl_opt CL_OPT_LSOURCE =	{ CL_OPT_LSOURCE_INT,"","moving light source (default clickable light source)" };
const cl_opt CL_OPT_WHEEL =		{ CL_OPT_WHEEL_INT,"","switch object with mouse wheel (default zoom)" };
const cl_opt CL_OPT_HELP =		{ CL_OPT_HELP_INT,"","print help" };

#define NUM_CL_OPT 11

const cl_opt CL_OPT[NUM_CL_OPT] =
{
	CL_OPT_OBJECT,	
	CL_OPT_CPU,
	CL_OPT_UPDATE,
	CL_OPT_WINDOW, 
	CL_OPT_RSPEED, 
	CL_OPT_NICE, 
	CL_OPT_FGCOLOR, 
	CL_OPT_SHADE, 
	CL_OPT_BGCOLOR, 
	//CL_OPT_INVERT, 
	//CL_OPT_MODE, 
	CL_OPT_LSOURCE, 
	//CL_OPT_WHEEL,
	CL_OPT_HELP
};

void printUsage()
{
  printf("wmCube %s (%s) (C) Robert Kling 2003\n\n", WMCUBE_VERSION, WMCUBE_RELDATE);
  printf("Usage: wmcube [options]\n\n");

  for (int i = 0; i < NUM_CL_OPT; i++)
	{
		
		printf(" -%c %s", CL_OPT[i].c, CL_OPT[i].parms);
		for (int j = 0; j < 22 - strlen(CL_OPT[i].parms); j++) printf(" ");
		printf("%s\n", CL_OPT[i].desc);
	}

//  printf("\nThe -c option is only availible on Linux and Solaris.");
//  printf("\nThe -n option is only availible on FreeBSD, Linux and NetBSD.\n\n"); 
  fflush(stdout);
}

class WMCanvasCallback : public WMCallback, public WMCanvas
{
public:
  WMCanvasCallback() : WMCallback(), WMCanvas() { }
};


#endif
