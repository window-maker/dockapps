/*
 *    wmbatteries - A dockapp to monitor ACPI status of two batteries
 *    Copyright (C) 2003  Florian Krohs <krohs@uni.de>

 *    Based on work by Thomas Nemeth <tnemeth@free.fr>
 *    Copyright (C) 2002  Thomas Nemeth <tnemeth@free.fr>
 *    and on work by Seiichi SATO <ssato@sh.rim.or.jp>
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "files.h"
#include <signal.h>
#include "dockapp.h"
#include "backlight_on.xpm"
#include "backlight_off.xpm"
#include "parts.xpm"
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef linux
#include <sys/stat.h>
#endif

#define WMBATTERIES_VERSION "0.1.3"

#define FREE(data) {if (data) free (data); data = NULL;}

#define RATE_HISTORY 10
#define BLINK_ONLOADING_TIMEOUT 500
#define SIZE	    58
#define MAXSTRLEN   512
#define WINDOWED_BG ". c #AEAAAE"
#define MAX_HISTORY 16
#define CPUNUM_NONE -1

#define CHARGING 3
#define DISCHARGING 1
#define UNKNOWN 0

#define TOGGLEMODE 1
#define TOGGLESPEED 2

#define DEFAULT_UPDATE_INTERVAL 5

#define RATE 0
#define TEMP 1

#define NONE     0
#define STATE_OK 1
#define INFO_OK  2
#define BAT_OK   3

#ifdef DEBUG
#define DEBUGSTRING(STRING) printf("DEBUG: %s\n",STRING);
#endif

#ifndef DEBUG
#define DEBUGSTRING(STRING)
#endif

# ifdef linux
#  define ACPIDEV "/proc/acpi/info"
# endif

typedef struct AcpiInfos {
  const char driver_version[10];
  int        ac_line_status;
  int        battery_status[2];
  int        battery_percentage[2];
  long       rate[2];
  long       remain[2];
  long       currcap[2];
  int	       thermal_temp;
  int	       thermal_state;
  int	       hours_left;
  int	       minutes_left;
  int          low;
} AcpiInfos;

typedef struct RateListElem {
  long rate[2];
  struct RateListElem *next;	
} RateListElem;

typedef enum { LIGHTOFF, LIGHTON } light;


Pixmap pixmap;
Pixmap backdrop_on;
Pixmap backdrop_off;
Pixmap parts;
Pixmap mask;
static char	*display_name     = "";
static char	light_color[256]  = "";	/* back-light color */
char            tmp_string[256];
static char	*config_file      = NULL;	/* name of configfile */
static unsigned update_interval   = DEFAULT_UPDATE_INTERVAL;
static light    backlight         = LIGHTOFF;
static unsigned switch_authorized = True;
static unsigned alarm_level       = 20;
static unsigned alarm_level_temp  = 70;
static char     *notif_cmd        = NULL;
static char     *suspend_cmd      = NULL;
static char     *standby_cmd      = NULL;
static int 	mode			  = TEMP;
static int 	togglemode		  = TOGGLEMODE;
static int 	togglespeed		  = TOGGLESPEED;
static int 	animationspeed    = 500;
static AcpiInfos cur_acpi_infos;
static number_of_batteries = 2;
static char state_files[2][256]={BAT0_STATE_FILE,BAT1_STATE_FILE};
static char info_files[2][256]={BAT0_INFO_FILE,BAT1_INFO_FILE};
static char thermal[256]=THERMAL_FILE;
static char ac_state[256]=AC_STATE_FILE;
static int      history_size      = RATE_HISTORY;

static RateListElem *rateElements;
static RateListElem *firstRateElem;

#ifdef linux
# ifndef ACPI_32_BIT_SUPPORT
#  define ACPI_32_BIT_SUPPORT      0x0002
# endif
#endif


/* prototypes */
static void parse_config_file(char *config);
static void update();
static void switch_light();
static void draw_remaining_time(AcpiInfos infos);
static void draw_batt(AcpiInfos infos);
static void draw_low();
static void draw_rate(AcpiInfos infos);
static void draw_temp(AcpiInfos infos);
static void draw_statusdigit(AcpiInfos infos);
static void draw_pcgraph(AcpiInfos infos);
static void parse_arguments(int argc, char **argv);
static void print_help(char *prog);
static void acpi_getinfos(AcpiInfos *infos);
static int  acpi_exists();
static int  my_system (char *cmd);
static void blink_batt();
static void draw_all();

