
#include <stdio.h>
#include <stdlib.h>
#include <gai/gai.h>
#include "aquarium.h"
#include "../config.h"
#include "fish.h"
#include "bubble.h"
#include "date.h"
#include "clock.h"
#include "background.h"
#include "over.h"
#include "leds.h"
#include "bottom.h"
#ifdef XMMS
#include "xmms_sn.h"
#endif
#include "soundeffects.h"
#include "thermometer.h"

/* Over "plugins" */
#include "matrix.h"
#include "plasma.h"
#include "tetris.h"
#include "settings.h"



void bubble_save_settings(void)
{
    Bubble_settings *bubble_settings;

    bubble_settings = bubble_get_settings_ptr();

    gai_save_int("bubble/max_bubbles", bubble_settings->max_bubbles);

}

void thermometer_load_settings(void)
{
    Thermometer_settings *thermometer_settings;

    thermometer_settings = thermometer_get_settings_ptr();

    thermometer_settings->draw1 = gai_load_int_with_default("thermometer/draw1", DRAW_BEFORE);
    thermometer_settings->vert1 = gai_load_int_with_default("thermometer/vert1", BOTTOM);
    thermometer_settings->horz1 = gai_load_int_with_default("thermometer/horz1", RIGHT);
    thermometer_settings->split1 = gai_load_int_with_default("thermometer/split1", FALSE);

    thermometer_settings->c1 = gai_load_gaicolor_with_default("thermometer/c1", (GaiColor){0xe0, 0x00,0x00, 0xff});
    thermometer_settings->c1_s = gai_load_gaicolor_with_default("thermometer/c1_s", (GaiColor){0x00, 0xe0, 0xFF, 0x00});

    thermometer_settings->messure1 = gai_load_int_with_default("thermometer/cpu1", STATUS_CPU);
    thermometer_settings->messure1_s = gai_load_int_with_default("thermometer/cpu1_s", STATUS_OFF);
    thermometer_settings->roof1 = gai_load_int_with_default("thermometer/roof1", 33600/8);
    thermometer_settings->roof1_s = gai_load_int_with_default("thermometer/roof1_s", 33600/8);
    thermometer_settings->mount_point1_s = gai_load_string_with_default("thermometer/mount_point1_s", "/");
    thermometer_settings->mount_point1 = gai_load_string_with_default("thermometer/mount_point1", "/");

    thermometer_settings->draw2 = gai_load_int_with_default("thermometer/draw2", DRAW_BEFORE);
    thermometer_settings->vert2 = gai_load_int_with_default("thermometer/vert2", BOTTOM);
    thermometer_settings->horz2 = gai_load_int_with_default("thermometer/horz2", RIGHT);
    thermometer_settings->split2 = gai_load_int_with_default("thermometer/split2", FALSE);

    thermometer_settings->c2 = gai_load_gaicolor_with_default("thermometer/c2", (GaiColor){0x00, 0xe0, 0x00, 0xff});
    thermometer_settings->c2_s = gai_load_gaicolor_with_default("thermometer/c2_s", (GaiColor){0xe0, 0xe0, 0x00, 0xff});

    thermometer_settings->messure2 = gai_load_int_with_default("thermometer/cpu2", STATUS_OFF);
    thermometer_settings->messure2_s = gai_load_int_with_default("thermometer/cpu2_s", STATUS_OFF);
    thermometer_settings->roof2 = gai_load_int_with_default("thermometer/roof2", 33600/8);
    thermometer_settings->roof2_s = gai_load_int_with_default("thermometer/roof2_s", 33600/8);
    thermometer_settings->mount_point2_s = gai_load_string_with_default("thermometer/mount_point2_s", "/");
    thermometer_settings->mount_point2 = gai_load_string_with_default("thermometer/mount_point2", "/");



}

