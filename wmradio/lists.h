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

#ifndef _LISTS_H
#define _LISTS_H
#include <stdlib.h>

typedef struct _List List;

struct _List {
    List *prev, *next;
    void *data;
};

typedef int (*list_for_each_function)(List *list,void *data);

List *list_new_item();
List *list_last(List *list);
List *list_add(List *list, List *newitem);
List *list_add_data(List *list, void *data);
List *list_node_with_data(List *list, void *data);
int list_length(List *list);
List *list_nth_node(List *list, int index);
List *list_remove_node(List *list, List *node);
List *list_delete_node(List *list, List *node);
void list_for_each(List *list, list_for_each_function func, void *data);
void list_free(List *list);
int list_has_node(List *list, List *node);

#endif
