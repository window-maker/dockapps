#include "../wmapp.h"
#include "../wmwindow.h"
#include "../wmframe.h"
#include "../wmcanvas.h"
#include "../wmbutton.h"
#include "../wmellipse.h"
#include "debian-tiny.xpm"

// This file creates a window with a small, very simple paint program.

// Define a class that will be a WMCanvas widget with callbacks
class WMPainter : public WMCallback, public WMCanvas, public WMEllipse {
  public: WMPainter() : WMCallback(), WMCanvas() { }
};

// Make one of the buttons elliptical.
class WMEllipticalButton : public WMEllipse, public WMButton {
  public: WMEllipticalButton() : WMEllipse(), WMButton() { }
};

// Callback to paint onto this widget when it's clicked on
void
paint(const WMApp *a, WMWidget *w, void *)
{
  WMMouseClick click = a->mouseclick().b_relative_to(w);
  WMPainter *p = dynamic_cast<WMPainter *>(w);
  // draw a 3x3 square at the mouse location clicked upon
  if (p) p->fill_rectangle(click.x - 2, click.y - 2, 3, 3);
  a->repaint();
}

// Callbacks for the buttons -----------------------------------------------

// This callback changes the current paint color of the WMCanvas, depending
// upon which button was pressed.
void
changecolor(const WMApp *a, WMWidget *w, void *color)
{
  WMColor::WMColor c = *static_cast<WMColor::WMColor *>(color);
  WMPainter *wp = dynamic_cast<WMPainter *>(w);
  if (wp) wp->setcolor(c);
}

// This callback will be attached to the big button
// on the second window.  It switches between windows.
// Note: if the last statement in a callback function requests
// a switch to a new window, you don't need a "repaint()".
void
switch_to_0(const WMApp *a, void *)
{ a->switch_to(0); }

void makewindow1(WMWindow *w1)
{
  static WMEllipticalButton debianwin;
  static WMEllipticalButton colorbutton[4];
  static WMFrame top, topleft, bottom;
  static WMPainter drawing;
  static int colors[4] = { 0x000000, 0xFF0000, 0x00FF00, 0x0000FF };
  
  w1->addchild(top);
  w1->addchild(bottom);
  w1->setorientation(Orientation::Vertical);
  w1->setaspectratios(1, 3);

  top.addchild(topleft);
  top.addchild(debianwin);
  top.setaspectratios(4, 1);
  
  topleft.setpadding(0);
  topleft.setborder(1);
  topleft.settransparency(false);
  
  for (unsigned int i = 0; i < 4; i++) {
    colorbutton[i].setbgcolor(colors[i]);
    colorbutton[i].addcallback(changecolor, &drawing, colors + i);
    topleft.addchild(colorbutton[i]);
  }
  
  debianwin.seticon(debian_tiny_xpm);
  debianwin.addcallback(switch_to_0, 0);
  
  drawing.addcallback(paint, &drawing);
  bottom.addchild(drawing);
  drawing.setborder(4);
}

