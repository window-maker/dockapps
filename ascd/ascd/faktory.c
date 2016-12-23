/* ===========================================================================
 * AScd: faktory.c
 * The theme handling functions
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

#define MAX_LINE_NAME 16

void tes_sncpy(char *out, char *in, int n)
{
    register int i;
    for (i=0; i<=n-1; i++) out[i] = in[i];
    out[i] = '\0';
}

char *tes_xgets(char *chaine, int nb, FILE *fichier)
{
    char *r;
    r = fgets(chaine, nb, fichier);
    if (feof(fichier) == 0) chaine[strlen(chaine) - 1] = 0;
    return r;
}

int fak_parse_line(char *ligne, char *key, char *arguments)
{
    unsigned int pos = 0;

    if ((strlen(ligne) > 0) && (ligne[0] != '#')) {
	while ((ligne[pos] != ' ') && (ligne[pos] != 9)) pos++;
	tes_sncpy(key, ligne, pos);
	while (((ligne[pos] == ' ') || (ligne[pos] == 9)) && (pos < strlen(ligne))) pos++;
	if (pos < strlen(ligne)) strcpy(arguments, ligne + pos);
	else strcpy(arguments, "");
	if (debug > 2) fprintf(stderr,"++ input: [%s]\n   key:   [%s]\n   args:  [%s]\n", ligne, key, arguments);
	return TRUE;
    } else {
	strcpy(key, "");
	strcpy(arguments, "");
	if (debug > 2) fprintf(stderr,"++ input: [%s]\n   key:   [%s]\n   args:  [%s]\n", ligne, key, arguments);
	return FALSE;
    }
}

int fak_use_alt(int i)
{
    int use_alt = 0;
    switch (thdata[i].left) {
    case FAK_CD_PLAY:
	if (cur_cdmode == WM_CDM_PLAYING) use_alt = 1;
	break;
    case FAK_CD_PAUSE:
	if (cur_cdmode == WM_CDM_PAUSED) use_alt = 1;
	break;
    case FAK_CD_STOP:
	if (cur_cdmode == WM_CDM_STOPPED) use_alt = 1;
	break;
    case FAK_CD_EJECT:
	if (cur_cdmode == WM_CDM_EJECTED) use_alt = 1;
	break;
    case FAK_CD_STOPEJECT:
	if (cur_cdmode == WM_CDM_STOPPED) use_alt = 1;
	break;
    case FAK_CD_EJECTQUIT:
	if ((cur_cdmode == WM_CDM_EJECTED) || (cur_cdmode == WM_CDM_STOPPED)) use_alt = 1;
	break;
    case FAK_CD_LOOP:
	if (loop_mode) use_alt = 1;
	break;
    case FAK_CD_MUTE:
	if (muted) use_alt = 1;
	break;
    case FAK_CD_VOLUME:
	if (muted) use_alt = 1;
	break;
    case FAK_CD_INTRO:
	if (intro_mode) use_alt = 1;
	break;
    case FAK_TOG_AUTOREPEAT:
	if (autorepeat) use_alt = 1;
	break;
    case FAK_TOG_AUTOPLAY:
	if (autoplay) use_alt = 1;
	break;
    case FAK_TOG_UPPER:
	if (force_upper) use_alt = 1;
	break;
    case FAK_TOG_SHOWDB:
	if (show_db) use_alt = 1;
	break;
    case FAK_TOG_SHOWARTIST:
	if (show_artist) use_alt = 1;
	break;
    case FAK_TOG_ISKIPS:
	if (ignore_avoid) use_alt = 1;
	break;
    case FAK_TSELECT:
    case FAK_TNEXT:
    case FAK_TPREVIOUS:
	if (theme_select) use_alt = 1;
	break;
    default:
	use_alt = 0;
	break;
    }
    return use_alt;
}

void fak_validate_pixmap(char *string, char *name)
{
    sprintf(string, "%s/Themes/%s/%s", THDIR, theme, name);
    if (access(string, R_OK) != 0) {
	sprintf(string, "%s/Default/%s", THDIR, name);
	if (access(string, R_OK) != 0) {
	    fprintf(stderr, "\nascd: fatal error while loading theme '%s'\n", theme);
	    fprintf(stderr, "      Pixmap file '%s' was not found.\n\n", name);
	    exit(1);
	}
    }
}

void fak_init_theme(int upgrade)
{
    /* re-init all the buttons structure and globals */

    int i;

    if (debug) fprintf(stderr, "** Init theme variables.\n   Buttons...\n");
    for (i = 1; i <= but_max ; i++) {

	thdata[i].type = 0;
	thdata[i].panel = 0;
	thdata[i].left = 0;
	thdata[i].mid = 0;
	thdata[i].right = 0;
	thdata[i].x = 0;
	thdata[i].y = 0;
	thdata[i].w = 0;
	thdata[i].h = 0;
	thdata[i].arg = 0;
	thdata[i].icon = FALSE;
	thdata[i].ox = 0;
	thdata[i].oy = 0;

	if (strcmp(thdata[i].xpm_file, "") != 0) {
	    strcpy(thdata[i].xpm_file, "");
	    /*if (upgrade) XpmFree(&thdata[i].xpm);*/
	}
	if (strcmp(thdata[i].altxpm_file, "") != 0) {
	    strcpy(thdata[i].altxpm_file, "");
	    /*if (upgrade) XpmFree(&thdata[i].altxpm);*/
	}
    }

    if (debug) fprintf(stderr, "   Strings & globals...\n");

    strcpy(th_name, "");
    strcpy(th_author, "");
    strcpy(th_release, "");
    strcpy(th_email, "");
    strcpy(th_url, "");
    strcpy(th_comment, "");
    strcpy(th_alpha1, "");
    strcpy(th_alpha2, "");
    strcpy(th_background, "");
    strcpy(th_icon_window, "");

    th_no_minus = FALSE;

    but_max = 0;
    but_counter = 0;
    but_msg = 0;
    but_tracknbr = 0;
    but_db = 0;
    but_current = 0;

    icon_counter = 0;
    icon_msg = 0;
    icon_tracknbr = 0;

    panels = 0;
    panel = 1;

    panel_stop = 0;
    panel_play = 0;

    /*
    if (upgrade) {
	XpmFree(&alphaXPM);
	XpmFree(&ralphaXPM);
	XpmFree(&backXPM);
    }
    */
    if (debug) fprintf(stderr, "-> done.\n");

}

