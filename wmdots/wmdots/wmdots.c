// WM-Dots the Window Maker 3d Rotating Dots Demo by Mark I Manning IV
// =======================================================================

// This demo is based on an old DOS demo I did a number of years ago.  The
// trick was to get it to work in an 64 x 64 window instead of the
// 320 x 200 of the original. (and in 16 instead of 256 colours :)

// Much of these sources are based on other dockable application sources.
// For this reason, and because it is the right thing to do, this file is
// distributed under the GPL.  Use it and abuse it as you will.

// Flames, critisizm and general nukings can be sent to....

//  i440r@mailcity.com

// I have included the sources (in 100% pure ASM) to the afore mentioned
// original demo.  These you can also consider covered by the GPL.

// Maybe someone can help me convert the bump mapping in the original to
// use only 16 colours :)  (actually, I have space in the master.xpm for
// 32 colours if I retain a 2 x 2 pixle size.  If i go to 1 x 1 i can
// fit 64 colours in there :)

// Yea... I know... Im supposed to be sparing of system resources like
// colour allocations but come on guys... this IS a demo after all :)
// Demos are supposed to be abusive of system resources eh??? :)

//  This source file is 100% TAB FREE. Use any editor/viewer you want
//  to view it with and set ANY size tabs and it will STILL look right.
//  TABS ARE EVIL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Also Linus T is wrong. Indentations of 8 are an abomination and so is
// k&r source formatting.  So There!

// Apart from that... Linus for president :)
//                    or George W. Bush!!!

// -----------------------------------------------------------------------
// blah :)

// Version 0.1 beta     Released Saturday October 9 1999
//
//    To do...  Fix bug that causes core dump when the "cross" object
//               rotates outside window (its not a clipping problem).
//              Find out why the "cross" object is not cross shaped in
//               this demo when it is in the dos version.
//              Change pixle sizes to 1 x 1 for objects that are in
//               the distance.
//              Move all object definitions out of this file and into
//               some kind of rc file and allow user defined objects
//              Add greets to the credits display :)
//              Impliment better light shading (in object space?)
//              Prevent dark pixles overwriting light ones
//
//    Done...   Scrapped idea about different pixle sizez, it didn't look
//               as good as i thunked it would :)
//              Fixed adjust_angle() to be angle +128 not angle +127
//              Fixed cross by defining xyz as char instead of int
//              Fixed core dump problem with cross:  Oopts it was a
//               clipping problem, z clipping points behind camera! :)
//              Added size macro (prolly a bad name for it tho :)
//              Implimented better light shading but its still not
//               quite right yet...
//              Removed math.h from includes as im not using any
//               functions therein.  Saved a whopping 4 bytes :)
//              Added code to draw_point() to prevent dark pels from
//               being drawn over lighter pels.  Dark pels are further
//               away and should be BEHIND the lighter ones.  There is
//               almost no cpu bash here and I dont need to z sort pels
//
//
// Version 0.2 beta     Released Saturday October 9 1999
//
//    To do..   Still gotta move objects to rc file....
//
//
//
//    Done...

// -----------------------------------------------------------------------
// includes

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/xpm.h>
#include "../wmgeneral/wmgeneral.h"
#include "master.xpm"
#include "sintab.c"             // sin() and cos() are crap!

// -----------------------------------------------------------------------
// defines

#define PEL_SIZE 2              // 1 or 2 only

#define MAX_POINTS 125          // max # pels in an object

#define MASK_WIDTH 64           // window bitmap mask dimentions
#define MASK_HEIGHT 64
#define MASK_SIZE MASK_WIDTH * MASK_HEIGHT

// adjust angle for cos table lookup.  cos = right angle further
// on in sin table, circle = 512 degrees...

#define adjust(angle) ((angle + 128) & 0x1ff)
#define size(array) ((sizeof(array) / sizeof(array[0])))

// -----------------------------------------------------------------------
// typedefs...

typedef struct                  // a point in 3d space
{
  char x, y, z;
}xyz;

typedef struct                  // a point in 2d space
{
  int x, y;
}xy;

