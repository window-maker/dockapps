#ifndef SOUNDEFFECTS_H
#define SOUNDEFFECTS_H

void sound_eatscream(void);
void sound_explode(void);
void sound_bubbles(void);

#define TYPE_MP3 0
#define TYPE_OGG 1

typedef struct {
    int on;
    int type;
    char *prg;
} Sound_settings;

Sound_settings *sound_get_settings_ptr(void);

#endif
