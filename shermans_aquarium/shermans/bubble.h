#ifndef BUBBLE_H
#define BUBBLE_H

/* The number of frames for a bubble */
/* Don't change this, if you are not knowing what you are doing! */
#define BUBBLES_FRAMES 5

/* Is this physically illegal? */
#define VARIABLE_BUBBLE_SPEED



typedef struct {
    int x;			/* x position */
    float y;			/* y position */
    float speed;		/* The speed upwards the bubble has.
				   Is this physically wrong? Does all bubble
				   rise to the surface with the same speed? */
} Bubble;


typedef struct
{
    int max_bubbles;
} Bubble_settings;

Bubble_settings *bubble_get_settings_ptr(void);
void bubble_init(void);
void bubble_exit(void);
void bubble_update(void);

#endif