int fak_load_theme(char *th, int upgrade)
{
    char txt[256];
    char key[256];
    char arguments[256];
    int button = FALSE;
    unsigned int i;
    XWindowAttributes Attributes;
    int Ret;
    FILE *in;
#ifdef MIXER
    unsigned int j;
struct {
    char name[MAX_LINE_NAME];
    int dev;
} mixernames[SOUND_MIXER_NRDEVICES] = {
    { "volume", SOUND_MIXER_VOLUME },
    { "bass", SOUND_MIXER_BASS },
    { "treble", SOUND_MIXER_TREBLE },
    { "midi", SOUND_MIXER_SYNTH },
    { "pcm", SOUND_MIXER_PCM },
    { "speaker", SOUND_MIXER_SPEAKER },
    { "line", SOUND_MIXER_LINE },
    { "mic", SOUND_MIXER_MIC },
    { "cd", SOUND_MIXER_CD },
    { "imix", SOUND_MIXER_IMIX },
    { "altpcm", SOUND_MIXER_ALTPCM },
    { "reclev", SOUND_MIXER_RECLEV },
    { "igain", SOUND_MIXER_IGAIN },
    { "ogain", SOUND_MIXER_OGAIN },
    { "line1", SOUND_MIXER_LINE1 },
    { "line2", SOUND_MIXER_LINE2 },
    { "line3", SOUND_MIXER_LINE3 }
};
#endif

    /* check if theme directory if ok: */

    sprintf(txt, "%s/Themes/%s", THDIR, th);

    if (debug) fprintf(stderr, "** Theme to load: %s\n", txt);

    if (access(txt, X_OK | R_OK) != 0) {
	sprintf(txt, "%s/Themes/default", THDIR);
	if (access(txt, X_OK | R_OK) != 0) {
	    fprintf(stderr, "ascd: fatal error\n\n");
	    fprintf(stderr, "The '%s' and 'default' themes folders are not \navailable in %s.\n\n", th, THDIR);
	    fprintf(stderr, "Please check the permissions and/or redo ascd installation.\n\n");
	    exit(1);
	} else {
	    strcpy(th, "default");
	}
    }

    sprintf(txt, "%s/Themes/%s/Theme", THDIR, th);

    /* parse and load the file */

    if (in = fopen(txt, "r")) {

	/* we first clear everything... */

	fak_init_theme(upgrade);

	/* that's ok, let's go! */

	if (debug) fprintf(stderr, "-> Parsing Theme file...\n");
	while (tes_xgets(txt, 255, in)) {
	    if (fak_parse_line(txt, key, arguments)) {
		if (key[0] == '{') {
		    button = TRUE;
		    if (but_max < FAK_BMAX - 1) {
			but_max ++;
			strcpy(thdata[but_max].xpm_file, "");
			strcpy(thdata[but_max].altxpm_file, "");
			thdata[but_max].type = 0;
		    } else {
			/* no more free buttons. We'll ignore the rest */
			button = FALSE;
		    }
		} else if (key[0] == '}') {
		    button = FALSE;

		    /* update the pointers to msg, counter and tracknbr: */

		    switch(thdata[but_max].type) {
		    case FAK_COUNTER:
			if (thdata[but_max].icon) icon_counter = but_max;
			else but_counter = but_max;
			break;
		    case FAK_MSG:
			if (thdata[but_max].icon) icon_msg = but_max;
			else but_msg = but_max;
			break;
		    case FAK_DB:
			but_db = but_max;
			break;
		    case FAK_TRACKNBR:
			if (thdata[but_max].icon) icon_tracknbr = but_max;
			else but_tracknbr = but_max;
			break;
		    default:
			break;
		    }

		    if (debug > 1) {
			fprintf(stderr, "   Button %02d type: %02d commands: %02d %02d %02d arg: %02d\n", but_max, thdata[but_max].type, thdata[but_max].left, thdata[but_max].mid, thdata[but_max].right, thdata[but_max].arg);
			fprintf(stderr, "   Geometry: %02dx%02d on panel %02d\n", thdata[but_max].x, thdata[but_max].y, thdata[but_max].panel);
			fprintf(stderr, "   Pixmaps: [%s] and [%s]\n", thdata[but_max].xpm_file, thdata[but_max].altxpm_file);
		    }
		}

		if (!button) {
		    if (strcmp(key, "name") == 0) strcpy(th_name, arguments);
		    else if (strcmp(key, "release") == 0) strcpy(th_release, arguments);
		    else if (strcmp(key, "author") == 0) strcpy(th_author, arguments);
		    else if (strcmp(key, "email") == 0) strcpy(th_email, arguments);
		    else if (strcmp(key, "url") == 0) strcpy(th_url, arguments);
		    else if (strcmp(key, "comment") == 0) strcpy(th_comment, arguments);
		    else if (strcmp(key, "alpha1") == 0) strcpy(th_alpha1, arguments);
		    else if (strcmp(key, "alpha2") == 0) strcpy(th_alpha2, arguments);
		    else if (strcmp(key, "background") == 0) strcpy(th_background, arguments);
		    else if (strcmp(key, "icon_background") == 0) strcpy(th_icon_window, arguments);
		    else if (strcmp(key, "panels") == 0) panels = atoi(arguments);
		    else if (strcmp(key, "panel_stop") == 0) panel_stop = atoi(arguments);
		    else if (strcmp(key, "panel_play") == 0) panel_play = atoi(arguments);
		    else if (strcmp(key, "no_minus") == 0) th_no_minus = TRUE;
		} else {

		    /* button definition: */

		    if (strcmp(key, "icon") == 0) thdata[but_max].icon = TRUE;

		    if (strcmp(key, "type") == 0) {
			if (strcmp(arguments, "pixmap") == 0) thdata[but_max].type = FAK_PIXMAP;
			else if (strcmp(arguments, "counter") == 0) thdata[but_max].type = FAK_COUNTER;
			else if (strcmp(arguments, "tracknumber") == 0) thdata[but_max].type = FAK_TRACKNBR;
			else if (strcmp(arguments, "message") == 0) thdata[but_max].type = FAK_MSG;
			else if (strcmp(arguments, "database") == 0) thdata[but_max].type = FAK_DB;
			else if (strcmp(arguments, "progress_bar") == 0) thdata[but_max].type = FAK_CD_BAR;
			else if (strcmp(arguments, "vprogress_bar") == 0) thdata[but_max].type = FAK_VCD_BAR;
			else if (strcmp(arguments, "iprogress_bar") == 0) thdata[but_max].type = FAK_ICD_BAR;
			else if (strcmp(arguments, "pixmap_bar") == 0) thdata[but_max].type = FAK_CD_PIX;
			else if (strcmp(arguments, "vpixmap_bar") == 0) thdata[but_max].type = FAK_VCD_PIX;
			else if (strcmp(arguments, "volume_bar") == 0) thdata[but_max].type = FAK_VOL_BAR;
			else if (strcmp(arguments, "volume") == 0) thdata[but_max].type = FAK_VOL_BAR;
			else if (strcmp(arguments, "pixmap_volume") == 0) thdata[but_max].type = FAK_VOL_PIX;
			else if (strcmp(arguments, "vpixmap_volume") == 0) thdata[but_max].type = FAK_VVOL_PIX;
			else if (strcmp(arguments, "vvolume") == 0) thdata[but_max].type = FAK_VVOL_BAR;
			else if (strcmp(arguments, "vvolume_bar") == 0) thdata[but_max].type = FAK_VVOL_BAR;
			else if (strcmp(arguments, "ivolume") == 0) thdata[but_max].type = FAK_IVOL_BAR;
			else if (strcmp(arguments, "ivolume_bar") == 0) thdata[but_max].type = FAK_IVOL_BAR;
			else if (strcmp(arguments, "mixer_bar") == 0) thdata[but_max].type = FAK_MIXER_BAR;
			else if (strcmp(arguments, "mixer") == 0) thdata[but_max].type = FAK_MIXER_BAR;
			else if (strcmp(arguments, "vmixer_bar") == 0) thdata[but_max].type = FAK_VMIXER_BAR;
			else if (strcmp(arguments, "vmixer") == 0) thdata[but_max].type = FAK_VMIXER_BAR;
			else if (strcmp(arguments, "imixer_bar") == 0) thdata[but_max].type = FAK_IMIXER_BAR;
			else if (strcmp(arguments, "imixer") == 0) thdata[but_max].type = FAK_IMIXER_BAR;
		    }

		    i = 0;
		    if ((strcmp(key, "left") == 0) ||
			(strcmp(key, "middle") ==0) ||
			(strcmp(key, "mid") ==0) ||
			(strcmp(key, "right") == 0)) {

			/* --------------- available commands: ---------------- */

			/* basic orders */

			if (strcmp(arguments, "panel_switch") == 0) i = FAK_PANEL_SWITCH;
			else if (strcmp(arguments, "panel 1") == 0) i = FAK_PANEL1;
			else if (strcmp(arguments, "panel 2") == 0) i = FAK_PANEL2;
			else if (strcmp(arguments, "panel 3") == 0) i = FAK_PANEL3;
			else if (strcmp(arguments, "panel 4") == 0) i = FAK_PANEL4;
			else if (strcmp(arguments, "panel 5") == 0) i = FAK_PANEL5;
			else if (strcmp(arguments, "counter_mode") == 0) i = FAK_COUNTER_MODE;
			else if (strcmp(arguments, "quit") == 0) i = FAK_QUIT;
			else if (strcmp(arguments, "wings_window") == 0) i = FAK_WINGS;
			else if (strcmp(arguments, "theme_select") == 0) i = FAK_TSELECT;
			else if (strcmp(arguments, "theme_next") == 0) i = FAK_TNEXT;
			else if (strcmp(arguments, "theme_previous") == 0) i = FAK_TPREVIOUS;
			else if (strcmp(arguments, "ftrack_select") == 0) i = FAK_FTSELECT;
			else if (strcmp(arguments, "ftrack_next") == 0) i = FAK_FTNEXT;
			else if (strcmp(arguments, "ftrack_previous") == 0) i = FAK_FTPREVIOUS;
			else if (strcmp(arguments, "autoplay") == 0) i = FAK_TOG_AUTOPLAY;
			else if (strcmp(arguments, "autorepeat") == 0) i = FAK_TOG_AUTOREPEAT;
			else if (strcmp(arguments, "show_db") == 0) i = FAK_TOG_SHOWDB;
			else if (strcmp(arguments, "show_artist") == 0) i = FAK_TOG_SHOWARTIST;
			else if (strcmp(arguments, "uppercase") == 0) i = FAK_TOG_UPPER;
			else if (strcmp(arguments, "ignore_skips") == 0) i = FAK_TOG_ISKIPS;
			else if (strcmp(arguments, "save") == 0) i = FAK_SAVE;
			else if (strcmp(arguments, "load") == 0) i = FAK_LOAD;
			else if (strcmp(arguments, "quick_ref") == 0) i = FAK_QREF;

			/* CD player */

			else if (strcmp(arguments, "play") == 0) i = FAK_CD_PLAY;
			else if (strcmp(arguments, "pause") == 0) i = FAK_CD_PAUSE;
			else if (strcmp(arguments, "stop") == 0) i = FAK_CD_STOP;
			else if (strcmp(arguments, "eject") == 0) i = FAK_CD_EJECT;
			else if (strcmp(arguments, "stop_eject") == 0) i = FAK_CD_STOPEJECT;
			else if (strcmp(arguments, "eject_quit") == 0) i = FAK_CD_EJECTQUIT;
			else if (strcmp(arguments, "rew") == 0) i = FAK_CD_REW;
			else if (strcmp(arguments, "first_track") == 0) i = FAK_CD_FIRST;
			else if (strcmp(arguments, "previous_track") == 0) i = FAK_CD_PREVIOUS;
			else if (strcmp(arguments, "fwd") == 0) i = FAK_CD_FWD;
			else if (strcmp(arguments, "last_track") == 0) i = FAK_CD_LAST;
			else if (strcmp(arguments, "next_track") == 0) i = FAK_CD_NEXT;
			else if (strcmp(arguments, "direct_access") == 0) i = FAK_CD_DIRECT;
			else if (strcmp(arguments, "loop") == 0) i = FAK_CD_LOOP;
			else if (strcmp(arguments, "loop_start") == 0) i = FAK_CD_LSTART;
			else if (strcmp(arguments, "loop_end") == 0) i = FAK_CD_LEND;
			else if (strcmp(arguments, "loop_go_start") == 0) i = FAK_CD_GOLSTART;
			else if (strcmp(arguments, "loop_go_end") == 0) i = FAK_CD_GOLEND;
			else if (strcmp(arguments, "loop_track") == 0) i = FAK_CD_LTRACK;
			else if (strcmp(arguments, "loop_to_track") == 0) i = FAK_CD_LTOTRACK;
			else if (strcmp(arguments, "loop_from_track") == 0) i = FAK_CD_LFROMTRACK;
			else if (strcmp(arguments, "loop_clear") == 0) i = FAK_CD_LCLEAR;
			else if (strcmp(arguments, "intro_scan") == 0) i = FAK_CD_INTRO;
			else if (strcmp(arguments, "fade") == 0) i = FAK_CD_FADE;
			else if (strcmp(arguments, "random_jump") == 0) i = FAK_CD_RANDOM;
			else if (strcmp(arguments, "random_mode") == 0) i = FAK_CD_RMODE;
			else if (strcmp(arguments, "mute") == 0) i = FAK_CD_MUTE;
			else if (strcmp(arguments, "volume") == 0) i = FAK_CD_VOLUME;

			/* mixer */

			else if (strcmp(arguments, "set") == 0) i = FAK_MIXER_SET;
			else if (strcmp(arguments, "zero") == 0) i = FAK_MIXER_0;
			else if (strcmp(arguments, "0") == 0) i = FAK_MIXER_0;
			else if (strcmp(arguments, "50%") == 0) i = FAK_MIXER_50;
			else if (strcmp(arguments, "50") == 0) i = FAK_MIXER_50;
			else if (strcmp(arguments, "75%") == 0) i = FAK_MIXER_75;
			else if (strcmp(arguments, "75") == 0) i = FAK_MIXER_75;
			else if (strcmp(arguments, "100%") == 0) i = FAK_MIXER_100;
			else if (strcmp(arguments, "100") == 0) i = FAK_MIXER_100;

			/* ------------- which mouse button? --------------- */

			if (strcmp(key, "left") == 0) thdata[but_max].left = i;
			else if (strcmp(key, "middle") == 0) thdata[but_max].mid = i;
			else if (strcmp(key, "mid") == 0) thdata[but_max].mid = i;
			else if (strcmp(key, "right") == 0) thdata[but_max].right = i;
		    }

		    else if (strcmp(key, "panel") == 0) thdata[but_max].panel = atoi(arguments);
		    else if (strcmp(key, "x") == 0) thdata[but_max].x = atoi(arguments);
		    else if (strcmp(key, "y") == 0) thdata[but_max].y = atoi(arguments);
		    else if (strcmp(key, "w") == 0) thdata[but_max].w = atoi(arguments);
		    else if (strcmp(key, "h") == 0) thdata[but_max].h = atoi(arguments);
		    else if (strcmp(key, "arg") == 0) {
			thdata[but_max].arg = atoi(arguments);
#ifdef MIXER
			for(j = 0; j < SOUND_MIXER_NRDEVICES; j++) {
			    if(strcasecmp(mixernames[j].name, arguments) == 0) {
				thdata[but_max].arg = mixernames[j].dev;
				break;
			    }
			}
#endif
		    }
		    else if (strcmp(key, "pixmap") == 0) strcpy(thdata[but_max].xpm_file, arguments);
		    else if (strcmp(key, "alt") == 0) strcpy(thdata[but_max].altxpm_file, arguments);
		}
	    }
	}
	fclose(in);

	if (debug) {
	    fprintf(stderr, "-> Theme summary for '%s':\n", th_name);
	    fprintf(stderr, "   Release: %s\n", th_release);
	    fprintf(stderr, "   Author:  %s (%s)\n", th_author, th_email);
	    fprintf(stderr, "   URL:     %s\n", th_url);
	    fprintf(stderr, "   Comment: %s\n", th_comment);
	    fprintf(stderr, "   Stats:   %d panels, %d buttons\n", panels, but_max);
	    fprintf(stderr, "   Display: counter: %d / msg: %d (%d cars) / db: %d / track: %d\n", but_counter, but_msg, thdata[but_msg].w, but_db, but_tracknbr);
	    if (th_no_icon_window) 	    fprintf(stderr, "   Using separate icon definition, background = %s\n", th_icon_window);
	    fprintf(stderr, "-> Loading basic pixmap files... ");
	}

	XGetWindowAttributes(Disp, Root, &Attributes);

	/* the background pixmap: */

	if (debug) fprintf(stderr, "background ");

	fak_validate_pixmap(txt, th_background);
	Ret = XpmReadFileToPixmap(Disp, Root, txt, &backXPM.pixmap,
				  &backXPM.mask, &backXPM.attributes);
	if ((Ret != XpmSuccess) && (debug)) fprintf(stderr, "*!* ");

	/* the fonts: */

	if (debug) fprintf(stderr, "alpha 1 ");

	fak_validate_pixmap(txt, th_alpha1);
	Ret = XpmReadFileToPixmap(Disp, Root, txt, &alphaXPM.pixmap,
				  &alphaXPM.mask, &alphaXPM.attributes);
	if ((Ret != XpmSuccess) && (debug)) fprintf(stderr, "*!* ");

	if (debug) fprintf(stderr, "2 ");

	fak_validate_pixmap(txt, th_alpha2);
	Ret = XpmReadFileToPixmap(Disp, Root, txt, &ralphaXPM.pixmap,
				  &ralphaXPM.mask, &ralphaXPM.attributes);
	if ((Ret != XpmSuccess) && (debug)) fprintf(stderr, "*!* ");

	if (debug) fprintf(stderr, "\n-> Loading buttons pixmaps... ");
	for (i = 1; i <= but_max; i++) {
	    if (strlen(thdata[i].xpm_file) > 0) {
		if (debug > 1) fprintf(stderr, "%d ", i);
		fak_validate_pixmap(txt, thdata[i].xpm_file);
		thdata[i].xpm.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);
		Ret = XpmReadFileToPixmap(Disp, Root, txt, &thdata[i].xpm.pixmap, &thdata[i].xpm.mask, &thdata[i].xpm.attributes);
		if ((Ret != XpmSuccess) && (debug > 1)) fprintf(stderr, "*!* ");
	    }
	    if (strlen(thdata[i].altxpm_file) > 0) {
		if (debug > 1) fprintf(stderr, "a ");
		fak_validate_pixmap(txt, thdata[i].altxpm_file);
		thdata[i].xpm.attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);
		Ret = XpmReadFileToPixmap(Disp, Root, txt, &thdata[i].altxpm.pixmap, &thdata[i].altxpm.mask, &thdata[i].altxpm.attributes);
		if ((Ret != XpmSuccess) && (debug > 1)) fprintf(stderr, "*!* ");
	    }
	}

	/* the icon */

	if (strlen(th_icon_window) > 0) {
	    if (debug) fprintf(stderr, "\n-> Loading icon... ");
	    fak_validate_pixmap(txt, th_icon_window);
	    Ret = XpmReadFileToPixmap(Disp, Root, txt, &iconXPM.pixmap,
				      &iconXPM.mask, &iconXPM.attributes);
	    if ((Ret != XpmSuccess) && (debug)) fprintf(stderr, "*!* ");
	}

	if (debug) fprintf(stderr, "\n-> Theme loaded.\n");
	return TRUE;
    } else {
	return FALSE;
    }
}