static void debug(char *debug_string);
#ifdef linux
int acpi_read(AcpiInfos *i);
int init_stats(AcpiInfos *k);
#endif

int count;
int blink_pos=0;

static void debug(char *debug_string){
  printf("DEBUG: %s\n",debug_string);
}

int main(int argc, char **argv) {
	
  XEvent   event;
  XpmColorSymbol colors[2] = { {"Back0", NULL, 0}, {"Back1", NULL, 0} };
  int      ncolor = 0;
  struct   sigaction sa;
  long counter=0;
  long timeout;
  int charging;
  long togglecounter=0;
  long animationcounter=0;

  sa.sa_handler = SIG_IGN;
#ifdef SA_NOCLDWAIT
  sa.sa_flags = SA_NOCLDWAIT;
#else
  sa.sa_flags = 0;
#endif


  printf("wmbatteries %s  (c) Florian Krohs\n"
         "<florian.krohs@informatik.uni-oldenburg.de>\n\n"
         "This Software comes with absolut no warranty.\n"
         "Use at your own risk!\n\n",WMBATTERIES_VERSION);

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

  init_stats(&cur_acpi_infos);
  //acpi_getinfos(&cur_acpi_infos);
  //update();
  dockapp_open_window(display_name, PACKAGE, SIZE, SIZE, argc, argv);
  dockapp_set_eventmask(ButtonPressMask);

  if (strcmp(light_color,"")) {
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
  update();
  dockapp_show();
  long update_timeout = update_interval*1000;
  long animation_timeout = animationspeed;
  long toggle_timeout = togglespeed*1000;
  int show = 0;
  /* Main loop */
  while (1) {
    if (cur_acpi_infos.battery_status[0]==CHARGING || cur_acpi_infos.battery_status[1]==CHARGING)
      charging = 1;
    else 
      charging = 0;
    timeout = update_timeout;
    if( charging && animation_timeout<update_timeout){
      if(animation_timeout<toggle_timeout)
	timeout = animation_timeout;
      else if(togglemode) timeout = toggle_timeout;	
    } else if(update_timeout<toggle_timeout)
      timeout = update_timeout;
    else if(togglemode) timeout = toggle_timeout;
    if (dockapp_nextevent_or_timeout(&event, timeout)) {
      /* Next Event */
      switch (event.type) {
      case ButtonPress:
	switch (event.xbutton.button) {
	case 1: switch_light(); break;
	case 3: mode=!mode; draw_all();dockapp_copy2window(pixmap);break;
	default: break;
	}
	break;
      default: break;
      }
    } else {
      /* Time Out */
      update_timeout -= timeout;
      animation_timeout -= timeout;
      toggle_timeout -= timeout; 
      if(toggle_timeout<=0){
	toggle_timeout = togglespeed*1000;
	if(togglemode){
	  mode=!mode;
	  show = 1;
	}
      }
      if(animation_timeout<=0){
	animation_timeout = animationspeed;	
	if(charging){
	  blink_batt();
	  show = 1;
	}
      }
      if(update_timeout<=0){
	update();
	show = 1;
	update_timeout = update_interval*1000;
      }            		
      if(show) {
	/* show */
	draw_all();
	if(charging) {
	  blink_pos--;
	  blink_batt();
	}
	dockapp_copy2window(pixmap);
	show = 0;
      }
    }
  }

  return 0;
}


int init_stats(AcpiInfos *k) {
  int bat_status[2]={NONE,NONE};
  FILE *fd;
  char *buf;
  char *ptr;
  char present;
  int bat;
  int hist;
  int i;

  buf=(char *)malloc(sizeof(char)*512);
  if(buf == NULL)
    exit(-1);
  /* get info about existing batteries */
  number_of_batteries=0;
  for(i=0;i<2;i++){
    if((fd = fopen(state_files[i], "r"))){
      fread(buf,512,1,fd);
      fclose(fd);
      if(ptr = strstr(buf,"present:")) {
	present=*(ptr+25);
	if(present == 'y'){
	  bat_status[i]|=STATE_OK;
	}
      }
      if(ptr = strstr(buf,"present rate:")) {
	present=*(ptr+25);
	sscanf(ptr,"%d",&k->rate[bat]);
      }
    }
    if((fd = fopen(info_files[i], "r"))){
      fread(buf,512,1,fd);
      fclose(fd);
      if(ptr = strstr(buf,"present:")) {
	present=*(ptr+25);
	if(present == 'y'){
	  bat_status[i]|=INFO_OK;
	}
      }
      if(ptr = strstr(buf,"last full capacity:")) {
	present=*(ptr+25);
	sscanf(ptr,"%d",&k->currcap[bat]);
      }
    }


  }
  if(bat_status[0]==BAT_OK && bat_status[1]==BAT_OK){
    printf("BAT0 and BAT1 ok\n");
    number_of_batteries=2;
  } else if(bat_status[0]==BAT_OK) {
    printf("BAT0 ok\n");
    number_of_batteries=1;
  } else if(bat_status[1]==BAT_OK) {
    printf("BAT1 ok\n");
    number_of_batteries=1;
    strcpy(state_files[0],state_files[1]);
    strcpy(info_files[0],info_files[1]);
    k->currcap[0] = k->currcap[1];
    k->rate[0] = k->rate[1];
  }

  printf("%i batter%s found in system\n",number_of_batteries,number_of_batteries==1 ? "y" : "ies");

  // initialize buffer
  if ((rateElements = (RateListElem *) malloc(history_size * sizeof(RateListElem))) == NULL)
    exit(-1);
      
  firstRateElem = rateElements;


  /* get info about full battery charge */

  for(bat=0;bat<number_of_batteries;bat++){
    if ((fd = fopen(info_files[bat], "r"))) {
      fread(buf,512,1,fd);
      fclose(fd);
      if(ptr = strstr(buf,"last full capacity:")) {
	ptr += 25;
	sscanf(ptr,"%d",&k->currcap[bat]);
      }
    } 
    if ((fd = fopen(state_files[bat], "r"))) {
      fread(buf,512,1,fd);
      fclose(fd);
      if(ptr = strstr(buf,"present rate:")) {
	ptr += 25;
	sscanf(ptr,"%d",&k->rate[bat]);
      }
    }

  }
  for(i=0;i<2;i++){
    /* link rateElements */
    for(hist=0;hist<(history_size-1);hist++){
      (*(rateElements+hist)).next = rateElements+hist+1;
      (*(rateElements+hist)).rate[i] = k->rate[i];
    }
    (*(rateElements+history_size-1)).next = rateElements;
    (*(rateElements+history_size-1)).rate[i] = k->rate[i];    
  }
  free(buf);
  k->ac_line_status = 0;
  k->battery_status[0] = 0;
  k->battery_percentage[0] = 0;
  k->remain[0] = 0;
  k->battery_status[1] = 0;
  k->battery_percentage[1] = 0;
  k->remain[1] = 0;
  k->thermal_temp = 0;
  k->thermal_state = 0;
  DEBUGSTRING("end of init_stats()");
}

/* called by timer */
static void update() {
  static light pre_backlight;
  static Bool in_alarm_mode = False;

  /* get current battery usage in percent */
  acpi_getinfos(&cur_acpi_infos);

  /* alarm mode */
  if (cur_acpi_infos.low || (cur_acpi_infos.thermal_temp > alarm_level_temp)) {
    if (!in_alarm_mode) {
      in_alarm_mode = True;
      pre_backlight = backlight;
      my_system(notif_cmd);
    }
    if ( (switch_authorized) ||
	 ( (! switch_authorized) && (backlight != pre_backlight) ) ) {
      switch_light();
      return;
    }
  }
  else {
    if (in_alarm_mode) {
      in_alarm_mode = False;
      if (backlight != pre_backlight) {
	switch_light();
	return;
      }
    }
  }
  draw_all();
}

static void parse_config_file(char *config){

  FILE *fd=NULL;
  char *buf;
  char stringbuffer[256];
  char *ptr;
  char line[256] ;
  char *item;
  char *value;
  extern int errno;
  int linenr=0;
  int tmp;
  char *test;
  buf=(char *)malloc(sizeof(char)*512);
  if(buf == NULL)
    exit(-1);
  if(config != NULL) { //config file by command line
    DEBUGSTRING("using command line given config file name");
    DEBUGSTRING(config);
    if((fd = fopen(config, "r"))){
      DEBUGSTRING("config file found\n");
    } else {
      DEBUGSTRING("config file NOT found\n");
      DEBUGSTRING("falling back to default config file\n");
    }
  }
  if(fd==NULL) { // no config file found yet
      strcpy(stringbuffer,getenv("HOME"));
      strcat(stringbuffer,"/.wmbatteriesrc");
      DEBUGSTRING("trying config file in your $HOME dir\n");
      DEBUGSTRING(stringbuffer);
      if((fd = fopen(stringbuffer, "r"))){
    	DEBUGSTRING("config file found\n");
      } else {
	DEBUGSTRING("config file in $HOME dir nonexistant\n");
	DEBUGSTRING("trying global one in /etc\n");	
	if((fd = fopen("/etc/wmbatteries", "r"))){
	  DEBUGSTRING("config file found\n");
	}
	else {
	  DEBUGSTRING("no config file found. ignoring\n");
	}
      }
    }

  if(fd!=NULL){ // some config file was found, try parsing
    DEBUGSTRING("begin parsing\n");
    while( fgets( line, 255, fd ) != NULL )
    {
      
        item = strtok( line, "\t =\n\r" ) ;
        if( item != NULL && item[0] != '#' )
        {
            value = strtok( NULL, "\t =\n\r" ) ;
	    if(!strcmp(item,"backlight")){
	      if(strcasecmp(value,"yes") && strcasecmp(value,"true") && strcasecmp(value,"false") && strcasecmp(value,"no")) {
		printf("backlight option wrong in line %i,use yes/no or true/false\n",linenr);
	      } else {
		if(!strcasecmp(value,"true") || !strcasecmp(value,"yes")){
		  backlight = LIGHTON;
		} else {
		  backlight = LIGHTOFF;
		}
	      }	    
	    }

	    if(!strcmp(item,"lightcolor")){
	      strcpy(light_color,value);
	    }

	    if(!strcmp(item,"temperature")){
	      strcpy(thermal,value);
	    }

	    if(!strcmp(item,"bat0_state")){
	      strcpy(state_files[0],value);
	    }

	    if(!strcmp(item,"bat1_state")){
	      strcpy(state_files[1],value);
	    }

	    if(!strcmp(item,"bat0_info")){
	      strcpy(info_files[0],value);
	    }

	    if(!strcmp(item,"bat1_info")){
	      strcpy(info_files[1],value);
	    }

	    if(!strcmp(item,"ac_state")){
	      strcpy(ac_state,value);
	    }


	    if(!strcmp(item,"updateinterval")){
	      tmp=atoi(value);
	      if(tmp<1) {
		printf("update interval is out of range in line %i,must be > 0\n",linenr);
	      } else {
		update_interval=tmp;
	      }
	    }

	    if(!strcmp(item,"alarm")){
	      tmp=atoi(value);
	      if(tmp<1 || tmp>100) {
		printf("alarm is out of range in line %i,must be > 0 and <= 100\n",linenr);
	      } else {
		alarm_level=tmp;
	      }
	    }

	    if(!strcmp(item,"togglespeed")){
	      tmp=atoi(value);
	      if(tmp<1) {
		printf("togglespeed variable is out of range in line %i,must be > 0\n",linenr);
	      } else {
		togglespeed=tmp;
	      }
	    }

	    if(!strcmp(item,"animationspeed")){
	      tmp=atoi(value);
	      if(tmp<100) {
		printf("animationspeed variable is out of range in line %i,must be >= 100\n",linenr);
	      } else { 
		animationspeed=tmp;
	      }
	    }

	    if(!strcmp(item,"historysize")){
	      tmp=atoi(value);
	      if(tmp<1 || tmp>1000) {
		printf("historysize variable is out of range in line %i,must be >=1 and <=1000\n",linenr);
	      } else { 
		history_size=tmp;
	      }
	    }

	    if(!strcmp(item,"mode")){
	      if(strcmp(value,"rate") && strcmp(value,"toggle") && strcmp(value,"toggle")) {
		printf("mode must be one of rate,temp,toggle in line %i\n",linenr);
	      } else { 
		if(strcmp(value,"rate")) mode=RATE;
		if(strcmp(value,"temp")) mode=TEMP;
		if(strcmp(value,"toggle")) togglemode=1;
	      }
	    }



        }
	linenr++;
    }
    fclose(fd);
    DEBUGSTRING("end parsing\n");
  }
}


static void draw_all(){
  int bat;
  long allremain=0;
  long allcapacity=0;
  /* all clear */
  if (backlight == LIGHTON) 
    dockapp_copyarea(backdrop_on, pixmap, 0, 0, 58, 58, 0, 0);
  else 
    dockapp_copyarea(backdrop_off, pixmap, 0, 0, 58, 58, 0, 0);
  /* draw digit */
  draw_remaining_time(cur_acpi_infos);
  if(mode==RATE) draw_rate(cur_acpi_infos);
  else if(mode==TEMP) draw_temp(cur_acpi_infos);
  draw_statusdigit(cur_acpi_infos);
  draw_pcgraph(cur_acpi_infos);

  if(cur_acpi_infos.low) draw_low();
  
  draw_batt(cur_acpi_infos);
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

  draw_remaining_time(cur_acpi_infos);
  if(mode==RATE) draw_rate(cur_acpi_infos);
  else if(mode==TEMP) draw_temp(cur_acpi_infos);
  draw_statusdigit(cur_acpi_infos);
  draw_pcgraph(cur_acpi_infos);
  if(cur_acpi_infos.battery_status[0]==CHARGING || cur_acpi_infos.battery_status[1]==CHARGING){
    blink_batt();
  } else draw_batt(cur_acpi_infos);
  if(cur_acpi_infos.low){
    draw_low();
  }
  /* show */
  dockapp_copy2window(pixmap);
}

static void draw_batt(AcpiInfos infos){
  int y = 0;
  int i=0;
  if (backlight == LIGHTON) y = 28;
  for(i=0;i<number_of_batteries;i++){
    if(infos.battery_status[i]==DISCHARGING){	
      dockapp_copyarea(parts, pixmap,33+y , 63, 9, 5, 16+i*11, 39);
    }
  }
}

static void draw_remaining_time(AcpiInfos infos) {
  int y = 0;
  if (backlight == LIGHTON) y = 20;
  if (infos.ac_line_status == 1 && !(cur_acpi_infos.battery_status[0]==CHARGING || cur_acpi_infos.battery_status[1]==CHARGING)){
    dockapp_copyarea(parts, pixmap, 0, 68+68+y, 10, 20,  17, 5);
    dockapp_copyarea(parts, pixmap, 10, 68+68+y, 10, 20,  32, 5);
  } else {

    dockapp_copyarea(parts, pixmap, (infos.hours_left / 10) * 10, 68+y, 10, 20,  5, 5);
    dockapp_copyarea(parts, pixmap, (infos.hours_left % 10) * 10, 68+y, 10, 20, 17, 5);
    dockapp_copyarea(parts, pixmap, (infos.minutes_left / 10)  * 10, 68+y, 10, 20, 32, 5);
    dockapp_copyarea(parts, pixmap, (infos.minutes_left % 10)  * 10, 68+y, 10, 20, 44, 5);
  }

}

static void draw_low() {
  int y = 0;
  if (backlight == LIGHTON) y = 28;
  dockapp_copyarea(parts, pixmap,42+y , 58, 17, 7, 38, 38);

}

static void draw_temp(AcpiInfos infos) {
  int temp = infos.thermal_temp;
  int light_offset=0;
  if (backlight == LIGHTON) {
    light_offset=50;
  }

  if (temp < 0 || temp>99)  temp = 0;
  dockapp_copyarea(parts, pixmap, (temp/10)*5 + light_offset, 40, 5, 9, 23, 46);
  dockapp_copyarea(parts, pixmap, (temp%10)*5 + light_offset, 40, 5, 9, 29, 46);

  dockapp_copyarea(parts, pixmap, 10 + light_offset, 49, 5, 9, 36, 46);  //o
  dockapp_copyarea(parts, pixmap, 15 + light_offset, 49, 5, 9, 42, 46);  //C

}

static void blink_batt(){
  int light_offset=0;
  int bat=0;
  if (backlight == LIGHTON) {
    light_offset=50;
  }
  blink_pos=(blink_pos+1)%5;
  for(bat=0;bat<number_of_batteries;bat++){
    if(cur_acpi_infos.battery_status[bat]==CHARGING){
      dockapp_copyarea(parts, pixmap, blink_pos*9+light_offset , 117, 9, 5,  16+bat*11, 39);
    }
  }    
}


static void draw_statusdigit(AcpiInfos infos) {
  int light_offset=0;
  if (backlight == LIGHTON) {
    light_offset=28;
  }
  if (infos.ac_line_status == 1){
    dockapp_copyarea(parts, pixmap,33+light_offset , 58, 9, 5, 5, 39);
  }
}

static void draw_rate(AcpiInfos infos) {
  int light_offset=0;
  long rate = infos.rate[0]+infos.rate[1];
  if (backlight == LIGHTON) {
    light_offset=50;
  }

  dockapp_copyarea(parts, pixmap, (rate/10000)*5 + light_offset, 40, 5, 9, 5, 46);
  dockapp_copyarea(parts, pixmap, ((rate/1000)%10)*5 + light_offset, 40, 5, 9, 11, 46);
  dockapp_copyarea(parts, pixmap, ((rate/100)%10)*5 + light_offset, 40, 5, 9, 17, 46);
  dockapp_copyarea(parts, pixmap, ((rate/10)%10)*5 + light_offset, 40, 5, 9, 23, 46);
  dockapp_copyarea(parts, pixmap, (rate%10)*5 + light_offset, 40, 5, 9, 29, 46);

  dockapp_copyarea(parts, pixmap, 0 + light_offset, 49, 5, 9, 36, 46);  //m
  dockapp_copyarea(parts, pixmap, 5 + light_offset, 49, 5, 9, 42, 46);  //W

}

static void draw_pcgraph(AcpiInfos infos) {
  int num[2];
  int bat;
  int width;
  int light_offset=0;
  if (backlight == LIGHTON) {
    light_offset=5;
  }
  for(bat=0;bat<number_of_batteries;bat++){
    width = (infos.battery_percentage[bat]*32)/100;	
    dockapp_copyarea(parts, pixmap, 0, 58+light_offset, width, 5, 5, 26+6*bat);
    if(infos.battery_percentage[bat] == 100){ // don't display leading 0
      dockapp_copyarea(parts, pixmap, 4*(infos.battery_percentage[bat]/100), 126+light_offset, 3, 5, 38, 26+6*bat);
    }
    if(infos.battery_percentage[bat] > 9){ //don't display leading 0
      dockapp_copyarea(parts, pixmap, 4*((infos.battery_percentage[bat]%100)/10), 126+light_offset, 3, 5, 42, 26+6*bat);
    }
    dockapp_copyarea(parts, pixmap, 4*(infos.battery_percentage[bat]%10), 126+light_offset, 3, 5, 46, 26+6*bat);		
  }

}


static void parse_arguments(int argc, char **argv) {
  int i;
  int integer;
  char character;

  for (i = 1; i < argc; i++) { // first search for config file option
    if (!strcmp(argv[i], "--config") || !strcmp(argv[i], "-c")) {
      config_file = argv[i + 1];
      i++;
    }
  }
  // parse config file before other command line options, to allow overriding
  parse_config_file(config_file);
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
      print_help(argv[0]), exit(0);
    } else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")) {
      printf("%s version %s\n", PACKAGE, VERSION), exit(0);
    } else if (!strcmp(argv[i], "--display") || !strcmp(argv[i], "-d")) {
      display_name = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--backlight") || !strcmp(argv[i], "-bl")) {
      backlight = LIGHTON;
    } else if (!strcmp(argv[i], "--light-color") || !strcmp(argv[i], "-lc")) {
      strcpy(light_color,argv[i + 1]);
      i++;
    } else if (!strcmp(argv[i], "--config") || !strcmp(argv[i], "-c")) {
      config_file = argv[i + 1];
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
    } else if (!strcmp(argv[i], "--suspend") || !strcmp(argv[i], "-s")) {
      suspend_cmd = argv[i + 1];
      i++;
    } else if (!strcmp(argv[i], "--togglespeed") || !strcmp(argv[i], "-ts")) {
      if (argc == i + 1)
	fprintf(stderr, "%s: error parsing argument for option %s\n",
		argv[0], argv[i]), exit(1);
      if (sscanf(argv[i + 1], "%i", &integer) != 1)
	fprintf(stderr, "%s: error parsing argument for option %s\n",
		argv[0], argv[i]), exit(1);
      if ( integer < 1)
	fprintf(stderr, "%s: argument %s must be positive integer\n",
		argv[0], argv[i],update_interval), exit(1);
      togglespeed=integer;                        
      i++;                        
    } else if (!strcmp(argv[i], "--animationspeed") || !strcmp(argv[i], "-as")) {
      if (argc == i + 1)
	fprintf(stderr, "%s: error parsing argument for option %s\n",
		argv[0], argv[i]), exit(1);
      if (sscanf(argv[i + 1], "%i", &integer) != 1)
	fprintf(stderr, "%s: error parsing argument for option %s\n",
		argv[0], argv[i]), exit(1);
      if (integer < 100)
	fprintf(stderr, "%s: argument %s must be >=100\n",
		argv[0], argv[i]), exit(1);
      animationspeed=integer;            
      i++;                                                                       
    } else if (!strcmp(argv[i], "--historysize") || !strcmp(argv[i], "-hs")) {
      if (argc == i + 1)
	fprintf(stderr, "%s: error parsing argument for option %s\n",
		argv[0], argv[i]), exit(1);
      if (sscanf(argv[i + 1], "%i", &integer) != 1)
	fprintf(stderr, "%s: error parsing argument for option %s\n",
		argv[0], argv[i]), exit(1);
      if (integer < 1 || integer > 1000)
	fprintf(stderr, "%s: argument %s must be >=1 && <=1000\n",
		argv[0], argv[i]), exit(1);
      history_size=integer;
      i++;                                                                       
    } else if (!strcmp(argv[i], "--mode") || !strcmp(argv[i], "-m")) {
      if (argc == i + 1)
	fprintf(stderr, "%s: error parsing argument for option %s\n",
		argv[0], argv[i]), exit(1);
      if (sscanf(argv[i + 1], "%c", &character) != 1)
	fprintf(stderr, "%s: error parsing argument for option %s\n",
		argv[0], argv[i]), exit(1);
      if (!(character=='t' || character=='r' || character=='s'))
	fprintf(stderr, "%s: argument %s must be t,r or s\n",
		argv[0], argv[i]), exit(1);
      if(character=='s') togglemode=1;
      else if(character=='t') mode=TEMP;
      else if(character=='r') mode=RATE;	
      i++;
    } else if (!strcmp(argv[i], "--standby") || !strcmp(argv[i], "-S")) {
      standby_cmd = argv[i + 1];
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
	 "%s - Window Maker mails monitor dockapp\n"
	 "  -d,  --display <string>        display to use\n"
	 "  -bl, --backlight               turn on back-light\n"
	 "  -lc, --light-color <string>    back-light color(rgb:6E/C6/3B is default)\n"
	 "  -c,  --config <string>         set filename of config file\n"
	 "  -i,  --interval <number>       number of secs between updates (1 is default)\n"
	 "  -a,  --alarm <number>          low battery level when to raise alarm\n"
	 "                                 (20 is default)\n"
	 "  -h,  --help                    show this help text and exit\n"
	 "  -v,  --version                 show program version and exit\n"
	 "  -w,  --windowed                run the application in windowed mode\n"
	 "  -bw, --broken-wm               activate broken window manager fix\n"
	 "  -n,  --notify <string>         command to launch when alarm is on\n"
	 "  -s,  --suspend <string>        set command for acpi suspend\n"
	 "  -S,  --standby <string>        set command for acpi standby\n"
	 "  -m,  --mode [t|r|s]            set mode for the lower row , \n"
	 "                                 t=temperature,r=current rate,s=toggle\n"
	 "  -ts  --togglespeed <int>       set toggle speed in seconds\n"           
	 "  -as  --animationspeed <int>    set speed for charging animation in msec\n"           
	 "  -hs  --historysize <int>       set size of history for calculating\n"
	 "                                 average power consumption rate\n",           
	 prog, prog);
  /* OPTIONS SUPP :
   *  ? -f, --file    : configuration file
   */
}


