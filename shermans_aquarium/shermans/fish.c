

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aquarium.h"
#include "draw.h"
#include "fish.h"
#include "bottom.h"
#include "soundeffects.h"



/* Data for fishes */

static float fish1_speed[4] = { 1.0, 1.0, 1.0, 1.0 };
static float fish2_speed[4] = { 0.6, 0.6, 0.6, 0.6 };
static float fish3_speed[4] = { 1.1, 1.1, 1.1, 1.1 };
static float fish4_speed[4] = { 1.0, 1.0, 1.0, 1.0 };
static float fish5_speed[7] = { 0.8, 0.8, 0.7, 0.5, 0.7, 0.8, 0.8 };
static float fish6_speed[4] = { 1.2, 1.2, 1.2, 1.2 };

static float squid_speed[7] = { 0.1, 3.0, 3.0, 3.0, 3.0, 1.5, 1.0 };
static float swordfish_speed[4] = { 1.4, 1.4, 1.4, 1.4 };
static float blowfish_speed[4] = { 0.6, 0.6, 0.6, 0.6 };
static float ernest_speed[4] = { 0.8, 0.8, 0.8, 0.8 };
static float hunter_speed[4] = { 1.1, 1.1, 1.1, 1.1 };
static float lori_speed[4] = { 0.8, 0.8, 0.8, 0.8 };
static float prey_speed[4] = { 1.3, 1.3, 1.3, 1.3 };
static float sherman_speed[14] =
    { 1.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 1.75, 2.25, 2.5, 2.5, 2.5 };
static float fillmore_speed[15] =
    { 0.7, 0.7, 0.7, 0.7, 0.8, 0.9, 1.0, 1.0, 1.0, 0.7, 0.5, 0.5, 0.5, 0.6, 0.7 };

static float prey_hunter_speed[7] = {1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5};
static float blowup_speed[2] = {0.1, 0.1};

static float bdweller_speed[4] = {0.01, 0.01, 0.01, 0.01};


static int normal_animation[4] =    { 0, 1, 2, 1 };
static int sherman_animation[14] =  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 0 };
static int fillmore_animation[15] = { 0, 1, 2, 3, 4, 5, 6, 6, 6, 7, 8, 9, 10, 11,  0 };

static int squid_animation[7]={ 0, 1, 1, 1, 1, 1, 2};
static int fish5_animation[7]={ 0, 0, 1, 2, 1, 0, 0};
static int prey_hunter_animation[7]={0, 1, 2, 3, 4, 5, 6};


static Fish_animation fish_animation[NUMOFFISHTYPES] = {
    {"sherman/fish1.png",     3, 4, normal_animation,   fish1_speed     },
    {"sherman/fish2.png",     3, 4, normal_animation,   fish2_speed     },
    {"sherman/fish3.png",     3, 4, normal_animation,   fish3_speed     },
    {"sherman/fish4.png",     3, 4, normal_animation,   fish4_speed     },
    {"sherman/fish5.png",     3, 7, fish5_animation,    fish5_speed     },
    {"sherman/fish6.png",     3, 4, normal_animation,   fish6_speed     },
    {"sherman/squid.png",     3, 7, squid_animation,    squid_speed     },
    {"sherman/swordfish.png", 3, 4, normal_animation,   swordfish_speed },
    {"sherman/blowfish.png",  3, 4, normal_animation,   blowfish_speed  },
    {"sherman/ernest.png",    3, 4, normal_animation,   ernest_speed    },
    {"sherman/hunter.png",    3, 4, normal_animation,   hunter_speed    },
    {"sherman/lori.png",      3, 4, normal_animation,   lori_speed      },
    {"sherman/prey.png",      3, 4, normal_animation,   prey_speed      },
    {"sherman/sherman.png",  11,14, sherman_animation,  sherman_speed   },
    {"sherman/fillmore.png", 12,15, fillmore_animation, fillmore_speed  },
    {"sherman/bdweller.png",  3, 4, normal_animation,   bdweller_speed  },
    {"sherman/hawthorne.png", 3, 4, normal_animation,   bdweller_speed  },
    {"sherman/megan.png",    11,14, sherman_animation,  sherman_speed   },

};


static Fish *fishes;
static SA_Image blowup_data;
static SA_Image prey_hunter_data;

static SA_Image *fish_buffer;
static Fish_settings fish_settings;

static AquariumData *ad;
static int scale_diff, num_fish, hunter_is, blowfish_is;

