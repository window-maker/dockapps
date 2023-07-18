
#define NUM_FIGURES 10
#define NUM_MONTHS 12
#define NUM_DAYS 7

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <gai/gai.h>
#include "aquarium.h"
#include "date.h"
#include "draw.h"

static SA_Image figures, months, days;
static Date_settings date_settings;
static AquariumData *ad;

static int month_width[]={16,16,20,18,20,16,16,20,17,18,19,18};

Date_settings *date_get_settings_ptr(void)
{
    return &date_settings;
}


void date_init()
{
    ad = aquarium_get_settings_ptr();

    if(!date_settings.on)
	return;

    if(figures.image!=NULL)
	date_exit();

    load_image("clock/date/figures.png",&figures, NUM_FIGURES);

    load_image("clock/date/weekdays.png",&days, NUM_DAYS);

    load_image("clock/date/months.png", &months, NUM_MONTHS);


    change_colour_to(date_settings.c.r,
		     date_settings.c.g,
		     date_settings.c.b,
		     figures.image,
		     figures.pixbuf, TRUE);


    change_colour_to(date_settings.c.r,
		     date_settings.c.g,
		     date_settings.c.b,
		     days.image,
		     days.pixbuf, TRUE);

    change_colour_to(date_settings.c.r,
		     date_settings.c.g,
		     date_settings.c.b,
		     months.image,
		     months.pixbuf,TRUE);


}

void date_exit(void)
{
    if(figures.pixbuf!=NULL)
	g_object_unref(figures.pixbuf);
    if(days.pixbuf!=NULL)
	g_object_unref(days.pixbuf);
    if(months.pixbuf!=NULL)
	g_object_unref(months.pixbuf);

    memset(&figures,0,sizeof(SA_Image));
    memset(&days,0,sizeof(SA_Image));
    memset(&months,0,sizeof(SA_Image));

}

void date_update(int beforeorafter)
{
    int x=0,y=0;
    int wsize;
    time_t now;
    struct tm *mt;

    if(!date_settings.on)
	return;


    if(beforeorafter==date_settings.draw) {
	now = time(NULL);
	mt = localtime(&now);

	wsize=2*figures.width+days.width+month_width[mt->tm_mon]+1;


	/* If before day 10 in month */
	if(mt->tm_mday <10) wsize-=figures.width;

    
	switch(date_settings.vert)
	    {
	    case TOP:
		y=2;
		break;
	    case CENTER:
		y=(ad->ymax-figures.height)/2;
		break;
	    case BOTTOM:
		y=ad->ymax-figures.height-1;
		break;
	    }

	switch(date_settings.horz)
	    {
	    case LEFT:
		x=0;
		break;
	    case CENTER:
		x=(ad->xmax-wsize)/2;
		break;
	    case RIGHT:
		x=ad->xmax-wsize;
	    }

	draw_pic_alpha(days.image,
		       days.width,
		       days.height,
		       x,y,mt->tm_wday,date_settings.c.alpha);
	x+=days.width+2;
    
	if(mt->tm_mday >9){
	    draw_pic_alpha(figures.image,
			   figures.width,
			   figures.height,
			   x,y,mt->tm_mday/10,date_settings.c.alpha);
	    x+=figures.width-1;
	}
	draw_pic_alpha(figures.image,
		       figures.width,
		       figures.height,
		       x,y,mt->tm_mday%10,date_settings.c.alpha);

	x+=figures.width;

	draw_pic_alpha(months.image,
		       months.width,
		       months.height,
		       x,y,mt->tm_mon,date_settings.c.alpha);
    }

}
