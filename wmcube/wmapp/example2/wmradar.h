#ifndef _WMRADAR_H
#define _WMRADAR_H

#include <list>
#include "../wmellipse.h"
#include "../wmcanvas.h"
#include "../wmcallback.h"
#include "../wmtextbar.h"
#include "../wmslider.h"

class WMRadar : public WMCanvas, public WMCallback, public WMEllipse {
private:
  struct plane
  {
    double x, y, direction, brightness;
  };
  typedef std::list<plane> plist;
  
  struct collision
  { int x, y; };
  typedef std::list<collision> clist;
  
  plist planes;
  clist collisions;
  double angle;

  WMTextBar* text_score;
  WMSlider* speed;

protected:
  void real_display();

public:
  WMRadar(WMTextBar*, WMSlider*);
  
  void add_plane();
  bool release(int button, int x, int y); //change nearest plane's direction
  void increment_time();

private:
  void increment_score();
};

#endif  //_WMRADAR_H
