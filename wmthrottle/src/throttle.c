/*
 *    wmthrottle- A dockapp to throttle CPU via ACPI status
 *    Copyright (C) 2002  Thomas Nemeth <tnemeth@free.fr>
 *
 *    Based on work by Seiichi SATO <ssato@sh.rim.or.jp>
 *    Copyright (C) 2001,2002  Seiichi SATO <ssato@sh.rim.or.jp>
 *    and on work by Mark Staggs <me@markstaggs.net>
 *    Copyright (C) 2002  Mark Staggs <me@markstaggs.net>

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

#include <stdbool.h>

#include <signal.h>
#include "dockapp.h"
#include "backlight_on.xpm"
#include "backlight_off.xpm"
#include "parts.xpm"

#ifdef linux
#include <sys/stat.h>
#endif

#define FREE(data) {if (data) free (data); data = NULL;}

#define SIZE	    58
#define MAXSTRLEN   512
#define WINDOWED_BG ". c #AEAAAE"
#define MAX_HISTORY 16
#define CPUNUM_NONE -1

# ifdef linux
#  define ACPIDEV "/proc/acpi/info"
# endif

typedef struct AcpiInfos {
    int throttle_state;
    int throttle_perc;
} AcpiInfos;

typedef enum { LIGHTOFF, LIGHTON } light;


Pixmap pixmap;
Pixmap backdrop_on;
Pixmap backdrop_off;
Pixmap parts;
Pixmap mask;
int but_stat = -1;
char *tpercent[]={"100","88","75","63","50","38","25","13"};
static char	*display_name     = "";
static char	*light_color      = NULL;	/* back-light color */
static unsigned update_interval   = 1;
// temperature threshold to slow down cpu
static int temperature_slowdown = 80;
// current temperature
static int temp = 0;
static light    backlight         = LIGHTOFF;
static char *notif_cmd = NULL;
static unsigned alarm_level;
static char* suspend_cmd = NULL;
static char* standby_cmd = NULL;
static unsigned switch_authorized = True;
static unsigned performance_flag = False;
unsigned temperature_flag = False;
unsigned temperature_delta = 1;
static int statecount = 0;
static int currstate = 0;
static AcpiInfos cur_acpi_infos;

#ifdef linux
# ifndef ACPI_32_BIT_SUPPORT
#  define ACPI_32_BIT_SUPPORT      0x0002
# endif
#endif


/* prototypes */
static void update();
static void switch_light();
static void draw_pcdigit(AcpiInfos infos);
static void draw_statusdigit(AcpiInfos infos);
static void draw_pcgraph(AcpiInfos infos);
static void throttle_speed(int x);
static void highlight_but(AcpiInfos infos);
static void parse_arguments(int argc, char **argv);
static void print_help(char *prog);
static void acpi_getinfos(AcpiInfos *infos);
static int  acpi_exists();
static int  my_system (char *cmd);
#ifdef linux
int acpi_read(AcpiInfos *i);
int acpi_get_statecount();
int acpi_read_temp(int *temp);
void throttle_by_temp(int threshold); 
#endif
int count;


