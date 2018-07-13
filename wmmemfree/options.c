/*
 *  options.c - option parser
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wmmemfree.h"
#include "options.h"

char *opt_display        = OPT_DISPLAY;
int   opt_buffers        = OPT_BUFFERS;
int   opt_cache          = OPT_CACHE;
int   opt_window         = OPT_WINDOW;
int   opt_shape          = OPT_SHAPE;
int   opt_milisecs       = OPT_MILISECS;

void print_usage()
{
 printf("Usage: %s [OPTIONS]\n"
	"-h,  --help                print this help and exit\n"
	"-v,  --version             print version number and exit\n"
	"-display <DISPLAY>         specify the X11 display to connect to\n"
	"-m,  --milisecs <MILISECS> number of miliseconds between updates\n"
        "-b,  --buffers             consider buffer memory\n"
	"-nb, --no-buffers          ignore buffer memory\n"
	"-c,  --cache               consider cache memory\n"
	"-nc, --no-cache            ignore cache memory\n"
	"-w,  --window              run in a window\n"
	"-nw, --no-window           don't run in a window\n"
	"-s,  --shape               use the XShape extension\n"
	"-ns, --no-shape            don't use XShape extension\n",
	argv[0]);
}

void print_version()
{
 printf("WMMemFree version " OPT_VERSION "\n");
}

void parse_args()
{
 int n;

 for(n=1;n<argc;n++)
 {
  if(!strcmp(argv[n], "-h") ||
     !strcmp(argv[n], "--help"))
  {
   print_usage();
   exit(0);
  }
  else if(!strcmp(argv[n], "-v") ||
          !strcmp(argv[n], "--version"))
  {
   print_version();
   exit(0);
  }
  else if(!strcmp(argv[n], "-display"))
  {
   if(argc <= (++n))
   {
    print_usage();
    exit(1);
   }
   else opt_display = argv[n];
  }
  else if(!strcmp(argv[n], "-m") ||
          !strcmp(argv[n], "--milisecs"))
  {
   if(argc <= (++n))
   {
    print_usage();
    exit(1);
   }
   else
    opt_milisecs = atoi(argv[n]);
  }
  else if(!strcmp(argv[n], "-b") ||
          !strcmp(argv[n], "--buffers"))
   opt_buffers = 1;
  else if(!strcmp(argv[n], "-nb") ||
          !strcmp(argv[n], "--no-buffers"))
   opt_buffers = 0;
  else if(!strcmp(argv[n], "-c") ||
          !strcmp(argv[n], "--cache"))
   opt_cache = 1;
  else if(!strcmp(argv[n], "-nc") ||
          !strcmp(argv[n], "--no-cache"))
   opt_cache = 0;
  else if(!strcmp(argv[n], "-w") ||
          !strcmp(argv[n], "--window"))
   opt_window = 1;
  else if(!strcmp(argv[n], "-nw") ||
          !strcmp(argv[n], "--no-window"))
   opt_window = 0;
  else if(!strcmp(argv[n], "-s") ||
          !strcmp(argv[n], "--shape"))
   opt_shape = 1;
  else if(!strcmp(argv[n], "-ns") ||
          !strcmp(argv[n], "--no-shape"))
   opt_shape = 0;
  else
  {
   print_usage();
   exit(1);
  }
 }
}
