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

#ifndef _WMRADIO_H_
#define _WMRADIO_H_
#include "config.h"

#define VERSION "0.9"
#define PACKAGE "wmradio"

#define SCAN_NO_CHANGE 0
#define SCAN_START 1
#define SCAN_STOP 2

#define REVENT_MOUSE_MOVE 1
#define REVENT_BUTTON_PRESS 2
#define REVENT_BUTTON_RELEASE 3
#define REVENT_TIMER 4
#define REVENT_QUIT 5
#define REVENT_EXPOSE 6
#define REVENT_SCROLL_UP 7
#define REVENT_SCROLL_DOWN 8

#define CONTROL_STATE_PRESSED 1
#define CONTROL_STATE_NOT_PRESSED 0

typedef
    struct {
	char type;
	unsigned int x;
	unsigned int y;
	char button;
	char control;
	char shift;
    } RadioEvent;

typedef
    struct {
	int radiofd;
	char is_on;
	char current_station;
	int current_freq;
	char scan_in_progress;
	char dont_quit_mode;
    } RadioInfo;

typedef enum {
    POWER_SWITCH,
    TUNE_MINUS,
    TUNE_PLUS,
    SET_PRESET,
    SAVE_PRESET,
    SCAN,
    TUNE_NAME_PREV,
    TUNE_NAME_NEXT,
    READ_CONFIG
} RadioCommand;

void wmradio_handle_event(RadioEvent *e);
void wmradio_command(RadioCommand command, int value);
void wmradio_next_station(void);
void wmradio_prev_station(void);
RadioInfo *wmradio_radio_info(void);
void wmradio_init_radio_info(void);
int wmradio_init(void);
void wmradio_done(void);

#endif
