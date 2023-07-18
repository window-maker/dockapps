
/*

  Preference window for Sherman's aquarium, 
  using the General Applet Interface.

  Preference generator v2 interface.

*/

#include <stdio.h>
#include <gai/gai.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "../config.h"

#include "clock.h"
#include "date.h"
#include "background.h"
#include "soundeffects.h"
#include "bottom.h"
#include "bubble.h"
#include "fish.h"
#include "over.h"
#include "leds.h"
#include "thermometer.h"
#ifdef XMMS
#include "xmms_sn.h"
#endif
#include "settings.h"
#include "status.h"
#include "tetris.h"

static Date_settings date_settings;
static Clock_settings clock_settings;
static Background_settings background_settings;
static Sound_settings sound_settings;
static Bottom_settings bottom_settings;
static Bubble_settings bubble_settings;
static Fish_settings fish_settings;
static Over_settings over_settings;
static Leds_settings leds_settings;
static Thermometer_settings thermometer_settings;
static Tetris_settings tetris_settings;
#ifdef XMMS
static Xmms_sn_settings xmms_settings;
#endif
static General_settings general_settings;

static char *draw_list[] = {"Background", "Forground",NULL};
static char *vert_list[] = {"Top", "Center", "Bottom", NULL};
static char *horz_list[] = {"Left", "Center", "Right", NULL};
static char *vert_horz_list[] = {"Vertical", "Horizontal", NULL};
#ifdef XMMS
static char *xmms_scroll_list[] = {"Vertical","Horizontal",NULL};
#endif


static char *clock_type_list[] = {"Off", "Analog", "Digital", "Fuzzy",NULL};
static char *clock_fontsize_list[] = {"Large", "Small", NULL};
static char *bg_names[] = {"Solid colour", "Shaded colour", "Wateralike", "Image", NULL};
static char *sound_type[] = {"Use Mp3 files for sound effects", "Use Ogg files for sound effects", NULL};
static char *leds_func_list[] = {"Off", "Numlock", "Capslock", "Scrollock", NULL};
static char *leds_colour_list[] = {"Blue", "Blue-Green", "Orange", "Red", "Violet", "Yellow", "Pink", "Green", "Dark Blue", "Light blue", "Yellow-Green", NULL};
static char *mouse_list[] = {"Off", "Executes", NULL};

static GaiSS l_0_1000_1 = {0, 1000, 1}, l_0_100_1 = {0,100,1}, l_10_1000_5 = {10, 1000, 5}, l_0_50_1 = {0,50,1};
static GaiSS l_1_20_1 = {1, 20, 1},  l_0_200_1 = {0, 200, 1}, l_0_100_5 = {0, 100, 5}, l_0_1000000_1 = {0,1000000,1};
static GaiSS l_0_255_1 = {0, 255, 1}, l_4_50_1 = {4, 50, 1};

static char *fish_radio[] = {"User defined selection and population", 
			     "Random selection and given population size", 
			     "Random selection and random population size", 
			     NULL};

static char *over_radio[] = {"Nothing", "Matrix scroller", "Plasma", "Tetris", NULL};


static char *messure_list[] = {
                               "Off",
#ifdef STATUS_HAVE_FAN1
			       "Fan 1",
#endif 
#ifdef STATUS_HAVE_FAN2
			       "Fan 2",
#endif 
#ifdef STATUS_HAVE_TEMP1
			       "Thermometer 1 (CPU)",
#endif 
#ifdef STATUS_HAVE_TEMP2
			       "Thermometer 2 (Motherboard)",
#endif 
#ifdef STATUS_HAVE_SWAP
			       "Swap usage",
#endif 
#ifdef STATUS_HAVE_DISC
			       "Disc usage",
#endif 
#ifdef STATUS_HAVE_CPU
			       "CPU load",
#endif 
#ifdef STATUS_HAVE_MEM
			       "Memory usage",
#endif 
#ifdef STATUS_HAVE_NET_ETH0
			       "Network ETH0 (1st network card) recieve",
			       "Network ETH0 (1st network card) send",
			       "Network ETH0 (1st network card) mixed",
#endif 
#ifdef STATUS_HAVE_NET_ETH1
			       "Network ETH1 (2nd network card) recieve",
			       "Network ETH1 (2nd network card) send",
			       "Network ETH1 (2nd network card) mixed",

#endif 
#ifdef STATUS_HAVE_NET_PPP0
			       "Network: PPP0 (Dial-up modem) recieve",
			       "Network: PPP0 (Dial-up modem) send",
			       "Network: PPP0 (Dial-up modem) mixed",
#endif 
#ifdef STATUS_HAVE_NET_LO
			       "Network: Loopback recieve",
			       "Network: Loopback send",
			       "Network: Loopback mixed",
#endif
			       NULL}; 


