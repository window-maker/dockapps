#include <stdlib.h>
#include <string.h>
#include "song_hash.h"

#define HASH_TABLE_SIZE 11

struct hash_elt *Table[HASH_TABLE_SIZE];

int hash_fn(int num);
void free_elt(struct hash_elt *elt);

int hash_fn(int num)
{
	return (num % HASH_TABLE_SIZE);
}

void insert_song(int track_num, char *title, char *filename)
{
	int hash_value;
	struct hash_elt *hash_list;
	struct hash_elt *new_elt;


	new_elt = (struct hash_elt *) malloc(sizeof(struct hash_elt));
	if (new_elt != NULL)
	{
		new_elt->track_num = track_num;
		new_elt->title = (char *)strdup(title);
		new_elt->filename = (char *)strdup(filename);
		new_elt->next = NULL;
	}

	hash_value = hash_fn(track_num);
	hash_list = Table[hash_value];
	if (hash_list != NULL) {
		while (hash_list->next != NULL) {
			hash_list = hash_list->next;
		}
		hash_list->next = new_elt;
	}
	else
	{
		Table[hash_value] = new_elt;
	}
}

struct hash_elt *get_song(int track_num)
{
	int hash_value;
	struct hash_elt *hash_list;

	hash_value = hash_fn(track_num);
	hash_list = Table[hash_value];
	if (hash_list) {
	    while ((hash_list->track_num != track_num) &&
		   (hash_list->next != NULL)) {
		hash_list = hash_list->next;
	    }
	    if (hash_list->track_num == track_num) {
		return hash_list;
	    }
	    else {
		return NULL;
	    }
	}
	else {
	    return NULL;
	}
}

void free_elt(struct hash_elt *elt)
{
	if (elt != NULL)
	{
		if (elt->next != NULL) {
			free_elt(elt->next);
		}
		free(elt);
	}
}

void empty_hash()
{
	int i;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		free_elt(Table[i]);
		Table[i] = NULL;
	}
}

void print_hash()
{
	int i;
	struct hash_elt *hash_list;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		printf("%d:\n", i);
		hash_list = Table[i];
		while (hash_list != NULL) {
			printf("\t%d: %s\n",
			       hash_list->track_num,
			       hash_list->title);
			hash_list = hash_list->next;
		}
	}

}




