/* ===========================================================================
 * AScd: the AfterStep and WindowMaker CD player
 * gui_wings.c: window handling using WINGs toolkit
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

#include <dirent.h>
#include <WINGs.h>
#include "ext.h"

/*int old_track = 0;*/

void close_win(WMWidget *self, void *data) {
    WMCloseWindow(win);
    en_vue = FALSE;
    muted_volume = atoi(WMGetTextFieldText(b_mutedvol));
    min_volume = atoi(WMGetTextFieldText(b_minvol));
    max_volume = atoi(WMGetTextFieldText(b_maxvol));
    cue_time = atoi(WMGetTextFieldText(b_cuetime));
    fade_step = atoi(WMGetTextFieldText(b_fadestep));
}

void close_aboutwin(WMWidget *self, void *data) {
    WMCloseWindow(aboutwin);
    about_en_vue = FALSE;
}

void close_dbwin(WMWidget *self, void *data) {
    char txt[127];
    char txt2 [127];

    WMCloseWindow(dbwin);
    strcpy(cd->artist, WMGetTextFieldText(db_artist));
    strcpy(cd->cdname, WMGetTextFieldText(db_title));
    if (strlen(WMGetTextFieldText(db_track)) > 0) {
	cd->trk[db_curtrack - 1].songname = malloc(strlen(WMGetTextFieldText(db_track)) + 1);
	strcpy(cd->trk[db_curtrack - 1].songname, WMGetTextFieldText(db_track));
    }

    save();
    load();

    if (cd->artist != NULL) {
	strcpy(txt, cd->artist);
	if (strcmp(txt, "") == 0) strcpy(txt, "unknown artist");
    } else {
	strcpy(txt, "unknown artist");
    }

    strcat(txt, "\n");

    if (cd->cdname != NULL) {
	strcpy(txt2, cd->cdname);
	if (strcmp(txt2, "") == 0) strcpy(txt2, "unknown album");
    } else {
	strcpy(txt2, "unknown album");
    }

    strcat(txt, txt2);
    WMSetButtonText(b_title, txt);

    db_en_vue = FALSE;
}

void quit_ascd(WMWidget *self, void *data) {
    exit(0);
}

void save_cb(WMWidget *self, void *data) {
    muted_volume = atoi(WMGetTextFieldText(b_mutedvol));
    min_volume = atoi(WMGetTextFieldText(b_minvol));
    max_volume = atoi(WMGetTextFieldText(b_maxvol));
    cue_time = atoi(WMGetTextFieldText(b_cuetime));
    fade_step = atoi(WMGetTextFieldText(b_fadestep));
    cd_device = WMGetTextFieldText(b_device);
    save_rc_file();
}

void track_select(WMWidget *self, void *data) {
    show_db_pos = 0;
    direct_track =  WMGetPopUpButtonSelectedItem(self) + 1;
    cd_control(DIRECTTRACK);
    old_track = 0;
    if (cur_cdmode == WM_CDM_STOPPED) cd_control(PLAY);
}

void theme_pick(WMWidget *self, void *data) {
    strcpy(theme, WMGetPopUpButtonItem(self, WMGetPopUpButtonSelectedItem(self)));
    fak_load_theme(theme, TRUE);
    fak_maskset();
}

void help_cb(WMWidget *self, void *data) {
    quick_reference(0);
}

void mode1_cb(WMWidget *self, void *data) {
    if (WMGetButtonSelected(b_mode1)) {
	WMSetButtonSelected((b_mode2), FALSE);
    } else {
	WMSetButtonSelected((b_mode2), TRUE);
    }

    if (WMGetButtonSelected(b_mode1)) {
	if (WMGetButtonSelected(b_mode3)) time_mode = 0;
	else time_mode = 1;
    } else {
	if (WMGetButtonSelected(b_mode3)) time_mode = 2;
	else time_mode = 3;
    }
}

void mode2_cb(WMWidget *self, void *data) {
    if (WMGetButtonSelected(b_mode2)) {
	WMSetButtonSelected((b_mode1), FALSE);
    } else {
	WMSetButtonSelected((b_mode1), TRUE);
    }

    if (WMGetButtonSelected(b_mode1)) {
	if (WMGetButtonSelected(b_mode3)) time_mode = 0;
	else time_mode = 1;
    } else {
	if (WMGetButtonSelected(b_mode3)) time_mode = 2;
	else time_mode = 3;
    }
}

void mode3_cb(WMWidget *self, void *data) {
    if (WMGetButtonSelected(b_mode3)) {
	WMSetButtonSelected((b_mode4), FALSE);
    } else {
	WMSetButtonSelected((b_mode4), TRUE);
    }

    if (WMGetButtonSelected(b_mode1)) {
	if (WMGetButtonSelected(b_mode3)) time_mode = 0;
	else time_mode = 1;
    } else {
	if (WMGetButtonSelected(b_mode3)) time_mode = 2;
	else time_mode = 3;
    }
}

