#include "ini.h"
#include "lists.h"
#include <string.h>
#include <stdio.h>


List *ini_section_add(List *ini, char *name)
{
    char *c;
    IniSection *i;
    
    c = strdup(name);
    if(!c) return ini;
    i = (IniSection *)malloc(sizeof(IniSection));
    if(!i) return ini;
    i->name = c;
    i->variables = NULL;
    ini = list_add_data(ini,i);
    return ini;
}

List *ini_get_section_node(List *ini, char *name)
{
    List *item;
    IniSection *i;
    
    item = ini;
    while(item) {
	i = item -> data;
	if(strcmp(name, i->name) == 0) return item;
	item = item->next;
    }
    return NULL;
}

IniSection *ini_get_section(List *ini, char *name)
{
    List *item;
    
    item = ini_get_section_node(ini,name);
    if(!item) return NULL;
    return (IniSection *)(item->data);
}

List *ini_get_variable_node(List *ini, char *section, char *variable)
{
    IniSection *sec;
    IniVariable *ini_i;
    List *item;
    
    sec = ini_get_section(ini, section);
    if(!sec) return NULL;
    item = sec->variables;
    while(item) {
	ini_i = item->data;
	if(strcmp(variable,ini_i->name) == 0) return item;
	item = item->next;
    }
    return NULL;
}

IniVariable *ini_get_variable_ini_variable(List *ini, char *section, char *variable)
{
    List *item;
    
    item = ini_get_variable_node(ini,section,variable);
    if(!item) return NULL;
    return (IniVariable *)(item->data);
}

char *ini_get_variable(List *ini, char *section, char *variable, char *dflt)
{
    IniVariable *i;

    i = ini_get_variable_ini_variable(ini,section,variable);
    if(!i) return dflt;
    return i->value;
}

int ini_get_variable_as_int(List *ini, char *section, char *variable, int dflt)
{
    IniVariable *i;

    i = ini_get_variable_ini_variable(ini,section,variable);
    if(!i) return dflt;
    if(i->value_int == INI_INT_UNDEFINED) i->value_int = atoi(i->value);
    return i->value_int;
}

void ini_variable_add_empty(List *ini, char *section, char *variable)
{
    List *item;
    IniSection *sec;
    IniVariable *var;
    
    item = ini_get_section_node(ini,section);
    if(!item) {
	ini = ini_section_add(ini,section);
	item = ini_get_section_node(ini,section);
    }
    if(!item) return;
    
    sec = item->data;
    
    var = (IniVariable *)malloc(sizeof(IniVariable));
    if(!var) return;
    var->name = strdup(variable);
    var->value = NULL;
    var->value_int = INI_INT_UNDEFINED;
    
    sec->variables = list_add_data(sec->variables,var);
}

List *ini_set_variable(List *ini, char *section, char *variable, char *value)
{
    List *item;
    IniVariable *var;
    
    item = ini_get_section_node(ini,section);
    if(!item) {
	ini = ini_section_add(ini,section);
	item = ini_get_section_node(ini,section);
    }
    if(!item) return ini;
    
    item = ini_get_variable_node(ini,section,variable);
    if(!item) {
	ini_variable_add_empty(ini,section,variable);
	item = ini_get_variable_node(ini,section,variable);
    }
    if(!item) return ini;
    
    var = item->data;
    
    free(var->value);
    var->value = strdup(value);
    return ini;
}

List *ini_set_variable_as_int(List *ini, char *section, char *variable, int value)
{
    IniVariable *i;
    char value_str[256];
    
    snprintf(value_str,sizeof(value_str),"%i",value);
    ini = ini_set_variable(ini,section,variable,value_str);
    i = ini_get_variable_ini_variable(ini,section,variable);
    if(i) i->value_int = value;
    return ini;
}

void ini_print(FILE *f, List *ini) {
    List *section_item,*var_item;
    IniSection *section_data;
    IniVariable *ini_data;
    
    for(section_item = ini; section_item; section_item = section_item->next) {
	section_data = section_item->data;
	fprintf(f,"[%s]\n",section_data->name);
	for(var_item = section_data->variables; var_item; var_item = var_item->next) {
	    ini_data = var_item->data;
	    fprintf(f,"%s=%s\n",ini_data->name,ini_data->value);
	}
    }
}

