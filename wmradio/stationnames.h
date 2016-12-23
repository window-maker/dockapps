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

#ifndef _STATIONNAMES_H_
#define _STATIONNAMES_H_
#include <stdio.h>
#include "ini.h"

char *freqtostr(int freq);
int ato100i(char *str);
void station_add(char *name, int freq);
char *station_get_freq_name(int freq);
void station_set_freq_name(int freq, char *name);
List *station_nearest(int freq);
int station_next_freq(int freq);
int station_prev_freq(int freq);


#endif
