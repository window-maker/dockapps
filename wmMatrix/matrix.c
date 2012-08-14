/* xscreensaver, Copyright (c) 1999 Jamie Zawinski <jwz@jwz.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * Matrix -- simulate the text scrolls from the movie "The Matrix".
 *
 * The movie people distribute their own Windows/Mac screensaver that does
 * a similar thing, so I wrote one for Unix.  However, that version (the
 * Windows/Mac version at http://www.whatisthematrix.com/) doesn't match my
 * memory of what the screens in the movie looked like, so my `xmatrix'
 * does things differently.
 */

#include <stdio.h>
#include "images/small.xpm"
#include "images/medium.xpm"
#include "images/large.xpm"
#include "images/matrix.xbm"
/*
#define CHAR_HEIGHT 4
*/

#include "matrix.h"
/*
#include "resources.h"
*/

extern GC              NormalGC;
extern GC              EraseGC;
extern Pixel           back_pix, fore_pix;
extern int	       PixmapSize;
int		       CHAR_HEIGHT;





static void load_images (m_state *state) {

  if (state->xgwa.depth > 1) {

      XpmAttributes xpmattrs;
      int result;

      xpmattrs.valuemask  = 0;
      xpmattrs.valuemask |= XpmCloseness;
      xpmattrs.closeness  = 40000;
      xpmattrs.valuemask |= XpmVisual;
      xpmattrs.visual     = state->xgwa.visual;
      xpmattrs.valuemask |= XpmDepth;
      xpmattrs.depth      = state->xgwa.depth;
      xpmattrs.valuemask |= XpmColormap;
      xpmattrs.colormap   = state->xgwa.colormap;

      if (PixmapSize == 1){

	  CHAR_HEIGHT = 4;
          result = XpmCreatePixmapFromData (state->dpy, state->window, small,
                                        &state->images, 0 /* mask */,
                                        &xpmattrs);
      } else if (PixmapSize == 2){

	  CHAR_HEIGHT = 6;
          result = XpmCreatePixmapFromData (state->dpy, state->window, medium,
                                        &state->images, 0 /* mask */,
                                        &xpmattrs);
      } else {

	  CHAR_HEIGHT = 8;
          result = XpmCreatePixmapFromData (state->dpy, state->window, large,
                                        &state->images, 0 /* mask */,
                                        &xpmattrs);
      }

      if (!state->images || (result != XpmSuccess && result != XpmColorError))
        state->images = 0;

      state->image_width = xpmattrs.width;
      state->image_height = xpmattrs.height;
      state->nglyphs = state->image_height / CHAR_HEIGHT;

    } else {

      state->image_width = matrix_width;
      state->image_height = matrix_height;
      state->nglyphs = state->image_height / CHAR_HEIGHT;

      state->images = XCreatePixmapFromBitmapData (state->dpy, state->window,
                                     (char *) matrix_bits,
                                     state->image_width, state->image_height,
                                     back_pix, fore_pix, state->xgwa.depth);
    }

}


m_state *init_matrix( Display *dpy, Window window ) {

  m_state *state = (m_state *) calloc (sizeof(*state), 1);


  state->dpy = dpy;
  state->window = window;

  XGetWindowAttributes (dpy, window, &state->xgwa);
  load_images (state);

  state->draw_gc  = NormalGC;
  state->erase_gc = EraseGC;

  state->char_width = state->image_width / 2;
  state->char_height = CHAR_HEIGHT;

  state->grid_width  = state->xgwa.width  / state->char_width;
  state->grid_height = state->xgwa.height / state->char_height;
  state->grid_width++;
  state->grid_height++;

  state->cells = (m_cell *)calloc (sizeof(m_cell), state->grid_width * state->grid_height);
  state->feeders = (m_feeder *)calloc (sizeof(m_feeder), state->grid_width);

  state->density = 40;

  state->insert_top_p = False;
  state->insert_bottom_p = True;


  return state;

}


static void insert_glyph (m_state *state, int glyph, int x, int y) {

  Bool bottom_feeder_p = (y >= 0);
  m_cell *from, *to;

  if (y >= state->grid_height) return;

  if (bottom_feeder_p) {

      to = &state->cells[state->grid_width * y + x];

  } else {

      for (y = state->grid_height-1; y > 0; y--) {

          from = &state->cells[state->grid_width * (y-1) + x];
          to   = &state->cells[state->grid_width * y     + x];
          *to = *from;
          to->changed = True;

    }
    to = &state->cells[x];

  }

  to->glyph = glyph;
  to->changed = True;

  if (!to->glyph) ;
  else if (bottom_feeder_p) to->glow = 1 + (random() % 2);
  else to->glow = 0;

}


