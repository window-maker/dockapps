#include <iostream>
#include "wmtextbar.h"
#include "wmwindow.h"
#include "wmapp.h"

using std::cerr;
using std::endl;

// functions for WMTextBar -----------------------------------------------

void
WMTextBar::real_display()
{
  int charwidth, charheight, xbase, ybase;

  // Blit background color onto rectangle
  WMApp::Xw.fill_rectangle(window()->pixmap(), b_position(),
		  	   WMColor(Background));

  switch (fontsize()) {
    case 0: charwidth = 5, charheight = 9; break;
    case 1: charwidth = 6, charheight = 9; break;
    case 2: charwidth = 7, charheight = 11; break;
    default:
      cerr << "WMError: Text bar " << this << " has bad font size "
	   << fontsize() << endl;
      return;
  }

  if (b_width() < charwidth || b_height() < charheight)
    { cerr << "WMError: Text bar " << this << " too small" << endl; return; }
  
  xbase = b_left(), ybase = b_top();
  // center the characters in the direction perpendicular to their orientation
  if (orientation() == Orientation::Horizontal)
    ybase += (b_height() - charheight) / 2;
  else
    xbase += (b_width() - charwidth) / 2;

  // plot characters onto X display
  for (unsigned int i = 0; xbase + charwidth < b_right()
		&& ybase + charheight < b_bottom(); i++) {
    unsigned char c = text(i);
    if (c > ' ' && c <= 127) // 7-bit printable ASCII
      // note: for a degree character, use ASCII 127 (DEL)
      WMApp::Xw.copy_rectangle(app()->char_pixmaps[fontsize()],
			       window()->pixmap(),
      /* source X-Y coords  */ charwidth * (c % 16), charheight * (c / 16 - 2),
      /* source width/height*/ charwidth, charheight,
      /* destination coords */ xbase, ybase);
    else
      // no pixmap for this character; display a blank.
      WMApp::Xw.copy_rectangle(app()->char_pixmaps[fontsize()],
		      	       window()->pixmap(), 0, 0,
			       charwidth, charheight, xbase, ybase);

    if (orientation() == Orientation::Horizontal)
      xbase += charwidth;
    else
      ybase += charheight;
  }
}

