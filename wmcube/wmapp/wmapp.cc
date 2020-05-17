#include <string>
#include "wmapp.h"
#include "wmwindow.h"

using std::string;

namespace Unix {
  extern "C" {
#   include <unistd.h> // for usleep()
  }
};

// All the xpms we need:
namespace Xpms {
# include "xpm/charmap-small.xpm"
# include "xpm/charmap-medium.xpm"
# include "xpm/charmap-large.xpm"
# include "xpm/checkbox.xpm"
# include "xpm/xbutton.xpm"
# include "xpm/leds.xpm"
# include "xpm/emptybar.xpm"
# include "xpm/fullbar.xpm"
# include "xpm/tile.xpm"
};

// Xlib doesn't seem to work if these are class members.
// They're declared as extern in xwrapper.h.
X::Window wActiveWin, wProgramWin;
X::Atom	deleteWin;
X::Atom	_XA_GNUSTEP_WM_FUNC;

// static class members of WMApp declared here:

Xwrapper WMApp::Xw;
WMPixmap WMApp::char_pixmaps[3];
WMPixmap WMApp::checkbox_pixmap, WMApp::xbutton_pixmap, WMApp::leds_pixmap[4];
WMPixmap WMApp::emptybar_pixmap, WMApp::fullbar_pixmap;
WMPixmap WMApp::tile_pixmap;
Color WMApp::colormap[WMColor::numcolors];

char ** WMApp::wArgv;
string WMApp::wName;
int WMApp::wArgc;
int WMApp::wWindowSize;
enum WindowManager::WindowManager WMApp::wManager;


// initialize: static function to create pixmaps and set default colors.
// This should be called as the very first line of a program using this
// library.
void
WMApp::initialize(int argc, char *argv[])
{
  static bool inited = false;
  if (inited) return;

  wArgc = argc;
  wName = (argc && argv) ? string(argv[0]) : string("WMApplication");
  wArgv = argv;
  deleteWin = X::XInternAtom(Xw.xdisplay(), "WM_DELETE_WINDOW", false);
  _XA_GNUSTEP_WM_FUNC = X::XInternAtom(Xw.xdisplay(),
		  		       "_GNUSTEP_WM_FUNCTION", false);
  
  // create all the required pixmaps
  Xw.create_pixmap(char_pixmaps[0],	Xpms::charmap_small_xpm);
  Xw.create_pixmap(char_pixmaps[1],	Xpms::charmap_medium_xpm);
  Xw.create_pixmap(char_pixmaps[2],	Xpms::charmap_large_xpm);
  Xw.create_pixmap(checkbox_pixmap,	Xpms::checkbox_xpm);
  Xw.create_pixmap(xbutton_pixmap,	Xpms::xbutton_xpm);
  Xw.create_pixmap(fullbar_pixmap,	Xpms::fullbar_xpm);
  Xw.create_pixmap(emptybar_pixmap,	Xpms::emptybar_xpm);
  Xw.create_pixmap(tile_pixmap,		Xpms::tile_xpm);

  // create four LED pixmaps
  WMPixmap all_leds_pixmap;
  int ledwidth = 4, ledheight = 4;
  
  Xw.create_pixmap(all_leds_pixmap, Xpms::leds_xpm);
  for (unsigned int s = 0; s < 4; s++) {
    leds_pixmap[s].attr.width = ledwidth;
    leds_pixmap[s].attr.width = ledheight;
    Xw.create_pixmap(leds_pixmap[s], ledwidth, ledheight);
    Xw.copy_rectangle(all_leds_pixmap, leds_pixmap[s], ledwidth * s, 0,
		     ledwidth, ledheight, 0, 0);
  }

# define colormap(x) colormap[static_cast<int>(WMColor:: x)]
  // create colormap
  // XXX: eventually, read from RC file
  colormap(Background)		= 0x212121;
  colormap(Dim)			= 0x283C28;
  colormap(Medium)		= 0x188A86;
  colormap(Bright)		= 0x20B2AE;
  colormap(ButtonFace)		= 0xAEAAAE;
  colormap(ButtonBorderDim)	= 0;
  colormap(ButtonBorderMedium)	= 0x86828E;
  colormap(ButtonBorderBright)	= 0xF7F3FF;
  colormap(FrameBorderDim)	= 0;
  colormap(FrameBorderMedium)	= 0;
  colormap(FrameBorderBright)	= 0xC8C8C8; //0xAEAAAE;
# undef colormap

  inited = true;

  // set the window behavior from command-line options
  wManager = WindowManager::Other;
  wWindowSize = 64;
  
  for (int i = 1; i < wArgc && strcmp(wArgv[i], "--"); i++) {
    if (strcmp(wArgv[i], "-w") == 0) {
      wManager = WindowManager::WindowMaker; break;
    }
    else if (strcmp(wArgv[i], "-a") == 0) {
      wManager = WindowManager::Afterstep;
      wWindowSize = 56;
      break;
    }
  }
}  

