#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include <errno.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>

#include "config.h"

#include "ascd.h"
#include <workman/workman.h>

#ifdef WMK
#include <WINGs.h>
#endif

extern open();
extern time();

/* added this for this lib: */
extern unsigned int info_modified;

extern int wanted_track;
extern unsigned int do_autorepeat;

extern int anomalie;

/* xfascd vars */

extern unsigned int wanna_play;

#ifdef XFASCD
extern unsigned int update_xpm;
#endif
extern unsigned int blind_mode;
extern unsigned int loop_mode, intro_mode;
extern unsigned int datatrack, cue_time;
extern unsigned int loop_start_track, loop_end_track;
extern unsigned int loop_1;
extern unsigned int loop_2;
extern unsigned int direct_access, direct_track;
extern unsigned int ignore_avoid;

extern int lasttime;

extern Display *Disp;
extern Window Root;
extern Window Iconwin;
extern Window Win;
extern char *Geometry;
extern char device[];
extern char xv[];
extern int withdrawn;
extern GC WinGC;
extern int CarrierOn;

extern char led_text[];

extern unsigned int text_timeout;
extern long text_start;
extern int redraw;
extern int mixer_changed;

extern unsigned int autorepeat;
extern unsigned int autoplay;
extern unsigned int autoprobe;
extern unsigned int time_mode;
extern unsigned int global_mode;

extern unsigned int volume;
extern  unsigned int muted_volume;
extern unsigned int muted;
extern unsigned int unmuted_volume;

extern unsigned int fade_ok;

extern unsigned int old_track;

extern int selectors_timeout;

/* The WorkMan stuff: */

extern char *cd_device;

/* from/for WorkMan database code: */

extern int mark_a;
extern int mark_b;
extern int cur_stopmode;
extern int cur_playnew;

extern char theme[];
extern char selected_theme[];
extern unsigned int theme_select;
extern unsigned int theme_select_nbr;
extern int no_mask;
extern unsigned int show_db;
extern unsigned int show_artist;
extern unsigned int show_db_pos;
extern unsigned int show_icon_db_pos;
extern unsigned int force_upper;
extern unsigned int fast_track;
extern unsigned int xflaunch;
extern unsigned int fade_out;
extern unsigned int fade_step;

extern int min_volume,max_volume;  /* from LibWorkMan */

extern int debug;

extern unsigned int panel;           /* current panel */
extern unsigned int panels;          /* how many panels? */
extern unsigned int but_max;         /* how many buttons? */
extern unsigned int but_msg;         /* which one is the message zone? */
extern unsigned int but_counter;     /* which one is the counter? */
extern unsigned int but_tracknbr;    /* which one is the track number? */
extern unsigned int but_db;
extern unsigned int icon_msg;
extern unsigned int icon_counter;
extern unsigned int icon_tracknbr;
extern unsigned int but_current;

extern unsigned int panel_stop;
extern unsigned int panel_play;

extern char th_name[];
extern char th_author[];
extern char th_release[];
extern char th_email[];
extern char th_url[];
extern char th_comment[];
extern char th_alpha1[];
extern char th_alpha2[];
extern char th_background[];
extern char th_icon_window[];

extern int th_no_minus;
extern int th_no_icon_window;

extern struct fak_button thdata[];

extern XpmIcon alphaXPM;
extern XpmIcon ralphaXPM;
extern XpmIcon backXPM;
extern XpmIcon iconXPM;

#ifdef WMK
#include "wings_ext.h"
#endif

#ifdef MIXER
#include "mixer_ext.h"
#endif
