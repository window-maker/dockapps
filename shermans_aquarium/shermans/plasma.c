

/* Plasma "plugin" for Sherman's aquarium by Jonas Aaberg <cja@gmx.net> 2002 */

/*

  This plasma is based upon Jeremy Longley's JCL plasma for MS-DOS
  that was written probably around 1995.

*/

#include "aquarium.h"
#include "over.h"
#include "plasma.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_FRAMES 1000
#define GEN_PALETTE 11
#define GEN_MOVEMENT 10
#define START_UP_PLASMA 13

static unsigned char *surface = NULL, *movement = NULL, *palette = NULL, *pscreen = NULL;

static int frame;

static int p1, p2, p3, p4, p5, p6;
static int m1, m2, m3, m4;
static int c1, c2, c3, c11, c21, c31;

static int surface_x = 512;
static int surface_y = 300;



void plasma_exit(void)
{

}

void plasma_init(void)
{
    surface = movement = palette = pscreen = NULL;
}

void plasma_end(void)
{ 
    if(surface != NULL)
	g_free(surface);
    if(movement != NULL)
	g_free(movement);
    if(palette != NULL)
	g_free(palette);
    if(pscreen != NULL)
	g_free(pscreen);
    surface = movement = palette = pscreen = NULL;

}


void plasma_gen_surface(int start_num)
{
    int ypos;
    unsigned char value;
    float x, y;

    for(y = start_num * (surface_y / 10); y < ((start_num + 1) * surface_y / 10); y++){
	ypos = (int) y * surface_x;
	for(x = 0; x < surface_x; x++){
	    value = 64 + (unsigned char)(10 * (sin(x / p1) + cos(y / p2) +
			 cos(x / p3) + sin(y / p4) +
			 sin((x + y) / p5) +
			 cos(hypot(256 - x, 150 - y) / p6)));
	    surface[ypos+(int)x] = value;
	}
    }

}



void plasma_start(void)
{
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();

    frame=0;

    /* Surface randomization */
    p1 = g_rand_int_range(ad->rnd, 1, 101);
    p2 = p1 + g_rand_int_range(ad->rnd, -25, 25);
    p3 = g_rand_int_range(ad->rnd, 1, 101);
    p4 = p3 + g_rand_int_range(ad->rnd, -25, 25);
    p5 = g_rand_int_range(ad->rnd, 1, 101);
    p6 = p5 + g_rand_int_range(ad->rnd, -25, 25);

    /* Movement randomization */
    m1 = g_rand_int_range(ad->rnd, 1, 101);
    m2 = g_rand_int_range(ad->rnd, 1, 101);
    m3 = g_rand_int_range(ad->rnd, 1, 101);
    m4 = g_rand_int_range(ad->rnd, 1, 101);

    /* Colour randomization */

    c1 = g_rand_int_range(ad->rnd, 1, 101);
    c11 = g_rand_int_range(ad->rnd, 1, 101);
    c2 = g_rand_int_range(ad->rnd, 1, 101);
    c21 = g_rand_int_range(ad->rnd, 1, 101);
    c3 = g_rand_int_range(ad->rnd, 1, 101);
    c31= g_rand_int_range(ad->rnd, 1, 101);


    if(surface!=NULL)
	plasma_end();

    ad = aquarium_get_settings_ptr();

    /* Allocating memory */

    surface_x = ((int)(ad->xmax / 512.0 + 1)) * 512;
    surface_y = 2 * ad->ymax;

    surface = g_malloc0(surface_x * surface_y);
    movement = g_malloc0(4 * MAX_FRAMES);
    palette = g_malloc0(3 * 2 * MAX_FRAMES);


    pscreen = g_malloc0(ad->xmax*ad->ymax * 4);

}

void plasma_start_2(int start_num)
{
    float count;
    int lead, offset;

    if(start_num < GEN_MOVEMENT)
	plasma_gen_surface(start_num);
    if(start_num == GEN_MOVEMENT){

	for(count = 0; count < MAX_FRAMES; count++){
	    lead = (int)((surface_y / 3 - 4) + (surface_y / 3 - 10) * cos(count / m1)
			 + surface_x * (int)((surface_y / 6 - 2) + (surface_y / 6 - 3) * sin(count / m2)));
	    offset = (int)((surface_y / 3 - 4) + (surface_y / 3 - 8) * sin(count / m3)
			  + surface_x * (int)((surface_y / 6 - 2) + (surface_y / 6 - 3) * cos(count / m4))
			 - lead);
	    movement[(int)count * 4 + 0] = (unsigned char)((lead & 0xff00) >> 8);
	    movement[(int)count * 4 + 1] = (unsigned char)(lead & 0xff);
	    movement[(int)count * 4 + 2] = (unsigned char)((offset & 0xff00) >> 8);
	    movement[(int)count * 4 + 3] = (unsigned char)(offset & 0xff);
	}
    }

    if(start_num == GEN_PALETTE){
	for(count = 0; count < (((MAX_FRAMES + 768) / 256 + 1) * 256); count++){
	    palette[(int)count * 3 + 0] = (unsigned char)(sin(count / c11) * sin(count / c1) * 126 + 126);
	    palette[(int)count * 3 + 1] = (unsigned char)(sin(count / c21) * sin(count / c2) * 126 + 126);
	    palette[(int)count * 3 + 2] = (unsigned char)(sin(count / c31) * sin(count / c3) * 126 + 126);
	}
    }
    
}


void plasma_update(void)
{
    unsigned int lead, offset;
    unsigned char t;
    unsigned int sum;
    int p, ypos;
    int surface_ptr = 0;
    AquariumData *ad;

    int x, y;

    if(frame == MAX_FRAMES){
	plasma_start();
    }

    frame++;
	
    if(frame < START_UP_PLASMA){
	plasma_start_2(frame - 1);
	return;
    }

    t = movement[frame * 4 + 1];
    lead = (unsigned int)t;
    t = movement[frame * 4 + 0];
    lead += ((unsigned int)t) << 8;

    t = movement[frame * 4 + 3];
    offset = (unsigned int)t;
    t = movement[frame * 4 + 2];
    offset += ((unsigned int)t) << 8;

    sum = (offset + lead) & 0xffff;

    ad = aquarium_get_settings_ptr();

    for(y = 0; y < ad->ymax; y++){
	ypos = y * ad->xmax * 4;
	for(x = 0; x < ad->xmax; x++){
	    t = (unsigned char)((surface[lead + surface_ptr + x] + surface[sum + surface_ptr + x]) & 0xff);
	    p = 3 * (int)t;
	    pscreen[ypos + x * 4 + 0] = palette[3 * frame + p + 0];
	    pscreen[ypos + x * 4 + 1] = palette[3 * frame + p + 1];
	    pscreen[ypos + x * 4 + 2] = palette[3 * frame + p + 2];
	    //	    pscreen[ypos + x * 4 + 3] = 'E';
	}
	surface_ptr += surface_x;
    }

    over_draw(0, 0, 0, ad->xmax, ad->ymax,pscreen);
    

}
