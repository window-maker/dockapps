/* ===========================================================================
 * AScd: handlers.c
 * mouse events <-> integrated commands
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
 								   
#include "ext.h"
#include "faktory_prot.h"

void theme_select_init()
{
    char txt[256];
    DIR *dir_fd;
    struct dirent *dir_pt;
    int i;

    theme_select = 1;
    theme_select_nbr = 0;

    sprintf(txt, "%s/Themes", THDIR);
    if ((dir_fd = opendir(txt)) != NULL) {
	while((dir_pt = readdir(dir_fd)) != NULL) if (dir_pt->d_name[0] != '.') theme_select_nbr++;
	closedir(dir_fd);
    }
    
    sprintf(txt, "%s/Themes", THDIR);
    if ((dir_fd = opendir(txt)) != NULL) {
	i = 0;
	while((dir_pt = readdir(dir_fd)) != NULL) {
	    if (dir_pt->d_name[0] != '.') {
		i++;
		if (strcmp(dir_pt->d_name, theme) == 0) theme_select = i;
	    }
	}
	closedir(dir_fd);
    }
}

void fak_event_handle(int event, XEvent Event)
{

    /* global commands handling: panel switches, quit, GUI additional modules */

    switch (event) {
    case FAK_QUIT:
	exit(0);
	break;
    case FAK_PANEL_SWITCH:
	panel++;
	if (panel > panels) panel = 1;
	fak_redraw();
	fak_maskset();
	break;
    case FAK_PANEL1:
	panel = 1;
	fak_redraw();
	fak_maskset();
	break;
    case FAK_PANEL2:
	panel = 2;
	fak_redraw();
	fak_maskset();
	break;
    case FAK_PANEL3:
	panel = 3;
	fak_redraw();
	fak_maskset();
	break;
    case FAK_PANEL4:
	panel = 4;
	fak_redraw();
	fak_maskset();
	break;
    case FAK_PANEL5:
	panel = 5;
	fak_redraw();
	fak_maskset();
	break;
    case FAK_WINGS: /* must be compiled with WINGs support */
#ifdef WMK
	big_window(scr);
#endif
	break;
    case FAK_COUNTER_MODE:
	time_mode++ ;
	if (time_mode > 3) time_mode = 0;
	break;
    case FAK_TOG_SHOWDB:
	if (show_db) show_db = FALSE;
	else show_db = TRUE;
	redraw = TRUE;
	break;
    case FAK_TOG_SHOWARTIST:
	if (show_artist) show_artist = FALSE;
	else show_artist = TRUE;
	redraw = TRUE;
	break;
    case FAK_TOG_UPPER:
	if (force_upper) force_upper = FALSE;
	else force_upper = TRUE;
	redraw = TRUE;
	break;
    case FAK_TOG_AUTOPLAY:
	if (autoplay) autoplay = FALSE;
	else autoplay = TRUE;
	redraw = TRUE;
	break;
    case FAK_TOG_AUTOREPEAT:
	if (autorepeat) autorepeat = FALSE;
	else autorepeat = TRUE;
	redraw = TRUE;
	break;
    case FAK_TOG_ISKIPS:
	if (ignore_avoid) ignore_avoid = FALSE;
	else ignore_avoid = TRUE;
	redraw = TRUE;
	break;
    case FAK_TSELECT:
	if (theme_select == 0) {
	    selectors_timeout = time(NULL);
	    theme_select_init();
	} else {
	    theme_select = 0;
	    strcpy(theme, selected_theme);
	    fak_load_theme(theme, TRUE);
	    fak_maskset();
	}
	break;
    case FAK_TNEXT:
	selectors_timeout = time(NULL);
	if (theme_select == 0) theme_select_init();
	theme_select ++;
	if (theme_select > theme_select_nbr) theme_select = 1;
	break;
    case FAK_TPREVIOUS:
	selectors_timeout = time(NULL);
	if (theme_select == 0) theme_select_init();
	theme_select --;
	if (theme_select == 0) theme_select = theme_select_nbr;
	break;
    case FAK_FTSELECT:
	show_db_pos = 0;
	if (fast_track == 0) {
	    fast_track = cur_track;
	    selectors_timeout = time(NULL);
	} else {
	    if (fast_track != cur_track) {
		direct_track = fast_track;
		fast_track = 0;
		cd_control(DIRECTTRACK);
	    }
	}
	break;
    case FAK_FTNEXT:
	selectors_timeout = time(NULL);
	show_db_pos = 0;
	if (fast_track == 0) {
	    if (cur_track < cur_ntracks) fast_track = cur_track + 1;
	    else fast_track = 1;
	} else {
	    if (fast_track == cur_ntracks) fast_track = 1;
	    else fast_track++;
	}
	break;
    case FAK_FTPREVIOUS:
	selectors_timeout = time(NULL);
	show_db_pos = 0;
	if (fast_track == 0) {
	    if (cur_track > 1) fast_track = cur_track - 1;
	    else fast_track = cur_ntracks;
	} else {
	    if (fast_track == 1) fast_track = cur_ntracks;
	    else fast_track--;
	}
	break;
    case FAK_SAVE:
	newtext("Saving");
	save_rc_file();
	break;
    case FAK_LOAD:
	newtext("Loading");
	load_rc_file();
	break;
    case FAK_QREF:
	quick_reference(thdata[but_current].arg);
	break;
    default:
	break;
    }

    redraw = TRUE;
    fak_singlemask(but_current);
}

