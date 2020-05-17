#include <iostream>
#include "xwrapper.h"
#include "wmwidget.h"
using std::cerr;
using std::endl;

// This class includes most of the crap normally required to do X stuff.

Xwrapper::Xwrapper()
{
  xDisplay = X::XOpenDisplay(0);
  if (!xDisplay) { cerr << "Xwrapper: Could not open X display!" << endl; }
  xRootWindow = DefaultRootWindow(xDisplay);
  X::XGetWindowAttributes(xDisplay, xRootWindow, &xAttributes);

  X::XGCValues gcv;
  gcv.graphics_exposures = false;
  xGC = X::XCreateGC(xDisplay, xRootWindow,
		     X_MACRO(GCGraphicsExposures), &gcv);
}

Xwrapper::~Xwrapper()
{
  //XXX: This is only the beginning...
  X::XFreeGC(xDisplay, xGC);
  X::XCloseDisplay(xDisplay);
}

void
Xwrapper::set_GC(X::GC gc, Color c) const
{
  X::XSetForeground(xDisplay, gc, color_to_xcolor(c));
  X::XSetFillStyle(xDisplay, gc, X_MACRO(FillSolid));
}

unsigned long
Xwrapper::color_to_xcolor(Color c) const
{
  X::XColor xcolor;

  xcolor.pixel = 0;
  xcolor.flags = X_MACRO(DoRed | DoGreen | DoBlue);
  // map the range 0 -> 0xff to the range 0 -> 0xffff
  // (X insists on using "short" for color values)
  xcolor.red =   c.r() * 0x0101;
  xcolor.green = c.g() * 0x0101;
  xcolor.blue =  c.b() * 0x0101;

  if (X::XAllocColor(xDisplay, xAttributes.colormap, &xcolor))
    X::XFreeColors(xDisplay, xAttributes.colormap, &xcolor.pixel, 1, 0);
    //XXX: This is an incredibly stupid solution, but it works above 8-bit
    //color. It appears that decent color management is a shortcoming of X.
  return xcolor.pixel;
}

Color 
Xwrapper::get_point(const WMPixmap& src, int x, int y) const
{
  //magic incantaions to read a pixel from a pixmap
  X::XGCValues gcvalues;
  X::XGetGCValues(xDisplay, src.gc, GCPlaneMask, &gcvalues);

  // XXX: Surely there's a better way than allocating an image every time
  X::XImage* dest = XGetImage(xDisplay, src.pixmap, 0, 0, src.attr.width, src.attr.height, gcvalues.plane_mask, XYPixmap);

  Color color = XGetPixel(dest, x, y);
  XDestroyImage(dest);
  return color;
}

bool
Xwrapper::create_pixmap(WMPixmap & dest, char * pixmap_bytes[]) const
{
  dest.attr.exactColors = false;
  dest.attr.closeness = 40000;
  dest.attr.valuemask = X_MACRO(XpmExactColors | XpmCloseness |
				XpmReturnPixels | XpmReturnExtensions);
  int error = X::XpmCreatePixmapFromData(xDisplay, xRootWindow, pixmap_bytes,
		  			 &dest.pixmap, &dest.mask, &dest.attr);
  if (error == X_MACRO(XpmSuccess)) {
    X::XGCValues gcv;
    gcv.clip_mask = dest.mask;
    dest.gc = X::XCreateGC(xDisplay, dest.pixmap, X_MACRO(GCClipMask), &gcv);
    return true;
  }
  else return false;
}

bool
Xwrapper::create_pixmap(WMPixmap & dest, const WMPixmap & source) const
{
  create_pixmap(dest, source.attr.width, source.attr.height);
  copy_rectangle(source, dest, 0, 0, source.attr.width, source.attr.height,
		 0, 0);
  return true;
}

bool
Xwrapper::create_pixmap(WMPixmap & dest, const WMPixmap * source) const
{ return source ? create_pixmap(dest, *source) : false; }

bool
Xwrapper::create_pixmap(WMPixmap & dest, int width, int height) const
{
  dest.attr.width = width;
  dest.attr.height = height;
  dest.pixmap = X::XCreatePixmap(xDisplay, xRootWindow, width, height,
				 DefaultDepth(xDisplay,
				 DefaultScreen(xDisplay)));
  dest.mask = 0; //hope XCreateFoo never returns 0
  X::XGCValues gcv;
  dest.gc = X::XCreateGC(xDisplay, dest.pixmap, X_MACRO(None), &gcv);
  return true;
}

