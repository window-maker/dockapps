#include "wmled.h"
#include "wmapp.h"

// functions for WMLed ---------------------------------------------------

void
WMLed::real_display()
{
  seticon(WMApp::leds_pixmap[static_cast<int>(led())], false);
  if (icon()) WMImage::real_display();
}

