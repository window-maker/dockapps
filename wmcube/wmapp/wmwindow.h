#include "wmframe.h"
#include "wmcallback.h"
#include "xwrapper.h"

#ifndef _WMWINDOW_H
#define _WMWINDOW_H

class WMApp;

// WMWindow: A 64x64 pixel square containing a number of widgets.  You can
// set it to run callback functions at regular intervals.  Only one WMWindow
// may be displayed at once.  This is controlled by the WMApp containing a
// vector of WMWindows.
class WMWindow : private WMCallback, public WMFrame {
 friend class WMApp;

 private:
  WMApp *		wApp;
  mutable WMPixmap	wPixmap;

  // how often to execute callbacks and update display, in milliseconds
  int		wUpdateFreq;
  int		wCounter;
  // list of periods of individual callbacks (in units of wUpdateFreq)
  vector<int>	wFuncPeriod;

  void initpixmaps();
  void real_display();
  void real_activate();
  void real_deactivate();

  // should be called only by a WMApp.
  void display();
  void hide();
  void activate();
  void deactivate();
  void run_timed_functions();

 public:
  WMWindow();
  ~WMWindow();

  const WMWindow * window() const; // returns "this"

  // override these to make them no-ops; windows have no parents
  WMFrame *parent();
  void setparent(const WMFrame *);
  void setparent(const WMFrame &);
  
  WMApp * app() const;
  int updatefreq() const;
  WMPixmap & pixmap() const;

  void setapp(WMApp *);

  void setupdatefreq(int milliseconds);
  void add_timed_function(int period, data_func d, void * = 0);
  void add_timed_function(int period, widget_func w, WMWidget *, void * = 0);
  void clear_timed_functions();

  bool press(int button, int x, int y);
  bool release(int button, int x, int y);
};

// inline functions for WMWindow -----------------------------------------

inline void
WMWindow::activate() { WMFrame::activate(); }

inline void
WMWindow::deactivate() { WMFrame::deactivate(); }

inline void
WMWindow::hide() { WMFrame::hide(); }

inline const WMWindow *
WMWindow::window() const { return this; }

inline WMFrame *
WMWindow::parent() { return 0; }

inline void
WMWindow::setparent(const WMFrame *) { }

inline void
WMWindow::setparent(const WMFrame &) { }

inline WMApp *
WMWindow::app() const { return wApp; }

inline int
WMWindow::updatefreq() const { return wUpdateFreq; }

inline void
WMWindow::setupdatefreq(int milliseconds) { wUpdateFreq = milliseconds; }

inline void
WMWindow::add_timed_function(int period, data_func f, void *datap)
{
  WMCallback::addcallback(f, datap);
  wFuncPeriod.push_back(period);
}

inline void
WMWindow::add_timed_function(int period, widget_func f, WMWidget *w,
		             void *datap)
{
  WMCallback::addcallback(f, w, datap);
  wFuncPeriod.push_back(period);
}

inline void
WMWindow::clear_timed_functions()
{
  WMCallback::clearcallbacks();
  wFuncPeriod.clear();
}
  
inline WMPixmap &
WMWindow::pixmap() const { return wPixmap; }

inline bool
WMWindow::press(int button, int x, int y)
{ return WMFrame::press(button, x, y); }

inline bool
WMWindow::release(int button, int x, int y)
{ return WMFrame::release(button, x, y); }

#endif
