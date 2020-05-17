#include "wmbutton.h"
#include "wmapp.h"
#include "wmwindow.h"

// functions for WMButton ------------------------------------------------

WMButton::WMButton()
{
  setborder(1);
  setbgcolor(WMColor(ButtonFace), false);
  set_top_left_c(WMColor(ButtonBorderBright));
  set_bottom_right_c(WMColor(ButtonBorderDim));
}

void
WMButton::real_display()
{
  if (is_active())
    WMImage::real_display();
  else
    // If button is not clickable, don't draw an icon.
    // Note: If the button has no icon, I recommend that you change its
    // background color from the default; otherwise the button will always
    // look as if it is disabled.
    WMApp::Xw.fill_rectangle(window()->pixmap(), b_position(),
		    	     WMColor(ButtonFace));
}

bool 
WMButton::press(int button, int x, int y)
{ 
  if (contains(x, y)) {
    if (is_active()) {
      wPressed = true;
      set_top_left_c(WMColor(ButtonBorderDim));
      set_bottom_right_c(WMColor(ButtonBorderBright));
      draw_border();
      app()->repaint();
    }
    return true;
  }
  else return false;
}

bool
WMButton::release(int button, int x, int y)
{
  if (wPressed) {
    wPressed = false;
    set_top_left_c(WMColor(ButtonBorderBright));
    set_bottom_right_c(WMColor(ButtonBorderDim));
    draw_border();
    app()->repaint();
    if (contains(x, y))
      execute();
    return true;
  }
  else return false;
}
