#include "wmimage.h"

#ifndef _WMLED_H
#define _WMLED_H

// WMLed: displays a round LED-type thing with a different color depending on
// its status.
class WMLed : public virtual WMImage {
 public:
  enum LedState { Off, Good, Warning, Error };

 private:
  enum LedState wState;
  void real_display();
 
 public:
  WMLed(LedState s = Off);
  void setled(enum LedState s, bool dodisplay = true);
  enum LedState led() const;
};

// inline functions for WMLed --------------------------------------------

inline WMLed::WMLed(enum LedState s)
: wState(s) { setborder(0); }

inline void 
WMLed::setled(enum LedState s, bool dodisplay)
{ wState = s; if (dodisplay) display(); }

inline enum WMLed::LedState
WMLed::led() const { return wState; }

#endif
