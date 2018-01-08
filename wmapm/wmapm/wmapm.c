/*
 *	wmapm-3.1
 *
 *		A WindowMaker dockable application that allows laptop users
 *		to graphically monitor the status of their power source.
 *		(I.e. whether or not AC or battery is in use as well as
 *		 how long it will take to drain or charge the battery.)
 *
 *              Originally written (and copyrighted under GPL) by
 * 		Chris D. Faulhaber <jedgar@fxp.org>. Version 3.0
 * 		is an extensively modified version of version 2.0
 *		by Michael G. Henderson <mghenderson@lanl.gov>.
 *
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	the Free Software Foundation; either version 2, or (at your option)
 * 	any later version.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program (see the file COPYING); if not, write to the
 * 	Free Software Foundation, Inc.,
 * 	59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * 	Portions of code derived from:
 *   		apm/apmd/libapm : (c) 1996 Rickard E. Faith (r.faith@ieee.org)
 *   		wmmon           : (c) 1998 Martijn Pieterse (pieterse@xs4all.nl) and
 *                              Antoine Nulle (warp@xs4all.nl)
 *
 * 	Thanx to Timecop <timecop@linuxwarez.com> for pointing out/helping to
 *	Toggle fix the meter mismatch.
 *
 */



/*
 * 	Changes:
 *
 *  3.1		-Released: June 1, 1999.
 *			+ Added support for time left on FreeBSD 3.x/4.x
 *			  (Chris D. Faulhaber <jedgar@fxp.org>)
 *
 *
 *	3.01	-Released: January 3, 1999.
 *
 * 			+ Added a LowColor Pixmap for the poor saps using 8-bit displays
 * 			  on laptops. There are a *lot* of laptops out there that are only
 *			  8-bit. Use the "-l" command-line option to invoke.
 *
 *
 *	3.0	-Released: December 15, 1998.
 * 		 A Major overhaul performed. Changes include;
 *
 *			+ Added buttons to place laptop into "Suspend" (button labeled `Z')
 *			  or "Standby" (button labeled `S') mode. Buttons are separated
 *			  by status LEDs to minimize accidentally clicking on the wrong
 *			  one. I used `Z' for suspend because its like the laptop is
 *			  catching some Zs (i.e. going to sleep).
 *
 *			+ Replaced the 3 rectangular red/yellow/green status indicators
 *			  with 3 small round LEDs and moved them to a viewport at the
 *			  bottom between the two buttons. This array of LEDs could in future
 *			  be moved to a single LED in the main viewport to make room for
 *			  other things at this location (perhaps more buttons if apm supports
 *			  more things like truning off LCD, etc).
 *
 *			+ Created user-definable LowLevel and CriticalLevel thresholds. Yellow LED
 *			  comes on when Battery Percentage hits the LowLevel threshold. Red comes on
 *			  when you reach CriticalLevel threshold.
 *
 *			+ Made red status LED blink for extra noticability. User can define blink rate.
 *			  A BlinkRate of 0 turns off blinking.
 *
 *			+ Moved all of the other indicators into a single viewport above the
 *			  buttons and status LEDs.
 *
 *			+ Changed the red-dark-green colorbar to a banded blue LED bar that is tipped
 *			  with a color indicating capacity level. The tip color goes through
 *			  green-yellow-orange-red. A series of single-pixel dots is always present
 *			  below the bar to indicate its range. This is needed now, because
 *			  the bar is free-floating in the viewport. The single-pixel dots can be
 *			  seen easily on an LCD - the type of monitor wmapm is likely to be used.
 *
 *			+ Changed the `CHARGING' indicator with a single red `C' indicator at the
 *			  upper left of the APP.
 *
 *			+ Changed percentage indicator so that it can display 100%. (Used to only go
 *			  up to 99% max).
 *
 *			+ Changed time indicator to have a +/- sign depending on whether you are
 *			  charging up or draining down. (+ means you have that much battery life
 *			  left before its discharged. - means you have that much time to wait until
 *			  the battery is fully charged.)
 *
 *			+ Fixed a problem with very large "TimeLeft" values. If the time is greater
 *			  than the maximum time displayable 99 hours and 59 minutes, a ---:-- is
 *			  listed instead.  Since the time is based on measured charge/discharge rates,
 *			  when the battery is doing neither, the time is essentially infinite. On my
 *			  (M Henderson's) laptop, the time left indicated 32766 when this happened.
 *   			  FreeBSD systems should also show a ---:-- indicator. Dont have FreeBSD though
 *			  so I couldnt test it....
 *
 *			+ Changed Makefile to suid the apm program. This is needed to allow users to
 *			  invoke the standby and suspend capabilities in apm.
 *
 *			+ Sped up the loop to catch button press and expose events. But the querying of
 *			  /proc/apm is still done about once a second...
 *
 *			+ Added alert feature. User can use command line option -A <T1 T2> to turn on alerts
 *			  via wall. T1 and T2 are the time in seconds between updates for Low and Critical
 *			  status. By default the alerts are turned off.
 *
 * 			+ Various sundry code cleanups.
 *
 *
 * 	2.0	- Added FreeBSD support.
 *
 * 	1.3	- Fixed an annoying little problem with the the meter
 *		  not properly lowering as the battery is depleted.
 *		  Also did some code cleanup, enhanced the Makefile which
 *		  now includes 'make install'.
 *		  Released 980826
 *
 * 	1.2	- Fixed bug that showed 100% battery capacity
 *		  as 90% (I never noticed since my battery seems
 *		  to max out at 98%).
 *		  Thanx to Brice Ruth <bruth@ltic.com> for pointing out/helping fix the
 *		  100% bug (err...feature).
 *		  Released 980731
 *
 * 	1.1	- Removed libapm dependency; tweaked some code.
 *		  Released 980729
 *
 * 	1.0	- Initial release version.
 *		  Released 980725
 *
 */