void mode4_cb(WMWidget *self, void *data) {
    if (WMGetButtonSelected(b_mode4)) {
	WMSetButtonSelected((b_mode3), FALSE);
    } else {
	WMSetButtonSelected((b_mode3), TRUE);
    }

    if (WMGetButtonSelected(b_mode1)) {
	if (WMGetButtonSelected(b_mode3)) time_mode = 0;
	else time_mode = 1;
    } else {
	if (WMGetButtonSelected(b_mode3)) time_mode = 2;
	else time_mode = 3;
    }
}

void autoplay_cb(WMWidget *self, void *data) {
    if (autoplay) autoplay = 0;
    else autoplay = 1;

    if (WMGetButtonSelected(b_mode1)) {
	if (WMGetButtonSelected(b_mode3)) time_mode = 0;
	else time_mode = 1;
    } else {
	if (WMGetButtonSelected(b_mode3)) time_mode = 2;
	else time_mode = 3;
    }
}

void autorepeat_cb(WMWidget *self, void *data) {
    if (autorepeat) autorepeat = 0;
    else autorepeat = 1;
}

void ignoreavoid_cb(WMWidget *self, void *data) {
    if (ignore_avoid) ignore_avoid = 0;
    else ignore_avoid = 1;
}

void upper_cb(WMWidget *self, void *data) {
    if (force_upper) force_upper = 0;
    else force_upper = 1;
}

void scroll_cb(WMWidget *self, void *data) {
    if (show_db) show_db = 0;
    else show_db = 1;
}

void artist_cb(WMWidget *self, void *data) {
    if (show_artist) show_artist = 0;
    else show_artist = 1;
}

void volume_cb(WMWidget *self, void *data) {
    if (cur_cdmode != WM_CDM_EJECTED) {
	volume = WMGetSliderValue(self);
	cd_volume(volume, 10, max_volume);
	redraw = TRUE;
    }
}

/* --------------------------------------------------- */

