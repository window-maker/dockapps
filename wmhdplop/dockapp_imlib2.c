#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "dockapp_imlib2.h"
#include <X11/extensions/shape.h>
#include <X11/Xutil.h>
#ifdef GKRELLM
#include <gdk/gdkx.h>
#endif

/* require ISO C99 compatibility */
#define DOCKIMLIB2_ERR(...) { fprintf(stderr, "DockImlib2 " DOCKIMLIB2_VERSION " error: "); \
                                      fprintf(stderr, __VA_ARGS__); exit(1); }
#define DOCKIMLIB2_WARN(...) { fprintf(stderr, "DockImlib2 " DOCKIMLIB2_VERSION " warning: "); \
                                      fprintf(stderr, __VA_ARGS__); exit(1); }

static void dockimlib2_set_rect_shape(DockImlib2 *dock, int x, int y, int w, int h) {
  Pixmap mask = XCreatePixmap(dock->display, dock->win, dock->win_width, dock->win_height, 1); assert(mask);
  GC gc = XCreateGC(dock->display, mask, 0, NULL);
  XSetForeground(dock->display, gc, BlackPixel(dock->display, dock->screennum));
  XFillRectangle(dock->display, mask, gc, 0, 0, dock->win_width, dock->win_height);
  XSetForeground(dock->display, gc, WhitePixel(dock->display, dock->screennum));
  XFillRectangle(dock->display, mask, gc, x,y,w,h);
  XFreeGC(dock->display,gc);
  /* setup shaped window */
  XShapeCombineMask(dock->display, dock->normalwin, ShapeBounding, 
                    0, 0, mask, ShapeSet);
  if (dock->iconwin)
    XShapeCombineMask(dock->display, dock->iconwin, ShapeBounding,
                      0, 0, mask, ShapeSet);
  XFreePixmap(dock->display, mask);
}

/* some general windowmanager related functions ...*/
void set_window_title(Display *display, Window win, char *window_title, char *icon_title) {
  int rc;
  XTextProperty window_title_property;
  /* window name */
  rc = XStringListToTextProperty(&window_title,1, &window_title_property); assert(rc);
  XSetWMName(display, win, &window_title_property);
  XFree(window_title_property.value);
  
  /* icon window name */
  rc = XStringListToTextProperty(&icon_title,1, &window_title_property); assert(rc);
  XSetWMIconName(display, win, &window_title_property);
  XFree(window_title_property.value);
}

void
set_window_class_hint(Display *display, Window win, char *res_class, char *res_name) {
  XClassHint *class_hint;
  class_hint = XAllocClassHint();
  class_hint->res_name = res_name;
  class_hint->res_class = res_class;
  XSetClassHint(display, win, class_hint);
  XFree(class_hint);
}

