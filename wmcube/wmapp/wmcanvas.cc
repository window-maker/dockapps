#include <vector>
#include "wmcanvas.h"
#include "wmwindow.h"
#include "wmapp.h"

// functions for WMCanvas ------------------------------------------------

WMCanvas::WMCanvas(const WMPixmap * pm)
: WMImage(pm), wCurrentColor(WMColor(Bright)), wBuffered(false) { }

WMCanvas::WMCanvas(const WMPixmap & pm)
: WMImage(pm), wCurrentColor(WMColor(Bright)), wBuffered(false) { }

WMCanvas::WMCanvas(char *xpm[])
: WMImage(xpm), wCurrentColor(WMColor(Bright)), wBuffered(false) { }

// these are all variants of the drawing functions in xwrapper.h.
// Unlike there, they are relative to the borders of the widget.
// Note that icon() should return an XPM which is the dimension of
// the WMCanvas _minus_ the border thickness.

/*void
WMCanvas::copy_buffer()
{
  WMApp::Xw.copy_rectangle(*icon(), window()->pixmap(), 0, 0,
		           b_width(), b_height(), b_left(), b_top());
//  draw_border();
}*/

// don't use this macro in your programs, it's just to make my life easier in
// the WMCanvas method definitions
#define COPY_BUFFER() if (! buffered()) { display() /*copy_buffer()*/; }

Color
WMCanvas::get_point(int rel_x, int rel_y) const
{ return WMApp::Xw.get_point(*icon(), rel_x, rel_y); }

void
WMCanvas::draw_point(int rel_x, int rel_y)
{
  WMApp::Xw.draw_point(*p_icon(), rel_x, rel_y, color());
  COPY_BUFFER();
}

void
WMCanvas::draw_line(int rel_x1, int rel_y1, int rel_x2, int rel_y2)
{
  WMApp::Xw.draw_line(*p_icon(), rel_x1, rel_y1, rel_x2, rel_y2, color());
  COPY_BUFFER();
}

void
WMCanvas::draw_arc(int x, int y, int width, int height, int angle1, int angle2)
{
  WMApp::Xw.draw_arc(*p_icon(), x, y, width, height, angle1, angle2, color());
  COPY_BUFFER();
}

void
WMCanvas::draw_lines(const vector<X::XPoint>& points)
{
  X::XPoint *p = new X::XPoint[points.size()];
  for (size_t i = 0; i < points.size(); ++i)
  {
    p[i].x = points[i].x;
    p[i].y = points[i].y;
  }
  WMApp::Xw.draw_lines(*p_icon(), p, points.size(), color());
  delete[] p;
  COPY_BUFFER();
}

void
WMCanvas::draw_horizontal_gradient(int rel_x1, int rel_y1, int rel_x2,
				   int rel_y2, Color c1, Color c2, double amt)
{
  WMApp::Xw.draw_horizontal_gradient(*p_icon(), rel_x1, rel_y1, rel_x2, rel_y2,
				     c1, c2, amt);
  COPY_BUFFER();
}

void
WMCanvas::draw_vertical_gradient(int rel_x1, int rel_y1, int rel_x2,
				 int rel_y2, Color c1, Color c2, double amt)
{
  WMApp::Xw.draw_vertical_gradient(*p_icon(), rel_x1, rel_y1, rel_x2, rel_y2,
				   c1, c2, amt);
  COPY_BUFFER();
}

void
WMCanvas::empty_rectangle(int rel_x, int rel_y, int w, int h, int thickness)
{
  WMApp::Xw.draw_border(*p_icon(), rel_x, rel_y, w, h, thickness,
		        color(), color(), color());
  COPY_BUFFER();
}

void
WMCanvas::empty_rectangle(const WMRectangle & rel_posn, int thickness)
{
  WMApp::Xw.draw_border(*p_icon(), rel_posn, thickness,
		  	color(), color(), color());
  COPY_BUFFER();
}

void
WMCanvas::fill_rectangle(int rel_x, int rel_y, int w, int h)
{
  WMApp::Xw.fill_rectangle(*p_icon(), rel_x, rel_y, w, h, color());
  COPY_BUFFER();
}

void
WMCanvas::fill_rectangle(const WMRectangle & rel_posn)
{
  WMApp::Xw.fill_rectangle(*p_icon(), rel_posn, color());
  COPY_BUFFER();
}

void
WMCanvas::fill_arc(int x, int y, int width, int height, int angle1, int angle2)
{
  WMApp::Xw.fill_arc(*p_icon(), x, y, width, height, angle1, angle2, color());
  COPY_BUFFER();
}

void
WMCanvas::fill_polygon(const vector<X::XPoint>& points, Xwrapper::XShape shape)
{
  X::XPoint *p = new X::XPoint[points.size()];
  for (size_t i = 0; i < points.size(); ++i)
  {
    p[i].x = points[i].x;
    p[i].y = points[i].y;
  }
  WMApp::Xw.fill_polygon(*p_icon(), p, points.size(), color(), shape);
  delete[] p;
  COPY_BUFFER();
}

void
WMCanvas::copy_rectangle(const WMPixmap & source, int source_x, int source_y,
                         int source_w, int source_h, int dest_x, int dest_y)
{
  WMApp::Xw.copy_rectangle(source, *p_icon(), source_x, source_y,
			   source_w, source_h, dest_x, dest_y);
  COPY_BUFFER();
}

void
WMCanvas::copy_rectangle(const WMPixmap & source, const WMRectangle & posn,
                         int dest_x, int dest_y)
{
  WMApp::Xw.copy_rectangle(source, *p_icon(), posn, dest_x, dest_y);
  COPY_BUFFER();
}

#undef COPY_BUFFER

