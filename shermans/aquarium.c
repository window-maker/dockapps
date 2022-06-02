/* 
 * Sherman's aquarium - Applet v3.0.1
 *
 * (Formely known as aquarium applet.)
 *
 * Copyright (C) 2002-2003 Jonas Aaberg <cja@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You did not receive a copy of the GNU General Public License along with
 * this program; chances are, you already have at least 10 copies of this
 * license somewhere on your system.  If you still want it, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, 
 * MA 02111-1307, USA.
 *
 * This applet was once based upon timecop's <timecop@japan.co.jp> fishmon, 
 * but not much is now left of his code. (draw.c is mainly his code)
 *
 *
 */


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gai/gai.h>


#define GAI_APPLET_DEFINES
#include "../config.h"
#include "aquarium.h"
#include "draw.h"
#include "settings.h"

/* Features */
#include "clock.h"
#include "date.h"
#include "bubble.h"
#include "over.h"
#include "leds.h"
#include "thermometer.h"
#include "fish.h"
#include "background.h"
#ifdef XMMS
#include "xmms_sn.h"
#endif
#include "soundeffects.h"
#include "status.h"
#include "bottom.h"
#include "pref.h"


/* keeps track of mouse focus */
static int over_ruled;
static AquariumData ad;
static General_settings general_settings;

/* Only there for screensaver compability */
int fullscreen = 0, window_id = -1;


static void aquarium_enter(gpointer d)
{
    ad.proximity = 1;
    fish_enter();
}

static void aquarium_keypress(int key, gpointer d)
{
    if(ad.proximity)
	over_keypress(key);
}

static void aquarium_joystick(GaiFlagsJoystick flags, gpointer d)
{
    if(ad.proximity)
	over_joystick(flags);
}

static void aquarium_leave(gpointer d)
{
    ad.proximity = 0;
    fish_leave();
}

static void mouse_action(int flag, char *option)
{
    if(flag == MOUSE_OFF)
	return;

    if(flag == MOUSE_RUN){
	gai_exec(option);
	return;
    }
}

static void aquarium_mouseclick_left(int x, int y, gpointer d)
{
    mouse_action(general_settings.mouse_left, general_settings.mouse_left_option);
}

static void aquarium_mouseclick_middle(int x, int y, gpointer d)
{
    mouse_action(general_settings.mouse_middle, general_settings.mouse_middle_option);
}

static void aquarium_mouseclick_updown(int dir, gpointer d)
{
    if(dir == GDK_SCROLL_UP)
	mouse_action(general_settings.mouse_up, general_settings.mouse_up_option);
    if(dir == GDK_SCROLL_DOWN)
	mouse_action(general_settings.mouse_down, general_settings.mouse_down_option);
}



void aquarium_change(int orient, int new_w, int new_h, gpointer d)
{
    static int old_xmax = -1, old_ymax = -1;
    ad.dont_update = 1;

    ad.ymin = gai_scale(YMIN);
    ad.xmin = gai_scale(XMIN);
    
    if(gai_applet_mode() == GAI_DOCKAPP){
	if(general_settings.ratio_width > general_settings.ratio_height){
	    ad.xmax = gai_get_size()*general_settings.ratio_width/general_settings.ratio_height - 2*ad.xmin;
	    ad.ymax = gai_get_size() - 2*ad.ymin;
	} else {
	    ad.xmax = gai_get_size() - 2*ad.xmin;
	    ad.ymax = gai_get_size()*general_settings.ratio_height/general_settings.ratio_width - 2*ad.ymin;
	}
	if(old_ymax != ad.ymax || old_xmax != ad.xmax)
	    gai_background_set(ad.xmax+2*ad.xmin, ad.ymax+2*ad.ymin, GAI_BACKGROUND_MAX_SIZE_NONE, TRUE);
    } else {
	if(orient == GAI_HORIZONTAL){
	    if(general_settings.ratio_width > general_settings.ratio_height){
		ad.xmax = gai_get_size()*general_settings.ratio_width/general_settings.ratio_height - 2*ad.xmin;
		ad.ymax = gai_get_size() - 2*ad.ymin;
	    } else {
		ad.ymax = gai_get_size() - 2*ad.ymin;
		ad.xmax = gai_get_size()*general_settings.ratio_height/general_settings.ratio_width - 2*ad.xmin;
	    }
	    if(old_ymax != ad.ymax || old_xmax != ad.xmax)
		gai_background_set(ad.xmax+2*ad.xmin, ad.ymax+2*ad.ymin, GAI_BACKGROUND_MAX_SIZE_NONE, TRUE);
	} else {
	    /* Vertical */
	    if(general_settings.ratio_width > general_settings.ratio_height){
		ad.ymax = gai_get_size() * general_settings.ratio_width/general_settings.ratio_height - 2*ad.ymin;
		ad.xmax = gai_get_size() - 2*ad.xmin;
	    } else {
		ad.xmax = gai_get_size() - 2*ad.xmin;
		ad.ymax = gai_get_size()*general_settings.ratio_height/general_settings.ratio_width - 2*ad.ymin;
	    }
	    if(old_ymax != ad.ymax || old_xmax != ad.xmax)
		gai_background_set(ad.ymax+2*ad.ymin,ad.xmax+2*ad.xmin, GAI_BACKGROUND_MAX_SIZE_NONE, TRUE);
	}
    
}
    old_ymax = ad.ymax;
    old_xmax = ad.xmax;

    ad.virtual_aquarium_x = ad.xmax + 2 * VIRTUAL_AQUARIUM_DX;
    ad.virtual_aquarium_y = ad.ymax + 2 * VIRTUAL_AQUARIUM_DX;

    g_free(ad.rgb);
    g_free(ad.bgr);

    ad.rgb = g_malloc0((ad.xmax + 2) * (ad.ymax + 2) * 3);
    ad.bgr = g_malloc0((ad.xmax + 2) * (ad.ymax + 2) * 3);

    background_init();
    bottom_init();

    ad.dont_update = 0;

}