void fak_icon_text(char *string, unsigned int where, unsigned int col, unsigned int red)
{
    unsigned int x = 0;
    unsigned int offset = 0;
    unsigned hoffset = 0;
    unsigned voffset = 0;
    unsigned char txt[128];
    unsigned int j;
    unsigned int char_max;
    int out = FALSE;

    if (!th_no_icon_window) return;

    switch(where) {
    case COUNTER_PANEL:
	if (icon_counter == 0) out = TRUE;
	else {
	    voffset = thdata[icon_counter].y;
	    hoffset = thdata[icon_counter].x;
	}
	break;
    case MSG_PANEL:
	if (icon_msg == 0) out = TRUE;
	else {
	    voffset = thdata[icon_msg].y;
	    hoffset = thdata[icon_msg].x;
	}
	break;
    default:
	if (icon_tracknbr == 0) out = TRUE;
	else {
	    voffset = thdata[icon_tracknbr].y;
	    hoffset = thdata[icon_tracknbr].x;
	}
	break;
    }

    if (out) return;

    strcpy(txt, string);

    /* Deal with the string length: */

    if ((strlen(txt) > thdata[icon_msg].w) && (where == MSG_PANEL)) {
	txt[thdata[icon_msg].w] = 0;

    }

    if ((strcmp(txt, "") == 0) && (where == MSG_PANEL)) {
	for (x = 1; x <= thdata[icon_msg].w; x++) {
	    strcat(txt, " ");
	}
    }


    if (red) {
	char_max = alphaXPM.attributes.width / 6;
    } else {
	char_max = ralphaXPM.attributes.width / 6;
    }

    for(x = 0; x < strlen(txt); x++) {

	/* lookup in the accents table to remove them */
	if (! force_upper) {

	    if (char_max <= 116) { /* don't convert if 8bit font */
		for (j = 0 ; j <= strlen(ACCTABLE) - 2 ; j = j + 2) {
		    if ( txt[x] == (unsigned char)ACCTABLE[j] ) {
			txt[x] = ACCTABLE[j + 1];
			break;
		    }
		}
	    }

	} else {
	    /* force upper case */
	    for (j = 0 ; j <= strlen(UPACCTABLE) - 2 ; j = j + 2) {
		if ( txt[x] == (unsigned char)UPACCTABLE[j] ) {
		    txt[x] = UPACCTABLE[j + 1];
		    break;
		}
	    }
	    txt[x] = toupper(txt[x]);
	}

	if ((txt[x] >= 32) && (txt[x] <= char_max + 32)) offset = txt[x] - 32;
	else offset = 31;

	if (red) {
	    XCopyArea(Disp, ralphaXPM.pixmap, Iconwin, WinGC,
		      6 * offset, 0, 6, 9,
		      hoffset + (col * 6) + (x * 6),
		      voffset);
	} else {
	    XCopyArea(Disp, alphaXPM.pixmap, Iconwin, WinGC,
		      6 * offset, 0, 6, 9,
		      hoffset + (col * 6) + (x * 6),
		      voffset);
	}

    }
}