Fish_settings *fish_get_settings_ptr(void)
{
    return &fish_settings;
}

Fish *fish_get_fishes_ptr(void)
{
    return fishes;
}

void fish_enter(void)
{
    int i;

    for (i = 0; i < fish_settings.num_fish; i++){

    /* completely off the screen, don't bother showing their escape */
	if ( ((int)fishes[i].tx > (ad->xmax)) || 
	     ((int)fishes[i].tx < -(fishes[i].width)))
	    fishes[i].speed_mul = 0.0;
	else	
	    fishes[i].speed_mul = 2.5;
    }

}

void fish_leave(void)
{
    int i;

    for (i = 0; i < fish_settings.num_fish; i++)
	fishes[i].speed_mul = 1.0;

}


static int sel_fish(int j, int reset)
{
    static int local_fish[NUMOFFISHTYPES];

    if(reset){
	memset(&local_fish, 0, sizeof(int)*NUMOFFISHTYPES);
	return 0;
    }


    /* The Bottom fish first */
    if(fish_settings.bdweller > local_fish[BDWELLER]){
	local_fish[BDWELLER]++;
	return BDWELLER;
    }

    if(fish_settings.hawthorne > local_fish[HAWTHORNE]){
	local_fish[HAWTHORNE]++;
	return HAWTHORNE;
    }


    /* FIXME: Should randomize the order */

    if(fish_settings.fish1 > local_fish[FISH1]){
	local_fish[FISH1]++;
	return FISH1;
    }

    if(fish_settings.fish2 > local_fish[FISH2]){
	local_fish[FISH2]++;
	return FISH2;
    }

    if(fish_settings.fish3 > local_fish[FISH3]){
	local_fish[FISH3]++;
	return FISH3;
    }

    if(fish_settings.fish4 > local_fish[FISH4]){
	local_fish[FISH4]++;
	return FISH4;
    }

    if(fish_settings.fish5 > local_fish[FISH5]){
	local_fish[FISH5]++;
	return FISH5;
    }

    if(fish_settings.fish6 > local_fish[FISH6]){
	local_fish[FISH6]++;
	return FISH6;
    }


    if(fish_settings.squid > local_fish[SQUID]){
	local_fish[SQUID]++;
	return SQUID;
    }

    if(fish_settings.swordfish > local_fish[SWORDFISH]){
	local_fish[SWORDFISH]++;
	return SWORDFISH;
    }

    if(fish_settings.blowfish > local_fish[BLOWFISH]){
	local_fish[BLOWFISH]++;
	return BLOWFISH;
    }

    if(fish_settings.ernest > local_fish[ERNEST]){
	local_fish[ERNEST]++;
	return ERNEST;
    }

    if(fish_settings.hunter > local_fish[HUNTER]){
	local_fish[HUNTER]++;
	return HUNTER;
    }

    if(fish_settings.lori > local_fish[LORI]){
	local_fish[LORI]++;
	return LORI;
    }

    if(fish_settings.prey > local_fish[PREY]){
	local_fish[PREY]++;
	return PREY;
    }

    if(fish_settings.sherman > local_fish[SHERMAN]){
	local_fish[SHERMAN]++;
	return SHERMAN;
    }

    if(fish_settings.fillmore > local_fish[FILLMORE]){
	local_fish[FILLMORE]++;
	return FILLMORE;
    }

    if(fish_settings.megan > local_fish[MEGAN]){
	local_fish[MEGAN]++;
	return MEGAN;
    }
    return 0;
}


static int get_fish(int start, int wantedfish)
{
    int i;
    AquariumData *ad;
    ad = aquarium_get_settings_ptr();

    for(i = start + 1; i < fish_settings.num_fish; i++)
	if(fishes[i].type == wantedfish) return i;
  
    return -1;
}

/* This rouitine handles both the eating fish happening and
   the swordish hits a blowfish happening. */

/* 
   This rountine is the most messy you've seen. Sorry! 
   But it works ok. :-)
*/

