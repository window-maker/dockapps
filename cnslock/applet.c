#include <stdio.h>

#include "include/defines.h"
#include "include/cnslock.h"
#include "include/applet.h"

#include "graphics/master.xpm"

/* This function makes the dockapp window */
void make_new_cnslock_dockapp(int manager_style)
{
#define MASK GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK

    GdkWindowAttr attr;
    GdkWindowAttr attri;

    Window win;
    Window iconwin;
    XWMHints wmhints;
    XSizeHints sizehints;

    memset(&attr, 0, sizeof(GdkWindowAttr));

    attr.width = WINDOWSIZE_X;
    attr.height = WINDOWSIZE_Y;
    attr.x = 100;
    attr.y = 100;
    attr.title = "cnslockapplet";
    attr.event_mask = MASK;
    attr.wclass = GDK_INPUT_OUTPUT;
    attr.visual = gdk_visual_get_system();
    attr.colormap = gdk_colormap_get_system();
    attr.wmclass_name = "cnslockapplet";
    attr.wmclass_class = "cnslockapplet";
    attr.window_type = GDK_WINDOW_TOPLEVEL;

    /* make a copy for the iconwin - parameters are the same */
    memcpy(&attri, &attr, sizeof(GdkWindowAttr));

    ad.win = gdk_window_new(NULL, &attr, GDK_WA_TITLE | GDK_WA_WMCLASS | GDK_WA_VISUAL | GDK_WA_COLORMAP | GDK_WA_X |GDK_WA_Y);
    if (!ad.win)
	{
		fprintf(stderr, "FATAL: cannot make toplevel window\n");
		exit(1);
    }

    ad.iconwin = gdk_window_new(ad.win, &attri, GDK_WA_TITLE | GDK_WA_WMCLASS | GDK_WA_VISUAL | GDK_WA_COLORMAP | GDK_WA_X |GDK_WA_Y);
    if (!ad.iconwin)
	{
		fprintf(stderr, "FATAL: cannot make icon window\n");
		exit(1);
    }

	if(manager_style==0)
	{
	    win = GDK_WINDOW_XWINDOW(ad.win);
    	iconwin = GDK_WINDOW_XWINDOW(ad.iconwin);
		sizehints.flags = USSize;
		sizehints.width = WINDOWSIZE_X;
		sizehints.height = WINDOWSIZE_Y;
		XSetWMNormalHints(GDK_WINDOW_XDISPLAY(ad.win), win, &sizehints);
		wmhints.initial_state = WithdrawnState;
		wmhints.icon_window = iconwin;
		wmhints.icon_x = 0;
		wmhints.icon_y = 0;
		wmhints.window_group = win;
		wmhints.flags =	StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
		XSetWMHints(GDK_WINDOW_XDISPLAY(ad.win), win, &wmhints);
	}

    ad.gc = gdk_gc_new(ad.win);
    ad.pixmap =	gdk_pixmap_create_from_xpm_d(ad.win, &(ad.mask), NULL, master_xpm);
    gdk_window_shape_combine_mask(ad.win, ad.mask, 0, 0);
    gdk_window_shape_combine_mask(ad.iconwin, ad.mask, 0, 0);
    gdk_window_set_back_pixmap(ad.win, ad.pixmap, False);
    gdk_window_set_back_pixmap(ad.iconwin, ad.pixmap, False);
    gdk_window_show(ad.win);

    if(posx!=-1 && posy!=-1)
		gdk_window_move(ad.win, posx, posy);
		
#undef MASK
}
