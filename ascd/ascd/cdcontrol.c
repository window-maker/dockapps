/* ===========================================================================
 * AScd: cdcontrol.c
 * This is cd_control() function: accept orders from the ui player and send 
 * the appropriate commands to libworkman.
 *
 * Please do *not* modify this function as it is used without changes by
 * several programs!
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
 								   
#define CD_C_VERSION "1.4"
#include "ext.h"

void 
cd_control(int order)
{
    static int pos_changed = FALSE;
    int currenttrack;

    anomalie = 0;
    wanna_play = FALSE;

#ifdef XFASCD
    update_xpm = TRUE;
#endif

    if (cur_cdmode != WM_CDM_EJECTED) {
	if (cd->trk[cur_track - 1].data) datatrack = 1;
	else datatrack = 0;
    }

    if ((cur_ntracks == 1) && (cd->trk[cur_track - 1].data)) {

	/* only one track, and it's a data track. We ignore 
	   all commands to avoid some funny things. Only the EJECT
	   commands are allowed */

	if ((order != STOP) && (order != EJECT)) {
	    anomalie = 1;
	    return;
	}
    }
    
    switch(order) {

    case PLAY:
	loop_mode = 0;
	intro_mode = 0;
	if (! pos_changed) {
	    wm_cd_status();
	} else {
	    pos_changed = FALSE;
	}

	if (cur_track < 1) cur_track = 1;

	/* don't play data tracks: */
          
	if (! cd->trk[cur_track - 1].data) {
	    if (cur_cdmode != WM_CDM_EJECTED) {
		if (cur_cdmode != WM_CDM_PAUSED) {
		    if (do_autorepeat) {
			cur_track = 1;
			do_autorepeat = FALSE;
		    }

		    /* skip data and avoid tracks */
		    while (((cd->trk[cur_track - 1].data) || ((cd->trk[cur_track - 1].avoid == 1) && (!ignore_avoid))         ) && (cur_track <= cur_ntracks)) cur_track++;

		    if (cur_track > cur_ntracks) cur_track--;
		    if (!(cd->trk[cur_track - 1].data)) {
			wanna_play = TRUE;
			wm_cd_play(cur_track, 0, cur_ntracks + 1);
		    }
		}
	    }
	}
	break;

    case PAUSE:
	loop_mode = 0;
	intro_mode = 0;

	if (pos_changed) {
	    cur_track = wanted_track;
	    wanna_play = TRUE;
	    wm_cd_play(cur_track, 0, cur_ntracks + 1);
	    cur_cdmode = WM_CDM_PLAYING;
	} else {
	    wm_cd_pause();
	}

	if (cur_cdmode == WM_CDM_PLAYING) {
	    pos_changed = FALSE;
	}
	break;

    case STOP: /* Civilized handler: STOP and EJECT combined */
	loop_mode = 0;
	intro_mode = 0;
	if ((cur_cdmode == WM_CDM_PLAYING) || (cur_cdmode == WM_CDM_PAUSED)) {
	    cur_cdmode = WM_CDM_PAUSED;
	    wm_cd_stop();
	    cur_cdmode = WM_CDM_STOPPED;
	} else {
	    if (cur_cdmode != WM_CDM_EJECTED) {
		currenttrack = 0;
		wm_cd_eject();
	    }
	}
	pos_changed = FALSE;
	break;

    case STOPONLY: /* STOP, no more. Doesn't eject if CD is already stopped */
	loop_mode = 0;
	intro_mode = 0;
	if ((cur_cdmode == WM_CDM_PLAYING) || (cur_cdmode == WM_CDM_PAUSED)) {
	    cur_cdmode = WM_CDM_PAUSED;
	    wm_cd_stop();
	    cur_cdmode = WM_CDM_STOPPED;
	}
	pos_changed = FALSE;
	break;

    case EJECT: /* Troll handler: don't try to guess the current mode, just eject */
	if (cur_cdmode != WM_CDM_EJECTED) {
	    loop_mode = 0;
	    intro_mode = 0;
	    currenttrack = 0;
	    wm_cd_eject();
	}
	break;

    case CLOSETRAY: /* let's try this */
	if (cur_cdmode == WM_CDM_EJECTED) {
	    /* commented, as current libworkman crashes when it receive this order */
	    /*wm_cd_closetray();*/
	}
	break;

    case UPTRACK: /* next track */
	loop_mode = 0;
	intro_mode = 0;
	if (pos_changed) cur_track = wanted_track;

	if (cur_track + 1 > cur_ntracks ) {
	    cur_track = 1;
	} else {
	    cur_track ++;
	}
	if (cur_cdmode == WM_CDM_PAUSED) {
	    pos_changed = TRUE;
	    wanted_track = cur_track;
	}
	if (cur_cdmode == WM_CDM_PLAYING) {
	    wanna_play = TRUE;
	    wm_cd_play(cur_track, 0, cur_ntracks + 1);
	}
	break;

    case DNTRACK: /* previous track */
	loop_mode = 0;
	intro_mode = 0;
	if (pos_changed) cur_track = wanted_track;

	if (cur_cdmode == WM_CDM_PAUSED) {
	    if (cur_pos_rel < 2 || pos_changed) cur_track--;
	    cur_pos_rel = 0;
	    pos_changed = TRUE;
	} else if (cur_cdmode == WM_CDM_PLAYING) {
	    if (cur_pos_rel < 2) cur_track--;
	    cur_pos_rel = 0;
	} else {
	    cur_track--;
	}
	if (cur_track < 1) cur_track = cur_ntracks;
	if (cur_cdmode == WM_CDM_PAUSED) {
	    pos_changed = TRUE;
	    wanted_track = cur_track;
	}
	if (cur_cdmode == WM_CDM_PLAYING) {
	    wanna_play = TRUE;
	    wm_cd_play(cur_track, 0, cur_ntracks + 1);
	}
	break;

    case DIRECTTRACK: /* direct access to a random track */
	loop_mode = 0;
	intro_mode = 0;

	if (direct_track < 1) direct_track = 1;

	if (cur_cdmode == WM_CDM_PAUSED) pos_changed = TRUE;
	cur_track = direct_track;
	if (cur_track >= cur_ntracks + 1) cur_track = 1;

	/* if it's a data track, skip it */
	while ((cd->trk[cur_track - 1].data) && (cur_track <= cur_ntracks)) cur_track++;
          
	if (cur_cdmode == WM_CDM_PLAYING) {
	    wanna_play = TRUE;
	    wm_cd_play(cur_track, 0, cur_ntracks + 1);
	}
	break;

    case CUE:
	loop_mode = 0;
	intro_mode = 0;
	if (cur_cdmode == WM_CDM_PLAYING) {
	    wanna_play = TRUE;
	    wm_cd_play(cur_track, cur_pos_rel + cue_time, cur_ntracks + 1);
	}
	break;

    case REV:
	loop_mode = 0;
	intro_mode = 0;
	if ( (cur_cdmode == WM_CDM_PLAYING) && ( (cur_pos_rel - cue_time) >= 0 ) ) {
	    wanna_play = TRUE;
	    wm_cd_play(cur_track, cur_pos_rel - cue_time, cur_ntracks + 1);
	}
	break;

    case FIRST:
	loop_mode = 0;
	intro_mode = 0;
	cur_track = 1;
	if (cur_cdmode == WM_CDM_PLAYING) {
	    wanna_play = TRUE;
	    wm_cd_play(cur_track, 0, cur_ntracks + 1);
	} else if (cur_cdmode == WM_CDM_PAUSED) {
	    pos_changed = TRUE;
	    wanted_track = cur_track;
	}
	break;

    case LAST:
	loop_mode = 0;
	intro_mode = 0;
	cur_track = cur_ntracks;
	if (cur_cdmode == WM_CDM_PLAYING) {
	    wanna_play = TRUE;
	    wm_cd_play(cur_track, 0, cur_ntracks + 1);
	} else if (cur_cdmode == WM_CDM_PAUSED) {
	    pos_changed = TRUE;
	    wanted_track = cur_track;
	}
	break;

    case LOOP:
	if ( (loop_2 > loop_1) ||
	     (loop_end_track > loop_start_track) ) {
               
	    if (loop_start_track == 0) loop_start_track = 1;
	    intro_mode = 0;
	    if ( ( loop_2 > loop_1 ) 
		 || ( loop_start_track != loop_end_track ) )
		currenttrack = loop_start_track;
	    else {
		loop_end_track = cur_ntracks;
		currenttrack = loop_start_track = 1;
		loop_1 = 0;
		loop_2 = 
		    thiscd.trk[ loop_end_track - 1 ].length - 1;
	    }
	    wm_cd_play(loop_start_track, loop_1, cur_ntracks + 1);
	    wanna_play = TRUE;
	    loop_mode = 1;
	} else {
	    anomalie = 1;
	}
	break;

    case DIRECTACCESS: /* direct access to a random position of the current track */
	if (direct_access < 0) direct_access = 0;
	loop_mode = 0;
	intro_mode = 0;
	wm_cd_status();
	wm_cd_play(cur_track, direct_access, cur_ntracks + 1);
	wanna_play = TRUE;
	break;

    case GLOBALACCESS:
	if (direct_access < 0) direct_access = 0;
	loop_mode = 0;
	intro_mode = 0;
	wm_cd_status();
	wm_cd_play(1, direct_access, cur_ntracks + 1);
	wanna_play = TRUE;
	break;

    case INTROSCAN:
	if (! intro_mode) {
	    intro_mode = 1;
	    wm_cd_play(cur_track, 0, cur_ntracks + 1);
	} else {
	    intro_mode = 0;
	}
	break;
          
    case INTRONEXT:
	wm_cd_pause();
	currenttrack = cur_track;
	currenttrack++;
	if (cur_track +1 > cur_ntracks ) currenttrack = 1;
	else currenttrack = cur_track + 1;
	cur_pos_rel = 0;
	wm_cd_play(currenttrack, 0, cur_ntracks + 1);
	break;
          
    case LOCACCESS:
	if ( (loop_1 > 0) && (loop_start_track > 0) ) {
	    intro_mode = 0;
	    loop_mode = 0;
	    currenttrack = loop_start_track;
	    wm_cd_stop();
	    wm_cd_play(loop_start_track, loop_1, cur_ntracks + 1);
	    wanna_play = TRUE;
	} else {
	    anomalie = 1;
	}
	break;
    }
}

char *cd_control_version(void)
{
        char *s = NULL;

        wm_strmcat(&s, CD_C_VERSION);
        return s;
}