static void prey_hunter_hit()
{
    int i, j;
    int victim_val=-1, doer_val=-1;

    ad = aquarium_get_settings_ptr();


    /* FIX ME: Scaling of hunter that eats prey */
    //    if(fish_settings.scale_diff) return;

    for(i = 0; i < fish_settings.num_fish; i++){
	doer_val = get_fish(doer_val, HUNTER);
	if(doer_val == -1) 
	    break;

	/* Check if this one is eating right now. */
	if(fishes[doer_val].num_animation == 7) 
	    continue;

	victim_val = -1;

	for(j = 0; j < fish_settings.num_fish; j++){
	    victim_val = get_fish(victim_val, PREY);
	    if(victim_val ==- 1) 
		break;
	    if(fishes[victim_val].rev == fishes[doer_val].rev) 
		continue;
	    

	    /* Don't eat already dead fish */
	    if(fishes[victim_val].is_dead) 
		continue;

	    /* If parts of the prey is beyond the right edge, let it live */
	    if((fishes[victim_val].tx + fishes[victim_val].width) > (ad->xmax + ad->viewpoint_start_x)) 
		continue;
	    /* If the fish is below the edge, let it live */
	    if((fishes[victim_val].y) > (ad->ymax + ad->viewpoint_start_y)) 
		continue;

	    if(fishes[victim_val].tx < (ad->xmin - fishes[victim_val].width)) 
		continue;

	    /* If the prey is too high, let it live */
	    if((fishes[victim_val].y) < ad->viewpoint_start_y) 
		continue;



      
	    if(abs((fishes[victim_val].y + fishes[victim_val].height) -
		   (fishes[doer_val].y + fishes[doer_val].height)) > (fishes[doer_val].height / 2)) 
		continue;


      
	    /* rev=1 prey moves to towards right.*/
	    if(fishes[victim_val].rev){

		if((fishes[doer_val].tx - fishes[victim_val].tx - fishes[victim_val].width)
		   > (10 + fishes[victim_val].width) &&
		   (fishes[doer_val].tx - fishes[victim_val].tx - fishes[victim_val].width)
		   <(30 + fishes[victim_val].width)){
		    if((fishes[victim_val].y + fishes[victim_val].height)
		       >(fishes[doer_val].y + fishes[doer_val].height)) 
			fishes[doer_val].y += 2;
		    if((fishes[victim_val].y + fishes[victim_val].height)
		       <(fishes[doer_val].y + fishes[doer_val].height)) 
			fishes[doer_val].y -= 2;
		    continue;
		}

		if((fishes[doer_val].tx - fishes[victim_val].tx - fishes[victim_val].width)
		   > (10 + fishes[victim_val].width))
		    continue;
		if((fishes[doer_val].tx - fishes[victim_val].tx - fishes[victim_val].width)
		   < fishes[victim_val].width) 
		    continue;

	    }
	    else{
		if((fishes[victim_val].tx - fishes[doer_val].width - fishes[doer_val].tx)
		   > (10 + fishes[victim_val].width) &&
		   (fishes[victim_val].tx - fishes[doer_val].width - fishes[doer_val].tx)
		   < (30 + fishes[victim_val].width)){

		    if((fishes[victim_val].y + fishes[victim_val].height)
		       > (fishes[doer_val].y + fishes[doer_val].height)) 
			fishes[doer_val].y += 2;
		    if((fishes[victim_val].y + fishes[victim_val].height)
		       < (fishes[doer_val].y + fishes[doer_val].height)) 
			fishes[doer_val].y -= 2;
		    continue;
		}


		if((fishes[victim_val].tx - fishes[doer_val].width - fishes[doer_val].tx)
		   > (10 + fishes[victim_val].width)) 
		    continue;
		if((fishes[victim_val].tx - fishes[doer_val].width - fishes[doer_val].tx)
		   < fishes[victim_val].width) 
		    continue;
	    }
	   	    

	    /* Sometimes the hunter decides not to eat the prey for various reasons. */
	    if(g_rand_int_range(ad->rnd, 0 , 100) > fish_settings.hunter_agr) 
		continue;

	    //printf("hunter %d eats prey %d!\n",doer_val, victim_val);

	


	    /* Play scream */
	    sound_eatscream();

	    /* Removing eaten prey */
	    aquarium_clean_image((int)fishes[victim_val].tx, fishes[victim_val].y - ad->viewpoint_start_y,
				 fishes[victim_val].width, fishes[victim_val].height);


	    /*	    if(doer_val>victim_val) doer_val--;

	    fish_settings.num_fish--;
      
	    for(k=victim_val;k<fish_settings.num_fish;k++){
		fishes[k]=fishes[k+1];
	    }
	    */

	    fishes[victim_val].is_dead = TRUE;

	    /* Towards left */
      
	    if(!fishes[doer_val].rev)
		fishes[doer_val].tx -= (prey_hunter_data.width - fishes[doer_val].width);

	    fishes[doer_val].image = &prey_hunter_data;
	    fishes[doer_val].frame = 0;
	    fishes[doer_val].width = prey_hunter_data.width;
	    fishes[doer_val].height =prey_hunter_data.height;
	    fishes[doer_val].animation = prey_hunter_animation;
	    fishes[doer_val].num_animation = 7;
	    fishes[doer_val].speed = prey_hunter_speed;
	    fishes[doer_val].fast_frame_change = 5.0;
	    break;
      
	}

    }
}