GaiPI shermans_pref[] = {



/*--------------------------------------------------------------------------------*/
/* General */
/*--------------------------------------------------------------------------------*/



{GAI_NOTEBOOK, "General"},
{GAI_FRAME, "Sound format"},
     {GAI_CHECKBUTTON, "Sound effects",&sound_settings.on, &sound_settings.on},
     {GAI_TEXTENTRY, "External sound playing program: ", &sound_settings.prg, &sound_settings.prg},
     {GAI_RADIOBUTTON, sound_type, &sound_settings.type, &sound_settings.type},
{GAI_FRAME_E},

{GAI_FRAME_R, "Applet size ratio"},
     {GAI_SPINBUTTON, "Width:",  &general_settings.ratio_width,  &general_settings.ratio_width, &l_1_20_1},
     {GAI_SPINBUTTON, "Height:",  &general_settings.ratio_height,  &general_settings.ratio_height, &l_1_20_1},
     {GAI_TEXT,  "The ratios might be compromised\n"
            "in order to fit the gnome panel.\n"
            "When the gnome panel is in vertical\n"
	    "mode, the ratios will be rotated 90\n"
            "degrees."},
{GAI_FRAME_E},
{GAI_SPINBUTTON, "Max bubbles", &bubble_settings.max_bubbles,  &bubble_settings.max_bubbles, &l_0_1000_1},

{GAI_FRAME, "Bottom"},
     {GAI_CHECKBUTTON, "Have sea floor", &bottom_settings.have_sea_floor, &bottom_settings.have_sea_floor},
     {GAI_CHECKBUTTON, "Random number of plants up to the number below", &bottom_settings.random_plants, &bottom_settings.random_plants},
     {GAI_SPINBUTTON, "Number of plants:",  &bottom_settings.max_plants,&bottom_settings.max_plants, &l_0_100_1},
     {GAI_SPINBUTTON, "Scale  of plants:",  &bottom_settings.scale,&bottom_settings.scale, &l_10_1000_5},
     {GAI_SPINBUTTON, "Number of bottom animals:", &bottom_settings.num_bottom_animals,  &bottom_settings.num_bottom_animals, &l_0_50_1},
{GAI_FRAME_E},

/*--------------------------------------------------------------------------------*/
/* Background settings */
/*--------------------------------------------------------------------------------*/

{GAI_FRAME_R, "Background"},
     {GAI_RADIOBUTTON, bg_names, &background_settings.type,  &background_settings.type},
     {GAI_COLORSELECTOR, "Solid colour", &background_settings.solid_c, &background_settings.solid_c}, // r1
     {GAI_COLORSELECTOR, "Shaded - Top colour", &background_settings.shaded_top_c, &background_settings.shaded_top_c}, /// r2, r2
     {GAI_COLORSELECTOR, "Shaded - Bottom colour", &background_settings.shaded_bot_c, &background_settings.shaded_bot_c},
     {GAI_FILESELECTOR, "Background image", &background_settings.imagename, &background_settings.imagename_new},
{GAI_FRAME_E},

{GAI_NOTEBOOK_E},

/*--------------------------------------------------------------------------------*/
/* Mouse */
/*--------------------------------------------------------------------------------*/

{GAI_NOTEBOOK, "Mouse"},
{GAI_FRAME, "Mouse clicks"},

{GAI_FRAME, "Left button"},
{GAI_OPTIONMENU, "Does: ", &general_settings.mouse_left, &general_settings.mouse_left, mouse_list},
{GAI_TEXTENTRY, "Option: ", &general_settings.mouse_left_option, &general_settings.mouse_left_option},
{GAI_FRAME_E},
{GAI_FRAME_R, "Middle button"},
{GAI_OPTIONMENU, "Does: ", &general_settings.mouse_middle, &general_settings.mouse_middle, mouse_list},
{GAI_TEXTENTRY, "Option: ", &general_settings.mouse_middle_option, &general_settings.mouse_middle_option},
{GAI_FRAME_E},

{GAI_FRAME, "Scroll button up"},
{GAI_OPTIONMENU, "Does: ", &general_settings.mouse_up, &general_settings.mouse_up, mouse_list},
{GAI_TEXTENTRY, "Option: ", &general_settings.mouse_up_option, &general_settings.mouse_up_option},
{GAI_FRAME_E},
{GAI_FRAME_R, "Scroll button button down"},
{GAI_OPTIONMENU, "Does: ", &general_settings.mouse_down, &general_settings.mouse_down, mouse_list},
{GAI_TEXTENTRY, "Option: ", &general_settings.mouse_down_option, &general_settings.mouse_down_option},
{GAI_FRAME_E},


{GAI_FRAME_E},
{GAI_NOTEBOOK_E},


/*--------------------------------------------------------------------------------*/
/* Time */
/*--------------------------------------------------------------------------------*/
{GAI_NOTEBOOK, "Time"},
{GAI_FRAME, "Date"},
     {GAI_CHECKBUTTON, "On", &date_settings.on, &date_settings.on},
     {GAI_OPTIONMENU, "Draw on:", &date_settings.draw, &date_settings.draw, draw_list},
     {GAI_OPTIONMENU, "Vertical placement:", &date_settings.vert, &date_settings.vert, vert_list},
     {GAI_OPTIONMENU, "Horizontal placement:", &date_settings.horz, &date_settings.horz, horz_list}, 
     {GAI_COLORSELECTOR, "Font colour",&date_settings.c,&date_settings.c},
{GAI_FRAME_E},

{GAI_FRAME, "Clock"},
     {GAI_OPTIONMENU, "Vertical placement:", &clock_settings.vert, &clock_settings.vert, vert_list},
     {GAI_OPTIONMENU, "Horizontal placement:", &clock_settings.horz, &clock_settings.horz, horz_list},
     {GAI_OPTIONMENU, "Draw on:", &clock_settings.draw, &clock_settings.draw, draw_list},
     {GAI_OPTIONMENU, "Clock type:", &clock_settings.type, &clock_settings.type,clock_type_list},
     {GAI_FRAME, "Analog"},
          {GAI_CHECKBUTTON, "Keep the clock circular", &clock_settings.analog_keep_circular, &clock_settings.analog_keep_circular},
          {GAI_COLORSELECTOR, "Colour of the hour pointer",&clock_settings.analog_colour_hour, &clock_settings.analog_colour_hour},
          {GAI_COLORSELECTOR, "Colour of the minute pointer",&clock_settings.analog_colour_min, &clock_settings.analog_colour_min},
          {GAI_COLORSELECTOR, "Colour of the second pointer", &clock_settings.analog_colour_sec, &clock_settings.analog_colour_sec},
          {GAI_CHECKBUTTON, "With seconds",  &clock_settings.analog_seconds, &clock_settings.analog_seconds},
     {GAI_FRAME_E},
     {GAI_FRAME_R, "Digital"},
          {GAI_OPTIONMENU, "Font size:", &clock_settings.digital_fontsize, &clock_settings.digital_fontsize, clock_fontsize_list},
          {GAI_CHECKBUTTON, "Make the colon blink", &clock_settings.digital_blinking, &clock_settings.digital_blinking},
          {GAI_COLORSELECTOR, "Font colour", &clock_settings.digital_colour, &clock_settings.digital_colour},
          {GAI_CHECKBUTTON, "With seconds", &clock_settings.digital_seconds, &clock_settings.digital_seconds},
     {GAI_FRAME_E},
     {GAI_FRAME, "Fuzzy"},
          {GAI_COLORSELECTOR, "Font colour", &clock_settings.fuzzy_colour, &clock_settings.fuzzy_colour},
     {GAI_FRAME_E},
{GAI_FRAME_E},
{GAI_NOTEBOOK_E},


/*--------------------------------------------------------------------------------*/
/* Fish */
/*--------------------------------------------------------------------------------*/

{GAI_NOTEBOOK, "Fish"},

{GAI_FRAME, "Fish"},
     {GAI_SPINBUTTON, "Fish 1:", &fish_settings.fish1, &fish_settings.fish1, &l_0_100_1},
     {GAI_SPINBUTTON, "Fish 2:", &fish_settings.fish2, &fish_settings.fish2, &l_0_100_1},
     {GAI_SPINBUTTON, "Fish 3:", &fish_settings.fish3, &fish_settings.fish3, &l_0_100_1},
     {GAI_SPINBUTTON, "Fish 4:", &fish_settings.fish4, &fish_settings.fish4, &l_0_100_1},
     {GAI_SPINBUTTON, "Fish 5:", &fish_settings.fish5, &fish_settings.fish5, &l_0_100_1},
     {GAI_SPINBUTTON, "Fish 6:", &fish_settings.fish6, &fish_settings.fish6, &l_0_100_1},
     {GAI_SPINBUTTON, "Swordfish:", &fish_settings.swordfish, &fish_settings.swordfish, &l_0_100_1},
     {GAI_SPINBUTTON, "Blowfish:", &fish_settings.blowfish, &fish_settings.blowfish, &l_0_100_1},
     {GAI_SPINBUTTON, "Bottomdweller:", &fish_settings.bdweller, &fish_settings.bdweller, &l_0_100_1},
{GAI_FRAME_E},
{GAI_FRAME_R, "More fish"},
     {GAI_SPINBUTTON, "Fillmore:", &fish_settings.fillmore, &fish_settings.fillmore, &l_0_100_1},
     {GAI_SPINBUTTON, "Sherman:", &fish_settings.sherman, &fish_settings.sherman, &l_0_100_1},
     {GAI_SPINBUTTON, "Megan:", &fish_settings.megan, &fish_settings.megan, &l_0_100_1},
     {GAI_SPINBUTTON, "Prey:", &fish_settings.prey, &fish_settings.prey, &l_0_100_1},
     {GAI_SPINBUTTON, "Hunter:", &fish_settings.hunter, &fish_settings.hunter, &l_0_100_1},
     {GAI_SPINBUTTON, "Lori:", &fish_settings.lori, &fish_settings.lori, &l_0_100_1},
     {GAI_SPINBUTTON, "Ernest:", &fish_settings.ernest, &fish_settings.ernest, &l_0_100_1},
     {GAI_SPINBUTTON, "Squid:", &fish_settings.squid, &fish_settings.squid, &l_0_100_1},
     {GAI_SPINBUTTON, "Hawthorne:", &fish_settings.hawthorne, &fish_settings.hawthorne, &l_0_100_1},
{GAI_FRAME_E},

{GAI_FRAME, "Fish population"},
     {GAI_RADIOBUTTON, fish_radio, &fish_settings.type, &fish_settings.type},
     {GAI_SPINBUTTON, "Population size:",  &fish_settings.num_fish,  &fish_settings.num_fish, &l_0_200_1},
{GAI_FRAME_E},

{GAI_FRAME_R, "Fish behaviour"},
     {GAI_CHECKBUTTON, "Eating prey fish is ok", &fish_settings.eat,  &fish_settings.eat},
     {GAI_CHECKBUTTON, "Make blowfish explode is ok", &fish_settings.explode,  &fish_settings.explode},
     {GAI_CHECKBUTTON, "Dead fish might be reborn", &fish_settings.rebirth, &fish_settings.rebirth},
     {GAI_CHECKBUTTON,  "Fish from the same kind may differ in size.", &fish_settings.scale_diff, &fish_settings.scale_diff},
     {GAI_SPINBUTTON, "Scale (in %):",  &fish_settings.scale,  &fish_settings.scale, &l_10_1000_5},
     {GAI_SPINBUTTON, "Speed (in %):", &fish_settings.speed, &fish_settings.speed, &l_10_1000_5},
     {GAI_SPINBUTTON, "Hunter agressiveness (in %):", &fish_settings.hunter_agr,  &fish_settings.hunter_agr, &l_0_100_5},
     {GAI_SPINBUTTON, "Swordfish agressiveness (in %):", &fish_settings.swordfish_agr,  &fish_settings.swordfish_agr, &l_0_100_5},
{GAI_FRAME_E},
{GAI_NOTEBOOK_E},

/*--------------------------------------------------------------------------------*/
/* Thermometer */
/*--------------------------------------------------------------------------------*/

{GAI_NOTEBOOK, "Thermometer"},
{GAI_FRAME,"First Thermometer"},
     {GAI_OPTIONMENU, "Draw on: ", &thermometer_settings.draw1, &thermometer_settings.draw1, draw_list}, 
     {GAI_OPTIONMENU, "Vertical placement: ", &thermometer_settings.vert1, &thermometer_settings.vert1, vert_list}, 
     {GAI_OPTIONMENU, "Horizontal placement: ", &thermometer_settings.horz1, &thermometer_settings.horz1, horz_list}, 
     {GAI_CHECKBUTTON, "Split", &thermometer_settings.split1,&thermometer_settings.split1},
     {GAI_COLORSELECTOR, "Thermometer colour: ", &thermometer_settings.c1, &thermometer_settings.c1},
     {GAI_OPTIONMENU, "Messures: ", &thermometer_settings.messure1, &thermometer_settings.messure1, messure_list},
     {GAI_SPINBUTTON, "Roof level: ", &thermometer_settings.roof1, &thermometer_settings.roof1, &l_0_1000000_1},
     {GAI_TEXT, "Fans, thermometer and network/modem\nneed a roof level limit set."},
     {GAI_TEXTENTRY, "Mount point: ", &thermometer_settings.mount_point1, &thermometer_settings.mount_point1},
     {GAI_TEXT, "A mount point is needed for disc usage."},
     {GAI_FRAME, "Split"},
          {GAI_COLORSELECTOR, "Thermometer colour: ", &thermometer_settings.c1_s, &thermometer_settings.c1_s},
          {GAI_OPTIONMENU, "Messures: ", &thermometer_settings.messure1_s, &thermometer_settings.messure1_s, messure_list},
          {GAI_SPINBUTTON, "Roof level: ", &thermometer_settings.roof1_s, &thermometer_settings.roof1_s, &l_0_1000000_1},
          {GAI_TEXT, "Fans, thermometer and network/modem\nneed a roof level limit set."},
          {GAI_TEXTENTRY, "Mount point: ", &thermometer_settings.mount_point1_s, &thermometer_settings.mount_point1_s},
          {GAI_TEXT, "A mount point is needed for disc usage."},
     {GAI_FRAME_E},
{GAI_FRAME_E},

{GAI_FRAME_R,"Second Thermometer"},
     {GAI_OPTIONMENU, "Draw on: ", &thermometer_settings.draw2, &thermometer_settings.draw2, draw_list}, 
     {GAI_OPTIONMENU, "Vertical placement: ", &thermometer_settings.vert2, &thermometer_settings.vert2, vert_list}, 
     {GAI_OPTIONMENU, "Horizontal placement: ", &thermometer_settings.horz2, &thermometer_settings.horz2, horz_list}, 
     {GAI_CHECKBUTTON, "Split", &thermometer_settings.split2,&thermometer_settings.split2},
     {GAI_COLORSELECTOR, "Thermometer colour: ", &thermometer_settings.c2, &thermometer_settings.c2},
     {GAI_OPTIONMENU, "Messures: ", &thermometer_settings.messure2, &thermometer_settings.messure2, messure_list},
     {GAI_SPINBUTTON, "Roof level: ", &thermometer_settings.roof2, &thermometer_settings.roof2, &l_0_1000000_1},
     {GAI_TEXT, "Fans, thermometer and network/modem\nneed a roof level limit set."},
     {GAI_TEXTENTRY, "Mount point: ", &thermometer_settings.mount_point2, &thermometer_settings.mount_point2},
     {GAI_TEXT, "A mount point is needed for disc usage."},
     {GAI_FRAME, "Split"},
          {GAI_COLORSELECTOR, "Thermometer colour: ", &thermometer_settings.c2_s, &thermometer_settings.c2_s},
          {GAI_OPTIONMENU, "Messures: ", &thermometer_settings.messure2_s, &thermometer_settings.messure2_s, messure_list},
          {GAI_SPINBUTTON, "Roof level: ", &thermometer_settings.roof2_s, &thermometer_settings.roof2_s, &l_0_1000000_1},
          {GAI_TEXT, "Fans, thermometer and network/modem\nneed a roof level limit set."},
          {GAI_TEXTENTRY, "Mount point: ", &thermometer_settings.mount_point2_s, &thermometer_settings.mount_point2_s},
          {GAI_TEXT, "A mount point is needed for disc usage."},
     {GAI_FRAME_E},
{GAI_FRAME_E},
{GAI_NOTEBOOK_E},
/*--------------------------------------------------------------------------------*/
/* Leds */
/*--------------------------------------------------------------------------------*/
{GAI_NOTEBOOK, "Leds"},
{GAI_OPTIONMENU, "Draw on: ", &leds_settings.draw,  &leds_settings.draw, draw_list},
{GAI_OPTIONMENU, "Vertical placement: ", &leds_settings.vert, &leds_settings.vert, vert_list},
{GAI_OPTIONMENU, "Horizontal placement: ", &leds_settings.horz, &leds_settings.horz, horz_list},
{GAI_OPTIONMENU, "Layout: ", &leds_settings.vert_horz, &leds_settings.vert_horz, vert_horz_list},
{GAI_SPINBUTTON, "Alpha: ", &leds_settings.alpha, &leds_settings.alpha, &l_0_255_1},

{GAI_FRAME, "Led one"},
     {GAI_OPTIONMENU, "Is: ", &leds_settings.leds_func[0], &leds_settings.leds_func[0], leds_func_list},
     {GAI_OPTIONMENU, "Colour: ", &leds_settings.leds_colour[0],&leds_settings.leds_colour[0], leds_colour_list},
     {GAI_CHECKBUTTON, "Show shadow when off", &leds_settings.leds_show_off[0], &leds_settings.leds_show_off[0]},
{GAI_FRAME_E},

{GAI_FRAME_R, "Led two"},
     {GAI_OPTIONMENU, "Is: ", &leds_settings.leds_func[1], &leds_settings.leds_func[1], leds_func_list},
     {GAI_OPTIONMENU, "Colour: ", &leds_settings.leds_colour[1],&leds_settings.leds_colour[1], leds_colour_list},
     {GAI_CHECKBUTTON, "Show shadow when off", &leds_settings.leds_show_off[1], &leds_settings.leds_show_off[1]},
{GAI_FRAME_E},

{GAI_FRAME, "Led three"},
     {GAI_OPTIONMENU, "Is: ", &leds_settings.leds_func[2], &leds_settings.leds_func[2], leds_func_list},
     {GAI_OPTIONMENU, "Colour: ", &leds_settings.leds_colour[2],&leds_settings.leds_colour[2], leds_colour_list},
     {GAI_CHECKBUTTON, "Show shadow when off", &leds_settings.leds_show_off[2], &leds_settings.leds_show_off[2]},
{GAI_FRAME_E},

{GAI_FRAME_R, "Led four"},
     {GAI_OPTIONMENU, "Is: ", &leds_settings.leds_func[3], &leds_settings.leds_func[3], leds_func_list},
     {GAI_OPTIONMENU, "Colour: ", &leds_settings.leds_colour[3],&leds_settings.leds_colour[3], leds_colour_list},
     {GAI_CHECKBUTTON, "Show shadow when off", &leds_settings.leds_show_off[3], &leds_settings.leds_show_off[3]},
{GAI_FRAME_E},

{GAI_NOTEBOOK_E},


/*--------------------------------------------------------------------------------*/
/* Extra */
/*--------------------------------------------------------------------------------*/

{GAI_NOTEBOOK, "Extra"},
{GAI_CHECKBUTTON, "Hide mouse cursor when over applet", &over_settings.cursor_off, &over_settings.cursor_off},
{GAI_CHECKBUTTON, "Fade when mouse cursor is over", &over_settings.fade, &over_settings.fade},

{GAI_FRAME, "Mouse pointer over activates"},
     {GAI_RADIOBUTTON, over_radio, &over_settings.type,&over_settings.type},
{GAI_FRAME_E},

#ifdef XMMS
{GAI_FRAME, "Xmms song name displayer"},
     {GAI_CHECKBUTTON, "On", &xmms_settings.on,&xmms_settings.on},
     {GAI_OPTIONMENU, "Draw on:", &xmms_settings.draw,  &xmms_settings.draw, draw_list},
     {GAI_OPTIONMENU, "Vertical placement:", &xmms_settings.vert, &xmms_settings.vert, vert_list},
     {GAI_OPTIONMENU, "Horizontal placement:", &xmms_settings.horz, &xmms_settings.horz, horz_list},
     {GAI_OPTIONMENU, "Text scroll direction:", &xmms_settings.direction, &xmms_settings.direction, xmms_scroll_list},
     {GAI_COLORSELECTOR, "Font colour", &xmms_settings.c, &xmms_settings.c},
{GAI_FRAME_E},
#endif
{GAI_FRAME, "Tetris"},
    {GAI_CHECKBUTTON, "Use maximum size", &tetris_settings.size_limit, &tetris_settings.size_limit},
    {GAI_CHECKBUTTON, "Show next piece", &tetris_settings.show_next, &tetris_settings.show_next},
    {GAI_SPINBUTTON, "Width: ", &tetris_settings.width, &tetris_settings.width, &l_4_50_1},
    {GAI_SPINBUTTON, "Height: ", &tetris_settings.height, &tetris_settings.height, &l_4_50_1},
{GAI_FRAME_E},

{GAI_NOTEBOOK_E},

{GAI_END}};


