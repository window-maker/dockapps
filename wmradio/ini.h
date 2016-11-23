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

#ifndef _INI_H
#define _INI_H

#include <stdlib.h>
#include <stdio.h>
#include "lists.h"

#define INI_INT_UNDEFINED (-154257)

typedef struct {
    char *name;
    List *variables;
} IniSection;

typedef struct {
    char *name;
    char *value;
    int value_int;
} IniVariable;

List *ini_get_section_node(List *ini, char *name);
IniSection *ini_get_section(List *ini, char *name);
List *ini_get_variable_node(List *ini, char *section, char *variable);
IniVariable *ini_get_variable_ini_variable(List *ini, char *section, char *variable);
char *ini_get_variable(List *ini, char *section, char *variable, char *dflt);
int ini_get_variable_as_int(List *ini, char *section, char *variable, int dflt);
List *ini_set_variable(List *ini, char *section, char *variable, char *value);
List *ini_set_variable_as_int(List *ini, char *section, char *variable, int value);
void ini_print(FILE *f, List *ini);
void ini_save_file(List *ini, char *filename);
List *ini_load_file(List *ini, char *default_section, char *name);

void ini_delete_variable_node(List *ini, List *node);
void ini_delete_variable(List *ini, char *section, char *variable);
List *ini_delete_section(List *ini, char *section);
List *ini_delete_section_node(List *ini, List *section);
void ini_free(List *ini);

#endif