void fak_text(char *string, unsigned int where, unsigned int col, unsigned int red) {

    /* different behaviour than the old draw_text() found in AScd <= 0.12.... The
       where parameter replaces the odl "row" one and should be:

       0 = write to the counter zone
       1 = write to the message zone
       2 = write to the track number zone
    */

    unsigned int x = 0;
    unsigned int offset = 0;
    unsigned hoffset = 0;
    unsigned voffset = 0;
    unsigned char txt[128];
    unsigned int j;
    unsigned int char_max;
    int out = FALSE;
    int do_icon = TRUE;

    if (where == DB_PANEL) {
	do_icon = FALSE;
	if (but_db == 0) where = MSG_PANEL;
    }

    if (do_icon) fak_icon_text(string, where, col, red);

    switch(where) {
    case COUNTER_PANEL:
	if (but_counter == 0) out = TRUE;
	else {
	    voffset = thdata[but_counter].y;
	    hoffset = thdata[but_counter].x;
	}
	break;
    case MSG_PANEL:
	if (but_msg == 0) out = TRUE;
	else {
	    voffset = thdata[but_msg].y;
	    hoffset = thdata[but_msg].x;
	}
	break;
    case DB_PANEL:
	voffset = thdata[but_db].y;
	hoffset = thdata[but_db].x;
	break;
    default:
	if (but_tracknbr == 0) out = TRUE;
	else {
	    voffset = thdata[but_tracknbr].y;
	    hoffset = thdata[but_tracknbr].x;
	}
	break;
    }

    if (out) return;

    strcpy(txt, string);

    /* Deal with the string length: */

    if ((strlen(txt) > thdata[but_msg].w) && (where == MSG_PANEL)) {
	txt[thdata[but_msg].w] = 0;

    }

    if ((strlen(txt) > thdata[but_db].w) && (where == DB_PANEL)) {
	txt[thdata[but_msg].w] = 0;

    }

    if ((strcmp(txt, "") == 0) && (where == MSG_PANEL)) {
	for (x = 1; x <= thdata[but_msg].w; x++) {
	    strcat(txt, " ");
	}
    }

    if ((strcmp(txt, "") == 0) && (where == DB_PANEL)) {
	for (x = 1; x <= thdata[but_db].w; x++) {
	    strcat(txt, " ");
	}
    }

    if (red) {
	char_max = alphaXPM.attributes.width / 6;
    } else {
	char_max = ralphaXPM.attributes.width / 6;
    }

    for(x = 0; x < strlen(txt); x++) {

	/* lookup in the accents table to remove them */
	if (! force_upper) {

	    if (char_max <= 116) { /* don't convert if 8bit font */
		for (j = 0 ; j <= strlen(ACCTABLE) - 2 ; j = j + 2) {
		    if ( txt[x] == (unsigned char)ACCTABLE[j] ) {
			txt[x] = ACCTABLE[j + 1];
			break;
		    }
		}
	    }

	} else {
	    /* force upper case */
	    for (j = 0 ; j <= strlen(UPACCTABLE) - 2 ; j = j + 2) {
		if ( txt[x] == (unsigned char)UPACCTABLE[j] ) {
		    txt[x] = UPACCTABLE[j + 1];
		    break;
		}
	    }
	    txt[x] = toupper(txt[x]);
	}

	if ((txt[x] >= 32) && (txt[x] <= char_max + 32)) offset = txt[x] - 32;
	else offset = 31;

	if (red) {
	    XCopyArea(Disp, ralphaXPM.pixmap, Win, WinGC,
		      6 * offset, 0, 6, 9,
		      hoffset + (col * 6) + (x * 6),
		      voffset);

	    if (!th_no_icon_window) {
		XCopyArea(Disp, ralphaXPM.pixmap, Iconwin, WinGC,
			  6 * offset, 0, 6, 9,
			  hoffset + (col * 6) + (x * 6),
		      voffset);
	    }
	} else {
	    XCopyArea(Disp, alphaXPM.pixmap, Win, WinGC,
		      6 * offset, 0, 6, 9,
		      hoffset + (col * 6) + (x * 6),
		      voffset);

	    if (!th_no_icon_window) {
		XCopyArea(Disp, alphaXPM.pixmap, Iconwin, WinGC,
			  6 * offset, 0, 6, 9,
			  hoffset + (col * 6) + (x * 6),
		      voffset);
	    }
	}

    }
}