void pref_restart(gpointer d)
{
    Background_settings *bas;
    Bottom_settings *bos;
    Bubble_settings *bus;
    Clock_settings *cs;
    Date_settings *ds;
    Fish_settings *fs;
    General_settings *gs;
    Leds_settings *ls;
    Over_settings *os;
    Sound_settings *ss;
    Tetris_settings *tt;
    Thermometer_settings *ts;
#ifdef XMMS
    Xmms_sn_settings *xss;
#endif
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();
    ad->dont_update = 1;
    
    bas = background_get_settings_ptr();
    bos = bottom_get_settings_ptr();
    bus = bubble_get_settings_ptr();
    cs = clock_get_settings_ptr();
    ds = date_get_settings_ptr();
    fs = fish_get_settings_ptr();
    gs = general_get_settings_ptr();

    
    ls = leds_get_settings_ptr();
    os = over_get_settings_ptr();
    ss = sound_get_settings_ptr();
    ts = thermometer_get_settings_ptr();
    tt = tetris_get_settings_ptr();
#ifdef XMMS
    xss = xmms_sn_get_settings_ptr();
#endif

    memcpy(bas, &background_settings, sizeof(Background_settings));
    memcpy(bos, &bottom_settings, sizeof(Bottom_settings));
    memcpy(bus, &bubble_settings, sizeof(Bubble_settings));
    memcpy(cs, &clock_settings, sizeof(Clock_settings));
    memcpy(ds, &date_settings, sizeof(Date_settings));
    memcpy(fs, &fish_settings, sizeof(Fish_settings));
    memcpy(gs, &general_settings, sizeof(General_settings));
    memcpy(ls, &leds_settings, sizeof(Leds_settings));
    memcpy(os, &over_settings, sizeof(Over_settings));
    memcpy(ss, &sound_settings, sizeof(Sound_settings));    
    memcpy(tt, &tetris_settings, sizeof(Tetris_settings));
    memcpy(ts, &thermometer_settings, sizeof(Thermometer_settings)); 
#ifdef XMMS
    memcpy(xss, &xmms_settings, sizeof(Xmms_sn_settings));
#endif

    clock_save_settings();
    date_save_settings();
    bubble_save_settings();
    background_save_settings();
    fish_save_settings();
    general_save_settings();
    bottom_save_settings();

    over_save_settings();
    leds_save_settings();
    tetris_save_settings();
    thermometer_save_settings();
#ifdef XMMS
    xmms_sn_save_settings();
#endif
    sound_save_settings();

    aquarium_free_all();

    /* Load the fishes early, since aquarium change will need them */
    fish_init();

    if(gai_applet_mode() == GAI_DOCKAPP){
	if(gs->ratio_width > gs->ratio_height)
	    aquarium_change(GAI_HORIZONTAL, gai_get_size(), gai_get_size(), NULL);
	else
	    aquarium_change(GAI_VERTICAL, gai_get_size(), gai_get_size(), NULL);
    }
    else
	aquarium_change(gai_get_orient(), gai_get_size(), gai_get_size(), NULL);

    aquarium_reload_all();
    ad->dont_update = 0;
}