/* Not quite ok yet...*/
static void blowup_hit()
{

    int i,j;
    int victim_val = -1, doer_val = -1;
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();


    for(i = 0;i < fish_settings.num_fish; i++){
	doer_val = get_fish(doer_val, SWORDFISH);
	if(doer_val == -1) 
	    break;

	victim_val = -1;

	for(j = 0;j < fish_settings.num_fish; j++){
	    victim_val = get_fish(victim_val, BLOWFISH);
	    if(victim_val == -1) 
		break;

	    /* Is this blowfish blowing up? - Then take next.*/
	    if(fishes[victim_val].num_animation == 2) 
		continue;

	    /* Don't bother zombie blowfish */
	    if(fishes[victim_val].is_dead) 
		continue;
       
	    if(fishes[victim_val].rev == fishes[doer_val].rev) 
		continue;


	    /* If parts of the prey is beyond the right edge, let it live */
	    if((fishes[victim_val].tx + fishes[victim_val].width) 
	       > (ad->xmax + ad->viewpoint_start_x)) 
		continue;
	    /* If the fish is below the edge, let it live */
	    if((fishes[victim_val].y) > (ad->ymax + ad->viewpoint_start_y)) 
		continue;

	    if(fishes[victim_val].tx < (ad->xmin-fishes[victim_val].width)) 
		continue;

	    /* If the prey is too high, let it live */
	    if((fishes[victim_val].y) < ad->viewpoint_start_y)
		continue;

      
	    if(abs(fishes[victim_val].y - fishes[doer_val].y)
	       > (fishes[victim_val].height / 2)) 
		continue;


      
	    /* rev=1 prey moves to towards right.*/
	    if(fishes[victim_val].rev){

		if((fishes[doer_val].tx - fishes[victim_val].tx - fishes[victim_val].width)
		   > 0 &&
		   (fishes[doer_val].tx - fishes[victim_val].tx - fishes[victim_val].width)
		   < 20){
		    if((fishes[victim_val].y + fishes[victim_val].height / 2)
		       > (fishes[doer_val].y + fishes[doer_val].height / 2)) 
			fishes[doer_val].y += 2;
		    if((fishes[victim_val].y + fishes[victim_val].height / 2)
		       <(fishes[doer_val].y + fishes[doer_val].height / 2))
			fishes[doer_val].y -= 2;
		    continue;
		}

		if((fishes[doer_val].tx - fishes[victim_val].tx - fishes[victim_val].width)
		   > 0) continue;
		if((fishes[doer_val].tx - fishes[victim_val].tx - fishes[victim_val].width)
		   < -10) continue;
		/*
		  printf("killing from right!\n vx:%d - vy:%d - dx:%d - dy:%d", 
		  (int)fishes[victim_val].tx, fishes[victim_val].y,
		  (int)fishes[doer_val].tx, fishes[doer_val].y);
		*/
	    }
	    else{
		if((fishes[victim_val].tx - fishes[doer_val].width - fishes[doer_val].tx)
		   > 0 &&
		   (fishes[victim_val].tx - fishes[doer_val].width - fishes[doer_val].tx)
		   < 20){

		    if((fishes[victim_val].y + fishes[victim_val].height / 2) 
		       >(fishes[doer_val].y + fishes[doer_val].height / 2)) 
			fishes[doer_val].y += 2;

		    if((fishes[victim_val].y + fishes[victim_val].height / 2)
		       <(fishes[doer_val].y + fishes[doer_val].height / 2)) 
			fishes[doer_val].y -= 2;

		    continue;
		}


		if((fishes[victim_val].tx - fishes[doer_val].width - fishes[doer_val].tx)
		   > 0) continue;
		if((fishes[victim_val].tx - fishes[doer_val].width - fishes[doer_val].tx)
		   < -10) continue;
		/*
		  printf("killing from left!\n vx:%d - vy:%d - dx:%d - dy:%d", 
		  (int)fishes[victim_val].tx, fishes[victim_val].y,
		  (int)fishes[doer_val].tx, fishes[doer_val].y);

		*/
	    }
	    /*
	      printf("swordfish hit %d blowfish %d!\n",doer_val, victim_val);
	    */

	    /* Sometimes the blowfish owns the swordfish money, and that can save its life ;-) */
	    if(g_rand_int_range(ad->rnd, 0, 100) > fish_settings.swordfish_agr) 
		continue;

	    /* Removing the remains of the poor blowfish */
	    aquarium_clean_image((int)fishes[victim_val].tx, fishes[victim_val].y - ad->viewpoint_start_y,
				 fishes[victim_val].width, fishes[victim_val].height);



	    /* Exploding blowfish */
	    sound_explode();

	    fishes[victim_val].image = &blowup_data;
	    fishes[victim_val].frame = 0;
	    fishes[victim_val].width = blowup_data.width;
	    fishes[victim_val].height = blowup_data.height;
	    fishes[victim_val].animation = normal_animation;
	    fishes[victim_val].num_animation = 2;
	    fishes[victim_val].speed = blowup_speed;
	    fishes[victim_val].fast_frame_change = 1.0;
	    break;
      
	}

    }

}



