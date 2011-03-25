#include "../config.h"

/*
    Best viewed with vim5, using ts=4

    wmgeneral was taken from wmppp.

    It has a lot of routines which most of the wm* programs use.

    ------------------------------------------------------------

    Author: Martijn Pieterse (pieterse@xs4all.nl)

    ---
    CHANGES:
    ---
    15/08/2002 (Brad Jorsch, anomie@users.sourceforge.net)
        * Updated createXBMfromXPM to handle the case where the XBM is to be
          wider than the XPM, or the XBM width is not a multiple of 8.
        * Pulled createXBMfromXPM into its own file, because it's the same in
          both -gtk and -x11.

    11/08/2002 (Brad Jorsch, anomie@users.sourceforge.net)
        * Removed the rc-file and mouse region stuff to their own files.
        * Renamed this file to "wmgeneral-x11.c"
        * Renamed a few of the functions

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

#include <stdio.h>

/******************************************************************************\
|* createXBMfromXPM                                                           *|
\******************************************************************************/
void createXBMfromXPM(char *xbm, char **xpm, int sx, int sy) {

    int     i,j,k;
    int     width, height, numcol, depth;
    int     zero=0;
    unsigned char   bwrite;
    int     bcount;
    int     curpixel;

    while(sx&7){ sx++; }

    sscanf(*xpm, "%d %d %d %d", &width, &height, &numcol, &depth);
    width*=depth;

    for (k=0; k!=depth; k++)
    {
        zero <<=8;
        zero |= xpm[1][k];
    }

    for (i=numcol+1; i < numcol+sy+1; i++) {
        bcount = 0;
        bwrite = 0;
        for (j=0; j<sx*depth; j+=depth) {
            bwrite >>= 1;

            if(j<width){
                curpixel=0;
                for (k=0; k!=depth; k++)
                {
                    curpixel <<=8;
                    curpixel |= xpm[i][j+k];
                }
            } else {
                curpixel=zero;
            }

            if ( curpixel != zero ) {
                bwrite += 128;
            }
            bcount++;
            if (bcount == 8) {
                *xbm = bwrite;
                xbm++;
                bcount = 0;
                bwrite = 0;
            }
        }
    }
}
