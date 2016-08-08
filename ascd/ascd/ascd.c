/* ===========================================================================
 * AScd: the AfterStep and WindowMaker CD player
 * ascd.c: main source
 * ===========================================================================
 * Copyright (c) 1999 Denis Bourez and Rob Malda. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Denis Bourez & Rob Malda
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DENIS BOUREZ AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL DENIS BOUREZ, ROB MALDA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 * ===========================================================================
 */

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

#include "config.h"
#include "ascd.h"
#include "faktory_prot.h"
#include <workman/workman.h>

XpmIcon alphaXPM;
XpmIcon ralphaXPM;
XpmIcon backXPM;
XpmIcon iconXPM;

/* External functions */

extern time();
extern open();
extern cd_control(int);
extern cd_control_version();

/* the various switches: */

int debug = FALSE;                     /* verbose mode */
unsigned int autoplay = 0;         /* if set, play a disk when it is inserted -> see -a c.l. option  */
unsigned int autoprobe = TRUE;     /* probe the drive, but do not play (autoplay) */
unsigned int intro_mode = 0;       /* play only the beginning of the tracks */
unsigned int autorepeat = FALSE;   /* zzzzzzzzzzzzz */
unsigned int blind_mode = FALSE;   /* no counter updates = no drive LED flashes */
unsigned int loop_mode = 0;        /* loop from loop_1 to loop_2 */
unsigned int loop_start_track = 0; /* track of the beggining of the loop */
unsigned int loop_end_track =0;    /* track of the end of the loop */
unsigned int loop_1 = 0;           /* pos. 1 */
unsigned int loop_2 = 0;           /* pos. 2 */
unsigned int show_db = FALSE;      /* do we have to scroll song names? */
unsigned int show_artist = FALSE;  /* if show_db, do you append artist name? */
unsigned int show_db_pos = 0;      /* internal, used for the song name scrolling */
unsigned int show_icon_db_pos = 0; /* internal, used for the song name scrolling */
unsigned int force_upper = FALSE;  /* all messages must be in uppercase */
int ignore_avoid = FALSE;          /* if set, we play *all* tracks, ignoring 'avoid' tags in database */
unsigned int fast_track = 0;       /* the fast track select method */
unsigned int xflaunch = FALSE;     /* do we launch xfascd when right click on the display? */
unsigned int time_mode = 0;        /* display mode for the counter. see function RedrawWindow() */
unsigned int cue_time = 10;        /* nbr of seconds for cue -> see -c command line option */

/* internals */

int lasttime = -1;
extern char *cd_device;            /* the hardware device pointing to the CD ROM player */
unsigned int datatrack = 0;        /* is the current track a data track ? */
unsigned int direct_access = 0;    /* pos. to reach with the ruler */
unsigned int direct_track = 0;     /* if we want to go directly to another track */
unsigned int wanna_play = FALSE;
unsigned int do_autorepeat = FALSE;
int wanted_track = 0;
unsigned int old_track = 0;
unsigned int anomalie = 0;         /* cd_control return value */

/* let's talk about X... */

Display *Disp;
Window Root;
Window Iconwin;
Window Win;
char *Geometry = 0;
char device[128]=DEFAULTDEVICE;
char xv[128];
int withdrawn=FALSE;
GC WinGC;
int CarrierOn = FALSE;
int screen;

/* everything dealing with the hardware volume: */

int volume = MAX_VOL ;             /* CD volume */
int muted_volume = 0;              /* CD volume in muted mode */
int unmuted_volume = MAX_VOL;      /* CD volume to restore when leaving muted mode */
unsigned int muted = FALSE;        /* is the CD muted? */
unsigned int fade_out = FALSE;     /* do you have to start a fade in/out? */
unsigned int fade_step = 5;        /* the fading speed */
unsigned int fade_ok = 0;          /* can't remember. sorry!!!! */
extern int min_volume,max_volume;  /* from LibWorkMan */
int cur_balance = 10;              /* ? */

