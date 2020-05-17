#include <vector>
#include <deque>
#include "wmwidget.h"

#ifndef _WMHISTORY_H
#define _WMHISTORY_H

using std::vector;
using std::deque;

// WMHistory: a progress / load meter showing a history
// added by Jason
// XXX: Should we have an option to rescale the graph automatically?
class WMHistory : public virtual WMWidget {
 private:
  static const int size = 64; //Number of values that we keep around
  deque<int> wValues;
  int wTotal;
  void real_display();

 public:
  WMHistory();
  WMHistory(int total);
  WMHistory(const vector<int>& values, int total);
  // XXX: Too lazy to make any other constructors.

  // All indexed functions refer to the element added that many units ago.
  // An index of -1 therefore appends a new value to the graph.
  void setvalue(int value, int index = -1, bool dodisplay = true);

  //setvalues zeros the history before copying the given values in.
  void setvalues(const vector<int>& values, bool dodisplay = true);
  void setvalues(const vector<int>& values, int total, bool dodisplay = true);
  void clear(bool dodisplay = true);
  
  void settotal(int total, bool dodisplay = true);

  int value(int index = 0) const;
  int total() const;
  double fraction(int index = 0) const;
};

// inline functions for WMHistory ----------------------------------------

inline WMHistory::WMHistory ()
: wValues(WMHistory::size, 0), wTotal(0) { }

inline WMHistory::WMHistory (int total)
: wValues(WMHistory::size, 0), wTotal(total) { }

inline WMHistory::WMHistory (const vector<int>& values, int total)
: wValues(size), wTotal(total)
{ setvalues(values, false); }

inline void WMHistory::setvalues(const vector<int>& values, int total,
  bool dodisplay)
{ wTotal = total; setvalues(values, dodisplay); }

inline void WMHistory::settotal (int total, bool dodisplay)
{ wTotal = total; if (dodisplay) display(); }

inline int WMHistory::value (int index) const
{ return wValues[index]; }

inline int WMHistory::total () const
{ return wTotal; }

inline double WMHistory::fraction (int index) const
{ return static_cast<double>(wValues[index]) / wTotal; }

#endif