#ifdef SunOS
#include <unistd.h>
#include <fcntl.h>
#include <stropts.h>
#include <sys/battery.h>
#endif


#ifdef FreeBSD
#include <err.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <machine/apm_bios.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "wmapm.h"
#include <libdockapp/wmgeneral.h>
#include "wmapm_master.xpm"
#include "wmapm_master_LowColor.xpm"
#include "wmapm_mask.xbm"



#ifdef Linux
int apm_read(struct my_apm_info *i);
#else
# ifdef FreeBSD
int apm_read(apm_info_t temp_info);
# endif
#endif
int apm_exists();
void ParseCMDLine(int argc, char *argv[]);
void pressEvent(XButtonEvent *xev);




int	CriticalLevel 		= 10;
int	LowLevel      		= 40;
float   BlinkRate     		= 3.0;	 /* blinks per second */
float   UpdateRate    		= 0.8;   /* Number of updates per second */
int	Beep 	      		= 0;	 /* Controls beeping when you get to CriticalLevel: Off by default */
int	Volume	      		= 50;	 /* ring bell at 50% volume */
int	Alert	      		= 0;  	 /* Controls whether alert is sent to all users via wall: Off by default  */
int	UseLowColorPixmap 	= 0; 	 /* Use a lower number of colors for the poor saps on 8-bit displays -- common
					    on laptops! */
float  	LAlertRate     		= 300.0; /* send alert every 5 minutes when Low */
float  	CAlertRate     		= 120.0; /* send alert every 2 minutes when Critical */








/*
 *   main
 */
