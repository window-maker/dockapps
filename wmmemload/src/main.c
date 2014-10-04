/*
 *    WMMemLoad - A dockapp to monitor memory usage
 *    Copyright (C) 2002  Mark Staggs <me@markstaggs.net>
 *
 *    Based on work by Seiichi SATO <ssato@sh.rim.or.jp>
 *    Copyright (C) 2001,2002  Seiichi SATO <ssato@sh.rim.or.jp>

 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.

 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.

 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "dockapp.h"
#include "mem.h"
#include "backlight_on.xpm"
#include "backlight_off.xpm"
#include "parts.xpm"

#define SIZE	    58
#define WINDOWED_BG "  \tc #AEAAAE"
#define MAX_HISTORY 16
#define CPUNUM_NONE -1

typedef enum { LIGHTON, LIGHTOFF } light;

Pixmap pixmap;
Pixmap backdrop_on;
Pixmap backdrop_off;
Pixmap parts;
Pixmap mask;
static char	*display_name = "";
static char	*light_color = NULL;	/* back-light color */
static unsigned update_interval = 1;
static light    backlight = LIGHTOFF;

static struct	mem_options mem_opts;
static unsigned mem_usage = 0;
static unsigned swap_usage = 0;
static unsigned alarm_mem = 101;	
static unsigned alarm_swap = 101;

/* prototypes */
static void update(void);
static void switch_light(void);
static void draw_memdigit(int per);
static void draw_swapdigit(int per);
static void parse_arguments(int argc, char **argv);
static void print_help(char *prog);

int main(int argc, char **argv)
{
   XEvent event;
   XpmColorSymbol colors[2] = { {"Back0", NULL, 0}, {"Back1", NULL, 0} };
   int ncolor = 0;

   /* Parse CommandLine */
   mem_opts.ignore_buffers = mem_opts.ignore_cached
		                     = mem_opts.ignore_wired = False;
   parse_arguments(argc, argv);

   /* Initialize Application */
   mem_init();
   dockapp_open_window(display_name, PACKAGE, SIZE, SIZE, argc, argv);
   dockapp_set_eventmask(ButtonPressMask);

   if(light_color)
   {
      colors[0].pixel = dockapp_getcolor(light_color);
      colors[1].pixel = dockapp_blendedcolor(light_color, -24, -24, -24, 1.0);
      ncolor = 2;
   }

   /* change raw xpm data to pixmap */
   if(dockapp_iswindowed)
      backlight_on_xpm[1] = backlight_off_xpm[1] = WINDOWED_BG;

   if(!dockapp_xpm2pixmap(backlight_on_xpm, &backdrop_on, &mask, colors, ncolor))
   {
      fprintf(stderr, "Error initializing backlit background image.\n");
      exit(1);
   }
   if(!dockapp_xpm2pixmap(backlight_off_xpm, &backdrop_off, NULL, NULL, 0))
   {
      fprintf(stderr, "Error initializing background image.\n");
      exit(1);
   }
   if(!dockapp_xpm2pixmap(parts_xpm, &parts, NULL, colors, ncolor))
   {
      fprintf(stderr, "Error initializing parts image.\n");
      exit(1);
   }

   /* shape window */
   if(!dockapp_iswindowed)
      dockapp_setshape(mask, 0, 0);
   if(mask) XFreePixmap(display, mask);

   /* pixmap : draw area */
   pixmap = dockapp_XCreatePixmap(SIZE, SIZE);

   /* Initialize pixmap */
   if(backlight == LIGHTON) 
      dockapp_copyarea(backdrop_on, pixmap, 0, 0, SIZE, SIZE, 0, 0);
   else
      dockapp_copyarea(backdrop_off, pixmap, 0, 0, SIZE, SIZE, 0, 0);

   dockapp_set_background(pixmap);
   dockapp_show();

   /* Main loop */
   while(1)
   {
      if (dockapp_nextevent_or_timeout(&event, update_interval * 1000))
      {
         /* Next Event */
         switch(event.type)
         {
            case ButtonPress:
               switch_light();
               break;
            default: /* make gcc happy */
               break;
         }
      }
      else
      {
         /* Time Out */
         update();
      }
   }

   return 0;
}

