/***************************************************************************
                          wmpower.c  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003,2004,2005 by Noberasco Michele
    e-mail               : noberasco.gnu@disi.unige.it
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.              *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
                Many thanks to Filippo Panessa for his wmab...
                   it's code was the base for this program
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <X11/X.h>
#include <X11/xpm.h>

#include "open_syslog_on_stderr.h"
#include "power_management.h"
#include "dockapp.h"
#include "wmpower_master.xpm"
#include "wmpower_master_LowColor.xpm"
#include "wmpower_mask.xbm"

void ParseCMDLine (int argc, char *argv[]);
void ShowACstatus(int ac_on_line);
void ShowFanStatus(int fanstatus);
void ShowTemperature(int temp, int is_celsius);
void ShowChargeStatus(int charging);
void ShowBatteryTime(int time, int percentage, int charging, int ac_on_line);
void ShowBatteryPercentage(int percentage);
void ShowBatteryLed(int present, int percentage, int ac_on_line);

int no_meddling     = 0;   /* Should we stop managing power status?        */
int no_full_battery = 0;   /* Should we always use max power when plugged? */

int CriticalLevel   = 10;  /* Battery critical level */
int LowLevel        = 40;  /* Battery low level      */

#define CMDLINELEN 512
int WarnTime = 2;          /* When to execute the warn command */
char WarnCommand[CMDLINELEN] = ""; /* The warn command to execute      */

float BlinkRate = 3.00;    /* blinks per second      */

/* Controls beeping when you get to critical   */
/* battery level: Off by default               */
int Beep   =  0;    /* to beep or not to beep? */
int Volume = 50;    /* ring bell at 50% volume */

/* Mouse wheel */
unsigned int wheel_button_up   = 4;
unsigned int wheel_button_down = 5;

/* Monitor first battery by default */
int our_battery = 1;

/* Use a lower number of colors for the poor saps on  */
/* 8-bit displays -- common on laptops!               */
int UseLowColorPixmap = 0;

