#include "wmwidget.h"
#include "xwrapper.h"

#ifndef _WMIMAGE_H
#define _WMIMAGE_H

// WMImage: a widget that displays a picture in Xpm format
class WMImage : public virtual WMWidget {
 private:
  WMPixmap		wIcon;
  WMPixmap *		wIconPtr;

 protected:
  Color			wBGColor;
  virtual void 		real_display();
  WMPixmap *		p_icon();

 public:
  WMImage(const WMPixmap * pm = 0);
  WMImage(const WMPixmap & pm);
  WMImage(char *xpm[]);
  virtual ~WMImage();

  void seticon(bool dodisplay = true); //make an empty icon
  void seticon(const WMPixmap *, bool dodisplay = true);
  void seticon(const WMPixmap &, bool dodisplay = true);
  void seticon(char *xpm[], bool dodisplay = true);
  void setbgcolor(Color, bool dodisplay = true);
  Color bgcolor() const;
  const WMPixmap * icon() const;
};

// inline functions for WMImage ------------------------------------------

inline void
WMImage::seticon(const WMPixmap *pm, bool dodisplay)
{ if (pm) seticon(*pm, dodisplay); else wIconPtr = 0; }

inline void
WMImage::setbgcolor(Color c, bool dodisplay)
{ wBGColor = c; if (dodisplay) display(); }

inline Color
WMImage::bgcolor() const { return wBGColor; }

inline WMPixmap *
WMImage::p_icon() { return wIconPtr; }

inline const WMPixmap *
WMImage::icon() const { return wIconPtr; }

#endif
