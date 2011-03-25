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
    11/08/2002 (Brad Jorsch, anomie@users.sourceforge.net)
        * Moved all the rc-file related stuff to rcfile.[ch]

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
#include <string.h>
#include <stdlib.h>

#include "rcfile.h"

/******************************************************************************\
|* parse_rcfile                                                               *|
\******************************************************************************/

void parse_rcfile(const char *filename, rckeys *keys) {

    char    *p,*q;
    char    temp[128];
    char    *tokens = " :\t\n";
    FILE    *fp;
    int     i,key;

    fp = fopen(filename, "r");
    if (fp) {
        while (fgets(temp, 128, fp)) {
            key = 0;
            q = strdup(temp);
            q = strtok(q, tokens);
            while (key >= 0 && keys[key].label) {
                if ((!strcmp(q, keys[key].label))) {
                    p = strstr(temp, keys[key].label);
                    p += strlen(keys[key].label);
                    p += strspn(p, tokens);
                    if ((i = strcspn(p, "#\n"))) p[i] = 0;
                    free(*keys[key].var);
                    *keys[key].var = strdup(p);
                    key = -1;
                } else key++;
            }
            free(q);
        }
        fclose(fp);
    }
}

/******************************************************************************\
|* parse_rcfile2                                                              *|
\******************************************************************************/

void parse_rcfile2(const char *filename, rckeys2 *keys) {

    char    *p;
    char    temp[128];
    char    *tokens = " :\t\n";
    FILE    *fp;
    int     i,key;
    char    *family = NULL;

    fp = fopen(filename, "r");
    if (fp) {
        while (fgets(temp, 128, fp)) {
            key = 0;
            while (key >= 0 && keys[key].label) {
                if ((p = strstr(temp, keys[key].label))) {
                    p += strlen(keys[key].label);
                    p += strspn(p, tokens);
                    if ((i = strcspn(p, "#\n"))) p[i] = 0;
                    free(*keys[key].var);
                    *keys[key].var = strdup(p);
                    key = -1;
                } else key++;
            }
        }
        fclose(fp);
    }
    free(family);
}