bool
Xwrapper::create_pixmap(WMPixmap & dest, int width, int height, int depth) const
{
  dest.attr.width = width;
  dest.attr.height = height;
  dest.pixmap = X::XCreatePixmap(xDisplay, xRootWindow, width, height, depth);
  dest.mask = 0; //hope XCreateFoo never returns 0
  X::XGCValues gcv;
  dest.gc = X::XCreateGC(xDisplay, dest.pixmap, X_MACRO(None), &gcv);
  return true;
}

void
Xwrapper::free_pixmap(WMPixmap & dest) const
{
  X::XFreeGC(xDisplay, dest.gc);
  X::XFreePixmap(xDisplay, dest.pixmap);
  if (dest.mask) X::XFreePixmap(xDisplay, dest.mask);
}

void
Xwrapper::draw_point(WMPixmap & dest, int x, int y, Color c) const
{
  set_GC(dest.gc, c);
  X::XDrawPoint(xDisplay, dest.pixmap, dest.gc, x, y);
}

void
Xwrapper::draw_line(WMPixmap & dest, int x1, int y1, int x2, int y2,
		    Color c) const
{
  set_GC(dest.gc, c);
  X::XDrawLine(xDisplay, dest.pixmap, dest.gc, x1, y1, x2, y2);
}

void
Xwrapper::draw_arc(WMPixmap & dest, int x, int y, int width, int height,
        int angle1, int angle2, Color c)
{
  set_GC(dest.gc, c);
  X::XDrawArc(xDisplay, dest.pixmap, dest.gc, x, y, width, height, angle1,
      angle2);
}

void
Xwrapper::draw_lines(WMPixmap & dest, const X::XPoint* points, int npoints,
        Color c) const
{
  set_GC(dest.gc, c);
  X::XDrawLines(xDisplay, dest.pixmap, dest.gc, const_cast<X::XPoint*>(points),
      npoints, X_MACRO(CoordModeOrigin));
}

void    
Xwrapper::draw_horizontal_gradient(WMPixmap & dest, int x1, int y1, int x2,
        int y2, Color c1, Color c2, double amount)
{
  if (!(x2 - x1) || !amount)
    return; //gradient isn't wide enough, or none of it is to be drawn      
  if (x1 > x2) //put x1 on left while keeping colors straight
  {
    int tempx = x2; x2 = x1; x1 = tempx;
    Color tempc = c2; c2 = c1; c1 = tempc;
  }

  int glinesm1 = x2 - x1;
  int glines = glinesm1 + 1; //number of lines in the full gradient
  double dlines = glines * amount; //number of lines that we want to show
  int tdlines = static_cast<int> (dlines); //number of lines truncated to int
  double residue = dlines - tdlines; //amount left over from integer truncation
  for (int x = 0; x < tdlines; x++)
    draw_line(dest, x1 + x, y1, x1 + x, y2, Color::alpha_blend(c2, c1,
          1.0 * x / glinesm1));
  if (tdlines < glines) //correct undrawn line by using alpha blending
  {
    Color backcolor = get_point(dest, x1 + tdlines, y1);
    Color forecolor = Color::alpha_blend(c2, c1, 1.0 * tdlines / glinesm1);
    Color color = Color::alpha_blend(forecolor, backcolor, residue);
    draw_line(dest, x1 + tdlines, y1, x1 + tdlines, y2, color);
  }
}

void
Xwrapper::draw_vertical_gradient(WMPixmap & dest, int x1, int y1, int x2,
                                int y2, Color c1, Color c2, double amount)
{
  if (!(y2 - y1) || !amount)
    return; //gradient isn't high enough, or none of it is to be drawn      
  if (y1 > y2) //put y1 on left while keeping colors straight
  {
    int tempy = y2; y2 = y1; y1 = tempy;
    Color tempc = c2; c2 = c1; c1 = tempc;
  }

  int glinesm1 = y2 - y1;
  int glines = glinesm1 + 1; //number of lines in the full gradient
  double dlines = glines * amount; //number of lines that we want to show
  int tdlines = static_cast<int> (dlines); //number of lines truncated to int
  double residue = dlines - tdlines; //amount left over from integer truncation
  for (int y = 0; y < tdlines; y++)
    draw_line(dest, x1, y1 + y, x2, y1 + y, Color::alpha_blend(c2, c1,
          1.0 * y / glinesm1));
  if (tdlines < glines) //correct undrawn line by using alpha blending
  {
    Color backcolor = get_point(dest, x1, y1 + tdlines);
    Color forecolor = Color::alpha_blend(c2, c1, 1.0 * tdlines / glinesm1);
    Color color = Color::alpha_blend(forecolor, backcolor, residue);
    draw_line(dest, x1, y1 + tdlines, x2, y1 + tdlines, color);
  }
}