void fish_update(void)
{
    int i,j;
    AquariumData *ad;
    Bottom_settings *bottom;

    ad = aquarium_get_settings_ptr();
    bottom = bottom_get_settings_ptr();

    if((ad->special_action & 1) && fish_settings.eat)
	prey_hunter_hit();

    if((ad->special_action & 2) == 2 && fish_settings.explode)
	blowup_hit();

#ifdef DEBUG
    printf("\t\t - Done special hits.\n");
#endif


    for(i = 0;i < fish_settings.num_fish; i++){
	aquarium_clean_image((int) fishes[i].tx,
			     fishes[i].y - ad->viewpoint_start_y,
			     fishes[i].width, fishes[i].height);
    }


    for(i = 0; i < fish_settings.num_fish; i++){
	if (fishes[i].frame >= (fishes[i].num_animation - 1)){
	    if(fish_settings.eat){
		if(fishes[i].image == &prey_hunter_data){
		    
		    //printf("Done eating.Continue.\n");
		    
	    
		    /* Left movement is ok. */
		    /*		  else fishes[i].tx-=fishes[i].width-fish_buffer[HUNTER].width;*/
	    
		    if(!fish_settings.scale_diff){
			if(fishes[i].rev) 
			    fishes[i].tx += fishes[i].width - fish_buffer[HUNTER].width;
			fishes[i].width = fish_buffer[HUNTER].width;
			fishes[i].height = fish_buffer[HUNTER].height;
			fishes[i].image = &fish_buffer[HUNTER];
		    } else {
			if(fishes[i].rev) 
			    fishes[i].tx += fishes[i].width - fish_buffer[hunter_is].width;
			fishes[i].width = fish_buffer[hunter_is].width;
			fishes[i].height = fish_buffer[hunter_is].height;
			fishes[i].image = &fish_buffer[hunter_is];
		    }

		    fishes[i].animation = normal_animation;
		    fishes[i].num_animation = 4;
		    fishes[i].speed = hunter_speed;
		    fishes[i].fast_frame_change = 1.0;
		    fishes[i].frame = 0;
	    
	    
		}
	    }
	
	    if(fish_settings.explode) {
	  
		if(fishes[i].image == &blowup_data){
		    //printf("Done blowing up.\n");
		    fishes[i].is_dead = TRUE;

		    if(!fish_settings.scale_diff){
			fishes[i].width = fish_buffer[BLOWFISH].width;
			fishes[i].height = fish_buffer[BLOWFISH].height;
			fishes[i].image = &fish_buffer[BLOWFISH];
		    } else {
			fishes[i].width = fish_buffer[blowfish_is].width;
			fishes[i].height = fish_buffer[blowfish_is].height;
			fishes[i].image = &fish_buffer[blowfish_is];
		    }

		    fishes[i].animation = normal_animation;
		    fishes[i].num_animation = 4;
		    fishes[i].speed = prey_speed;
		    fishes[i].fast_frame_change = 1.0;
		    fishes[i].frame = 0;


		    /*
		    fish_settings.num_fish--;
		    for(k=i;k<fish_settings.num_fish;k++){
			fishes[k]=fishes[k+1];
			}*/

	    
		}
	    }
	}
    }
    
    sel_fish(0, TRUE);

    for (i = 0; i < fish_settings.num_fish; i++) {

	if(fishes[i].is_dead == TRUE){

	    if(fish_settings.rebirth){
		if(g_rand_int_range(ad->rnd, 0, 100) < 20){
		    //		    printf("Rebirth and salvation..\n");
		    /* Salvation and rebirth...*/
		    fishes[i].is_dead = FALSE;
		    fishes[i].frame = g_rand_int_range(ad->rnd, 0, fishes[i].num_animation);


		    if(g_rand_boolean(ad->rnd)){
			fishes[i].rev = 1;
			fishes[i].travel =
			    g_rand_int_range(ad->rnd, 0, (ad->virtual_aquarium_x - ad->xmax) / 2) +
			    fishes[i].width;
			fishes[i].tx = -fishes[i].width - fishes[i].travel;
		    } else {
			fishes[i].rev = 0;
			fishes[i].travel = 
			    g_rand_int_range(ad->rnd, 0, (ad->virtual_aquarium_x - ad->xmax) / 2) +
			    fishes[i].width;
			fishes[i].tx = ad->xmax + fishes[i].travel;
		    }
		    fishes[i].y = g_rand_int_range(ad->rnd, 0, (ad->virtual_aquarium_y - fishes[i].height));

		    fishes[i].speed_mul = 1.0 + g_rand_double_range(ad->rnd, -15.0 / 100.0, 15.0 / 100.0);
		}

	    } else {
		/* Don't draw zombie fish */
		continue;
	    }
	}
	

	/* frozen fish doesn't need to be handled, or drawn */
	if (fishes[i].speed_mul == 0)
	    continue;

	if((fishes[i].type == BDWELLER || fishes[i].type == HAWTHORNE) && !bottom->have_sea_floor)
	    continue;

	/* move fish in horizontal direction, left or right */
	/* Large aquarium, the fishes are shown more seldom */

	if (!fishes[i].rev) {

	    fishes[i].tx -=
		(fishes[i].speed[fishes[i].frame]) * (fishes[i].speed_mul * fish_settings.speed / 100);

	    if (fishes[i].tx < (-fishes[i].width - fishes[i].travel)) {
		/* we moved out of bounds. change direction,
		 * position, speed. */

		fishes[i].travel = g_rand_int_range(ad->rnd, 0, (ad->virtual_aquarium_x - ad->xmax) / 2) +
		    fishes[i].width;
		fishes[i].tx = -fishes[i].width - fishes[i].travel;
		fishes[i].rev = 1;

		/* If bottom creature, keep the the Y level */
		if(fishes[i].type != BDWELLER && fishes[i].type != HAWTHORNE)
		    fishes[i].y = g_rand_int_range(ad->rnd, 0, ad->virtual_aquarium_y - fishes[i].height);
		

		if (ad->proximity)
		    fishes[i].speed_mul = 0;
		else
		    fishes[i].speed_mul = 1;

	    }
	} else {


	    fishes[i].tx +=
		(fishes[i].speed[fishes[i].frame]) * (fishes[i].speed_mul * fish_settings.speed / 100);


	    if (fishes[i].tx > ad->xmax + fishes[i].travel) {
		/* we moved out of bounds. change direction,
		 * position, speed. */

		fishes[i].travel = g_rand_int_range(ad->rnd, 0, (ad->virtual_aquarium_x - ad->xmax) / 2) +
		    fishes[i].width;
		fishes[i].tx = ad->xmax + fishes[i].travel;
		fishes[i].rev = 0;

		/* If bottom creature, keep the the Y level */
		if(fishes[i].type != BDWELLER && fishes[i].type != HAWTHORNE)
		    fishes[i].y = g_rand_int_range(ad->rnd, 0, ad->virtual_aquarium_y - fishes[i].height);

		if (ad->proximity)
		    fishes[i].speed_mul = 0;
		else
		    fishes[i].speed_mul = 1;

	    }
	}

	/* move fish in vertical position randomly by one pixel up or down */
	/* If the fish last time moved up or down, larger chance that it does that again. */

	j = g_rand_int_range(ad->rnd, 0, 16);

	/* The bottondweller & hawthorne shall be still ;-) */
	if(fishes[i].type == BDWELLER || fishes[i].type == HAWTHORNE) 
	    j = 0;

	if (((fishes[i].updown == 1) && (j == 6 || j == 7 || j == 8))
	    || j == 8) {
	    fishes[i].y++;
	    fishes[i].updown = 1;
	} else if (((fishes[i].updown == -1)
		    && (j == 12 || j == 13 || j == 14)) || j == 12) {
	    fishes[i].y--;
	    fishes[i].updown = -1;
	} else
	    fishes[i].updown = 0;
#ifdef DEBUG
	/*	printf("\t\t - Calling draw_fish()\n");*/
#endif
	    /* animate fishes using fish_animation array */
	    aquarium_draw_image((int) fishes[i].tx,
				fishes[i].y - ad->viewpoint_start_y,
				fishes[i].animation[fishes[i].frame],
				fishes[i].rev, fishes[i].image);

	/* switch to next swimming frame */


	fishes[i].delay += fishes[i].speed[fishes[i].frame] * fishes[i].fast_frame_change;
#ifdef DEGUG
	printf("delay:%f speed:%f frame:%d num_ani:%d anipic:%d\n",
	       fishes[i].delay, fishes[i].speed[fishes[i].frame],fishes[i].frame, fishes[i].num_animation, fishes[i].animation[fishes[i].frame]);
#endif
	if (fishes[i].delay >= (7 * fishes[i].speed[fishes[i].frame])) {
	    if (fishes[i].frame >= (fishes[i].num_animation - 1)){
		fishes[i].frame = 0;
	    }	  
	    else fishes[i].frame ++;
	    fishes[i].delay = 0;
	}

    }
}