void thermometer_save_settings(void)
{
    Thermometer_settings *thermometer_settings;

    thermometer_settings = thermometer_get_settings_ptr();

    gai_save_int("thermometer/draw1", thermometer_settings->draw1);
    gai_save_int("thermometer/vert1", thermometer_settings->vert1);
    gai_save_int("thermometer/horz1", thermometer_settings->horz1);
    gai_save_int("thermometer/split1", thermometer_settings->split1);
    gai_save_gaicolor("thermometer/c1",  thermometer_settings->c1);
    gai_save_gaicolor("thermometer/c1_s", thermometer_settings->c1_s);

    gai_save_int("thermometer/cpu1", thermometer_settings->messure1);
    gai_save_int("thermometer/cpu1_s", thermometer_settings->messure1_s);
    gai_save_int("thermometer/roof1", thermometer_settings->roof1);
    gai_save_int("thermometer/roof1_s", thermometer_settings->roof1_s);
    gai_save_string("thermometer/mount_point1_s", thermometer_settings->mount_point1_s);
    gai_save_string("thermometer/mount_point1", thermometer_settings->mount_point1);


    gai_save_int("thermometer/draw2", thermometer_settings->draw2);
    gai_save_int("thermometer/vert2", thermometer_settings->vert2);
    gai_save_int("thermometer/horz2", thermometer_settings->horz2);
    gai_save_int("thermometer/split2", thermometer_settings->split2);
    gai_save_gaicolor("thermometer/c2", thermometer_settings->c2);
    gai_save_gaicolor("thermometer/c2_s", thermometer_settings->c2_s);


    gai_save_int("thermometer/cpu2", thermometer_settings->messure2);
    gai_save_int("thermometer/cpu2_s", thermometer_settings->messure2_s);
    gai_save_int("thermometer/roof2", thermometer_settings->roof2);
    gai_save_int("thermometer/roof2_s", thermometer_settings->roof2_s);
    gai_save_string("thermometer/mount_point2_s", thermometer_settings->mount_point2_s);
    gai_save_string("thermometer/mount_point2", thermometer_settings->mount_point2);



}

void tetris_load_settings(void)
{
    Tetris_settings *tetris_settings;

    tetris_settings = tetris_get_settings_ptr();

    tetris_settings->size_limit = gai_load_bool_with_default("tetris/size_limit", FALSE);
    tetris_settings->show_next = gai_load_bool_with_default("tetris/show_next", TRUE);
    tetris_settings->width = gai_load_int_with_default("tetris/width", 10);
    tetris_settings->height = gai_load_int_with_default("tetris/height", 10);
}    

void tetris_save_settings(void)
{
    Tetris_settings *tetris_settings;

    tetris_settings = tetris_get_settings_ptr();

    gai_save_bool("tetris/size_limit", tetris_settings->size_limit);
    gai_save_int("tetris/width", tetris_settings->width);
    gai_save_int("tetris/height", tetris_settings->height);
    gai_save_bool("tetris/show_next", tetris_settings->show_next);
}    


void bubble_load_settings(void)
{
    Bubble_settings *bubble_settings;

    bubble_settings = bubble_get_settings_ptr();

    bubble_settings->max_bubbles = gai_load_int_with_default("bubble/max_bubbles", 5);

}


void date_load_settings(void)
{
    Date_settings *date_settings;

    date_settings = date_get_settings_ptr();
      
    date_settings->on = gai_load_bool_with_default("date/on", TRUE);

    date_settings->draw = gai_load_int_with_default("date/draw", DRAW_AFTER);


    date_settings->vert = gai_load_bool_with_default("date/vert", TOP);
    date_settings->horz = gai_load_bool_with_default("date/horz", RIGHT);

    date_settings->c = gai_load_gaicolor_with_default("date/color", (GaiColor){208, 208, 208, 80});

}

void date_save_settings(void)
{

    Date_settings *date_settings;

    date_settings = date_get_settings_ptr();

    
    gai_save_bool("date/on", date_settings->on);
    gai_save_bool("date/draw", date_settings->draw);

    gai_save_bool("date/horz", date_settings->horz);
    gai_save_bool("date/vert", date_settings->vert);

    gai_save_gaicolor("date/color", date_settings->c);

}


