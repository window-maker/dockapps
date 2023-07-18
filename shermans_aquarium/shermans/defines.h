#ifndef DEFINES_H
#define DEFINES_H


/*
  Magic for the graphics function to know when they are called
  after or before the fish are drawn.
  Best kept here.
*/

#define DRAW_BEFORE 0
#define DRAW_AFTER 1

#define MAX_COMICS 8

#define SHERMAN_PATH "shermans_aquarium/"

#define LEFT 0
#define CENTER 1
#define RIGHT 2
#define TOP 0
#define BOTTOM 2




/* This defines how large the CPU load buffer shall be. Having a
   small value makes the termometer response very fast on the
   different CPU load. With 20 it is responsing quite smooth.

*/

#define CPUSMOOTHNESS 20

/* perform clipping outside this range, also this is the size of the
 * drawing area */
/* This is how much that is seen on the screen of the aquarium */

#define XMIN 4
#define XMAX 56
#define YMIN 4
#define YMAX 56

#define WINDOWSIZE_X 64
#define WINDOWSIZE_Y 64


/* How large the aquarium that the fishes lives in. */

#define VIRTUAL_AQUARIUM_DX 80
#define VIRTUAL_AQUARIUM_DY 40

#define VIEWPOINT_START_X  60
#define VIEWPOINT_START_Y  40


#endif
