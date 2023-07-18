
/* 
   This is a GAI example applet written by Jonas Aaberg <cja@gmx.net> 2003-2004.

   It displays the current time. 

   Released under GNU GPL.

*/

#include <time.h>
#include <stdio.h>
#include <gai/gai.h>
#include "config.h"


static GaiColor color;
static int show_sec, ampm;
static GaiPI gai_clock_pref[] = {{GAI_COLORSELECTOR, "Digit colour:", &color, &color},
				 {GAI_CHECKBUTTON, "Show seconds:", &show_sec, &show_sec},
				 {GAI_CHECKBUTTON, "AM/PM format:", &ampm, &ampm},
				 {GAI_END}};

char *create_clock_str(void)
{
    time_t my_t;
    struct tm *my_tm;
    my_t = time(NULL);
    my_tm = localtime(&my_t);

    if(!ampm){
	if(show_sec)
	    return g_strdup_printf("%d:%.2d:%.2d",my_tm->tm_hour, my_tm->tm_min, my_tm->tm_sec);
	else
	    return g_strdup_printf("%d:%.2d",my_tm->tm_hour, my_tm->tm_min);
    } else {
	if(show_sec){
	    if(my_tm->tm_hour >12)
		return g_strdup_printf("%d:%.2d:%.2d PM",my_tm->tm_hour%12, my_tm->tm_min, my_tm->tm_sec);
	    else
		return g_strdup_printf("%d:%.2d:%.2d AM",my_tm->tm_hour, my_tm->tm_min, my_tm->tm_sec);
	} else {
	    if(my_tm->tm_hour >12)
		return g_strdup_printf("%d:%.2d PM",my_tm->tm_hour%12, my_tm->tm_min);
	    else
		return g_strdup_printf("%d:%.2d AM",my_tm->tm_hour, my_tm->tm_min);
	}
    }

}

GdkPixbuf *getclkimage(void)
{
    char *tmp;
    GdkPixbuf *clk_image;
    tmp = create_clock_str();
    clk_image = gai_text_create(tmp, "courier", 10, GAI_TEXT_NORMAL, color.r, color.g, color.b);
    g_free(tmp);
    return clk_image;
}

void update_clock(void)
{

    GdkPixbuf *clk_image;

    clk_image = getclkimage();
    gai_draw_bg(clk_image, 0, 0,
    	        gdk_pixbuf_get_width(clk_image),
		gdk_pixbuf_get_height(clk_image),
	        0, 0);

    gai_draw_update_bg();
    g_object_unref(clk_image);

}

void setbackground()
{
    GdkPixbuf *clk_image;

    clk_image = getclkimage();
    gai_background_set(gdk_pixbuf_get_width(clk_image), 
		       gdk_pixbuf_get_height(clk_image), 
		       gdk_pixbuf_get_height(clk_image), FALSE);

    g_object_unref(clk_image);

}

void gai_clock_settings(gboolean changed, gpointer d)
{

    gai_save_gaicolor("gai-clock/color", color);

    gai_save_int("gai-clock/show_sec", show_sec);
    gai_save_int("gai-clock/ampm", ampm);

    /* for updating the size */
    setbackground();
}

int main(int argc, char **argv)
{
    GaiFlagsType gf = GAI_FLAGS_NEVER_ROTATE;

    gai_init2(&applet_defines, &argc, &argv);

    color = gai_load_gaicolor_with_default("gai-clock/color", (GaiColor){0,0,0,255});

    show_sec = gai_load_int_with_default("gai-clock/show_sec", 1);
    ampm = gai_load_int_with_default("gai-clock/ampm", 0);

    setbackground();
    gai_flags_set(gf);

    gai_preferences2("GAI Digital Clock - Preferences",
		     gai_clock_pref,
		     "Select if you want the seconds to be shown or not,\n"
		     "and if you want the time in AM/PM format or in 24 hour format\n",
		     (GaiCallback1 *)gai_clock_settings,
		     NULL);

    /* Update two times a second */
    gai_signal_on_update((GaiCallback0 *)update_clock, 500, NULL);

    gai_start();

    return 0;
}
