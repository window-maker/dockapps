/*
 *   xutils.c - A collection of X-windows utilties for creating WindowMAker
 *		DockApps.
 *
 *     This file contains alot of the lower-level X windows routines. Origins with wmppp
 *     (by  Martijn Pieterse (pieterse@xs4all.nl)), but its been hacked up quite a bit
 *     and passed on from one new DockApp to the next.
 *
 *
 *
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2, or (at your option)
 *      any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program (see the file COPYING); if not, write to the
 *      Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *      Boston, MA  02111-1307, USA
 *
 *
 *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/XKBlib.h>
#include "xutils.h"



/*
 *   X11 Variables
 */
int		x_fd;
XSizeHints	mysizehints;
XWMHints	mywmhints;
Pixel		back_pix, fore_pix;
char		*Geometry = "";
GC		NormalGC;
XpmIcon		wmgen;
Pixmap		pixmask;


/*
 *  Colors for wmCalClock
 */
extern char    TimeColor[30];
extern char    BackgroundColor[30];



/* Images for keyboard layouts */
XImage *kb_orig_img[4];
XImage *kb_trans_img[4];
extern int KbLoadedImgs;
extern float KbTransparency;

#define KB_X_OFFSET	5
#define KB_Y_OFFSET	24
#define KB_DES_WIDTH	54
#define KB_DES_HEIGHT	35
#define KB_MAX_GROUPS	4



/*
 *   flush_expose
 */
static int flush_expose(Window w) {

    XEvent 	dummy;
    int		i=0;

    while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
    i++;

    return i;

}









/*
 *   RedrawWindow
 *   RedrawWindowXY
 */
void RedrawWindow(void) {

    flush_expose(iconwin);
    XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, 0,0, wmgen.attributes.width, wmgen.attributes.height, 0, 0);

    flush_expose(win);
    XCopyArea(display, wmgen.pixmap, win, NormalGC, 0,0, wmgen.attributes.width, wmgen.attributes.height, 0, 0);

}

void RedrawWindowXY(int x, int y) {

    flush_expose(iconwin);
    XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, x,y, wmgen.attributes.width, wmgen.attributes.height, 0, 0);

    flush_expose(win);
    XCopyArea(display, wmgen.pixmap, win, NormalGC, x,y, wmgen.attributes.width, wmgen.attributes.height, 0, 0);

}







/*
 *   copyXPMArea
 *   copyXBMArea
 */
void copyXPMArea(int x, int y, int sx, int sy, int dx, int dy) {
	XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}