int main(int argc, char **argv) {

	
    XEvent   event;
    XpmColorSymbol colors[2] = { {"Back0", NULL, 0}, {"Back1", NULL, 0} };
    int      ncolor = 0;
    struct   sigaction sa;
    FILE *fp;

    sa.sa_handler = SIG_IGN;
#ifdef SA_NOCLDWAIT
    sa.sa_flags = SA_NOCLDWAIT;
#else
    sa.sa_flags = 0;
#endif
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    /* Parse CommandLine */
    parse_arguments(argc, argv);

    /* Check for ACPI support */
    if (!acpi_exists()) {
#ifdef linux
        fprintf(stderr, "No ACPI support in kernel\n");
#else
        fprintf(stderr, "Unable to access ACPI info\n");
#endif
        exit(1);
    }

    /* Initialize Application */
    acpi_getinfos(&cur_acpi_infos);
	 statecount = acpi_get_statecount();
    dockapp_open_window(display_name, PACKAGE, SIZE, SIZE, argc, argv);
    dockapp_set_eventmask(ButtonPressMask);
    init_dock();

    if (light_color) {
        colors[0].pixel = dockapp_getcolor(light_color);
        colors[1].pixel = dockapp_blendedcolor(light_color, -24, -24, -24, 1.0);
        ncolor = 2;
    }

    /* change raw xpm data to pixmap */
    if (dockapp_iswindowed)
        backlight_on_xpm[1] = backlight_off_xpm[1] = WINDOWED_BG;

    if (!dockapp_xpm2pixmap(backlight_on_xpm, &backdrop_on, &mask, colors, ncolor)) {
        fprintf(stderr, "Error initializing backlit background image.\n");
        exit(1);
    }
    if (!dockapp_xpm2pixmap(backlight_off_xpm, &backdrop_off, NULL, NULL, 0)) {
        fprintf(stderr, "Error initializing background image.\n");
        exit(1);
    }
    if (!dockapp_xpm2pixmap(parts_xpm, &parts, NULL, colors, ncolor)) {
        fprintf(stderr, "Error initializing parts image.\n");
        exit(1);
    }

    /* shape window */
    if (!dockapp_iswindowed) dockapp_setshape(mask, 0, 0);
    if (mask) XFreePixmap(display, mask);

    /* pixmap : draw area */
    pixmap = dockapp_XCreatePixmap(SIZE, SIZE);

    /* Initialize pixmap */
    if (backlight == LIGHTON) 
        dockapp_copyarea(backdrop_on, pixmap, 0, 0, SIZE, SIZE, 0, 0);
    else
        dockapp_copyarea(backdrop_off, pixmap, 0, 0, SIZE, SIZE, 0, 0);

    dockapp_set_background(pixmap);
    dockapp_show();

    /* Main loop */
    while (1) {
        if (dockapp_nextevent_or_timeout(&event, update_interval * 1000)) {
            /* Next Event */
            switch (event.type) {
                case ButtonPress:
	           but_stat = CheckMouseRegion(event.xbutton.x, event.xbutton.y);
		    switch(but_stat) {
		      case -1: switch_light(); break;
				case 8: temperature_flag = !temperature_flag; break;
		      default: 
			       throttle_speed(but_stat);
			       break;
                    }
                default: break;
            }
        } else {
            /* Time Out */
	    if(  temperature_flag && temperature_slowdown < 80 )
		    throttle_by_temp( acpi_read_temp( &temp ));
            update();
        }
    }

    return 0;
}

/* called by timer */
static void update() {
    static light pre_backlight;
    static Bool in_alarm_mode = False;

    acpi_getinfos(&cur_acpi_infos);

    /* all clear */
    if (backlight == LIGHTON) 
        dockapp_copyarea(backdrop_on, pixmap, 0, 0, 58, 58, 0, 0);
    else 
        dockapp_copyarea(backdrop_off, pixmap, 0, 0, 58, 58, 0, 0);

    /* draw digit */
    draw_pcdigit(cur_acpi_infos);
    draw_statusdigit(cur_acpi_infos);
    draw_pcgraph(cur_acpi_infos);
    highlight_but(cur_acpi_infos);

    /* show */
    dockapp_copy2window(pixmap);
}


