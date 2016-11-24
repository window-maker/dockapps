#ifndef GLOBAL_H
#define GLOBAL_H

//#include "config.h"

#ifdef __GNUC__
# define UNUSED __attribute((unused))
#else
# define UNUSED
#endif

#ifdef GLOBALS_HERE
# define DECL_GLOB(x) x;
# define DECL_GLOB_INIT(x,y) x = y
#else
# define DECL_GLOB(x) extern x
# define DECL_GLOB_INIT(x,y) extern x
#endif


#ifndef NO_BLAHBLAH
# define BLAHBLAH(n,x) if (Prefs.verbosity >= n) { x; fflush(stdout); }
#else
# define BLAHBLAH(n,x)
#endif

#ifndef NO_BITFIELDS
#  define BITFIELD(n) :n
#else
#  define BITFIELD(n) 
#endif 

#include "dockapp_imlib2.h"

typedef enum { AL_NONE=0, AL_LEFT=1, AL_HCENTER=2, AL_RIGHT=4, AL_TOP=8, AL_VCENTER=16,AL_BOTTOM=32} align_enum;

typedef struct {
  int verbosity;
  int enable_hddtemp;      /* enable querying and display of hd temp */
  unsigned hddtemp_port;
  int enable_power_status; /* enable querying of power status */
  int disable_swap_matrix; /* disable the animation reflecting swap activity */
  int disable_io_matrix;   /* disable background animation reflecting disk activity */
  int disable_hd_leds;     /* hide the small led indicating disk activity (if !disable_hd_leds) */
  int disable_hd_list;     /* hide the list of monitored drives */
  char *bigfontname, *smallfontname; /* ttf font name  + "/size" in pixels */
  float popup_throughput_threshold;
  unsigned iomatrix_colormap;
  unsigned debug_swapio, debug_disk_wr, debug_disk_rd;
  unsigned popup_throughput_pos, hdlist_pos;
  int temperatures_unit; /* 'C' or 'F' */
  DockImlib2Prefs xprefs;
} structPrefs;

DECL_GLOB(structPrefs Prefs);

#define ASSIGN_STRING(a,v) { if (a) { free(a); a = NULL; } a = strdup(v); assert(a); }

#endif

