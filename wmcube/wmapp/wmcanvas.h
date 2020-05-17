#include <vector>
#include "wmimage.h"

#ifndef _WMCANVAS_H
#define _WMCANVAS_H

using std::vector;

// WMCanvas: a widget inheriting from WMImage that allows you to change the
// image using drawing functions
class WMCanvas : public virtual WMImage {
  Color wCurrentColor;
  bool wBuffered;
 public:
  WMCanvas(const WMPixmap * pm = 0);
  WMCanvas(const WMPixmap & pm);
  WMCanvas(char *xpm[]);

  void setcolor(Color);
  void setbuffered(bool);
  Color color() const;
  bool buffered() const;
  
  // these are all variants of the drawing functions in xwrapper.h.
  // Unlike there, they are relative to the borders of the widget.
  Color get_point(int rel_x, int rel_y) const;
  void draw_point(int rel_x, int rel_y);
  void draw_line(int rel_x1, int rel_y1, int rel_x2, int rel_y2);
  void draw_arc(int x, int y, int width, int height, int angle1, int angle2);
      //angles are in units of degrees * 64
  void draw_lines(const vector<X::XPoint>& points);
      //X::XPoint is simply a struct { short x, y };
  void draw_horizontal_gradient(int rel_x1, int rel_y1, int rel_x2, int rel_y2,
		  		Color c1, Color c2, double amount = 1.0);
  void draw_vertical_gradient(int rel_x1, int rel_y1, int rel_x2, int rel_y2,
		  	      Color c1, Color c2, double amount = 1.0);
  void empty_rectangle(int rel_x, int rel_y, int width, int height,
		       int thickness);
  void empty_rectangle(const WMRectangle & rel_posn, int thickness);
  void fill_rectangle(int rel_x, int rel_y, int width, int height);
  void fill_rectangle(const WMRectangle & rel_posn);
  void fill_arc(int x, int y, int width, int height, int angle1, int angle2);
      //angles are in units of degrees * 64
  void fill_polygon(const vector<X::XPoint>& points,
      Xwrapper::XShape shape = Xwrapper::complex);
      //see xwrapper.h for the meaning of shape
  void copy_rectangle(const WMPixmap & source, int source_x, int source_y,
		      int source_w, int source_h,
		      int dest_x = 0, int dest_y = 0);
  void copy_rectangle(const WMPixmap & source, const WMRectangle & posn,
		      int dest_x = 0, int dest_y = 0);
};

// inline functions for WMCanvas -----------------------------------------

inline void
WMCanvas::setcolor(Color c) { wCurrentColor = c; }

inline void
WMCanvas::setbuffered(bool b) { wBuffered = b; }

inline Color
WMCanvas::color() const { return wCurrentColor; }

inline bool
WMCanvas::buffered() const { return wBuffered; }

#endif
