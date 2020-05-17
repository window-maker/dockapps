#include <vector>
#include "wmwidget.h"
#include "wmclickable.h"

#ifndef _WMCALLBACK_H
#define _WMCALLBACK_H

using std::vector;

// WMCallback: A class capable of setting and executing callback functions.
class WMCallback : public virtual WMWidget, public virtual WMClickable {
 public:
  typedef void (* data_func)(const WMApp *, void *);
  typedef void (* widget_func)(const WMApp *, WMWidget *, void *);

 private:
  class callback {
    public: virtual void execute(const WMApp *) = 0;
  };
  
  class data_callback : public callback {
    data_func		func;
    void *		data;
   public:
    data_callback(data_func, void *);
    void execute(const WMApp *);
  };

  class widget_callback : public callback {
    widget_func		func;
    WMWidget *		widget;
    void *		data;
   public:
    widget_callback(widget_func, WMWidget *, void *);
    void execute(const WMApp *);
  };

  vector<callback*> wCallback;
 
 public:
  WMCallback();
  WMCallback(data_func, void * = 0);
  WMCallback(widget_func, WMWidget *, void * = 0);
  virtual ~WMCallback() { clearcallbacks(); }
  
  unsigned int numcallbacks() const;
  void addcallback(data_func, void * = 0);
  void addcallback(widget_func, WMWidget *, void * = 0);
  void runcallback(unsigned int i);
  void clearcallbacks();
  
  // check to see if a mouse click hit the widget; if so, run callbacks
  virtual bool press(int button, int x, int y);
  virtual bool release(int button, int x, int y);  
  virtual void execute();
};

// inline functions for WMCallback ---------------------------------------

inline
WMCallback::data_callback::data_callback(data_func f, void *datap)
: func(f), data(datap) { }

inline void
WMCallback::data_callback::execute(const WMApp *app)
{ (func)(app, data); }

inline
WMCallback::widget_callback::widget_callback
(widget_func f, WMWidget *w, void *datap)
: func(f), widget(w), data(datap) { }

inline void
WMCallback::widget_callback::execute(const WMApp *app)
{ (func)(app, widget, data); }

inline
WMCallback::WMCallback()
{ }

inline
WMCallback::WMCallback(data_func f, void *data)
{ addcallback(f, data); }

inline
WMCallback::WMCallback(widget_func f, WMWidget *w, void *data)
{ addcallback(f, w, data); }

inline void
WMCallback::addcallback(data_func f, void *data)
{ wCallback.push_back(new data_callback(f, data)); }
  
inline void
WMCallback::addcallback(widget_func f, WMWidget *w, void *data)
{ wCallback.push_back(new widget_callback(f, w, data)); }

inline void
WMCallback::runcallback(unsigned int i)
{ wCallback[i]->execute(app()); }

inline unsigned int
WMCallback::numcallbacks() const { return wCallback.size(); }

inline void
WMCallback::clearcallbacks()
{
  for (vector<callback *>::iterator i = wCallback.begin();
      i != wCallback.end(); ++i)
    delete *i;
  wCallback.clear();
}

#endif
