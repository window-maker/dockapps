/*
 * wmmp3
 * Copyright (c)1999 Patrick Crosby <xb@dotfiles.com>.
 * This software covered by the GPL.  See COPYING file for details.
 *
 * main.c
 *                                                                
 * This is the main body of the application.  Handles all initialization
 * plus the X event loop.
 *                                                                
 * $Id: main.c,v 1.11 1999/10/08 22:21:32 pcrosby Exp $
 */

#include "main.h"

#define B_STOP 0
#define B_PLAY 1
#define B_BACK 2
#define B_NEXT 3
#define B_PREV_DIR 4
#define B_RAND 5
#define B_NEXT_DIR 6
#define B_REPEAT 7
#define B_TITLE 8

void loadconfig();
void show_help();
void show_version();
int check_options(int argc, char *argv[]);
int handle_button_press(int x, int y);
void handle_button_release(int i);

struct coord {
    int x;
    int y;
    int w;
    int h;
};

/* from Steven Jorgensen */
void stripspace(char *s) {
    char *t;
    t = s + strlen(s) - 1;
    while (t > s && isspace(*t)) {
	t--;
    }
    t++;
    *t = '\0';
}
 
int handle_button_press(int x, int y) {
    int i;
	
    i = CheckMouseRegion(x, y);
    switch (i) {
    case B_STOP:
	button_down(i);
	button_up(B_PLAY);	/* raise play */
	button_up(B_PREV_DIR);	/* reactivate directory up/down */
	button_up(B_NEXT_DIR);
	stop();
	break;
    case B_PLAY:
	button_down(i);
	button_gray(B_PREV_DIR);	/* gray out directory up/down */
	button_gray(B_NEXT_DIR);
	user_play();
	break;
    case B_BACK:
	button_down(i);
	back();
	break;
    case B_NEXT:
	button_down(i);
	next();
	break;
    case B_PREV_DIR:
	dir_up(i);
	break;
    case B_RAND:
	random_toggle(i);
	break;
    case B_NEXT_DIR:
	dir_down(i);
	break;
    case B_REPEAT:
	repeat_toggle(i);
	break;
    case B_TITLE:
	turn_on_scroll();
	break;
    default:
	fprintf(stderr, "unknown button pressed\n");
    }
    RedrawWindow();	

    return(i);
}

void handle_button_release(int i)
{
    /* play stays down, don't mess with toggles */
    /* ignore song title press too */
    if (((i != B_PLAY) && (i != B_RAND)) && 
	((i != B_REPEAT) && (i != B_TITLE))) {
	if (!is_playing()) {
	    button_up(i);
	    RedrawWindow();
	} else {
	    /* don't undo gray of dir up/down */
	    if (((i == B_STOP) || (i == B_BACK)) || (i == B_NEXT)) {
		button_up(i);
		RedrawWindow();
	    }
	}
    }
}

void loadconfig()
{
    struct passwd *pw;
    char *config_filename;
    FILE *fp;

    errno = 0;

    /* set defualts in case anything fails */
    set_mpg123("/usr/local/bin/mpg123");
    set_mp3ext(".mp3");
    set_playlistext(".m3u");

    pw = getpwuid(getuid());
    /* don't forget about the string terminator... */
    config_filename = (char *) malloc(sizeof(char) *
				      (strlen(pw->pw_dir) + 8));
    sprintf(config_filename, "%s/.wmmp3", pw->pw_dir);


    fp = fopen(config_filename, "r");
    if (fp != NULL) {
	char line[256];
	char variable[256];
	char value[256];

	fgets(line, 256, fp);
	while (!feof(fp)) {
	    if ((line[0] != '#') && (line[0] != '\n')) {
		if (sscanf(line, " %s = %[^\n]", variable, value) < 2) {
		    fprintf(stderr, "Malformed line in config file: %s\n", line);
		} else {
		    if (strcmp(variable, "mpg123") == 0) {
			stripspace(value);
			set_mpg123(value);
		    } else if (strcmp(variable, "mp3dir") == 0) {
			stripspace(value);
			add_mp3dir(value);
		    } else if (strcmp(variable, "mp3dirname") == 0) {
			stripspace(value);
			add_mp3dirname(value);
		    } else if (strcmp(variable, "mp3ext") == 0) {
			stripspace(value);
			set_mp3ext(value);
		    } else if (strcmp(variable, "playlistext") == 0) {
			stripspace(value);
			set_playlistext(value);
		    } else if (strcmp(variable, "alwaysscroll") == 0) {
			stripspace(value);
			set_alwaysscroll(value);
		    } else {
			fprintf(stderr, "Unrecognized variable in config file: %s\n",
				variable);
		    }
		}
	    }
	    fgets(line, 256, fp);
	}


	fclose(fp);
    } else {
	fprintf(stderr, "open of %s failed: %s\n",
		config_filename,
		strerror(errno));
    }

    free(config_filename);
}