typedef struct
{
  int counter;                  // count down to next speed/shape change
  int reset;                    // initial value of counter
  int *ptr;                     // pointer to item to modify
  int delta;                    // ammount to change item by
  int upper;                    // upper limit for item
  int lower;                    // lower limit for item
}modifier;

// -----------------------------------------------------------------------
// object structures...

xyz cube[]=
{
  {-35,-35,-35},{-35,-35,-15},{-35,-35, 05},{-35,-35, 25},{-35,-35, 45},
  {-35,-15,-35},{-35,-15,-15},{-35,-15, 05},{-35,-15, 25},{-35,-15, 45},
  {-35, 05,-35},{-35, 05,-15},{-35, 05, 05},{-35, 05, 25},{-35, 05, 45},
  {-35, 25,-35},{-35, 25,-15},{-35, 25, 05},{-35, 25, 25},{-35, 25, 45},
  {-35, 45,-35},{-35, 45,-15},{-35, 45, 05},{-35, 45, 25},{-35, 45, 45},
  {-15,-35,-35},{-15,-35,-15},{-15,-35, 05},{-15,-35, 25},{-15,-35, 45},
  {-15,-15,-35},{-15,-15,-15},{-15,-15, 05},{-15,-15, 25},{-15,-15, 45},
  {-15, 05,-35},{-15, 05,-15},{-15, 05, 05},{-15, 05, 25},{-15, 05, 45},
  {-15, 25,-35},{-15, 25,-15},{-15, 25, 05},{-15, 25, 25},{-15, 25, 45},
  {-15, 45,-35},{-15, 45,-15},{-15, 45, 05},{-15, 45, 25},{-15, 45, 45},
  { 05,-35,-35},{ 05,-35,-15},{ 05,-35, 05},{ 05,-35, 25},{ 05,-35, 45},
  { 05,-15,-35},{ 05,-15,-15},{ 05,-15, 05},{ 05,-15, 25},{ 05,-15, 45},
  { 05, 05,-35},{ 05, 05,-15},{ 05, 05, 05},{ 05, 05, 25},{ 05, 05, 45},
  { 05, 25,-35},{ 05, 25,-15},{ 05, 25, 05},{ 05, 25, 25},{ 05, 25, 45},
  { 05, 45,-35},{ 05, 45,-15},{ 05, 45, 05},{ 05, 45, 25},{ 05, 45, 45},
  { 25,-35,-35},{ 25,-35,-15},{ 25,-35, 05},{ 25,-35, 25},{ 25,-35, 45},
  { 25,-15,-35},{ 25,-15,-15},{ 25,-15, 05},{ 25,-15, 25},{ 25,-15, 45},
  { 25, 05,-35},{ 25, 05,-15},{ 25, 05, 05},{ 25, 05, 25},{ 25, 05, 45},
  { 25, 25,-35},{ 25, 25,-15},{ 25, 25, 05},{ 25, 25, 25},{ 25, 25, 45},
  { 25, 45,-35},{ 25, 45,-15},{ 25, 45, 05},{ 25, 45, 25},{ 25, 45, 45},
  { 45,-35,-35},{ 45,-35,-15},{ 45,-35, 05},{ 45,-35, 25},{ 45,-35, 45},
  { 45,-15,-35},{ 45,-15,-15},{ 45,-15, 05},{ 45,-15, 25},{ 45,-15, 45},
  { 45, 05,-35},{ 45, 05,-15},{ 45, 05, 05},{ 45, 05, 25},{ 45, 05, 45},
  { 45, 25,-35},{ 45, 25,-15},{ 45, 25, 05},{ 45, 25, 25},{ 45, 25, 45},
  { 45, 45,-35},{ 45, 45,-15},{ 45, 45, 05},{ 45, 45, 25},{ 45, 45, 45}
};

