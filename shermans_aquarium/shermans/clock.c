

/* Different kind of displaying the time. */


#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <gai/gai.h>



#include "aquarium.h"
#include "clock.h"
#include "draw.h"

/* This is for the Fuzzy clock */
#define FUZZY_IMAGES 18
#define TWENTY 12
#define QUARTER 13
#define HALF 14
#define OCLOCK 15
#define PAST 16
#define TO 17

static SA_Image *fuzzy_clock_data=NULL;


/* This is for the digital clock*/
#define DIGITAL_IMAGES 10
#define COLON 10
static SA_Image dig, colon;


/* This is for the analog clock */
static int old_sec, oldxmax, oldymax;;
static int sec_x,sec_y,min_x,min_y,hour_x,hour_y;
static unsigned int analog_final_colour_hour;
static unsigned int analog_final_colour_min;
static unsigned int analog_final_colour_sec;


/* Save settings lives inside this struct */
static Clock_settings clock_settings;
static AquariumData *ad;


static int what_is_init;

/* This returns the clock settings structure. */
Clock_settings *clock_get_settings_ptr(void)
{
    return &clock_settings;
}




void clock_exit_digital(void)
{

    if(dig.pixbuf!=NULL)
	g_object_unref(dig.pixbuf);
    if(colon.pixbuf!=NULL)
	g_object_unref(colon.pixbuf);

    memset(&dig,0,sizeof(SA_Image));
    memset(&colon,0,sizeof(SA_Image));

}
void clock_exit_analog(void)
{
    /* Nothing to do here */
}

void clock_exit_fuzzy(void)
{
    int i;

    for(i=0;;i++){
	if(fuzzy_clock_data[i].image==NULL) break;
	g_object_unref(fuzzy_clock_data[i].pixbuf);
    }
    
    g_free(fuzzy_clock_data);
    fuzzy_clock_data=NULL;
}


void clock_exit(void)
{
    if(what_is_init == CLOCK_DIGITAL)
	clock_exit_digital();
    if(what_is_init == CLOCK_ANALOG)
	clock_exit_analog();
    if(what_is_init == CLOCK_FUZZY)
	clock_exit_fuzzy();
}




void clock_init_digital(void)
{


    what_is_init = CLOCK_DIGITAL;

    if(dig.image!=NULL)
	clock_exit_digital();

    if(clock_settings.digital_fontsize == CLOCK_LARGE_FONT){
	load_image("clock/digital/bigfigures.png", &dig, DIGITAL_IMAGES);
	load_image("clock/digital/bigcolon.png",&colon,1);
    }
    else{
	load_image("clock/digital/smallfigures.png", &dig, DIGITAL_IMAGES);
	load_image("clock/digital/smallcolon.png", &colon,1);
    }
    

    change_colour_to(clock_settings.digital_colour.r,
		     clock_settings.digital_colour.g,
		     clock_settings.digital_colour.b,
		     dig.image,dig.pixbuf, FALSE);
    change_colour_to(clock_settings.digital_colour.r,
		     clock_settings.digital_colour.g,
		     clock_settings.digital_colour.b,
		     colon.image,colon.pixbuf, FALSE);

}

