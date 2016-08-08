#ifndef __SONG_HASH__
#define __SONG_HASH__

#include <stdio.h>

struct hash_elt {
	struct hash_elt *next;
	char *title;
	char *filename;
	int track_num;
};

void insert_song(int track_num, char *title, char *filename);
struct hash_elt *get_song(int track_num);
void empty_hash();
void print_hash();

#endif