/* called when mouse button pressed */
static void switch_light() {
    switch (backlight) {
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
    acpi_getinfos(&cur_acpi_infos);
    draw_pcdigit(cur_acpi_infos);
    draw_statusdigit(cur_acpi_infos);
    draw_pcgraph(cur_acpi_infos);

    /* show */
    dockapp_copy2window(pixmap);
}

static void draw_pcdigit(AcpiInfos infos) {
    int v100, v10, v1;
    int xd = 0;
    int num = infos.throttle_perc;

    if (num < 0)  num = 0;

    v100 = num / 100;
    v10  = (num - v100 * 100) / 10;
    v1   = (num - v100 * 100 - v10 * 10);

    if (backlight == LIGHTON) xd = 50;

    /* draw digit */
    dockapp_copyarea(parts, pixmap, v1 * 5 + xd, 40, 5, 9, 17, 45);
    if (v10 != 0)
        dockapp_copyarea(parts, pixmap, v10 * 5 + xd, 40, 5, 9, 11, 45);
    if (v100 == 1) {
        dockapp_copyarea(parts, pixmap, 5 + xd, 40, 5, 9, 5, 45);
        dockapp_copyarea(parts, pixmap, 0 + xd, 40, 5, 9, 11, 45);
    }
}

static void highlight_but(AcpiInfos infos) {

	int xd;
	int x = infos.throttle_state;
        

	if (x < 4) {
	        xd = x*12;
	 	dockapp_copyarea(parts, pixmap, xd, 58, 12, 12, 5 + xd, 3);
	}
	else {
	        xd = (x-4)*12;
		dockapp_copyarea(parts, pixmap, xd, 70, 12, 12, 5 + xd, 15);
	}


}

static void draw_statusdigit(AcpiInfos infos) {
    int xd = 0;
    int y = 31;

    if (backlight == LIGHTON) {
        y = 40;
        xd = 50;
    }
    /* draw digit */
    if (infos.throttle_state > 4)
        dockapp_copyarea(parts, pixmap, 1 + xd, 50, 6 ,6 ,34 ,46);
    if (infos.throttle_state <= 4)
        dockapp_copyarea(parts, pixmap, 7 + xd, 50, 6 ,6 ,34 ,46);
    if(infos.throttle_state < 3) 
        dockapp_copyarea(parts, pixmap, 13 + xd, 50, 6 ,6 ,34 ,46);
}


static void draw_pcgraph(AcpiInfos infos) {
    int xd = 100;
    int nb;
    int num = infos.throttle_perc / 6.25 ;

    if (num < 0) num = 0;

    if (backlight == LIGHTON) xd = 102;

    /* draw digit */
    for (nb = 0 ; nb < num ; nb++)
        dockapp_copyarea(parts, pixmap, xd, 0, 2, 9, 6 + nb * 3, 33);
}


static void parse_arguments(int argc, char **argv) {
    int i;
    int integer;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            print_help(argv[0]), exit(0);
        } else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
            printf("%s version %s\n", PACKAGE, VERSION), exit(0);
        // new code for performance
		  } else if (!strcmp(argv[i], "--performance") || !strcmp(argv[i], "-p")) {
            performance_flag = True;
        } else if (!strcmp(argv[i], "--display") || !strcmp(argv[i], "-d")) {
            display_name = argv[i + 1];
            i++;
        } else if (!strcmp(argv[i], "--backlight") || !strcmp(argv[i], "-bl")) {
            backlight = LIGHTON;
        } else if (!strcmp(argv[i], "--light-color") || !strcmp(argv[i], "-lc")) {
            light_color = argv[i + 1];
            i++;
        } else if (!strcmp(argv[i], "--interval") || !strcmp(argv[i], "-i")) {
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
        } else if (!strcmp(argv[i], "--alarm") || !strcmp(argv[i], "-a")) {
            if (argc == i + 1)
                    fprintf(stderr, "%s: error parsing argument for option %s\n",
                            argv[0], argv[i]), exit(1);
            if (sscanf(argv[i + 1], "%i", &integer) != 1)
                fprintf(stderr, "%s: error parsing argument for option %s\n",
                        argv[0], argv[i]), exit(1);
            if ( (integer < 0) || (integer > 100) )
                fprintf(stderr, "%s: argument %s must be >=0 and <=100\n",
                        argv[0], argv[i]), exit(1);
            alarm_level = integer;
            i++;
        } else if (!strcmp(argv[i], "--windowed") || !strcmp(argv[i], "-w")) {
            dockapp_iswindowed = True;
        } else if (!strcmp(argv[i], "--broken-wm") || !strcmp(argv[i], "-bw")) {
            dockapp_isbrokenwm = True;
        } else if (!strcmp(argv[i], "--notify") || !strcmp(argv[i], "-n")) {
            notif_cmd = argv[i + 1];
            i++;
			// temperature threshold for cpu downclocking
        } else if (!strcmp(argv[i], "--temperature") || !strcmp(argv[i], "-t")) {
            if (argc == i + 1)
                fprintf(stderr, "%s: error parsing argument for option %s\n",
                        argv[0], argv[i]), exit(1);
            if (sscanf(argv[i + 1], "%i", &integer) != 1)
                fprintf(stderr, "%s: error parsing argument for option %s\n",
                        argv[0], argv[i]), exit(1);
            if (integer < 1)
                fprintf(stderr, "%s: argument %s must be >=1\n",
                        argv[0], argv[i]), exit(1);
            temperature_slowdown = integer;
				temperature_flag = True;
            i++;
			// temperature "delta" value for state transitions
        } else if ( !strcmp(argv[i], "-e")) {
            if (argc == i + 1)
                fprintf(stderr, "%s: error parsing argument for option %s\n",
                        argv[0], argv[i]), exit(1);
            if (sscanf(argv[i + 1], "%i", &integer) != 1)
                fprintf(stderr, "%s: error parsing argument for option %s\n",
                        argv[0], argv[i]), exit(1);
            if (integer < 1)
                fprintf(stderr, "%s: argument %s must be >=1\n",
                        argv[0], argv[i]), exit(1);
            temperature_delta = integer;
				temperature_flag = True;
            i++;
        } else {
            fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[i]);
            print_help(argv[0]), exit(1);
        }
    }
}