int fak_flush_expose(Window w)
{
    XEvent dummy;
    int i=0;

    while (XCheckTypedWindowEvent (Disp, w, Expose, &dummy))i++;
    return i;
}

void fak_minus(void) {
    if (th_no_minus) return;

    if (time_mode == 1) {
	if (!th_no_icon_window) {
	    if (but_counter == 0) return;
	    XCopyArea(Disp, alphaXPM.pixmap, Win, WinGC,
		      6 * (45 - 32), 0, 6, 7,
		      thdata[but_counter].x, thdata[but_counter].y);
	    XCopyArea(Disp, alphaXPM.pixmap, Iconwin, WinGC,
		      6 * (45 - 32), 0, 6, 7,
		      thdata[but_counter].x, thdata[but_counter].y);
	} else {
	    if (but_counter > 0) {
		XCopyArea(Disp, alphaXPM.pixmap, Win, WinGC,
			  6 * (45 - 32), 0, 6, 7,
			  thdata[but_counter].x, thdata[but_counter].y);
	    }
	    if (icon_counter > 0) {
		XCopyArea(Disp, alphaXPM.pixmap, Iconwin, WinGC,
			  6 * (45 - 32), 0, 6, 7,
			  thdata[icon_counter].x, thdata[icon_counter].y);
	    }
	}
    } else if (time_mode == 3) {
	if (!th_no_icon_window) {
	    if (but_counter == 0) return;
	    XCopyArea(Disp, alphaXPM.pixmap, Win, WinGC,
		      588, 0, 4, 7,
		      thdata[but_counter].x, thdata[but_counter].y);
	    XCopyArea(Disp, alphaXPM.pixmap, Iconwin, WinGC,
		      588, 0, 4, 7,
		      thdata[but_counter].x, thdata[but_counter].y);
	} else {
	    if (but_counter > 0) {
		XCopyArea(Disp, alphaXPM.pixmap, Win, WinGC,
			  588, 0, 4, 7,
			  thdata[but_counter].x, thdata[but_counter].y);
	    }
	    if (icon_counter > 0) {
		XCopyArea(Disp, alphaXPM.pixmap, Iconwin, WinGC,
			  588, 0, 4, 7,
			  thdata[icon_counter].x, thdata[icon_counter].y);
	    }
	}
    }

    /* if displaying CD times, we display a little 'cd' */

    if (time_mode >= 2) {
	if (!th_no_icon_window) {
	    if (but_counter == 0) return;
	    XCopyArea(Disp, ralphaXPM.pixmap, Win, WinGC,
		      582, 0, 4, 4,
		      thdata[but_counter].x, thdata[but_counter].y);
	    XCopyArea(Disp, ralphaXPM.pixmap, Iconwin, WinGC,
		      582, 0, 4, 4,
		      thdata[but_counter].x, thdata[but_counter].y);
	} else {
	    if (but_counter > 0) {
		XCopyArea(Disp, ralphaXPM.pixmap, Win, WinGC,
			  582, 0, 4, 4,
			  thdata[but_counter].x, thdata[but_counter].y);
	    }
	    if (icon_counter > 0) {
		XCopyArea(Disp, ralphaXPM.pixmap, Iconwin, WinGC,
			  582, 0, 4, 4,
			  thdata[icon_counter].x, thdata[icon_counter].y);
	    }
	}
    }

}

void fak_singlemask(int i)
{
    /* we first remove pixmap and alt from the mask */

    if (strlen(thdata[i].xpm_file) > 0) {
	if (!th_no_icon_window) {
	    XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].xpm.mask, ShapeSubtract);
	    XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].xpm.mask, ShapeSubtract);
	} else {
	    if (thdata[i].icon) {
		XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
				  thdata[i].xpm.mask, ShapeSubtract);
	    } else {
		XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
				  thdata[i].xpm.mask, ShapeSubtract);
	    }
	}
    }

    if (strlen(thdata[i].altxpm_file) > 0) {
	if (!th_no_icon_window) {
	    XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].altxpm.mask, ShapeSubtract);
	    XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].altxpm.mask, ShapeSubtract);
	} else {
	    if (thdata[i].icon) {
		XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
				  thdata[i].altxpm.mask, ShapeSubtract);
	    } else {
		XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
				  thdata[i].altxpm.mask, ShapeSubtract);
	    }
	}
    }

    /* then we set of the two : */

    if ((fak_use_alt(i)) && (strlen(thdata[i].altxpm_file) > 0)) {
	if (!th_no_icon_window) {
	    XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].altxpm.mask, ShapeUnion);
	    XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].altxpm.mask, ShapeUnion);
	} else {
	    if (thdata[i].icon) {
		XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
				  thdata[i].altxpm.mask, ShapeUnion);
	    } else {
		XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
				  thdata[i].altxpm.mask, ShapeUnion);
	    }
	}
    } else {
	if (!th_no_icon_window) {
	    XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].xpm.mask, ShapeUnion);
	    XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].xpm.mask, ShapeUnion);
	} else {
	    if (thdata[i].icon) {
	    XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
			      thdata[i].xpm.mask, ShapeUnion);
	    } else {
		XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
				  thdata[i].xpm.mask, ShapeUnion);
	    }
	}
    }
}

void fak_maskset()
{
    int i;

    if (debug) fprintf(stderr, "** Setting pixmaps mask\n");
    if (debug) fprintf(stderr, "-> background\n");

    XShapeCombineMask(Disp, Win, ShapeBounding, 0, 0,
		      backXPM.mask, ShapeSet);

    if (!th_no_icon_window) {
	XShapeCombineMask(Disp, Iconwin, ShapeBounding, 0, 0,
			  backXPM.mask, ShapeSet);
    } else {
	if (strlen(th_icon_window) > 0) {
	    XShapeCombineMask(Disp, Iconwin, ShapeBounding, 0, 0,
			      iconXPM.mask, ShapeSet);
	}
    }
    if (debug) fprintf(stderr, "-> buttons ");
    for (i=1; i <= but_max; i++) {
	if ((thdata[i].panel == panel) || (thdata[i].panel == 0)) {
	    if (strlen(thdata[i].xpm_file) > 0) {
		if (debug) fprintf(stderr, "%d ", i);
		if ((fak_use_alt(i)) && (strlen(thdata[i].altxpm_file) > 0)) {
		    if (!th_no_icon_window) {
			XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
				      thdata[i].altxpm.mask, ShapeUnion);
			XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
					  thdata[i].altxpm.mask, ShapeUnion);
		    } else {
			if (thdata[i].icon) {
			    XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
					      thdata[i].altxpm.mask, ShapeUnion);
			} else {
			    XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
					      thdata[i].altxpm.mask, ShapeUnion);
			}
		    }
		} else {
		    if (!th_no_icon_window) {
			XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
					  thdata[i].xpm.mask, ShapeUnion);
			XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
				      thdata[i].xpm.mask, ShapeUnion);
		    } else {
			if (thdata[i].icon) {
			    XShapeCombineMask(Disp, Iconwin, ShapeBounding, thdata[i].x, thdata[i].y,
					      thdata[i].xpm.mask, ShapeUnion);
			} else {
			    XShapeCombineMask(Disp, Win, ShapeBounding, thdata[i].x, thdata[i].y,
					      thdata[i].xpm.mask, ShapeUnion);
			}
		    }
		}
	    }
	}
    }
    if (debug) fprintf(stderr, "\n-> done.\n");
}

