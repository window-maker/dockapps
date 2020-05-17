#include <stdio.h>
#include <stdexcept>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <unistd.h>

#include "WmcObject.h"
#include "wmcube.h"

using namespace std;

/**************************************************

 Defines

**************************************************/

#define DEFAULT_WMC_PATH "/usr/share/wmcube"
#define WMC_REDRAW 2 // centiseconds (= 50 fps)
#define CANVAS_BGCOLOR 0x202020
#define CYAN 0x20B2AE

#define WINDOW_NEW 	0
#define WINDOW_CLASSIC	1

// Rotation speed (in radians) for each screen update
#define ROT_SPEED_X (float)clopt_rspeed * (0.005 + (float)cpu_load / 3000.0)
#define ROT_SPEED_Y (float)clopt_rspeed * (0.005 + (float)cpu_load / 3500.0)
#define ROT_SPEED_Z (float)clopt_rspeed * (0.005 + (float)cpu_load / 4000.0)

// Placement for the objects in new/classic mode
#define XOFF_NEW 27
#define YOFF_NEW 20
#define ZOFF_NEW 400
#define XOFF_CLASSIC XOFF_NEW
#define YOFF_CLASSIC 27
#define ZOFF_CLASSIC -ZOFF_NEW

/**************************************************

 Function predeclaration

**************************************************/

// Timer and widget callbacks
void gatherStatistics(const WMApp *a, void *);
void displayWmcObject(const WMApp *a, WMWidget *w, void *);
void displayCpuText(const WMApp *a, WMWidget *w, void *);
void displayCpuMeter(const WMApp *a, WMWidget *w, void *);
void canvasClicked(const WMApp *a, WMWidget *w, void *);

// Other funcs
void switchWindow(WmcObject *currentwmc);
void scanWmcDirectory(const char *dirname);
char *nextObject();
char *prevObject();
void clearCanvas(WMCanvas *canvas);
bool checkMouseRegion(const WMCanvas *c, int button, int ulx, int uly, int lrx, int lry);

/**************************************************

 Global variables

**************************************************/

// "Main" variables
WMApp 		wmcube;
CpuMonitor 	*cpumon = 0;
WmcObject 	*wmc, *wmcnew, *wmcold;
int 		cpu_load = 0;
int 		switching = 0;
int			xoff = XOFF_NEW, yoff = YOFF_NEW, zoff = ZOFF_NEW;

// A list of available object's filenames and an iterator
vector <char *> objects;
vector <char *>::const_iterator obj_iterator;

// Variables for command line options (set to their defaults)
char	*clopt_object 	= 0;
int	 	 clopt_cpu 		= -1;
int	 	 clopt_update 	= 50;
int		 clopt_window 	= WINDOW_NEW; 
float	 clopt_rspeed 	= 3;
bool	 clopt_nice 	= true;
unsigned clopt_fgcolor 	= CYAN;
float 	 clopt_shade 	= 90.0;
unsigned clopt_bgcolor 	= CANVAS_BGCOLOR;
int	 	 clopt_invert	= false;
int		 clopt_mode 	= SOLID;
bool	 clopt_lsource 	= false;
bool	 clopt_wheel 	= false;
bool	 clopt_help		= false;