void clock_load_settings(void)
{
    Clock_settings *clock_settings;

    clock_settings = clock_get_settings_ptr();

    
    clock_settings->type = gai_load_int_with_default("clock/type", CLOCK_OFF);

    clock_settings->analog_seconds = gai_load_bool_with_default("clock/analog_seconds", TRUE);
    clock_settings->digital_seconds = gai_load_bool_with_default("clock/digital_seconds", TRUE);


    clock_settings->draw = gai_load_int_with_default("clock/draw", DRAW_BEFORE);

    clock_settings->vert = gai_load_int_with_default("clock/vert", CENTER);
    clock_settings->horz = gai_load_int_with_default("clock/horz", CENTER);


    /* Set up some mint green colour as default */

    clock_settings->digital_colour = gai_load_gaicolor_with_default("clock/digital_colour", (GaiColor){4, 226, 145, 80});
    clock_settings->digital_blinking = gai_load_bool_with_default("clock/digital_blinking", TRUE);
    clock_settings->digital_fontsize = gai_load_int_with_default("clock/digital_fontsize", CLOCK_SMALL_FONT);


    /* Some yellow colour */
    clock_settings->fuzzy_colour = gai_load_gaicolor_with_default("clock/fuzzy_colour", (GaiColor){226, 226, 4, 80});

    /* Some red version */
    clock_settings->analog_colour_hour = gai_load_gaicolor_with_default("clock/analog_colour_hour", (GaiColor){226, 49, 4, 80});

    /* Green */
    clock_settings->analog_colour_min = gai_load_gaicolor_with_default("clock/analog_colour_min", (GaiColor){23, 226, 4, 80});

    /* Yellow */
    clock_settings->analog_colour_sec = gai_load_gaicolor_with_default("clock/analog_colour_sec", (GaiColor){255, 0, 255, 80});

    clock_settings->analog_keep_circular = gai_load_bool_with_default("clock/analog_keep_circular", TRUE);


}


void clock_save_settings(void)
{
    Clock_settings *clock_settings;

    clock_settings = clock_get_settings_ptr();

    gai_save_int("clock/type", clock_settings->type);

    gai_save_bool("clock/analog_seconds", clock_settings->analog_seconds);
    gai_save_bool("clock/digital_seconds", clock_settings->digital_seconds);

    gai_save_int("clock/draw", clock_settings->draw);
    gai_save_int("clock/horz", clock_settings->horz);
    gai_save_int("clock/vert", clock_settings->vert);

    gai_save_gaicolor("clock/digital_colour", clock_settings->digital_colour);
    gai_save_bool("clock/digital_blinking", clock_settings->digital_blinking);
    gai_save_int("clock/digital_fontsize", clock_settings->digital_fontsize);

    gai_save_gaicolor("clock/fuzzy_colour", clock_settings->fuzzy_colour);

    gai_save_gaicolor("clock/analog_colour_hour",clock_settings->analog_colour_hour);
    gai_save_gaicolor("clock/analog_colour_min", clock_settings->analog_colour_min);
    gai_save_gaicolor("clock/analog_colour_sec", clock_settings->analog_colour_sec);

    gai_save_bool("clock/analog_keep_circular",clock_settings->analog_keep_circular);


}


void background_load_settings(void)
{
    Background_settings *background_settings;

    background_settings = background_get_settings_ptr();


    background_settings->type = gai_load_int_with_default("background/type", BG_SHADED);

    background_settings->imagename = gai_load_string_with_default("background/imagename", "");
    background_settings->imagename_new = NULL;

    background_settings->solid_c = gai_load_gaicolor_with_default("background/solid", (GaiColor){12, 100, 220, 255});
    background_settings->shaded_top_c = gai_load_gaicolor_with_default("background/shaded_top", (GaiColor){83, 155, 220, 255});
    background_settings->shaded_bot_c = gai_load_gaicolor_with_default("background/shaded_bot",(GaiColor){5, 40, 80, 255});
}


