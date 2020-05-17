#include "wmwidget.h"

#ifndef _WMMETER_H
#define _WMMETER_H

// WMMeter: Base class for widgets that display a state of progress or load.
// Cannot be instantiated (inherits the pure virtual function real_display).
class WMMeter : public virtual WMWidget {
 private:
  int wValue, wTotal;
  
 public:
  WMMeter();
  WMMeter(int total);
  WMMeter(int value, int total);
  
  virtual void setvalue(int value, bool dodisplay = true);
  virtual void settotal(int total, bool dodisplay = true);

  int value() const;
  int total() const;
  double fraction() const;
};

// inline functions for WMMeter ------------------------------------------

inline WMMeter::WMMeter()
: wValue(0), wTotal(0) { }

inline WMMeter::WMMeter(int total)
: wValue(0), wTotal(total) { }

inline WMMeter::WMMeter(int val, int total)
: wValue(val), wTotal(total) { }

inline void
WMMeter::setvalue(int val, bool dodisplay)
{ wValue = val; if (dodisplay) display(); }

inline void
WMMeter::settotal(int total, bool dodisplay)
{ wTotal = total; if (dodisplay) display(); }

inline int
WMMeter::value() const { return wValue; }

inline int
WMMeter::total() const { return wTotal; }

inline double
WMMeter::fraction() const
{
  if (wTotal == 0) return 0;
  else return static_cast<double>(wValue) / wTotal;
}		  

#endif
