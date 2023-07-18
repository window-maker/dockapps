#ifndef DRAW_H
#define DRAW_H
/* drawing */
#include "aquarium.h"

void draw_pic_alpha(unsigned char *, int, int, int,int, int, int);

/*void draw_fish(int, int, int, int, int, int, unsigned char *);*/
void draw_image(int, int, int, int, SA_Image *);
void draw_image_bg(int, int, int, int, SA_Image *);
void draw_image_alpha_h(int, int, int, int, SA_Image *);

void anti_line(int, int, int, int, int, int, int);
void putpixel(int, int, float, int, int);
void change_colour_to(int, int ,int, unsigned char *,GdkPixbuf *, int);
#endif
