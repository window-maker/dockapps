
#include <X11/Xlib.h>
#include <stdlib.h>

#ifdef GAI
#include <gai/gai.h>
#else

#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct {
    unsigned char r,g,b,alpha;
} GaiColor;

void gai_display_error_continue(const char *);
GdkPixbuf *gai_load_image(const char *);

#endif

#include "aquarium.h"
#include "draw.h"
#include "grabscreen.h"
#include "background.h"


static Background_settings background_settings;
static AquariumData *ad;

Background_settings *background_get_settings_ptr(void)
{
    return &background_settings;
}

static void background_prepare_solid(unsigned char r, unsigned char g, unsigned b)
{
    int i,j,t;

    for (i = 0; i < ad->ymax; i++) {
	for (j = 0; j < ad->xmax; j++) {
	    t = (i * ad->xmax * 3) + j * 3;
	    ad->bgr[t + 0] = r;
	    ad->bgr[t + 1] = g;
	    ad->bgr[t + 2] = b;
	}
    }

}

static void background_prepare_shaded(unsigned char r1, unsigned char g1, unsigned b1,
				      unsigned char r2, unsigned char g2, unsigned b2)
{
    int i,j,t;
    float d1,d2,d3;

    d1 = ((float)r2 - (float)r1) / (float) ad->ymax;
    d2 = ((float)g2 - (float)g1) / (float) ad->ymax;
    d3 = ((float)b2 - (float)b1) / (float) ad->ymax;

    for (i = 0; i < ad->ymax; i++) {
	for (j = 0; j < ad->xmax; j++) {
	    t = (i * ad->xmax * 3) + j * 3;
	    ad->bgr[t + 0] = (unsigned char)((float)r1 + ((float) i * d1));
	    ad->bgr[t + 1] = (unsigned char)((float)g1 + ((float) i * d2));
	    ad->bgr[t + 2] = (unsigned char)((float)b1 + ((float) i * d3));
	}
    }

}


static void background_prepare_image(char *image_file, int user_choise)
{
    GdkPixbuf *water=NULL, *water2=NULL;
    GError *imerr=NULL;
    unsigned char *buff;
    int i, j, k=0, y, rowstride, alpha;

    if(user_choise){
	water = gdk_pixbuf_new_from_file(image_file, &imerr);
	if (water==NULL){
	    gai_display_error_continue("Can't load image!\nChanging to default water image background.");
	    water = gai_load_image("water.png");
	}
    } else {
	water = gai_load_image("water.png");
    }
    
    water2 = gdk_pixbuf_scale_simple(water, ad->xmax, ad->ymax,
				     GDK_INTERP_BILINEAR);
    buff = gdk_pixbuf_get_pixels(water2);

    rowstride = gdk_pixbuf_get_rowstride(water2);
    alpha = (int)gdk_pixbuf_get_has_alpha(water2);
    
    for (i = 0; i < ad->ymax; i++) {
	y = i * rowstride;
	for (j = 0; j < ((ad->xmax * (3+alpha))); j += (3+alpha)) {
	    ad->bgr[k + 0] = buff[y + j + 0];
	    ad->bgr[k + 1] = buff[y + j + 1];
	    ad->bgr[k + 2] = buff[y + j + 2];
	    k += 3;
	}
    }

    g_object_unref(water);
    g_object_unref(water2);

}

static void background_prepare_desktop(void)
{
    int rowstride, alpha, i, j, k=0, y;
    unsigned char *buff, *str;
    GdkPixbuf *desktop, *tmp_desktop;
    GdkWindow *win;
    Display *display;
    Window xwin;

    win = gdk_get_default_root_window();
    str = getenv("SDL_WINDOWID");

    if(str == NULL){
	xwin = GDK_WINDOW_XWINDOW(win);
    }
    else{
	xwin = (Window)atoi(str);
	display = XOpenDisplay(NULL);
	grab_screen_image(ScreenOfDisplay(display, DefaultScreen(display)), xwin);
    }


    desktop = gdk_pixbuf_get_from_drawable(NULL, 
				      GDK_DRAWABLE(win), 
				      NULL, 0,0,0,0,
				      gdk_screen_width(), gdk_screen_height());

    if(gdk_screen_width() != ad->xmax || gdk_screen_height() != ad->ymax){
	tmp_desktop = gdk_pixbuf_scale_simple(desktop, ad->xmax, ad->ymax,
					      GDK_INTERP_BILINEAR);
	g_object_unref(desktop);
	desktop = tmp_desktop;
    }

    buff = gdk_pixbuf_get_pixels(desktop);

    rowstride = gdk_pixbuf_get_rowstride(desktop);
    alpha = (int)gdk_pixbuf_get_has_alpha(desktop);
    
    for (i = 0; i < ad->ymax; i++) {
	y = i * rowstride;
	for (j = 0; j < ((ad->xmax * (3+alpha))); j += (3+alpha)) {
	    ad->bgr[k + 0] = buff[y + j + 0];
	    ad->bgr[k + 1] = buff[y + j + 1];
	    ad->bgr[k + 2] = buff[y + j + 2];
	    k += 3;
	}
    }

    g_object_unref(desktop);
}

void background_exit(void)
{

}

void background_init(void)
{
    ad = aquarium_get_settings_ptr();

    if(background_settings.desktop){
	background_prepare_desktop();
	return;
    }


    if(background_settings.type == BG_SOLID)
	background_prepare_solid(background_settings.solid_c.r,
				 background_settings.solid_c.g,
				 background_settings.solid_c.b);

    if(background_settings.type == BG_SHADED)
	background_prepare_shaded(background_settings.shaded_top_c.r,
				  background_settings.shaded_top_c.g,
				  background_settings.shaded_top_c.b,
				  background_settings.shaded_bot_c.r,
				  background_settings.shaded_bot_c.g,
				  background_settings.shaded_bot_c.b);

    if(background_settings.type == BG_IMAGE)
	background_prepare_image(background_settings.imagename, 1);

    if(background_settings.type == BG_WATER)
	background_prepare_image(NULL,0);


}
