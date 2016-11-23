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

#ifndef _RC_H_
#define _RC_H_
#include "ini.h"

#define SECTION_CONFIG "config"
#define SECTION_NAMES  "names"

void  rc_read_config(void);
void rc_free_config(void);
void  rc_save_config(void);
void  rc_set_variable(char *sec, char *var, char *val);
char *rc_get_variable(char *sec, char *var, char *def);
int   rc_get_variable_as_int(char *sec, char *var, int def);
void  rc_set_variable_as_int(char *sec, char *var, int value);
void  rc_set_variable_as_pseudofloat(char *section, char *variable, int value);
int   rc_get_variable_as_pseudofloat(char *section, char *variable, int dflt);
int   rc_get_freq(int index);
void rc_set_freq(int index, int value);
IniVariable *rc_get_variable_ini_variable(char *section, char *variable);
IniSection *rc_get_section(char *section);
void rc_free_config(void);

#endif
