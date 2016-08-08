/*
 * wmmp3
 * Copyright (c)1999 Patrick Crosby <xb@dotfiles.com>.
 * This software covered by the GPL.  See COPYING file for details.
 *
 * mpg123ctl.h
 *                                                                
 * Header file for mpg123ctl.c 
 *                                                                
 * $Id: mpg123ctl.h,v 1.6 1999/10/12 03:03:33 pcrosby Exp $
 */

#ifndef __MPG123CTL__

#define __MPG123CTL__

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include "wmgeneral.h"
#include "buttons.h"
#include "song_hash.h"

int is_playing();
void user_play();
void stop();
void next();
void back();
void set_mpg123(char *s);
void set_mp3ext(char *s);
void set_playlistext(char *s);
void set_alwaysscroll(char *s);
void add_mp3dir(char *s);
void add_mp3dirname(char *s);
void dir_up(int button_num);
void dir_down(int button_num);
void turn_on_scroll();
void random_toggle(int button_num);
void repeat_toggle(int button_num);

#endif


