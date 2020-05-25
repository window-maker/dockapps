/*
	wmcore by bitman 	<bitman@bitmania.de>

				 http://www.bitmania.de

	based on wmlm by ben jarvis <bjarvis@bresnanlink.net>

	This is a dockapp that shows the usage of each core in the system.
	The dockapp splits into two displays, the upper one showing the common usage of the system and the
	lower display showing one graph per each core.

	It detects the number of cores and computes the usage to be represented as a bar graph.
	wmcore works with a variable number of cores, I have tested the display with 1 up to 16 (simulated) cores.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <libdockapp/wmgeneral.h>
#include <libdockapp/misc.h>

#include "wmcore_master.xpm"
#include "wmcore_mask.xbm"




#define DEBUG 0 	// 0 disable, 1 enable

void usage() {
 	printf("Usage: wmcore [options]\n\n");
	printf("  -d n		      Number of microseconds of delay between each screen update.\n");
	printf("		      (The default is 1000000 = 1 sec)\n");
}


#define MAX_CPU 20



int main(int argc, char **argv) {
	short c;
	int delay = 1000000;
	unsigned int t;
	unsigned char h;
	unsigned int cpu;	// current number of cpu when looping over all cpus
	unsigned int cpu_max=0;	// number of detected cpus in the system

	unsigned int  curr_user[MAX_CPU];
	unsigned int  curr_nice[MAX_CPU];
	unsigned int  curr_syst[MAX_CPU];
	unsigned int  curr_idle[MAX_CPU];
	unsigned int  curr_total[MAX_CPU];

	unsigned int  prev_idle[MAX_CPU];
	unsigned int  prev_total[MAX_CPU];

	unsigned int  diff_idle[MAX_CPU];
	unsigned int  diff_total[MAX_CPU];
	unsigned int  diff_usage[MAX_CPU];


	char * token;


	char tmp[200];
	FILE *in;

	XEvent Event;

	fputs(PACKAGE_STRING " by bitman@bitmania.de\n", stdout);
	for (c=1;c<=argc;c++) {
		if (argv[c] == NULL) {}
		else if (!strcmp(argv[c],"-d")) {
			c++;
			delay = atoi(argv[c]);
			printf("delaying %d microseconds between updates\n", delay);
		}
		else if (!strcmp(argv[c],"-h")) {
			usage();
			exit(0);
		}
	}

	#if DEBUG == 1
	#endif

	openXwindow(argc,argv,wmcore_master_xpm,wmcore_mask_bits,wmcore_mask_width,wmcore_mask_height);
	RedrawWindow();


	// Check how many cpu cores are present
 	if((in = fopen("/proc/stat","r")) == NULL)
 	{
	  printf("\nUnable to open file /proc/stat\n");
	  exit(1);
	} else {
	  fgets(tmp,200,in);
	  while( strstr(tmp, "cpu") ) {
	    fgets(tmp,200,in);
	    cpu_max++;
	  }
	  cpu_max--;
	  fclose(in);
	}
	printf("CPU-Cores detected: %i\n",cpu_max);


	// Calculate the height per bar, to fit in the screen
	h=44/cpu_max;


	// Init values
	for(cpu=0; cpu<cpu_max; cpu++) {
	  prev_idle[cpu]=0;
	  prev_total[cpu]=0;
	}


	// the main loop
	while (1) {
	  while(XPending(display)) {
	    XNextEvent(display,&Event);
	    switch (Event.type) {
		case Expose:
			RedrawWindow();
			break;
		case DestroyNotify:
			XCloseDisplay(display);
			exit(0);
			break;
	    }
	  }


	  // get the cpu core usage
 	  if((in = fopen("/proc/stat","r")) == NULL)
 	  {
	    printf("\nUnable to open file /proc/stat\n");
	    exit(1);
	  } else {
    	    fgets(tmp,200,in);
	    for(cpu=0; cpu < cpu_max; cpu++) {
    	      fgets(tmp,200,in);	// read next line
	      token = strtok(tmp," ");
	      curr_user[cpu]  = atoi(strtok(NULL," "));
	      curr_nice[cpu]  = atoi(strtok(NULL," "));
	      curr_syst[cpu]  = atoi(strtok(NULL," "));
	      curr_idle[cpu]  = atoi(strtok(NULL," "));
	      curr_total[cpu] = curr_user[cpu] + curr_nice[cpu] + curr_syst[cpu] + curr_idle[cpu];
	      diff_idle[cpu]  = curr_idle[cpu]-prev_idle[cpu];
	      diff_total[cpu] = curr_total[cpu]-prev_total[cpu];
	      if(diff_total[cpu]>0){
	        diff_usage[cpu] = (unsigned int) (1000*(diff_total[cpu]-diff_idle[cpu])/diff_total[cpu]+5)/10 ;
	      } else {
	        diff_usage[cpu] = 0;
	      }
	    }
	    fclose(in);
	  }

	  // Display total usage over all cores in upper display field
	  t=0;
	  for(cpu=0; cpu<cpu_max; cpu++) {
		t=t+diff_usage[cpu];
          }
	  t=t/cpu_max;
  	  t=((t*60)/100);
	  copyXPMArea(129,0,   61,8, 1, 2 );	// blank preveous bar
	  copyXPMArea(65,0,     t,8, 1, 2 );	// draw new bar



	  // Display the bar graphs for each core in lower display field
	  for(cpu=0; cpu<cpu_max; cpu++) {
	    // erase preveous bar
	    copyXPMArea(129,0,   61,h, 1, 18+(cpu*h));

	    // calculate the length of the bar and display
  	    t=((diff_usage[cpu]*60)/100);
	    copyXPMArea(65,0,  t,h, 1, 18+(cpu*h));

	    // Remember the total and idle CPU times for the next run.
	    prev_total[cpu]=curr_total[cpu];
	    prev_idle[cpu]=curr_idle[cpu];
	  }

	  RedrawWindow();
	  usleep(delay);
	}
	return 0;
}
