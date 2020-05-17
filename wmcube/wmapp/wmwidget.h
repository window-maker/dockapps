#include "colors.h"

#ifndef _WMWIDGET_H
#define _WMWIDGET_H

// needed forward declarations
class WMApp;
class WMWindow;
class WMFrame;

// Classes for writing complicated WindowMaker Dockapp programs! Yaay!
// Note: I'm lazy, so ALL LAYOUT has to be done statically before the
// WMApp starts to run.

namespace Orientation {
  enum Orientation { Horizontal, Vertical };
};

class WMRectangle {
 private:
  int wLeft, wTop, wWidth, wHeight;
 public:
  WMRectangle(int Left = 0, int Top = 0, int Width = 0, int Height = 0);
  WMRectangle(const WMRectangle &r);
  WMRectangle & operator = (const WMRectangle &r);
  ~WMRectangle();

  int left() const;
  int right() const;
  int top() const;
  int bottom() const;
  int width() const;
  int height() const;
  bool contains(int x, int y) const;

  void setleft(int);
  void settop(int);
  void setwidth(int);
  void setheight(int);
};

// WMWidget: the base class for all Dockapp widgets.  Cannot be instantiated.
class WMWidget {
 private:
  enum Orientation::Orientation	wOrientation;
  WMRectangle			wPosition;
  int				wBorder;
  Color				wTopLeft, wBottomRight;
  const WMFrame *		wParent;
  bool  	                wDisplayed;
  bool				wActive;
  
 protected:
  // these functions do the actual work of displaying a widget.
  virtual void real_display() = 0;
  virtual void draw_border();
  virtual void real_hide();
  
  // colors for border of widget
  Color top_left_c() const;
  Color bottom_right_c() const;
  void set_top_left_c(const Color & c);
  void set_bottom_right_c(const Color & c);
  
  virtual void real_activate()   { } // these will be no-ops except
  virtual void real_deactivate() { } // for clickable widgets
 
  void setactive(bool);
  void setdisplayed(bool);
  
 public:
  WMWidget(const WMRectangle &layout = WMRectangle(0, 0, 0, 0),
	   Orientation::Orientation o = Orientation::Horizontal);
  virtual ~WMWidget();

  virtual WMApp *app() const;
  virtual const WMWindow *window() const;
  virtual const WMFrame *parent() const;
  virtual void setparent(const WMFrame *); // should be protected, but then
  virtual void setparent(const WMFrame &); // WMFrame can't use on children
  virtual void clip(char *xpm_array[65]);
  
  virtual void setposition(const WMRectangle &posn);
  void setorientation(Orientation::Orientation o);

  // these functions do all the busywork of displaying a widget.
  virtual void display();	// over-ridden by WMFrame
  virtual void hide();		// in order to act recursively
  virtual void activate();	// on all child widgets
  virtual void deactivate();	//
  
  const WMRectangle & position() const;
  enum Orientation::Orientation orientation() const;
  virtual bool contains(int x, int y) const;

  int top() const;
  int left() const;
  int bottom() const;
  int right() const;
  int border() const;
  
  int width() const;
  int height() const;  
  int parallel() const;		// dimension parallel to orientation
  int perpend() const;		// dimension perpendicular to orientation
  int par_to_parent() const;	// dimension parallel to parent's orientation
  int perp_to_parent() const;	// dimension perp. to parent's orientation

  // like above, but only considering the part of the widget INSIDE its border
  WMRectangle b_position() const;
  int b_top() const;
  int b_left() const;
  int b_bottom() const;
  int b_right() const;
  int b_width() const;
  int b_height() const;  
  int b_parallel() const;
  int b_perpend() const;
  int b_par_to_parent() const;
  int b_perp_to_parent() const;
 
  // "set" accessor functions
  void settop(int);
  void setleft(int);
  void setborder(int);
  
  void setwidth(int);
  void setheight(int);
  void setparallel(int);
  void setperpend(int);
  void set_par_to_parent(int);
  void set_perp_to_parent(int);
  
  bool is_displayed() const;
  bool is_active() const;
};


// inline functions for WMRectangle --------------------------------------

inline WMRectangle::WMRectangle(int Left, int Top, int Width, int Height)
: wLeft(Left), wTop(Top), wWidth(Width), wHeight(Height)
{ }

inline WMRectangle::WMRectangle(const WMRectangle &r)
: wLeft(r.wLeft), wTop(r.wTop), wWidth(r.wWidth), wHeight(r.wHeight)
{ }

inline WMRectangle::~WMRectangle() { }

inline WMRectangle &
WMRectangle::operator = (const WMRectangle &r)
{
  wLeft = r.wLeft, wTop = r.wTop, wWidth = r.wWidth, wHeight = r.wHeight;
  return *this;
}

inline int
WMRectangle::left() const { return wLeft; }

inline int
WMRectangle::right() const { return wLeft + wWidth; }

inline int
WMRectangle::top() const { return wTop; }

