#ifndef BACKGROUND_H
#define BACKGROUND_H

#define BG_SOLID 0 
#define BG_SHADED 1
#define BG_WATER 2
#define BG_IMAGE 3


typedef struct
{
    char *imagename, *imagename_new;
    int type;
    int desktop;
    GaiColor shaded_top_c, shaded_bot_c, solid_c;

} Background_settings;

Background_settings *background_get_settings_ptr(void);
void background_init(void);
void background_exit(void);
#endif
