#include <vector>
#include <string>
#include "wmwidget.h"
#include "xwrapper.h"
#include "colors.h"

#ifndef _WMAPP_H
#define _WMAPP_H

class WMWindow;

namespace WindowManager {
  enum WindowManager { WindowMaker, Afterstep, Other };
};

struct WMMouseClick {
  int button;
  int x;
  int y;

  WMMouseClick(int Button = Button1, int X = 0, int Y = 0);
  WMMouseClick relative_to(const WMWidget &w) const;
  WMMouseClick relative_to(const WMWidget *w) const;
  WMMouseClick b_relative_to(const WMWidget &w) const;
  WMMouseClick b_relative_to(const WMWidget *w) const;
};

inline WMMouseClick::WMMouseClick(int Button, int X, int Y)
: button(Button), x(X), y(Y) { }

inline WMMouseClick
WMMouseClick::relative_to(const WMWidget &w) const
{ return WMMouseClick(button, x - w.left(), y - w.top()); }

inline WMMouseClick
WMMouseClick::relative_to(const WMWidget *w) const
{ return relative_to(*w); }

inline WMMouseClick
WMMouseClick::b_relative_to(const WMWidget &w) const
{ return WMMouseClick(button, x - w.b_left(), y - w.b_top()); }

inline WMMouseClick
WMMouseClick::b_relative_to(const WMWidget *w) const
{ return b_relative_to(*w); }

class WMApp {
 private:
  static int		wWindowSize;
  static int		wArgc;
  static char **	wArgv;
  static std::string		wName;

  std::vector<WMWindow *>	wWindows;
  mutable int		wActiveWindow;
  mutable int		wNextWindow;
  mutable bool		wFinished;
  mutable WMMouseClick	wMouseEvent;
  
  void create_window(X::Window *dest, int left, int top) const;
  void Xsetup();
  void Xwait() const;
	
 public:
  static WindowManager::WindowManager wManager;
  static Xwrapper Xw;
  static WMPixmap char_pixmaps[3];
  static WMPixmap checkbox_pixmap, xbutton_pixmap, leds_pixmap[4];
  static WMPixmap emptybar_pixmap, fullbar_pixmap;
  static WMPixmap tile_pixmap;
  static Color colormap[WMColor::numcolors];

  // Always call this function first in main():
  static void initialize(int argc = 0, char *argv[] = 0);
  static unsigned int size();
  
  WMApp();
  ~WMApp();

  void addwindow(WMWindow *);
  void addwindow(WMWindow &);

  // Use this call to start the GUI.
  void run(unsigned int window = 0);
  void run(int window);
  void run(WMWindow &);
  void run(WMWindow *);

  // Use these within callbacks of windows or buttons in order to switch
  // to a different window, or to exit the GUI.
  void switch_to(unsigned int window) const;
  void switch_to(int window) const;
  void switch_to(WMWindow &) const;
  void switch_to(WMWindow *) const;
  void stop() const;

  // Use this within callbacks of windows or buttons in order to play
  // with other widgets.
  WMWindow * window(unsigned int) const;
  WMWindow * current() const;
  unsigned int currentnum() const;

  // Use this within callbacks to see which mouse button was pressed
  // and where.
  const WMMouseClick & mouseclick() const;

  // Windows use this call to see if it's time for them to exit yet.
  bool done() const;

  // utility functions for windows
  void mask() const;
  void repaint() const;
  void Xshow() const;
};

inline
WMApp::WMApp()
: wActiveWindow(0), wNextWindow(0), wFinished(false) { }

inline
WMApp::~WMApp()
{
  if (wProgramWin == wActiveWin)
    X::XDestroyWindow(Xw.xdisplay(), wProgramWin);
  else {
    X::XDestroyWindow(Xw.xdisplay(), wProgramWin);
    X::XDestroyWindow(Xw.xdisplay(), wActiveWin);
  }
}

inline void
WMApp::addwindow(WMWindow *w) { wWindows.push_back(w); }

inline void
WMApp::addwindow(WMWindow &w) { wWindows.push_back(&w); }

inline unsigned int
WMApp::size() { return wWindowSize; }

inline WMWindow *
WMApp::window(unsigned int i) const
{ return i < wWindows.size() ? wWindows[i] : 0; }

inline WMWindow *
WMApp::current() const { return wWindows[wActiveWindow]; }

inline unsigned int
WMApp::currentnum() const { return wActiveWindow; }

inline const WMMouseClick &
WMApp::mouseclick() const { return wMouseEvent; }

inline void
WMApp::switch_to(unsigned int window) const
{ if (window < wWindows.size()) wNextWindow = window; }

inline void
WMApp::switch_to(int window) const
{
  if (window >= 0 && window < static_cast<int>(wWindows.size()))
    wNextWindow = window;
}

inline void
WMApp::stop() const { wFinished = true; }

inline bool
WMApp::done() const { return wFinished || wActiveWindow != wNextWindow; }

#endif