void clock_init_fuzzy(void)
{
    int i;
    char *fuzzy_image_names[] ={
	"clock/fuzzy/One.png",
	"clock/fuzzy/Two.png",
	"clock/fuzzy/Three.png",
	"clock/fuzzy/Four.png",
	"clock/fuzzy/Five.png",
	"clock/fuzzy/Six.png",
	"clock/fuzzy/Seven.png",
	"clock/fuzzy/Eight.png",
	"clock/fuzzy/Nine.png",
	"clock/fuzzy/Ten.png",
	"clock/fuzzy/Eleven.png",
	"clock/fuzzy/Twelve.png",
	"clock/fuzzy/Twenty.png",
	"clock/fuzzy/Quarter.png",
	"clock/fuzzy/Half.png",
	"clock/fuzzy/Oclock.png",
	"clock/fuzzy/Past.png",
	"clock/fuzzy/To.png",
	NULL
    };


    what_is_init = CLOCK_FUZZY;

    /* Should be possible to call this init more than once */
    if(fuzzy_clock_data!=NULL)
	clock_exit_fuzzy();

    fuzzy_clock_data = g_malloc0(sizeof(SA_Image)*FUZZY_IMAGES);

    for(i=0;;i++){
	if(fuzzy_image_names[i]==NULL) break;

	load_image(fuzzy_image_names[i], &fuzzy_clock_data[i],1);
	change_colour_to(clock_settings.fuzzy_colour.r,
			 clock_settings.fuzzy_colour.g,
			 clock_settings.fuzzy_colour.b,
			 fuzzy_clock_data[i].image,fuzzy_clock_data[i].pixbuf, FALSE);
	
    }

}

void clock_init_analog(void)
{

    what_is_init = CLOCK_ANALOG;


    /* Setting up colours */
    analog_final_colour_hour = 
	(((unsigned int)clock_settings.analog_colour_hour.r)<<16) +
	(((unsigned int)clock_settings.analog_colour_hour.g)<<8) +
	(unsigned int)clock_settings.analog_colour_hour.b;

    analog_final_colour_min = 
	(((unsigned int)clock_settings.analog_colour_min.r)<<16) +
	(((unsigned int)clock_settings.analog_colour_min.g)<<8) +
	(unsigned int)clock_settings.analog_colour_min.b;

    if(clock_settings.analog_seconds)
	analog_final_colour_sec = 
	    (((unsigned int)clock_settings.analog_colour_sec.r)<<16) +
	    (((unsigned int)clock_settings.analog_colour_sec.g)<<8) +
	    (unsigned int)clock_settings.analog_colour_sec.b;

    old_sec=oldxmax=oldymax=-1;
}


/* Loads graphics - Can be called upon a restart or changed settings. */

void clock_init(void)
{

    ad = aquarium_get_settings_ptr();

    if (clock_settings.type == CLOCK_OFF)
	return;

    if(clock_settings.type == CLOCK_DIGITAL)
	clock_init_digital();

    if(clock_settings.type == CLOCK_ANALOG)
	clock_init_analog();

    if(clock_settings.type == CLOCK_FUZZY)
	clock_init_fuzzy();

}




void clock_update_fuzzy(int hour, int min, int sec)
{
    int i,x=4,y=4;
    int to_or_past, ptr2img[5], num_ptr;
    int largest_one=-1;

    /* Convert min to closes 5 min */
    min=((int)((min*2+5)/10)*5);

    if(min>30){
	min=60-min;
	to_or_past=TO;
	hour++;
    }
    else
	to_or_past=PAST;


    /* Convert from 24 format to 12 hour format.*/
    if(hour>12) hour-=12;

    /* X minutes past zero doesnt exist */
    if(hour==0) hour=12;

    /* Most have tree lines */
    num_ptr = 3;
    ptr2img[1]=to_or_past;
    ptr2img[2]=hour-1;

    switch(min){
    case 0:
	num_ptr = 2;
	ptr2img[0]=hour-1;
	ptr2img[1]=OCLOCK;
	break;
    case 5:
	ptr2img[0]=4;	/* Five */
	break;
    case 10:
	ptr2img[0]=9;	/* Ten */
	break;
    case 15:
	ptr2img[0]=QUARTER;
	break;
    case 20:
	ptr2img[0]=TWENTY;
	break;
    case 25:
	num_ptr = 4;
	ptr2img[0]=TWENTY;
	ptr2img[1]=4;
	ptr2img[2]=to_or_past;
	ptr2img[3]=hour-1;
	break;
    case 30:
	ptr2img[0]=HALF;
	break;

    default:
	printf("I don't understand this kind of time.sorry\n");
    }

    /* Assume the images have about the same height */

    if(clock_settings.vert == TOP)
	y=2;
    else if(clock_settings.vert == CENTER)
	y=(ad->ymax - num_ptr*(fuzzy_clock_data[0].height+2))/2;
    else if(clock_settings.vert == BOTTOM)
	y=ad->ymax - num_ptr*(fuzzy_clock_data[0].height+2)-2;


    /* Find the widest one */
    for(i=0;i<num_ptr;i++){
	if(largest_one < fuzzy_clock_data[ptr2img[i]].width)
	    largest_one = fuzzy_clock_data[ptr2img[i]].width;
    }
    

    for(i=0;i<num_ptr;i++){
	    
	if(clock_settings.horz == LEFT)
	    x = 2+(largest_one - fuzzy_clock_data[ptr2img[i]].width)/2;
	else if (clock_settings.horz == CENTER)
	    x=(ad->xmax-fuzzy_clock_data[ptr2img[i]].width)/2;
	else if (clock_settings.horz == RIGHT)
	    x=(ad->xmax-fuzzy_clock_data[ptr2img[i]].width - 
	       (largest_one - fuzzy_clock_data[ptr2img[i]].width)/2-2);

	draw_pic_alpha(fuzzy_clock_data[ptr2img[i]].image,
		       fuzzy_clock_data[ptr2img[i]].width,
		       fuzzy_clock_data[ptr2img[i]].height,
		       x,y,0,(int)clock_settings.fuzzy_colour.alpha);
	y+=fuzzy_clock_data[ptr2img[i]].height+2;
    }


}