void background_save_settings(void)
{
    Background_settings *background_settings;

    background_settings = background_get_settings_ptr();

    gai_save_string("background/imagename", background_settings->imagename);

    gai_save_bool("background/type", background_settings->type);

    gai_save_gaicolor("background/solid", background_settings->solid_c);
    gai_save_gaicolor("background/shaded_top", background_settings->shaded_top_c);
    gai_save_gaicolor("background/shaded_bot", background_settings->shaded_bot_c);

}


void bottom_load_settings(void)
{
    Bottom_settings *bottom_settings;

    bottom_settings = bottom_get_settings_ptr();

    bottom_settings->have_sea_floor = gai_load_bool_with_default("bottom/have_sea_floor", TRUE);
    bottom_settings->max_plants = gai_load_int_with_default("bottom/max_plants", 5);
    bottom_settings->num_bottom_animals = gai_load_int_with_default("bottom/num_bottom_animals", 1);
    bottom_settings->scale = gai_load_int_with_default("bottom/scale", 30);
    bottom_settings->random_plants = gai_load_bool_with_default("bottom/random_plants", TRUE);

}

void bottom_save_settings(void)
{

    Bottom_settings *bottom_settings;

    bottom_settings = bottom_get_settings_ptr();

    gai_save_bool("bottom/have_sea_floor",bottom_settings->have_sea_floor);
    gai_save_bool("bottom/random_plants",bottom_settings->random_plants);
    gai_save_int("bottom/max_plants",bottom_settings->max_plants);
    gai_save_int("bottom/scale",bottom_settings->scale);
    gai_save_int("bottom/num_bottom_animals",bottom_settings->num_bottom_animals);


}




void fish_load_settings(void)
{
    int numfish = 0;
    Fish_settings *fish_settings;
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();
    fish_settings = fish_get_settings_ptr();
    
    fish_settings->eat = gai_load_bool_with_default("fish/eat", TRUE);
    fish_settings->explode = gai_load_bool_with_default("fish/explode", TRUE);

    fish_settings->scale = gai_load_int_with_default("fish/scale", DEFAULT_SCALE);


    fish_settings->speed = gai_load_int_with_default("fish/speed", DEFAULT_SPEED);

    fish_settings->scale_diff = gai_load_bool_with_default("fish/scale_diff", TRUE);
    fish_settings->rebirth = gai_load_bool_with_default("fish/rebirth", FALSE);

    fish_settings->swordfish_agr = gai_load_int_with_default("fish/swordfish_agr", 75);
    fish_settings->hunter_agr = gai_load_int_with_default("fish/hunter_agr", 75);


    fish_settings->num_fish = gai_load_int_with_default("fish/num_fish", NRFISH);
    fish_settings->type = gai_load_int_with_default("fish/type", RANDOM_FISH);


    numfish += fish_settings->fish1 = gai_load_int_with_default("fish/fish1", 1);
    numfish += fish_settings->fish2 = gai_load_int_with_default("fish/fish2", 1);
    numfish += fish_settings->fish3 = gai_load_int_with_default("fish/fish3", 1);
    numfish += fish_settings->fish4 = gai_load_int_with_default("fish/fish4", 1);
    numfish += fish_settings->fish5 = gai_load_int_with_default("fish/fish5", 1);
    numfish += fish_settings->fish6 = gai_load_int_with_default("fish/fish6", 1);
    numfish += fish_settings->swordfish = gai_load_int_with_default("fish/swordfish", 1);
    numfish += fish_settings->blowfish = gai_load_int_with_default("fish/blowfish", 1);
    numfish += fish_settings->fillmore = gai_load_int_with_default("fish/fillmore", 1);
    numfish += fish_settings->sherman = gai_load_int_with_default("fish/sherman", 1);
    numfish += fish_settings->prey = gai_load_int_with_default("fish/prey", 1);
    numfish += fish_settings->hunter = gai_load_int_with_default("fish/hunter", 1);
    numfish += fish_settings->lori = gai_load_int_with_default("fish/lori", 1);
    numfish += fish_settings->ernest = gai_load_int_with_default("fish/ernest", 1);
    numfish += fish_settings->squid = gai_load_int_with_default("fish/squid", 1);
    numfish += fish_settings->megan = gai_load_int_with_default("fish/megan", 1);
    numfish += fish_settings->bdweller = gai_load_int_with_default("fish/bdweller", 1);
    numfish += fish_settings->hawthorne = gai_load_int_with_default("fish/hawthorne", 1);

    if(fish_settings->type == SELECTION_FISH){
	fish_settings->num_fish = numfish;
    }

    if(fish_settings->type == RANDOM_POP_FISH)
	fish_settings->num_fish = g_rand_int_range(ad->rnd, 1, 21);


}




