#ifndef FISH_H
#define FISH_H

#include "aquarium.h"

#define FISH1 0
#define FISH2 1
#define FISH3 2
#define FISH4 3
#define FISH5 4
#define FISH6 5
#define SQUID 6
#define SWORDFISH 7
#define BLOWFISH 8
#define ERNEST 9
#define HUNTER 10
#define LORI 11
#define PREY 12
#define SHERMAN 13
#define FILLMORE 14
#define BDWELLER 15
#define HAWTHORNE 16
#define MEGAN 17

/* Default size of fishes */
#define DEFAULT_SCALE 50

#define DEFAULT_SPEED 100

#define FULLSCREEN_DEFAULT_SCALE 60

/* how many fishes do you want in your aquarium? */
#define NRFISH 10
#define FULLSCREEN_NRFISH 20

/* The number of fishes you have graphic for */
#define NUMOFFISHTYPES 18


    
#define RANDOM_FISH 0
#define SELECTION_FISH 1
#define RANDOM_POP_FISH 2


/* structure describing each fish */
typedef struct {

    float tx;			/* current x position */
    int y;			/* current y position */
    int travel;			/* how far to move beyond the screen */
    int rev;			/* going left or right? */
    int frame;			/* current animation frame */
    float delay;		/* how quick we swap frames */
    int updown;			/* Was last moment up or down? - Larger chance to do it again. */
    int type;			/* Kind of fish */
    int width;
    int height;
    SA_Image *image;
    int *animation;
    int num_animation;
    float *speed;
    float speed_mul;
    float fast_frame_change;
    int is_dead;			/* Is the fish alive? I.E, eaten prey or punched blowfish? */
} Fish;


typedef struct {
    char *file;
    int pics;
    int frames;
    int *animation;
    float *speed;
} Fish_animation;



typedef struct
{
    int eat;
    int explode;
    int scale;
    
    /* Have a scale difference of +- 15% */
    int scale_diff;

    /* 100 = Original speed */
    int speed;

    int rebirth;

    int num_fish;
    int type;

    /* Agressiveness of hunter and swordfish */
    int swordfish_agr, hunter_agr;


    int fish1;
    int fish2;
    int fish3;
    int fish4;
    int fish5;
    int fish6;
    int swordfish;
    int blowfish;
    int fillmore;
    int sherman;
    int prey;
    int hunter;
    int lori;
    int ernest;
    int squid;
    int megan;
    int bdweller;
    int hawthorne;
} Fish_settings;

Fish *fish_get_fishes_ptr(void);
Fish_settings *fish_get_settings_ptr(void);
/*SA_Image *fish_get_blowup_data_ptr(void);
SA_Image *fish_get_prey_hunter_data_ptr(void);
Fish_animation **fish_get_fish_animation_ptr(void);*/

void fish_enter(void);
void fish_leave(void);
void fish_init(void);
void fish_turn(void);
void fish_exit(void);
void fish_update(void);

#endif
