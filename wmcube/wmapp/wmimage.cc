#include <iostream>
#include "wmimage.h"
#include "wmwindow.h"
#include "wmapp.h"

using std::cerr;
using std::endl;

// functions for WMImage -------------------------------------------------

WMImage::WMImage(const WMPixmap *pm)
: wBGColor(WMColor(Background)) { seticon(pm); }

WMImage::WMImage(const WMPixmap &pm)
: wBGColor(WMColor(Background)) { seticon(pm); }

WMImage::WMImage(char *xpm[])
: wBGColor(WMColor(Background)) { seticon(xpm); }

WMImage::~WMImage()
{ if (icon()) WMApp::Xw.free_pixmap(wIcon); }

void
WMImage::seticon(bool dodisplay)
{
  if (icon()) WMApp::Xw.free_pixmap(wIcon);
  if(WMApp::Xw.create_pixmap(wIcon, b_width(), b_height()))
  {
    WMApp::Xw.fill_rectangle(wIcon, 0, 0, b_width(), b_height(), bgcolor());
    wIconPtr = &wIcon;
    if (dodisplay) display();
  }
  else wIconPtr = 0;
}

void
WMImage::seticon(const WMPixmap &pm, bool dodisplay)
{
  if (icon()) WMApp::Xw.free_pixmap(wIcon);
  if (WMApp::Xw.create_pixmap(wIcon, pm))
    { wIconPtr = &wIcon; if (dodisplay) display(); }
  else wIconPtr = 0;
}

void
WMImage::seticon(char *xpm[], bool dodisplay)
{
  if (icon()) WMApp::Xw.free_pixmap(wIcon);
  if (WMApp::Xw.create_pixmap(wIcon, xpm))
    { wIconPtr = &wIcon; if (dodisplay) display(); }
  else wIconPtr = 0;
}

void
WMImage::real_display()
{
  const int imgwidth = icon() ? icon()->attr.width : 0;
  const int imgheight = icon() ? icon()->attr.height : 0;

  // if no icon, create an icon that consists only of the background color
  if (! icon())
    seticon(false);

  // if widget is not big enough to hold icon, complain about it
  else if (imgwidth > b_width() || imgheight > b_height()) {
    cerr << "WMError: Image " << this << " too small for icon" << endl;
    return;
  }

  // if icon is smaller than widget, color in the rest with background color
  else if (imgwidth < b_width() || imgheight < b_height()) {
    WMPixmap temp;
      
    // Blit background color onto rectangle
    WMApp::Xw.create_pixmap(temp, b_width(), b_height());
    WMApp::Xw.fill_rectangle(temp, 0, 0, b_width(), b_height(), bgcolor());
    // center icon onto background
    WMApp::Xw.copy_rectangle(*icon(), temp, 0, 0, imgwidth, imgheight,
		     (b_width() - imgwidth) / 2, (b_height() - imgheight) / 2);
    // swap pixmaps
    WMApp::Xw.free_pixmap(wIcon);
    WMApp::Xw.create_pixmap(wIcon, temp);
    WMApp::Xw.free_pixmap(temp);
  }

  // if we get here, widget (minus border) and icon are the same size -
  // hallelujah!
  WMApp::Xw.copy_rectangle(*icon(), window()->pixmap(), 0, 0,
		  	   b_width(), b_height(), b_left(), b_top());
}