static void acpi_getinfos(AcpiInfos *infos) {
  DEBUGSTRING("acpi_getinfos\n")
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
  int 	capacity[2],remain[2];
  int bat;
  char 	*buf;
  char 	*ptr;
  char 	stat;
  buf=(char *)malloc(sizeof(char)*512);
  RateListElem currRateElement;
  int hist;
  long rate;
  float time;
  long allcapacity=0;
  long allremain=0;

  rate = 0;
  
  
  DEBUGSTRING("acpi_read()\n")

  /* get acpi thermal cpu info */
  if ((fd = fopen(thermal, "r"))) {
    fscanf(fd, "temperature: %d", &i->thermal_temp);
    fclose(fd);
  }
  if ((fd = fopen(ac_state, "r"))) {
    bzero(buf, 512);
    fscanf(fd, "state: %s", buf);
    fclose(fd);
    if(strstr(buf, "on-line") != NULL) i->ac_line_status=1;
    if(strstr(buf, "off-line") != NULL) i->ac_line_status=0;
  }
  for(bat=0;bat<number_of_batteries;bat++){
  
    if ((fd = fopen(state_files[bat], "r"))) {
      bzero(buf, 512);
      fread(buf,512,1,fd);
      fclose(fd);
      if(( ptr = strstr(buf,"charging state:"))) {
	stat = *(ptr + 25);
	switch (stat) 
	  {
	  case 'd':
	    i->battery_status[bat]=1;
	    break;
	  case 'c':
	    i->battery_status[bat]=3;
	    break;
	  case 'u':
	    i->battery_status[bat]=0;
	    break;
	  }
      }
      if ((ptr = strstr (buf, "remaining capacity:"))) {
	ptr += 25;
	sscanf(ptr,"%d",&i->remain[bat]);
      }
      if ((ptr = strstr (buf, "present rate:"))) {
	ptr += 25;
	sscanf(ptr,"%d",&((*firstRateElem).rate[bat]));
      }
    }
     
    i->battery_percentage[bat] = (((float)(i->remain[bat])*100)/cur_acpi_infos.currcap[bat]);



    currRateElement = *firstRateElem;
    if(currRateElement.rate[bat]!=0){
      for(hist=0;hist<history_size;hist++){
	if(currRateElement.rate[bat]!=0){
	  rate += currRateElement.rate[bat];
	} else rate+= (*firstRateElem).rate[bat];
	currRateElement = *currRateElement.next;
      }
    } else {
      rate=0;
      i->rate[bat]=0;	
    }



    /* calc average */
    rate = rate / history_size;
    i->rate[bat] = rate;
  }

  if((i->battery_status[0]==1 || i->battery_status[1]==1) && (i->rate[0]+i->rate[1])>0){
    time = (float)(i->remain[0]+i->remain[1])/(float)(i->rate[0]+i->rate[1]);
    i->hours_left=(int)time;
    i->minutes_left=(int)((time-(int)time)*60);	
  }
  if(i->battery_status[0]==0 && i->battery_status[1]==0){
    i->hours_left=0;
    i->minutes_left=0;	
  }
  if((i->battery_status[0]==3||i->battery_status[1]==3) && (i->rate[0]>0 || i->rate[1]>0)){
    time = (float)(cur_acpi_infos.currcap[0] - i->remain[0] + cur_acpi_infos.currcap[1] - i->remain[1])/(float)(i->rate[0]+i->rate[1]);
    i->hours_left=(int)time;
    i->minutes_left=(int)(60*(time-(int)time));
  }
  for(bat=0;bat<number_of_batteries;bat++){
    allremain += i->remain[bat];
    allcapacity += cur_acpi_infos.currcap[bat];
  }

  cur_acpi_infos.low=0;
  if(allcapacity>0){
    if(((double)allremain/(double)allcapacity)*100<alarm_level){ 
      cur_acpi_infos.low=1;
    }
  } 

  DEBUGSTRING("MID acpi_read()\n") 
  firstRateElem = ((*firstRateElem).next);	
  free(buf);
  DEBUGSTRING("END acpi_read()\n")
  return retcode;
}
#endif