char led_text[9];                  /* the 'help' messages */
unsigned int text_timeout = 1;     /* messages timemout in seconds */
long text_start = 0;               /* timeout */

/* misc, have to add explanations here...! */

int selectors_timeout = 0;
int redraw = TRUE;
int has_clicked = FALSE;
int info_modified = 0;
int big_spaces = 0;
int slow_down = 0;

/* ====================================================================
   Faktory: theme subsystem
   ==================================================================== */

char theme[FAK_CMAX];
char selected_theme[FAK_CMAX];
unsigned int theme_select = 0;
unsigned int theme_select_nbr = 0;

unsigned int panel = 1;           /* current panel */
unsigned int panels = 0;          /* how many panels? */
unsigned int but_max = 0;         /* how many buttons? */
unsigned int but_msg = 0;         /* which one is the message zone? */
unsigned int but_counter = 0;     /* which one is the counter? */
unsigned int but_tracknbr = 0;    /* which one is the track number? */
unsigned int but_db = 0;          /* which one is the database zone? */
unsigned int icon_msg = 0;
unsigned int icon_counter = 0;
unsigned int icon_tracknbr = 0;
unsigned int but_current = 0;     /* last selected button */

unsigned int panel_stop = 0;      /* autoswitch to this panel on stop/eject */
unsigned int panel_play = 0;      /* autoswitch to this panel on play */

char th_name[FAK_CMAX];
char th_author[FAK_CMAX];
char th_release[FAK_CMAX];
char th_email[FAK_CMAX];
char th_url[FAK_CMAX];
char th_comment[FAK_CMAX];
char th_alpha1[FAK_CMAX];
char th_alpha2[FAK_CMAX];
char th_background[FAK_CMAX];
char th_icon_window[FAK_CMAX];

int th_no_minus = FALSE; 
int th_no_icon_window = FALSE;

struct fak_button thdata[FAK_BMAX];

/*****************************************************************************/

/* ====================================================================
   The optional modules globals:
   ==================================================================== */

#ifdef WMK
#include "wings_global.c"
#endif

#ifdef MIXER
#include "mixer_global.c"
#endif

/*****************************************************************************/

void mouse_events(XEvent Event) 
{
    int i;
    int j;
    int do_it = TRUE;

    for (i=1 ; i <= but_max ; i++) {
	if ((thdata[i].panel == panel) || (thdata[i].panel == 0)) {
	    if (strlen(thdata[i].xpm_file) > 0) {
		if ((Event.xbutton.y >= thdata[i].y) && (Event.xbutton.y <= thdata[i].y + thdata[i].xpm.attributes.height)) {
		    if ((Event.xbutton.x >= thdata[i].x) && (Event.xbutton.x <= thdata[i].x + thdata[i].xpm.attributes.width)) {
			if (th_no_icon_window) {
			    /* Special Icon Mode: we must check in which window the user clicked */
			    do_it = TRUE;
			    if (Event.xbutton.window == Iconwin) {
				if (!thdata[i].icon) do_it = FALSE;
			    } else {
				if (thdata[i].icon) do_it = FALSE;
			    }
			}
			/* no commands defined? We skip this one! */
			if (((thdata[i].left != 0) || (thdata[i].mid != 0) || (thdata[i].right != 0)) && (do_it)) {
			    if (debug) fprintf(stderr, "** User selected button %d with mouse button %d\n", i, Event.xbutton.button);
			    j = 0;
			    switch(Event.xbutton.button) {
			    case 1: j = thdata[i].left; break;
			    case 2: j = thdata[i].mid; break;
			    default: j = thdata[i].right; break;
			    }
			    but_current = i;
			    if (debug) fprintf(stderr, "-> command = %d\n", j);
			    if ((j >= 1) && (j <= 49)) fak_event_handle(j, Event);
			    else if ((j >= 50) && (j <= 99)) cd_event_handle(j, Event);
#ifdef MIXER
			    else if ((j >= 100) && (j <= 199)) mixer_event_handle(j, Event);
#endif
			    break;
			}
		    }
		}
	    }
	}
    }

} 

