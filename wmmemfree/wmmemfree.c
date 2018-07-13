/*
 *  wmmemfree - dockapp which displays memory usage
 *
 *  Copyright (C) 2003 Draghicioiu Mihai <misuceldestept@go.ro>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

#include "wmmemfree.h"
#include "dockapp.h"
#include "draw.h"
#include "options.h"

int    argc;
char **argv;
int    exitloop = 0;

void handle_timer(int sig)
{
 if(sig == SIGALRM)
 {
  draw_window();
 }
}

void start_timer()
{
 struct itimerval tv;

 tv.it_value.tv_sec = 2; /* give 2 seconds for safety */
 tv.it_value.tv_usec = 0;
 tv.it_interval.tv_sec = 0;
 tv.it_interval.tv_usec = opt_milisecs * 1000;
 signal(SIGALRM, handle_timer);
 setitimer(ITIMER_REAL, &tv, NULL);
}

void stop_timer()
{
 struct itimerval tv;

 signal(SIGALRM, SIG_IGN);
 tv.it_value.tv_sec = 0;
 tv.it_value.tv_usec = 0;
 tv.it_interval.tv_sec = 0;
 tv.it_interval.tv_usec = 0;
 setitimer(ITIMER_REAL, &tv, NULL);
}

int main(int carg, char **varg)
{
 argc = carg;
 argv = varg;
 parse_args();
 make_window();
 start_timer();
 while(!exitloop)
 {
  process_events();
 }
 stop_timer();
 free_stuff();
 return 0;
}