xyz star[]=
{
  {-35,-35, 05},{-35,-15, 05},{-35, 05, 05},{-35, 25, 05},{-35, 45, 05},
  {-15,-35, 05},{-15,-15, 05},{-15, 05, 05},{-15, 25, 05},{-15, 45, 05},
  { 05,-35, 05},{ 05,-15, 05},{ 05, 05, 05},{ 05, 25, 05},{ 05, 45, 05},
  { 25,-35, 05},{ 25,-15, 05},{ 25, 05, 05},{ 25, 25, 05},{ 25, 45, 05},
  { 45,-35, 05},{ 45,-15, 05},{ 45, 05, 05},{ 45, 25, 05},{ 45, 45, 05},
  { 05,-35,-35},{ 05,-35,-15},{ 05,-35, 25},{ 05,-35, 45},{-35, 05,-35},
  { 05,-15,-35},{ 05,-15,-15},{ 05,-15, 25},{ 05,-15, 45},{-15, 05,-35},
  { 05, 05,-35},{ 05, 05,-15},{ 05, 05, 25},{ 05, 05, 45},{ 05, 05,-35},
  { 05, 25,-35},{ 05, 25,-15},{ 05, 25, 25},{ 05, 25, 45},{ 25, 05,-35},
  { 05, 45,-35},{ 05, 45,-15},{ 05, 45, 25},{ 05, 45, 45},{ 45, 05,-35},
  {-35, 05,-15},{-35, 05, 25},{-35, 05, 45},{-15, 05,-15},{-15, 05, 25},
  {-15, 05, 45},{ 05, 05,-15},{ 05, 05, 25},{ 05, 05, 45},{ 25, 05,-15},
  { 25, 05, 25},{ 25, 05, 45},{ 45, 05,-15},{ 45, 05, 25},{ 45, 05, 45}
};

xyz dots[]=
{
  {-35,-35,-35},{-35,-35, 45},{ 45, 45,-35},{ 45, 45, 45},{ 5,-35, 5},
  {  5, 45,  5}
};

xyz square[]=
{
  {-35,-35,-35},{-35,-15,-35},{-35, 05,-35},{-35, 25,-35},{-35, 45,-35},
  {-15,-35,-35},{-15,-15,-35},{-15, 05,-35},{-15, 25,-35},{-15, 45,-35},
  { 05,-35,-35},{ 05,-15,-35},{ 05, 05,-35},{ 05, 25,-35},{ 05, 45,-35},
  { 25,-35,-35},{ 25,-15,-35},{ 25, 05,-35},{ 25, 25,-35},{ 25, 45,-35},
  { 45,-35,-35},{ 45,-15,-35},{ 45, 05,-35},{ 45, 25,-35},{ 45, 45,-35}
};

xyz cross[]=
{
  {0x00,0x00,0x19}, {0x00,0x05,0x19}, {0x00,0x14,0x01}, {0x00,0x32,0x00}, 
  {0x00,0x7D,0x00}, {0x00,0xC9,0x00}, {0x01,0x5F,0x01}, {0x01,0xAB,0x01}, 
  {0x01,0xF6,0x01}, {0x02,0x41,0x02}, {0x02,0x8D,0x02}, {0x02,0xD8,0x02}, 
  {0x03,0x23,0x03}, {0x03,0x05,0x04}, {0x03,0x6F,0x03}, {0x03,0xBA,0x03}, 
  {0x04,0x51,0x04}, {0x04,0x9C,0x04}, {0x04,0xE7,0x04}, {0x05,0x13,0x06},
  {0x05,0x32,0x05}, {0x05,0x7D,0x05}, {0x05,0xC8,0x05}, {0x06,0x5E,0x06}, 
  {0x06,0xA9,0x06}, {0x06,0xF4,0x06}, {0x07,0x3F,0x07}, {0x07,0x8A,0x07}, 
  {0x07,0xD5,0x07}, {0x08,0x00,0x09}, {0x08,0x20,0x08}, {0x08,0x6B,0x08}, 
  {0x08,0xB5,0x08}, {0x09,0x4B,0x09}, {0x09,0x95,0x09}, {0x09,0xE0,0x09}, 
  {0x0A,0x09,0x0B}, {0x0A,0x2A,0x0A}, {0x0A,0x75,0x0A}, {0x0A,0xBF,0x0A},
  {0x0B,0x54,0x0B}, {0x0D,0x07,0x26}, {0x0F,0x02,0x28}, {0x19,0x09,0x32}, 
  {0x1E,0x04,0x37}, {0x22,0x0B,0x3B}, {0x2C,0x06,0x45}, {0x2D,0x01,0x46}, 
  {0x39,0x08,0x52}, {0x3D,0x03,0x56}, {0x43,0x0A,0x5C}, {0x4B,0x00,0x64}, 
  {0x4B,0x05,0x64}, {0x58,0x07,0x71}, {0x5B,0x02,0x74}, {0x64,0x09,0x7C}, 
  {0x84,0x08,0x9C}, {0x88,0x03,0xA1}, {0x8D,0x0A,0xA6}, {0x96,0x00,0xAF},
  {0x96,0x05,0xAF}, {0xA3,0x07,0xBC}, {0xA6,0x02,0xBF}, {0xAE,0x09,0xC7}, 
  {0xB5,0x04,0xCE}, {0xC2,0x06,0xDB}, {0xC4,0x01,0xDD}, {0xCE,0x08,0xE7}, 
  {0xD3,0x03,0xEC}, {0xD8,0x0A,0xF1}, {0xE1,0x05,0xFA}, {0xE2,0x00,0xFB}, 
  {0xEE,0x07,0x07}, {0xF1,0x02,0x0A}, {0xF9,0x09,0x11}
};

