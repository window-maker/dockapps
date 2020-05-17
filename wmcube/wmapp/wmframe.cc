#include <iostream>
#include <cstdarg>
#include "wmframe.h"
#include "wmclickable.h"
#include "wmwindow.h"
#include "wmapp.h"

using std::cerr;
using std::endl;

// functions for WMFrame -------------------------------------------------

WMFrame::~WMFrame()
{
  if (wClipMask) {
    WMApp::Xw.free_pixmap(*wClipMask);
    delete wClipMask;
  }
}

void
WMFrame::real_display()
{
  if (border() || !transparency())
    // fill frame with background color before adding children
    WMApp::Xw.fill_rectangle(window()->pixmap(), b_position(),
  	     		     WMColor(Background));
}

void
WMFrame::display()
{
  if (!(parent() && parent()->is_displayed())) return;
  real_display();
  setdisplayed(true);

  X::GC old_gc = 0;
  if (wClipMask) {
    // keep children from drawing where they shouldn't
    old_gc = window()->pixmap().gc;
    window()->pixmap().gc = wClipMask->gc;
  }
  for (unsigned int i = 0; i < numchildren(); i++)
    child(i)->display();
  if (wClipMask)
    window()->pixmap().gc = old_gc;

  draw_border();
}

void
WMFrame::hide()
{
  if (!is_displayed()) return;
  for (unsigned int i = 0; i < numchildren(); i++)
    child(i)->hide();
  real_hide();
  setdisplayed(false);
}

void
WMFrame::activate()
{
  if (is_active()) return;
  setactive(true);
  for (unsigned int i = 0; i < numchildren(); i++)
    child(i)->activate();
  real_activate();
  if (app()->done()) return;
  if (is_displayed()) display();
}

void
WMFrame::deactivate()
{ 
  if (!is_active()) return;
  real_deactivate();
  for (unsigned int i = 0; i < numchildren(); i++)
    child(i)->deactivate();
  setactive(false);
  if (is_displayed()) display();
}

bool
WMFrame::addchild(WMWidget *w)
{
  if (is_displayed()) return false;

  // child cannot already have a parent
  if (w->parent()) {
    cerr << "WMError: Reparenting child widget " << w << " to " << this
	    << " not allowed" << endl;
    return false;
  }

  // child cannot recursively contain "this"
  const WMWidget *test = this;
  while ((test = test->parent()))
    if (test == w) {
      cerr << "WMError: Widget " << w << " cannot be its own ancestor" << endl;
      return false;
    }
  
  w->setparent(this);
  wChildren.push_back(w);
  return true;
}

bool
WMFrame::removechild(WMWidget *w)
{
  if (is_displayed()) return false;
  for (unsigned int i = 0; i < numchildren(); i++)
    if (w == child(i)) {
      child(i)->setparent(0);
      wChildren.erase(wChildren.begin() + i);
      return true;
    }
  cerr << "WMError: Widget " << w << " is not a child of " << this << endl;
  return false;
}

void
WMFrame::clip(char *xpm_array[65])
{ 
  if (! is_displayed()) setaspectratios();
  for (unsigned int i = 0; i < numchildren(); i++)
    child(i)->clip(xpm_array);

  if (! transparency() || border()) { // create a mask for drawing children
    if (!wClipMask) {
      wClipMask = new WMPixmap;
      WMApp::Xw.create_pixmap(*wClipMask, xpm_array);
      X::XFreePixmap(WMApp::Xw.xdisplay(), wClipMask->pixmap);
      // free unused pixmap
      wClipMask->pixmap = wClipMask->mask;
      wClipMask->mask = 0; // muck around so xwrapper::free_pixmap works right
      X::XSetClipMask(WMApp::Xw.xdisplay(), wClipMask->gc, wClipMask->pixmap);
    }
    
    for (int col = left(); col < right(); col++)
      for (int row = top(); row < bottom(); row++)
        xpm_array[row + 3][col] = contains(col, row) ? 'X' : ' ';
    // clip in the whole frame on the window's mask, but don't let widgets
    // draw outside the frame
  }
}

bool
WMFrame::press(int button, int x, int y)
{
  WMClickable	*c;
  
  if (!contains(x, y)) {
    return false; // button pressed is not within this frame
  }
    
  for (unsigned int i = 0; i < numchildren(); i++)
    if ((c = dynamic_cast<WMClickable *>(child(i))))
      if (c->press(button, x, y)) return true;
  return false;
}

// If the mouse button is released over the WMWidget on which it was
// initially pressed, that widget's callbacks should be executed.
bool 
WMFrame::release(int button, int x, int y)
{
  WMClickable	*c;
  
  for (unsigned int i = 0; i < numchildren(); i++)
    if ((c = dynamic_cast<WMClickable *>(child(i))))
      if (c->release(button, x, y)) return true; 
  return false;
}