#ifndef GKRELLM
static void dockimlib2_xinit(DockImlib2 *dock, DockImlib2Prefs *prefs) {
  XSizeHints *xsh;
  int i;
  char *sdisplay = getenv("DISPLAY");
  char *pgeom = NULL;
  int undocked = 0;
  char sdockgeom[40];

  assert(prefs->argv); // this should be always set ..
  
  if (prefs->flags & DOCKPREF_DISPLAY) sdisplay = prefs->display;
  if (prefs->flags & DOCKPREF_GEOMETRY) { pgeom = prefs->geometry; undocked = 1; }
  
  dock->display = XOpenDisplay(sdisplay);
  if(!dock->display) DOCKIMLIB2_ERR("Couldn't connect to display %s\n", sdisplay);
  dock->screennum = DefaultScreen(dock->display);
  dock->visual = DefaultVisual(dock->display, dock->screennum);
  dock->depth = DefaultDepth(dock->display, dock->screennum);
  dock->colormap = DefaultColormap(dock->display, dock->screennum);
  dock->rootwin = RootWindow(dock->display, dock->screennum);

  dock->atom_WM_DELETE_WINDOW = XInternAtom(dock->display, "WM_DELETE_WINDOW", False);
  dock->atom_WM_PROTOCOLS = XInternAtom(dock->display, "WM_PROTOCOLS", False);

  /* set size hints */
  xsh = XAllocSizeHints(); assert(xsh);
  xsh->flags = 0;
  xsh->width = dock->w;
  xsh->height = dock->h;
  xsh->x = xsh->y = 0;

  snprintf(sdockgeom, sizeof sdockgeom, "%dx%d+0+0", prefs->dockapp_size, prefs->dockapp_size);

  xsh->flags = XWMGeometry(dock->display, dock->screennum, pgeom, sdockgeom, 0,
                           xsh, &xsh->x, &xsh->y, &xsh->width, &xsh->height, &i);
  if (undocked) {
    dock->win_width  = dock->w = xsh->width; 
    dock->win_height = dock->h = xsh->height; 
    dock->x0 = dock->y0 = 0;
  }
  xsh->base_width = xsh->width;
  xsh->base_height = xsh->height;
  xsh->flags |= USSize | PMinSize | PMaxSize | PSize;
  xsh->min_height = 24; xsh->min_height = 24;
  xsh->max_width = 500;
  xsh->max_height = 500;

  /* create the application window */
  dock->normalwin = XCreateSimpleWindow(dock->display, dock->rootwin,
                                        xsh->x, xsh->y, xsh->width, xsh->height, 0,
                                        BlackPixel(dock->display, dock->screennum),
                                        WhitePixel(dock->display, dock->screennum));
  
  if(!dock->normalwin) DOCKIMLIB2_ERR("Couldn't create window\n");
  if (!undocked) {
    /* create icon window */
    dock->iconwin = XCreateSimpleWindow(dock->display, dock->rootwin,
                                        xsh->x, xsh->y, xsh->width, xsh->height, 0,
                                        BlackPixel(dock->display, dock->screennum),
                                        WhitePixel(dock->display, dock->screennum));
    if(!dock->iconwin) DOCKIMLIB2_ERR("Couldn't create icon window\n");
    dock->win = dock->iconwin;
  } else {
    dock->iconwin = None;
    dock->win = dock->normalwin;
  }
  dock->iconwin_mapped = dock->normalwin_mapped = 1; /* by default */

  /* start with an empty window in order to get the background pixmap */
  dockimlib2_set_rect_shape(dock,32,32,1,0);
  
  /* set window manager hints */
  if (!undocked) {
    XWMHints *xwmh;
    xwmh = XAllocWMHints();
    xwmh->flags = WindowGroupHint | IconWindowHint | StateHint;
    xwmh->icon_window = dock->iconwin;
    xwmh->window_group = dock->normalwin;
    xwmh->initial_state = WithdrawnState;
    XSetWMHints(dock->display, dock->normalwin, xwmh);
    XFree(xwmh); xwmh = NULL;
  }
  set_window_class_hint(dock->display, dock->normalwin, prefs->argv[0], prefs->argv[0]);

  /* set size hints */
  XSetWMNormalHints(dock->display, dock->normalwin, xsh);

  set_window_title(dock->display, dock->normalwin, "wmhdplop", "wmhdplop");

  /* select events to catch */
  {
    long evmask = ExposureMask |  ButtonPressMask | ButtonReleaseMask | VisibilityChangeMask |
      PointerMotionMask | EnterWindowMask | LeaveWindowMask | StructureNotifyMask;
    XSelectInput(dock->display, dock->normalwin, evmask);
    if (dock->iconwin) 
      XSelectInput(dock->display, dock->iconwin, evmask);
  }
  XSetWMProtocols(dock->display, dock->normalwin, &dock->atom_WM_DELETE_WINDOW, 1);

  /* set the command line for restarting */
  XSetCommand(dock->display, dock->normalwin, prefs->argv, prefs->argc);

  /* map the main window */
  XMapWindow(dock->display, dock->normalwin);

  XFree(xsh); xsh = NULL;
}

#else /* GKRELLM */
void dockimlib2_gkrellm_xinit(DockImlib2 *dock, GdkDrawable *gkdrawable) {

  dock->display = GDK_WINDOW_XDISPLAY(gkdrawable);
  dock->visual = GDK_VISUAL_XVISUAL(gdk_drawable_get_visual(gkdrawable));
  dock->depth = gdk_drawable_get_depth(gkdrawable);
  dock->colormap = GDK_COLORMAP_XCOLORMAP(gdk_drawable_get_colormap(gkdrawable));
  dock->screennum = DefaultScreen(dock->display);
  dock->rootwin = RootWindow(dock->display, dock->screennum);

  dock->normalwin = XCreateSimpleWindow(dock->display, GDK_PIXMAP_XID(gkdrawable),
					0, 0, dock->w, dock->h, 0,
					BlackPixel(dock->display, dock->screennum),
					WhitePixel(dock->display, dock->screennum));
  //dock->normalwin = GDK_PIXMAP_XID(gkdrawable); // CA CLIGNOTE !!!
  dock->iconwin = None; dock->iconwin_mapped = 0;
  dock->win = dock->normalwin; dock->normalwin_mapped = 1;

  /* start with an empty window in order to get the background pixmap */
  dockimlib2_set_rect_shape(dock,32,32,1,0);
  
  /* map the main window */
  XMapWindow(dock->display, dock->normalwin);
}
#endif /* GKRELLM */