// -----------------------------------------------------------------------
// object lists...

xyz *obj_list[]=                // pointers to each object
{
  (xyz *) &cube,
  (xyz *) &dots,
  (xyz *) &star,
  (xyz *) &square,
  (xyz *) &cross
};

int point_counts[]=             // number of points in each object
{
  size(cube),
  size(dots),
  size(star),
  size(square),
  size(cross)
};

#define NUM_OBJECTS size(obj_list)

// -----------------------------------------------------------------------
// i love global variables :)

// I hate 40 page functions with 50 levels if,and and but loop nesting

char *ProgName = NULL;
char *Version = "0.2 Beta";

int x_angle = 0;                // angles of rotation in each axis
int y_angle = 0;
int z_angle = 0;

short cos_x = 0;                // trig stuff
short cos_y = 0;
short cos_z = 0;
short sin_x = 0;
short sin_y = 0;
short sin_z = 0;

int x_off = 30;                 // world space position of object
int y_off = 30;
int z_off = 150;

int delta_x = 1;                // rotational speed of object
int delta_y = 1;
int delta_z = 1;

int itters = 1;                 // number of frames till shape change
int num_points = 0;             // number of points in object
int obj_number = 0;
xyz *object = NULL;             // pointer to current object

xy trail_1[125];                // frame histories
xy trail_2[125];
xy trail_3[125];

char *p_buff;                   // addr of pel buffer ( see draw_point() )

// -----------------------------------------------------------------------
//

modifier w1 =                   // changes x rotation speed
{
   30,                          // count down counter
   34,                          // reset value for countdown counter
   &delta_x,                    // item to modify on count = 0
   2,                           // ammount to add to item
   8,                           // upper limit for item
   0,                           // lower limit for item
};

modifier w2 =                   // changes y rotation speed
{
  20, 20, &delta_y, -1, 6, 0
};

modifier w3 =                   // changes z rotation speed
{
  30, 30, &delta_z, 1, 7, 0
};

modifier w4 =                   // zooms object in / out of window
{
  4, 4, &z_off, -2, 200, 90
};

// modifier w5 =                // these two do some funky things with
// {                            // object space but they tend to make
//   10,10,&x_off,-3,30,10      // the objects small and look further
// };                           // away so im not using them
//
// modifier w6 =                // but go ahead and take a look at what
// {                            // they do :)
//   10,30,&y_off,-5,30,10
// };

// -----------------------------------------------------------------------
// draw a point at x/y in specified colour

void draw_point(int x, int y, int c)
{
  char *p;

  if(x != -1)                   // is point clipped ?
  {
    c <<= 1;                    // adjust c for xpm lookup of colour
    p = p_buff + (x + (MASK_WIDTH * y));

    if (*p > c)                 // if pel at *p is higher its darker so
    {                           // its ok to overwrite it
      *p = c;                   // remember pel colour at this position
      copyXPMArea(c, 65, PEL_SIZE, PEL_SIZE, x, y);
    }
  }
}