void show_help()
{
    printf("wmmp3 -- by Patrick Crosby -- Version %s\n", VERSION);
    printf("At this moment, there are no command line options that do\n");
    printf("anything special:\n");
    printf("\t-h,--help:  print this message\n");
    printf("\t-v,--version:  print version info\n");
    printf("All options are set in ~/.wmmp3.  See sample.wmmp3 in the\n");
    printf("distribution or http://dotfiles.com/software/ for more info.\n");
}

void show_version()
{
    printf("wmmp3 -- by Patrick Crosby -- Version %s\n", VERSION);
}

int check_options(int argc, char *argv[])
{
    int option_entered = 0;
    int i;

	
    for (i = 1; i < argc; i++) {
	if (streq(argv[i], "-h")) {
	    option_entered = 1;
	    show_help();
	}
	else if (streq(argv[i], "--help")) {
	    option_entered = 1;
	    show_help();
	}
	else if (streq(argv[i], "-v")) {
	    option_entered = 1;
	    show_version();
	}
	else if (streq(argv[i], "--version")) {
	    option_entered = 1;
	    show_version();
	}
    }

    return option_entered;
}

void main(int argc, char *argv[])
{
    struct coord pos[] = {
	{35, 34, 12, 11},	/* stop */
	{46, 34, 12, 11},	/* play */
	{35, 45, 12, 11},	/* back */
	{46, 45, 12, 11},	/* next */
	{6, 34, 12, 11},	/* prev_dir */
	{17, 34, 12, 11},	/* random */
	{6, 45, 12, 11},	/* next_dir */
	{17, 45, 12, 11},	/* repeat */
	{5, 18, 54, 12}		/* song title */
    };

    char mask_bits[64 * 64];
    int mask_width = 64;
    int mask_height = 64;
    XEvent Event;
    int i;
    int btn_num;

    if (check_options(argc, argv)) {
	exit(1);
    }

    loadconfig();

    /* set up the display area for wmmp3 */
    createXBMfromXPM(mask_bits, wmmp3_xpm, mask_width, mask_height);
    openXwindow(argc, argv, wmmp3_xpm, mask_bits, mask_width, mask_height);

    font_init();
    draw_string("wmmp3", 5, 5);


    i = 0;
    for (i = 0; i < 9; i++)
	AddMouseRegion(i, pos[i].x, pos[i].y,
		       pos[i].x + pos[i].w,
		       pos[i].y + pos[i].h);

    /* event loop */
    while (1) {
	while (XPending(display)) {
	    XNextEvent(display, &Event);
	    switch (Event.type) {
	    case Expose:
		RedrawWindow();
		break;
	    case ClientMessage:
		break;
	    case DestroyNotify:
		XCloseDisplay(display);
		exit(0);
	    case ButtonPress:
		btn_num = handle_button_press(Event.xbutton.x, 
					      Event.xbutton.y);
		break;
	    case ButtonRelease:
		handle_button_release(btn_num);
		break;
	    }

	}
	usleep(50000);
    }
    exit(0);
}

