
#include <stdlib.h>
#include <string.h>


#include "aquarium.h"
#include "bubble.h"
#include "draw.h"
#include "soundeffects.h"

/* current bubble count */
static int nr_bubbles;

static Bubble *bubbles = NULL;
static SA_Image bubbles_image_data;


static int bubble_state_change;

static Bubble_settings bubble_settings;

Bubble_settings *bubble_get_settings_ptr(void)
{
    return &bubble_settings;
}

void bubble_exit(void)
{
    if(bubbles_image_data.pixbuf != NULL)
	g_object_unref(bubbles_image_data.pixbuf);
    memset(&bubbles_image_data, 0, sizeof(SA_Image));
}

void bubble_init(void)
{
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();

    nr_bubbles = 0;
    bubble_state_change = ad->ymax / (BUBBLES_FRAMES - 1);


    if(bubbles!=NULL)
	g_free(bubbles);

    bubbles = g_malloc0(sizeof(Bubble) * bubble_settings.max_bubbles);

    /* Load bubbles */

    load_image("bubbles.png", &bubbles_image_data, BUBBLES_FRAMES);

}

void bubble_update(void)
{
    int i, x, y;
    int seq;
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();


    for(i = 0; i < nr_bubbles; i++){
	aquarium_clean_image(bubbles[i].x, bubbles[i].y,
			     bubbles_image_data.width, 
			     bubbles_image_data.height);
    }


    /* make a new bubble, if needed */
    if(((nr_bubbles < bubble_settings.max_bubbles)) && (g_rand_int_range(ad->rnd, 0, 100) <= 32)) {
	bubbles[nr_bubbles].x = ad->viewpoint_start_x + (g_rand_int_range(ad->rnd, 0, ad->xmax));
	bubbles[nr_bubbles].y = ad->viewpoint_start_y + ad->ymax;

	bubbles[nr_bubbles].speed = g_rand_double_range(ad->rnd, 1.0/13.0, 12.0/13.0);

	nr_bubbles++;
    }



    /* Update and draw the bubbles */
    for (i = 0; i < nr_bubbles; i++) {

	/* Move the bubble vertically */
	bubbles[i].y -= bubbles[i].speed;

	/* Did we lose it? */
	if (bubbles[i].y < ad->viewpoint_start_y) {
	    /* Yes; nuke it */
	    bubbles[i].x = bubbles[nr_bubbles - 1].x;
	    bubbles[i].y = bubbles[nr_bubbles - 1].y;
	    bubbles[i].speed = bubbles[nr_bubbles - 1].speed;
	    nr_bubbles--;

	    /* We must check the previously last bubble, which is
	     * now the current bubble, also. */
	    i--;
	    continue;
	}

	/* Draw the bubble */
	x = bubbles[i].x - ad->viewpoint_start_x;
	y = ((int) bubbles[i].y) - ad->viewpoint_start_y;

	/* calculate bubble sequence - 0 to 4 (determine bubble sprite idx) */
	seq = y / bubble_state_change;

	/* draw the bubble, using offset-to-center calculated from current
	 * sequence, and make the bubble bigger as we go up. 120 - alpha */

	if(seq < BUBBLES_FRAMES){
	    aquarium_draw_pic_alpha(&bubbles_image_data, bubbles_image_data.width, 
			   bubbles_image_data.height, x, y,
			   seq, 120);
	}

    }

    /* Sometimes play bubble sound */
    sound_bubbles();
}
