/*
 * wmmp3
 * Copyright (c)1999 Patrick Crosby <xb@dotfiles.com>.
 * This software covered by the GPL.  See COPYING file for details.
 *
 * mpg123ctl.c
 *
 * This file contains all the functions for controlling the
 * mpg123 backend processes.
 *
 * Random play functionality courtesy of:
 *       Matthew D. Campbell <matt@campbellhome.dhs.org>
 *
 * $Id: mpg123ctl.c,v 1.15 1999/10/12 04:41:20 pcrosby Exp $
 */

#include "mpg123ctl.h"

#define MAXDIRS 100
#define MAX_TITLE_LEN 9

void set_playlist();
void signal_play();
void play();
void play_next();
void play_prev();
void init_ctl();
void push_dir(char *s);
void push_dirname(char *s);
char *pop_dir();
char *next_mp3dir();
char *prev_mp3dir();
char *current_mp3dir();
void show_directory_name();
void load_next_dir();
void load_prev_dir();
void alarmhandler(int sig);
void scroll_title();
void finish();
void dostuff();

char *dirs[MAXDIRS];
int top = 0;
int max_dirs = 0;
int current_dir = 0;

char *dirnames[MAXDIRS];
int ntop = 0;
int nmax_dirs = 0;

char mpg123_cmd[512];
char mp3ext[12];
char playlistext[12];

int next_song = 0;
int num_songs = 0;
int play_pid = 0;

char title[512];
int scroll_pos = 0;
int do_scroll = 0;
int always_scroll = 0;

int repeat_flag = 0;
int random_flag = 0;
int *rand_song_num = NULL;

int is_playing()
{
    if (play_pid > 0)
	return 1;
    else
	return 0;
}

/*
 * patch received from Steven Jorgensen to fix following function
 */

void set_playlist()
{
    char *directory;
    DIR *dp;
    struct dirent *entry;
    char filename[512];

    int i, tempnum, temppos;

    directory = (char *) current_mp3dir();

    if (directory)
    {
    dp = opendir(directory);
    if (dp == NULL) {
	char *new_directory;
	new_directory = (char *) next_mp3dir();
	dp = opendir(new_directory);
	while (new_directory && (strcmp(new_directory, directory) != 0) &&
	       (dp == NULL)) {
	    strcpy(directory, new_directory);
	    new_directory = (char *) next_mp3dir();
	    dp = opendir(new_directory);
	}
	if (new_directory)
	    strcpy(directory, new_directory);
    }
    if (dp != NULL) {
	show_directory_name();
	num_songs = 0;
	empty_hash();
	entry = readdir(dp);
	while (entry != NULL) {
	    if (strstr(entry->d_name, mp3ext)) {
		sprintf(filename, "%s/%s", directory, entry->d_name);
		insert_song(num_songs, entry->d_name, filename);
		num_songs++;
	    }
	    entry = readdir(dp);
	}
	next_song = 0;

	/* Create Pseudo-random permutation of list */
	srand(time(NULL));
	rand_song_num = (int *)malloc(sizeof(int)*num_songs);
	if (!rand_song_num) {
	    /* This shouldn't happen - the list isn't that big */
	    fprintf(stderr,
		    "Error: cannot allocate randomized list\n");
	    exit(1);
	}
	for (i=0; i<num_songs; i++) rand_song_num[i] = i;
	for (i=0; i<num_songs; i++) {
	    tempnum = rand_song_num[i];
	    temppos = rand()%num_songs;
	    rand_song_num[i] = rand_song_num[temppos];
	    rand_song_num[temppos] = tempnum;
	}
    }
    }
}


/*
 * functions that actually control mpg123
 */

void signal_play()
{
    int status;

    waitpid(play_pid, &status, 0);
    play_pid = 0;
    play_next();
}

void play(char *song_name)
{
    if ((play_pid = fork()) == 0) {
	execl(mpg123_cmd, mpg123_cmd, "-q",
	      song_name, 0);
	_exit(1);
    }
}

void play_next()
{
    struct hash_elt *song;

    if (next_song >= num_songs) {
	if (repeat_flag == 0) {
	    load_next_dir();
	} else {
	    next_song = 0;
	}
    }
    song = get_song(next_song);
    if (random_flag) {
	song = get_song(rand_song_num[next_song]);
    }
    if (song) {
	strcpy(title, song->title);
	strcat(title, " ");
	scroll_pos = 0;
	do_scroll = 1;
	scroll_title();

	play(song->filename);
	next_song++;
	signal(SIGCHLD, signal_play);
	signal(SIGALRM, alarmhandler);
	alarm(1);
    }
}

void play_prev()
{
    struct hash_elt *song;

    if (next_song <= 1) {
	next_song = 0;
    }
    else {
	next_song = next_song - 2;
    }

    play_next();
}

void user_play()
{
    if (play_pid == 0) {
	signal(SIGCHLD, signal_play);
	set_playlist();
	play_next();
    }
}