// -----------------------------------------------------------------------
// Erase previously drawn point (draw again in BLACK!!!)

void erase_point(int x, int y)
{
  if(x != -1)                   // is point clipped?
  {
    copyXPMArea(34, 65, PEL_SIZE, PEL_SIZE, x, y);
  }
}

// -----------------------------------------------------------------------
// Process pending X events

void do_pending(void)
{
  XEvent Event;

  while (XPending(display))             // for all pending events do...
  {
    XNextEvent(display, &Event);        // get event type

    switch (Event.type)                 // we are only interested in...
    {
      case Expose:                      // expose events and...
        RedrawWindow();
      break;

      case DestroyNotify:               // our own "pending" demise :)
        XCloseDisplay(display);
        exit(0);
      break;
    }
  }
}

// -----------------------------------------------------------------------
// clear frame history buffers

void clear_histories(void)
{
  int i = MAX_POINTS;

  while (i--)                           // for loops suck
  {
    trail_1[i].x = trail_1[i].y = -1;   // -1 = invalid coordinate
    trail_2[i].x = trail_2[i].y = -1;   // draw_point() ignores -1
    trail_3[i].x = trail_3[i].y = -1;
  }
}

// -----------------------------------------------------------------------
// erase points that are 3 frames old. shift frame histories

void do_trails(void)
{
  int i = MAX_POINTS;

  while (i--)
  {
    erase_point(trail_3[i].x, trail_3[i].y);

    trail_3[i].x = trail_2[i].x;        // shift points within history
    trail_2[i].x = trail_1[i].x;        //  buffers
    trail_1[i].x = -1;

    trail_3[i].y = trail_2[i].y;
    trail_2[i].y = trail_1[i].y;
    trail_1[i].y = -1;
  }
}

// -----------------------------------------------------------------------
// pre calculate sin and cosine values for x y and z angles of rotation

void sincos(void)
{
  sin_x = sin_tab[x_angle];
  sin_y = sin_tab[y_angle];
  sin_z = sin_tab[z_angle];

  cos_x = sin_tab[adjust(x_angle)];
  cos_y = sin_tab[adjust(y_angle)];
  cos_z = sin_tab[adjust(z_angle)];
}

// -----------------------------------------------------------------------
// roatate object about x y and z axis (in object space)

void rotate(int *px, int *py, int *pz)
{
  int tx, ty, tz;                       // temp store

  if (x_angle)                          // rotate point about x axis...
  {
    ty = (*py * cos_x) - (*pz * sin_x);
    tz = (*py * sin_x) + (*pz * cos_x);

    *py = (ty >> 14);                   // sin table is scaled up so we
    *pz = (tz >> 14);                   // must re scale all results down
  }

  if (y_angle)                          // rotate point about y axis
  {
    tx = (*px * cos_y) - (*pz * sin_y);
    tz = (*px * sin_y) + (*pz * cos_y);

    *px = (tx >> 14);
    *pz = (tz >> 14);
  }

  if (z_angle)                          // rotate point about z axis
  {
    tx = (*px * cos_z) - (*py * sin_z);
    ty = (*px * sin_z) + (*py * cos_z);

    *px = (tx >> 14);
    *py = (ty >> 14);
  }
}

// -----------------------------------------------------------------------
// project point in 3d space onto plane in 2d space

void project(int px, int py, int pz, int *x, int *y)
{
  int tx, ty;                   // temp store...

  *x = *y = -1;                 // assume point is clipped

  if ((z_off + pz - 5) < 0)
  {
    return;
  }

  ty = ((y_off * py) / (z_off + pz)) + 30;

  if ((ty > 5) && (ty < 59))
  {
    tx = ((x_off * px) / (z_off + pz)) + 30;

    if ((tx > 5) && (tx < 59))
    {
      *x = tx;
      *y = ty;
    }
  }
}

// -----------------------------------------------------------------------
// draw one frame of object...