static void fish_flip(SA_Image *this_fish)
{
    int j, k, ypos;

    if(this_fish->image != NULL){

      this_fish->rev = g_malloc0(this_fish->width * 
				 this_fish->full_height * 4 );
    
      for(j = 0;j < (this_fish->full_height); j++){
	ypos = j * this_fish->width * 4;
	for(k = 0; k < this_fish->width; k++){
	  this_fish->rev[ypos + (this_fish->width - k - 1) * 4]     = this_fish->image[ypos + k * 4];
	  this_fish->rev[ypos + (this_fish->width - k - 1) * 4 + 1] = this_fish->image[ypos + k * 4 + 1];
	  this_fish->rev[ypos + (this_fish->width - k - 1) * 4 + 2] = this_fish->image[ypos + k * 4 + 2];
	  this_fish->rev[ypos + (this_fish->width - k - 1) * 4 + 3] = this_fish->image[ypos + k * 4 + 3];
	}
      }
    }
}


void fish_turn(void)
{
  int i, m;


  if(fish_settings.scale_diff)
      m = fish_settings.num_fish;
  else 
      m = NUMOFFISHTYPES;

  for(i = 0; i < m; i++){
      fish_flip(&fish_buffer[i]);
  }

  fish_flip(&blowup_data);
  fish_flip(&prey_hunter_data);

}


