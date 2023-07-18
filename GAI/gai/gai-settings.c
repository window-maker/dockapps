/*
 * GAI - General Applet Interface Library
 * Copyright (C) 2003-2004 Jonas Aaberg <cja@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * - settings wrappers
 */

#include "../config.h"
#include "gai.h"
#include "gai-private.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef GAI_WITH_GNOME
#include <libgnome/libgnome.h>
#else
#include "gai-gnome-config.h"
#endif


void
gai_save_glist(const char *name, GList *data)
{
    char *tmp;
    int i;


    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);

    tmp = g_strdup_printf("%s_items",name);
    gai_save_int(tmp,g_list_length(data));
    g_free(tmp);

    for(i=0;i<g_list_length(data);i++){
	tmp = g_strdup_printf("%s_%.3d",name,i);
	gai_save_string(tmp,g_list_nth_data(data,i));
	g_free(tmp);
    }

    GAI_LEAVE;
}



void 
gai_save_int(const char *name, int data)
{
    char *tmp;

    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);

    tmp = g_strdup_printf("%s/", GAI.applet.name);
    gnome_config_push_prefix(tmp);
    g_free(tmp);

    gnome_config_set_int(name,data);

    gnome_config_sync();
    gnome_config_drop_all();
    gnome_config_pop_prefix();
    GAI_LEAVE;
}

void 
gai_save_bool(const char *name, int data)
{
    g_assert(name != NULL);
    gai_save_int (name, data);
}

void 
gai_save_float(const char *name, float data)
{
    char *tmp;

    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);

    tmp = g_strdup_printf("%s/", GAI.applet.name);
    gnome_config_push_prefix(tmp);
    g_free(tmp);

    gnome_config_set_float(name,data);

    gnome_config_sync();
    gnome_config_drop_all();
    gnome_config_pop_prefix();
    GAI_LEAVE;
}


void gai_save_string(const char *name, const char *data)
{
    char *tmp;

    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);
    g_assert(data != NULL);

    tmp = g_strdup_printf ("%s/", GAI.applet.name);
    gnome_config_push_prefix (tmp);
    g_free (tmp);

    gnome_config_set_string (name, data);

    gnome_config_sync();
    gnome_config_drop_all();
    gnome_config_pop_prefix();
    GAI_LEAVE;
}


void
gai_save_gaicolor(const char *name, GaiColor data)
{
    char *tmp;

    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);

    tmp = g_strdup_printf("%s/", GAI.applet.name);
    gnome_config_push_prefix(tmp);
    g_free(tmp);

    tmp = g_strdup_printf("%s_red", name);

    gnome_config_set_int(tmp, (int)data.r);
    g_free(tmp);

    tmp = g_strdup_printf("%s_green", name);
    gnome_config_set_int(tmp, (int)data.g);
    g_free(tmp);

    tmp = g_strdup_printf("%s_blue", name);
    gnome_config_set_int(tmp,(int)data.b);
    g_free(tmp);

    tmp = g_strdup_printf("%s_alpha", name);
    gnome_config_set_int(tmp,(int)data.alpha);
    g_free(tmp);


    gnome_config_sync();
    gnome_config_drop_all();
    gnome_config_pop_prefix();
    GAI_LEAVE;
}


GList *
gai_load_glist_with_default(const char *name, GList *default_list)
{
    GList *data = NULL;
    int i, j;
    char *tmp;

    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);

    tmp = g_strdup_printf ("%s/", GAI.applet.name);
    gnome_config_push_prefix (tmp);
    g_free (tmp);

    tmp = g_strdup_printf("%s_items", name);
    j = gai_load_int_with_default(tmp, 0);
    g_free(tmp);

    if(!j){
	if(default_list != NULL){
	    for(i=0;i<g_list_length(default_list);i++){
		data = g_list_append(data, g_list_nth_data(default_list,i));
	    }
	}
    } else {

	for(i=0;i<j;i++){
	    tmp = g_strdup_printf("%s_%.3d",name,i);
	    data = g_list_append(data, gai_load_string_with_default(tmp, ""));
	    g_free(tmp);
	}
    }

    gnome_config_pop_prefix();
    GAI_LEAVE;
    return data;
}


char *gai_load_string_with_default(const char *name, const char *valdefault)
{
    char *tmp, *str;

    GAI_ENTER; gai_is_init();
    g_assert(name != NULL);
    g_assert(valdefault != NULL);

    tmp = g_strdup_printf ("%s/", GAI.applet.name);
    gnome_config_push_prefix(tmp);
    g_free (tmp);

    tmp = g_strdup_printf ("%s=%s", name, valdefault);
    str = gnome_config_get_string_with_default (tmp, NULL);
    g_free (tmp);

    gnome_config_pop_prefix();
    GAI_LEAVE;
    return str;
}