static void add_fontpath(const char *path, int recurse) {
  struct stat st;
    
  if (stat(path,&st) != 0 ||
      !S_ISDIR(st.st_mode)) return;
  if (recurse > 3) return; /* prevent scanning of whole hd/infinite recursions in case of a bad symlink */
  printf("add font path: '%s'\n", path);
  imlib_add_path_to_font_path(path);
  if (recurse) {
    DIR *d = opendir(path);
    struct dirent *de;
    if (!d) return;
    while ((de = readdir(d))) {
      char s[1024];
      if (strcmp(de->d_name,".") == 0 ||
          strcmp(de->d_name,"..") == 0) continue;
      snprintf(s,sizeof s,"%s/%s",path, de->d_name);
      add_fontpath(s,recurse+1);
    }
    closedir(d);
  }
}

void dockimlib2_reset_imlib(DockImlib2 *dock) {
  imlib_free_image();
  dock->img = imlib_create_image(dock->w, dock->h);
  imlib_context_set_image(dock->img);
  imlib_context_set_drawable(dock->win);
  dockimlib2_set_rect_shape(dock, dock->x0, dock->y0, dock->w, dock->h);
}

/* setup some default values for imlib2 */
static void dockimlib2_setup_imlib(DockImlib2 *dock) {
  char fp[512];
   /* set the maximum number of colors to allocate for 8bpp and less to 32 */
  imlib_set_color_usage(32);
  /* dither for depths < 24bpp */
  imlib_context_set_dither(1);
  /* set the display , visual, colormap and drawable we are using */
  imlib_context_set_display(dock->display);
  imlib_context_set_visual(dock->visual);
  imlib_context_set_colormap(dock->colormap);
  imlib_context_set_drawable(dock->win);
  dock->img = imlib_create_image(dock->w, dock->h);
  imlib_context_set_image(dock->img);

  /* some default font paths */
  snprintf(fp, 512, "%s/.fonts", getenv("HOME"));
  add_fontpath(fp,1);
  /*add_fontpath("/usr/share/fonts/truetype",1);
  add_fontpath("/usr/share/fonts/ttf",1);*/
  add_fontpath("/usr/share/fonts",1);
  add_fontpath("/usr/X11R6/lib/X11/fonts/truetype",1);
  add_fontpath("/usr/X11R6/lib/X11/fonts/TrueType",1);
  add_fontpath("/usr/X11R6/lib/X11/fonts/ttf",1);
  add_fontpath("/usr/X11R6/lib/X11/fonts/TTF",1);
  /*imlib_add_path_to_font_path(fp);
  imlib_add_path_to_font_path("/usr/share/fonts/truetype");
  imlib_add_path_to_font_path("/usr/share/fonts/truetype/freefont");
  imlib_add_path_to_font_path("/usr/share/fonts/truetype/ttf-bitstream-vera");
  imlib_add_path_to_font_path("/usr/share/fonts/ttf/vera");*/ /* vera on mdk */
  imlib_context_set_TTF_encoding(IMLIB_TTF_ENCODING_ISO_8859_1);
  //imlib_set_cache_size(0);imlib_set_font_cache_size(0);
}

/* wait for the dockapp to be swallowed and grab the background pixmap */
static void dockimlib2_xstartup(DockImlib2 *dock) {
  dock->bg = NULL;
  dockimlib2_set_rect_shape(dock, dock->x0, dock->y0, dock->w, dock->h);
#if 0
  Window parent = 0;
  int exposed = 0, mapped = 0;
  printf("xstartup..\n");
  do {
    XEvent event;
    XNextEvent(dock->display, &event);
    switch (event.type) {
    case ReparentNotify: {
      XReparentEvent ev = event.xreparent;
      if (ev.window == dock->win) {
        parent = ev.parent;
      }
    } break;
    case MapNotify: {
      XMappingEvent ev = event.xmapping;
      printf("MapNotify: ev.win = %lx\n", ev.window);
      if (ev.window == dock->win) mapped = 1;
    } break;
    case Expose: {
      XExposeEvent ev = event.xexpose;
      printf("Expose: ev.win = %lx\n", ev.window);
      if (ev.window == dock->win) exposed = 1;
    } break;
    }
  } while (parent == 0 && !exposed && !mapped);
  if (parent == dock->rootwin) {
    printf("... oups, parent window is rootwin ... are you really running windowmaker?\n");
    dock->bg = imlib_create_image(dock->w, dock->h);
  } else {
    imlib_context_set_drawable(parent);
    dock->bg = imlib_create_image_from_drawable(0, dock->x0, dock->y0, dock->w, dock->h, 1);
    imlib_context_set_drawable(dock->win);
    dockimlib2_set_rect_shape(dock, dock->x0, dock->y0, dock->w, dock->h);
  }
  printf("xstartup : success\n");
#endif
}