/* called by timer */
static void update(void)
{
   static light pre_backlight;
   static Bool in_alarm_mode = False;

   /* get current cpu usage in percent */
   mem_getusage(&mem_usage, &swap_usage, &mem_opts);

   /* alarm mode */
   if(mem_usage >= alarm_mem || swap_usage >= alarm_swap) 
   {
      if(!in_alarm_mode)
      {
         in_alarm_mode = True;
         pre_backlight = backlight;
      }
      if(backlight == LIGHTOFF)
      {
         switch_light();
         return;
      }
   } 
   else
   {
      if(in_alarm_mode)
      {
         in_alarm_mode = False;
         if (backlight != pre_backlight) 
         {
            switch_light();
            return;
         }
      }
   }

   /* all clear */
   if (backlight == LIGHTON) 
      dockapp_copyarea(backdrop_on, pixmap, 0, 0, 58, 58, 0, 0);
   else 
      dockapp_copyarea(backdrop_off, pixmap, 0, 0, 58, 58, 0, 0);

   /* draw digit */
   draw_memdigit(mem_usage);
   draw_swapdigit(swap_usage);

   /* show */
   dockapp_copy2window(pixmap);
}

/* called when mouse button pressed */
static void switch_light(void)
{
   switch (backlight)
   {
      case LIGHTOFF:
         backlight = LIGHTON;
         dockapp_copyarea(backdrop_on, pixmap, 0, 0, 58, 58, 0, 0);
         break;
      case LIGHTON:
         backlight = LIGHTOFF;
         dockapp_copyarea(backdrop_off, pixmap, 0, 0, 58, 58, 0, 0);
         break;
   }

   /* redraw digit */
   mem_getusage(&mem_usage, &swap_usage, &mem_opts);
   draw_memdigit(mem_usage);
   draw_swapdigit(swap_usage);

   /* show */
   dockapp_copy2window(pixmap);
}

static void draw_memdigit(int per)
{
   int v100, v10, v1;
   int y = 0;

   if (per < 0) per = 0;
   if (per > 100) per = 100;

   v100 = per / 100;
   v10  = (per - v100 * 100) / 10;
   v1   = (per - v100 * 100 - v10 * 10);

   if (backlight == LIGHTON) y = 20;

   /* draw digit */
   dockapp_copyarea(parts, pixmap, v1 * 10, y, 10, 20, 29, 7);
   if (v10 != 0)
      dockapp_copyarea(parts, pixmap, v10 * 10, y, 10, 20, 17, 7);
   if (v100 == 1)
   {
      dockapp_copyarea(parts, pixmap, 10, y, 10, 20,  5, 7);
      dockapp_copyarea(parts, pixmap, 0, y, 10, 20, 17, 7);
   }
}


static void draw_swapdigit(int per)
{
   int v100, v10, v1;
   int y = 0;

   if (per < 0) per = 0;
   if (per > 100) per = 100;

   v100 = per / 100;
   v10  = (per - v100 * 100) / 10;
   v1   = (per - v100 * 100 - v10 * 10);

   if (backlight == LIGHTON) y = 20;

   /* draw digit */
   dockapp_copyarea(parts, pixmap, v1 * 10, y, 10, 20, 29, 34);
   if (v10 != 0)
      dockapp_copyarea(parts, pixmap, v10 * 10, y, 10, 20, 17, 34);
   if (v100 == 1)
   {
      dockapp_copyarea(parts, pixmap, 10, y, 10, 20, 5, 34);
      dockapp_copyarea(parts, pixmap, 0, y, 10, 20, 17, 34);
   }
}