/* ------------------------------------------------------------------------
   GUI control
   ------------------------------------------------------------------------ */

Pixel get_color(char *ColorName) 
{
    XColor Color;
    XWindowAttributes Attributes;

    XGetWindowAttributes(Disp,Root,&Attributes);
    Color.pixel = 0;

    if (!XParseColor (Disp, Attributes.colormap, ColorName, &Color))
      fprintf(stderr,"ascd: can't parse %s\n", ColorName);
    else if(!XAllocColor (Disp, Attributes.colormap, &Color))
      fprintf(stderr,"ascd: can't allocate %s\n", ColorName);

    return Color.pixel;
} 

void create_window(int argc, char **argv) 
{
    int i;
    unsigned int borderwidth ;
    char *display_name = NULL;
    char *wname = "AScd";
    XGCValues gcv;
    unsigned long gcm;
    XTextProperty name;
    Pixel back_pix, fore_pix;
    int x_fd;
    int d_depth;
    int ScreenWidth, ScreenHeight;
    XSizeHints SizeHints;
    XWMHints WmHints;
    XClassHint classHint;

    /* Open display */
    if (!(Disp = XOpenDisplay(display_name))) {
	fprintf(stderr,"ascd: can't open display %s\n", XDisplayName(display_name));
	exit (1);
    }

    screen = DefaultScreen(Disp);

#ifdef WMK
    scr = WMCreateScreen(Disp, DefaultScreen(Disp));
    create_big_window(scr);
    create_db_window(scr);
    create_about_window(scr);
#endif

    Root = RootWindow(Disp, screen);
    d_depth = DefaultDepth(Disp, screen);
    x_fd = XConnectionNumber(Disp);
    ScreenHeight = DisplayHeight(Disp,screen);
    ScreenWidth = DisplayWidth(Disp,screen);

    /* it's time to load the visual theme! */

    if (fak_load_theme(theme, FALSE)) {
       if (debug) fprintf(stderr, "-> back from fak_load_theme!\n");
    } else {
	fprintf(stderr, "ascd: fatal error\n\n");
	fprintf(stderr, "The '%s' theme definition file can't be read.\n\n", theme);
	exit(1);
    }

    SizeHints.flags= USSize|USPosition;
    SizeHints.x = 0;
    SizeHints.y = 0;
    back_pix = get_color("black");
    fore_pix = get_color("white");

    XWMGeometry(Disp, screen, Geometry, NULL, (borderwidth = 1), &SizeHints,
		&SizeHints.x,&SizeHints.y,&SizeHints.width,
		&SizeHints.height, &i);

    SizeHints.width = backXPM.attributes.width;
    SizeHints.height= backXPM.attributes.height;

    Win = XCreateSimpleWindow(Disp, Root,
			      SizeHints.x,
			      SizeHints.y,
			      SizeHints.width,
			      SizeHints.height,
			      borderwidth,
			      fore_pix,
			      back_pix);

    if (!th_no_icon_window) {
	Iconwin = XCreateSimpleWindow(Disp,Win,
				      SizeHints.x,SizeHints.y,
				      SizeHints.width,
				      SizeHints.height,
				      borderwidth,
				      fore_pix,
				      back_pix);
    } else {
	Iconwin = XCreateSimpleWindow(Disp, Win, 
				      SizeHints.x,
				      SizeHints.y,
				      iconXPM.attributes.width,
				      iconXPM.attributes.height,
				      borderwidth,
				      fore_pix,
				      back_pix);
    }

    XSetWMNormalHints(Disp, Win, &SizeHints);

    classHint.res_name = "ascd" ;
    classHint.res_class = "AScd";

    XSetClassHint (Disp, Win, &classHint);

    XSelectInput(Disp, Win, (ExposureMask | ButtonPressMask | StructureNotifyMask));

    XSelectInput(Disp, Iconwin, (ExposureMask | ButtonPressMask |
				 StructureNotifyMask));
    
    if (XStringListToTextProperty(&wname, 1, &name) ==0) {
	fprintf(stderr, "ascd: can't allocate window name\n");
	exit(-1);
    }

    XSetWMName(Disp, Win, &name);

    /* Create WinGC */
    gcm = GCForeground|GCBackground|GCGraphicsExposures;
    gcv.foreground = fore_pix;
    gcv.background = back_pix;
    gcv.graphics_exposures = False;
    WinGC = XCreateGC(Disp, Root, gcm, &gcv);

    WmHints.initial_state = withdrawn ? WithdrawnState : NormalState ;
    WmHints.icon_window = Iconwin;
    WmHints.window_group = Win;
    WmHints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
    WmHints.icon_x = SizeHints.x;
    WmHints.icon_y = SizeHints.y;
    XSetWMHints(Disp, Win, &WmHints);
    XSetCommand(Disp, Win, argv, argc);
    
    XMapWindow(Disp, Win);

    
    if (debug) fprintf(stderr, "-> calling fak_maskset() from create_window()...\n");
    fak_maskset();
    if (debug) fprintf(stderr, "-> calling fak_redraw() from create_window()...\n");
    fak_redraw();
    if (debug) fprintf(stderr, "-> Leaving create_window()\n");
} 