void ini_save_file(List *ini, char *filename)
{
    FILE *f;
    
    f = fopen(filename,"w");
    ini_print(f,ini);
    fclose(f);
}

void ini_readline(FILE *f,char *buffer,int size)
{
    char *p;
    
    fgets(buffer,size-1,f);
    p = strstr(buffer,"\n");
    if(p) *p = 0;
    p = strstr(buffer,";");
    if(p) *p = 0;    
    p = strstr(buffer,"#");
    if(p) *p = 0;
}

char *ini_buffer_to_section(char *buffer)
{
    char *b,*e;
    
    b = strstr(buffer,"[");
    if(!b) return NULL;
    e = strstr(b,"]");
    if(!e) return NULL;
    *b = 0;
    *e = 0;
    b++;
    return b;
}

void ini_buffer_to_variable(char *buffer, char **var, char **val)
{
    char *p;
    
    p = strstr(buffer,"=");
    if(!p) {
	*var = NULL;
	*val = NULL;
	return;
    }
    *p = 0;
    p++;
    *var = buffer;
    *val = p;
}

List *ini_load_file(List *ini, char *default_section, char *name)
{
    FILE *f;
    char buffer[1024],current_section[100];
    char *section, *var, *val;
    
    memset(current_section,0,sizeof(current_section));
    if(default_section) strncpy(current_section,default_section,sizeof(current_section)-1);
    f = fopen(name,"r");
    if(!f) return NULL;
    while(!feof(f)) {
	ini_readline(f,buffer,sizeof(buffer));
	section = ini_buffer_to_section(buffer);
	if(section) {
	    memset(current_section,0,sizeof(current_section));
	    strncpy(current_section,section,sizeof(current_section)-1);
	} else {
	    if(current_section[0]) {
		ini_buffer_to_variable(buffer,&var,&val);
		if( var && val ) {
		    ini = ini_set_variable(ini,current_section,var,val);
		}
	    }
	}
    }
    fclose(f);
    return ini;
}

List *ini_section_node_with_variable_node(List *ini, List *node)
{
    IniSection *section_data;
    List *section;
    
    for(section = ini; section; section = section->next) {
	section_data = section->data;
	if( list_has_node(section_data->variables,node) ) return section;
    }
    return NULL;
}

void ini_delete_variable_ini_variable(IniVariable *var)
{
    if(var->name) free(var->name);
    if(var->value) free(var->value);
    free(var);    
}

void ini_delete_variable_node(List *ini, List *node)
{
    IniVariable *ini_var;
    List *section_node;
    IniSection *section_data;
    
    section_node = ini_section_node_with_variable_node(ini,node);
    if(!section_node) return;
    section_data = section_node->data;
    
    ini_var = node->data;
    ini_delete_variable_ini_variable(ini_var);
    section_data->variables = list_delete_node(section_data->variables,node);
}

void ini_delete_variable(List *ini, char *section, char *variable)
{
    List *node;
    
    node = ini_get_variable_node(ini,section,variable);
    if(!node) return;
    ini_delete_variable_node(ini,node);
}

List *ini_delete_section(List *ini, char *section)
{
    List *section_node, *var_node;
    IniSection *section_data;
    
    section_node = ini_get_section_node(ini,section);
    if(!section_node) return ini;
    section_data = section_node->data;
    for(var_node = section_data->variables; var_node; var_node=var_node->next) {
	ini_delete_variable_ini_variable(var_node->data);
    }
    list_free(section_data->variables);
    if(section_data->name) free(section_data->name);
    free(section_data);
    ini = list_delete_node(ini,section_node);
    return ini;
}

List *ini_delete_section_node(List *ini, List *section)
{
    IniSection *section_data = section->data;
    return ini_delete_section(ini,section_data->name);
}

void ini_free(List *ini)
{
    while(ini) {
	ini = ini_delete_section_node(ini,ini);
    }
}