static void parse_arguments(int argc, char **argv)
{
   int i;
   int integer;
   for (i = 1; i < argc; i++) 
   {
      if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
         print_help(argv[0]), exit(0);
      else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v"))
         printf("%s version %s\n", PACKAGE, VERSION), exit(0);
      else if (!strcmp(argv[i], "--display") || !strcmp(argv[i], "-d")) 
      {
         display_name = argv[i + 1];
         i++;
      }
      else if (!strcmp(argv[i], "--alarm-mem") || !strcmp(argv[i], "-am")) 
      {
         if (argc == i + 1)
            alarm_mem = 90;
         else if (sscanf(argv[i + 1], "%i", &integer) != 1)
            alarm_mem = 90;
         else if (integer < 0 || integer > 100)
            fprintf(stderr, "%s: argument %s must be from 0 to 100\n",
                    argv[0], argv[i]), exit(1);
         else
            alarm_mem = integer, i++;
      }
      else if (!strcmp(argv[i], "--alarm-swap") || !strcmp(argv[i], "-as")) 
      {
         if (argc == i + 1)
            alarm_swap = 50;
         else if (sscanf(argv[i + 1], "%i", &integer) != 1)
            alarm_swap = 50;
         else if (integer < 0 || integer > 100)
            fprintf(stderr, "%s: argument %s must be from 0 to 100\n",
                    argv[0], argv[i]), exit(1);
         else
            alarm_swap = integer, i++;
      }
      else if (!strcmp(argv[i], "--backlight") || !strcmp(argv[i], "-bl"))
         backlight = LIGHTON;
      else if (!strcmp(argv[i], "--light-color") || !strcmp(argv[i], "-lc")) 
      {
         light_color = argv[i + 1];
         i++;
      }
      else if (!strcmp(argv[i], "--interval") || !strcmp(argv[i], "-i")) 
      {
         if (argc == i + 1)
            fprintf(stderr, "%s: error parsing argument for option %s\n",
                    argv[0], argv[i]), exit(1);
         if (sscanf(argv[i + 1], "%i", &integer) != 1)
             fprintf(stderr, "%s: error parsing argument for option %s\n",
                     argv[0], argv[i]), exit(1);
         if (integer < 1)
             fprintf(stderr, "%s: argument %s must be >=1\n",
                     argv[0], argv[i]), exit(1);
         update_interval = integer;
         i++;
      }
      else if (!strcmp(argv[i], "--windowed") || !strcmp(argv[i], "-w"))
         dockapp_iswindowed = True;
      else if (!strcmp(argv[i], "--broken-wm") || !strcmp(argv[i], "-bw"))
         dockapp_isbrokenwm = True;
#ifdef IGNORE_BUFFERS
      else if (!strcmp(argv[i], "--ignore-buffers") || !strcmp(argv[i], "-b"))
         mem_opts.ignore_buffers = True;
#endif
#ifdef IGNORE_CACHED
      else if (!strcmp(argv[i], "--ignore-cached") || !strcmp(argv[i], "-c"))
         mem_opts.ignore_cached = True;
#endif
#ifdef IGNORE_WIRED
      else if (!strcmp(argv[i], "--ignore-wired") || !strcmp(argv[i], "-wr"))
         mem_opts.ignore_wired = True;
#endif
      else
      {
         fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[i]);
         print_help(argv[0]), exit(1);
      }
   }
   if (alarm_mem != 101 && alarm_swap != 101)
   { 
      fprintf(stderr,
              "%s: select either '-am, --alarm-mem' or '-as, --alarm-swap'\n",
              argv[0]);
      exit(1);
   }
}

static void print_help(char *prog)
{
   printf("Usage : %s [OPTIONS]\n", prog);
   printf("WMMemMon - Window Maker memory/swap monitor dockapp\n");
   printf("  -d,  --display <string>        display to use\n");
   printf("  -bl, --backlight               turn on back-light\n");
   printf("  -lc, --light-color <string>    back-light color(rgb:6E/C6/3B is default)\n");
   printf("  -i,  --interval <number>       number of secs between updates (1 is default)\n");
#ifdef IGNORE_BUFFERS
   printf("  -b,  --ignore-buffers          ignore buffers\n");
#endif
#ifdef IGNORE_CACHED
   printf("  -c,  --ignore-cached           ignore cached pages\n");
#endif
#ifdef IGNORE_WIRED
   printf("  -wr, --ignore-wired            ignore wired pages\n");
#endif
   printf("  -h,  --help                    show this help text and exit\n");
   printf("  -v,  --version                 show program version and exit\n");
   printf("  -w,  --windowed                run the application in windowed mode\n");
   printf("  -bw, --broken-wm               activate broken window manager fix\n");
   printf("  -am, --alarm-mem <percentage>  activate alarm mode of memory. <percentage>\n");
   printf("                                 is threshold of percentage from 0 to 100.\n");
   printf("                                 (90 is default)\n");
   printf("  -as, --alarm-swap <percentage> activate alarm mode of swap. <percentage> is\n");
   printf("                                 threshold of percentage from 0 to 100.\n");
   printf("                                 (50 is default)\n");
}