void fish_save_settings(void)
{
    Fish_settings *fish_settings;

    fish_settings = fish_get_settings_ptr();
  
    gai_save_bool("fish/eat", fish_settings->eat);
    gai_save_bool("fish/explode", fish_settings->explode);
    gai_save_int("fish/scale", fish_settings->scale);
    
    gai_save_int("fish/swordfish_agr", fish_settings->swordfish_agr);
    gai_save_int("fish/hunter_agr", fish_settings->hunter_agr);


    gai_save_bool("fish/scale_diff", fish_settings->scale_diff);
    gai_save_int("fish/type",  fish_settings->type);
    gai_save_bool("fish/rebirth", fish_settings->rebirth);


    if(fish_settings->type == SELECTION_FISH)
	fish_settings->num_fish = fish_settings->fish1 + fish_settings->fish2 + fish_settings->fish3 + 
	    fish_settings->fish4 + fish_settings->fish5 + fish_settings->fish6 + fish_settings->swordfish +
	    fish_settings->blowfish + fish_settings->fillmore + fish_settings->sherman + fish_settings->prey +
	    fish_settings->hunter + fish_settings->lori + fish_settings->ernest + fish_settings->squid + 
	    fish_settings->megan;

    gai_save_int("fish/fish1", fish_settings->fish1);
    gai_save_int("fish/fish2", fish_settings->fish2);
    gai_save_int("fish/fish3", fish_settings->fish3);
    gai_save_int("fish/fish4", fish_settings->fish4);
    gai_save_int("fish/fish5", fish_settings->fish5);
    gai_save_int("fish/fish6", fish_settings->fish6);
    gai_save_int("fish/swordfish", fish_settings->swordfish);
    gai_save_int("fish/blowfish", fish_settings->blowfish);
    gai_save_int("fish/fillmore", fish_settings->fillmore);
    gai_save_int("fish/sherman", fish_settings->sherman);
    gai_save_int("fish/prey", fish_settings->prey);
    gai_save_int("fish/hunter", fish_settings->hunter);
    gai_save_int("fish/lori", fish_settings->lori);
    gai_save_int("fish/ernest", fish_settings->ernest);
    gai_save_int("fish/squid", fish_settings->squid);
    gai_save_int("fish/megan", fish_settings->megan);
    gai_save_int("fish/bdweller", fish_settings->bdweller);
    gai_save_int("fish/hawthorne", fish_settings->hawthorne);
    gai_save_int("fish/num_fish", fish_settings->num_fish);


}


void over_load_settings(void)
{
    Over_settings *over_settings;

    over_settings = over_get_settings_ptr();


    over_settings->type = gai_load_int_with_default("over/type", OVER_MATRIX);
    over_settings->fade = gai_load_bool_with_default("over/fade", TRUE);
    over_settings->cursor_off = gai_load_bool_with_default("over/cursor_off", TRUE);

}


void over_save_settings(void)
{
    Over_settings *over_settings;

    over_settings = over_get_settings_ptr();
    gai_save_int("over/type", over_settings->type);
    gai_save_bool("over/fade", over_settings->fade);
    gai_save_bool("over/cursor_off", over_settings->cursor_off);

}

