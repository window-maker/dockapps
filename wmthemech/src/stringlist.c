#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h> /* memcpy */

#define BASE_SIZE 1024

struct string_list_t {
	unsigned int size;
	unsigned int used;
	int * item;
};


struct string_list_t *
create_list ()
{
	struct string_list_t * list = (struct string_list_t *)
		malloc (sizeof (struct string_list_t) );
	list->item = malloc (BASE_SIZE * sizeof (int) );
	list->size = BASE_SIZE;
	list->used = 0;
	
	return list;
}


void
delete_list (struct string_list_t * list)
{
	free (list->item);
}


int
get_item (struct string_list_t * list, unsigned int item)
{
	return ( item >= list->used) ? -1 : list->item[item];
}


int
add_item (struct string_list_t * list, unsigned int item)
{
	if (list->used == list->size) {
		list->size += BASE_SIZE;
		list->item = realloc (list->item, list->size * sizeof (int) );
	}

	list->item[list->used++] = item;

	return 0;
}


int
add_list (struct string_list_t * dst, struct string_list_t * src)
{
	if (dst->used + src->used > dst->size) {
		dst->size = dst->used + src->used;
		dst->item = realloc (dst->item, dst->size * sizeof (int) );
	}

	memcpy (&dst->item[dst->used], src->item, src->used * sizeof (int) );
	dst->used += src->used;

	return 0;
}
