#include <algorithm>  // for max, min
#include "wmslider.h"
#include "wmapp.h"

void slider_click(const WMApp *a, WMWidget *w, void *)
{
  WMSlider *s;
  if (!(s = dynamic_cast<WMSlider *>(w))) return;

  const WMMouseClick click = a->mouseclick().b_relative_to(w);
  int newval;

  if (click.button > Button3) {
    const int increment = s->total() / 10;
    newval = s->value() + (click.button == Button4 ? increment : -increment);
  }
  else {
    const int xrel = click.x;
    // the messy definition of yrel is due to widget layout using a downward-
    // going y-axis, but WMMeterBars using an upward-going y-axis
    const int yrel = s->b_height() - click.y;
    const int rel = (s->orientation() == Orientation::Horizontal) ? xrel : yrel;
    newval = (rel * s->total()) / s->b_parallel();
  }

  s->setvalue(std::max(0, std::min(s->total(), newval)));
  a->repaint();
}

void
WMSlider::attach_callbacks()
{
  addcallback(slider_click, this, 0);
}

// clearcallbacks() shouldn't delete the main callback function of this class
void
WMSlider::clearcallbacks()
{
  WMCallback::clearcallbacks();
  WMSlider::attach_callbacks();
}
