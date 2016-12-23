/*
 * Copyright (C) 12 Jun 2003 Tomas Cermak
 *
 * This file is part of wmradio program.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <sys/time.h>
#include <unistd.h>
#include <locale.h>
#include <math.h>
#include "config.h"
#include "const.h"
#include "skin.h"
#include "radio.h"
#include "wmradio.h"
#include "stationnames.h"
#include "rc.h"

#include "osd.h"
#include "fifo.h"

#ifdef GNOME_RADIO
#include "gnome_applet_envelope.h"
#else
#include "wm_envelope.h"
#endif
static int min_freq = 8750, max_freq = 10800;
RadioInfo radio_info;


RadioInfo *wmradio_radio_info(void)
{
    return &radio_info;
}

void wmradio_init_radio_info(void) {
    radio_info.is_on = 0;
    radio_info.current_station = 0;
    radio_info.dont_quit_mode = 0;
}

int open_radio()
{
    radio_info.radiofd = open(rc_get_variable(SECTION_CONFIG,"device","/dev/radio"),O_RDONLY);
    return radio_info.radiofd>=0;
}

void close_radio()
{
    if(radio_info.radiofd != -1) close(radio_info.radiofd);
}

void radio_refresh_freq (void) {
    char str[30];

    radio_setfreq(radio_info.radiofd,radio_info.current_freq);
    if(rc_get_variable_as_int(SECTION_CONFIG,"osd",0)) {
	sprintf(str, "%.2i FM", radio_info.current_freq);
	osd_print(str);
    }
}

void tuned_to_preset_station(void)
{
    int i;
    int x,y;

    x = radio_info.current_freq;
    for(i = 0; i<6; i++) {
	y = rc_get_freq(i);
	if(x == y) {
	    skin_select_station(i);
	    return;
	}
    }
}

void tune_plus(int fine_tuning)
{
    if(fine_tuning) {
        radio_info.current_freq += 1;
    } else {
        radio_info.current_freq += 10;
    };
    if(radio_info.current_freq>max_freq) radio_info.current_freq = min_freq;
    radio_refresh_freq();
}

void tune_minus(int fine_tuning)
{
    if(fine_tuning) {
        radio_info.current_freq -= 1;
    } else {
        radio_info.current_freq -= 10;
    };
    if(radio_info.current_freq<min_freq) radio_info.current_freq = max_freq;
    radio_refresh_freq();
}

void wmradio_scan(int start)
{
    static char starting = 0;
    int signal;

    switch(start) {
    case SCAN_START:
        starting = radio_info.scan_in_progress = 1;
	break;
    case SCAN_STOP:
	radio_info.scan_in_progress = 0;
	tuned_to_preset_station();
    };
    if(!radio_info.scan_in_progress) return;
    tune_plus(0);
    usleep(100000);
    signal = radio_getsignal(radio_info.radiofd);
    if(starting) {
        if( signal<2 ) starting = 0;
    } else {
        if( signal>2 ) {
	    radio_info.scan_in_progress = 0;
	    tuned_to_preset_station();
	}
    }
}

void wmradio_next_station(void)
{
    radio_info.current_freq = station_next_freq(radio_info.current_freq);
    radio_refresh_freq();
}

void wmradio_prev_station(void)
{
    radio_info.current_freq = station_prev_freq(radio_info.current_freq);
    radio_refresh_freq();
}

#define MOD(A,B) ((A)<0)? (B)+((A)%(B)) : (A)%(B)

void wmradio_command(RadioCommand command, int value) {
    switch (command) {
    case POWER_SWITCH:
	if( (value && radio_info.dont_quit_mode)
	    || (!value && !radio_info.dont_quit_mode)){
	    if (radio_info.is_on) {
		radio_mute(radio_info.radiofd);
		close_radio();
	    }
	    /* rc_save_config(); */
	    video_close();
	    return;
	}
	if (radio_info.is_on) {
	    if(rc_get_variable_as_int(SECTION_CONFIG,"osd",0)) {
		osd_print("FM off");
	    }
	    radio_mute(radio_info.radiofd);
	    close_radio();
	    skin_switch_radio(SKIN_OFF);
	    radio_info.is_on = 0;
	    /* rc_save_config(); */
	} else {
	    if(open_radio()) {
		radio_refresh_freq();
		radio_unmute(radio_info.radiofd);
		skin_switch_radio(SKIN_ON);
		radio_info.is_on = 1;
	    } else {
		printf("wmradio: can't open radio device\n");
#ifdef ONLY_TEST
		/* those next lines are for tests on my notebook */
		/* without /dev/rario                            */
		printf("         but radio is compiled with ONLY_TEST\n");
		radio_refresh_freq();
		radio_unmute(radio_info.radiofd);
		skin_switch_radio(SKIN_ON);
		radio_info.is_on = 1;
#endif /* ONLY_TEST */
	    }
	}
        break;
    case TUNE_MINUS:
	if(radio_info.is_on) {
	    tune_minus(value);
	    skin_unselect_button();
	    tuned_to_preset_station();
	}
        break;
    case TUNE_PLUS:
	if(radio_info.is_on) {
	    tune_plus(value);
	    skin_unselect_button();
	    tuned_to_preset_station();
	}
        break;
    case SET_PRESET:
	if(radio_info.is_on){
	    radio_info.current_station = value;
	    radio_info.current_freq = rc_get_freq(value);
	    radio_refresh_freq();
	    skin_select_station(value);
	}
        break;
    case SAVE_PRESET:
	if(radio_info.is_on){
	    rc_set_freq(value,radio_info.current_freq);
	    skin_select_station(value);
	    rc_save_config();
	}
	break;
    case SCAN:
	if(radio_info.is_on){
	    wmradio_scan(SCAN_START);
	    skin_unselect_button();
	}
        break;
    case TUNE_NAME_PREV:
	if(radio_info.is_on){
	    wmradio_prev_station();
	    skin_unselect_button();
	    tuned_to_preset_station();
	}
	break;
    case TUNE_NAME_NEXT:
	if(radio_info.is_on){
	    wmradio_next_station();
	    skin_unselect_button();
	    tuned_to_preset_station();
	}
	break;
    case READ_CONFIG:
	rc_free_config();
	rc_read_config();
	radio_info.dont_quit_mode = rc_get_variable_as_int(SECTION_CONFIG,"dont-quit-mode",0);
	break;
    }
}

