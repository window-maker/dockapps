#ifndef BOTTOM_H
#define BOTTOM_H

#define MAXPLANTS 4

#define DEFAULT_HAWTHORNE 2
#define DEFAULT_BDWELLER 2

#define BOTTOM_SCALE 30

#define NUMBOTTOMIMAGES 3

/* Number of plants, stones and none moving bottom things */
/* Only change if you add new graphics. */
#define NUMOFBOTTOMITEMS 18

#define DEFAULT_BOTTOM_ANIMALS 1



typedef struct
{
    int have_sea_floor;
    int max_plants;
    int random_plants;
    int scale;
    int num_bottom_animals;
} Bottom_settings;


typedef struct {
    char *image;
    int flags;
} Bottom;


Bottom_settings *bottom_get_settings_ptr(void);
void bottom_init(void);
void bottom_exit(void);

#endif
