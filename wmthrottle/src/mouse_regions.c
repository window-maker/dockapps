#include "../config.h"
#include <stdio.h>

/*
 *
 *  My thanks for this handly little piece of code below - saved me
 *  a lot of hassle with the buttons
 *
 *  Anthony Peacock
 *
    Best viewed with vim5, using ts=4

    wmgeneral was taken from wmppp.

    It has a lot of routines which most of the wm* programs use.

    ------------------------------------------------------------

    Author: Martijn Pieterse (pieterse@xs4all.nl)

    ---
    CHANGES:
    ---
    11/08/2002 (Brad Jorsch, anomie@users.sourceforge.net)
        * Moved all the mouse region related stuff to mouse_regions.[ch]

    28/08/2001 (Brad Jorsch, anomie@users.sourceforge.net)
        * Added EnableMouseRegion and DisableMouseRegion
        * Got annoyed with the 81-character lines. Fixed it. If you don't like
          it, find a different copy of wmgeneral.c ;)
        * GraphicsExpose events are enabled here.
        * GetXPM is exported. It optionally takes an XpmColorSymbol array.
        * GetColor is exported.

    30/09/2000 (Brad Jorsch, anomie@users.sourceforge.net)
    * You know, wmgen.mask sounds like a much nicer place to store the
      mask... why don't we do that?

    21/09/1999 (Brad Jorsch, anomie@users.sourceforge.net)
        * Changed openXwindow to use only the filename, sans path,
          as the name and class properties of the app.

    14/09/1998 (Dave Clark, clarkd@skyia.com)
        * Updated createXBMfromXPM routine
        * Now supports >256 colors
    11/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Removed a bug from parse_rcfile. You could
          not use "start" in a command if a label was
          also start.
        * Changed the needed geometry string.
          We don't use window size, and don't support
          negative positions.
    03/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Added parse_rcfile2
    02/09/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Added -geometry support (untested)
    28/08/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Added createXBMfromXPM routine
        * Saves a lot of work with changing xpm's.
    02/05/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * changed the read_rc_file to parse_rcfile, as suggested by Marcelo E. Magallon
        * debugged the parse_rc file.
    30/04/1998 (Martijn Pieterse, pieterse@xs4all.nl)
        * Ripped similar code from all the wm* programs,
          and put them in a single file.

*/

#include "mouse_regions.h"

  /*****************/
 /* Mouse Regions */
/*****************/

typedef struct {
    int     enable;
    int     top;
    int     bottom;
    int     left;
    int     right;
} MOUSE_REGION;

MOUSE_REGION    mouse_region[MAX_MOUSE_REGION];

/******************************************************************************\
|* AddMouseRegion                                                             *|
\******************************************************************************/

void AddMouseRegion(int index, int left, int top, int right, int bottom) {

    if (index < MAX_MOUSE_REGION) {
        mouse_region[index].enable = 1;
        mouse_region[index].top = top;
        mouse_region[index].left = left;
        mouse_region[index].bottom = bottom;
        mouse_region[index].right = right;
    }
}

/******************************************************************************\
|* CheckMouseRegion                                                           *|
\******************************************************************************/

int CheckMouseRegion(int x, int y) {
    int     i;
    int     found;

    found = 0;

    for (i=0; i<MAX_MOUSE_REGION && !found; i++) {
        if (mouse_region[i].enable==1 &&
            x <= mouse_region[i].right &&
            x >= mouse_region[i].left &&
            y <= mouse_region[i].bottom &&
            y >= mouse_region[i].top)
            found = 1;
    }
    if (!found) return -1;
    return (i-1);
}

/******************************************************************************\
|* EnableMouseRegion                                                          *|
\******************************************************************************/

void EnableMouseRegion(int i) {
    if(i<MAX_MOUSE_REGION && mouse_region[i].enable==2)
        mouse_region[i].enable=1;
}

/******************************************************************************\
|* DisableMouseRegion                                                         *|
\******************************************************************************/

void DisableMouseRegion(int i) {
    if(i<MAX_MOUSE_REGION && mouse_region[i].enable==1)
        mouse_region[i].enable=2;
}
