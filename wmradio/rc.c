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

#include "rc.h"
#include "const.h"
#include "skin.h"
#include "stationnames.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include "ini.h"

List *ini = NULL;

void rc_set_variable(char *sec, char *var, char *val)
{
    ini = ini_set_variable(ini,sec,var,val);
}

char *rc_get_variable(char *sec, char *var, char *def)
{
    return ini_get_variable(ini,sec,var,def);
}

int rc_get_variable_as_int(char *sec, char *var, int def)
{
    return ini_get_variable_as_int(ini,sec,var,def);
}

void rc_set_variable_as_int(char *sec, char *var, int value)
{
    ini = ini_set_variable_as_int(ini,sec,var,value);
}

void rc_save_config(void)
{
    char rcfile[256];

    setlocale(LC_ALL,"C");
    snprintf(rcfile,sizeof(rcfile),"%s/.wmradio",getenv("HOME"));
    ini_save_file(ini,rcfile);
}

void rc_read_config(void)
{
    char rcfile[256];

    snprintf(rcfile,sizeof(rcfile),"%s/.wmradio",getenv("HOME"));
    ini = ini_load_file(NULL,"config",rcfile);
}

void rc_set_variable_as_pseudofloat(char *section, char *variable, int value)
{
    IniVariable *i;

    ini = ini_set_variable(ini,section,variable,freqtostr(value));
    i = ini_get_variable_ini_variable(ini,section,variable);
    if(i) i->value_int = value;
}

int rc_get_variable_as_pseudofloat(char *section, char *variable, int dflt)
{
    IniVariable *i;

    i = ini_get_variable_ini_variable(ini,section,variable);
    if(!i) return dflt;
    if(i->value_int == INI_INT_UNDEFINED) i->value_int = ato100i(i->value);
    return i->value_int;
}

int rc_get_freq(int index)
{
    char buffer[10];

    snprintf(buffer,sizeof(buffer),"preset%i",index);
    return rc_get_variable_as_pseudofloat(SECTION_CONFIG,buffer,9700);
}

void rc_set_freq(int index, int value)
{
    char buffer[10];

    snprintf(buffer,sizeof(buffer),"preset%i",index);
    rc_set_variable_as_pseudofloat(SECTION_CONFIG,buffer,value);
}

IniVariable *rc_get_variable_ini_variable(char *section, char *variable)
{
    return ini_get_variable_ini_variable(ini,section,variable);
}

IniSection *rc_get_section(char *section)
{
    return ini_get_section(ini,section);
}

void rc_free_config(void)
{
    ini_free(ini);
}
