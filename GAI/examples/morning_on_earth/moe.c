
/* 

   Morning on earth v0.2 

   - An example how to use GAI by Jonas Aaberg <cja@gmx.net>
   
   (Morning on earth is a song by the power metal band "Pain of Salvation")

   Requires GAI version 0.5.3 or later

   Released under GNU GPL.
   
*/
#include "config.h"
#include <gai/gai.h>

/* Data about the image. It includes 18 images of the earth, each is 50x47 large */

#define MOE_HEIGHT 47
#define MOE_WIDTH 50
#define MOE_FRAMES 18

static int frame_counter = 0;
static GdkPixbuf *moe;




/* This linked structure describes the preference window */
static char *button1_prg, *button2_prg;

/* This line creates a Text entry field that looks like this:
   Mouse button 1 starts: [              ]
   with the default string is the first argument after the text, (void*)&button_prg1
   and the second is where to return a new argument. GAI takes care of allocation and deallocation of
   text strings.
   Notice that button1_prg must be a valid pointer to a string or a NULL pointer.
*/


/* This is the main structure. It makes a notebook if there are more than one box else it
   just becomes one window. When you only have one box the text won't be shown. */

static GaiPI moe_pref2[] = {{GAI_TEXTENTRY, "Clicking on the left mouse button starts:", 
			     &button1_prg, &button1_prg},
			    {GAI_TEXTENTRY,
			     "Clicking on the middle mouse button starts:", 
			     &button2_prg, &button2_prg},
			    {GAI_END}};

/* This function is called by a timer. You select the timeout in ms. */

void moe_update(gboolean changed, gpointer d)
{
    /* Calculated next frame */
    if(frame_counter == MOE_FRAMES-1)
	frame_counter = 0;
    else
	frame_counter++;

    /* Draw next frame into a buffer that is dedicated to the background */
    gai_draw_bg(moe, 0, frame_counter*MOE_HEIGHT, 
		MOE_WIDTH, MOE_HEIGHT, 0,0);

    /* Update the changed background */
    gai_draw_update_bg();

    /* You draw things on the foreground or on the background. Background drawing
       is generally slower but shall be used when you update the graphics of the
       applet more seldom than about once a second. */


}

/* This function is called when the "Ok" or "Apply" button is pressed in the
   preference menu. */

void moe_save_settings(gpointer d)
{
    /* It saves the eventual changes of what program to execute on what button */
    gai_save_string("moe/prg1", button1_prg);
    gai_save_string("moe/prg2", button2_prg);
}

/* Function that is called when mouse button 1 is pressed */
void moe_button1(int x, int y, gpointer d)
{
    /* Execute button 1 program */
    gai_exec(button1_prg);
}

/* Function that is called when mouse button 2 is pressed */
void moe_button2(int x, int y, gpointer d)
{
    /* Execute button 2 program */
    gai_exec(button2_prg);
}



int main(int argc, char **argv)
{
    
    GaiFlagsType gf = GAI_FLAGS_NEVER_ROTATE;
    
    /* Initialized the gai library */

    gai_init2(&applet_defines, &argc, &argv);


    /* Load the saved image "moe.png" into a GdkPixbuf */
    moe = gai_load_image("moe.png");

    /* Load previous settings of what programs to run on the mouse clicks */
    /* And if its the first time, the default is to run no program at all.*/

    button1_prg = gai_load_string_with_default("moe/prg1", "");
    button2_prg = gai_load_string_with_default("moe/prg2", "");

    /* Generate a background of the size (MOE_WIDTH x MOE_HEIGHT) 
       Sets with GAI_BACKGROUND_MAX_SIZE_NONE that gai can scale it to any size
       it wishes depending on the size of the gnome panel. 
       FALSE says that don't make a border.
    */
    gai_background_set(MOE_WIDTH, MOE_HEIGHT,
		       GAI_BACKGROUND_MAX_SIZE_NONE, FALSE);
		       
    /* Tell GAI not to rotate the applet when you have the panel vertical
       oriented */
    gai_flags_set(gf);


    /* Sets up that gai shall call moe_update() each 1000 ms */
    gai_signal_on_update((GaiCallback0 *)moe_update, 1000, NULL);

    /* Connect the mouse button clicks to moe_button1(..) and moe_button2(...) */
    gai_signal_on_mouse_button_click((GaiCallback2 *)moe_button1, GAI_MOUSE_BUTTON_1, NULL);
    gai_signal_on_mouse_button_click((GaiCallback2 *)moe_button2, GAI_MOUSE_BUTTON_2, NULL);

    /* Tell gai to use the prefernce box that we defined at the top of this file. */
    gai_preferences2("Morning on earth - Preferences",
		    moe_pref2,
		    /* Help text */
		    "If you want to be able to start a program\n"
		    "by clicking on the earth, add them here.\n",
		    /* function to call when "ok" or "apply" is pressed */
		    (GaiCallback1 *)moe_save_settings, 
		    NULL);

    /* Start it all and just wait for inputs and call moe_update one a second. */
    gai_start();
    /* Never returns, but gcc is more happy with this line */
    return 0;
}
