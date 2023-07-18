
/* This is a layer between main and different functions that can be called
   when the mouse pointer is over */


#include <stdio.h>
#include "aquarium.h"
#include "over.h"
#include "draw.h"
#include "matrix.h"
#include "plasma.h"
#include "tetris.h"

static int fade, old_over = -1;
static unsigned char *screen;
static Over_settings over_settings;
static AquariumData *ad;


Over_settings *over_get_settings_ptr(void)
{
    return &over_settings;
}

void over_keypress(int key)
{
    if(over_settings.type == OVER_TETRIS)
	tetris_keypress(key);

}
void over_joystick(GaiFlagsJoystick jflags)
{
    if(over_settings.type == OVER_TETRIS)
	tetris_joystick(jflags);
}

void over_init(void)
{
    GaiFlagsType gf;

    screen=NULL;
    fade=0;

    ad = aquarium_get_settings_ptr();

    
    if(over_settings.cursor_off)
	gf = GAI_FLAGS_MOUSE_PTR_HIDE;
    else
	gf = GAI_FLAGS_MOUSE_PTR_SHOW;

    gf |= GAI_FLAGS_ALLOW_ROTATE;

    gai_flags_set(gf);

    if(over_settings.type == OVER_OFF)
	return;

    old_over = over_settings.type;

    if(over_settings.type == OVER_MATRIX)
	matrix_init();
    if(over_settings.type == OVER_PLASMA)
	plasma_init();
    if(over_settings.type == OVER_TETRIS)
	tetris_init();
}

void over_start(void)
{
    if(screen != NULL){
	g_free(screen);
	screen = NULL;
    }

    if(over_settings.type == OVER_OFF)
	return;

    screen = g_malloc0((ad->xmax+2)*4*(ad->ymax+2));

    if(over_settings.type == OVER_MATRIX)
	matrix_start();
    if(over_settings.type == OVER_PLASMA)
	plasma_start();
    if(over_settings.type == OVER_TETRIS)
	tetris_start();
}

void over_end(void)
{
    if(screen != NULL)
	g_free(screen);

    screen = NULL;

    if(over_settings.type == OVER_OFF)
	return;

    if(over_settings.type == OVER_MATRIX)
	matrix_end();
    if(over_settings.type == OVER_PLASMA)
	plasma_end();
    if(over_settings.type == OVER_TETRIS)
	tetris_end();
}

void over_exit(void)
{


    if(screen!=NULL)
	g_free(screen);

    screen=NULL;

    if(old_over == OVER_MATRIX)
	matrix_exit();
    if(old_over == OVER_PLASMA)
	plasma_exit();
    if(old_over == OVER_TETRIS)
	tetris_exit();
}



int over_update(int mouse_over)
{
   
    int full_over=0;
  


    if(over_settings.type == OVER_OFF)
	return 0;

    /* All is faded up and mouse is over, return */

    if(fade==0 && !mouse_over) return 0;

    if(mouse_over){

	if(fade==0) 
	    over_start();

	if(fade<255) 
	    fade+=5;

	if(!over_settings.fade) 
	    fade=255;
    }
    else{
	if(fade>0)
	    fade-=5;

	if(!over_settings.fade) 
	    fade=0;

	if(fade==0){ 
	    over_end();
	    return 0;
	}

    }

    if(fade==0) full_over = 1;

    if(over_settings.type == OVER_MATRIX)
	matrix_update();
    if(over_settings.type == OVER_PLASMA)
	plasma_update();
    if(over_settings.type == OVER_TETRIS)
	tetris_update();

    draw_pic_alpha(screen, ad->xmax, ad->ymax, 0, 0, 0, fade);

    return full_over;
}



/* This one is very alike to draw_fish - Only small differences.*/

void over_draw(int x, int y, int idx, int width, int height, unsigned char *pic)
{
    /* bounding box of the clipped sprite */
    int dw, di, dh, ds;

    int w, h, pos, fidx, ypos, pic_pos;

    /* completely off the screen, don't bother drawing */
    if ((y < (-height)) || (y > (ad->ymax)) || (x > (ad->xmax)) || (x < -(width)))
	return;


    /* do clipping for top side */
    ds = 0;
    if (y < 0)
	ds = -(y);

    /* do clipping for bottom side */
    dh = height;
    if ((y + height) > ad->ymax)
	dh = ad->ymax - y;

    /* do clipping for right side */
    dw = width;
    if ((x + width) > ad->xmax)
	dw = width - ((x + width) - ad->xmax);

    /* do clipping for left side */
    di = 0;
    if (x < 0)
	di = -(x);

    fidx = width * height * 4 * idx;

    for (h = ds; h < dh; h++) {
	/* offset to beginning of current row */
	ypos = (h + y) * ad->xmax * 4;
	for (w = di; w < dw; w++) {
	    pic_pos = h * width * 4 + w * 4 + fidx;
	    pos = ypos + w * 4 + x * 4;
	    if (pic[pic_pos + 3] != 0) {
		screen[pos] = pic[pic_pos];
		screen[pos + 1] = pic[pic_pos + 1];
		screen[pos + 2] = pic[pic_pos + 2];
		screen[pos + 3] = 1;
	    }
	}

    }

    
}