void newtext(char *txt) { 
    strcpy(led_text, txt);
    text_start = 0;
} 

void show_icon_db_f() {
    /* scroll the song title */
    char txt[256];
    char txt2[256];
    char dsp[9];
    int track;
    int red;

    if (!th_no_icon_window) return;
    if (icon_msg == 0) return;

    if (theme_select == 0) {

	/* there is a message to display, we'll wait for the message to
	   time-out before handling the track title... */

	if (strlen(led_text) > 0) return;

	/* if in fast track selector, we show the title of the track to reach */

	if (fast_track > 0) {
	    track = fast_track - 1;
	    red = FALSE;
	} else {
	    track = cur_track - 1;
	    red = TRUE;
	}

        if (track > cur_ntracks - 1) return;
       
	if (cd->trk[track].songname != NULL) {
	    if (show_artist) {
		if (cd->artist != NULL) {
		    sprintf(txt2, "%s: %s", cd->artist, cd->trk[track].songname);
		} else {
		    strcpy(txt2, cd->trk[track].songname);
		}
	    } else {
		strcpy(txt2, cd->trk[track].songname);
	    }
	    fak_icon_text("", MSG_PANEL, 0, red);
	    if (strlen(txt2) > thdata[icon_msg].w) {
		strcpy(txt, "       ");
		strcat(txt, txt2);
		if (show_icon_db_pos > strlen(txt)) show_icon_db_pos = 0;
		tes_sncpy(dsp, txt + show_icon_db_pos, thdata[icon_msg].w);
		fak_icon_text(dsp, MSG_PANEL, 0, red);
		show_icon_db_pos ++;
	    } else {
		fak_icon_text(txt2, MSG_PANEL, 0, red);
	    }
	} else {
	    strcpy(txt, "");
	}
    }
}

void show_db_f() {
    /* scroll the song title */
    char txt[256];
    char txt2[256];
    char dsp[9];
    int track;
    int red;
    int where;
    int longueur;
    int i;

    if (but_db > 0) {
	where = DB_PANEL;
	longueur = thdata[but_db].w;
    } else {
	where = MSG_PANEL;
	longueur = thdata[but_msg].w;
    }

    if ((theme_select == 0) || (where == DB_PANEL)) {

	/* 
	   there is a message to display, we'll wait for the message
	   to time-out before handling the track title... (but only if
	   there is no separate areas) 
	*/

	if ((strlen(led_text) > 0) && (where == MSG_PANEL)) return;

	/* if in fast track selector, we show the title of the track to reach */

	if (fast_track > 0) {
	    track = fast_track - 1;
	    red = FALSE;
	} else {
	    track = cur_track - 1;
	    red = TRUE;
	}

	/* warning: avoid weird datas: */

        if (track > cur_ntracks - 1) return;
	if (track < 0) return;
       
       	if (debug > 1) fprintf(stderr, "** Show DB TRACK = %d\n", track);

	if (cd->trk[track].songname != NULL) {
	    if (show_artist) {
		if (cd->artist != NULL) {
		    sprintf(txt2, "%s: %s", cd->artist, cd->trk[track].songname);
		} else {
		    strcpy(txt2, cd->trk[track].songname);
		}
	    } else {
		strcpy(txt2, cd->trk[track].songname);
	    }
	    fak_text("", where, 0, red);
	    if (strlen(txt2) > longueur) {
		strcpy(txt, "");
		for (i = 0 ;  i < longueur ; i++) strcat(txt, " ");
		strcat(txt, txt2);
		if (show_db_pos > strlen(txt)) show_db_pos = 0;
		tes_sncpy(dsp, txt + show_db_pos, longueur);
		fak_text(dsp, DB_PANEL, 0, red);
		show_db_pos ++;
	    } else {
		fak_text(txt2, DB_PANEL, 0, red);
	    }
	} else {
	    strcpy(txt, "");
	}
    }

    show_icon_db_f();
}