static void print_help(char *prog)
{
    printf("Usage : %s [OPTIONS]\n"
           "%s - Window Maker dockapp for changing acpi throttle/performance states of cpu\n"
           "  -d,  --display <string>        display to use\n"
           "  -bl, --backlight               turn on back-light\n"
           "  -lc, --light-color <string>    back-light color(rgb:6E/C6/3B is default)\n"
           "  -i,  --interval <number>       number of secs between updates (1 is default)\n"
           "  -h,  --help                    show this help text and exit\n"
           "  -v,  --version                 show program version and exit\n"
           "  -w,  --windowed                run the application in windowed mode\n"
           "  -bw, --broken-wm               activate broken window manager fix"
			  " (use this if you have problems running it in your window manager)\n"
           "  -p, --performance              use acpi performance instead of throttling\n"
           "  -t, --temperature              temperature threshold to start downclocking"
			  " (to avoid fan noise)\n"
			  "  -e                             temperature difference that will cause a state transition"
			  " (default: 1)\n",
           prog, prog);
    /* OPTIONS SUPP :
     *  ? -f, --file    : configuration file
     */
}

static void throttle_speed(int x) {

	FILE *fd;

	if( performance_flag ) {
		if(fd = fopen("/proc/acpi/processor/CPU0/performance","w")) {
			 fprintf(fd,"%d",x);
			 fclose(fd);
	      } else
			fprintf(stderr,"Could not change performance state\n");	
	} else {
		if(fd = fopen("/proc/acpi/processor/CPU0/throttling","w")) {
			fprintf(fd,"%d",x);
			fclose(fd);
		}
		else
			fprintf(stderr,"Could not throttle machine\n");
	}
}

static void acpi_getinfos(AcpiInfos *infos) {
    if (
#if defined(linux) || defined(solaris)
 (acpi_read(infos))
#else
# ifdef freebsd
    (acpi_read(&temp_info))
# endif
#endif
     ) {
        fprintf(stderr, "Cannot read ACPI information: %i\n");
        exit(1);
    }
}