/**************************************************

 Main

**************************************************/ 
int main(int argc, char *argv[])
{
  WMApp::initialize(argc, argv);
	
  /**************************************************

    Parse command line parameters
  
  ***************************************************/
  int  c = 0;
  char clstr[64] = "";
  
  for (int i = 0; i < NUM_CL_OPT; i++)
    {
      clstr[c++] = CL_OPT[i].c;
      if (strlen(CL_OPT[i].parms) > 0) clstr[c++] = ':';
    }
  
  clstr[c] = 0;
  
  while ((c = getopt (argc, argv, clstr)) != -1)
    {
      switch (c)
		{
		case CL_OPT_OBJECT_INT:
		  clopt_object = strndup(optarg, 256);
		  break;
		case CL_OPT_CPU_INT:
		  clopt_cpu = atoi(optarg);
		  break;
		case CL_OPT_UPDATE_INT:
		  clopt_update = atoi(optarg);
		  break;
		case CL_OPT_WINDOW_INT:
		  clopt_window = WINDOW_CLASSIC;
		  break;
		case CL_OPT_RSPEED_INT:
		  clopt_rspeed = atoi(optarg);
		  break;
		case CL_OPT_NICE_INT:
		  clopt_nice = false;
		  break;
		case CL_OPT_FGCOLOR_INT:
		  sscanf(optarg, "%x", &clopt_fgcolor);
		  //printf("FGCOLOR = 0x%X   shade = %.3f\n", clopt_fgcolor, clopt_shade); 
		  break;
		case CL_OPT_SHADE_INT:
		  clopt_shade = atoi(optarg);
		  break;
		case CL_OPT_BGCOLOR_INT:
		  sscanf(optarg, "%x", &clopt_bgcolor);
		  //printf("BGCOLOR = 0x%X\n", clopt_bgcolor); 
		  break;
		case CL_OPT_INVERT_INT:
		  clopt_invert = true;
		  break;
		case CL_OPT_MODE_INT:
		  clopt_mode = atoi(optarg);
		  break;
		case CL_OPT_LSOURCE_INT:
		  clopt_lsource = true;
		  break;
		case CL_OPT_WHEEL_INT:
		  clopt_wheel = true;
		  break;
		case CL_OPT_HELP_INT:
		  clopt_help = true;
		  printUsage();
		  break;
		default:
		  clopt_help = true;
		  printUsage();
		  break;
		}
    }
  
  
  /**************************************************
    
     Set up cpu monitor
    
  ***************************************************/
    
    try
      {
	
#ifdef DARWIN
  		cpumon = new CpuMonitor();
#endif
	
#ifdef FREEBSD
  		cpumon = new CpuMonitor(clopt_nice);
#endif
	
#ifdef LINUX
  		cpumon = new CpuMonitor(clopt_cpu, clopt_nice);
#endif
	
#ifdef NETBSD
  		cpumon = new CpuMonitor(clopt_nice);
#endif
	
#ifdef OPENBSD
  		cpumon = new CpuMonitor();
#endif
	
#ifdef SOLARIS
  		cpumon = new CpuMonitor(clopt_cpu);
#endif
	
      }
    catch (exception &e)
      {
		cout << e.what() << endl;
		return -1;
      }
    
    cpumon->setRange(99); // Only two digits fits in text area
    
    /**************************************************
      
      Scan wmc-directory and load (initial) object
      
    **************************************************/
    try
      {
		if (clopt_object == 0)
	  	{
	  		scanWmcDirectory(DEFAULT_WMC_PATH);
	  		wmc = (objects.size() > 0) ? new WmcObject(nextObject()) : new WmcObject();
	  	}
		else
	  	{
	    	scanWmcDirectory(clopt_object);
	    	wmc = (objects.size() > 0) ? new WmcObject(nextObject()) : new WmcObject(clopt_object);
	  	}
      }
    catch (exception &e)
    {
		cout << e.what() << endl;
		return -1;
    }
    
    wmc->setColorShading(clopt_fgcolor, clopt_shade);
	wmc->setMode(clopt_mode);

    printf("Objects in %s: %d\n", DEFAULT_WMC_PATH, objects.size());
    
    /***********************************************************************
    
      Set up the new style window	
    
    ***********************************************************************/
    
    WMWindow nw;
    WMFrame nmain, bottom, botleft, botright;
    WMCanvasCallback ncanvas;
    WMTextBar cpu("00", 0);
    WMMeterBar meter(0, 100);
    
    nw.setpadding(1);
    nw.addchild(nmain);
    nw.addchild(bottom);
    nw.setorientation(Orientation::Vertical);
    nw.setaspectratios(80, 21);
    nw.add_timed_function(clopt_update, gatherStatistics, 0);
    nw.add_timed_function(clopt_update, displayCpuText,  &cpu, 0);
    nw.add_timed_function(clopt_update, displayCpuMeter, &meter, 0);
    nw.add_timed_function(WMC_REDRAW, displayWmcObject, &ncanvas, wmc);

    ncanvas.addcallback(canvasClicked, &ncanvas, 0);	
  
    ncanvas.setbuffered(true);
    meter.setstyle(WMMeterBar::Blue);
  
    //bottom.setpadding(2);
    //bottom.setborder(0);
    nmain.addchild(ncanvas);
    bottom.addchild(botleft);
    bottom.addchild(botright);
    bottom.setorientation(Orientation::Horizontal);
    bottom.setaspectratios(4, 11);

    meter.setorientation(Orientation::Horizontal);

    botleft.addchild(cpu);
    botright.setorientation(Orientation::Vertical);
    botright.setpadding(2);
    botright.addchild(meter);

  	/***********************************************************************

     Set up the classic style window

  	***********************************************************************/
  
	WMWindow cw;
	WMCanvasCallback ccanvas;
	    
	cw.setpadding(1);
	cw.addchild(ccanvas);
	cw.setaspectratios(1, 1);
	cw.add_timed_function(clopt_update, gatherStatistics, 0);
	cw.add_timed_function(WMC_REDRAW, displayWmcObject, &ccanvas, wmc);
	
	ccanvas.addcallback(canvasClicked, &ccanvas, 0);
	ccanvas.setbuffered(true);
	  
	/***********************************************************************
	
	   Ok we're more or less set to go..
	  
	***********************************************************************/
	
	wmcube.addwindow(nw);
	wmcube.addwindow(cw);
	
	// If the user selected to start in classic mode...
	if (clopt_window == WINDOW_CLASSIC)
	{
		// .. we need to tell switchWindow that we currently are in new mode.
  		clopt_window = WINDOW_NEW;
  		switchWindow(wmc);	
		wmcube.run(cw);
	}
	else
		wmcube.run(nw);
	
	delete cpumon;
	delete wmc;
	
	return 0;
}

