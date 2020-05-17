#include <cmath>
#include "wmellipse.h"
#include "wmwindow.h"
#include "wmapp.h"

// functions for WMEllipse -----------------------------------------------

void
WMEllipse::draw_border()
{ WMEllipse::draw_border(top_left_c(), bottom_right_c(), 5 /* steps */); }

void
WMEllipse::draw_border(Color c1, Color c2, int steps)
{
  if (border()) {
    X::XGCValues gcv;
    gcv.line_width = border();
    X::XChangeGC(WMApp::Xw.xdisplay(), window()->pixmap().gc,
        X_MACRO(GCLineWidth), &gcv);

    // These dimensions are all experimentally determined :)
    int l = left() + (border() - 1) / 2,
        t = top() + (border() - 1) / 2,
        w = width() - border() + (1 - border() % 2),
        h = height() - border() + (1 - border() % 2);

    const double weight_inc = 1.0 / (steps + 1);
    const double angle_inc = 45 / steps;
    for (double weight = weight_inc, angle = 0; angle < 44;
        weight += weight_inc, angle += angle_inc)
    {
      Color c = Color::alpha_blend(c1, c2, weight);
      WMApp::Xw.draw_arc(window()->pixmap(), l, t, w, h, (int)(angle * 64),
          (int)(angle_inc * 64), c);
      WMApp::Xw.draw_arc(window()->pixmap(), l, t, w, h, (int)((270 - angle) * 64),
          (int)(-angle_inc * 64), c);
    }
      
    WMApp::Xw.draw_arc(window()->pixmap(), l, t, w, h, 45 * 64, 180 * 64, c1);
    WMApp::Xw.draw_arc(window()->pixmap(), l, t, w, h, 270 * 64, 90 * 64, c2);
    
    gcv.line_width = 0;
    X::XChangeGC(WMApp::Xw.xdisplay(), window()->pixmap().gc,
        X_MACRO(GCLineWidth), &gcv);
  }
}

bool
WMEllipse::contains(int x, int y) const
{
  double h_axis = std::ldexp(double(width()), -1);
  double v_axis = std::ldexp(double(height()), -1);
  double xc = (h_axis + left() - 0.5 - x) / h_axis;
  xc *= xc;
  double yc = (v_axis + top() - 0.5 - y) / v_axis;
  yc *= yc;
  return ((xc + yc) <= 1.0);
}
