#ifndef OVER_H
#define OVER_H
#include <gai/gai.h>
#include "aquarium.h"


#define OVER_OFF 0
#define OVER_MATRIX 1
#define OVER_PLASMA 2
#define OVER_TETRIS 3


typedef struct
{
    int fade;
    int cursor_off;
    int type;
} Over_settings;

void over_init(void);
void over_exit(void);
int over_update(int);
void over_draw(int, int, int, int, int, unsigned char*);
void over_keypress(int);
void over_joystick(GaiFlagsJoystick);
Over_settings *over_get_settings_ptr(void);
#endif