/********************************************************************************

 void switchWindow(WmcObject *currentwmc)

 Switch between new/classic mode. 

********************************************************************************/
void switchWindow(WmcObject *currentwmc)
{
  //printf("switching windows\n"); fflush(stdout);
  
  if (clopt_window == WINDOW_CLASSIC)
    {
		clopt_window = WINDOW_NEW;
		xoff = XOFF_NEW;
		yoff = YOFF_NEW;
		zoff = ZOFF_NEW;
	}
  else
    {
		clopt_window = WINDOW_CLASSIC;
		xoff = XOFF_CLASSIC;
		yoff = YOFF_CLASSIC;
		zoff = ZOFF_CLASSIC;
    }
    
  wmcube.switch_to(clopt_window);	
  
  // Zoom/translate object to fit current canvas size
  currentwmc->modifyZOffset(zoff);
  currentwmc->setYOffset(yoff);
}

/********************************************************************************

 void canvasClicked(const WMApp *a, WMWidget *w, void *)

 Event callback for everything that has to do with the canvas that the object(s)
 are being drawn in.

********************************************************************************/
#define ZOOM_STEP     100
#define LSOURCE_Z_OFF 10

void canvasClicked(const WMApp *a, WMWidget *w, void *)
{
  WMCanvas *c = dynamic_cast<WMCanvas *>(w);
  WmcObject *current;

  if (!c) return;

  current = switching ? wmcnew : wmc;
  
  /*
    Mouse-wheel zoom in and out (unless clopt_wheel == true).
  */
  if (((a->mouseclick().button == Button4) && (!clopt_wheel)) || 
  		checkMouseRegion(c, Button1, 47, 12, 65, 48))
  {
    current->modifyZOffset(-ZOOM_STEP);
    return;
  }  
  if (((a->mouseclick().button == Button5) && (!clopt_wheel))  || 
  		checkMouseRegion(c, Button1, 0, 12, 16, 48))
  {
    current->modifyZOffset( ZOOM_STEP);
    return;
  }
  
  /*
    Switch window mode
  */
  if (checkMouseRegion(c, Button1, 0, 0, 13, 13)) 
	{
  		switchWindow(current);
  		return;
	}
	 
  /*
    Switch 3d mode
  */
  if ((a->mouseclick().button == Button2) || checkMouseRegion(c, Button1, 15, yoff-10, 48, yoff+10))
	{			
  		current->setMode(++clopt_mode % NUM_MODES);
  		return;
	}
    
  /*
    Change lightsource position with right mouse button
  */
  if (a->mouseclick().button == Button3)
  {
    current->setLightSource(xoff - a->mouseclick().relative_to(c).x, 
    						yoff - a->mouseclick().relative_to(c).y,
			    			LSOURCE_Z_OFF);
	return;
  }
  
  /*
    Switch objects (if not already doing so)
  */
 if ((switching == 0) && (objects.size() > 0))
    {
      // Store a pointer to the old object so we can delete it later
      wmcold = wmc;
      
      // Switch object with mousewheel instead of zooming
      if ((a->mouseclick().button == Button4) && clopt_wheel)
		{
	  		// Scrolling object up with mousewheel
	  		switching = 1;
	  		wmcnew = new WmcObject(nextObject());
		}
      else if ((a->mouseclick().button == Button5) && clopt_wheel)
		{
	 		// Scrolling object down with mousewheel
	  		switching = -1;
	  		wmcnew = new WmcObject(prevObject());
		}
      // Switch objects by clicking
      else if (checkMouseRegion(c, Button1, 12, 0, 60, yoff - 5))
		{
	  		// Scrolling object up by clicking top of canvas
	  		switching = 1;
	  		wmcnew = new WmcObject(nextObject());
		}
      else if (checkMouseRegion(c, Button1, 12, yoff + 5, 60, 60))
		{
	  		// Scrolling object down by clicking bottom of canvas
	  		switching = -1;
	  		wmcnew = new WmcObject(prevObject());
		}
		
		if (clopt_window == WINDOW_CLASSIC) wmcnew->modifyZOffset(ZOFF_CLASSIC);
		wmcnew->setColorShading(clopt_fgcolor, clopt_shade);
		wmcnew->setMode(clopt_mode % NUM_MODES);
				
		return;
    }
}