int main(int argc, char *argv[]) {


    struct my_apm_info 	my_cur_info;
    int                	time_left,
			hour_left,
			min_left,
			digit;
#ifdef FreeBSD
    struct apm_info 	temp_info;
#endif
    XEvent		event;
    int			m, mMax, n, nMax, k, Toggle;
    long int		r, rMax, s, sMax;
    FILE		*fp;







    BlinkRate = 3.0;
    UpdateRate = 1.0/1.25;




    /*
     *  Parse any command line arguments.
     */
    ParseCMDLine(argc, argv);

    BlinkRate = (BlinkRate >= 0.0) ? BlinkRate : -1.0*BlinkRate;
    UpdateRate = (UpdateRate >= 0.0) ? UpdateRate : -1.0*UpdateRate;


    nMax = (int)( 1.0e6/(2.0*UpdateRate*DELAY)  );
    mMax = (BlinkRate > 0.0) ? (int)( 1.0e6/(2.0*BlinkRate*DELAY)  ) : nMax;
    rMax = (int)( LAlertRate*1.0e6/(2.0*DELAY)  );
    sMax = (int)( CAlertRate*1.0e6/(2.0*DELAY)  );









    /*
     *  Check for APM support
     */
    if (!apm_exists()) {
#ifdef Linux
        fprintf(stderr, "No APM support in kernel\n");
#else
        fprintf(stderr, "Unable to access APM info\n");
#endif
        exit(1);
    }




    if (UseLowColorPixmap)
        openXwindow(argc, argv, wmapm_master_LowColor, wmapm_mask_bits, wmapm_mask_width, wmapm_mask_height);
    else
        openXwindow(argc, argv, wmapm_master, wmapm_mask_bits, wmapm_mask_width, wmapm_mask_height);




    /*
     *     Loop until we die...
     */
    n = 32000;
    m = 32000;
    r = rMax+1;
    s = sMax+1;
    while(1) {


	/*
	 *  Only process apm info only every nMax cycles of this
	 *  loop. We run it faster to catch the xevents like button
	 *  presses and expose events, etc...
	 *
	 *  DELAY is set at 0.00625 seconds, so process apm info
	 *  every 1.25 seconds...
	 *
	 */
	if (n>nMax){

	    n = 0;

#if defined(Linux) || defined(SunOS)
	    if (apm_read(&my_cur_info)) {
#else
# ifdef FreeBSD
            if (apm_read(&temp_info)) {
# endif
#endif
		fprintf(stderr, "Cannot read APM information: %i\n");
		exit(1);
	    }


#ifdef FreeBSD     /* Convert status's */
	    my_cur_info.ac_line_status     = (int)temp_info.ai_acline;
	    my_cur_info.battery_status     = (int)temp_info.ai_batt_stat;
	    my_cur_info.battery_percentage = (int)temp_info.ai_batt_life;
	    my_cur_info.battery_time       = (int)temp_info.ai_batt_time;
#endif






	    /*
	     *   Check AC status.
	     */
	    switch (my_cur_info.ac_line_status) {

	        case 1:
			/*
			 *   AC on-line. I.e. we are "plugged-in".
			 */
			copyXPMArea(68, 6, 26, 7, 31, 35);
	       		break;
	        default:
			/*
			 *   AC off-line. I.e. we are using battery.
			 */
			copyXPMArea(68, 20, 26, 7, 31, 35);

	    }




	    /*
	     *    Paste up the default charge status and time
	     */
	    copyXPMArea(104,  6,  5, 7, 6, 7);
	    copyXPMArea(83, 93, 41, 9, 15, 7);





	    /*
	     *  Check to see if we are charging.
	     */
	    if ( (int)(my_cur_info.battery_status) == 3){

	        /*
	         *   Battery Status: Charging.
	         */
	        copyXPMArea(98,  6, 5, 7,  6,  7);
	        copyXPMArea(75, 81, 1, 2, 17,  9);
	        copyXPMArea(75, 81, 1, 2, 17, 12);

	    }





	    /*
	     *   Repaint buttons.
	     */
	    copyXPMArea(42, 106, 13, 11, 5, 48);
	    copyXPMArea(57, 106, 13, 11, 46, 48);








	    /*
	     *  Paste up the "Time Left". This time means:
	     *
	     *         If not charging: Time left before battery drains to 0%
	     *         If charging:     Time left before battery gets to maximum
	     *
	     */
#ifdef Linux
	    if (my_cur_info.battery_time >= ((my_cur_info.using_minutes) ? 1440 : 86400) ) {
#else
# ifdef FreeBSD
	    if (my_cur_info.battery_time >= 86400) {
# endif
#endif

		/*
		 *  If battery_time is too large, it likely means that there is
		 *  no charging or discharging going on. So just display a "null"
		 *  indicator (--:--).
		 *
		 */
		copyXPMArea(83, 106, 41, 9, 15, 7);

	    } else if (my_cur_info.battery_time >= 0) {

#ifdef Linux
		time_left = (my_cur_info.using_minutes) ? my_cur_info.battery_time : my_cur_info.battery_time / 60;
#endif
#ifdef FreeBSD
		time_left = (my_cur_info.using_minutes) ? my_cur_info.battery_time / 60 : my_cur_info.battery_time / 3600;
#endif

	        hour_left = time_left / 60;
	        min_left  = time_left % 60;

	        copyXPMArea( (hour_left / 10) * 7 + 5, 93, 7, 9, 21, 7); 	/* Show 10's (hour) */
	        copyXPMArea((hour_left % 10) * 7 + 5, 93, 7, 9, 29, 7); 	/* Show 1's (hour)  */
	        copyXPMArea(76, 93, 2, 9, 38, 7);		 		/* colon  	    */
	        copyXPMArea((min_left / 10) * 7 + 5, 93, 7, 9, 42, 7); 		/* Show 10's (min)  */
	        copyXPMArea((min_left % 10) * 7 + 5, 93, 7, 9, 50, 7); 		/* Show 1's (min)   */

	    }





	    /*
	     *   Do Battery Percentage.
 	     */
	    copyXPMArea(76, 81, 19, 7, 7, 34);            		/* Show Default % */
	    copyXPMArea(66, 31, 49, 9, 7, 21);           		/* Show Default Meter */
	    if (my_cur_info.battery_percentage == 100){
	            copyXPMArea(15, 81, 1, 7,  7, 34);             	/* If 100%, show 100% */
	            copyXPMArea( 5, 81, 6, 7,  9, 34);
	            copyXPMArea( 5, 81, 6, 7, 15, 34);
	            copyXPMArea(64, 81, 7, 7, 21, 34);            	/* Show '%' */
	    	    copyXPMArea(66, 42, 49, 9, 7, 21);           	/* Show Meter */
	    } else {

		if (my_cur_info.battery_percentage >= 10)
		    copyXPMArea((my_cur_info.battery_percentage / 10) * 6 + 4, 81, 6, 7,  9, 34);  	/* Show 10's */
		copyXPMArea((my_cur_info.battery_percentage % 10) * 6 + 4, 81, 6, 7, 15, 34);		/* Show 1's */
		copyXPMArea(64, 81, 7, 7, 21, 34);							/* Show '%' */

		/*
		 *  Show Meter
		 */
		k = my_cur_info.battery_percentage * 49 / 100;
		copyXPMArea(66, 42, k, 9, 7, 21);
		if (k%2)
		    copyXPMArea(66+k-1, 52, 1, 9, 7+k-1, 21);
		else
		    copyXPMArea(66+k, 52, 1, 9, 7+k, 21);
	    }



	} else {

	    /*
	     *  Update the counter. When it hits nMax, we will
	     *  process /proc/apm information again.
	     */
	    ++n;

	}




	/*
	 *  This controls the 3 LEDs
  	 */
	if (m>mMax){

	    m = 0;


	    if (( (int)(my_cur_info.battery_status) == 2)
		    ||( (int)(my_cur_info.battery_percentage) <= CriticalLevel )){

	        /*
	         *  Battery Status: Critical.
		 *  Blink the red led on/off...
	         */
		if (Toggle||(BlinkRate == 0.0)){
		    if (Beep) XBell(display, Volume);
		    Toggle = 0;
	            copyXPMArea(95, 68, 4, 4, 24, 51);
		} else{
		    Toggle = 1;
	            copyXPMArea(75, 68, 4, 4, 24, 51);
		}
	        copyXPMArea(81, 68, 4, 4, 30, 51); 		/* turn off yellow */
	        copyXPMArea(87, 68, 4, 4, 36, 51); 		/* turn off green  */

	    } else if (( (int)(my_cur_info.battery_status) == 1)
		    ||( (int)(my_cur_info.battery_percentage) <= LowLevel )){

	        /*
	         *  Battery Status: Low.
	         */
	        copyXPMArea(75,  68, 4, 4, 24, 51); 		/* turn off red    */
	        copyXPMArea(101, 68, 4, 4, 30, 51); 		/* turn ON  yellow */
	        copyXPMArea(87,  68, 4, 4, 36, 51); 		/* turn off green  */

	    } else if (( (int)( my_cur_info.battery_status ) == 0)
		    ||( (int)(my_cur_info.battery_percentage) > LowLevel )){

	        /*
	         *  Battery Status: High.
	         */
	        copyXPMArea(75,  68, 4, 4, 24, 51);  		/* turn off red    */
	        copyXPMArea(81,  68, 4, 4, 30, 51);  		/* turn off yellow */
	        copyXPMArea(107, 68, 4, 4, 36, 51);  		/* turn ON  green  */

	    }


	} else {

	    /*
	     *  Update the counter.
	     */
	    ++m;

	}




	/*
	 *  This controls Critical Alerts
  	 */
	if (Alert){
	    if (( (int)(my_cur_info.battery_status) == 2)
		    ||( (int)(my_cur_info.battery_percentage) <= CriticalLevel )){

	        if (s>sMax){

	            s = 0;
	            fp = popen("wall", "w");
	            fprintf(fp, "Battery is critical!. Percent: %d\n", (int)(my_cur_info.battery_percentage));
	            pclose(fp);

	        } else {

	            /*
	             *  Update the counter.
	             */
	            ++s;

	        }

	    } else if (( (int)(my_cur_info.battery_status) == 1)
		    ||( (int)(my_cur_info.battery_percentage) <= LowLevel )){

	        if (r>rMax){

	            r = 0;
	            fp = popen("wall", "w");
	            fprintf(fp, "Battery is low. Percent: %d\n", (int)(my_cur_info.battery_percentage));
	            pclose(fp);

	        } else {

	            /*
	             *  Update the counter.
	             */
	            ++r;

	        }

	    }
	}



	/*
	 *   Process any pending X events.
	 */
        while(XPending(display)){
            XNextEvent(display, &event);
            switch(event.type){
                case Expose:
                        RedrawWindow();
                        break;
                case ButtonPress:
                        pressEvent(&event.xbutton);
                        break;
                case ButtonRelease:
                        break;
            }
        }






	/*
	 *  Redraw and wait for next update
	 */
	RedrawWindow();
	usleep(DELAY);


     }



}



















/*
 *  This routine handles button presses. Pressing the 'S' button
 *  invokes 'apm -S' to place the machine into standby mode. And
 *  pressing the 'Z' buton invokes 'apm -s' to place the machine
 *  into suspend mode.
 *
 *  Note: in order for users other than root to be able to run
 *        'apm -s' and 'apm -S', you need to make apm suid (i.e.
 *        run 'chmod +s /usr/bin/apm' as root).  This will allow
 *        'normal' users to execute apm with root privilages.
 *
 */
void pressEvent(XButtonEvent *xev){

   int x=xev->x;
   int y=xev->y;

   if(x>=5 && y>=48 && x<=17 && y<=58){

	/*
	 *  Standby Call.
	 *
	 *  Draw button as 'pushed'. Redraw window to show it.
	 *  Call 'apm -S' to standby. Sleep for 2 seconds so that
	 *  the button doesnt immediately redraw back to unpressed
	 *  before the 'apm -S' takes effect.
	 */
	copyXPMArea(5, 106, 13, 11, 5, 48);
	RedrawWindow();
#ifndef SunOS
  	system("apm -S");
#endif
	usleep(2000000L);

   } else if (x>=46 && y>=48 && x<=58 && y<=58){

	/*
	 *  Suspend Call.
	 *
	 *  Draw button as 'pushed'. Redraw window to show it.
	 *  Call 'apm -s' to suspend. Sleep for 2 seconds so that
	 *  the button doesnt immediately redraw back to unpressed
	 *  before the 'apm -s' takes effect.
	 */
	copyXPMArea(21, 106, 13, 11, 46, 48);
	RedrawWindow();
#ifndef SunOS
  	system("apm -s");
#endif
	usleep(2000000L);

   }

   return;

}






/*
 *	apm_exists()
 *			- Check to see if /proc/apm exists...
 *
 */
int apm_exists()
{

    if (access(APMDEV, R_OK))

	/*
	 *  Cannot find /proc/apm
	 */
	return 0;

    else

	return 1;

}




/*
 *  	apm_read()
 *			- Read in the information found in /proc/apm...
 *
 */
#ifdef Linux
int apm_read(struct my_apm_info *i){

    FILE 	*str;
    char 	 units[10];
    char 	 buffer[100];
    int  	 retcode = 0;

    /*
     *  Open /proc/apm for reading
     */
    if (!(str = fopen(APMDEV, "r")))
	return 1;



    /*
     *  Scan in the information....
     */
    fgets(buffer, sizeof(buffer) - 1, str);
    buffer[sizeof(buffer) - 1] = '\0';
    sscanf(buffer, "%s %d.%d %x %x %x %x %d%% %d %s\n",
	  (char *)i->driver_version,
	  &i->apm_version_major,
	  &i->apm_version_minor,
	  &i->apm_flags,
	  &i->ac_line_status,
	  &i->battery_status,
	  &i->battery_flags,
	  &i->battery_percentage,
	  &i->battery_time,
	  units);


    i->using_minutes = !strncmp(units, "min", 3) ? 1 : 0;



    /*
     *  Old Style
     */
    if (i->driver_version[0] == 'B') {
	strcpy((char *)i->driver_version, "pre-0.7");
	i->apm_version_major  = 0;
	i->apm_version_minor  = 0;
	i->apm_flags          = 0;
	i->ac_line_status     = 0xff;
	i->battery_status     = 0xff;
	i->battery_flags      = 0xff;
	i->battery_percentage = -1;
	i->battery_time       = -1;
	i->using_minutes      = 1;




	sscanf(buffer, "BIOS version: %d.%d", &i->apm_version_major, &i->apm_version_minor);

	fgets(buffer, sizeof(buffer) - 1, str);
	sscanf(buffer, "Flags: 0x%02x", &i->apm_flags);

	if (i->apm_flags & APM_32_BIT_SUPPORT) {

	    fgets(buffer, sizeof(buffer) - 1, str);
	    fgets(buffer, sizeof(buffer) - 1, str);

	    if (buffer[0] != 'P') {

		if (!strncmp(buffer+4, "off line", 8))     i->ac_line_status = 0;
		else if (!strncmp(buffer+4, "on line", 7)) i->ac_line_status = 1;
		else if (!strncmp(buffer+4, "on back", 7)) i->ac_line_status = 2;

		fgets(buffer, sizeof(buffer) - 1, str);
		if (!strncmp(buffer+16, "high", 4))        i->battery_status = 0;
		else if (!strncmp(buffer+16, "low", 3))    i->battery_status = 1;
		else if (!strncmp(buffer+16, "crit", 4))   i->battery_status = 2;
		else if (!strncmp(buffer+16, "charg", 5))  i->battery_status = 3;

		fgets(buffer, sizeof(buffer) - 1, str);
		if (strncmp(buffer+14, "unknown", 7))      i->battery_percentage = atoi(buffer + 14);

		if (i->apm_version_major >= 1 && i->apm_version_minor >= 1) {
		    fgets(buffer, sizeof(buffer) - 1, str);
		    sscanf(buffer, "Battery flag: 0x%02x", &i->battery_flags);
		    fgets(buffer, sizeof(buffer) - 1, str);
		    if (strncmp(buffer+14, "unknown", 7))  i->battery_time = atoi(buffer + 14);
		}


	    }
	}
    }



    /*
     *    Take care of battery percentages > 100%
     */
    if (i->battery_percentage > 100) i->battery_percentage = -1;

    fclose(str);
    return retcode;

}
#else
# ifdef FreeBSD
int apm_read(apm_info_t temp_info) {

    int fd;

    if ( (fd = open(APMDEV, O_RDWR)) < 0){

	return(1);

    } else if ( ioctl(fd, APMIO_GETINFO, temp_info) == -1 ) {

        close(fd);
        return(1);

    } else {

	close(fd);
	return(0);

    }

}
# endif
#endif

#ifdef SunOS
int apm_read(struct my_apm_info *i) {

	int fd;
	battery_t info;

	memset(i,0,sizeof(*i));
	if ((fd = open(APMDEV,O_RDONLY)) < 0) {
		perror("open");
		exit(1);
	}

 	if (ioctl(fd,BATT_STATUS,&info) < 0) return(1);

 	close(fd);

 	i->battery_percentage = info.capacity;
 	i->battery_time = info.discharge_time;
 	i->using_minutes = 0;

 	/*  convert to internal status:
 	 *
 	 *  0 = high
 	 *  1 = low
 	 *  2 = critical
 	 *  3 = charging
 	 */
 	switch(info.status) {
 	    case EMPTY:				/* Battery has (effectively) no capacity */
 		i->battery_status = 2;
 		break;
 	    case LOW_CAPACITY:			/* Battery has less than 25% capacity */
 		i->battery_status = 1;
 		break;
 	    case MED_CAPACITY:			/* Battery has less than 50% capacity */
 		i->battery_status = 1;
 		break;
 	    case HIGH_CAPACITY:			/* Battery has less than 75% capacity */
 	    case FULL_CAPACITY:			/* Battery has more than 75% capacity */
 		i->battery_status = 0;
 		break;
 	    default:
 		i->battery_status = 2;
 		break;
 	}

 	switch(info.charge) {
 	    case DISCHARGE:				/* Battery is discharging (i.e. in use) */
 		i->ac_line_status = 0;
 		break;
 	    case FULL_CHARGE:			/* Battery is charging at its fastest rate */
 	    case TRICKLE_CHARGE:		/* Battery is charging at a slower rate */
 	    default:
 		i->ac_line_status = 1;
 		break;
 	}

 	if (i->battery_percentage > 100) i->battery_percentage = 100;

 	/*  Not sure what else we can fill in right now.
 	 *  Relevant information is:
 	 *
 	 *  info.id_string = type of battery (internal, external, etc)
	 *  info.total = total capacity (mWhrs)
 	 */

 	return(0);

}
#endif



/*
 *   ParseCMDLine()
 */
void ParseCMDLine(int argc, char *argv[]) {
char *cmdline;
int  i,j;
char puke[20];

    for (i = 1; i < argc; i++) {
	cmdline = argv[i];

	if (cmdline[0] == '-') {
	    switch(cmdline[1]) {
	  	case 'd':
			++i;
			break;
	  	case 'A':
			Alert = 1;
			LAlertRate = atof(argv[++i]);
			CAlertRate = atof(argv[++i]);
			break;
	  	case 'b':
			BlinkRate = atof(argv[++i]);
			break;
	  	case 'C':
			CriticalLevel = atoi(argv[++i]);
			break;
	  	case 'L':
			LowLevel = atoi(argv[++i]);
			break;
	  	case 'l':
			UseLowColorPixmap = 1;
			break;
	  	case 'B':
			Beep = 1;
			Volume = atoi(argv[++i]);
			break;
	  	default:
			printf("\nwmapm version: %s\n", WMAPM_VERSION);
	    		printf("usage:\n");
	    		printf("\t-display <display>\tUse alternate display.\n");
	    		printf("\t-l\t\t\tUse a low-color pixmap to conserve colors on 8-bit displays.\n");
	    		printf("\t-L <LowLevel>\t\tDefine level at which yellow LED turns on.\n");
	    		printf("\t             \t\tCriticalLevel takes precedence if LowLevel<CriticalLevel.\n");
	    		printf("\t-C <CriticalLevel>\tDefine level at which red LED turns on.\n");
	    		printf("\t-b <BlinkRate>\t\tBlink rate for red LED. (0 for no blinking.)\n");
	    		printf("\t-B <Volume>\t\tBeep at Critical Level. Volume is between -100%% to 100%%.\n");
	    		printf("\t-A <T1 T2>\t\tSend messages to users terminals when Low and critical.\n");
	    		printf("\t             \t\tT1 is seconds between messages when Low. T2 is seconds between messages when Critical.\n");
	    		printf("\t-h\t\t\tDisplay help screen.\n\n");
	    		exit(1);
	    }
	}
    }


}