void
Xwrapper::draw_border(WMPixmap & dest, int x, int y, int width, int height,
		      int thickness, Color topleft, Color botright,
		      Color corner) const
{
  if (width / 2 < thickness || height / 2 < thickness) return;

  fill_rectangle(dest, x, y, width, thickness, topleft);
  fill_rectangle(dest, x, y, thickness, height, topleft);
  fill_rectangle(dest, x, y + height - thickness, width, thickness, botright);
  fill_rectangle(dest, x + width - thickness, y, thickness, height, botright);
  fill_rectangle(dest, x, y + height - thickness, thickness, thickness, corner);
  fill_rectangle(dest, x + width - thickness, y, thickness, thickness, corner);
}

void
Xwrapper::draw_border(WMPixmap & dest, const WMRectangle & r, int thickness,
		      Color topleft, Color botright, Color corner) const
{
  draw_border(dest, r.left(), r.top(), r.width(), r.height(), thickness,
	      topleft, botright, corner);
}

void
Xwrapper::fill_rectangle(WMPixmap & dest, int x, int y,
			 int width, int height, Color c) const
{
  set_GC(dest.gc, c);
  X::XFillRectangle(xDisplay, dest.pixmap, dest.gc, x, y, width, height);
}

void
Xwrapper::fill_rectangle(WMPixmap & dest, const WMRectangle & r, Color c)
const
{ fill_rectangle(dest, r.left(), r.top(), r.width(), r.height(), c); }

void
Xwrapper::fill_arc(WMPixmap & dest, int x, int y, int width, int height,
        int angle1, int angle2, Color c)
{
  set_GC(dest.gc, c);
  X::XFillArc(xDisplay, dest.pixmap, dest.gc, x, y, width, height, angle1,
      angle2);
}

void
Xwrapper::fill_polygon(WMPixmap & dest, const X::XPoint* points, int npoints,
        Color c, XShape shape) const
{
  set_GC(dest.gc, c);
  X::XFillPolygon(xDisplay, dest.pixmap, dest.gc,
      const_cast<X::XPoint*>(points), npoints, shape,
      X_MACRO(CoordModeOrigin));
}

void
Xwrapper::clear_rectangle(WMPixmap & dest, int x, int y,
			  int width, int height) const
{
  X::XClearArea(xDisplay, dest.pixmap, x, y, width, height,
		/* don't generate expose events */ false);
}

void
Xwrapper::clear_rectangle(WMPixmap & dest, const WMRectangle & r) const
{ clear_rectangle(dest, r.left(), r.top(), r.width(), r.height()); }

void
Xwrapper::copy_rectangle(const WMPixmap & source, WMPixmap & dest,
			 int source_x, int source_y,
			 int source_w, int source_h,
			 int dest_x, int dest_y) const
{
  X::XCopyArea(xDisplay, source.pixmap, dest.pixmap, dest.gc, source_x,
      source_y, source_w, source_h, dest_x, dest_y);
}

void
Xwrapper::copy_rectangle(const WMPixmap & source, WMPixmap & dest,
			 const WMRectangle & r, int dest_x, int dest_y) const
{
  copy_rectangle(source, dest, r.left(), r.top(), r.width(), r.height(),
		 dest_x, dest_y);
}

void
Xwrapper::copy_rectangle(const WMPixmap & source, X::Drawable & dest,
			 int source_x, int source_y,
			 int source_w, int source_h,
			 int dest_x, int dest_y) const
{
  X::XCopyArea(xDisplay, source.pixmap, dest, xGC, source_x, source_y,
               source_w, source_h, dest_x, dest_y);
}

void
Xwrapper::copy_rectangle(const WMPixmap & source, X::Drawable & dest,
			 const WMRectangle & r, int dest_x, int dest_y) const
{
  copy_rectangle(source, dest, r.left(), r.top(), r.width(), r.height(),
		 dest_x, dest_y);
}

