#ifndef DOCKAPP_IMLIB2_H
#define DOCKAPP_IMLIB2_H

#include <X11/Xlib.h>
#include <Imlib2.h>
#ifdef GKRELLM
#include <gdk/gdk.h>
#endif
/*
#ifndef GKRELLM
#define DOCK_WIDTH 64
#else
#define DOCK_WIDTH 60
#endif
#define DOCK_HEIGHT DOCK_WIDTH
*/
typedef struct {
  Display *display;
  Window  normalwin, iconwin, rootwin;
  int iconwin_mapped, normalwin_mapped;
  Window  win; /* either normalwin or iconwin */
  Visual  *visual;
  Colormap colormap;
  int depth, screennum;
  Imlib_Image bg, img;  /* background picture, and "work" picture */
  unsigned x0,y0,w,h;   /* pos & dimensions of the "work" area */
  unsigned win_width, win_height; /* size of dockapps (DOCK_WIDTH x DOCK_HEIGHT) */
  Atom atom_WM_DELETE_WINDOW, atom_WM_PROTOCOLS;
} DockImlib2;


#define DOCKPREF_DISPLAY 1
#define DOCKPREF_GEOMETRY 2
typedef struct {
  char **argv; int argc; /* this should be always set */
  int flags; /* combination of DOCKPREF_xxx */
  char *display;
  char *geometry;
  int  dockapp_size;
} DockImlib2Prefs;

#define DOCKIMLIB2_VERSION "0.9.0"

#ifndef GKRELLM
DockImlib2* dockimlib2_setup(int x0, int y0, int w, int h, DockImlib2Prefs *p);
#else
DockImlib2* dockimlib2_gkrellm_setup(int x0, int y0, int w, int h, DockImlib2Prefs *p, GdkDrawable *gkdrawable);
void dockimlib2_gkrellm_xinit(DockImlib2 *dock, GdkDrawable *gkdrawable);
#endif
Imlib_Font *load_font(char *prefname, char **flist);

void dockimlib2_reset_imlib(DockImlib2 *dock);

void dockimlib2_render(DockImlib2 *dock);
const char *dockimlib2_last_loaded_font();
#endif