void cd_event_handle(int event, XEvent Event)
{
    /* All the AScd CD player commands, access to cd_control() */

    switch (event) {
    case FAK_CD_PLAY:
    case FAK_CD_PAUSE:
	if (cur_cdmode == WM_CDM_PAUSED) {
	    newtext("Playing");
	    wanna_play = TRUE;
	    cd_control(PAUSE);
	} else if (cur_cdmode == WM_CDM_PLAYING) {
	    if (fast_track != 0) {
		direct_track = fast_track;
		fast_track = 0;
		cd_control(DIRECTTRACK);
	    } else {
		newtext("Paused");
		cd_control(PAUSE);
	    }
	} else if (cur_cdmode == WM_CDM_EJECTED) {
	    /* the future?. Not yet working on my FreeBSD box... */
	    cd_control(CLOSETRAY);
	} else {
	    newtext("Play");
	    wanna_play = TRUE;
	    cd_control(PLAY);
	    if (cd->volume > 0) {
		volume=cd->volume;
		cd_volume(volume, 10, max_volume);
	    }
	}
	redraw = TRUE;
	fak_redraw();
	break;
    case FAK_CD_STOP:
	fast_track = 0;
	if (cur_cdmode != WM_CDM_STOPPED) {
	    newtext("Stop");
	    cd_control(STOPONLY);
	    wm_cd_status();
	    cur_track = 0;
	    redraw = TRUE;
	    fak_redraw();
	}
	break;
    case FAK_CD_STOPEJECT:
	fast_track = 0;
	if (cur_cdmode != WM_CDM_STOPPED) {
	    newtext("Stop");
	    cd_control(STOPONLY);
	    wm_cd_status();
	    cur_track = 0;
	    redraw = TRUE;
	    fak_redraw();
	    fak_maskset();
	} else {
	    newtext("Eject");
	    cd_control(EJECT);
	    redraw = TRUE;
	    fak_redraw();
	}
	break;
    case FAK_CD_EJECT:
	fast_track = 0;
	newtext("Eject");
	cd_control(EJECT);
	redraw = TRUE;
	fak_redraw();	
	break;
    case FAK_CD_EJECTQUIT:
	fast_track = 0;
	if ((cur_cdmode != WM_CDM_EJECTED) && (cur_cdmode != WM_CDM_STOPPED)) {
	    newtext("Eject");
	    cd_control(EJECT);
	    redraw = TRUE;
	    fak_redraw();
	} else {
	    exit(0);
	}
	break;
    case FAK_CD_NEXT:
	cd_control(UPTRACK);
	break;
    case FAK_CD_PREVIOUS:
	cd_control(DNTRACK);
	break;
    case FAK_CD_FIRST:
	cd_control(FIRST);
	break;
    case FAK_CD_LAST:
	cd_control(LAST);
	break;
    case FAK_CD_REW:
	cd_control(REV);
	break;
    case FAK_CD_FWD:
	cd_control(CUE);
	break;
    case FAK_CD_DIRECT:
	/* Direct Access:
	   we have to compute the offset to pass it to cd_control. We also have to
	   check if we want to move inside the current track or in the whole CD */
	
	if (cur_track < 1) cur_track = 1;
	
	if ((time_mode == 2) || (time_mode == 3)) {
	    if ((thdata[but_current].type == FAK_CD_BAR) || (thdata[but_current].type == FAK_CD_PIX)) {
		direct_access = (int)((float)(Event.xbutton.x - thdata[but_current].x) / (float)thdata[but_current].xpm.attributes.width * (float)cur_cdlen);
	    } else if (thdata[but_current].type == FAK_ICD_BAR) {
		direct_access = (int)((float)(Event.xbutton.y - thdata[but_current].y) / (float)thdata[but_current].xpm.attributes.height * (float)cur_cdlen);
		direct_access = cur_cdlen - direct_access;
	    } else {
		direct_access = (int)((float)(Event.xbutton.y - thdata[but_current].y) / (float)thdata[but_current].xpm.attributes.height * (float)cur_cdlen);
	    }
	    cd_control(GLOBALACCESS);
	} else {
	    if ((thdata[but_current].type == FAK_CD_BAR) || (thdata[but_current].type == FAK_CD_PIX)) {
		direct_access = (int)((float)(Event.xbutton.x - thdata[but_current].x) / (float)thdata[but_current].xpm.attributes.width * (float)cur_tracklen);
	    } else if (thdata[but_current].type == FAK_ICD_BAR) {
		direct_access = (int)((float)(Event.xbutton.y - thdata[but_current].y) / (float)thdata[but_current].xpm.attributes.height * (float)cur_tracklen);
		direct_access = cur_tracklen - direct_access;
	    } else {
		direct_access = (int)((float)(Event.xbutton.y - thdata[but_current].y) / (float)thdata[but_current].xpm.attributes.height * (float)cur_tracklen);
	    }
	    cd_control(DIRECTACCESS);
	}
	wm_cd_status();
	redraw = TRUE;
	fak_redraw();
	fak_maskset();
	break;
    case FAK_CD_VOLUME:
	if ((cur_cdmode != WM_CDM_EJECTED) && (cur_cdmode != WM_CDM_STOPPED)) {
	    if ((thdata[but_current].xpm.attributes.width > 0) && (max_volume > 0)) {
		if (thdata[but_current].type == FAK_VVOL_BAR) {
		    volume = (int)((float)(Event.xbutton.y - thdata[but_current].y) / (float)thdata[but_current].xpm.attributes.height * (float)max_volume);
		} else if (thdata[but_current].type == FAK_IVOL_BAR) {
		    volume = (int)((float)(Event.xbutton.y - thdata[but_current].y) / (float)thdata[but_current].xpm.attributes.height * (float)max_volume);
		    volume = max_volume - volume;
		} else {
		    volume = (int)((float)(Event.xbutton.x - thdata[but_current].x) / (float)thdata[but_current].xpm.attributes.width * (float)max_volume);
		}
		cd_volume(volume, 10, max_volume);
		wm_cd_status();
		redraw = TRUE;
		fak_redraw();
	    }
	}
	break;
    case FAK_CD_LOOP:
	if (loop_mode) {
	    loop_mode = 0;
	    newtext("Loop Off");
	} else {
	    cd_control(LOOP);
	    if (!anomalie) newtext("Loop On");
	    else newtext("ERROR!");
	}
	redraw = TRUE;
	break;
    case FAK_CD_LSTART:
	loop_1 = cur_pos_rel;
	newtext("L. Start");
	loop_start_track = cur_track;
	break;
    case FAK_CD_LEND:
	loop_2 = cur_pos_rel;
	loop_end_track = cur_track;
	newtext("L. End");
	break;
    case FAK_CD_GOLSTART:
	direct_access = loop_1;
	cd_control(DIRECTACCESS);
	break;
    case FAK_CD_GOLEND:
	direct_access = loop_2;
	cd_control(DIRECTACCESS);
	break;
    case FAK_CD_LTRACK:
	loop_1 = 0;
	loop_2 = cur_tracklen;
	loop_start_track = cur_track;
	loop_end_track = cur_track;
	break;
    case FAK_CD_LCLEAR:
	loop_1 = 0;
	loop_2 = 0;
	loop_start_track = 1;
	loop_end_track = 1;
	newtext("L. Clear");
	break;
    case FAK_CD_INTRO:
	cd_control(INTROSCAN);
	wm_cd_status();
	newtext("Intro");
	fak_maskset();
	break;
    case FAK_CD_MUTE:
	if (muted) {
	    muted = 0;
	    newtext("Mute Off");
	    cd_volume(volume, 10, max_volume);
	} else {
	    newtext("Mute On");
	    muted = 1;
	    cd_volume(muted_volume, 10, max_volume);
	}
	break;
    case FAK_CD_FADE:
	fade_out = TRUE;
	newtext("Fade...");
	/*fak_redraw();*/
	break;
    default:
	break;
    }

    redraw = TRUE;
    fak_singlemask(but_current);
}