inline int
WMRectangle::bottom() const { return wTop + wHeight; }

inline int
WMRectangle::width() const { return wWidth; }

inline int
WMRectangle::height() const { return wHeight; }

inline void
WMRectangle::setleft(int l) { wLeft = l; }

inline void
WMRectangle::settop(int t) { wTop = t; }

inline void
WMRectangle::setwidth(int w) { wWidth = w; }

inline void
WMRectangle::setheight(int h) { wHeight = h; }

inline bool
WMRectangle::contains(int x, int y) const
{ return x >= left() && x < right() && y >= top() && y < bottom(); }

// inline functions for WMWidget -----------------------------------------

inline WMWidget::~WMWidget() { }

inline const WMFrame *
WMWidget::parent() const { return wParent; }

inline void
WMWidget::clip(char *xpm_array[65])
{
  for (int col = left(); col < right(); col++)
    for (int row = top(); row < bottom(); row++)
      if (contains(col, row))
        xpm_array[row + 3][col] = 'X';
}

inline void
WMWidget::setparent(const WMFrame *f) { wParent = f; }

inline void
WMWidget::setparent(const WMFrame &f) { wParent = &f; }

inline void
WMWidget::setactive(bool b) { wActive = b; }
	
inline void
WMWidget::setdisplayed(bool b) { wDisplayed = b; }

inline Color
WMWidget::top_left_c() const { return wTopLeft; }

inline Color
WMWidget::bottom_right_c() const { return wBottomRight; }

inline void 
WMWidget::set_top_left_c(const Color & c) { wTopLeft = c; }

inline void
WMWidget::set_bottom_right_c(const Color & c) { wBottomRight = c; }

inline void
WMWidget::setposition(const WMRectangle &posn)
{ if (!wDisplayed) wPosition = posn; }

inline void
WMWidget::setorientation(Orientation::Orientation o)
{ if (!wDisplayed) wOrientation = o; }

inline const WMRectangle &
WMWidget::position() const { return wPosition; }

inline enum Orientation::Orientation
WMWidget::orientation() const { return wOrientation; }

inline bool
WMWidget::contains(int x, int y) const { return position().contains(x, y); }

inline int
WMWidget::top() const { return position().top(); }

inline int
WMWidget::left() const { return position().left(); }

inline int
WMWidget::bottom() const { return position().bottom(); }

inline int
WMWidget::right() const { return position().right(); }

inline int
WMWidget::border() const { return wBorder; }

inline int
WMWidget::width() const { return position().width(); }

inline int
WMWidget::height() const { return position().height(); }

inline int
WMWidget::parallel() const
{
  return (orientation() == Orientation::Horizontal) ?
	  position().width() : position().height();
}

inline int
WMWidget::perpend() const
{
  return (orientation() == Orientation::Horizontal) ?
	  position().height() : position().width();
}

// like above, but only considering the part of the widget INSIDE its border:

inline WMRectangle
WMWidget::b_position() const
{
  return WMRectangle(left() + border(), top() + border(),
		     width() - 2 * border(), height() - 2 * border());
}

inline int
WMWidget::b_top() const { return top() + border(); }

inline int
WMWidget::b_left() const { return left() + border(); }

inline int
WMWidget::b_bottom() const { return bottom() - border(); }

inline int
WMWidget::b_right() const { return right() - border(); }

inline int
WMWidget::b_width() const { return width() - 2 * border(); }

inline int
WMWidget::b_height() const { return height() - 2 * border(); }

inline int
WMWidget::b_parallel() const { return parallel() - 2 * border(); }

inline int
WMWidget::b_perpend() const { return perpend() - 2 * border(); }

inline int
WMWidget::b_par_to_parent() const { return par_to_parent() - 2 * border(); }

inline int
WMWidget::b_perp_to_parent() const { return perp_to_parent() - 2 * border(); }

// "set" accessor functions:

inline void
WMWidget::setwidth(int w)
{ if (!wDisplayed) wPosition.setwidth(w); }

inline void
WMWidget::setheight(int h)
{ if (!wDisplayed) wPosition.setheight(h); }

inline void
WMWidget::settop(int t)
{ if (!wDisplayed) wPosition.settop(t); }

inline void
WMWidget::setleft(int l)
{ if (!wDisplayed) wPosition.setleft(l); }

inline void
WMWidget::setborder(int b)
{ if (!wDisplayed) wBorder = b; }

inline void
WMWidget::setparallel(int p)
{
  if (!wDisplayed)
    ((wOrientation == Orientation::Horizontal) ?
     wPosition.setwidth(p) : wPosition.setheight(p));
}

inline void
WMWidget::setperpend(int p)
{
  if (!wDisplayed)
    ((wOrientation == Orientation::Horizontal) ?
     wPosition.setheight(p) : wPosition.setwidth(p));
}

inline bool
WMWidget::is_displayed() const { return wDisplayed; }

inline bool
WMWidget::is_active() const { return wActive; }

#endif
