#include "wmimage.h"
#include "wmcallback.h"

#ifndef _WMBUTTON_H
#define _WMBUTTON_H

// WMButton: An image that can execute callback functions when clicked upon
class WMButton : public WMCallback, public WMImage {
 private:
  void real_display();
  
 public:
  WMButton();
  bool press(int button, int x, int y);
  bool release(int button, int x, int y);
};

#endif
