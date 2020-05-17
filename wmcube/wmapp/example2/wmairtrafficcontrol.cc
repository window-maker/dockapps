#include <ctime>
#include <cstdlib>
#include "wmradar.h"
#include "../wmslider.h"
#include "../wmcallback.h"
#include "../wmapp.h"
#include "../wmwindow.h"
#include "../wmframe.h"
#include "../wmbutton.h"
#include "../xpm/xbutton.xpm"

//WM Air Traffic Control: The FAQ said don't rely on callback timings for
//air traffic control, so I did.
//
//Click on the radar to change the nearest plane's heading towards that point.
//Move the slider to change the planes' speed.
//If two planes collide, they will disappear, a red point will appear, and
//the score will be incremented.

void
halt(const WMApp *a, void *) { a->stop(); }

void add_plane(const WMApp* a, WMWidget* w, void*)
{
  if (!(std::rand() % 3))
  {
    WMRadar* r = dynamic_cast<WMRadar*>(w);
    if (r)
      r->add_plane();
  }
}

void increment_time(const WMApp* a, WMWidget* w, void*)
{
  WMRadar* r = dynamic_cast<WMRadar*>(w);
  if (r)
    r->increment_time();
  a->repaint();
}

int main(int argc, char *argv[])
{
  WMApp::initialize(argc, argv);

  WMApp a;
  WMWindow win;

  WMTextBar score;
  WMSlider speed;
  WMRadar radar(&score, &speed);
  WMFrame left, right;
  WMButton quit;
  
  win.add_timed_function(1, increment_time, &radar, 0);
  win.add_timed_function(80, add_plane, &radar, 0);
  win.addchild(left);
  win.addchild(right);
  win.setaspectratios(50, 15);

  left.addchild(radar);
  left.addchild(score);
  left.setorientation(Orientation::Vertical);
  left.setaspectratios(50, 15);

  right.addchild(speed);
  right.addchild(quit);
  right.setorientation(Orientation::Vertical);
  right.setaspectratios(50, 15);

  quit.seticon(xbutton_xpm);
  quit.addcallback(halt, 0);

  speed.setorientation(Orientation::Vertical);

  std::srand(std::time(0));
  
  a.addwindow(win);
  a.run();
  return 0;
}