void fak_redraw()
{
    char txt[256];
    int i;
    long int dodo = 300000;
    char cdtime[6];
    int disp_time;
    int use_alt = FALSE;
    int offset;
    DIR *dir_fd;
    struct dirent *dir_pt;
    unsigned int fx;
    unsigned int fy;
    unsigned int fw;
    unsigned int fh;

    /* ============================================================== */
    /* ============================================================== */
    /* ============================================================== */
    /* First part: check different modes before doing the real redraw */
    /* ============================================================== */
    /* ============================================================== */
    /* ============================================================== */

    if (debug) fprintf(stderr, "** Entering redraw routine\n");

    if (!blind_mode) {
	if ((cur_cdmode == WM_CDM_PLAYING) || (wanna_play)) wm_cd_status();
    }

    /* the panel auto-switch mode: */

    if (((cur_cdmode == WM_CDM_STOPPED) || (cur_cdmode == WM_CDM_EJECTED)) && (panel_stop > 0)) panel = panel_stop;
    if (((cur_cdmode == WM_CDM_PLAYING) || (cur_cdmode == WM_CDM_PAUSED)) && (panel_play > 0)) panel = panel_play;

    /* ===================================================================
       Auto-Repeat mode: at then end of the CD, play again the first track
       =================================================================== */

    if ((cur_track >= cur_ntracks) && (cur_cdmode != WM_CDM_PAUSED) && (cur_cdmode != WM_CDM_PLAYING)) {
	/*if (wanna_play) {*/
	if (autorepeat) {
	    newtext("A.Repeat");
	    do_autorepeat = TRUE;
	    cd_control(PLAY);
	} else {
	    cd_control(STOPONLY); /* added in 0.13 */
	    if (debug) fprintf(stderr, "-> Stopping\n");
	    cur_cdmode = WM_CDM_STOPPED;
	    wm_cd_status();
	    cur_track = 0;
	    redraw = TRUE;
	    fak_maskset();
	}
    }

    /* ======================================================= */
    /* the next track handling. We have to skip 'avoid' tracks */
    /* ======================================================= */

    if ((cur_cdmode != WM_CDM_EJECTED) && (! ignore_avoid)) {
	if (cur_ntracks > 1) { /* only if there are more than ONE track */
	    if (cur_track != old_track) {
		if (debug) fprintf(stderr, "-> Track change (current = %d, old = %d)\n", cur_track, old_track);
		redraw = TRUE;
		i = cur_track - 1;
		while ((i <= cur_ntracks) && (cd->trk[i].avoid == 1)) {
		    i ++;
		}

		/* go to the next valid track, only if it is now the
		   current track... */

		if (i != cur_track - 1) {
		    direct_track = i + 1;
		    cd_control(DIRECTTRACK);
		}
	    }
	}
    }

    /* ======================================================================
       Do we *really* need to update the counter? If the cur time is the same
       as last time, we simply exit the redraw function
       ====================================================================== */

    if ((cur_track == old_track) && (lasttime == cur_pos_rel) && (!redraw) && (text_start == 0) && (strlen(led_text) == 0)) return;

    lasttime = cur_pos_rel;

    /* ===================================== */
    /* ===================================== */
    /* ===================================== */
    /* ===================================== */
    /* Dealing with the __real__ redraw now: */
    /* ===================================== */
    /* ===================================== */
    /* ===================================== */
    /* ===================================== */

    if (debug) fprintf(stderr, "-> Redrawing\n");

    fak_flush_expose(Win);
    fak_flush_expose(Iconwin);


    /* ================================ */
    /* draw the text if we have a query */
    /* ================================ */

    /* but but but... If we see a "redraw" query,
       we skip the text! It will be displayed next time
       the function is called... 0.13 */

    if (!redraw) {
	if (strlen(led_text) > 0) {
	    if (text_start == 0) {
		text_start = time(NULL);
		fak_text("", MSG_PANEL, 0, FALSE);
		fak_text(led_text, MSG_PANEL, 0, FALSE);
	    }

	    if (time(NULL) - text_start > text_timeout) {
		text_start = 0;
		redraw = TRUE;
		strcpy(led_text, "");
		fak_text("", MSG_PANEL, 0, FALSE);
	    } else {
		if (!fade_out) return;
	    }
	}
    }

    /* ======================================= */
    /* if WINGs support, update the track list */
    /* ======================================= */

#ifdef WMK
    if (cur_track != old_track) {
	update_track();
	old_track = cur_track;
    }
#endif

    /* ===============================================
       We don't redraw everything if not required,
       so whe ckech fo each element if redraw variable
       is set
       =============================================== */

    /* the background: */

    if (redraw) {

	if (debug) fprintf(stderr, "-> ** GLOBAL REDRAW NEEDED **\n");

	XCopyArea(Disp,backXPM.pixmap,Win,WinGC,0,0,backXPM.attributes.width, backXPM.attributes.height,0,0);
	if (!th_no_icon_window) {
	    XCopyArea(Disp,backXPM.pixmap,Iconwin,WinGC,0,0,backXPM.attributes.width, backXPM.attributes.height,0,0);
	} else {
	    if (strlen(th_icon_window) > 0) {
		XCopyArea(Disp,iconXPM.pixmap,Iconwin,WinGC,0,0,iconXPM.attributes.width, iconXPM.attributes.height,0,0);
	    }
	}
    }

    /* the buttons: */

    for (i = 1; i <= but_max; i++) {

	use_alt = FALSE;

	if ((thdata[i].panel == panel) || (thdata[i].panel == 0)) {
	    if (thdata[i].type == FAK_VCD_BAR) {
		if ( (cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED) ) {
		    if ((time_mode == 2) || (time_mode == 3)) {
			if (cur_cdlen > 0) {
			    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
				      thdata[i].xpm.attributes.width,
				      0 + (int)((float)cur_pos_abs / (float)cur_cdlen * (float)thdata[i].xpm.attributes.height),
				      thdata[i].x,
				      thdata[i].y);
			    if (!th_no_icon_window) {
				XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
					  thdata[i].xpm.attributes.width,
					  0 + (int)((float)cur_pos_abs / (float)cur_cdlen * (float)thdata[i].xpm.attributes.height),
					  thdata[i].x,
					  thdata[i].y);
			    }
			}
		    } else {
			if (cur_tracklen > 0) {
			    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
				      thdata[i].xpm.attributes.width,
				      0 + (int)((float)cur_pos_rel / (float)cur_tracklen * (float)thdata[i].xpm.attributes.height),
				      thdata[i].x,
				      thdata[i].y);
			    if (!th_no_icon_window) {
				XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
					  thdata[i].xpm.attributes.width,
					  0 + (int)((float)cur_pos_rel / (float)cur_tracklen * (float)thdata[i].xpm.attributes.height),
					  thdata[i].x,
					  thdata[i].y);
			    }
			}
		    }
		}

	    } else if (thdata[i].type == FAK_ICD_BAR) {
		if ( (cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED) ) {
		    if ((time_mode == 2) || (time_mode == 3)) {
			if (cur_cdlen > 0) {
			    offset = (int)((float)cur_pos_abs / (float)cur_cdlen * (float)thdata[i].xpm.attributes.height);
			    offset = thdata[i].xpm.attributes.height - offset;

			    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
				      thdata[i].xpm.attributes.width,
				      thdata[i].xpm.attributes.height - offset,
				      thdata[i].x,
				      thdata[i].y +offset);

			    if (!th_no_icon_window) {
				XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
					  thdata[i].xpm.attributes.width,
					  thdata[i].xpm.attributes.height - offset,
					  thdata[i].x,
					  thdata[i].y +offset);
			    }
			}
		    } else {
			if (cur_tracklen > 0) {
			    offset = (int)((float)cur_pos_rel / (float)cur_tracklen * (float)thdata[i].xpm.attributes.height);
			    offset = thdata[i].xpm.attributes.height - offset;

			    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
				      thdata[i].xpm.attributes.width,
				      thdata[i].xpm.attributes.height - offset,
				      thdata[i].x,
				      thdata[i].y +offset);

			    if (!th_no_icon_window) {
				XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
					  thdata[i].xpm.attributes.width,
					  thdata[i].xpm.attributes.height - offset,
					  thdata[i].x,
					  thdata[i].y +offset);
			    }
			}
		    }
		}

	    } else if (thdata[i].type == FAK_CD_BAR) {
		if ( (cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED) ) {
		    if ((time_mode == 2) || (time_mode == 3)) {
			if (cur_cdlen > 0) {
			    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
				      0 + (int)((float)cur_pos_abs / (float)cur_cdlen * (float)thdata[i].xpm.attributes.width),
				      thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
			    if (!th_no_icon_window) {
				XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
					  0 + (int)((float)cur_pos_abs / (float)cur_cdlen * (float)thdata[i].xpm.attributes.width),
					  thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
			    }
			}
		    } else {
			if (cur_tracklen > 0) {
			    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
				      0 + (int)((float)cur_pos_rel / (float)cur_tracklen * (float)thdata[i].xpm.attributes.width),
				      thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
			    if (!th_no_icon_window) {
				XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
					  0 + (int)((float)cur_pos_rel / (float)cur_tracklen * (float)thdata[i].xpm.attributes.width),
					  thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
			    }
			}
		    }
		}

	    } else if (thdata[i].type == FAK_VCD_PIX) { /************** vertical pixmap slider *****************/
		if ((cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED) && (cur_cdlen > 0)) {

		    /* erase the old position : */

		    if (thdata[i].ox > 0) {
			XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC,
				  0, thdata[i].oy,
				  thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				  thdata[i].x,
				  thdata[i].oy + thdata[i].y);

			if (!th_no_icon_window) {
			    XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC,
				      0, thdata[i].oy,
				      thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				      thdata[i].x,
				      thdata[i].oy + thdata[i].y);
			}
		    }

		    /* compute the new geometry: */

		    if ((time_mode == 2) || (time_mode == 3)) fx = (int)((float)cur_pos_abs / (float)cur_cdlen * (float)(thdata[i].xpm.attributes.width - thdata[i].altxpm.attributes.width));
		    else fx = (int)((float)cur_pos_rel / (float)cur_tracklen * (float)(thdata[i].xpm.attributes.width - thdata[i].altxpm.attributes.width));

		    /* ugly, need to CHANGE THIS!!!!! */
		    if (fx < 1) fx = 1;

		    /* and show it: */

		    XCopyArea(Disp, thdata[i].altxpm.pixmap, Win, WinGC,
			      0, 0,
			      thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
			      thdata[i].x,
			      thdata[i].y + fx);

		    if (!th_no_icon_window) {
			XCopyArea(Disp, thdata[i].altxpm.pixmap, Iconwin, WinGC,
				  0, 0,
				  thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				  thdata[i].x,
				  thdata[i].y + fx);
		    }

		    /* finaly, save it! */
		    thdata[i].oy = fx;
		}
	    } else if (thdata[i].type == FAK_CD_PIX) { /**************** horizontal pixmap slider *****************/
		if ((cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED) && (cur_cdlen > 0)) {
		    /* erase the old position : */

		    if (thdata[i].ox > 0) {
			XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC,
				  thdata[i].ox, 0,
				  thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				  thdata[i].ox + thdata[i].x,
				  thdata[i].y);

			if (!th_no_icon_window) {
			    XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC,
				      thdata[i].ox, 0,
				      thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				      thdata[i].ox + thdata[i].x,
				      thdata[i].y);
			}
		    }

		    /* compute the new geometry: */

		    if ((time_mode == 2) || (time_mode == 3)) fx = (int)((float)cur_pos_abs / (float)cur_cdlen * (float)(thdata[i].xpm.attributes.height - thdata[i].altxpm.attributes.height));
		    else fx = (int)((float)cur_pos_rel / (float)cur_tracklen * (float)(thdata[i].xpm.attributes.height - thdata[i].altxpm.attributes.height));

		    /* ugly, need to CHANGE THIS!!!!! */
		    if (fx < 1) fx = 1;

		    /* and show it: */

		    XCopyArea(Disp, thdata[i].altxpm.pixmap, Win, WinGC,
			      0, 0,
			      thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
			      thdata[i].x + fx,
			      thdata[i].y);

		    if (!th_no_icon_window) {
			XCopyArea(Disp, thdata[i].altxpm.pixmap, Iconwin, WinGC,
				  0, 0,
				  thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				  thdata[i].x + fx,
				  thdata[i].y);
		    }

		    /* finaly, save it! */
		    thdata[i].ox = fx;
		}
	    } else if (thdata[i].type == FAK_VOL_BAR) {
		if ( (cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED) ) {
		    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
			      0 + (int)((float)volume / (float)max_volume * (float)thdata[i].xpm.attributes.width),
			      thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
		    if (!th_no_icon_window) {
			XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
				  0 + (int)((float)volume / (float)max_volume * (float)thdata[i].xpm.attributes.width),
				  thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
		    }
		}
	    } else if (thdata[i].type == FAK_VOL_PIX) {
		if ((cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED)) {

		    /* first erase the old position : */

		    if (thdata[i].ox > 0) {
			XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC,
				  thdata[i].ox, 0,
				  thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				  thdata[i].ox + thdata[i].x,
				  thdata[i].y);

			if (!th_no_icon_window) {
			    XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC,
				      thdata[i].ox, 0,
				      thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				      thdata[i].ox + thdata[i].x,
				      thdata[i].y);
			}
		    }

		    /* compute the new geometry: */

		    fx = (int)((float)volume / (float)max_volume * (float)(thdata[i].xpm.attributes.width - thdata[i].altxpm.attributes.width));

		    /* ugly, need to CHANGE THIS!!!!! */
		    if (fx < 1) fx = 1;

		    /* and show it: */

		    XCopyArea(Disp, thdata[i].altxpm.pixmap, Win, WinGC,
			      0, 0,
			      thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
			      thdata[i].x + fx,
			      thdata[i].y);

		    if (!th_no_icon_window) {
			XCopyArea(Disp, thdata[i].altxpm.pixmap, Iconwin, WinGC,
				  0, 0,
				  thdata[i].altxpm.attributes.width, thdata[i].altxpm.attributes.height,
				  thdata[i].x + fx,
				  thdata[i].y);
		    }

		    /* finaly, save it! */
		    thdata[i].ox = fx;
		}
	    } else if (thdata[i].type == FAK_VVOL_BAR) {
		if ( (cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED) ) {
		    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
			      thdata[i].xpm.attributes.width,
			      0 + (int)((float)volume / (float)max_volume * (float)thdata[i].xpm.attributes.height),
			      thdata[i].x,
			      thdata[i].y);
		    if (!th_no_icon_window) {
			XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
			      thdata[i].xpm.attributes.width,
			      0 + (int)((float)volume / (float)max_volume * (float)thdata[i].xpm.attributes.height),
			      thdata[i].x,
			      thdata[i].y);
		    }
		}
	    } else if (thdata[i].type == FAK_IVOL_BAR) {
		if ( (cur_cdmode != WM_CDM_STOPPED) && (cur_cdmode != WM_CDM_EJECTED) ) {
		    offset = (int)((float)volume / (float)max_volume * (float)thdata[i].xpm.attributes.height);
		    offset = thdata[i].xpm.attributes.height - offset;
		    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, offset,
			      thdata[i].xpm.attributes.width,
			      thdata[i].xpm.attributes.height - offset,
			      thdata[i].x,
			      thdata[i].y +offset);
		    if (!th_no_icon_window) {
			XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, offset,
			      thdata[i].xpm.attributes.width,
			      thdata[i].xpm.attributes.height - offset,
			      thdata[i].x,
			      thdata[i].y +offset);
		    }
		}
	    } else if (thdata[i].type == FAK_MIXER_BAR) {
#ifdef MIXER
		XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
			  0 + (int)((float)getvol(thdata[i].arg) / 100.0 * (float)thdata[i].xpm.attributes.width),
			  thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
		if (!th_no_icon_window) {
		    XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
			      0 + (int)((float)getvol(thdata[i].arg) / 100.0 * (float)thdata[i].xpm.attributes.width),
			      thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
		}
#endif
	    } else if (thdata[i].type == FAK_VMIXER_BAR) {
#ifdef MIXER
		XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0,
			  thdata[i].xpm.attributes.width,
			  0 + (int)((float)getvol(thdata[i].arg) / 100.0 * (float)thdata[i].xpm.attributes.height),
			  thdata[i].x,
			  thdata[i].y);
		if (!th_no_icon_window) {
		    XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0,
			      thdata[i].xpm.attributes.width,
			      0 + (int)((float)getvol(thdata[i].arg) / 100.0 * (float)thdata[i].xpm.attributes.height),
			      thdata[i].x,
			      thdata[i].y);
		}