void copyXBMArea(int x, int y, int sx, int sy, int dx, int dy) {

	XCopyArea(display, wmgen.mask, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}



/*
 *   initXwindow
 */
void initXwindow(int argc, char *argv[]){

    int		 i;
    char	*display_name = NULL;

    for (i=1; argv[i]; ++i) {
        if (!strcmp(argv[i], "-display")) display_name = argv[i+1];
    }


    if (!(display = XOpenDisplay(display_name))) {
        fprintf(stderr, "%s: can't open display %s\n",
                argv[0], XDisplayName(display_name));
        exit(1);
    }


    screen  	 = DefaultScreen(display);
    Root    	 = RootWindow(display, screen);
    DisplayDepth = DefaultDepth(display, screen);
    x_fd    	 = XConnectionNumber(display);

}





/*
 *   openXwindow
 */
void openXwindow(int argc, char *argv[], char *pixmap_bytes[], char *pixmask_bits,
    int pixmask_width, int pixmask_height) {

    unsigned int	borderwidth = 1;
    XClassHint		classHint;
    char		*wname = argv[0];
    XTextProperty	name;
    XGCValues		gcv;
    unsigned long	gcm;
    int			dummy=0, red, grn, blu;
    XpmColorSymbol 	cols[10]={	{"Back", NULL, 0},
    					{"Color1", NULL, 0},
    					{"Color2", NULL, 0},
    					{"Color3", NULL, 0},
    					{"Color4", NULL, 0},
    					{"Color5", NULL, 0},
    					{"Color6", NULL, 0},
    					{"Color7", NULL, 0},
    					{"Color8", NULL, 0},
    					{"Color9", NULL, 0}};




    /*
     *  Create Pixmap
     */
    cols[0].pixel		  = getColor(BackgroundColor, 1.0000, &red, &grn, &blu);
    cols[1].pixel		  = getBlendedColor(TimeColor, 0.1522, red, grn, blu);
    cols[2].pixel		  = getBlendedColor(TimeColor, 0.2602, red, grn, blu);
    cols[3].pixel		  = getBlendedColor(TimeColor, 0.3761, red, grn, blu);
    cols[4].pixel		  = getBlendedColor(TimeColor, 0.4841, red, grn, blu);
    cols[5].pixel		  = getBlendedColor(TimeColor, 0.5922, red, grn, blu);
    cols[6].pixel		  = getBlendedColor(TimeColor, 0.6980, red, grn, blu);
    cols[7].pixel		  = getBlendedColor(TimeColor, 0.7961, red, grn, blu);
    cols[8].pixel		  = getBlendedColor(TimeColor, 0.8941, red, grn, blu);
    cols[9].pixel		  = getBlendedColor(TimeColor, 1.0000, red, grn, blu);

    wmgen.attributes.numsymbols   = 10;
    wmgen.attributes.colorsymbols = cols;
    wmgen.attributes.exactColors  = False;
    wmgen.attributes.closeness    = 40000;
    wmgen.attributes.valuemask    = XpmReturnPixels | XpmReturnExtensions | XpmColorSymbols
							| XpmExactColors | XpmCloseness | XpmSize;
    if (XpmCreatePixmapFromData(display, Root, pixmap_bytes,
      &(wmgen.pixmap), &(wmgen.mask), &(wmgen.attributes)) != XpmSuccess){
	fprintf(stderr, "Not enough free colorcells.\n");
	exit(1);
    }




    /*
     *  Create a window
     */
    mysizehints.flags = USSize | USPosition;
    mysizehints.x = 0;
    mysizehints.y = 0;

    back_pix = getColor("white", 1.0, &red, &grn, &blu);
    fore_pix = getColor("black", 1.0, &red, &grn, &blu);

    XWMGeometry(display, screen, Geometry, NULL, borderwidth, &mysizehints,
				&mysizehints.x, &mysizehints.y,&mysizehints.width,&mysizehints.height, &dummy);

    mysizehints.width = 64;
    mysizehints.height = 64;



    win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,
				mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

    iconwin = XCreateSimpleWindow(display, win, mysizehints.x, mysizehints.y,
				mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);



    /*
     *  Activate hints
     */
    XSetWMNormalHints(display, win, &mysizehints);
    classHint.res_name = wname;
    classHint.res_class = wname;
    XSetClassHint(display, win, &classHint);



    /*
     *  Set up the xevents that you want the relevent windows to inherit
     *  Currently, its seems that setting KeyPress events here has no
     *  effect. I.e. for some you will need to Grab the focus and then return
     *  it after you are done...
     */
    XSelectInput(display, win, ButtonPressMask | ExposureMask | ButtonReleaseMask
		| PointerMotionMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask
						| KeyPressMask | KeyReleaseMask);
    XSelectInput(display, iconwin, ButtonPressMask | ExposureMask | ButtonReleaseMask
		| PointerMotionMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask
						| KeyPressMask | KeyReleaseMask);


    if (XStringListToTextProperty(&wname, 1, &name) == 0) {
        fprintf(stderr, "%s: can't allocate window name\n", wname);
        exit(1);
    }


    XSetWMName(display, win, &name);

    /*
     *   Create Graphics Context (GC) for drawing
     */
    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = fore_pix;
    gcv.background = back_pix;
    gcv.graphics_exposures = 0;
    NormalGC = XCreateGC(display, Root, gcm, &gcv);



    pixmask = XCreateBitmapFromData(display, win, pixmask_bits, pixmask_width, pixmask_height);
    XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
    XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);


    mywmhints.initial_state = WithdrawnState;
    mywmhints.icon_window = iconwin;
    mywmhints.icon_x = mysizehints.x;
    mywmhints.icon_y = mysizehints.y;
    mywmhints.window_group = win;
    mywmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;


    XSetWMHints(display, win, &mywmhints);


    XSetCommand(display, win, argv, argc);
    XMapWindow(display, win);

}

unsigned long getColor(char *ColorName, float fac, int *red, int *grn, int *blu) {

     XColor 		Color;
     XWindowAttributes 	Attributes;

     XGetWindowAttributes(display, Root, &Attributes);
     Color.pixel = 0;

     XParseColor(display, Attributes.colormap, ColorName, &Color);
     Color.red 	 = (unsigned short)(fac*(Color.red-24) + 24);
     Color.blue  = (unsigned short)(fac*(Color.blue-24) + 24);
     Color.green = (unsigned short)(fac*(Color.green-24) + 24);
     Color.flags = DoRed | DoGreen | DoBlue;
     XAllocColor(display, Attributes.colormap, &Color);


     *red = Color.red;
     *grn = Color.green;
     *blu = Color.blue;
     return Color.pixel;

}