void create_about_window(WMScreen *scr) {
    char txt[256];
    WMWidget *lb1, *lb2, *lb3, *lb4, *lb5, *lb6;
    WMFont *font;

    aboutwin = WMCreateWindow(scr, "AScd-about");
    WMSetWindowCloseAction(aboutwin, close_aboutwin, NULL);
    WMSetWindowTitle(aboutwin, "AScd Info");
    WMResizeWidget(aboutwin, 350, 170);

    lb1 = WMCreateLabel(aboutwin);
    WMResizeWidget(lb1, 35, 20);
    WMMoveWidget(lb1, 10, 10);
    font = WMBoldSystemFontOfSize(scr, 12);
    if (font) {
	WMSetLabelFont(lb1, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(lb1, "AScd");

    lb2 = WMCreateLabel(aboutwin);
    WMResizeWidget(lb2, 200, 20);
    WMMoveWidget(lb2, 50, 10);
    sprintf(txt, "release %s", VERSION);
    WMSetLabelText(lb2, txt);

    lb3 = WMCreateLabel(aboutwin);
    WMResizeWidget(lb3, 280, 40);
    WMMoveWidget(lb3, 50, 30);
    font = WMSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(lb3, font);
	WMReleaseFont(font);
    }
    sprintf(txt, "(c) 1996-1999 Denis Bourez and Rob Malda\nContact: denis@rsn.fdn.fr\nhttp://worldserver.oleane.com/rsn/ascd-en.html");
    WMSetLabelText(lb3, txt);

    lb5 = WMCreateLabel(aboutwin);
    WMResizeWidget(lb5, 60, 20);
    WMMoveWidget(lb5, 10, 70);
    font = WMBoldSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(lb5, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(lb5, "Ctrl:");

    lb4 = WMCreateLabel(aboutwin);
    WMResizeWidget(lb4, 260, 20);
    WMMoveWidget(lb4, 50, 70);
    font = WMSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(lb4, font);
	WMReleaseFont(font);
    }
    sprintf(txt, "CD Control %s + %s", cd_control_version(),  wm_libver_string());
    WMSetLabelText(lb4, txt);

    lb6 = WMCreateLabel(aboutwin);
    WMResizeWidget(lb6, 60, 20);
    WMMoveWidget(lb6, 10, 85);
    font = WMBoldSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(lb6, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(lb6, "Theme:");

    about_th1 = WMCreateLabel(aboutwin);
    WMResizeWidget(about_th1, 280, 12);
    WMMoveWidget(about_th1, 50, 89);
    font = WMSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(about_th1, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(about_th1, "A");

    about_th2 = WMCreateLabel(aboutwin);
    WMResizeWidget(about_th2, 280, 12);
    WMMoveWidget(about_th2, 50, 101);
    font = WMSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(about_th2, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(about_th2, "B");

    about_th3 = WMCreateLabel(aboutwin);
    WMResizeWidget(about_th3, 280, 12);
    WMMoveWidget(about_th3, 50, 113);
    font = WMSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(about_th3, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(about_th3, "C");

    about_th4 = WMCreateLabel(aboutwin);
    WMResizeWidget(about_th4, 280, 12);
    WMMoveWidget(about_th4, 50, 125);
    font = WMSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(about_th4, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(about_th4, "D");

    about_th5 = WMCreateLabel(aboutwin);
    WMResizeWidget(about_th5, 280, 12);
    WMMoveWidget(about_th5, 50, 137);
    font = WMSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(about_th5, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(about_th5, "E");

    about_th6 = WMCreateLabel(aboutwin);
    WMResizeWidget(about_th6, 280, 12);
    WMMoveWidget(about_th6, 50, 149);
    font = WMSystemFontOfSize(scr, 10);
    if (font) {
	WMSetLabelFont(about_th6, font);
	WMReleaseFont(font);
    }
    WMSetLabelText(about_th6, "F");



    WMRealizeWidget(aboutwin);
}

void about_window(WMScreen *scr) {
    char txt[256];

    if (! about_en_vue) {
	sprintf(txt, "%s %s", th_name, th_release);
	WMSetLabelText(about_th1, txt);
	WMSetLabelText(about_th2, th_author);
	WMSetLabelText(about_th3, th_email);
	WMSetLabelText(about_th4, th_url);
	WMSetLabelText(about_th5, th_comment);
	sprintf(txt, "Faktory: %dx%d (panels: %d Buttons: %d)", backXPM.attributes.width, backXPM.attributes.height, panels, but_max);
	WMSetLabelText(about_th6, txt);

	WMRealizeWidget(aboutwin);
	WMMapSubwidgets(aboutwin);
	WMMapWidget(aboutwin);
	about_en_vue = TRUE;
    } else {
	WMCloseWindow(aboutwin);
	about_en_vue = FALSE;
    }
}

/* --------------------------------------------------- */

void db_prev_cb(WMWidget *self, void *data) {
    char txt[256];

    if (cur_cdmode == WM_CDM_EJECTED) return;

    strcpy(cd->artist, WMGetTextFieldText(db_artist));
    strcpy(cd->cdname, WMGetTextFieldText(db_title));
    if (strlen(WMGetTextFieldText(db_track)) > 0) {
	cd->trk[db_curtrack - 1].songname = malloc(strlen(WMGetTextFieldText(db_track)) + 1);
	strcpy(cd->trk[db_curtrack - 1].songname, WMGetTextFieldText(db_track));
    }
    save();
    load();

    if (db_curtrack == 1) db_curtrack = cur_ntracks;
    else db_curtrack--;
    sprintf(txt, "Tk %d:", db_curtrack);
    WMSetLabelText(db_tlabel, txt);

    if (cd->trk[db_curtrack - 1].songname != NULL) {
	sprintf(txt,  cd->trk[db_curtrack - 1].songname);
    } else {
	strcpy(txt, "");
    }
    WMSetTextFieldText(db_track, txt);

    if (cd->trk[db_curtrack - 1].avoid) {
	WMSetButtonSelected(db_avoid, TRUE);
    } else {
	WMSetButtonSelected(db_avoid, FALSE);
    }

    WMRealizeWidget(dbwin);
}

void db_next_cb(WMWidget *self, void *data) {
    char txt[256];

    if (cur_cdmode == WM_CDM_EJECTED) return;

    strcpy(cd->artist, WMGetTextFieldText(db_artist));
    strcpy(cd->cdname, WMGetTextFieldText(db_title));
    if (strlen(WMGetTextFieldText(db_track)) > 0) {
	cd->trk[db_curtrack - 1].songname = malloc(strlen(WMGetTextFieldText(db_track)) + 1);
	strcpy(cd->trk[db_curtrack - 1].songname, WMGetTextFieldText(db_track));
    }
    save();
    load();

    if (db_curtrack == cur_ntracks) db_curtrack = 1;
    else db_curtrack++;
    sprintf(txt, "Tk %d:", db_curtrack);
    WMSetLabelText(db_tlabel, txt);

    if (cd->trk[db_curtrack - 1].songname != NULL) {
	sprintf(txt,  cd->trk[db_curtrack - 1].songname);
    } else {
	strcpy(txt, "");
    }
    WMSetTextFieldText(db_track, txt);

    if (cd->trk[db_curtrack - 1].avoid) {
	WMSetButtonSelected(db_avoid, TRUE);
    } else {
	WMSetButtonSelected(db_avoid, FALSE);
    }

    WMRealizeWidget(dbwin);
}

void db_avoid_cb(WMWidget *self, void *data) {
    if (cur_cdmode == WM_CDM_EJECTED) return;
    if (cd->trk[db_curtrack - 1].avoid) {
	cd->trk[db_curtrack - 1].avoid = 0;
    } else {
	cd->trk[db_curtrack - 1].avoid = 1;
    }
    save();
    load();
}

void db_playauto_cb(WMWidget *self, void *data) {
    if (cur_cdmode == WM_CDM_EJECTED) return;
    if (cd->autoplay) {
	cd->autoplay = 0;
    } else {
	cd->autoplay = 1;
    }
    save();
    load();
}

void db_volume_cb(WMWidget *self, void *data) {
    if (cur_cdmode != WM_CDM_EJECTED) {
	cd->volume = WMGetSliderValue(self);
	save();
	load();
	volume = cd->volume;
	cd_volume(volume, 10, max_volume);
	redraw = TRUE;
	WMSetSliderMinValue(b_volume, min_volume);
	WMSetSliderMaxValue(b_volume, max_volume);
	WMSetSliderValue(b_volume, volume);
    }
}

void create_db_window(WMScreen *scr) {
    char txt[256];
    WMWidget *fr, *fr2;
    WMWidget *lb1, *lb2, *lb3;

    dbwin = WMCreateWindow(scr, "AScd-db");
    WMSetWindowCloseAction(dbwin, close_dbwin, NULL);
    WMSetWindowTitle(dbwin, "AScd database edition");
    WMResizeWidget(dbwin, 350, 220);


    fr = WMCreateFrame(dbwin);
    WMMoveWidget(fr, 10, 05);
    WMResizeWidget(fr, 330, 105);
    WMSetFrameTitle(fr, "CD info:");

    lb1 = WMCreateLabel(dbwin);
    WMResizeWidget(lb1, 40, 20);
    WMMoveWidget(lb1, 16, 20);
    WMSetLabelText(lb1, "Artist:");

    db_artist = WMCreateTextField(dbwin);
    WMMoveWidget(db_artist, 70, 20);
    WMResizeWidget(db_artist, 255, 20);

    lb2 = WMCreateLabel(dbwin);
    WMResizeWidget(lb2, 40, 20);
    WMMoveWidget(lb2, 16, 40);
    WMSetLabelText(lb2, "Title:");

    db_title = WMCreateTextField(dbwin);
    WMMoveWidget(db_title, 70, 40);
    WMResizeWidget(db_title, 255, 20);

    db_playauto = WMCreateButton(dbwin, WBTSwitch);
    WMMoveWidget(db_playauto, 70, 65);
    WMResizeWidget(db_playauto, 255, 20);
    WMSetButtonText(db_playauto, "play this CD automatically (WorkMan)");
    WMSetButtonAction(db_playauto, db_playauto_cb, NULL);

    lb3 = WMCreateLabel(dbwin);
    WMResizeWidget(lb3, 54, 20);
    WMMoveWidget(lb3, 16, 85);
    WMSetLabelText(lb3, "Volume:");

    db_volume = WMCreateSlider(dbwin);
    WMResizeWidget(db_volume, 255, 15);
    WMMoveWidget(db_volume, 70, 88);
    WMSetSliderKnobThickness(db_volume, 22);
    WMSetSliderAction(db_volume, db_volume_cb, NULL);

    /* --- */

    fr2 = WMCreateFrame(dbwin);
    WMMoveWidget(fr2, 10, 115);
    WMResizeWidget(fr2, 330, 65);
    WMSetFrameTitle(fr2, "Track info:");

    db_tlabel = WMCreateLabel(dbwin);
    WMResizeWidget(db_tlabel, 54, 20);
    WMMoveWidget(db_tlabel, 16, 130);
    sprintf(txt, "Track %d:", db_curtrack);
    WMSetLabelText(db_tlabel, txt);

    db_track = WMCreateTextField(dbwin);
    WMMoveWidget(db_track, 70, 130);
    WMResizeWidget(db_track, 255, 20);

    db_avoid = WMCreateButton(dbwin, WBTSwitch);
    WMMoveWidget(db_avoid, 70, 152);
    WMResizeWidget(db_avoid, 235, 20);
    WMSetButtonText(db_avoid, "don't play this track");
    WMSetButtonAction(db_avoid, db_avoid_cb, NULL);

    /* --- */

    db_prev = WMCreateButton(dbwin, 0);
    WMResizeWidget(db_prev, 110, 25);
    WMMoveWidget(db_prev, 10, 190);
    WMSetButtonText(db_prev, "Previous Track");
    WMSetButtonAction(db_prev, db_prev_cb, NULL);

    db_next = WMCreateButton(dbwin, 0);
    WMResizeWidget(db_next, 110, 25);
    WMMoveWidget(db_next, 125, 190);
    WMSetButtonText(db_next, "Next Track");
    WMSetButtonAction(db_next, db_next_cb, NULL);

    db_close = WMCreateButton(dbwin, 0);
    WMResizeWidget(db_close, 55, 25);
    WMMoveWidget(db_close, 285, 190);
    WMSetButtonText(db_close, "Close");
    WMSetButtonAction(db_close, close_dbwin, NULL);

    WMRealizeWidget(dbwin);
}

void db_window(WMScreen *scr) {
    char txt[256];

    if (! db_en_vue) {
	if (cur_cdmode == WM_CDM_EJECTED) return;

	/* 0.13 fix */
	if (db_curtrack < 1) db_curtrack = 1;

	load();
	if (cd->artist != NULL) strcpy(txt, cd->artist);
	else strcpy(txt, "");
	WMSetTextFieldText(db_artist, txt);

	if (cd->cdname != NULL) strcpy(txt, cd->cdname);
	else strcpy(txt, "");
	WMSetTextFieldText(db_title, txt);

	if (cd->autoplay) {
	    WMSetButtonSelected(db_playauto, TRUE);
	} else {
	    WMSetButtonSelected(db_playauto, TRUE);
	}

	if (cd->trk[db_curtrack - 1].songname != NULL) {
	    sprintf(txt,  cd->trk[db_curtrack - 1].songname);
	} else {
	    strcpy(txt, "");
	}
	WMSetTextFieldText(db_track, txt);

	if (cd->trk[db_curtrack - 1].avoid) {
	    WMSetButtonSelected(db_avoid, TRUE);
	} else {
	    WMSetButtonSelected(db_avoid, FALSE);
	}

	WMSetSliderMinValue(db_volume, min_volume);
	WMSetSliderMaxValue(db_volume, max_volume);
	if (cd->volume > 0) WMSetSliderValue(db_volume, cd->volume);
	else WMSetSliderValue(db_volume, max_volume);

	WMRealizeWidget(dbwin);
	WMMapSubwidgets(dbwin);
	WMMapWidget(dbwin);
	db_en_vue = TRUE;
    } else {
	WMCloseWindow(dbwin);
	db_en_vue = FALSE;
    }
}

void db_cb(WMWidget *self, void *data) {
    char txt[127];
    sprintf(txt, "Tk %d:", db_curtrack);
    WMSetLabelText(db_tlabel, txt);
    /*WMRealizeWidget(dbwin);*/
    db_window(scr);
}

void about_cb(WMWidget *self, void *data) {
    about_window(scr);
}

void create_big_window(WMScreen *scr) {
    WMWidget *fr, *fr3, *fr4, *fr5;
    WMWidget *lb1, *lb2, *lb3, *lb4, *lb6;

    win = WMCreateWindow(scr, "AScd");
    WMSetWindowCloseAction(win, close_win, NULL);
    WMSetWindowTitle(win, "AScd");
    WMResizeWidget(win, 320, 370);

    b_title = WMCreateButton(win, 0);
    WMResizeWidget(b_title, 300, 40);
    WMMoveWidget(b_title, 10, 5);
    WMSetButtonText(b_title, "1\n2");
    WMSetButtonAction(b_title, db_cb, NULL);

    pop = WMCreatePopUpButton(win);
    WMResizeWidget(pop, 300, 20);
    WMMoveWidget(pop, 10, 50);
    WMSetPopUpButtonPullsDown(pop, True);
    WMSetPopUpButtonText(pop, "Tracks");
    WMAddPopUpButtonItem(pop, "1");
    WMSetPopUpButtonAction(pop, track_select, NULL);

    fr = WMCreateFrame(win);
    WMMoveWidget(fr, 10, 75);
    WMResizeWidget(fr, 110, 140);
    WMSetFrameTitle(fr, "Playback:");

    b_autoplay = WMCreateButton(win, WBTSwitch);
    WMMoveWidget(b_autoplay, 20, 95);
    WMResizeWidget(b_autoplay, 90, 20);
    WMSetButtonText(b_autoplay, "autoplay");
    WMSetButtonAction(b_autoplay, autoplay_cb, NULL);

    b_autorepeat = WMCreateButton(win, WBTSwitch);
    WMMoveWidget(b_autorepeat, 20, 115);
    WMResizeWidget(b_autorepeat, 90, 20);
    WMSetButtonText(b_autorepeat, "autorepeat");
    WMSetButtonAction(b_autorepeat, autorepeat_cb, NULL);

    b_ignoreavoid = WMCreateButton(win, WBTSwitch);
    WMMoveWidget(b_ignoreavoid, 20, 135);
    WMResizeWidget(b_ignoreavoid, 90, 20);
    WMSetButtonText(b_ignoreavoid, "apply skips");
    WMSetButtonAction(b_ignoreavoid, ignoreavoid_cb, NULL);

    lb6 = WMCreateLabel(win);
    WMResizeWidget(lb6, 80, 20);
    WMMoveWidget(lb6, 15, 165);
    WMSetLabelText(lb6, "CD Device:");

    b_device = WMCreateTextField(win);
    WMResizeWidget(b_device, 85, 20);
    WMMoveWidget(b_device, 20, 185);


    fr3 = WMCreateFrame(win);
    WMMoveWidget(fr3, 130, 75);
    WMResizeWidget(fr3, 180, 140);
    WMSetFrameTitle(fr3, "Display:");

    b_scroll = WMCreateButton(win, WBTSwitch);
    WMMoveWidget(b_scroll, 140, 95);
    WMResizeWidget(b_scroll, 130, 20);
    WMSetButtonText(b_scroll, "scroll song names");
    WMSetButtonAction(b_scroll, scroll_cb, NULL);

    b_artist = WMCreateButton(win, WBTSwitch);
    WMMoveWidget(b_artist, 140, 115);
    WMResizeWidget(b_artist, 130, 20);
    WMSetButtonText(b_artist, "add artist name");
    WMSetButtonAction(b_artist, artist_cb, NULL);

    b_upper = WMCreateButton(win, WBTSwitch);
    WMMoveWidget(b_upper, 140, 135);
    WMResizeWidget(b_upper, 130, 20);
    WMSetButtonText(b_upper, "force uppercase");
    WMSetButtonAction(b_upper, upper_cb, NULL);

    lb4 = WMCreateLabel(win);
    WMResizeWidget(lb4, 80, 20);
    WMMoveWidget(lb4, 135, 165);
    WMSetLabelText(lb4, "Theme:");

    pop2 = WMCreatePopUpButton(win);
    WMResizeWidget(pop2, 160, 20);
    WMMoveWidget(pop2, 140, 185);
    WMSetPopUpButtonText(pop2, theme);
    WMAddPopUpButtonItem(pop2, "1");
    WMSetPopUpButtonAction(pop2, theme_pick, NULL);

    b_help = WMCreateButton(win, 0);
    WMResizeWidget(b_help, 80, 18);
    WMMoveWidget(b_help, 220, 165);
    WMSetButtonText(b_help, "Quick Ref.");
    WMSetButtonAction(b_help, help_cb, NULL);

    /* the volume */

    fr4 = WMCreateFrame(win);
    WMMoveWidget(fr4, 10, 220);
    WMResizeWidget(fr4, 300, 80);
    WMSetFrameTitle(fr4, "Volume & Misc:");

    b_minvol = WMCreateTextField(win);
    WMResizeWidget(b_minvol, 30, 20);
    WMMoveWidget(b_minvol, 20, 240);

    b_volume = WMCreateSlider(win);
    WMResizeWidget(b_volume, 220, 20);
    WMMoveWidget(b_volume, 50, 240);
    WMSetSliderKnobThickness(b_volume, 22);
    WMSetSliderAction(b_volume, volume_cb, NULL);

    b_maxvol = WMCreateTextField(win);
    WMResizeWidget(b_maxvol, 30, 20);
    WMMoveWidget(b_maxvol, 270, 240);

    lb1 = WMCreateLabel(win);
    WMResizeWidget(lb1, 40, 20);
    WMMoveWidget(lb1, 15, 270);
    WMSetLabelText(lb1, "Muted");

    b_mutedvol = WMCreateTextField(win);
    WMResizeWidget(b_mutedvol, 30, 20);
    WMMoveWidget(b_mutedvol, 65, 270);

    lb2 = WMCreateLabel(win);
    WMResizeWidget(lb2, 70, 20);
    WMMoveWidget(lb2, 105, 270);
    WMSetLabelText(lb2, "Cue Time");

    b_cuetime = WMCreateTextField(win);
    WMResizeWidget(b_cuetime, 25, 20);
    WMMoveWidget(b_cuetime, 180, 270);

    lb3 = WMCreateLabel(win);
    WMResizeWidget(lb3, 33, 20);
    WMMoveWidget(lb3, 225, 270);
    WMSetLabelText(lb3, "Fade");

    b_fadestep = WMCreateTextField(win);
    WMResizeWidget(b_fadestep, 25, 20);
    WMMoveWidget(b_fadestep, 275, 270);

    /* the counter modes */

    fr5 = WMCreateFrame(win);
    WMMoveWidget(fr5, 10, 305);
    WMResizeWidget(fr5, 190, 60);
    WMSetFrameTitle(fr5, "Counter Mode:");

    b_mode1 = WMCreateButton(win, WBTRadio);
    WMMoveWidget(b_mode1, 20, 320);
    WMResizeWidget(b_mode1, 55, 20);
    WMSetButtonText(b_mode1, "Track");
    WMSetButtonAction(b_mode1, mode1_cb, NULL);

    b_mode2 = WMCreateButton(win, WBTRadio);
    WMMoveWidget(b_mode2, 20, 340);
    WMResizeWidget(b_mode2, 55, 20);
    WMSetButtonText(b_mode2, "CD");
    WMSetButtonAction(b_mode2, mode2_cb, NULL);

    b_mode3 = WMCreateButton(win, WBTRadio);
    WMMoveWidget(b_mode3, 90, 320);
    WMResizeWidget(b_mode3, 85, 20);
    WMSetButtonText(b_mode3, "Elapsed");
    WMSetButtonAction(b_mode3, mode3_cb, NULL);

    b_mode4 = WMCreateButton(win, WBTRadio);
    WMMoveWidget(b_mode4, 90, 340);
    WMResizeWidget(b_mode4, 85, 20);
    WMSetButtonText(b_mode4, "Remaining");
    WMSetButtonAction(b_mode4, mode4_cb, NULL);

    /* the global buttons */

    b_save = WMCreateButton(win, 0);
    WMResizeWidget(b_save, 50, 25);
    WMMoveWidget(b_save, 205, 340);
    WMSetButtonText(b_save, "Save");
    WMSetButtonAction(b_save, save_cb, NULL);

    b_about = WMCreateButton(win, 0);
    WMResizeWidget(b_about, 50, 25);
    WMMoveWidget(b_about, 205, 310);
    WMSetButtonText(b_about, "Info");
    WMSetButtonAction(b_about, about_cb, NULL);

    b_save = WMCreateButton(win, 0);
    WMResizeWidget(b_save, 50, 25);
    WMMoveWidget(b_save, 260, 310);
    WMSetButtonText(b_save, "Quit");
    WMSetButtonAction(b_save, quit_ascd, NULL);

    b_close = WMCreateButton(win, 0);
    WMResizeWidget(b_close, 50, 25);
    WMMoveWidget(b_close, 260, 340);
    WMSetButtonText(b_close, "Close");
    WMSetButtonAction(b_close, close_win, NULL);

    /* finished, draw everything */

    WMRealizeWidget(win);

    /*
    WMMapSubwidgets(win);
    WMMapWidget(win);
    */
}

void update_track() {
    int i;
    char txt[127];
    char txt2[127];

    if (en_vue) {
	if (cur_cdmode != WM_CDM_EJECTED) {

	    if (cd->artist != NULL) {
		strcpy(txt, cd->artist);
		if (strcmp(txt, "") == 0) strcpy(txt, "unknown artist");
	    } else {
		strcpy(txt, "unknown artist");
	    }

	    strcat(txt, "\n");

	    if (cd->cdname != NULL) {
		strcpy(txt2, cd->cdname);
		if (strcmp(txt2, "") == 0) strcpy(txt2, "unknown album");
	    } else {
		strcpy(txt2, "unknown album");
	    }

	    strcat(txt, txt2);
	    WMSetButtonText(b_title, txt);

	    for (i = WMGetPopUpButtonNumberOfItems(pop); i >= 0; i --) {
		WMRemovePopUpButtonItem(pop, i);
	    }

	    for (i = 0; i < cur_ntracks; i++) {
		sprintf(txt, "%2d ", i + 1);
		if (cd->trk[i].songname != NULL) strcat(txt,  cd->trk[i].songname);
		WMAddPopUpButtonItem(pop, txt);
	    }

	    pistes = cur_ntracks;

	    if (cur_track >= 1) {
		if (cd->trk[cur_track - 1].songname != NULL) {
		    sprintf(txt, "%s", cd->trk[cur_track - 1].songname);
		    if (strcmp(txt, "") == 0) sprintf(txt, "Track %d", cur_track);
		    WMSetPopUpButtonText(pop, txt);
		} else {
		    sprintf(txt, "Track %d", cur_track);
		    WMSetPopUpButtonText(pop, txt);
		}
	    } else {
		WMSetPopUpButtonText(pop, "Track 1");
	    }

	} else {
	    WMSetButtonText(b_title, "No CD in drive\nor the tray is not loaded");
	}
    }
}

void big_window(WMScreen *scr) {
    int i;
    char txt[256];
    DIR *dir_fd;
    struct dirent *dir_pt;

    if (! en_vue) {

	/*if (cur_cdmode == WM_CDM_EJECTED) return;*/

	en_vue = TRUE;

	sprintf(txt, "%d", cue_time);
	WMSetTextFieldText(b_cuetime, txt);
	sprintf(txt, "%d", fade_step);
	WMSetTextFieldText(b_fadestep, txt);
	sprintf(txt, "%d", min_volume);
	WMSetTextFieldText(b_minvol, txt);
	sprintf(txt, "%d", max_volume);
	WMSetTextFieldText(b_maxvol, txt);
	sprintf(txt, "%d", muted_volume);
	WMSetTextFieldText(b_mutedvol, txt);

	WMSetTextFieldText(b_device, cd_device);

	WMSetSliderMinValue(b_volume, min_volume);
	WMSetSliderMaxValue(b_volume, max_volume);
	WMSetSliderValue(b_volume, volume);

	/* remove everything in the two popups */

	for (i = WMGetPopUpButtonNumberOfItems(pop); i >= 0; i --) {
	    WMRemovePopUpButtonItem(pop, i);
	}

	for (i = WMGetPopUpButtonNumberOfItems(pop2); i >= 0; i --) {
	    WMRemovePopUpButtonItem(pop2, i);
	}

	/* update the database stuff. AScd 0.13: I deleted all this
	   stuff from this function as update_track() do the same! */

	update_track();

	/* make the theme select menu */

	sprintf(txt, "%s/Themes", THDIR);
	if ((dir_fd = opendir(txt)) != NULL) {
	    while((dir_pt = readdir(dir_fd)) != NULL) {
		if (dir_pt->d_name[0] != '.') {
		    WMAddPopUpButtonItem(pop2, dir_pt->d_name);
		}
	    }
	    closedir(dir_fd);
	}
	WMSetPopUpButtonText(pop2, theme);

	/* several options toggles */

	if (autoplay) {
	    WMSetButtonSelected(b_autoplay, TRUE);
	} else {
	    WMSetButtonSelected(b_autoplay, FALSE);
	}

	if (autorepeat) {
	    WMSetButtonSelected(b_autorepeat, TRUE);
	} else {
	    WMSetButtonSelected(b_autorepeat, FALSE);
	}

	if (show_db) {
	    WMSetButtonSelected(b_scroll, TRUE);
	} else {
	    WMSetButtonSelected(b_scroll, FALSE);
	}

	if (show_artist) {
	    WMSetButtonSelected(b_artist, TRUE);
	} else {
	    WMSetButtonSelected(b_artist, FALSE);
	}

	if (ignore_avoid) {
	    WMSetButtonSelected(b_ignoreavoid, FALSE);
	} else {
	    WMSetButtonSelected(b_ignoreavoid, TRUE);
	}

	if (force_upper) {
	    WMSetButtonSelected(b_upper, TRUE);
	} else {
	    WMSetButtonSelected(b_upper, FALSE);
	}

	/* finally, the counter mode radio buttons */

	switch(time_mode) {
	case 0:
	    WMSetButtonSelected((b_mode1), TRUE);
	    WMSetButtonSelected((b_mode2), FALSE);
	    WMSetButtonSelected((b_mode3), TRUE);
	    WMSetButtonSelected((b_mode4), FALSE);
	    break;
	case 1:
	    WMSetButtonSelected((b_mode1), TRUE);
	    WMSetButtonSelected((b_mode2), FALSE);
	    WMSetButtonSelected((b_mode4), TRUE);
	    WMSetButtonSelected((b_mode3), FALSE);
	    break;
	case 2:
	    WMSetButtonSelected((b_mode2), TRUE);
	    WMSetButtonSelected((b_mode1), FALSE);
	    WMSetButtonSelected((b_mode3), TRUE);
	    WMSetButtonSelected((b_mode4), FALSE);
	    break;
	default:
	    WMSetButtonSelected((b_mode2), TRUE);
	    WMSetButtonSelected((b_mode1), FALSE);
	    WMSetButtonSelected((b_mode4), TRUE);
	    WMSetButtonSelected((b_mode3), FALSE);
	    break;
	}

	WMRealizeWidget(win);
	WMMapSubwidgets(win);
	WMMapWidget(win);
    } else {
	WMCloseWindow(win);
	muted_volume = atoi(WMGetTextFieldText(b_mutedvol));
	min_volume = atoi(WMGetTextFieldText(b_minvol));
	max_volume = atoi(WMGetTextFieldText(b_maxvol));
	cue_time = atoi(WMGetTextFieldText(b_cuetime));
	fade_step = atoi(WMGetTextFieldText(b_fadestep));
	en_vue = FALSE;
    }
}