/********************************************************************************

 bool checkMouseRegion(const WMCanvas *c, int button, int ulx, int uly, int lrx, int lry)

 ulx = upper left x, go figure on the rest..

********************************************************************************/

bool checkMouseRegion(const WMCanvas *c, int button, int ulx, int uly, int lrx, int lry)
{
	if ((c->app())->mouseclick().button != button) return false;
	if (((c->app())->mouseclick().relative_to(c).y > lry) || 
		((c->app())->mouseclick().relative_to(c).y < uly)) return false;
	if (((c->app())->mouseclick().relative_to(c).x < ulx) || 
		((c->app())->mouseclick().relative_to(c).x > lrx)) return false;
	
	return true;	
}

/********************************************************************************

 void displayWmcObject(const WMApp *a, WMWidget *w, void *)

 Timer-callback for drawing the object(s) in the canvas that is passed in w.

 Defines are for the scroll-in/out-speed of the objects (SCROLL_SPEED) as well 
 as the initial displacement from the center for the new object (INITIAL_DISP).

 Variable 'switching' take on values -1 (scrolling down), 0 (not switching)
 and 1 (scrolling up).

********************************************************************************/
#define SCROLL_SPEED 2.0
#define INITIAL_DISP switching * 2 * yoff

void displayWmcObject(const WMApp *a, WMWidget *w, void *)
{
  static float yd = 0.0;
  int local_yoff;
  static float mlight = 0.0;
  WMCanvas *c = dynamic_cast<WMCanvas *>(w);

  if (!c) return;

  clearCanvas(c);
  
  if (switching != 0)
    {
      /*
		We're currently switching objects - do alot of magic.
		Basically we're scrolling 'wmcnew' in from top/bottom
		while simultanously scrolling 'wmc' out. 
      */
      yd += SCROLL_SPEED;
      local_yoff = yoff - switching * (int)yd;
      
      wmc->rotate(ROT_SPEED_X, ROT_SPEED_Y, ROT_SPEED_Z);
      wmc->setYOffset(local_yoff);
      wmc->draw(c);

      wmcnew->setYOffset(yoff + INITIAL_DISP - switching * (int)yd);
      wmcnew->rotate(ROT_SPEED_X, ROT_SPEED_Y, ROT_SPEED_Z);
      wmcnew->draw(c);
      
      if (yd >= switching * INITIAL_DISP)
      	{
	  		// Finished switching, delete the old object
	  		switching = 0;
	  		yd = 0.0;
	  		mlight = 0.0;
	  		wmc = wmcnew;
	  		delete wmcold;
		}
    }
  else
    {
      wmc->rotate(ROT_SPEED_X, ROT_SPEED_Y, ROT_SPEED_Z);
            
      if (clopt_lsource)
		{
      	   wmc->setLightSource(55.0 * cos(mlight*0.002), 50.0 * sin(mlight*0.040), LSOURCE_Z_OFF);
           mlight += 1.0;
		}

      wmc->draw(c);
    }
  
  a->repaint();
}