void pref_init(void)
{
    Background_settings *bas;
    Bottom_settings *bos;
    Bubble_settings *bus;
    Clock_settings *cs;
    Date_settings *ds;
    Fish_settings *fs;
    Leds_settings *ls;
    Over_settings *os;
    Sound_settings *ss;
    Tetris_settings *tt;
    Thermometer_settings *ts;
#ifdef XMMS
    Xmms_sn_settings *xss;
#endif
    General_settings *gs;

    AquariumData *ad;

    
    ad = aquarium_get_settings_ptr();
    
    bas = background_get_settings_ptr();
    bos = bottom_get_settings_ptr();
    bus = bubble_get_settings_ptr();
    cs = clock_get_settings_ptr();
    ds = date_get_settings_ptr();
    fs = fish_get_settings_ptr();
    gs = general_get_settings_ptr();

    ls = leds_get_settings_ptr();
    os = over_get_settings_ptr();
    ss = sound_get_settings_ptr();
    tt = tetris_get_settings_ptr();
    ts = thermometer_get_settings_ptr();
#ifdef XMMS
    xss = xmms_sn_get_settings_ptr();
#endif

    memcpy(&background_settings, bas, sizeof(Background_settings));
    memcpy(&bottom_settings, bos, sizeof(Bottom_settings));
    memcpy(&bubble_settings, bus, sizeof(Bubble_settings));
    memcpy(&clock_settings, cs, sizeof(Clock_settings));
    memcpy(&date_settings, ds, sizeof(Date_settings));
    memcpy(&fish_settings, fs, sizeof(Fish_settings));
    memcpy(&general_settings, gs, sizeof(General_settings));
    memcpy(&leds_settings, ls, sizeof(Leds_settings));
    memcpy(&over_settings, os, sizeof(Over_settings));
    memcpy(&sound_settings, ss, sizeof(Sound_settings));
    memcpy(&tetris_settings, tt, sizeof(Tetris_settings));
    memcpy(&thermometer_settings, ts, sizeof(Thermometer_settings));
#ifdef XMMS
    memcpy(&xmms_settings, xss, sizeof(Xmms_sn_settings));
#endif

}
