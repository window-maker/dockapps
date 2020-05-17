#include "colors.h"

#ifndef _MY_XWRAPPER_H
#define _MY_XWRAPPER_H

//These must be included here so that important types and functions aren't
//dragged into the X namespace, but left undefined in the global namespace.
extern "C" {
#   include <stddef.h>
#   include <stdlib.h>
#   include <sys/types.h>
#   include <malloc.h>
}
namespace X {
  // Unfortunately, many common X calls are actually macros and the X::
  // prefix must be avoided when using them.  Still, it is nice to use
  // a separate namespace whenever possible.

  // the next line ensures that X macros are expanded correctly into functions
  // in the X namespace, since _XPrivDisplay appears to be the only
  // real X object used by all X macros:
# define _XPrivDisplay X::_XPrivDisplay

  // this is just so I can easily see which identifiers are macros from Xlib
  // that can't be prefixed by "X::"
# define X_MACRO(x) (x)

  extern "C" {
#   include <X11/X.h>
#   include <X11/Xlib.h>
#   include <X11/xpm.h>
#   include <X11/extensions/shape.h>
  }
};

// forward declaration for rectangle
struct WMRectangle;

struct WMPixmap {
  X::Pixmap pixmap;
  X::Pixmap mask;
  X::GC     gc;
  X::XpmAttributes attr;
};

// global X thingies
extern X::Window wActiveWin, wProgramWin;
extern X::Atom deleteWin, _XA_GNUSTEP_WM_FUNC;

class Xwrapper {
 private:
  X::Display *		xDisplay;
  X::Window		xRootWindow;
  X::XWindowAttributes	xAttributes;
  mutable X::GC		xGC;

  unsigned long color_to_xcolor(Color) const;
  void		set_GC(X::GC, Color) const;
  
 public:
  Xwrapper();
  ~Xwrapper();

  X::Display *		xdisplay() const;
  const X::Window	xrootwin() const;
  X::GC			get_GC() const;
  
  bool create_pixmap(WMPixmap & dest, char * pixmap_bytes[]) const;
  bool create_pixmap(WMPixmap & dest, const WMPixmap & source) const;
  bool create_pixmap(WMPixmap & dest, const WMPixmap * source) const;
  bool create_pixmap(WMPixmap & dest, int width, int height) const;
  bool create_pixmap(WMPixmap & dest, int width, int height, int depth) const;

  void free_pixmap(WMPixmap & dest) const;

  Color get_point(const WMPixmap & src, int x, int y) const;
  
  void draw_point(WMPixmap & dest, int x, int y, Color c) const;
  void draw_line(WMPixmap & dest, int x1, int y1, int x2, int y2, Color c)
	  const;
  void draw_arc(WMPixmap & dest, int x, int y, int width, int height,
      int angle1, int angle2, Color c); //angles are in units of degrees * 64
  void draw_lines(WMPixmap & dest, const X::XPoint* points, int npoints,
      Color c) const; //X::XPoint is simply a struct { short x, y };

  // draws a color gradient from left to right or bottom to top from color1
  // to color2. amount is the proportion of the gradient to draw i.e. 0.5
  // draws half the gradient. It should always be at least 2 pixels wide.
  void draw_horizontal_gradient(WMPixmap & dest, int x1, int y1, int x2, int y2, Color c1, Color c2, double amount = 1.0);
  void draw_vertical_gradient(WMPixmap & dest, int x1, int y1, int x2, int y2, Color c1, Color c2, double amount = 1.0);
  
  void draw_border(WMPixmap & dest, int x, int y, int width, int height,
		   int thickness, Color topleft, Color botright, Color corner)
	  const;
  void draw_border(WMPixmap & dest, const WMRectangle & posn, int thickness,
		   Color topleft, Color botright, Color corner) const;
  
  void fill_rectangle(WMPixmap & dest, int x, int y, int width, int height,
		      Color c) const;
  void fill_rectangle(WMPixmap & dest, const WMRectangle & posn, Color c)
	  const;

  void fill_arc(WMPixmap & dest, int x, int y, int width, int height,
      int angle1, int angle2, Color c); //angles are in units of degrees * 64

  enum XShape { complex = X_MACRO(Complex), convex = X_MACRO(Convex),
    nonconvex = X_MACRO(Nonconvex) };
  void fill_polygon(WMPixmap & dest, const X::XPoint* points, int npoints,
      Color c, XShape shape = complex) const;
      //shape is simply a hint to the X server about what type of shape is
      //being drawn. See XFillPolygon(3) for a complete description.
  
  void clear_rectangle(WMPixmap & dest, int x, int y, int width, int height)
	  const;
  void clear_rectangle(WMPixmap & dest, const WMRectangle & posn) const;
  
  void copy_rectangle(const WMPixmap & source, WMPixmap & dest,
		      int source_x, int source_y, int source_w, int source_h,
		      int dest_x = 0, int dest_y = 0) const;
  void copy_rectangle(const WMPixmap & source, WMPixmap & dest,
		      const WMRectangle & posn, int dest_x = 0, int dest_y = 0)
	  	      const;
  void copy_rectangle(const WMPixmap & source, X::Drawable & dest,
		      int source_x, int source_y, int source_w, int source_h,
		      int dest_x = 0, int dest_y = 0) const;
  void copy_rectangle(const WMPixmap & source, X::Drawable & dest,
		      const WMRectangle & posn, int dest_x = 0, int dest_y = 0)
	  	      const;
};

inline X::Display *
Xwrapper::xdisplay() const { return xDisplay; }

inline const X::Window
Xwrapper::xrootwin() const { return xRootWindow; }

inline X::GC
Xwrapper::get_GC() const { return xGC; }

#endif
