#include "wmwidget.h"

#ifndef _WMELLIPSE_H
#define _WMELLIPSE_H

// WMEllipse: Widgets inherited from this class will be displayed within an
// elliptical border. Simply inherit from WMEllipse and the desired widget.
// Note: In order to get a circle, you must make the widget's width the same
// as its height, for example by placing it inside a WMFrame with padding.
class WMEllipse : public virtual WMWidget {
 protected:
  virtual void	  draw_border(Color c1, Color c2, int nsteps);
  virtual void    draw_border();

 public:
  bool contains(int x, int y) const;

  virtual ~WMEllipse() { }
};

#endif