int main (int argc, char *argv[])
{
  pm_status power_status;
  XEvent event;
  int fbc_toggle=1, fbc_auto=1;
  int old_battery_charging;
  time_t polling = 0;
  struct timespec delay;     /* pause between interface updates */
  char Command[CMDLINELEN+3];
  int warned = 0;

  delay.tv_sec  = 0;
  delay.tv_nsec = 500000000;

  BlinkRate     = (BlinkRate >= 0.0) ? BlinkRate : -1.0 * BlinkRate;
	waittime      = 0;       /* /proc polling interval */
	minBrightness = -1;
	maxBrightness = -1;

  fprintf(stderr, "\nWelcome to wmpower version %s...\n", VERSION);

	cpufreq_online_governor  = NULL;
	cpufreq_offline_governor = NULL;

  /* Parse any command line arguments. */
  ParseCMDLine (argc, argv);

  /*  Check for Power Management support  */
  if (!pm_support(our_battery))
  {
    fprintf (stderr, "\nNo power management support...\n");
    return EXIT_FAILURE;
  }

  /* Create window of the program */
  if (UseLowColorPixmap) openXwindow (argc, argv, wmpower_master_LowColor, (char *) wmpower_mask_bits, wmpower_mask_width, wmpower_mask_height);
  else openXwindow (argc, argv, wmpower_master, (char *) wmpower_mask_bits, wmpower_mask_width, wmpower_mask_height);

  /* Loop until we die... */
  while (1)
  {
    /* Get current power status */
    old_battery_charging = power_status.battery_charging;
    if (!waittime) get_power_status(&power_status);
    else if ((time(NULL)-polling) >= waittime)
    {
      get_power_status(&power_status);
      polling = time(NULL);
    }

    /* Manage power features only if function is not disabled */
    if (!no_meddling)
    {
      /* Re-enable auto power mode switching whan battery status changes */
      if (old_battery_charging != power_status.battery_charging) fbc_auto = 1;

      /* Enable fast battery charge mode if on AC and batt is charging   */
      if (!no_full_battery && power_status.ac_on_line && power_status.battery_charging && fbc_auto && !fbc_toggle && !(power_status.battery_percentage == 100))
      {
        fast_battery_charge(1);
				fbc_toggle = 1;
        fbc_auto   = 1;
      }

      /* Adjust variables value when battery reaches 100% */
      if (fbc_toggle && (power_status.battery_percentage == 100))
      {
				fast_battery_charge(0);
        fbc_toggle = 0;
        fbc_auto   = 1;
      }

			/* If battery not present and fast charge mode, disable it */
			if (fbc_toggle && !(power_status.battery_present))
			{
				fast_battery_charge(0);
				fbc_toggle = 0;
				fbc_auto   = 1;
			}

      /* Set various pm features whenever applicable */
      set_pm_features();
    }

    /* Execute the warning command, if needed */
    if (WarnCommand && *WarnCommand && !power_status.ac_on_line && !warned
				&& power_status.battery_time <= WarnTime)
		{
			warned = 1;
			sprintf(Command, "%s &", WarnCommand);
			system(Command);
    }
    if (power_status.ac_on_line)
      warned = 0;

    /* Show AC status led */
    ShowACstatus(power_status.ac_on_line);

    /* Display FAN status. */
    ShowFanStatus(power_status.fan_status);

    /* Display temperature. */
    ShowTemperature(power_status.temperature, power_status.temp_is_celsius);

    /* Display charge status */
    ShowChargeStatus(power_status.battery_charging);

    /* Display the "Time Left" */
    ShowBatteryTime(power_status.battery_time, power_status.battery_percentage, power_status.battery_charging, power_status.ac_on_line);

    /* Display battery percentage */
    ShowBatteryPercentage(power_status.battery_percentage);

    /* Display battery status led */
    ShowBatteryLed(power_status.battery_present, power_status.battery_percentage, power_status.ac_on_line);

    /* Process any pending X events. */
    while (XPending (display))
    {
      XNextEvent (display, &event);
      switch (event.type)
      {
				case Expose:
					RedrawWindow ();
					continue;
				case ButtonPress:
					if (no_meddling)
					{
						fprintf(stderr, "You cannot change PM status in '-no-meddling' mode of operation\n");
						continue;
					}
					if (event.xbutton.button == wheel_button_up)
					{
						lcdBrightness_UpOneStep();
						continue;
					}
					if (event.xbutton.button == wheel_button_down)
					{
						lcdBrightness_DownOneStep();
						continue;
					}
					fbc_toggle = !get_fast_battery_charge_mode();
					fbc_auto   = 0;
          fast_battery_charge(fbc_toggle);
					continue;
				case ButtonRelease:
					continue;
      }
    }

    /* Redraw and wait for next update */
    RedrawWindow ();
    nanosleep(&delay, NULL);
  }
}


/* Show AC status led */
void ShowACstatus(int ac_on_line)
{
  /* Check AC status. */
  if (ac_on_line)
    /* AC on-line. I.e. we are "plugged-in". */
    copyXPMArea (68, 6, 12, 7, 31, 35);
  else
    /* AC off-line. I.e. we are using battery. */
    copyXPMArea (68, 20, 12, 7, 31, 35);
}



/* Display fan status */
void ShowFanStatus(int fan_status)
{
  if (fan_status == PM_Error)
  {
    /* Plot the red - Symbol                */
    copyXPMArea (165, 60, 6, 7, 23, 50);
    return;
  }

  /* Plot fan status: 0 not active, 1 running */
  copyXPMArea (fan_status * 6 + 4, 69, 6, 7, 23, 50);
}



/* Display charge status */
void ShowChargeStatus(int charging)
{
  /* Paste up the default charge status and time */
  copyXPMArea ( 83, 93, 41, 9, 15, 7);
  copyXPMArea (104,  6,  5, 7,  6, 7);

  /* Check to see if we are charging. */
  if (charging)
    /* Battery Status: Charging.      */
    copyXPMArea (82, 68, 7, 9, 6, 7);
  else
    /* Battery Status: NOT Charging.  */
    copyXPMArea (88, 68, 7, 9, 6, 7);
}