/* Analoge clock */

void clock_update_analog(int hour, int min, int sec)
{
    int size_l_x,size_r_x,size_b_y, size_t_y;
    int center_x=0, center_y=0;
    int curr_x, curr_y;


    if(clock_settings.horz == RIGHT){
	if (ad->xmax < ad->ymax) center_x = ad->xmax/2;
	else center_x=ad->xmax-ad->ymax/2;
    }
    else if(clock_settings.horz == CENTER)
	center_x = ad->xmax/2;

    else if(clock_settings.horz == LEFT){
	if (ad->xmax < ad->ymax) center_x = ad->xmax/2;
	else center_x=ad->ymax/2;
    }

    if(clock_settings.vert == TOP){
	if (ad->xmax > ad->ymax) center_y = ad->ymax/2;
	else center_y=ad->xmax/2;
    }
    else if(clock_settings.vert == CENTER)
	center_y=ad->ymax/2;

    else if(clock_settings.vert == BOTTOM){
	if (ad->xmax > ad->ymax) center_y = ad->ymax/2;
	else center_y=ad->ymax-ad->xmax/2;
    }




    /* Check if we shall keep the clock circular or not.*/
    if(clock_settings.analog_keep_circular){
	if(ad->xmax > ad->ymax)
	    size_l_x=size_r_x=size_t_y=size_b_y=center_y; //ad->ymax;
	else
	    size_l_x=size_r_x=size_t_y=size_b_y=center_x ;//ad->xmax;
    }
    else{
	size_r_x = ad->xmax - center_x;
	size_l_x = center_x;
	size_t_y = center_y;
	size_b_y = ad->ymax - center_y;
    }
 


    /* Only calculate new placement when it is nessecary, not each frame */

    if(clock_settings.analog_seconds){
	if(old_sec!=sec || oldxmax!=ad->xmax || oldymax!=ad->ymax){
	    old_sec=sec;

	    if(sec>30) curr_x = size_l_x;
	    else curr_x = size_r_x;
	    if(sec <15 || sec >45) curr_y = size_t_y;
	    else curr_y=size_b_y;
	    
	    sec_x=(int)(((float)curr_x*0.9)*cos((2*M_PI*(float)sec)/60.0-M_PI/2));
	    sec_y=(int)(((float)curr_y*0.9)*sin((2*M_PI*(float)sec)/60.0-M_PI/2));
	}
    }

    if(sec==0 || oldxmax!=ad->xmax || oldymax!=ad->ymax){

	if(min>30) curr_x = size_l_x;
	else curr_x = size_r_x;
	if(min <15 || min >45) curr_y = size_t_y;
	else curr_y=size_b_y;

	min_x=(int)(((float)curr_x*0.74)*cos((2*M_PI*(float)min)/60.0-M_PI/2));
	min_y=(int)(((float)curr_y*0.74)*sin((2*M_PI*(float)min)/60.0-M_PI/2));

	if(hour>6) curr_x = size_l_x;
	else curr_x = size_r_x;
	if(sec <3 || sec >9) curr_y = size_t_y;
	else curr_y=size_b_y;

	hour_x=(int)(((float)curr_x*0.58)*cos(2*M_PI*((float)hour+(float)min/60.0)/12.0-M_PI/2));
	hour_y=(int)(((float)curr_y*0.58)*sin(2*M_PI*((float)hour+(float)min/60.0)/12.0-M_PI/2));
    }

    oldxmax=ad->xmax;
    oldymax=ad->ymax;
   
    anti_line(center_x,center_y,
	      center_x+hour_x,
	      center_y+hour_y,
	      1,analog_final_colour_hour,1);

    anti_line(center_x,center_y,
	      center_x+min_x,
	      center_y+min_y,
	      1,analog_final_colour_min,1);
    
    if(clock_settings.analog_seconds){
	anti_line(center_x,center_y,
		  center_x+sec_x,
		  center_y+sec_y,
		  1,analog_final_colour_sec,1);
    }

}