void tetris_load_highscores(void)
{
    int i;
    char tmp[100];

    Tetris_highscore_table *t;
    t = tetris_get_highscore_table_ptr();

    for(i=0;i<10;i++){
	g_snprintf(tmp, 100, "tetris/highscore%d_score", i);
	t[i].score = gai_load_int_with_default(tmp, 10-i);

	g_snprintf(tmp, 100, "tetris/highscore%d_level", i);
	t[i].level = gai_load_int_with_default(tmp, 1);

	g_snprintf(tmp, 100, "tetris/highscore%d_lines", i);
	t[i].lines = gai_load_int_with_default(tmp, 10-i);
    }


}

void tetris_save_highscores(void)
{
    char tmp[100];
    int i;
    Tetris_highscore_table *t;
    t = tetris_get_highscore_table_ptr();

    for(i=0;i<10;i++){
	g_snprintf(tmp, 100, "tetris/highscore%d_score", i);
	gai_save_int(tmp, t[i].score);
	g_snprintf(tmp, 100, "tetris/highscore%d_level", i);
	gai_save_int(tmp, t[i].level);
	g_snprintf(tmp, 100, "tetris/highscore%d_lines", i);
	gai_save_int(tmp, t[i].lines);
    }

}

void leds_load_settings(void)
{
    int i;
    char *tmp;
    Leds_settings *leds_settings;

    leds_settings = leds_get_settings_ptr();

    
    leds_settings->draw = gai_load_int_with_default("leds/draw", DRAW_AFTER);

    leds_settings->horz = gai_load_int_with_default("leds/horz", LEFT);
    leds_settings->vert = gai_load_int_with_default("leds/vert", TOP);


    leds_settings->vert_horz = gai_load_bool_with_default("leds/vert_horz",  TRUE);
    leds_settings->alpha = gai_load_int_with_default("leds/alpha",  0x80);

    for(i=0;i<NUMLEDS;i++){
	tmp = g_strdup_printf("leds/led%d_func", i);
	leds_settings->leds_func[i] = gai_load_int_with_default(tmp, LEDS_OFF);
	g_free(tmp);
	tmp = g_strdup_printf("leds/led%d_colour", i);
	leds_settings->leds_colour[i] = gai_load_int_with_default(tmp, LEDS_VIOLET);
	g_free(tmp);
	tmp = g_strdup_printf("leds/led%d_show_off", i);
	leds_settings->leds_show_off[i] = gai_load_int_with_default(tmp, FALSE);
	g_free(tmp);

    }
}



void leds_save_settings(void)
{
    int i;
    char *tmp;
    Leds_settings *leds_settings;

    leds_settings = leds_get_settings_ptr();

    gai_save_int("leds/draw", leds_settings->draw);

    gai_save_int("leds/vert", leds_settings->vert);
    gai_save_int("leds/horz", leds_settings->horz);

    gai_save_int("leds/vert_horz", leds_settings->vert_horz);
    gai_save_int("leds/alpha",  leds_settings->alpha);

    for(i=0;i<NUMLEDS;i++){
	tmp = g_strdup_printf("leds/led%d_func",  i);
	gai_save_int(tmp,  leds_settings->leds_func[i]);
	g_free(tmp);

	tmp = g_strdup_printf("leds/led%d_colour", i);
	gai_save_int(tmp, leds_settings->leds_colour[i]);
	g_free(tmp);

	tmp = g_strdup_printf("leds/led%d_show_off", i);
	gai_save_int(tmp, leds_settings->leds_show_off[i]);
	g_free(tmp);
    }



}
#ifdef XMMS
void xmms_sn_load_settings(void)
{
    Xmms_sn_settings *xmms_sn_settings;

    xmms_sn_settings = xmms_sn_get_settings_ptr();


    xmms_sn_settings->on = gai_load_bool_with_default("xmms_sn/on", FALSE);

    xmms_sn_settings->c = gai_load_gaicolor_with_default("xmms_sn/colour", (GaiColor){45, 145, 245, 255});

    xmms_sn_settings->draw = gai_load_int_with_default("xmms_sn/draw", DRAW_AFTER);

    xmms_sn_settings->direction = gai_load_int_with_default("xmms_sn/direction", XMMS_SN_HORIZONTAL);
    xmms_sn_settings->speed = gai_load_int_with_default("xmms_sn/speed", 200);


    xmms_sn_settings->horz = gai_load_int_with_default("xmms_sn/horz", CENTER);
    xmms_sn_settings->vert = gai_load_int_with_default("xmms_sn/vert", LEFT);

    xmms_sn_settings->fb = gai_load_int_with_default("xmms_sn/fb", XMMS_SN_FORWARDS);

}