/* ------------------------------------------------------------------------
   Loooooooooooooooooping.........
   ------------------------------------------------------------------------ */

void main_loop() 
{
    unsigned int no_disk = 0;
    long int dodo = RDTIME;
    XEvent Event;

    while(1) {
       
        if (debug > 1) fprintf(stderr, "** [Main Loop] mode = %02d track = %02d \n", cur_cdmode, cur_track);
       
	if (cur_cdmode == WM_CDM_EJECTED) no_disk = 1;

	slow_down++;
	
	if (slow_down > 10) {

#ifdef WMK
	    /* in 0.13, we no longer close WINGs windows if ejected
	    if ((cur_cdmode == WM_CDM_EJECTED) && (en_vue)) {
		WMCloseWindow(win);
		en_vue = FALSE;
	    }
	    */
#endif

	    if (no_disk == 1) {
		cur_ntracks = 0; /* 0.13pr6: what a hack!!!! */

		if (autoplay || autoprobe) {
		    dodo = RDTIME2;
		    wm_cd_status();
		}

		if (cur_cdmode != WM_CDM_EJECTED) no_disk = 0;
		if ( (cur_cdmode == WM_CDM_STOPPED) && (autoplay) )  {
		    newtext("Autoplay");
		    if (cd->volume > 0) {
			volume=cd->volume;
			cd_volume(volume, 10, max_volume);
		    }
		    cd_control(PLAY);
		    wm_cd_status();
		    dodo = RDTIME;
		    fak_maskset();
		    fak_redraw();
		}
	    }
	    
	    /* The Loop Mode : */
	    if ( (cur_track == loop_end_track ) && (cur_pos_rel >= loop_2) && (loop_mode) ) {
		cd_control(LOOP);
		fak_redraw();
	    }
	    
	    /* The Intro Scan Mode : */
	    if ( (cur_pos_rel > cue_time) && (intro_mode) ) {
		cd_control(INTRONEXT);
		fak_redraw();
	    }
	}

	if ((slow_down == 1) || (slow_down == 6)) {
	    if ((show_db) && (cur_cdmode == WM_CDM_PLAYING)) show_db_f();
	}

	/* Check events */
	
	while (XPending(Disp))
	    {
		
		XNextEvent(Disp, &Event);
		
#ifdef WMK		
		if (!WMHandleEvent(&Event)) {
#endif
		switch(Event.type) {
		    
		    /* ---------------------- Redraw Window --------------------- */
		    
		case Expose:
		    if(Event.xexpose.count == 0) {
			lasttime=01;
			redraw = TRUE;
			fak_redraw();
		    } else {
			if (debug > 1) fprintf(stderr, "** XEVent - expose, not handled, count = %d\n", Event.xexpose.count);
		    }
		    break;
		    
		    /* ----------------------- Mouse Click ---------------------- */
		    
		case ButtonPress:
		    
		    wm_cd_status();
		    mouse_events(Event);
		    break;
		    
		    /* ------------------------ Destroy Window ------------------- */
		    
		case DestroyNotify:
		    XFreeGC(Disp, WinGC);
		    XDestroyWindow(Disp, Win);
		    XDestroyWindow(Disp, Iconwin);
		    XCloseDisplay(Disp);
		    exit(0);
		    break;
		default:
		    if (debug > 1) fprintf(stderr, "** XEvent - unknown event type = %d\n", Event.type);
		    break;
		}

#ifdef WMK		
		}
#endif
		XFlush(Disp);
	    } /* check event */
	
	usleep(dodo);
	
	/* ----------------- now we have to redraw the screen: ---------------- */
	
	if ((slow_down > 10) || (has_clicked)) {
	    fak_redraw();
	    slow_down = 0;
	    has_clicked = FALSE;
	}
    }
} 

