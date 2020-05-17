#include "wmcallback.h"
#include "wmmeterbar.h"

#ifndef _WMSLIDER_H
#define _WMSLIDER_H

// WMSlider: a bar that can be slid up or down to set a value.
// Derived from WMMeterBar and WMCallback.
class WMSlider : public WMCallback, public WMMeterBar {
 private:
  void attach_callbacks();
  
 public:
  WMSlider();
  WMSlider(int total);
  WMSlider(int value, int total);

  void clearcallbacks();
};

inline WMSlider::WMSlider()
{ setstyle(Blue, false); attach_callbacks(); }

inline WMSlider::WMSlider(int total)
: WMMeterBar(total)
{ setstyle(Blue, false); attach_callbacks(); }

inline WMSlider::WMSlider(int value, int total)
: WMMeterBar(value, total)
{ setstyle(Blue, false); attach_callbacks(); }

#endif
