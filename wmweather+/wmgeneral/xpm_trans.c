#include "../config.h"

/*
	Best viewed with vim5, using ts=4

        An add-on to wmgeneral to copy XPM areas with transparency and opacity.

	------------------------------------------------------------

	Author: Brad Jorsch (anomie@users.sourceforge.net)

	---
	CHANGES:
    ---
    16/08/2001 (Brad Jorsch, anomie@users.sourceforge.net)
	* Wrote these routines.

*/

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "wmgeneral-x11.h"

extern int screen;
extern XpmIcon wmgen;
extern GC NormalGC;

static int get_shift(unsigned mask){
    int i=0;

    while(!mask&1){
        mask>>=1;
        i++;
    }
    return i;
}

void combineWithTrans(int sx, int sy, unsigned w, unsigned h, int dx, int dy){
    XImage *pix, *mask;
    unsigned int ww, hh, bar;
    int foo;
    Window baz;
    unsigned x, y;

    XGetGeometry(display, wmgen.pixmap, &baz, &foo, &foo, &ww, &hh, &bar, &bar);
    pix=XGetImage(display, wmgen.pixmap, 0, 0, ww, hh, AllPlanes, ZPixmap);
    XGetGeometry(display, wmgen.mask, &baz, &foo, &foo, &ww, &hh, &bar, &bar);
    mask=XGetImage(display, wmgen.mask, 0, 0, ww, hh, AllPlanes, ZPixmap);

    for(y=0; y<h; y++){
        for(x=0; x<w; x++){
            if(!XGetPixel(mask, sx+x, sy+y)) continue;
            XPutPixel(pix, dx+x, dy+y, XGetPixel(pix, sx+x, sy+y));
        }
    }
    XPutImage(display, wmgen.pixmap, NormalGC, pix, 0, 0, 0, 0, pix->width, pix->height);

    XDestroyImage(pix);
    XDestroyImage(mask);
}

void combineWithOpacity(int sx, int sy, unsigned w, unsigned h, int dx, int dy, int o){
    XImage *pix, *mask;
    unsigned int ww, hh, bar;
    int foo;
    Window baz;
    int rmask, gmask, bmask;
    int rshift, gshift, bshift;
    unsigned long spixel, dpixel;
    unsigned x, y;
    int c_o;

    if(o==0) return;
    if(o==256){
        combineWithTrans(sx, sy, w, h, dx, dy);
        return;
    }

    XGetGeometry(display, wmgen.pixmap, &baz, &foo, &foo, &ww, &hh, &bar, &bar);
    pix=XGetImage(display, wmgen.pixmap, 0, 0, ww, hh, AllPlanes, ZPixmap);
    XGetGeometry(display, wmgen.mask, &baz, &foo, &foo, &ww, &hh, &bar, &bar);
    mask=XGetImage(display, wmgen.mask, 0, 0, ww, hh, AllPlanes, ZPixmap);

    if (pix->depth == DefaultDepth(display, screen)) {{
        Visual *visual=DefaultVisual(display, screen);
        rmask = visual->red_mask;
        gmask = visual->green_mask;
        bmask = visual->blue_mask;
    }} else {
        rmask = pix->red_mask;
        gmask = pix->green_mask;
        bmask = pix->blue_mask;
    }

    c_o=256-o;
    rshift=get_shift(rmask);
    gshift=get_shift(gmask);
    bshift=get_shift(bmask);
/* NOTE: >>s then <<s to prevent overflow when multiplying opacity */
#define AVG(m, s) ((((((spixel&m)>>s)*o+((dpixel&m)>>s)*c_o)>>8)<<s)&m)
    for(y=0; y<h; y++){
        for(x=0; x<w; x++){
            if(!XGetPixel(mask, sx+x, sy+y)) continue;
            spixel=XGetPixel(pix, sx+x, sy+y);
            if(!XGetPixel(mask, dx+x, dy+y)){
                XPutPixel(pix, dx+x, dy+y, spixel);
            } else {
                dpixel=XGetPixel(pix, dx+x, dy+y);
                XPutPixel(pix, dx+x, dy+y,
                          AVG(rmask, rshift) |
                          AVG(gmask, gshift) |
                          AVG(bmask, bshift));
            }
        }
    }
#undef AVG
    XPutImage(display, wmgen.pixmap, NormalGC, pix, 0, 0, 0, 0, pix->width, pix->height);

    XDestroyImage(pix);
    XDestroyImage(mask);
}
