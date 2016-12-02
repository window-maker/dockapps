#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <getopt.h>

#include "wmgeneral.h"
#include "pixmaps.h"

#define HOUR 1
#define MINUTE 2
#define SECOND 3

#define VERSION "0.1"
#define RELEASED "October 2006"

/* Prototypes */
void beat(void);
void display_time(int number,int time_unit);
void display_lines(time_t seconds);
void display_date(int day_of_week, int day_of_month, int month, int year,int mode,time_t seconds);
void setColor(char *color);
void setMode(char *mode);
void display_help(void);
void display_version(void);
bool errorColor(char *color);
bool errorMode(char *mode);

/*Global Variables*/
time_t actualtime;
int g_mode=0;
char color_line[12]="+\tc #";


bool errorMode(char *mode)
{
    if (strncmp(mode,"0",1)!=0 && strncmp(mode,"1",1)!=0)
        return true;
    else
        return false;
}

bool errorColor(char *color)
{
    int i=0;

    if (strlen(color)!=6)
        return true;
    

    for (i=0;i<6;i++)
    {
        if ((color[i]<48 || color[i]>57) && (color[i]<65 || color[i]>70) && (color[i]<97 || color[i]>102))
            return true;
    }
    
    return false;
}

void print_version()
{
	puts("\033[1mwmHexaClock\033[0m");
	printf("Version: \t%s\n",VERSION);
	printf("Released: \t%s\n",RELEASED);
	puts("Info: \t\thttp://www.bjoernw.de/");
	exit (0);
}

void print_help()
{
	puts("Usage: wmHexaClock [OPTIONS]");
	puts("Possible Options are:\n");
	puts(" -v or --version:\t\tGives information about version and release date");
	puts(" -h or --help:\t\t\tPrints this dialog");
	puts(" -c [COLOR] or --color [COLOR]:\tSpecifies the foreground color; for [COLOR] you use the hexadecimal notation RRGGBB (R=red, G=green, B=blue)");
	puts(" \t\t\t\tExample: \"wmHexaClock -c FF0000\" for red");
	puts(" -m [i] or --mode [i]:\t\tThere are two different display modes; for [i] you type 0 or 1 to chose between them (0 is default)");
	exit(0);
}

void setColor(char *color)
{
	if (!errorColor(color))
		xpm_numbers[3]=strncat(color_line,color,6); //Edit the Color-Line in xpm_numbers (-> pixmaps.h)
	else
	{
		fprintf (stderr,"The Color Code is nor correct! Example: \"-c 00FF00\" for green!\n");
		exit (1);
	}
}

void setMode(char *mode)
{
	if (!errorMode(mode))
		g_mode=((int)*(mode))-48;
	else
	{
		fprintf (stderr,"Please choose between mode 0 and 1. Example: \"-mode 0\"!\n");
		exit (1);
	}
}

int main(int argc, char *argv[])
{
	XEvent	event;
	actualtime=time(0);
   
	static struct option long_options[] =
	{
		/* These options set a flag. */
		{"version", no_argument, 0, 'v'},
		{"help",   no_argument,  0, 'h'},
		{"color", required_argument, 0, 'c'},
		{"mode",  required_argument, 0, 'm'},
		{0, 0, 0, 0}
	};


	while (1)
	{
		int c;
		int option_index = 0;
		     
		c = getopt_long (argc, argv, "vhc:m:", long_options, &option_index);
		
		/* Detect the end of the options. */
		if (c == -1)
	        	break;
		
		switch (c)
		{
			case 'v':
				print_version();
				break;
			case 'h':
				print_help();
				break;
			case 'c':
				setColor(optarg);	
				break;
			case 'm':
				setMode(optarg);
				break;
			case '?':
				puts("Type wmHexaClock -h for information");
				exit(0);
				break;
			default:
				abort();
		}
	}

	openXwindow(argc, argv, xpm_numbers, xpm_master, xpm_mask_bits, xpm_mask_width, xpm_mask_height);

 
	/* Loop Forever */
	while (1) 
	{
		if (actualtime!=time(0))
			beat();    
		/* Process any pending X events. */
		while (XPending(display)) 
		{
			XNextEvent(display, &event);
			if (event.type==Expose)
				RedrawWindow();
		}
		usleep(10000);
	}

	/* we should never get here */
	return (0);
}