int acpi_exists() {
    if (access(ACPIDEV, R_OK))
        return 0;
    else
        return 1;
}


static int my_system (char *cmd) {
    int           pid;
    extern char **environ;

    if (cmd == 0) return 1;
    pid = fork ();
    if (pid == -1) return -1;
    if (pid == 0) {
        pid = fork ();
        if (pid == 0) {
            char *argv[4];
            argv[0] = "sh";
            argv[1] = "-c";
            argv[2] = cmd;
            argv[3] = 0;
            execve ("/bin/sh", argv, environ);
            exit (0);
        }
        exit (0);
    }
    return 0;
}


#ifdef linux
int acpi_read(AcpiInfos *i) {
    FILE        *fd;
    int         retcode = 0;
    int 	capacity,remain;
    char 	*buf;
    char 	*ptr;
    char 	stat;

    buf=(char *)malloc(sizeof(char)*512);

 // new performance code
 if( performance_flag ) {
  if ((fd = fopen("/proc/acpi/processor/CPU0/performance", "r"))){
             fread(buf,512,1,fd);
             fclose(fd);
             if(ptr = strstr(buf,"active state:")) {
                ptr += 26;
                sscanf(ptr,"%d",&i->throttle_state);
              }
          }
 } else { 
 if ((fd = fopen("/proc/acpi/processor/CPU0/throttling", "r"))) {
	          fread(buf,512,1,fd);
	          fclose(fd);
	          if(ptr = strstr(buf,"active state:")) {
	             ptr += 26;
	             sscanf(ptr,"%d",&i->throttle_state);
	           }
	       }
	}
    free(buf);
    sscanf(tpercent[cur_acpi_infos.throttle_state],"%d",&i->throttle_perc);
    return retcode;
}

// get cpu temperature
int acpi_read_temp(int *temp) {
	FILE		*fd;
	char		*ptr;
	char		*buf;
	char		*tmp;
	tmp = (char *)malloc(sizeof(char)*2);
	
	buf=(char *)malloc(sizeof(char)*512);

	if ((fd = fopen("/proc/acpi/thermal_zone/THRM/temperature", "r"))){
				fread(buf,512,1,fd);
             fclose(fd);

             if(ptr = strstr(buf,"temperature:")) {
               
					ptr += 25;
					strncpy( tmp, ptr, 2 );
					sscanf(tmp,"%d",  &temp ); //&i->throttle_state);
              }
	}
	free( buf );
	free( tmp );
	return temp;
}

// throttle according to the current temperature
void throttle_by_temp(int cur_temp ) {
	
	int state;

	// calculate new state
	if( cur_temp < temperature_slowdown )
		state = 0;
	else if( cur_temp == temperature_slowdown )
		state = 1;
	else if( cur_temp >= temperature_slowdown + ( statecount - 2 ) * temperature_delta )
		state = statecount - 1;
	else
		state = 1 + ( cur_temp - temperature_slowdown ) / temperature_delta;

	// change state
	throttle_speed( state );
}

//get acpi state count (seems to work for throttling, too)
int acpi_get_statecount() {
	 FILE    *fd;
    int		*retcode;
    char 	*buf;
    char 	*ptr;
	 
    buf=(char *)malloc(sizeof(char)*512);

 if( performance_flag ) {
  if ((fd = fopen("/proc/acpi/processor/CPU0/performance", "r"))){
             fread(buf,512,1,fd);
             fclose(fd);
             if(ptr = strstr(buf,"state count:")) {
                ptr += 25;
                sscanf(ptr,"%d",&retcode);
              }
          }
 } else { 
 if ((fd = fopen("/proc/acpi/processor/CPU0/throttling", "r"))) {
	          fread(buf,512,1,fd);
	          fclose(fd);
	          if(ptr = strstr(buf,"state count:")) {
	             ptr += 25;
	             sscanf(ptr,"%d",&retcode);
	           }
	       }
	}
    free(buf);
    return retcode;
}
#endif

