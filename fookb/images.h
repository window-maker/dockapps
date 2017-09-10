/*
 * images.h
 * 
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 */

#ifndef IMAGES_H
#define IMAGES_H

int get_width();
int get_height();
void read_images(Display *dpy);
char *get_me_name(int);

//extern XImage *stupid_picture[5];	/* Icons for fookb */

void update_window(Window, GC, unsigned int, Display *);
#endif				/* IMAGES_H */
