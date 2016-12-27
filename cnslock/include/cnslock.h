#ifndef CNSLOCK_H_
#define CNSLOCK_H_

#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include "defines.h"

/* this is the max size of the RGB buffer: 56 * 56 * 3
 * used for memcpy, memset, etc operations */
#define RGBSIZE (XMAX * YMAX * 3)

/* main dockapp info structure.  windows, buffers, etc */
typedef struct {
    Display *display;		/* X11 display */
    GdkWindow *win;			/* main window */
    GdkWindow *iconwin;		/* icon window */
    GdkGC *gc;				/* drawing GC */
    GdkPixmap *pixmap;		/* main dockapp pixmap */
    GdkBitmap *mask;		/* dockapp mask */

    /* main image buffer */
    unsigned char rgb[RGBSIZE];

    /* back buffer - stores things we dont want to redraw all the time */
    unsigned char bgr[RGBSIZE];

} AppletData;

extern AppletData ad;
extern int posx;
extern int posy;
#endif



