#include "wmmeter.h"

#ifndef _WMMETERBAR_H
#define _WMMETERBAR_H

// WMMeterBar: a progress bar showing the instantaneous progress or load
// added by Jason
class WMMeterBar : public virtual WMMeter {
 public:
  // The styles correspond to those in wmmon, wmtop, and wmsmixer respectively: 
  // green-to-red bar on dark green-to-red background; likewise on dark
  // colorless background; and turquoise hashmarks on dark background.
  enum Style { Spectrum, Spec_No_BG, Blue };

 private:
  enum Style wStyle;

 protected:
  void real_display();

 public:
  WMMeterBar();
  WMMeterBar(int total);
  WMMeterBar(int value, int total);
    
  void setstyle(enum Style, bool dodisplay = true);
  enum Style style() const;
};

// inline functions for WMMeterBar ---------------------------------------

inline WMMeterBar::WMMeterBar() : WMMeter() { }

inline WMMeterBar::WMMeterBar(int total) : WMMeter()
{ settotal(total); }

inline WMMeterBar::WMMeterBar(int val, int total) : WMMeter()
{ settotal(total); setvalue(val); }

inline void
WMMeterBar::setstyle(enum Style s, bool dodisplay)
{ wStyle = s; if (dodisplay) display(); }

inline enum WMMeterBar::Style
WMMeterBar::style() const { return wStyle; }

#endif