void stop()
{
    int status;

    if (play_pid > 0) {
	signal(SIGCHLD, SIG_IGN);
	kill(play_pid, SIGINT);
	kill(play_pid, SIGTERM);
	kill(play_pid, SIGKILL);
	waitpid(play_pid, &status, 0);
	play_pid = 0;
    }
}

void next()
{
    stop();
    play_next();
}

void back()
{
    stop();
    play_prev();
}

/*
 * initialization functions
 */

void init_ctl()
{
    signal(SIGINT, finish);
    signal(SIGCHLD, dostuff);
    set_playlist();
}

void set_mpg123(char *s)
{

    strcpy(mpg123_cmd, s);
}

void set_mp3ext(char *s)
{
    strcpy(mp3ext, s);
}

void set_playlistext(char *s)
{
    strcpy(playlistext, s);
}

void set_alwaysscroll(char *s) {
    char *temp = s;
    while (*temp) {
	*temp = tolower(*temp);
	temp++;
    }
    if ( !strcmp(s, "on") || !strcmp(s, "yes") || !strcmp(s, "1") ||
	 !strcmp(s, "true")) {
	always_scroll = 1;
    } else {
	always_scroll = 0;
    }
}


void push_dir(char *s)
{
    dirs[top] = (char *) malloc(strlen(s) + 1);
    strcpy(dirs[top], s);
    top++;
    max_dirs++;
}

void push_dirname(char *s)
{
    /* from Steven Jorgensen */
    if ((strlen(s) + 1) < 10)
	dirnames[ntop] = (char *) malloc(10);
    else
	dirnames[ntop] = (char *) malloc(strlen(s) + 1);
    strcpy(dirnames[ntop], s);
    ntop++;
    nmax_dirs++;
}

char *pop_dir()
{
    max_dirs--;
    return (dirs[top--]);
}

void add_mp3dir(char *s)
{
    push_dir(s);
}

void add_mp3dirname(char *s)
{
    push_dirname(s);
}

/*
 * directory manipulation
 */

char *next_mp3dir()
{
    if (current_dir < (max_dirs - 1)) {
	current_dir++;
    }
    return (dirs[current_dir]);
}

char *prev_mp3dir()
{
    if (current_dir > 0) {
	current_dir--;
    }
    return (dirs[current_dir]);
}

char *current_mp3dir()
{
    return (dirs[current_dir]);
}

void show_directory_name()
{
    if (dirnames[current_dir] != NULL) {
	while (strlen(dirnames[current_dir]) < 9) {
	    strcat(dirnames[current_dir], " ");
	}
	draw_string(dirnames[current_dir], 5, 5);
    } else {
	draw_string("no dirname", 5, 5);
    }
}

void dir_up(int button_num)
{
    if (!is_playing()) {
	button_down(button_num);
	load_prev_dir();
    }
}

void dir_down(int button_num)
{
    if (!is_playing()) {
	button_down(button_num);
	load_next_dir();
    }
}

void load_next_dir()
{
    next_mp3dir();
    set_playlist();
}

void load_prev_dir()
{
    prev_mp3dir();
    set_playlist();
}

/*
 * song title functions
 */

void alarmhandler(int sig)
{
    if ((play_pid > 0) && (do_scroll == 1)) {
	scroll_title();
	signal(SIGALRM, alarmhandler);
	alarm(1);
    }
}

void scroll_title()
{
    char s[MAX_TITLE_LEN + 1];
    int i;
    int title_len;

    title_len = strlen(title);
    if (do_scroll) {
	for (i = 0; i < MAX_TITLE_LEN; i++) {
	    s[i] = title[(i + scroll_pos) % title_len];
	}
	s[i] = '\0';
	draw_string(s, 5, 19);
	scroll_pos++;
	if (scroll_pos > title_len) {
	    scroll_pos = 0;
	    if (!always_scroll) {
		do_scroll = 0;
	    }
	}
    } else {
	draw_string(title, 5, 19);
    }
    RedrawWindow();
}

void turn_on_scroll()
{
    if ((!do_scroll) && (is_playing())) {
	do_scroll = 1;
	scroll_pos = 0;
	signal(SIGALRM, alarmhandler);
	alarm(1);
    }
}

/*
 * toggles
 */

void random_toggle(int button_num)
{
/*	button_gray(button_num); */
    if (random_flag == 0) {
	button_down(button_num);
	random_flag = 1;
    } else {
	button_up(button_num);
	random_flag = 0;
    }
}

void repeat_toggle(int button_num)
{
    if (repeat_flag == 0) {
	button_down(button_num);
	repeat_flag = 1;
    } else {
	button_up(button_num);
	repeat_flag = 0;
    }
}

/*
 * cleanup
 */

void finish()
{
    stop();
}

/*
 * misc
 */
void dostuff()
{
				/* empty */
}

