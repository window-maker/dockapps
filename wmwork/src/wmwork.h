/* $Id: wmwork.h,v 1.15 2005/02/14 19:14:58 godisch Exp $
 * vim: set noet ts=4:
 *
 * Copyright (c) 2002-2005 Martin A. Godisch <martin@godisch.de>
 */

#ifndef _WMWORK_H
#define _WMWORK_H

#include <config.h>

struct Project {
	char name[4];
	time_t time;
	char *comment;
	struct Project *prev, *next;
};

void ButtonDown(int);
void ButtonUp(int);
void ButtonEnable(int);
void ButtonDisable(int);

void do_opts(int, char**, int*, char**, char**, int*);
int  compat(void);

void drawTime(time_t, time_t, int, int, int);
void drawProject(const char*);

int read_log(void);
int write_log(void);
int write_record(void);

int make_lock(int);

#endif
