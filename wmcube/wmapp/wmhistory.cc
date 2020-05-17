#include <iostream>
#include "wmhistory.h"
#include "wmwindow.h"
#include "wmapp.h"

using std::cerr;
using std::endl;

// functions for WMHistory -----------------------------------------------
// added by Jason

void WMHistory::setvalue (int value, int index, bool dodisplay)
{
  if (index == -1)
  {
    wValues.pop_back();
    wValues.push_front(value);
  }
  else
    wValues[index] = value;
  if (dodisplay)
    display();
}

void WMHistory::setvalues (const vector<int>& values, bool dodisplay)
{
  vector<int>::const_iterator src = values.begin();
  deque<int>::iterator dest = wValues.begin();
  while (src != values.end() && dest != wValues.end())
    *dest++ = *src++;
  while (dest != wValues.end())
    *dest++ = 0;
  if (dodisplay)
    display();
}

void WMHistory::clear(bool dodisplay)
{ setvalues(vector<int>(0), dodisplay); }

void WMHistory::real_display ()
// XXX: Currently no vertical style, since it doesn't make a lot of sense
{
  WMApp::Xw.fill_rectangle(window()->pixmap(), b_position(),
		  	   WMColor(Background));
  if (!wTotal)
    return;
  if (width() < 2 * border() || height() < 2 * border())
    { cerr << "WMError: History " << this << " too small" << endl; return; }

  for (int x = 0; x < b_width(); x++)
  {
    int h = static_cast<int> (fraction(x) * b_height());
    if (h < 1)
      continue;
    else if (h == 1)
      WMApp::Xw.draw_point(window()->pixmap(), b_right() - 1 - x, b_bottom() - 1
		           , WMColor(Bright));
    else
    {
      WMApp::Xw.draw_line(window()->pixmap(), b_right() - 1 - x, b_bottom() - h,
		          b_right() - 1 - x, b_bottom() - h + 1, WMColor(Bright));
      if (h > 2)
        WMApp::Xw.draw_line(window()->pixmap(), b_right() - 1 - x, b_bottom() - h + 2, b_right() - 1 - x, b_bottom() - 1, WMColor(Medium));
    }
  }
}

