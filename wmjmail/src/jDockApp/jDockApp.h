#ifndef JetDockApp_H
#define JetDockApp_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "../xutils/xutils.h"

#include "../xutils/xutils.h"

#include "jDockApp_main.xpm"
#include "jDockApp_mask.xbm"

/* These are defined in your application */
void setup();
void do_update();
void do_expose();
void do_button_release();

/* Thse are in jDockApp */
void set_update_delay(int);
void set_loop_delay(int);

/* These two are in jprintf.c */
void jpprintf(int x, int y, int color, const char *, ...);
void  jprintf(              int color, const char *, ...);

#define clear_window() copyXPMArea(587, 0, 64, 64, 0, 0)
#define true  1
#define false 0

#endif