void clock_update_digital(int hour, int min, int sec)
{
    int sum_width, list[10], list_max,i;
    int x=0,y=0;

    list[0]=hour/10;
    list[1]=hour%10;
    list[2]=COLON;
    list[3]=min/10;
    list[4]=min%10;
    list_max=5;



    if(clock_settings.digital_seconds) {
	list[5]=COLON;
	list[6]=sec/10;
	list[7]=sec%10;
	list_max=8;
	sum_width = 2*colon.width + 6*dig.width;
    }
    else {
	sum_width = colon.width + 4*dig.width;
    }



    if(clock_settings.horz == LEFT)
	x = 2;
    else if(clock_settings.horz == CENTER)
	x = (ad->xmax - sum_width)/2;
    else if(clock_settings.horz == RIGHT)
	x = ad->xmax - sum_width - 2;

    if(clock_settings.vert == TOP)
	y = 2;
    else if(clock_settings.vert == CENTER)
	y = (ad->ymax - dig.height)/2;
    else if(clock_settings.vert == BOTTOM)
	y = ad->ymax - dig.height - 2;


    /*Digital Clock*/

    for(i=0;i<list_max;i++){
	if(list[i]==COLON){
	    if(clock_settings.digital_blinking && (sec%2)){
		x+=colon.width;
		continue;
	    }
	    draw_pic_alpha(colon.image, colon.width, colon.height, x, y,
			   0, (int)clock_settings.digital_colour.alpha);
	    	x+=colon.width;
	}
	else{
	    draw_pic_alpha(dig.image, dig.width, dig.height, x, y,
			   list[i], (int)clock_settings.digital_colour.alpha);
	    x+=dig.width;
	}
    }	

}



/* Shall be called both before and after fish are drawn */
void clock_update(int beforeorafter)
{
    time_t now;
    struct tm *mt;

    if (clock_settings.type == CLOCK_OFF)
	return;

    if(beforeorafter==clock_settings.draw){

	now = time(NULL);
	mt = localtime(&now);


	if(clock_settings.type == CLOCK_DIGITAL)
	    clock_update_digital(mt->tm_hour,mt->tm_min,mt->tm_sec);
	if(clock_settings.type == CLOCK_ANALOG)
	    clock_update_analog(mt->tm_hour,mt->tm_min,mt->tm_sec);
	if(clock_settings.type == CLOCK_FUZZY)
	    clock_update_fuzzy(mt->tm_hour,mt->tm_min,mt->tm_sec);
    }
}




