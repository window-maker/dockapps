

#include <X11/XKBlib.h>
#include <stdio.h>
#include "leds.h"
#include "aquarium.h"
#include "draw.h"

static SA_Image *led_images = NULL;
static int num_led_images = 0;
static Leds_settings leds_settings;
static AquariumData *ad;

Leds_settings *leds_get_settings_ptr(void)
{
    return &leds_settings;
}

/*
  Returns the turned on leds:
  Bit 0 is Capslock
  Bit 1 is Numlock
  Bit 2 is Scrollock
*/

/*
  This code is based upon some lines(actually two lines :-) 
  in E-Leds by Mathias Meisfjordskar<mathiasm@ifi.uio.no>
  Released under GPL.
*/

static int check_kleds(void)
{
    unsigned int states;

    if (XkbGetIndicatorState(GDK_DISPLAY(), XkbUseCoreKbd, &states) != Success) {
	perror("Error while reading Indicator status\n");
	return -1;
    }
    return (states & 0x7);
}

static int get_capslock(void)
{
    if((check_kleds() & 1) == 1) return 1;
    else return 0;
}

static int get_numlock(void)
{
    if((check_kleds() & 2) == 2) return 1;
    else return 0;
}

static int get_scrollock(void)
{
    if((check_kleds() & 4) == 4) return 1;
    else return 0;
}


static void do_led(int func, int colour, int show_off, int x, int y, int alpha)
{
    int is_on;
    
    switch(func){
    case LEDS_OFF:
	return;
    case LEDS_CAPSLOCK:
	is_on = get_capslock();
	break;
    case LEDS_NUMLOCK:
	is_on = get_numlock();
	break;
    case LEDS_SCROLLOCK:
	is_on = get_scrollock();
	break;
    default:
	is_on=0;
	break;
    }
    
    
    if((!is_on && show_off) || is_on)
	draw_pic_alpha(led_images[colour].image,
		       led_images[colour].width,
		       led_images[colour].height,
		       x, y, !is_on, alpha);

}

void leds_exit(void)
{
    int i;

    if(led_images == NULL) 
	return;

    for(i = 0; i < num_led_images; i++){
	if(led_images[i].image != NULL)
	    g_object_unref(led_images[i].pixbuf);
    }

    g_free(led_images);
    led_images = NULL;
    num_led_images = 0;

}

void leds_init(void)
{
    int i;
    char *led_files[] = {
    "leds/blue.png",
    "leds/bluegreen.png",
    "leds/orange.png",
    "leds/red.png",
    "leds/violet.png",
    "leds/yellow.png",
    "leds/pink.png",
    "leds/green.png",
    "leds/darkblue.png",
    "leds/lightblue.png",
    "leds/yellowgreen.png",
    NULL};

    ad = aquarium_get_settings_ptr();

    if(led_images != NULL)
	leds_exit();

    for(num_led_images=0; led_files[num_led_images] != NULL; num_led_images++);

    led_images = g_malloc0(sizeof(SA_Image) * num_led_images);

    for(i = 0; i < num_led_images; i++)
	load_image(led_files[i], &led_images[i], 2);
}





void leds_update(int beforeorafter)
{
    int numleds=0;
    int x = 2, y = 2, i;

    if(beforeorafter != leds_settings.draw)
	return;

    for(i = 0; i < 3 ; i++)
	if(leds_settings.leds_func[i] != LEDS_OFF)
	    numleds++;

    switch(leds_settings.horz){
    case LEFT:
	x = 2;
	break;
    case RIGHT:
	x = ad->xmax-numleds * led_images[0].width - 2;
	break;
    case CENTER:
	x = (ad->xmax-numleds * led_images[0].width) / 2;
	break;
    default:
	break;
    }

    switch(leds_settings.vert){
    case TOP:
	y = 2;
	break;
    case BOTTOM:
	y = ad->ymax-numleds * led_images[0].height - 2;
	break;
    case CENTER:
	y = (ad->ymax-numleds * led_images[0].height) / 2;
	break;
    default:
	break;
    }

    for(i = 0; i < NUMLEDS; i++)
	do_led(leds_settings.leds_func[i], 
	       leds_settings.leds_colour[i],
	       leds_settings.leds_show_off[i], 
	       x + !leds_settings.vert_horz * led_images[0].width * i, 
	       y + leds_settings.vert_horz * led_images[0].height * i, leds_settings.alpha);

}