/* Display battery status led */
void ShowBatteryLed(int present, int percentage, int ac_on_line)
{
  static int Toggle;  /* Switch for battery led blinking */

  if (!present)
  {
    copyXPMArea (95, 19, 16, 10, 43, 34);
    return;
  }

  /* Battery Status: Critical.   */
  /* Blink the red led on/off... */
  if (percentage <= CriticalLevel && !ac_on_line)
  {
    if (Toggle || (BlinkRate == 0.0))
    {
      if (Beep && !ac_on_line) XBell (display, Volume);
      Toggle = 0;
      copyXPMArea (4, 105, 16, 10, 43, 34);
    }
    else
    {
      Toggle = 1;
      copyXPMArea (58, 105, 16, 10, 43, 34);
    }
    return;
  }

  /* Battery Status: Low. */
  /* Fixed yellow led     */
  if (percentage <= LowLevel)
  {
    copyXPMArea (22, 105, 16, 10, 43, 34);
    return;
  }

  /* Battery Status: Normal. */
  /* Fixed blue led          */
  copyXPMArea (40, 105, 16, 10, 43, 34);
}



/* Display Temperature */
void ShowTemperature(int temp, int temp_is_celsius)
{
  /* PM_Error getting temperature value */
  /* or value out of range              */
  if ( (temp < 0) || (temp > 99) )
  {
    /* Plot PM_Error message */
    copyXPMArea (165, 60, 6, 7, 33, 50);
    copyXPMArea (165, 60, 6, 7, 39, 50);
    copyXPMArea (135, 60, 6, 7, 45, 50);
    copyXPMArea ( 68, 69, 6, 7, 51, 50);
    return;
  }

  /* Plot temperature */
  if (temp < 10) copyXPMArea ((temp) * 6 + 4, 69, 6, 7, 39, 50);
  else
  {
    copyXPMArea ((temp / 10) * 6 + 4, 69, 6, 7, 33, 50);
    copyXPMArea ((temp % 10) * 6 + 4, 69, 6, 7, 39, 50);
  }

  /* Plot the ° Symbol */
  copyXPMArea (135, 60, 6, 7, 45, 50);

  /* Plot the C Symbol */
  if (temp_is_celsius) copyXPMArea (68, 69, 6, 7, 51, 50);
}



/* Display the "Time Left". This time means:                   */
/* If not charging: Time left before battery drains to 0%      */
/* If charging:     Time left before battery gets to maximum   */
void ShowBatteryTime(int time, int percentage, int charging, int ac_on_line)
{
  int battery_time=time;
  int hour, min;

  if ( (battery_time < -1) || ((battery_time == 0)&&(percentage == 0)) || (ac_on_line&&(percentage == 100)) )
  {
    /* In case battery is fully charged and we are on AC power,
		 * or there is some problem reading battery time
		 * we display a "null" indicator (--:--)
		 */
    copyXPMArea (83, 106, 41, 9, 15, 7);
    return;
  }

  /* Now we are sure battery time is consistent */
  if (percentage == 100) battery_time = 0;
  hour = battery_time / 60;
  min  = battery_time % 60;

	/* show '-' sign when charging, '+' otherwise */
	if (charging)
		copyXPMArea (83, 106, 41, 9, 15, 7);
	else
		copyXPMArea (83, 93,  41, 9, 15, 7);

  /* Show 10's (hour) */
  copyXPMArea ((hour / 10) * 7 + 5, 93, 7, 9, 21, 7);

  /* Show 1's (hour)  */
  copyXPMArea ((hour % 10) * 7 + 5, 93, 7, 9, 29, 7);

  /* colon            */
  copyXPMArea (76, 93, 2, 9, 38, 7);

  /* Show 10's (min)  */
  copyXPMArea ((min / 10) * 7 + 5, 93, 7, 9, 42, 7);

  /* Show 1's (min)   */
  copyXPMArea ((min % 10) * 7 + 5, 93, 7, 9, 50, 7);
}