#ifndef GKRELLM
DockImlib2* dockimlib2_setup(int x0, int y0, int w, int h, DockImlib2Prefs *prefs) {
#else
DockImlib2* dockimlib2_gkrellm_setup(int x0, int y0, int w, int h, DockImlib2Prefs *prefs, GdkDrawable *gkdrawable) {
#endif
  DockImlib2 *dock = calloc(1,sizeof(DockImlib2)); assert(dock);
  //gdk_drawable_get_size(gkdrawable, &dock->win_width, &dock->win_height);
  //printf("x0=%d, y0=%d, width = %d,%d, height = %d,%d\n", x0,y0,w,h,dock->win_width, dock->win_height);
  dock->win_width = w+x0; dock->win_height = h+y0;//DOCK_WIDTH;
  dock->x0 = x0; dock->y0 = y0;
  dock->w = w; dock->h = h;
#ifndef GKRELLM
  dockimlib2_xinit(dock, prefs);
#else
  (void) prefs;
  dockimlib2_gkrellm_xinit(dock, gkdrawable);
#endif
  dockimlib2_setup_imlib(dock);
  dockimlib2_xstartup(dock);
  return dock;
}

static char *last_font_name = 0;

const char* dockimlib2_last_loaded_font() { return last_font_name; }
 
Imlib_Font *imlib_load_font_nocase(const char *name) {
  Imlib_Font *f;
  int i;
  if (last_font_name) free(last_font_name);
  last_font_name = strdup(name); 
  if ((f = imlib_load_font(last_font_name))) return f;
  for (i=0; last_font_name[i]; ++i) last_font_name[i] = tolower(last_font_name[i]);
  if ((f = imlib_load_font(last_font_name))) return f;
  for (i=0; last_font_name[i]; ++i) last_font_name[i] = toupper(last_font_name[i]);
  f = imlib_load_font(last_font_name);
  return f; 
}

Imlib_Font *load_font(char *prefname, char **flist_) {
  Imlib_Font *f = NULL;
  char **flist = flist_;
  if (prefname) {
    f = imlib_load_font_nocase(prefname);
    if (!f) {
      int i,np; char **s;
      fprintf(stderr, "warning: could not find font '%s' in the font path:\n",prefname);
      s = imlib_list_font_path(&np);
      for (i=0; i < np; ++i) fprintf(stderr, "  %s\n", s[i]);
    } else {
      printf("loaded font %s\n", prefname);
    }
  }
  if (!f) {
    for (; *flist; ++flist) {
      if ((f = imlib_load_font_nocase(*flist))) {
        printf("loaded font %s\n", *flist); break;
      }
    }
    if (!f) {
      fprintf(stderr, "could not load a default ttf font .. I tried ");
      flist = flist_;
      for (; *flist; ++flist)
        fprintf(stderr, "'%s'%s", *flist, (flist[1]?", ":""));
      fprintf(stderr, "\nUse the --font* options to change the fontpath/fontnames\n");
    }
  }
  return f;
}

/* 
   merges dock->bg and dock->img, and renders the result on the window 
   this function does not alter the imlib context
*/
void dockimlib2_render(DockImlib2 *dock) {
  Pixmap olddrawable = imlib_context_get_drawable();
  Imlib_Image oldimage = imlib_context_get_image();
  //imlib_context_set_drawable(dock->win);
  imlib_context_set_image(dock->img);
  if (imlib_image_has_alpha()) {
    imlib_context_set_image(dock->bg);
    Imlib_Image bg = imlib_clone_image();
    imlib_context_set_image(bg);
    imlib_blend_image_onto_image(dock->img, 0, 0, 0, dock->w, dock->h, 0, 0, dock->w, dock->h);

    if (dock->normalwin) {
      imlib_context_set_drawable(dock->normalwin);
      imlib_render_image_on_drawable(dock->x0, dock->y0);
    }
    if (dock->iconwin) {
      imlib_context_set_drawable(dock->iconwin);
      imlib_render_image_on_drawable(dock->x0, dock->y0);
    }    
    /* XSetWindowBackgroundPixmap(dock->display, dock->GKwin, dock->win);
       XClearWindow(dock->display, dock->GKWin); */
    imlib_free_image();
  } else {
    if (dock->normalwin_mapped && dock->normalwin) {
      imlib_context_set_drawable(dock->normalwin);
      imlib_render_image_on_drawable(dock->x0, dock->y0); /* imlib_render_image_on_drawable generates many pages faults !? */
    }
    if (dock->iconwin_mapped && dock->iconwin) {
      imlib_context_set_drawable(dock->iconwin);
      imlib_render_image_on_drawable(dock->x0, dock->y0);
    }
  }
  imlib_context_set_image(oldimage);
  imlib_context_set_drawable(olddrawable);
}
