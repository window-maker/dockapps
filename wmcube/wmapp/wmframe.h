#include <vector>
#include "wmwidget.h"
#include "wmclickable.h"
#include "xwrapper.h"

#ifndef _WMFRAME_H
#define _WMFRAME_H

using std::vector;

// WMFrame: A widget that can contain other widgets
class WMFrame : public virtual WMClickable, public virtual WMWidget {
 private:
  int			wPadding;
  bool			wRatiosSet;
  bool			wTransparentPadding;
  WMPixmap		*wClipMask;

 protected:
  vector<WMWidget *>    wChildren;
  virtual void real_display();
  
 public:	 
  WMFrame();
  virtual ~WMFrame();
  
  WMWidget * child(int i) const;
  unsigned int numchildren() const;
  bool addchild(WMWidget &child);
  bool addchild(WMWidget *child);
  bool removechild(WMWidget &child);
  bool removechild(WMWidget *child);

  void clip(char *xpm_array[65]);
  virtual void display();
  virtual void hide();
  virtual void activate();
  virtual void deactivate();

  int padding() const;
  bool transparency() const;
  
  void setpadding(int);
  void settransparency(bool);
  
  // what to do if a mouse button is pressed?
  bool press(int button, int x, int y);
  bool release(int button, int x, int y);
  
  // NOTE: The setaspectratios() function must be called on frames / windows
  // in order from Parent to Child widgets!
  //
  // if passed no argument, this function allots equal space to
  // all child widgets that do not already have a declared size.
  // Otherwise, it uses the vector or va_list it's passed as a list of how
  // the widgets should be sized relative to each other. Note that even though 
  // widgets with non-zero sizes are not resized, the vector of sizes must
  // contain dummy place-holder arguments for them anyway.
  //
  // For example, if the frame contains 4 widgets a,b,c,d, of which b already
  // has a known (non-zero) size, and you call this function with argument
  // being the vector (5,4,3), then widget a will be allotted 5/(5+3) = 5/8
  // and widget c will be allotted 3/(5+3) = 3/8 of the space not already
  // allotted to widget b.  Widget d will not be displayed. On the other hand,
  // if you call this function with no argument, then a, c, and d will EACH be
  // allocated 1/3 of the space not already taken by widget b.
  //
  // This function, however it is called, will scale widgets to the
  // perpendicular size of the frame if their perpendicular dimension is not
  // known.  If the perpendicular size of the widget IS known, it will be
  // rescaled only if the widget is larger than the frame in that dimension.
  bool setaspectratios(vector<int> ratiolist = vector<int>());

  // a va_list version for people too lazy to create an std::vector:
  bool setaspectratios(int ratio1, ...);
};

// inline functions for WMFrame ------------------------------------------

inline WMFrame::WMFrame()
: wPadding(2), wRatiosSet(false), wTransparentPadding(true), wClipMask(0),
  wChildren() { setborder(0); }

inline unsigned int
WMFrame::numchildren() const { return wChildren.size(); }

inline WMWidget *
WMFrame::child(int i) const { return wChildren[i]; }

inline bool
WMFrame::addchild(WMWidget &child) { return addchild(&child); }

inline bool
WMFrame::removechild(WMWidget &child) { return removechild(&child); }

inline int
WMFrame::padding() const { return wPadding; }

inline bool
WMFrame::transparency() const { return wTransparentPadding; }

inline void
WMFrame::setpadding(int p) { if (!is_displayed()) wPadding = p; }
    
inline void
WMFrame::settransparency(bool b)
{ if (!is_displayed()) wTransparentPadding = b; }

#endif