int 
gai_load_int_with_default (const char *name, int valdefault)
{
    int i;
    char *tmp;

    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);

    tmp = g_strdup_printf("%s/",GAI.applet.name);
    gnome_config_push_prefix (tmp);
    g_free (tmp);

    tmp = g_strdup_printf ("%s=%d", name, valdefault);
    i = gnome_config_get_int_with_default (tmp, NULL);
    g_free (tmp);

    gnome_config_pop_prefix();
    GAI_LEAVE;
    return i;
}

int 
gai_load_bool_with_default (const char *name, int valdefault)
{
    return gai_load_int_with_default(name, valdefault);
}


float 
gai_load_float_with_default(const char *name, float valdefault)
{
    float i;
    char *tmp;

    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);

    tmp = g_strdup_printf("%s/", GAI.applet.name);
    gnome_config_push_prefix(tmp);
    g_free(tmp);

    tmp = g_strdup_printf ("%s=%f", name, valdefault);
    i = gnome_config_get_float_with_default (tmp, NULL);
    g_free(tmp);

    gnome_config_pop_prefix();
    GAI_LEAVE;
    return i;
}


GaiColor 
gai_load_gaicolor_with_default(const char *name, GaiColor valdefault)
{
    GaiColor i;
    char *tmp;

    GAI_ENTER;  gai_is_init();
    g_assert(name != NULL);

    tmp = g_strdup_printf("%s/", GAI.applet.name);
    gnome_config_push_prefix(tmp);
    g_free(tmp);

    tmp = g_strdup_printf ("%s_red=%d", name, (int)valdefault.r);
    i.r = gnome_config_get_int_with_default (tmp, NULL);
    g_free(tmp);

    tmp = g_strdup_printf ("%s_green=%d", name, (int)valdefault.g);
    i.g = gnome_config_get_int_with_default (tmp, NULL);
    g_free(tmp);

    tmp = g_strdup_printf ("%s_blue=%d", name, (int)valdefault.b);
    i.b = gnome_config_get_int_with_default (tmp, NULL);
    g_free(tmp);

    tmp = g_strdup_printf ("%s_alpha=%d", name, (int)valdefault.alpha);
    i.alpha = gnome_config_get_int_with_default (tmp, NULL);
    g_free(tmp);

    gnome_config_pop_prefix();
    GAI_LEAVE;
    return i;
}



static char *gai_settings_fix_name(const char *name)
{
    char *full_name, *new_name;
    int i;

    new_name = g_malloc0(strlen(name)+1);

    for(i=0;i<strlen(name);i++){
	if(name[i]=='/') new_name[i] = '_';
	else new_name[i] = name[i];
    }

    full_name = g_strdup_printf("%s/.gnome2/%s",getenv("HOME"),new_name);

    g_free(new_name);

    return full_name;
}

unsigned char *gai_load_raw_data(const char *name, int *size)
{
    int new_size;
    char  *full_name, *data; 
    FILE *raw_data_file;

    full_name = gai_settings_fix_name(name);

    raw_data_file = fopen(full_name, "r");
    g_free(full_name);

    if(raw_data_file == NULL)
	return NULL;
    
    fseek(raw_data_file,0,SEEK_END);
    new_size = ftell(raw_data_file);
    fseek(raw_data_file,0,SEEK_SET);


    data = g_malloc0(new_size);

    if(size !=NULL)
	(*size) = new_size;

    fread(data,1,new_size,raw_data_file);

    fclose(raw_data_file);
    return data;

}

void gai_save_raw_data(const char *name, unsigned const char *data, int size)
{
    char  *full_name;
    int new_size;
    FILE *raw_data_file;

    full_name = gai_settings_fix_name(name);

    raw_data_file = fopen(full_name, "w+");
    g_free(full_name);

    if(raw_data_file == NULL){
	perror(_("Error open raw data:"));
	gai_display_error_continue(_("Error open raw data! Permissions ok?"));
	return;
    }
    new_size = fwrite(data,1,size,raw_data_file);

    if(new_size != size){
	perror(_("Error saving raw data!"));
	fclose(raw_data_file);
	gai_display_error_continue(_("Error saving raw data! Diskfull? Permissions?"));
	return;
    }
    fclose(raw_data_file);

}


/*
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