void beat()
{
	struct tm *time_struct;
	actualtime=time(0);
	time_struct=localtime(&actualtime);

	cleanXPMArea();
  
	display_time(time_struct->tm_hour,HOUR);
	display_time(time_struct->tm_min,MINUTE);
	display_time(time_struct->tm_sec,SECOND);
	copyXPMArea(177,0,8,12,27,3);   /*Display the two dots*/
	
	display_lines(time_struct->tm_sec);

	display_date(time_struct->tm_wday,time_struct->tm_mday,time_struct->tm_mon,time_struct->tm_year+1900,g_mode,time_struct->tm_sec);
	
	RedrawWindow();   
}

void display_time(int number,int time_unit){

	int first_pos=number/16;
	int second_pos=number%16;

	if (time_unit==HOUR)
	{
		copyXPMArea(first_pos*11,0,11,12,5,5);
		copyXPMArea(second_pos*11,0,11,12,16,5);
	}
	else if (time_unit==MINUTE)
	{
		copyXPMArea(first_pos*11,0,11,12,35,5);
		copyXPMArea(second_pos*11,0,11,12,46,5);
	}
	else if (time_unit==SECOND)
	{
		copyXPMArea(first_pos*7,12,7,8,40,20);
		copyXPMArea(second_pos*7,12,7,8,48,20);
	}

	return;
}


void display_lines(time_t seconds)
{
	int i;

	for (i=0;i<(seconds/8);i++)
	copyXPMArea(0,29,8,4,i*8+4,32);

	return;
}

void display_date(int day_of_week, int day_of_month, int month, int year,int mode,time_t seconds)
{
	int first_pos_day=day_of_month/16;
	int second_pos_day=day_of_month%16;
	int first_pos_month=(month+1)/16;
	int second_pos_month=(month+1)%16;
	int first_pos_year=year/256;
	int second_pos_year=(year-first_pos_year*256)/16;
	int third_pos_year=year-first_pos_year*256-second_pos_year*16;
    
	static bool rec_mode;
    
	if (mode==1)
	{
		copyXPMArea((day_of_week-1)*22,20,22,9,5,39);
		copyXPMArea(first_pos_day*7,12,7,9,30,39);
		copyXPMArea(second_pos_day*7,12,7,9,37,39);
		copyXPMArea(8,30,2,2,45,46);
		copyXPMArea(first_pos_month*7,12,7,9,47,39);
		copyXPMArea(second_pos_month*7,12,7,9,54,39);
		copyXPMArea(first_pos_year*7,12,7,8,34,50);
		copyXPMArea(second_pos_year*7,12,7,8,44,50);
		copyXPMArea(third_pos_year*7,12,7,8,53,50);
	}
	else
	{   
		if (seconds%3==0)
			rec_mode=!rec_mode;
	}

	if (rec_mode)    
			copyXPMArea((day_of_week-1)*31,32,31,12,16,39);
	else
	{
		copyXPMArea(first_pos_day*11,0,11,12,3,36);
		copyXPMArea(second_pos_day*11,0,11,12,14,36);
		copyXPMArea(180,9,4,3,26,46);
		copyXPMArea(first_pos_month*11,0,11,12,32,36);
		copyXPMArea(second_pos_month*11,0,11,12,43,36);
		copyXPMArea(180,9,4,3,55,46);
		copyXPMArea(first_pos_year*7,12,7,8,18,51);
		copyXPMArea(second_pos_year*7,12,7,8,28,51);
		copyXPMArea(third_pos_year*7,12,7,8,37,51);
	}

}