void fish_init(void)
{
    int i, num_bottom_fish;
    int type = 0, special_fish = 0;

    Bottom_settings *bottom_settings;
    
    bottom_settings = bottom_get_settings_ptr();

    /* Set up this for the rest of the life */
    ad = aquarium_get_settings_ptr();

    ad->special_action = 0;


    /* Save settings for fish_exit() */
    scale_diff = fish_settings.scale_diff;

    /* 0 - 30 fishes */
    if(fish_settings.type == RANDOM_POP_FISH)
	fish_settings.num_fish = g_rand_int_range(ad->rnd, 1, 30);

    num_fish = fish_settings.num_fish;

    if(fish_settings.scale_diff)
	fish_buffer = g_malloc0(sizeof(SA_Image) * fish_settings.num_fish);
    else
	fish_buffer = g_malloc0(sizeof(SA_Image) * NUMOFFISHTYPES);


    fishes = g_malloc0(sizeof(Fish) * fish_settings.num_fish);

    num_bottom_fish = bottom_settings->num_bottom_animals;

    if(num_bottom_fish > num_fish)
	num_bottom_fish = num_fish;

    for (i = 0; i < fish_settings.num_fish; i++) {

	if(fish_settings.type == RANDOM_FISH || fish_settings.type == RANDOM_POP_FISH) {

	    /* FIX ME! Bottom dweller and hawthorne chance !*/
	    if((i<num_bottom_fish) && (bottom_settings->have_sea_floor)) {

		if(g_rand_boolean(ad->rnd)) 
		    type = BDWELLER;
		else type = HAWTHORNE;
	    }
	    else
		do{
		    type = g_rand_int_range(ad->rnd, 0, NUMOFFISHTYPES); 
		} while(type == BDWELLER || type == HAWTHORNE);
	}
	else{
	    type = sel_fish(i, FALSE);
	}


	if(type == PREY) special_fish |= 1;
	if(type == HUNTER) special_fish |= 2;
	if(type == SWORDFISH) special_fish |= 4;
	if(type == BLOWFISH) special_fish |= 8;


	if(!fish_settings.scale_diff){

	    if (fish_buffer[type].image == NULL) {
		load_image_n_scale(fish_animation[type].file,
				   &fish_buffer[type],
				   fish_animation[type].pics,
				   fish_settings.scale);

	    }

	    fishes[i].width = fish_buffer[type].width;
	    fishes[i].height = fish_buffer[type].height;

	    fishes[i].image = &fish_buffer[type];

	} else {

	    if(type != HUNTER && type != BLOWFISH){
		load_image_n_scale(fish_animation[type].file, &fish_buffer[i],
				   fish_animation[type].pics,
				   abs(fish_settings.scale + (g_rand_int_range(ad->rnd, -15, 15)) * fish_settings.scale / 100) );
	    } else {
		if(type == HUNTER) hunter_is = i;
		if(type == BLOWFISH) blowfish_is = i;

		load_image_n_scale(fish_animation[type].file, &fish_buffer[i],
				   fish_animation[type].pics,
				   fish_settings.scale);
	    }

	    fishes[i].width = fish_buffer[i].width;
	    fishes[i].height = fish_buffer[i].height;

	    fishes[i].image = &fish_buffer[i];

	}


	fishes[i].num_animation = fish_animation[type].frames;
	fishes[i].animation = fish_animation[type].animation;

	fishes[i].speed = fish_animation[type].speed;


	fishes[i].type = type;
	fishes[i].frame = g_rand_int_range(ad->rnd, 0, fishes[i].num_animation);

	fishes[i].rev = g_rand_boolean(ad->rnd);
	fishes[i].tx = g_rand_int_range(ad->rnd, 0, ad->virtual_aquarium_x - fishes[i].width);

	fishes[i].speed_mul =  1.0 + g_rand_double_range(ad->rnd, -15.0 / 100.0, 15.0 / 100.0);


	fishes[i].fast_frame_change = 1.0;

	fishes[i].is_dead = FALSE;

	fishes[i].updown = 0;
	fishes[i].travel = g_rand_int_range(ad->rnd, 0, (ad->virtual_aquarium_x - ad->xmax) / 2) + fishes[i].width;
	fishes[i].y = g_rand_int_range(ad->rnd, 0, ad->virtual_aquarium_y - fishes[i].height);

    }

    
    if((special_fish & 3) == 3 && fish_settings.eat){
	load_image_n_scale("sherman/eating.png", 
			   &prey_hunter_data,
			   7, fish_settings.scale);

	ad->special_action |= 1;
    } else
	prey_hunter_data.pixbuf = NULL;

    if((special_fish & 12) == 12 && fish_settings.explode){
	load_image_n_scale("sherman/blowup.png", 
			   &blowup_data,
			   7, fish_settings.scale);

	ad->special_action|=2;
    }
    else
    	blowup_data.pixbuf = NULL;

}


void fish_exit(void)
{
    int i,j;

    if(scale_diff)
	j = num_fish;
    else
	j = NUMOFFISHTYPES;

    for(i = 0; i < j; i++){
	if(fish_buffer[i].pixbuf != NULL){
	    g_object_unref(fish_buffer[i].pixbuf);

	    if(fish_buffer[i].rev !=NULL)
		g_free(fish_buffer[i].rev);
	}
    }

    
    if(blowup_data.pixbuf != NULL){
	g_object_unref(blowup_data.pixbuf);

	if(blowup_data.rev != NULL)
	    g_free(blowup_data.rev);

    }

    if(prey_hunter_data.pixbuf != NULL){
	g_object_unref(prey_hunter_data.pixbuf);

	if(prey_hunter_data.rev != NULL)
	    g_free(prey_hunter_data.rev);
    }

    g_free(fish_buffer);
    g_free(fishes);
}


