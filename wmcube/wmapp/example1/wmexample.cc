#include "../wmapp.h"
#include "../wmwindow.h"

// This is an example WindowMaker dockapp program written using the
// WMApp dockapp library.  The first window shows off various widgets
// and demonstrates how to make a clock.  The second window illustrates
// a miniature paint program.

extern void makewindow0(WMWindow *w0_ptr); // see window0.cc
extern void makewindow1(WMWindow *w1_ptr); // see window1.cc

// The main() function, where we lay out all the widgets -------------------
// The files window0.cc and window1.cc are much more interesting.

int main(int argc, char *argv[])
{
  // This should always be the very first line in the main() function of
  // a WMApplication:
  WMApp::initialize(argc, argv);

  WMApp a;
  WMWindow w0, w1;
  makewindow0(&w0);
  makewindow1(&w1);

  // Attach windows to the application
  a.addwindow(w0);
  a.addwindow(w1);

  // This makes everything go.
  a.run();

  // Any cleanup code after the window closes should go here.
  // (In this case, none is needed.)
  
  return 0;
}


