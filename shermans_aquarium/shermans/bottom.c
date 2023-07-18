
#include <stdlib.h>
#include "aquarium.h"
#include "draw.h"
#include "bottom.h"
#include "fish.h"

static Bottom_settings bottom_settings;
static AquariumData *ad;



Bottom_settings *bottom_get_settings_ptr(void)
{
    return &bottom_settings;
}

void bottom_init(void)
{
    SA_Image sea_floor;
    SA_Image bottom_stuff;

    Bottom bottom_items[NUMOFBOTTOMITEMS] = {
    {"bottom/plant1.png",        0},
    {"bottom/plant2.png",        0},
    {"bottom/plant3.png",        0},
    {"bottom/plant4.png",        0},
    {"bottom/plant5.png",        0},
    {"bottom/plant6.png",        0},
    {"bottom/plant7.png",        0},
    {"bottom/plant9.png", 	 0},
    {"bottom/plant10.png",       0},
    {"bottom/smallstone1.png",   0},
    {"bottom/smallstone3.png",   0},
    {"bottom/stone1.png",        0},
    {"bottom/stone2.png",        0},
    {"bottom/stone3.png",        0},
    {"bottom/weirdplant.png",    0},
    {"bottom/weirdplant2.png",   0},
    {"bottom/octo1.png",         0},
    {"bottom/bigplant.png",	 0}
    };

    char *bottom_image_files[] = {
    "bottom/bottom1.png",
    "bottom/bottom2.png",
    "bottom/bottom3.png",
    NULL
    };


    int start_sea_floor_x, start_sea_floor_y;

    int curr_sea_floor_height;
    int i,j,k,l,jj, step_max, current_loc,x=0;
    int old_height=0,old_x=0, old_current_loc=0, bx[10], bw[10], nfa;
    Fish *fishes;
    Fish_settings *fish_settings;

    ad = aquarium_get_settings_ptr();

    fishes = fish_get_fishes_ptr();

    fish_settings = fish_get_settings_ptr();


    if(!bottom_settings.have_sea_floor) 
	return;

    i = g_rand_int_range(ad->rnd, 0, NUMBOTTOMIMAGES);

    load_image_n_scale(bottom_image_files[i], &sea_floor, 1, bottom_settings.scale);

    start_sea_floor_x = g_rand_int_range(ad->rnd, 0, sea_floor.width);

    curr_sea_floor_height = sea_floor.height - g_rand_int_range(ad->rnd, 0, sea_floor.height*4/5);

    if(curr_sea_floor_height == 0)
	curr_sea_floor_height = 1;

    
    start_sea_floor_y = ad->ymax - curr_sea_floor_height;


    for(i = (start_sea_floor_x - sea_floor.width); i < ad->xmax; i += sea_floor.width)
	draw_image_bg(i, start_sea_floor_y, 0, 0, &sea_floor);
    

  
    /* Number of plants and stones default max 15 */

    if(bottom_settings.random_plants)
	j = g_rand_int_range(ad->rnd, 1, bottom_settings.max_plants+1);
    else
	j = bottom_settings.max_plants;
  
    step_max = curr_sea_floor_height / j;

    /* The smallest stone/plant is 32 high */
    current_loc = ad->ymax - curr_sea_floor_height + 10;
    //printf("Curr loc: %d %d %d\n", current_loc, ad->ymax, curr_sea_floor_height);

    /* See if we have a bottom dweller and or a hacktorn*/
    nfa=0;
    
    for(i = 0;i < fish_settings->num_fish; i++){
	if(fishes[i].type == BDWELLER || fishes[i].type == HAWTHORNE){

	    for(jj = 0; jj < 10; jj++){
		if(ad->xmax-fishes[i].width == 0)		   
		    x = 0;
		else
		    x = g_rand_int_range(ad->rnd, 0, ad->xmax - fishes[i].width);

		for(l = 0; l < nfa; l++)
		    if(x > bx[l] && x < (bx[l]+bw[l])) 
			break;
		if(l == nfa) break;
	    }  
	    /* Do just 10 athempts to place a bottom fish, if not sucessfull,
	       then try next fish */
	    if(jj == 10){
		/* Make sure the fish is outside the picture. */
		fishes[i].tx = 2 * ad->xmax;
		fishes[i].y = 0;
		continue;
	    }     
 
	    bx[nfa] = x;
	    bw[nfa] = fishes[i].width;
	    fishes[i].tx = bx[nfa];
	    /* (int)(20*(float)scale/100)*/
	    fishes[i].y = current_loc + curr_sea_floor_height / 3 + g_rand_int_range(ad->rnd, 0, curr_sea_floor_height) + ad->viewpoint_start_y - 30;
	    if(fishes[i].type == BDWELLER)
		fishes[i].fast_frame_change = 0.02;
	    else
		fishes[i].fast_frame_change = 0.07;

	    fishes[i].speed_mul = 0.0001;
	    nfa++;
	}
    }
      

    /* Freeing sea floor. Not needed any longer */
    g_object_unref(sea_floor.pixbuf);

      
    for(i = 0; i < j; i++){
	k=g_rand_int_range(ad->rnd, 0, NUMOFBOTTOMITEMS);


	load_image_n_scale(bottom_items[k].image, &bottom_stuff,
			   1, (bottom_settings.scale / 2) + (g_rand_int_range(ad->rnd, 0, bottom_settings.scale / 2) * bottom_settings.scale / 100));
    
	/* from -15 to ad.xmax+15 */
	jj = 0;
	if(nfa != 0){
	    for(jj = 0; jj < 10; jj++){
		x = g_rand_int_range(ad->rnd, 0, ad->xmax + 30) - 15;
		for(l = 0;l < nfa; l++)
		    if(x > (bx[l] - bottom_stuff.width) && x < (bx[l] + bw[l])) break;
		if(l == nfa) break;
	    }
	    
	}
	else
	    x = g_rand_int_range(ad->rnd, 0, ad->xmax + 30) - 15;

	/* If we fail to put plants too many times.. */
	if(jj == 10) continue;

	/* Are they too close? */
	if(abs(x - old_x) < bottom_stuff.width)
	    if((old_current_loc + old_height) > (bottom_stuff.height + current_loc)){
		current_loc = old_current_loc + old_height - bottom_stuff.height + 5;
	    }
	
    
	draw_image_bg(x, current_loc-bottom_stuff.height + 4,
		      0, g_rand_int_range(ad->rnd, 0, 2),
		      &bottom_stuff);
    
	old_current_loc = current_loc;
	current_loc += g_rand_int_range(ad->rnd, 0, step_max);

	old_height = bottom_stuff.height;
	old_x = x;
    
	g_object_unref(bottom_stuff.pixbuf);

	bottom_stuff.image = NULL;

    }

}

void bottom_exit(void)
{

}