void wmradio_handle_event(RadioEvent *e)
{
    int i;
    int presetindex;
/*     int stations[] = {5829,5830,5831,5824,5825,5826};*/

    switch (e->type) {
    case REVENT_EXPOSE:
	video_draw(radio_info.current_freq,radio_getstereo(radio_info.radiofd));
	break;
    case REVENT_QUIT:
	if(radio_info.is_on) radio_mute(radio_info.radiofd);
	close_radio();
	/* rc_save_config(); */
	video_close();
	break;
    case REVENT_BUTTON_PRESS:
	skin_mouse_event(e->x,e-> y,e->button,1);
	video_draw(radio_info.current_freq,radio_getstereo(radio_info.radiofd));
	break;
    case REVENT_TIMER:
	wmradio_scan(SCAN_NO_CHANGE);
	video_draw(radio_info.current_freq,radio_getstereo(radio_info.radiofd));
	break;
    case REVENT_BUTTON_RELEASE:
	setlocale(LC_ALL,"C");
	wmradio_scan(SCAN_STOP);
	i = skin_mouse_event(e->x, e->y,e->button, 0);
	presetindex = -1;
	if( (e->button == 4 || e->button == 5) ) {
	} else {
	    /* this is not mouse wheel */
	    if (i) {
		switch (i) {
		case XOR_OFF:
                    wmradio_command(POWER_SWITCH, e->control);
		    break;
		case XOR_TUNEm:
		    if(e->shift) { wmradio_command(TUNE_NAME_PREV, e->control); }
		    else { wmradio_command(TUNE_MINUS, e->control); }
		    break;
		case XOR_TUNEp:
		    if(e->shift) { wmradio_command(TUNE_NAME_NEXT, e->control); }
		    else { wmradio_command(TUNE_PLUS, e->control); }
		    break;
		case XOR_SCAN:
                    wmradio_command(SCAN, 0);
		    break;
		case XOR_PRESET1:
		    presetindex = 0;
		    break;
		case XOR_PRESET2:
		    presetindex = 1;
		    break;
		case XOR_PRESET3:
		    presetindex = 2;
		    break;
		case XOR_PRESET4:
		    presetindex = 3;
		    break;
		case XOR_PRESET5:
		    presetindex = 4;
		    break;
		case XOR_PRESET6:
		    presetindex = 5;
		    break;
		};
		if(presetindex >= 0) {
		    if( e->control ) {
			wmradio_command(SAVE_PRESET,presetindex);
		    } else {
			wmradio_command(SET_PRESET,presetindex);
		    }
		    radio_info.current_station = presetindex;
		}
	    }
	}
	video_draw(radio_info.current_freq,
		   radio_getstereo(radio_info.radiofd));
	break;
    case REVENT_SCROLL_UP:
	skin_mouse_event(e->x, e->y,e->button, 1);
	i = skin_mouse_event(e->x, e->y,e->button, 0);
	if(i == 4507)
	    if(e->shift) {
		wmradio_command(TUNE_MINUS, e->control);
	    } else {
		wmradio_command(TUNE_NAME_PREV, e->control);
	    }
	else
	    wmradio_command(SET_PRESET,
			    MOD(radio_info.current_station-1, 6));
	break;
    case REVENT_SCROLL_DOWN:
	skin_mouse_event(e->x, e->y,e->button, 1);
	i = skin_mouse_event(e->x, e->y,e->button, 0);
	if(i == 4507)
	    if(e->shift) {
		wmradio_command(TUNE_PLUS, e->control);
	    } else {
		wmradio_command(TUNE_NAME_NEXT, e->control);
	    }
	else
	    wmradio_command(SET_PRESET,
			    MOD(radio_info.current_station+1, 6));
	break;
    }
    fifo_parse();
}

