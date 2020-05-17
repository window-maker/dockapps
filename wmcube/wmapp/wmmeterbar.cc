#include <iostream>
#include <algorithm> // for max()
#include "wmmeterbar.h"
#include "wmwindow.h"
#include "wmapp.h"

using std::cerr;
using std::endl;

// functions for WMMeterBar ----------------------------------------------
// added by Jason

void
WMMeterBar::real_display()
{
  if ((orientation() == Orientation::Horizontal && width() < 2 * border())
      || (orientation() == Orientation::Vertical && height() < 2 * border()))
    { cerr << "WMError: Meter Bar " << this << " too small" << endl; return; }

  //first draw desired background 
  switch (style())
  {                     
    case Spectrum:              
      if (orientation() == Orientation::Horizontal)
        WMApp::Xw.draw_horizontal_gradient(window()->pixmap(),
            b_left(), b_top(), b_right(), b_bottom(), 0x5C00, 0x5C0000);
      else
        WMApp::Xw.draw_vertical_gradient(window()->pixmap(),
	    b_left(), b_top(), b_right(), b_bottom(), 0x5C0000, 0x5C00);
      break;
    case Spec_No_BG:
      WMApp::Xw.fill_rectangle(window()->pixmap(), b_position(),
          WMColor(Background));
      break;
    case Blue:
      WMApp::Xw.fill_rectangle(window()->pixmap(), b_position(),
          WMColor(Background));
      if (orientation() == Orientation::Horizontal)
        for (int x = 0; x < b_width(); x += 2)
          WMApp::Xw.draw_line(window()->pixmap(), x + b_left(), b_top(),
              x + b_left(), b_bottom(), 0x004941);
      else
        for (int y = 0; y < b_height(); y += 2)
          WMApp::Xw.draw_line(window()->pixmap(), b_left(),
              b_bottom() - y, b_right(), b_bottom() - y, 0x004941);
      break;
  }

  //then draw the progress bar
  if (total() && value())
  switch (style())
  {
    case Spectrum:
    case Spec_No_BG:
      if (orientation() == Orientation::Horizontal)
        WMApp::Xw.draw_horizontal_gradient(window()->pixmap(),
            b_left(), b_top(), b_right(), b_bottom(),
	    0xFF00, 0xFF0000, fraction());
      else
        WMApp::Xw.draw_vertical_gradient(window()->pixmap(),
	    b_left(), b_bottom(), b_right(), b_top(),
            0xFF00, 0xFF0000, fraction());
      break;
    case Blue:
      if (orientation() == Orientation::Horizontal)
        for (int x = 0;
	     x < static_cast<int>(std::min(1.0, fraction()) * b_width());
	     x += 2)
          WMApp::Xw.draw_line(window()->pixmap(), x + b_left(), b_top(),
              x + b_left(), b_bottom(), WMColor(Bright));
      else
        for (int y = 0;
	     y < static_cast<int>(std::min(1.0, fraction()) * b_height());
	     y += 2)
          WMApp::Xw.draw_line(window()->pixmap(), b_left(), b_bottom() - y,
	      b_right(), b_bottom() - y, WMColor(Bright));
      break;
  }
}