void prepare_graphics(void)
{

    clock_init();
    date_init();

    bubble_init();

    thermometer_init();

    leds_init();
#ifdef XMMS
    xmms_sn_init();
#endif
    fish_init();

    background_init();
    bottom_init();

    status_init();
    over_init();

}


void aquarium_update(gpointer d)
{
    static gboolean inited = 0;
  /* If this function is called during reload of images,
       just quit and wait until that is done. */

    if(ad.dont_update)
	return;

    if(!inited){
	inited = TRUE;
	if(gai_applet_mode() == GAI_DOCKAPP){
	    if(ad.xmax < ad.ymax){
		aquarium_change(gai_get_orient(), 
				gai_get_size(),
				gai_get_size()*general_settings.ratio_height/general_settings.ratio_width,
				NULL);
	    } else {
		aquarium_change(gai_get_orient(), 
				gai_get_size()*general_settings.ratio_width/general_settings.ratio_height,
				gai_get_size(),
				NULL);
	    }
	} else {
	    if(gai_get_orient() == GAI_HORIZONTAL){
		aquarium_change(gai_get_orient(), 
				gai_get_size(),
				gai_get_size()*general_settings.ratio_height/general_settings.ratio_width,
				NULL);

	    }
	    else {
		aquarium_change(gai_get_orient(), 
				gai_get_size()*general_settings.ratio_width/general_settings.ratio_height,
				gai_get_size(),
				NULL);
	    }
	}
    }

    if(ad.drawingarea == NULL)
	ad.drawingarea = gai_get_drawingarea();
    
    memcpy(ad.rgb, ad.bgr, ad.ymax * ad.xmax * 3);

    /* If the mouse over function is completly domiating,
       then, only call the mouse over function */

    if(!over_ruled) {
	leds_update(DRAW_BEFORE);
	thermometer_update(DRAW_BEFORE);
	clock_update(DRAW_BEFORE);
#ifdef XMMS
	xmms_sn_update(DRAW_BEFORE);
#endif
	date_update(DRAW_BEFORE);
	

	fish_update();
	bubble_update();


	thermometer_update(DRAW_AFTER);
	clock_update(DRAW_AFTER);
#ifdef XMMS
	xmms_sn_update(DRAW_AFTER);
#endif
	date_update(DRAW_AFTER);
	leds_update(DRAW_AFTER); 
    }

    over_ruled = over_update(ad.proximity);

    ad.gc = gai_get_gc();

    /* Draw it, not using gai */
    gdk_draw_rgb_image(ad.drawingarea->window, 	ad.gc, ad.xmin, ad.ymin, ad.xmax, ad.ymax,
		       GDK_RGB_DITHER_NONE,ad.rgb, ad.xmax*3);
    gdk_flush();
}
  


void aquarium_free_all()
{

    background_exit();
    clock_exit();
    date_exit();
    over_exit();
#ifdef XMMS
    xmms_sn_exit();
#endif
    thermometer_exit();
    status_exit();
    bubble_exit();
    fish_exit();
    bottom_exit();

    over_exit();
    
}

void aquarium_reload_all()
{
    background_init();
    clock_init();
    date_init();
    status_init();
#ifdef XMMS
    xmms_sn_init();
#endif
    thermometer_init();
    bubble_init();
    /* We are loading fishes before aquarium_change gets activated, since it needs to call bottom and bg */
    //    printf("fish init\n");
    //fish_init();
    bottom_init();		/* Must be after fish, because it draw bottom creatures. */
    over_init();

}