#endif
	    } else if (thdata[i].type == FAK_IMIXER_BAR) {
#ifdef MIXER
		offset = (int)((float)getvol(thdata[i].arg) / 100.0 * (float)thdata[i].xpm.attributes.height);
		offset = thdata[i].xpm.attributes.height - offset;
		XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC,
			  0,
			  offset,
			  thdata[i].xpm.attributes.width,
			  thdata[i].xpm.attributes.height - offset,
			  thdata[i].x,
			  thdata[i].y + offset);
		if (!th_no_icon_window) {
		    XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, offset,
			      thdata[i].xpm.attributes.width,
			      thdata[i].xpm.attributes.height - offset,
			      thdata[i].x,
			      thdata[i].y + offset);
		}
#endif
	    } else {
		if (redraw) {
		    if (strlen(thdata[i].xpm_file) > 0) {
			if (strlen(thdata[i].altxpm_file) == 0) use_alt = FALSE;
			else use_alt = fak_use_alt(i);

			if (use_alt) {
			    if (!th_no_icon_window) {
				XCopyArea(Disp, thdata[i].altxpm.pixmap, Win, WinGC, 0, 0, thdata[i].xpm.attributes.width, thdata[i].altxpm.attributes.height, thdata[i].x, thdata[i].y);
				XCopyArea(Disp, thdata[i].altxpm.pixmap, Iconwin, WinGC, 0, 0, thdata[i].xpm.attributes.width, thdata[i].altxpm.attributes.height, thdata[i].x, thdata[i].y);
			    } else {
				if (thdata[i].icon) {
				    XCopyArea(Disp, thdata[i].altxpm.pixmap, Iconwin, WinGC, 0, 0, thdata[i].xpm.attributes.width, thdata[i].altxpm.attributes.height, thdata[i].x, thdata[i].y);
				} else {
				    XCopyArea(Disp, thdata[i].altxpm.pixmap, Win, WinGC, 0, 0, thdata[i].xpm.attributes.width, thdata[i].altxpm.attributes.height, thdata[i].x, thdata[i].y);
				}
			    }
			} else {
			    if (!th_no_icon_window) {
				XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0, thdata[i].xpm.attributes.width, thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
				XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0, thdata[i].xpm.attributes.width, thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
			    } else {
				if (thdata[i].icon) {
				    XCopyArea(Disp, thdata[i].xpm.pixmap, Iconwin, WinGC, 0, 0, thdata[i].xpm.attributes.width, thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
				} else {
				    XCopyArea(Disp, thdata[i].xpm.pixmap, Win, WinGC, 0, 0, thdata[i].xpm.attributes.width, thdata[i].xpm.attributes.height, thdata[i].x, thdata[i].y);
				}
			    }
			}
		    }
		}
	    }
	}
    }

    redraw = FALSE;

   if (debug) fprintf(stderr, "-> Display [last section]\n");

    /* The Track number */

    /* if ((cur_cdmode == WM_CDM_PLAYING) || (cur_cdmode == WM_CDM_PAUSED)) { */
    if (!th_no_minus) fak_text("      ", COUNTER_PANEL, 0, FALSE);
    else fak_text("     ", COUNTER_PANEL, 0, FALSE);

    /* no CD? no counters! */

    if (cur_cdmode == WM_CDM_EJECTED) {
	if (!th_no_minus) fak_text("AScd", COUNTER_PANEL, 1, FALSE);
	else fak_text("AScd", COUNTER_PANEL, 0, FALSE);
	fak_text("NO CD", MSG_PANEL, 1, TRUE);
        if (debug) fprintf(stderr, "-> No CD. Leaving redraw routine\n");
	return;
    }

    if (debug) fprintf(stderr, "-> Dealing with theme/fast_track selectors\n");

    /* ================== */
    /* The theme selector */
    /* ================== */

    if (theme_select > 0) {
	/* 5 seconds timeout */
	if (time(NULL) - selectors_timeout <= 5) {
	    fak_text("THEME", COUNTER_PANEL, 0, TRUE);
	    sprintf(cdtime, "%02d", theme_select);
	    fak_text(cdtime, TRACK_PANEL, 0, FALSE);

	    i = 0;

	    sprintf(txt, "%s/Themes", THDIR);
	    if ((dir_fd = opendir(txt)) != NULL) {
		i = 0;
		while((dir_pt = readdir(dir_fd)) != NULL) {
		    if (dir_pt->d_name[0] != '.') {
			i++;
			if (i == theme_select) {
			    strcpy(selected_theme, dir_pt->d_name);
			    fak_text(dir_pt->d_name, MSG_PANEL, 0, FALSE);
			}
		    }
		}
		closedir(dir_fd);
	    }
	    return;
	} else {
	    theme_select = 0;
	    redraw = TRUE;
	}
    }

    /* =================== */
    /* Fast Track selector */
    /* =================== */

    if (fast_track == 0) {
	if (cur_track > 0) {
	    /* 15 seconds timeout */
	    if (time_mode != 15) {
		sprintf(cdtime, "%02d", cur_track);
	    } else {
		sprintf(cdtime, "%02d", cur_ntracks - cur_track);
	    }
	} else {
	    sprintf(cdtime, "%02d", cur_ntracks);
	}
        if ((thdata[but_tracknbr].panel == panel) || (thdata[but_tracknbr].panel == 0))
	    fak_text(cdtime, TRACK_PANEL, 0, TRUE);
    } else {
	if (time(NULL) - selectors_timeout <= 3) {
	    sprintf(cdtime, "%02d", fast_track);
	    fak_text("TRACK", COUNTER_PANEL, 0, TRUE);
	    if ((thdata[but_tracknbr].panel == panel) || (thdata[but_tracknbr].panel == 0))
		fak_text(cdtime, TRACK_PANEL, 0, FALSE);
	    return;
	} else {
	    selectors_timeout = 0;
	    fast_track = 0;
	    redraw = TRUE;
	}
    }

    if (debug) fprintf(stderr, "-> Counter\n");

   /* the counter: */

    switch(time_mode) {
    case 0:
	fak_minus();
	disp_time = cur_pos_rel;
	break;
    case 1:
	fak_minus();
	disp_time = cur_tracklen - cur_pos_rel ;
	break;
    case 2:
	fak_minus();
	disp_time = cur_pos_abs;
	break;
    default:
	fak_minus();
	disp_time = cur_cdlen - cur_pos_abs;
	break;
    }

    strcpy(cdtime, "00:00");
    if ((cur_cdmode == WM_CDM_PLAYING) || (cur_cdmode == WM_CDM_PAUSED)) {
	cdtime[0] = 0 ;
	cdtime[1] = (disp_time / 60);
	if (cdtime[1] >= 10) {
	    cdtime[0] = (cdtime[1] / 10);
	    cdtime[1] = (cdtime[1] % 10);
	}
	cdtime[3] = ((disp_time % 60) / 10);
	cdtime[4] = (disp_time % 10);
    } else {
	cdtime[0] = 0;
	cdtime[1] = (cur_cdlen / 60);
	if (cdtime[1] >= 10) {
	    cdtime[0] = (cdtime[1] / 10);
	    cdtime[1] = (cdtime[1] % 10);
	}
	cdtime[3] = ((cur_cdlen % 60) / 10);
	cdtime[4] = (cur_cdlen % 10);
    }

    cdtime[0] = cdtime[0] + 48;
    cdtime[1] = cdtime[1] + 48;
    cdtime[3] = cdtime[3] + 48;
    cdtime[4] = cdtime[4] + 48;

    if (th_no_minus) fak_text(cdtime, 0, 0, FALSE);
    else fak_text(cdtime, 0, 1, FALSE);

    /* ================= */
    /* The auto fade out */
    /* ================= */

    if ((cur_cdmode == WM_CDM_PLAYING) && (fade_out)) {
	if (!fade_ok) {
	    fade_ok = 1;
	    return;
	} else {
	    unmuted_volume = volume ;
	    while (volume > min_volume) {
		volume = volume - fade_step;
		cd_volume(volume, 10, max_volume);
		usleep(dodo);
	    }
	    fade_out = 0;
	    fade_ok = 0;
	    cd_control(PAUSE);
	    old_track = 0;
	    volume = unmuted_volume;
	    cd_volume(volume, 10, max_volume);
	}
    }

    /* ================= */
    /* The auto fade in: */
    /* ================= */

    if ((cur_cdmode == WM_CDM_PAUSED) && (fade_out)){
	if (!fade_ok) {
	    fade_ok = 1;
	    return;
	} else {
	    unmuted_volume = volume;
	    volume = min_volume;
	    cd_volume(volume, 10, max_volume);
	    wm_cd_status();
	    usleep(dodo * 5);
	    cd_control(PAUSE);
	    usleep(dodo * 5);
	    while (volume < max_volume) {
		volume = volume + fade_step;
		cd_volume(volume, 10, max_volume);
		usleep(dodo);
	    }
	    fade_out = 0;
	    fade_ok = 0;
	    volume = unmuted_volume;
	    cd_volume(volume, 10, max_volume);
	}
    }

    /* end of redraw routine */

    old_track = cur_track;
    if (debug) fprintf(stderr, "-> End of global redraw. Leaving redraw routine\n");
}
