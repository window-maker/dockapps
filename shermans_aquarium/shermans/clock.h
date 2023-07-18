
/* Header file for Sherman's aquarium's clock <cja@gmx.net> */

#ifndef CLOCK_H
#define CLOCK_H

/* Kind of clock */ 
#define CLOCK_OFF 0
#define CLOCK_DIGITAL 2
#define CLOCK_ANALOG 1
#define CLOCK_FUZZY 3

#define CLOCK_LARGE_FONT 0
#define CLOCK_SMALL_FONT 1

/* Settings that are saved, gui selected or given at command line */

typedef struct
{
    int type;			/* 0=Off, 1=Digital, 2=Analog, 3=Fuzzy */
    //    int with_seconds;	/* 0=No seconds, 1=With seconds */
    //    int when_update;	/* 0=Updates before fish, 1=After */
    int horz;			/* Horizontal placement, 0=Left, 1=Center, 2=Right*/
    int vert;			/* Vertical placement, 0=Top, 1=Center, 2=Bottom*/
	
    /* For the digital clock */
    GaiColor digital_colour;	/* RGB settings for digital clock */


    int digital_blinking;	/* If no seconds, 1=Blink colon */
    //    int digital_font_size;	/* 0=Small font, 1=Large font */

    /* For the fuzzy clock */
    GaiColor fuzzy_colour;		/* Color for fuzzy clock */

    /* For the analog clock */
    GaiColor analog_colour_hour;	/* Colour of all the pointers */
    GaiColor analog_colour_min;
    GaiColor analog_colour_sec;

    int analog_keep_circular;	/* 0=Size adjusts after size of applet, 
				   1=Make it circular. */

    /* New better suited for GAI pref */
    int analog_seconds;
    int digital_seconds;


    int draw;
    int digital_fontsize;

} Clock_settings;



void clock_init(void);
void clock_update(int);
void clock_exit(void);
Clock_settings *clock_get_settings_ptr(void);

#endif
