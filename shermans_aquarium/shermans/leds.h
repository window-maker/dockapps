#ifndef LEDS_H
#define LEDS_H

#define LEDS_OFF 0
#define LEDS_NUMLOCK 1
#define LEDS_CAPSLOCK 2
#define LEDS_SCROLLOCK 3


#define LEDS_VIOLET 0
#define NUMLEDS 4

typedef struct
{

    /* Lying or standing? */
    int vert_horz;
    int horz, vert;
    int draw;
    int alpha;

    int leds_func[NUMLEDS], leds_colour[NUMLEDS], leds_show_off[NUMLEDS];


} Leds_settings;

void leds_init(void);
void leds_update(int);
void leds_exit(void);
Leds_settings *leds_get_settings_ptr(void);
#endif
