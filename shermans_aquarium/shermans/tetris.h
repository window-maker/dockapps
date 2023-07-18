#ifndef TETRIS_H
#define TETRIS_H

#include <gai/gai.h>

#include "aquarium.h"
typedef struct
{
    int score;
    int level;
    int lines;
    gboolean size_limit;
    int height, width;
} Tetris_highscore_table;

typedef struct
{
    gboolean size_limit;
    gboolean show_next;
    int height, width;
} Tetris_settings;

Tetris_highscore_table *tetris_get_highscore_table_ptr(void);
Tetris_settings *tetris_get_settings_ptr(void);
void tetris_init(void);
void tetris_exit(void);
void tetris_start(void);
void tetris_end(void);
void tetris_update(void);
void tetris_keypress(int);
void tetris_joystick(GaiFlagsJoystick);

#endif