unsigned long getBlendedColor(char *ColorName, float fac, int red, int grn, int blu) {

     XColor 		Color;
     XWindowAttributes 	Attributes;

     XGetWindowAttributes(display, Root, &Attributes);
     Color.pixel = 0;

     XParseColor(display, Attributes.colormap, ColorName, &Color);
     Color.red 	 = (unsigned short)(fac*(Color.red-red) + red);
     Color.blue  = (unsigned short)(fac*(Color.blue-grn) + grn);
     Color.green = (unsigned short)(fac*(Color.green-blu) + blu);
     Color.flags = DoRed | DoGreen | DoBlue;
     XAllocColor(display, Attributes.colormap, &Color);

     return Color.pixel;

}


int LoadKbImg(char *names)
{
     char tmp[1024], *point = NULL, aux[1024];
     int i;
     XpmAttributes attr;

     tmp[1023] = '\0';
     strncpy(tmp, names, 1024);
     if (tmp[1023] != '\0') {
         fprintf(stderr, "wmCalClock: Error parsing image names for keyboard layouts.\n");

	 return (-1);
     }

     /* load images from file */
     point = strtok(tmp, ",");
     for (i = 0; i < KB_MAX_GROUPS; i++) {
	 if (point) {
	     if ((point[0] == '.') || (point[0] == '/')) {
	         strcpy(aux, point);
	     } else {
	         strcpy(aux, KB_PIXMAP_PREFIX);
		 strncat(aux, point, 1024 - strlen(aux) - 1);
	     }
	     attr.valuemask = 0;
	     if (XpmReadFileToImage(display, aux, &kb_orig_img[i], NULL, &attr) != XpmSuccess) {
	         fprintf(stderr, "wmCalClock: Could not read flag image %s.\n",
			 point);

		 return (-1);
	     }
	     if ((attr.width != KB_DES_WIDTH) || (attr.height != KB_DES_HEIGHT)) {
	         fprintf(stderr, "wmCalClock: %s hasn't %dx%d dimension.",
		         point, KB_DES_WIDTH, KB_DES_HEIGHT);

		 return (-1);
	     }
	     kb_trans_img[i] = XSubImage(kb_orig_img[i], 0, 0, KB_DES_WIDTH, KB_DES_HEIGHT);
	 } else {
	     break;
	 }
         point = strtok(NULL, ",");
     }


     return ((i > KB_MAX_GROUPS) ? KB_MAX_GROUPS : i);
}


void CreateKbTranImgs(void)
{
     int i, j, k;
     XImage *image;
     XColor c_o, c_n;	/* original and new color */
     XWindowAttributes attributes;
     float KT = KbTransparency;

     XGetWindowAttributes(display, Root, &attributes);
     image = XGetImage(display, wmgen.pixmap, KB_X_OFFSET, KB_Y_OFFSET,
		       KB_DES_WIDTH, KB_DES_HEIGHT, -1, ZPixmap);

     for (i = 0; i < KB_DES_WIDTH; i++) {
         for (j = 0; j < KB_DES_HEIGHT; j++) {
	     c_o.pixel = XGetPixel(image, i, j);
	     XQueryColor(display, attributes.colormap, &c_o);

	     for (k = 0; k < KbLoadedImgs; k++) {
	         c_n.pixel = XGetPixel(kb_orig_img[k], i, j);
		 XQueryColor(display, attributes.colormap, &c_n);
		 c_n.red = c_o.red * (1 - KT) + c_n.red * KT;
		 c_n.green = c_o.green * (1 - KT) + c_n.green * KT;
		 c_n.blue = c_o.blue * (1 - KT) + c_n.blue * KT;
		 XAllocColor(display, attributes.colormap, &c_n);
		 XPutPixel(kb_trans_img[k], i, j, c_n.pixel);
	     }
	 }
     }

     XDestroyImage(image);
}


void ShowGroupImage(int group)
{
     XPutImage(display, wmgen.pixmap, NormalGC, kb_trans_img[group],
	    0, 0, KB_X_OFFSET, KB_Y_OFFSET, KB_DES_WIDTH, KB_DES_HEIGHT);
}