static void feed_matrix (m_state *state) {

    int x;


    /* 
     *  Update according to current feeders. 
     */
    for (x = 0; x < state->grid_width; x++) {

        m_feeder *f = &state->feeders[x];

        if (f->throttle) {		/* this is a delay tick, synced to frame. */

            f->throttle--;

        } else if (f->remaining > 0) {	/* how many items are in the pipe */

	    int g = (random() % state->nglyphs) + 1;
	    insert_glyph (state, g, x, f->y);
	    f->remaining--;
	    if (f->y >= 0)  f->y++; /* bottom_feeder_p */

        } else {	/* if pipe is empty, insert spaces */

            insert_glyph (state, 0, x, f->y);
            if (f->y >= 0) f->y++;   /* bottom_feeder_p */

        }

	if ((random() % 10) == 0) {  /* randomly change throttle speed */

	    f->throttle = ((random() % 5) + (random() % 5));

	}

    }

}

static int densitizer (m_state *state) {

  /* Horrid kludge that converts percentages (density of screen coverage)
     to the parameter that actually controls this.  I got this mapping
     empirically, on a 1024x768 screen.  Sue me. */
  if      (state->density < 10) return 85;
  else if (state->density < 15) return 60;
  else if (state->density < 20) return 45;
  else if (state->density < 25) return 25;
  else if (state->density < 30) return 20;
  else if (state->density < 35) return 15;
  else if (state->density < 45) return 10;
  else if (state->density < 50) return 8;
  else if (state->density < 55) return 7;
  else if (state->density < 65) return 5;
  else if (state->density < 80) return 3;
  else if (state->density < 90) return 2;
  else return 1;

}


static void hack_matrix (m_state *state) {

  int x;

  /* Glow some characters. */
  if (!state->insert_bottom_p) {

      int i = random() % (state->grid_width / 2);
      while (--i > 0) {

          int x = random() % state->grid_width;
          int y = random() % state->grid_height;
          m_cell *cell = &state->cells[state->grid_width * y + x];
          if (cell->glyph && cell->glow == 0) {

              cell->glow = random() % 10;
              cell->changed = True;
	  }

      }
  }

  /* Change some of the feeders. */
  for (x = 0; x < state->grid_width; x++) {

      m_feeder *f = &state->feeders[x];
      Bool bottom_feeder_p;

      if (f->remaining > 0)	/* never change if pipe isn't empty */
        continue;

      if ((random() % densitizer(state)) != 0) /* then change N% of the time */
        continue;

      f->remaining = 3 + (random() % state->grid_height);
      f->throttle = ((random() % 5) + (random() % 5));

      if ((random() % 4) != 0)
        f->remaining = 0;

      if (state->insert_top_p && state->insert_bottom_p)
        bottom_feeder_p = (random() & 1);
      else
        bottom_feeder_p = state->insert_bottom_p;

      if (bottom_feeder_p)
        f->y = random() % (state->grid_height / 2);
      else
        f->y = -1;
    }
}


void draw_matrix (m_state *state, int d) {

    int x, y;
    int count = 0;

    state->density = d;
    feed_matrix( state );
    hack_matrix( state );

    for (y = 0; y < state->grid_height; y++) {
	for (x = 0; x < state->grid_width; x++) {

	    m_cell *cell = &state->cells[state->grid_width * y + x];

	    if ( cell->glyph ) count++;

	    if ( !cell->changed ) continue; 

	    if ( cell->glyph == 0 ) {

		XFillRectangle( state->dpy, state->window, state->erase_gc,
				x * state->char_width, y * state->char_height,
				state->char_width, state->char_height );
	    } else {

		XCopyArea( state->dpy, state->images, state->window, state->draw_gc,
				(cell->glow ? state->char_width : 0), (cell->glyph - 1) * state->char_height,
				state->char_width, state->char_height, x * state->char_width, y * state->char_height );
						
	    }

	    cell->changed = False;

	    if (cell->glow > 0) {

		cell->glow--;
		cell->changed = True;

	    }

	}
    }


#if 0
  {
    static int i = 0;
    static int ndens = 0;
    static int tdens = 0;
    i++;
    if (i > 50)
      {
        int dens = (100.0 *
                    (((double)count) /
                     ((double) (state->grid_width * state->grid_height))));
        tdens += dens;
        ndens++;
        printf ("density: %d%% (%d%%)\n", dens, (tdens / ndens));
        i = 0;
      }
  }
#endif

}