/* ------------------------------------------------------------------------
   So... let's go!
   ------------------------------------------------------------------------ */

int main(int argc, char *argv[]) 
{
    extern char *rcfile, *dbfiles;  

    /*printf("AScd %s\n", VERSION);*/
    
    /* CD device: */

#ifndef NO_D_DEVICE
    cd_device = malloc(strlen(DEFAULTDEVICE) + 1);
    strcpy(cd_device, DEFAULTDEVICE);
#endif

    strcpy(led_text, "");
    strcpy(theme, "default");
    strcpy(xv, "xv");
    
    /* the WorkMan database. It's still not used in ascd, but we need this
       to start the WorkMan code
    */

    rcfile = getenv("WORKMANRC");
    if (rcfile) rcfile = (char *)wm_strdup(rcfile);
    dbfiles = getenv("WORKMANDB");
    if (dbfiles) dbfiles = (char *)wm_strdup(dbfiles);
    split_workmandb();

    load_rc_file(FALSE);
    command_line_parse(argc, argv);

#ifdef WMK
   if (debug) fprintf(stderr, "** [WINGs] init app...\n");
    WMInitializeApplication("AScd", &argc, argv);
#endif

#ifdef MIXER
    mixer_ok = 1;
   if (debug) fprintf(stderr, "** [mixer] checking mixer device...\n");
    if (! check_mixer()) {
        fprintf(stderr, "ascd: can't initialize mixer device. Mixing support disabled.\n");
	mixer_ok = 0;
    }
#endif

   if (debug) fprintf(stderr, "** creating main window...\n");
   create_window(argc, argv);
   if (debug) fprintf(stderr, "** checking CD status...\n");
   wm_cd_status();
    
    if (cur_cdmode != WM_CDM_EJECTED) {
       if (debug) fprintf(stderr, "** reading CD initial volume...\n");
	volume = wm_cd_read_initial_volume(max_volume);
    } else {
	if (debug) fprintf(stderr, "** CD is ejected: volume ignored.\n");
    }

    if (debug) fprintf(stderr, "** checking autoplay...\n");
    if ((autoplay) && (cur_cdmode == WM_CDM_STOPPED)) {
	if (cur_track < 1) {
	    cur_track = 1;
	    wm_cd_status();
	}
       if (debug) fprintf(stderr, "-> autoplay: play command\n");
	cd_control(PLAY);
       if (debug) fprintf(stderr, "-> autoplay: checking status\n");
	wm_cd_status();
	if (debug) fprintf(stderr, "-> autoplay: redrawing\n");
	fak_redraw();
       if (debug) fprintf(stderr, "-> autoplay: done\n");
    } else {
       if (debug) fprintf(stderr, "-> no autoplay\n");
    }

   if (debug) fprintf(stderr, "** checking current CD volume in database\n");
   
    if (((cur_cdmode == WM_CDM_PLAYING) || (cur_cdmode == WM_CDM_PAUSED)) && (cd->volume > 0)) {
	volume=cd->volume;
	cd_volume(volume, 10, max_volume);
    }
   
    fak_maskset();

    if (debug) fprintf(stderr, "** Init passed. Entering main loop.\n");
    redraw = TRUE;
    main_loop();
    return 0;
}