/* Display battery percentage */
void ShowBatteryPercentage(int percentage)
{
  int k;

  copyXPMArea (76, 81, 19, 7, 7, 34);	/* Show Default % */
  copyXPMArea (66, 31, 49, 9, 7, 21);	/* Show Default Meter */

  if (percentage == 100)
  {
    /* If 100%, show 100% */
    copyXPMArea (15, 81, 1, 7,  7, 34);
    copyXPMArea ( 5, 81, 6, 7,  9, 34);
    copyXPMArea ( 5, 81, 6, 7, 15, 34);
    copyXPMArea (64, 81, 7, 7, 21, 34);	/* Show '%' */

    /* Show rainbow battery bar             */
    copyXPMArea (66, 52, 49, 9, 7, 21);

    return;
  }
  /* Show 10's */
  if (percentage >= 10)	copyXPMArea ((percentage / 10) * 6 + 4, 81, 6, 7, 9, 34);

  /* Show 1's */
  copyXPMArea ((percentage % 10) * 6 + 4, 81, 6, 7, 15, 34);

  /* Show '%' */
  copyXPMArea (64, 81, 7, 7, 21, 34);

  /* Show Meter */
  k = percentage * 49 / 100;

  /* Show rainbow battery bar */
  copyXPMArea (66, 52, k, 9, 7, 21);
  if (k % 2) copyXPMArea (66 + k - 1, 52, 1, 9, 7 + k - 1, 21);
  else copyXPMArea (66 + k, 52, 1, 9, 7 + k, 21);
}



/* Show message about usage */
void message(void)
{
  printf("\nwmpower is a tool for checking and setting power management status for");
  printf("\nlaptop computers. Right now is supports both APM and APCI enabled");
  printf("\nkernels, plus special support for Toshiba and Compal hardware.");
  printf("\n\nUsage: wmpower [options]\n");
  printf("\n\nOptions:\n");
  printf("\t-no-meddling\t\tDon't manage power status, just show info.\n");
  printf("\t-no-full-battery\tDon't wait for 100%% battery before going back\n");
  printf("\t\t\t\tto full power.\n");
  printf("\t-no-cpufreq\t\tDon't scale CPU frequency according to power status.\n");
  printf("\t-no-noflushd\t\tDisable use of \"noflushd\" daemon:\n");
  printf("\t\t\t\tnoflushd is a tool for managing spin-down\n");
  printf("\t\t\t\tof hard disks after a certain amount of time\n");
  printf("\t\t\t\tsee <http://noflushd.sourceforge.net> for details.\n");
  printf("\t-no-toshiba\t\tDisable direct access to toshiba hardware,\n");
  printf("\t\t\t\tuse only generic ACPI/APM calls instead.\n");
  printf("\t\t\t\tThis is recommended on newer toshibas.\n");
  printf("\t-battery <num>\t\tMonitor your nth battery instead of first one.\n");
  printf("\t-display <display>\tUse alternate display.\n");
  printf("\t-geometry <geometry>\twmpower window geometry.\n");
  printf("\t-l\t\t\tUse a low-color pixmap.\n");
  printf("\t-L <LowLevel>\t\tDefine level at which yellow LED turns on.\n");
  printf("\t-C <CriticalLevel>\tDefine level at which red LED turns on.\n");
  printf("\t-B <Volume>\t\tBeep at Critical Level (-100%% to 100%%).\n");
  printf("\t-w <command>\t\tWarn command to run when remaining time is low.\n");
  printf("\t-W <minutes>\t\tMinutes of remaining time when to run warn command.\n");
  printf("\t-u <seconds>\t\tSet wmpower polling interval.\n");
  printf("\t-m <brightness>\t\tUse this LCD brightness value while running on battery power.\n");
  printf("\t-M <brightness>\t\tUse this LCD brightness value while running on AC power.\n");
  printf("\t-g <governor>\t\tUse this CPUFreq scaling governor while running on battery power.\n");
  printf("\t-G <governor>\t\tUse this CPUFreq scaling governor while running on AC power.\n");
  printf("\t-s\t\t\tMake wmpower log to syslog instead of standard error.\n");
  printf("\t-h\t\t\tDisplay this help screen.\n");
  printf("\nClicking on program window at run-time overrides any option,");
  printf("\nthus switching between low-power and full-power modes.");
  printf("\nYou can use the mouse wheel to adjust your lcd brightness.\n\n");

  exit(EXIT_FAILURE);
}


