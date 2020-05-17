#include "wmcallback.h"

// functions for WMCallback ----------------------------------------------

void
WMCallback::execute()
{
  // execute all callbacks for this object, in order
  for (unsigned int i = 0; i < numcallbacks(); i++)
    runcallback(i);
}

bool
WMCallback::press(int button, int x, int y)
{
  if (contains(x, y)) {
    wPressed = true;
    return true;
  }
  else return false;
}

bool
WMCallback::release(int button, int x, int y)
{
  if (wPressed) {
    wPressed = false;
    if (contains(x,y))
      execute();
    return true;
  }
  else return false;
}