int wmradio_init(void)
{
    if (rc_get_variable_as_int(SECTION_CONFIG,"osd",0) ){
	if (! osd_init("wmradio",
		       rc_get_variable(SECTION_CONFIG,"osd-font","*-courier-*"),
		       rc_get_variable(SECTION_CONFIG,"osd-color","green"),
		       rc_get_variable_as_int(SECTION_CONFIG,"osd-position",10),
		       rc_get_variable_as_int(SECTION_CONFIG,"osd-position",10),
		       rc_get_variable_as_int(SECTION_CONFIG,"osd-shadow-offset",5),
		       rc_get_variable_as_int(SECTION_CONFIG,"osd-timeout",3)
		      )
	    ) {
	    printf("osd init failed\n");
	    rc_set_variable_as_int(SECTION_CONFIG,"osd",0);
	}
    }
    fifo_init();
    skin_switch_radio(SKIN_OFF);
    radio_info.is_on = 0;
    radio_info.dont_quit_mode =  rc_get_variable_as_int(SECTION_CONFIG,"dont-quit-mode",1);
    radio_info.current_freq = rc_get_freq(radio_info.current_station);
    if(! rc_get_variable_as_int(SECTION_CONFIG,"start-muted",1) ) {
	if(open_radio()) {
	    radio_setfreq(radio_info.radiofd,radio_info.current_freq);
	    radio_unmute(radio_info.radiofd);
	    skin_switch_radio(SKIN_ON);
	    radio_info.is_on = 1;
	} else {
	    printf("wmradio: can't open radio device\n");
	    radio_info.is_on = 0;
	}
    }
    return 1;
}

void wmradio_done(void)
{
    osd_close();
    fifo_close();
}