// This function needs to be fixed to take care of leftover space if the
// integer division wipes out a remainder.
bool
WMFrame::setaspectratios(vector<int> r)
{
  int totalspace = 0, availablespace;
  if (wRatiosSet || numchildren() == 0) return true; // nothing to do

  // if insufficient space to display any children, return an error
  if (width() < 2 * border() || height() < 2 * border())
    { cerr << "WMError: Frame " << this << " too small" << endl; return false; }

  if (r.size() > 0) {
    // Widgets that do not already have a size, for whom there is no
    // corresponding element in the vector of size ratios, will be
    // made size zero
    while (r.size() < numchildren())
      r.push_back(0);
  }
  
  // calculate total space consumed by widgets with known sizes
  for (unsigned int i = 0; i < numchildren(); i++)
    totalspace += child(i)->par_to_parent();
  availablespace = b_parallel() - totalspace - (numchildren() - 1) * padding();

  // If widgets of known size take up more space than the frame has,
  // rescale them in proportion and do not display the other widgets.
  if (availablespace < 1) {
    int numgoodchildren = 0;
    for (unsigned int i = 0; i < numchildren(); i++) {
      if (child(i)->par_to_parent() > 0)
	numgoodchildren++;
    }
    availablespace = b_parallel() - (numgoodchildren - 1) * padding();
    if (availablespace < 1 || numgoodchildren == 0) {
      // not enough space, even for only the known-size widgets
      cerr << "WMError: Frame " << this << " too small" << endl;
      return false;
    }

    for (unsigned int i = 0; i < numchildren(); i++) {
      if (child(i)->par_to_parent() > 0) {
	double ratio = child(i)->par_to_parent() / totalspace;
	child(i)->set_par_to_parent(static_cast<int>
				    (ratio * availablespace));
      }
    }
  }
  
  else { // Otherwise, scale the unknown-sized widgets appropriately.
    double ratiosum = 0;
    for (unsigned int i = 0; i < numchildren(); i++) {
      if (child(i)->par_to_parent() == 0)
	ratiosum += ((r.size() > 0) ? r[i] : 1.0);
    }
    for (unsigned int i = 0; i < numchildren(); i++)
      if (child(i)->par_to_parent() == 0) {
	double ratio = (r.size() ? r[i] : 1.0) / ratiosum;
	child(i)->set_par_to_parent(static_cast<int>
				    (ratio * availablespace));
      }
  }
  
  // Scale widgets in the perpendicular direction if necessary
  for (unsigned int i = 0; i < numchildren(); i++)
    if (child(i)->perp_to_parent() <= 0
	|| child(i)->perp_to_parent() > b_perpend())
      child(i)->set_perp_to_parent(b_perpend());
  
  // Finally, set the location of all these child widgets.
  int posn = 0;
  for (unsigned int i = 0; i < numchildren(); i++) {
    if (child(i)->width() && child(i)->height()) {
      if (orientation() == Orientation::Horizontal) {
	child(i)->setleft(b_left() + posn);
	child(i)->settop(b_top());
      }
      else {
	child(i)->setleft(b_left());
	child(i)->settop(b_top() + posn);
      }
      posn += child(i)->par_to_parent() + padding();
    }
  }

  // Deal with rounding errors: if the last widget doesn't reach all the
  // way to the end of the parent, force it to.  This isn't the best way
  // to do this, but it's the easiest :)
  if (child(numchildren() - 1)->par_to_parent()) {
    WMWidget *lastchild = child(numchildren() - 1);
    if (orientation() == Orientation::Horizontal)
      lastchild->setwidth(right() - border() - lastchild->left());
    else
      lastchild->setheight(bottom() - border() - lastchild->top());
  }
    
  wRatiosSet = true;
  return true;
}

// varargs version for people too lazy to create an std::vector.
// Use with caution: the number of arguments MUST equal the number
// of children, since we can't check the number of arguments of a
// varargs function within the function.  I'm not sure what happens
// if you try to provide this function with fewer arguments than
// the widget has children, but I imagine it's bad.
bool
WMFrame::setaspectratios(int ratio, ...)
{
  if (numchildren() < 1)
    // nothing to do, return OK
    return true;
	
  std::va_list args;
  vector<int> v = vector<int>();
  v.push_back(ratio);

  va_start(args, ratio);
  for (unsigned int i = 1; i < numchildren(); i++) {
    ratio = va_arg(args, int);
    v.push_back(ratio);
  }
  va_end(args);

  return setaspectratios(v);
}