// Here begin the magic Xlib incantations required to create a dockapp window.
// I have no idea how this works, it just does.  Code mostly obtained from
// wmsmixer by Damian Kramer, based in turn upon wmmixer by Sam Hawker.

// simple function to create an XWindow	
void
WMApp::create_window(X::Window *dest, int left, int top) const
{
  X::XClassHint classHint;

  *dest = X::XCreateSimpleWindow(Xw.xdisplay(), Xw.xrootwin(), left, top,
				 wWindowSize, wWindowSize, 0, 0, 0);

  //XXX: May be able to get away without allocating new char arrays, but I don't
  //trust X not to modify them.
  classHint.res_name = new char[wName.size() + 1];
  classHint.res_class = new char[wName.size() + 1];
  strcpy(classHint.res_name, wName.c_str());
  strcpy(classHint.res_class, wName.c_str());
  X::XSetClassHint(Xw.xdisplay(), *dest, &classHint);
  delete[] classHint.res_name;
  delete[] classHint.res_class;
}

// This function sets up the X window.
// Called from WMApp::run()
void WMApp::Xsetup()
{
  X::Display *	display = Xw.xdisplay();
  
  X::XWMHints   wmhints;
  X::XSizeHints shints;
  int		winsize = wWindowSize;
  bool		pos;

  shints.x = shints.y = shints.flags = 0;
  pos = (X::XWMGeometry(display, X::XDefaultScreen(display), "" /*geometry*/,
			0, 0, &shints, &shints.x, &shints.y, &shints.width, &shints.height,
      &shints.win_gravity) & X_MACRO(XValue | YValue));
  shints.min_width = shints.min_height = winsize;
  shints.max_width = shints.max_height = winsize;
  shints.base_width = shints.base_height = winsize;
  shints.flags = X_MACRO(PMinSize | PMaxSize | PBaseSize);
  create_window(&wProgramWin, shints.x, shints.y);

  if (pos || wManager == WindowManager::WindowMaker
	|| wManager == WindowManager::Afterstep)
    shints.flags |= X_MACRO(USPosition);

  if (wManager == WindowManager::WindowMaker) {
    wmhints.initial_state = X_MACRO(WithdrawnState);
    wmhints.flags = X_MACRO(WindowGroupHint | StateHint | IconWindowHint);
    create_window(&wActiveWin, shints.x, shints.y);
    wmhints.icon_window = wActiveWin;
  }
  else {
    wmhints.initial_state = X_MACRO(NormalState);
    wmhints.flags = X_MACRO(WindowGroupHint | StateHint);
    wActiveWin = wProgramWin;
  }

  wmhints.window_group = wProgramWin;
  X::XSetWMHints(display, wProgramWin, &wmhints);
  X::XSetWMNormalHints(display, wProgramWin, &shints);
  if (wArgc > 0)
    X::XSetCommand(display, wProgramWin, wArgv, wArgc);
  X::XStoreName(display, wProgramWin, wName.c_str());
  X::XSetIconName(display, wProgramWin, wName.c_str());
  X::XSetWMProtocols(display, wActiveWin, &deleteWin, 1);
}

// This function masks out the "clear" parts of the X window.
// Called from WMWindow::real_display()
void
WMApp::mask() const
{
  X::Display * display = Xw.xdisplay();
  WMPixmap pixmap = current()->pixmap();

  if (wManager == WindowManager::WindowMaker ||
      wManager == WindowManager::Afterstep)
    // apply the mask if using a compliant window manager
    X::XShapeCombineMask(display, wActiveWin, X_MACRO(ShapeBounding),
		         0, 0, pixmap.mask, X_MACRO(ShapeSet));
  else
    // otherwise, create a tile background for the window
    Xw.copy_rectangle(tile_pixmap, pixmap.pixmap, 0, 0, wWindowSize,
        wWindowSize);

  Xw.fill_rectangle(pixmap, 0, 0, wWindowSize, wWindowSize,
      WMColor(Background));
}

