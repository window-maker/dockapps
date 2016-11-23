#include "lists.h"
#include <string.h>

List *list_new_item()
{
    List *item;
    item = (List *)malloc(sizeof(List));
    if(item) memset(item, 0, sizeof(List));
    return item;
}

List *list_last(List *list)
{
    List *item;
    
    item = list;
    if(!item) return NULL;
    while(item->next) item = item->next;
    return item;
}

List *list_add(List *list, List *newitem)
{
    List *lastitem;
    
    if(!newitem) return list;
    if(!list) return newitem;
    
    lastitem = list_last(list);
    lastitem -> next = newitem;
    newitem  -> prev = lastitem;
    
    return list;
}

List *list_add_data(List *list, void *data)
{
    List *newitem;
    
    newitem = list_new_item();
    if(!newitem) return list;
    list = list_add(list,newitem);
    newitem  -> data = data;
    return list;
}

List *list_node_with_data(List *list, void *data)
{
    List *item;
    
    item = list;
    while(item) {
	if(item->data == data) return item;
	item = item->next;
    }
    return NULL;
}

int list_length(List *list)
{
    int l = 0;
    List *item;
    
    if(!list) return 0;
    item = list;
    while(item) {
	l++;
	item = item->next;
    }
    return l;
}

List *list_nth_node(List *list, int index)
{
    List *item;
    int i;
    
    item = list;
    i = 0;
    while(item) {
	if( i == index ) return item;
	i++;
	item = item->next;
    }
    return NULL;
}

List *list_remove_node(List *list, List *node)
{
    List *prev,*next;
    
    if(!list) return list;
    if(!node) return list;
    
    prev = node ->prev;
    next = node ->next;
    node -> next = NULL;
    node -> prev = NULL;
    if(prev) prev->next = next;
    if(next) next->prev = prev;
    if(prev) return list;
    return prev;
}

List *list_delete_node(List *list, List *node)
{
    list = list_remove_node(list,node);
    free(node);
    return list;
}

void list_for_each(List *list, list_for_each_function func, void *data)
{
    List *item = list;
    
    if(!list) return;
    if(!func) return;
    
    while(item) {
	if( func(item, data) ) return;
	item = item -> next;
    }
}

void list_free(List *list)
{
    List *item, *prev;
    
    item = list_last(list);
    
    while(item) {
	prev = item->prev;
	free(item);
	item = prev;
    }
}

int list_has_node(List *list, List *node)
{
    List *i;
    
    for(i=list; i ; i = i->next) {
	if( i == node ) return 1;
    }
    return 0;
}