/* This function (re)sets all the global variables to their default */
/* Some globals, don't have a default, those are not set ofcourse */

void init(void)
{

    memset(&ad, 0, sizeof(AquariumData));

    ad.rnd = g_rand_new();

    clock_load_settings();
    date_load_settings();
#ifdef XMMS
    xmms_sn_load_settings();
#endif

    bubble_load_settings();

    thermometer_load_settings();
    over_load_settings();
    leds_load_settings();
    

    background_load_settings();
    bottom_load_settings();

    fish_load_settings();
    general_load_settings();
    sound_load_settings();
    tetris_load_settings();

    /* Default at start up is horizontal gnome-panel */
    ad.ymax = YMAX;
    ad.xmax = XMAX;

    if(general_settings.ratio_width > general_settings.ratio_height){
	ad.xmax = (XMAX+2*XMIN)*general_settings.ratio_width/general_settings.ratio_height - 2*XMIN;
    } else {
	ad.ymax = (YMAX+2*YMIN)*general_settings.ratio_height/general_settings.ratio_width - 2*YMIN;
    }

    ad.ymin = YMIN;
    ad.xmin = XMIN;

    ad.virtual_aquarium_x = XMAX + 2 * VIRTUAL_AQUARIUM_DX;
    ad.virtual_aquarium_y = YMAX + 2 * VIRTUAL_AQUARIUM_DY;
    ad.viewpoint_start_x = VIEWPOINT_START_X;
    ad.viewpoint_start_y = VIEWPOINT_START_Y;

    ad.proximity = 0;
    over_ruled = 0;

    ad.posx=ad.posy=-1;

    ad.gc = NULL;
    ad.drawingarea = NULL;

    ad.special_action = 0;
}




int main(int argc, char **argv)
{

    gai_init2(&applet_defines ,&argc, &argv);


    /* Load saved settings and reset others */
    init();

    ad.rgb = g_malloc0((ad.xmax + 2) * (ad.ymax + 2) * 3);
    ad.bgr = g_malloc0((ad.xmax + 2) * (ad.ymax + 2) * 3);

    prepare_graphics();

    pref_init();

    gai_background_set(ad.xmax+2*ad.xmin, ad.ymax+2*ad.ymin, GAI_BACKGROUND_MAX_SIZE_NONE, TRUE);

    gai_preferences2("Sherman's aquarium - Preferences", 
		    shermans_pref, 
		    "You cannot do anything dangerous with this applet,\n"
		     "so just play around and look at what the applet does.\n"
		     "If you still have some questions, feel free to mail me, cja@gmx.net.\n\n"
		     "Thanks for trying Sherman's aquarium!\n\n"
		     "http://aquariumapplet.sourceforge.net",
		    (GaiCallback0 *)pref_restart, NULL);

    /* 20 frames per second */
    gai_signal_on_update((GaiCallback0 *)aquarium_update, 1000/50, NULL);
    gai_signal_on_enter((GaiCallback0 *)aquarium_enter, NULL);
    gai_signal_on_leave((GaiCallback0 *)aquarium_leave, NULL);
    gai_signal_on_change((GaiCallback3 *)aquarium_change, NULL);
    gai_signal_on_keypress((GaiCallback1 *)aquarium_keypress, NULL);
    gai_signal_on_joystick((GaiCallback1 *)aquarium_joystick, NULL);

    gai_signal_on_mouse_button_click((GaiCallback2 *)aquarium_mouseclick_left, GAI_MOUSE_BUTTON_1, NULL);
    gai_signal_on_mouse_button_click((GaiCallback2 *)aquarium_mouseclick_middle, GAI_MOUSE_BUTTON_2, NULL);
    gai_signal_on_scroll_buttons((GaiCallback1 *)aquarium_mouseclick_updown, NULL);


    gai_start();
    return 0;
}


AquariumData *aquarium_get_settings_ptr(void)
{
    return &ad;
}

unsigned char *aquarium_install_path(void)
{
    return applet_defines.image_path;
}

General_settings *general_get_settings_ptr(void)
{
    return &general_settings;
}


void aquarium_draw_image(int x, int y, int idx, int rev, SA_Image *image)
{
    draw_image(x, y, idx, rev, image);
}

void aquarium_draw_pic_alpha(SA_Image *image, int w, int h, int x, int y, int idx, int alpha)
{
    draw_pic_alpha(image->image, w, h, x, y, idx, alpha);
} 


void aquarium_clean_image(int x, int y, int w, int h)
{

}

/* This is just to keep background.c happy */
void grab_screen_image (Screen *s, Window w)
{

}
