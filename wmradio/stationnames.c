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

#include "stationnames.h"
#include "rc.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define STATION_NAMES_INTERVAL 0.03

char *freqtostr(int freq)
{
    static char buffer[10];
    
    sprintf(buffer, "%i.%.2i\n", freq / 100,freq % 100);
    return buffer;
}

int ato100i(char *str)
{
    char *p,*q;
    int result,tmp;
    
    p = strstr(str,".");
    if(!p) {
	return atoi(str) * 100;
    }
    q = p; q++;
    *p = 0;
    result = atoi(str) * 100;
    tmp = atoi(q);
    *p = '.';
    
    switch(strlen(q)) {
    case 1:
	result += 10 * tmp;
	break;
    case 2:
	result += tmp;
    }
    return result;
}

IniVariable *station_find_by_name(char *name)
{
    return rc_get_variable_ini_variable(SECTION_NAMES,name);
}

IniVariable *station_find_by_freq(int freq)
{
    IniSection *section;
    IniVariable *variable;
    List *item;
    int cfreq;
    
    section = rc_get_section(SECTION_NAMES);
    if(!section) return NULL;
    for(item = section->variables; item; item = item->next) {
	variable = item->data;
	cfreq = ato100i(variable->value);
	if((cfreq > freq - STATION_NAMES_INTERVAL) &&(cfreq < freq + STATION_NAMES_INTERVAL))
	    return variable;
    }
    return NULL;
}

void station_add(char *name, int freq)
{
    rc_set_variable(SECTION_NAMES,name,freqtostr(freq));
}

char *station_get_freq_name(int freq)
{
    IniVariable *variable;
    
    variable = station_find_by_freq(freq);
    if(!variable) return NULL;
    return variable->name;
}

void station_set_freq_name(int freq, char *name)
{
    rc_set_variable(SECTION_NAMES,name,freqtostr(freq));
}

List *station_nearest(int freq)
{
    IniSection *section;
    List *item,*current;
    IniVariable *item_var;
    int delta; 
   
    section = rc_get_section(SECTION_NAMES);
    if(! section ) return NULL;
    item = current = section -> variables;
    if(!item) return NULL;
    item_var = item->data;
    delta = abs(freq - ato100i(item_var->value));
    while(item) {
	item_var = item->data;
	if(abs(freq - ato100i(item_var->value)) < delta) {
	    current = item;
	    delta = abs(freq - ato100i(item_var->value));
	}
	item = item->next;
    }
    return current;
}

int station_next_freq(int freq)
{
    IniVariable *item_var;
    List *item;
    
    item = station_nearest(freq);
    if(!item) return freq;
    if(item->next) {
	item_var = item->next->data;
	return ato100i(item_var->value);
    }
    while(item->prev) item = item->prev;
    item_var = item->data;
    return ato100i(item_var->value);
}

int station_prev_freq(int freq)
{
    IniVariable *item_var;
    List *item;
    
    item = station_nearest(freq);
    if(!item) return freq;
    if(item->prev) {
	item_var = item->prev->data;
	return ato100i(item_var->value);
    }
    while(item->next) item = item->next;
    item_var = item->data;
    return ato100i(item_var->value);
}