// This function redraws the X window if necessary.
// Called from WMWindow::real_display()
void
WMApp::repaint() const
{
  X::XEvent xev;

  Xw.copy_rectangle(current()->pixmap(), wActiveWin,
                    0, 0, wWindowSize, wWindowSize);

  while (X::XCheckTypedEvent(Xw.xdisplay(), X_MACRO(Expose), &xev))
    /* loop */;
  X::XFlush(Xw.xdisplay());
}

// This function displays the X window.
// Called from WMWindow::real_activate()
void
WMApp::Xshow() const
{
  X::Display * display = Xw.xdisplay();
  
  // request input events and map the window
  X::XSelectInput(display, wActiveWin,
		  X_MACRO(ExposureMask | ButtonPressMask | ButtonReleaseMask)
		  | X_MACRO(ButtonMotionMask));
  X::XMapWindow(display, wProgramWin);
  repaint();
  
  // loop over X events and callbacks
  Xwait();	
}

// This function waits for X events, redraws the window when necessary,
// and calls callbacks of the window.
void
WMApp::Xwait() const
{
  X::Display * display = Xw.xdisplay();
  X::XEvent xev;

  while (true) {
    // sleep for the specified time in milliseconds
    Unix::usleep(1000 * current()->updatefreq());
    // execute any timed functions which need it
    current()->run_timed_functions();

    // execute any pending X events
    while (XPending(display)) {
      X::XNextEvent(display, &xev);

      switch (xev.type) {
        case Expose:
          repaint();
	  break;
	case ButtonPress:
	  current()->press(xev.xbutton.button, xev.xbutton.x, xev.xbutton.y);
	  break;
	case ButtonRelease:
	  wMouseEvent.button = xev.xbutton.button;
	  wMouseEvent.x = xev.xbutton.x;
	  wMouseEvent.y = xev.xbutton.y;
	  current()->release(xev.xbutton.button, xev.xbutton.x, xev.xbutton.y);
	  if (done()) return;
	  break;
	case ClientMessage:
	  if (static_cast<unsigned int>(xev.xclient.data.l[0]) == deleteWin)
	    { stop(); return; }
	  break;
	default: break;
      }
    }
  }
}

// End of magic Xlib incantations.

// The following functions deal with setting the state of the application
// and running it.

void
WMApp::switch_to(WMWindow *w) const
{
  for (unsigned int i = 0; i < wWindows.size(); i++)
    if (w == wWindows[i]) {
      switch_to(i);
      return;
    }
}

void
WMApp::switch_to(WMWindow &w) const { switch_to(&w); }

// run: This function starts the GUI of the dockapp.  Call it as the
// last line in a program using this library.  Calling run() with no
// argument starts the program on the first window.  You can also
// call run(WMWindow *), run(WMWindow &), or run(unsigned int) in order
// to start with a different window.
//
// So your program in the simplest case should look like this:
//
// int main(int argc, char *argv[]) {
//   WMApp::initialize(argc, argv);
//   WMWindow w;
//   WMApp a = WMApp(w);
//   // layout the window and set up callbacks here
//   // ...
//   a.addwindow(w);
//   a.run();
//   return 0;
// }

void
WMApp::run(WMWindow *w)
{
  for (unsigned int i = 0; i < wWindows.size(); i++)
    if (w == wWindows[i]) {
      run(i);
      return;
    }
}

void
WMApp::run(WMWindow &w) { run(&w); }

void
WMApp::run(int w) { run(static_cast<unsigned int>(w)); }

void
WMApp::run(unsigned int w)
{
  if (w >= wWindows.size()) return;

  // set up pointers from windows to "this"
  for (unsigned int i = 0; i < wWindows.size(); i++)
    wWindows[i]->wApp = this;
  
  Xsetup();
  wActiveWindow = wNextWindow = w;

  while (wFinished == false) {
    // This function does not exit until wNextWindow != wActiveWindow
    // or wFinished == true.  User interaction happens in here.
    current()->display();

    // To switch to a different window, set a callback on a button within
    // this window, like this:
    // 	void callback1(const WMApp *a, void *) { a->switch_to(5); }
    // To exit the program, likewise, you need a callback like
    // 	void callback2(const WMApp *a, void *) { a->stop(); }
    // (Alternately you could put the window on a timer by setting one of
    // these as a callback of the window itself, substituting WMWindow for
    // WMButton in the function definitions.)

    wActiveWindow = wNextWindow;

    // if we have switched to a different window, continue with that one.
  }

  // otherwise, exit the WMApp::run() function.  (Normally a call like
  // wm_app->run(); will be the last in a program's main() function,
  // in which case we also exit the program.)
  return;
}

