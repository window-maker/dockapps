#include <iostream>
#include "wmwindow.h"
#include "wmapp.h"

using std::cerr;
using std::endl;

// functions for WMWindow ------------------------------------------------

// this is here instead of inlined due to dependency on WMApp
WMWindow::WMWindow()
: WMFrame(), wApp(0), wUpdateFreq(10 /* milliseconds */),
  wCounter(0), wFuncPeriod()
{ 
  setpadding(2);
  setborder(0);

  pixmap().pixmap = 0;
  //use this as a flag determining whether the pixmaps are initialized

  int offset = WMApp::size() / 2 - 28;
  setposition(WMRectangle(offset, offset, 56, 56));
} 

WMWindow::~WMWindow ()
{
  if (pixmap().pixmap) { //only free if it was actually allocated
    WMApp::Xw.free_pixmap(pixmap());
  }
}

void
WMWindow::run_timed_functions()
{
  for (unsigned int i = 0; i < numcallbacks(); i++)
    if (! (wCounter % wFuncPeriod[i])) {
      runcallback(i);
    }
  wCounter++;
}

void
WMWindow::initpixmaps()
{
  // Xlib has no facility to create a mask on the fly from an xpm (at least
  // not that I know about), so we have to do it the hard way, initializing a
  // mask from a char* array:

  char *mask_xpm[67];
  for (unsigned int i = 0; i < 67; i++)
    mask_xpm[i] = new char[65];

  strcpy(mask_xpm[0], "64 64 2 1");		// width, height, #colors, (?)
  strcpy(mask_xpm[1], "       c None");		// transparent
  strcpy(mask_xpm[2], "X      c #000000");	// black
  for (unsigned int i = 3; i < 67; i++)
    strcpy(mask_xpm[i], // string with 64 spaces --v
      "                                                                ");
  //   0123456789012345678901234567890123456789012345678901234567890123
  //             1         2         3         4         5         6
  
  // now mask out where we want to display things
  for (unsigned int i = 0; i < numchildren(); i++)
    child(i)->clip(mask_xpm);

  // finally, create the pixmap and mask
  WMApp::Xw.create_pixmap(pixmap(), mask_xpm);
  WMApp::Xw.fill_rectangle(pixmap(), 0, 0, 64, 64, 0xFFFFFF /*white*/);

  // clean up
  for (unsigned int i = 0; i < 67; i++)
    delete [] mask_xpm[i];
}

void
WMWindow::display()
{
  // obviously, we replace the check for existence of a parent
  // with check for existence of an app
  if (!app()) {
    cerr << "WMError: Window " << this << "has no parent application" << endl;
    return;
  }

  real_display();
  setdisplayed(true);
  for (unsigned int i = 0; i < numchildren(); i++)
    child(i)->display();
  
  // show the window and start running the GUI and callbacks
  app()->Xshow();

  // don't get here until the GUI has been requested to switch to 
  // a different window (via a callback), or to exit
  return;
}

void
WMWindow::real_display()
{
  if (!wPixmap.pixmap)
    initpixmaps(); //call this function only once
  
  // apply the mask to the application window so that areas not included
  // in any widget are clear to the icon tile below
  app()->mask();
}

void
WMWindow::real_activate() { display(); }

void
WMWindow::real_deactivate() { hide(); }