void xmms_sn_save_settings(void)
{
    Xmms_sn_settings *xmms_sn_settings;

    xmms_sn_settings = xmms_sn_get_settings_ptr();


    gai_save_bool("xmms_sn/on", xmms_sn_settings->on);

    gai_save_gaicolor("xmms_sn/colour", xmms_sn_settings->c);

    gai_save_int("xmms_sn/draw", xmms_sn_settings->draw);
    gai_save_int("xmms_sn/direction", xmms_sn_settings->direction);
    gai_save_int("xmms_sn/speed", xmms_sn_settings->speed);

    gai_save_int("xmms_sn/horz", xmms_sn_settings->horz);
    gai_save_int("xmms_sn/vert", xmms_sn_settings->vert);
    gai_save_int("xmms_sn/fb", xmms_sn_settings->fb);


    
}
#endif

void sound_load_settings(void)
{
    Sound_settings *sound_settings;

    sound_settings = sound_get_settings_ptr();

    sound_settings->on = gai_load_bool_with_default("sound/on", FALSE);

    sound_settings->type = gai_load_int_with_default("sound/type", TYPE_MP3);


    sound_settings->prg = gai_load_string_with_default("sound/prg", "mpg123 -q");

}

void sound_save_settings(void)
{
    Sound_settings *sound_settings;

    sound_settings = sound_get_settings_ptr();

    gai_save_bool("sound/on", sound_settings->on);
    gai_save_int("sound/type", sound_settings->type);


    gai_save_string("sound/prg", sound_settings->prg);

}

void general_load_settings(void)
{
    General_settings *general_settings;	
    general_settings = general_get_settings_ptr();
    general_settings->ratio_width = gai_load_int_with_default("general/ratio_width", 1);
    general_settings->ratio_height = gai_load_int_with_default("general/ratio_height", 1);

    general_settings->mouse_left = gai_load_int_with_default("general/mouse_left", MOUSE_OFF);
    general_settings->mouse_middle = gai_load_int_with_default("general/mouse_middle", MOUSE_OFF);
    general_settings->mouse_up = gai_load_int_with_default("general/mouse_up", MOUSE_OFF);
    general_settings->mouse_down = gai_load_int_with_default("general/mouse_down", MOUSE_OFF);

    general_settings->mouse_left_option = gai_load_string_with_default("general/mouse_left_option", "");
    general_settings->mouse_middle_option = gai_load_string_with_default("general/mouse_middle_option", "");
    general_settings->mouse_up_option = gai_load_string_with_default("general/mouse_up_option", "");
    general_settings->mouse_down_option = gai_load_string_with_default("general/mouse_down_option", "");

}

void general_save_settings(void)
{
    General_settings *general_settings;	
    general_settings = general_get_settings_ptr();

    gai_save_int("general/ratio_width", general_settings->ratio_width);
    gai_save_int("general/ratio_height", general_settings->ratio_height);

    gai_save_int("general/mouse_left",  general_settings->mouse_left);
    gai_save_int("general/mouse_middle",  general_settings->mouse_middle);
    gai_save_int("general/mouse_up", general_settings->mouse_up);
    gai_save_int("general/mouse_down", general_settings->mouse_down);

    gai_save_string("general/mouse_left_option", general_settings->mouse_left_option);
    gai_save_string("general/mouse_middle_option", general_settings->mouse_middle_option);
    gai_save_string("general/mouse_up_option", general_settings->mouse_up_option);
    gai_save_string("general/mouse_down_option", general_settings->mouse_down_option);

}