void do_frame(void)
{
  int px, py, pz;               // 3d coordinates to rotate
  int x, y, c;                  // 2d coordiantes of point and colour
  int i = num_points;           // loop counter
  int j = MASK_SIZE;
  char pel_buff[MASK_SIZE];     // frame buffer ( see draw_point() )
  p_buff = &pel_buff[0];

  while (j--)                   // erase pel buffer
  {
    pel_buff[j] = 32;
  }

  do_trails();                  // clear pels that are 3 frames old
  sincos();                     // calculate all sin/cos values

  while(i--)
  {
    px = object[i].x;           // collect point from object
    py = object[i].y;
    pz = object[i].z;

    rotate(&px, &py, &pz);      // rotate this point about x/y and z axis
    project(px, py, pz, &x, &y); // projection = convert xyz to xy

    trail_1[i].x = x;           // store frame history for all pels
    trail_1[i].y = y;           //  of this frame

    c = (((z_off - 90) / 14) & 15) < 1;
    c -= ((-pz / 5) - 10);

    draw_point(x, y, c);
  }
}

// -----------------------------------------------------------------------
// adjust rotational speeds / distance between min and max for each

void modify(modifier *mod)
{
  mod->counter--;

  if (!mod->counter)
  {
    mod->counter = mod->reset;

    *mod->ptr += mod->delta;

    if (*mod->ptr >= mod->upper || *mod->ptr <= mod->lower)
    {
      mod->delta = -(mod->delta);
    }
  }
}

// -----------------------------------------------------------------------
// do the above on each of the 4 modifiers

void do_deltas(void)
{
  modify(&w1);                  // modify x rotational speed
  modify(&w2);                  // modify y rotational speed
  modify(&w3);                  // modify z rotational speed
  modify(&w4);                  // zoom object in and out
//  modify(&w5);		  // not used because they make objects
//  modify(&w6);		  // stay in the distance
}

// -----------------------------------------------------------------------
// adjust x y and z angles of ritation for next frame

void change_angles(void)
{
  x_angle += delta_x;
  y_angle += delta_y;
  z_angle += delta_z;

  x_angle &= 0x1ff;
  y_angle &= 0x1ff;
  z_angle &= 0x1ff;
}

// -----------------------------------------------------------------------

void do_demo(void)
{
  while (1)                     // only way out is to die
  {
    while(--itters)             // countdown to next shape change...
    {                           //   shape change FORCED on entry
      change_angles();          // adjust angles of rotation
      do_frame();               // draw object
      do_deltas();              // modify rotation speeds etc

      RedrawWindow();           // lets see what weve drawn...
      do_pending();             // quit / expose
      usleep(50000);            // erm... coders never sleep :)
    }

    itters = 2500;              // should randomise this

    object = obj_list[obj_number];
    num_points = point_counts[obj_number++];

    if (obj_number == NUM_OBJECTS)
    {
      obj_number = 0;
    }
  }
}

// -----------------------------------------------------------------------
// display version info and credits

void credits(void)
{
  printf("\n");
  printf("WM-Dots DEMO? version %s\n",Version);
  printf("    by Mark I Manning IV\n\n");
  printf("greets to follow soon....\n\n");
}

// sgore for helping me get star office :)

// -----------------------------------------------------------------------
// scan command line args

void scan_args(int argc,char **argv)
{
  int i;

  for (i = 0; i < argc; i++)
  {
    char *arg = argv[i];

    if (*arg == '-')
    {
      switch (arg[1])
      {
        case 'v':
          credits();
          exit(0);
        break;

        default:
          ;
        break;
      }
    }
  }
}

// -----------------------------------------------------------------------
// main at BOTTOM of source so we have no EVIL forward refs!!

int main(int argc, char *argv[])
{
  char mask[MASK_SIZE];

  ProgName = argv[0];
  if (strlen(ProgName) >= 5)
  {
     ProgName += (strlen(ProgName) - 5);
  }

  scan_args(argc, argv);

  createXBMfromXPM(mask, master_xpm, MASK_WIDTH, MASK_HEIGHT);
  openXwindow(argc, argv, master_xpm, mask, MASK_WIDTH, MASK_HEIGHT);

  clear_histories();            // clear frame history buffers
  do_demo();                    // main = short and sweet :)

  return(0);                    // never gets executed but lets get rid
}                               //  of that stupid warning :)

// =======================================================================
