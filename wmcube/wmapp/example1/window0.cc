#include "../wmapp.h"
#include "../wmframe.h"
#include "../wmwindow.h"
#include "../wmtextbar.h"
#include "../wmslider.h"
#include "../wmled.h"
#include "../wmmeterbar.h"
#include "../wmhistory.h"
#include "../wmbutton.h"
#include "../wmellipse.h"

#include "../xpm/checkbox.xpm"
#include "../xpm/xbutton.xpm"
#include "debian-tiny.xpm"

// for the clock:
#include <time.h>
#include <sys/timeb.h>

// This file defines a window that contains various widgets to demonstrate
// the WMApp library.  Also it is shown how to make a clock widget.

// Callbacks for the buttons -----------------------------------------------

// This one ends the program (always useful) and will be attached to the
// button with the "X".
void
halt(const WMApp *a, void *) { a->stop(); }

// The following three callbacks will be attached to the checkbox button:

// This one changes the state of the middle LED if mousebutton 1 is clicked.
void
ledset(const WMApp *a, WMWidget *w, void *)
{
  if (a->mouseclick().button != Button1) return;

  WMLed *l = dynamic_cast<WMLed *>(w);
  if (l) {
    l->setled(static_cast<WMLed::LedState>((1 + l->led()) % 4));
    a->repaint();
  }
}

// This one toggles the style of the vertical meter if button 2 is clicked.
void
togglegraphstyle (const WMApp *a, WMWidget *w, void *)
{
  if (a->mouseclick().button != Button2) return;
	
  WMMeterBar *m = dynamic_cast<WMMeterBar *>(w);
  if (m) {
    m->setstyle(static_cast<WMMeterBar::Style>((1 + m->style()) % 3));
    m->setorientation((m->orientation() == Orientation::Horizontal) ? 
      Orientation::Vertical : Orientation::Horizontal);
    a->repaint();
  }
}

// This one toggles whether the "close" button can be pressed if mouse
// button 3 is clicked.
void
toggleclose(const WMApp *a, WMWidget *w, void *)
{
  if (a->mouseclick().button != Button3) return;
	
  WMButton *b = dynamic_cast<WMButton *>(w);
  if (b && b->is_active()) b->deactivate();
  else if (b && !b->is_active()) b->activate();
}

// This callback will be attached to the Debian button.  It switches
// between windows.
// Note: if the last statement in a callback function requests
// a switch to a new window, you don't need a "repaint()".
void
switch_to_1(const WMApp *a, void *)
{ a->switch_to(1); }

// Define a class that will be a WMHistory widget with callbacks
class WMHistCallback : public WMCallback, public WMHistory, public WMEllipse {
 public: WMHistCallback() : WMCallback(), WMHistory(), WMEllipse() { }
};

// Callback to clear this widget when it's clicked on
void
clearhistory(const WMApp *a, WMWidget *w, void *)
{
  WMHistCallback *h = dynamic_cast<WMHistCallback *>(w);
  if (h) h->clear();
}
     
// Callbacks for the windows -----------------------------------------------
// Window callbacks are executed every 50 milliseconds (by default; you
// can set this value) for the currently displayed window.

// This function will update the time on the clock display every 5 seconds.
void
displaytime(const WMApp *a, WMWidget *w, void *)
{
  struct timeb currenttime;
  string timestr;
  WMTextBar *b = dynamic_cast<WMTextBar *>(w);
  
  if (!b) return;
  ftime(&currenttime);
  // extract current time from date-and-time string
  timestr = string(ctime(&currenttime.time) + 11);
  b->settext(timestr);
  a->repaint();
}

// make random increments to the meterbar and history every 150 ms
void
updatehist (const WMApp *a, WMWidget *w1, void *w2)
{
  WMMeterBar *b = dynamic_cast<WMMeterBar *>(w1);
  if (b) {
    int increment = static_cast<int>(b->value() - 13 
		    + 31 * static_cast<float>(rand()) / RAND_MAX) + 307;
    //I added 307 becuase gcc does bad things to a signed dividend in a modulus,
    //which makes for *interesting* results
 
    b->setvalue(increment % 307, 306);
  }
  
  WMHistory *h = dynamic_cast<WMHistory *>(static_cast<WMWidget *>(w2));
  if (b && h) h->setvalue(b->value());
  a->repaint();
}
 
// The function where we lay out all the widgets ---------------------------

void
makewindow0(WMWindow *w0)
{
  // Everything following is GUI boilerplate code.
  static WMFrame top, mid, bottom, topright, botright;
  static WMTextBar time("00:00", 0);
  static WMSlider slider(30, 50);
  static WMLed l, m, r;
  static WMMeterBar meterbar;
  static WMHistCallback history;
  static WMButton debian, go, stop;
  
  w0->add_timed_function(500, displaytime, &time, 0);
  w0->add_timed_function(15, updatehist, &meterbar,
		         dynamic_cast<WMWidget *>(&history));
  w0->addchild(top);
  w0->addchild(mid);
  w0->addchild(bottom);
  w0->setorientation(Orientation::Vertical);
  w0->setaspectratios(1, 2, 1);

  top.addchild(time);
  top.addchild(topright);
  top.setaspectratios(6, 5);

  mid.addchild(history);
  mid.addchild(slider);
  slider.setorientation(Orientation::Vertical);
  mid.setaspectratios(3, 1);
  
  bottom.addchild(meterbar);
  bottom.addchild(botright);
  bottom.setaspectratios(1, 2);
  
  topright.addchild(l);
  topright.addchild(m);
  topright.addchild(r);
  topright.setpadding(0);
  topright.setborder(1);

  botright.addchild(debian);
  botright.addchild(go);
  botright.addchild(stop);
  botright.setpadding(0);
  botright.setborder(1);
  
  l.setled(WMLed::Good);
  r.setled(WMLed::Error);

  // should set initial total of seconds
  srand(std::time(0));
  meterbar.setstyle(WMMeterBar::Spectrum);
  meterbar.setvalue(150, false);
  meterbar.settotal(306, false);
  history.settotal(306, false);
  history.addcallback(clearhistory, &history, 0);
  
  debian.seticon(debian_tiny_xpm);
  debian.addcallback(switch_to_1, 0);
  
  go.seticon(checkbox_xpm);
  go.addcallback(ledset, &m, 0);
  go.addcallback(toggleclose, &stop, 0);
  go.addcallback(togglegraphstyle, &meterbar, 0);
  
  // Let's start this button out in an inactive state, just for fun.
  stop.deactivate();
  stop.seticon(xbutton_xpm);
  stop.addcallback(halt, 0);
}

