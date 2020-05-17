#include "wmwidget.h"
#include "wmwindow.h"
#include "wmapp.h"

// functions for WMWidget ------------------------------------------------

WMWidget::WMWidget(const WMRectangle &posn, Orientation::Orientation o)
: wOrientation(o), wPosition(posn), wBorder(1),
  wTopLeft(WMColor(FrameBorderDim)), wBottomRight(WMColor(FrameBorderBright)),
  wParent(0), wDisplayed(false), wActive(true)
{ }

int
WMWidget::par_to_parent() const
{
  if (!parent()) return -1;
  return (parent()->orientation() == Orientation::Horizontal) ?
          position().width() : position().height();
}

int
WMWidget::perp_to_parent() const
{
  if (!parent()) return -1;
  return (parent()->orientation() == Orientation::Horizontal) ?
          position().height() : position().width();
}

void
WMWidget::set_par_to_parent(int p)
{
  if (wParent && !wDisplayed)
    ((wParent->wOrientation == Orientation::Horizontal) ?
     wPosition.setwidth(p) : wPosition.setheight(p));
}

void
WMWidget::set_perp_to_parent(int p)
{
  if (wParent && !wDisplayed)
    ((wParent->wOrientation == Orientation::Horizontal) ?
     wPosition.setheight(p) : wPosition.setwidth(p));
}

const WMWindow *
WMWidget::window() const { return wParent ? wParent->window() : 0; }

WMApp *
WMWidget::app() const { return wParent ? wParent->app() : 0; }

void
WMWidget::display()
{
  if (!(parent() && parent()->is_displayed())) return;
  real_display();
  draw_border();
  setdisplayed(true);
}

void
WMWidget::draw_border()
{
  if (border() && 2 * border() < width() && 2 * border() < height())
    WMApp::Xw.draw_border(window()->pixmap(), position(), border(),
			  top_left_c(), bottom_right_c(),
			  Color::alpha_blend(top_left_c(), bottom_right_c(),
				  	     0.5));  
}
  
void
WMWidget::hide()
{
  if (!is_displayed()) return;
  real_hide();
  setdisplayed(false);
}

void
WMWidget::activate()
{
  if (is_active()) return;
  real_activate();
  setactive(true);
  if (is_displayed()) display();
}

void
WMWidget::deactivate()
{ 
  if (!is_active()) return;
  real_deactivate();
  setactive(false);
  if (is_displayed()) display();
}

void
WMWidget::real_hide()
{ if (window()) WMApp::Xw.clear_rectangle(window()->pixmap(), position()); }