/* Parse command line arguments */
void ParseCMDLine (int argc, char *argv[])
{
  char *cmdline;
  int i;

  for (i = 1; i < argc; i++)
  {
    cmdline = argv[i];
    if (cmdline[0] == '-')
    {
      switch (cmdline[1])
      {
				case 'b':
					if (!strcmp(cmdline, "-battery"))
					{
						if (argc == i+1) message();
						our_battery = atoi(argv[++i]);
						if (our_battery < 1) message();
					}
					else message();
					break;
				case 'd':
					++i;
					break;
				case 'C':
					if (cmdline[2] != '\0') message();
					if (argc == i+1) message();
					CriticalLevel = atoi (argv[++i]);
					break;
				case 'g':
					if ( !strcmp(cmdline, "-geometry"))
					{
						extern char *Geometry;
						if ( argc == i+1 ) message();
						Geometry = argv[++i];
						break;
					}
					if (cmdline[2] != '\0') message();
					cpufreq_offline_governor = argv[++i];
					break;
				case 'G':
					if (cmdline[2] != '\0') message();
					cpufreq_online_governor = argv[++i];
					break;
				case 'L':
					if (cmdline[2] != '\0') message();
					if (argc == i+1) message();
					LowLevel = atoi (argv[++i]);
					break;
				case 'l':
					if (cmdline[2] != '\0') message();
					UseLowColorPixmap = 1;
					break;
				case 'u':
					if (cmdline[2] != '\0') message();
					if (argc == i+1) message();
					waittime = atoi (argv[++i]);
					if (waittime <= 0) message();
					fprintf(stderr, "Polling time: %d second%s.\n", waittime, (waittime == 1)? "" : "s");
					break;
				case 'B':
					if (cmdline[2] != '\0') message();
					if (argc == i+1) message();
					Beep = 1;
					Volume = atoi (argv[++i]);
					break;
				case 'm':
					if (cmdline[2] != '\0') message();
					if (argc == i+1) message();
					minBrightness = atoi (argv[++i]);
					break;
				case 'M':
					if (cmdline[2] != '\0') message();
					if (argc == i+1) message();
					maxBrightness = atoi (argv[++i]);
					break;
				case 'w':
					if (cmdline[2] != '\0') message();
					if (argc == i+1) message();
					strncpy(WarnCommand, argv[++i], CMDLINELEN-1);
					break;
				case 'W':
					if (cmdline[2] != '\0') message();
					if (argc == i+1) message();
					WarnTime = atoi (argv[++i]);
					break;
				case 's':
					if (cmdline[2] != '\0') message();
					fprintf(stderr, "Switching to syslog logging...\n");
					open_syslog_on_stderr();
					break;
				case 'n':
					if (!strcmp(cmdline, "-no-meddling"))     {no_meddling = 1;             break;}
					if (!strcmp(cmdline, "-no-full-battery")) {no_full_battery = 1;         break;}
					if (!strcmp(cmdline, "-no-noflushd"))     {set_noflushd_use(0);         break;}
					if (!strcmp(cmdline, "-no-toshiba"))      {set_toshiba_hardware_use(0); break;}
					if (!strcmp(cmdline, "-no-cpufreq"))      {set_cpufreq_use(0);          break;}
				default:
					message();
      }
    }
		else message();
  }
}
