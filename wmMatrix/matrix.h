#include <stdlib.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>


typedef struct {
  int glyph;
  int changed;
  int glow;
} m_cell;

typedef struct {
  int remaining;
  int throttle;
  int y;
} m_feeder;

typedef struct {
  Display *dpy;
  Window window;
  XWindowAttributes xgwa;
  GC draw_gc, erase_gc;
  int grid_width, grid_height;
  int char_width, char_height;
  m_cell *cells;
  m_feeder *feeders;
  int insert_top_p, insert_bottom_p;
  int density;

  Pixmap images;
  int image_width, image_height;
  int nglyphs;

} m_state;

