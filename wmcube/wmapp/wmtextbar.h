#include "wmwidget.h"
#include <string>

#ifndef _WMTEXTBAR_H
#define _WMTEXTBAR_H

using std::string;

// WMTextBar: a place to draw text in.
class WMTextBar : public virtual WMWidget {
 private:
  string wText;
  int wFontsize;
  void real_display();
 
 public:
  WMTextBar(const string & text = "", int fontsize = 1);
  ~WMTextBar();

  void settext(const string & text, bool dodisplay = true);
  void setfont(int fontsize, bool dodisplay = true);
  const string & text() const;
  char text(unsigned int) const;
  int fontsize() const;
  int size() const;
};

// inline functions for WMTextBar ----------------------------------------

inline WMTextBar::WMTextBar(const string &text, int fontsize)
: wText(text), wFontsize(fontsize)
{ }

inline WMTextBar::~WMTextBar() { }

inline void
WMTextBar::settext(const string &text, bool dodisplay)
{ wText = text; if (dodisplay) display(); }

inline void
WMTextBar::setfont(int fontsize, bool dodisplay)
{ wFontsize = fontsize; if (dodisplay) display(); }

inline const string &
WMTextBar::text() const { return wText; }

inline char
WMTextBar::text(unsigned int posn) const
{ return (posn >= wText.size()) ? ' ' : wText[posn]; }

inline int
WMTextBar::fontsize() const { return wFontsize; }
	
inline int
WMTextBar::size() const { return wText.size(); }

#endif