/********************************************************************************

 void gatherStatistics(const WMApp *a, void *)

 Store the polled cpu-load in variable 'cpu_load'
 
********************************************************************************/
void gatherStatistics(const WMApp *a, void *)
{ 
  cpu_load = (int)cpumon->getLoad();
}

/********************************************************************************

 void displayCpuText(const WMApp *a, WMWidget *w, void *)'

 Display the cpu-load in the textbar passed in w.
 
********************************************************************************/
void displayCpuText(const WMApp *a, WMWidget *w, void *)
{
  char tmp[6];
  WMTextBar *b = dynamic_cast<WMTextBar*>(w);
  
  if (!b) return;

  sprintf(tmp, "%2d", cpu_load);
  b->settext(tmp);
  a->repaint();
}

/********************************************************************************

 void displayCpuMeter(const WMApp *a, WMWidget *w, void *)
 
 Display the cpu-load in the meterbar passed in w.
 
********************************************************************************/
void displayCpuMeter(const WMApp *a, WMWidget *w, void *)
{
  WMMeterBar *m = dynamic_cast<WMMeterBar *>(w);
  
  if (!m) return;
  
  m->setvalue(cpu_load, true);
  a->repaint();
}

/********************************************************************************

 char *nextObject()

 Returns the next objects' filename
 
********************************************************************************/
char *nextObject()
{
  if (obj_iterator == objects.end() - 1)
    obj_iterator = objects.begin();
  else
    obj_iterator++;

  //printf("returning next Object = %s\n", *obj_iterator); fflush(stdout);
  
  return *obj_iterator;
}

/********************************************************************************

 char *prevObject()

 Returns the previous objects' filename
 
********************************************************************************/
char *prevObject()
{
  if (obj_iterator == objects.begin())
    obj_iterator = objects.end() - 1;
  else
    obj_iterator--;
    
  //printf("returning previous Object = %s\n", *obj_iterator); fflush(stdout);

  return *obj_iterator;
}

/********************************************************************************

 int scanWmcDirectory(const char *dirname)

 Scans the directory 'dirname' and stores the full pathname of all found
 wmc-files in the global vector 'objects'. This code should be POSIX compliant
 which the old code was not (didnt work on Solaris for example). Objects are
 listed in some seemingly random order (creation time?).
 
********************************************************************************/
void scanWmcDirectory(const char *dirname)
{
  DIR *dir_stream = opendir(dirname);
  struct dirent *dirp;
  char *strp;

  if (dir_stream == NULL)
    return; //throw runtime_error("scanWmcDirectory: Could not open directory.");

  for (dirp = readdir(dir_stream); dirp != NULL; dirp = readdir(dir_stream))
    {
      if (strstr(dirp->d_name, ".wmc"))
		{
		  // Make sure there's space for the dirname, a slash, filename and terminating zero
		  strp = new char[strlen(dirname) + strlen(dirp->d_name) + 2];
		  strcpy(strp, dirname);
		  strcat(strp, "/");
		  strcat(strp, dirp->d_name);
		  objects.push_back(strp);
		  //printf("%s\n", objects[objects.size() - 1]);
		}
    }
  
  obj_iterator = objects.begin();
  closedir(dir_stream);
}

void clearCanvas(WMCanvas *canvas)
{
  canvas->setcolor((WMColor::WMColor)clopt_bgcolor);
  canvas->fill_rectangle(0, 0, 54, 54);
  //canvas->draw_vertical_gradient(0,0,53,40, 0x000000, 0x555555);
}
