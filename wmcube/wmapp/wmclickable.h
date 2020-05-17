#ifndef _WMCLICKABLE_H
#define _WMCLICKABLE_H

// WMClickable: Widgets that are clickable should inherit from this class.
class WMClickable {
 protected:
  bool wPressed;
 public:
  WMClickable();
  virtual ~WMClickable() { }
  
  virtual bool press(int button, int x, int y) = 0;
  virtual bool release(int button, int x, int y) = 0;
};

// inline functions for WMClickable --------------------------------------

inline WMClickable::WMClickable()
: wPressed(false) { }

#endif
