/***************************************************************************
                      power_management.c  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003-2005 by Noberasco Michele
    e-mail               : 2001s098@educ.disi.unige.it
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
 *   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.              *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lib_utils.h"
#include "power_management.h"
#include "libacpi.h"
#include "libapm.h"
#include "toshiba_lib.h"
#include "dell_lib.h"
#include "compal_lib.h"
#include "cpufreq.h"

/* what kind of machine is running us? */
#define UNKNOWN 0
#define TOSHIBA 1
#define DELL    2
#define COMPAL  3
int machine = UNKNOWN;

int fast_charge_mode=0;
int use_toshiba_hardware=1;
int pm_type=PM_Error;
int ac_on_line;
int battery_percentage;
int battery_present;
int use_noflushd=1;
int use_cpufreq=1;

/* Battery to monitor */
int Battery;

int get_fan_status();
void get_temperature(int *temperature, int *temp_is_celsius);
int fast_battery_charge(int toggle);
int calculate_battery_time(int battery_percentage, int ac_on_line);
kernel_versions Get_Kernel_version(void);


int pm_support(int use_this_battery)
{
  Battery = use_this_battery;

  if (!use_noflushd) fprintf(stderr, "use of noflushd is disabled\n");
	if (!use_cpufreq)  fprintf(stderr, "CPU frequency scaling disabled\n");

	/* What kernel version are we running in? */
	kernel_version = Get_Kernel_version();

  /* check for specific hardware */
  while (1)
  {
    /* is this a compal laptop? */
    if (machine_is_compal())
    {
      machine = COMPAL;
      fprintf(stderr, "detected Compal laptop, %s\n", compal_model);
      break;
    }
    /* is this a dell laptop? */
    if (machine_is_dell())
    {
      machine = DELL;
      fprintf(stderr, "detected DELL laptop\n");
      break;
    }
    /* Is this a Toshiba Laptop? */
    if (machine_is_toshiba(&use_toshiba_hardware))
    {
      machine = TOSHIBA;
      fprintf(stderr, "detected TOSHIBA laptop, %s\n", toshiba_model);
      if (!use_toshiba_hardware) fprintf(stderr, "direct access to TOSHIBA hardware disabled\n");
      break;
    }
    break;
  }

	/* Does this system support CPU frequency scaling? */
	if (use_cpufreq)
	{
		use_cpufreq = check_cpufreq();
		if (use_cpufreq) fprintf(stderr, "CPU frequency scaling available\n");
		else             fprintf(stderr, "CPU frequency scaling NOT available\n");

		/* Set default governors if we were supplied none */
		if (!cpufreq_offline_governor)
			cpufreq_offline_governor = CPUFREQ_GOV_ONDEMAND;

		if (!cpufreq_online_governor)
			cpufreq_online_governor = CPUFREQ_GOV_PERFORMANCE;
	}

  /* Is this an acpi system? */
  if (check_acpi())
  {
    pm_type= PM_ACPI;
    fprintf(stderr, "Detected ACPI subsystem\n");
    return 1;
  }

  /* Is this an APM system?  */
  if (apm_exists())
  {
    pm_type= PM_APM;
    fprintf(stderr, "detected APM subsystem\n");
    if (Battery != 1)
    {
      fprintf(stderr, "You must use ACPI to monitor any battery\n");
      fprintf(stderr, "other than the first one. Cannot continue.\n");
      exit(1);
    }
    return 1;
  }

  fprintf(stderr, "No power management subsystem detected\n");
  return 0;
}

void get_power_status(pm_status *power_status)
{
  /* Is this an ACPI system? */
  if (pm_type == PM_ACPI)
  {
    static ACPIinfo  acpiinfo;
    static ACADstate acadstate;
    static ACPIstate acpistate;

    read_acpi_info(&acpiinfo, Battery - 1);
    read_acpi_state(&acpistate, &acpiinfo, Battery - 1);
    read_acad_state(&acadstate);

    /* Check if we are on ac power */
    ac_on_line = power_status->ac_on_line = acadstate.state;

    /* Check to see if we are charging. */
    if (acpistate.state == CHARGING) power_status->battery_charging=1;
    else power_status->battery_charging=0;

    /* Check battery time and percentage */
    power_status->battery_time = acpistate.rtime;
    power_status->battery_percentage = acpistate.percentage;
    if (power_status->battery_percentage > 100) power_status->battery_percentage = 100;
    battery_percentage = power_status->battery_percentage;

    /* Check if battery is plugged in */
    battery_present = power_status->battery_present = acpistate.present;

    /* Get temperature and fan status */
    power_status->fan_status=get_fan_status();
    get_temperature(&(power_status->temperature), &(power_status->temp_is_celsius));

    if (fast_charge_mode && (power_status->battery_percentage == 100)) fast_battery_charge(0);

		/* Let's see wether we failed to get battery time */
		if (battery_present && (battery_percentage < 100) && (power_status->battery_time <= 0))
		{
			/* OK, we failed, we calculate the value ourselves */
			power_status->battery_time = calculate_battery_time(battery_percentage, ac_on_line);
		}

    return;
  }

  /* Is this an APM system? */
  if (pm_type == PM_APM)
  {
    struct_apm_data apm_data;

    if (!apm_read(&apm_data))
    {
      fprintf(stderr, "Could not read APM info!\n");
      return;

    }
    ac_on_line = power_status->ac_on_line = apm_data.ac_line_status;
    if ( apm_data.battery_percentage == -1)
    {
      battery_present = power_status->battery_present = 0;
      battery_percentage = power_status->battery_percentage = 0;
    }
    else
    {
      battery_present = power_status->battery_present = 1;
      power_status->battery_percentage = apm_data.battery_percentage;
      if (power_status->battery_percentage > 100) power_status->battery_percentage = 100;
      battery_percentage = power_status->battery_percentage;
    }
    if ( (int)(apm_data.battery_status) == 3)
      power_status->battery_charging=1;
    else power_status->battery_charging=0;
    power_status->battery_time = (apm_data.using_minutes) ? apm_data.battery_time : apm_data.battery_time / 60;
    power_status->fan_status=get_fan_status();
    get_temperature(&(power_status->temperature), &(power_status->temp_is_celsius));

    if (fast_charge_mode && (power_status->battery_percentage == 100)) fast_battery_charge(0);

    return;
  }
}

int get_fan_status(void)
{
  if (machine == COMPAL)  return compal_get_fan_status();
  if (machine == DELL)    return dell_get_fan_status();
  if (machine == TOSHIBA) return toshiba_get_fan_status(use_toshiba_hardware);
	if (pm_type == PM_ACPI) return acpi_get_fan_status();

  return PM_Error;
}


void get_temperature(int *temperature, int *temp_is_celsius)
{
  /* for Compal laptops... */
  if (machine == COMPAL)
  {
    int result = compal_get_temperature();
    if (result == PM_Error)
    {
      (*temperature)     = PM_Error;
      (*temp_is_celsius) = PM_Error;
      return;
    }
    (*temperature)     = result;
    (*temp_is_celsius) = 1;
    return;
  }

  /* for Dell laptops... */
  if (machine == DELL)
  {
    int result = dell_get_temperature();
    if (result == PM_Error)
    {
      (*temperature)     = PM_Error;
      (*temp_is_celsius) = PM_Error;
      return;
    }
    (*temperature)     = result;
    (*temp_is_celsius) = 1;
    return;
  }

  /* No special hardware... */
  if (pm_type == PM_ACPI)
  {
    acpi_get_temperature(temperature, temp_is_celsius);
    return;
  }

  /* No ACPI */
  (*temperature)     = PM_Error;
  (*temp_is_celsius) = PM_Error;
}


void internal_set_pm_features(int ac_status)
{
  static int noflushd    = -1;

  if (fast_charge_mode) ac_status=0;
  if (ac_status)
  { /* we are on AC power */
    /* Stop noflushd damon (disable HD spindown) */
    if (use_noflushd && noflushd)
    {
      system("/etc/init.d/noflushd stop >/dev/null 2>/dev/null");
      noflushd = 0;
    }

		/* Set CPU governor to 'performance' (or whatever our master chose when online) */
		if (use_cpufreq)
			if (!cpufreq_set_governor(cpufreq_online_governor))
				fprintf(stderr, "failed setting '%s' CPUfreq governor!\n", cpufreq_online_governor);

    if (machine == COMPAL)
    {
      /* Set LCD to maximum brightness */
			compal_set_lcd_brightness(COMPAL_LCD_MAX);
    }
    if (machine == TOSHIBA)
    {
      /* Set LCD to maximum brightness */
			toshiba_set_lcd_brightness(TOSHIBA_LCD_MAX, use_toshiba_hardware);
      /* Start fan */
      if (use_toshiba_hardware && (pm_type != PM_ACPI)) toshiba_set_fan_status(1);
      return;
    }
    if (pm_type == PM_ACPI)
    {
      /* Set generic ACPI features here... */
      return;
    }
  }
  else
  { /* we are on battery power */
    if (use_noflushd && (!noflushd || noflushd == -1))
    {
      /* Start noflushd damon (enable HD spindown) */
      system("/etc/init.d/noflushd start >/dev/null 2>/dev/null");
      noflushd = 1;
    }
		/* Set CPU governor to 'ondemand' (or whatever our master chose when offline) */
		if (use_cpufreq)
			if (!cpufreq_set_governor(cpufreq_offline_governor))
				fprintf(stderr, "failed setting '%s' CPUfreq governor!\n", cpufreq_offline_governor);

    if (machine == COMPAL)
    {
      /* Set LCD to minimum brightness */
			compal_set_lcd_brightness(COMPAL_LCD_MIN);
    }
    if (machine == TOSHIBA)
    {
      /* Set LCD to minimum brightness */
			toshiba_set_lcd_brightness(TOSHIBA_LCD_MIN, use_toshiba_hardware);
      /* Stop fan */
      if (use_toshiba_hardware && (pm_type != PM_ACPI)) toshiba_set_fan_status(0);
      return;
    }
    if (pm_type == PM_ACPI)
    {
      /* Set generic ACPI features here... */
      return;
    }
  }

}


void set_pm_features(void)
{
  static int old_value = -1;

  if (ac_on_line != old_value)
  {
    internal_set_pm_features(ac_on_line);
    old_value = ac_on_line;
  }

}


int fast_battery_charge(int toggle)
{

  if (toggle && !battery_present)
  {
    fprintf(stderr, "Fast charge mode without a battery? I think not ;-)\n");
    return 0;
  }
  if (toggle && !ac_on_line)
  {
    fprintf(stderr, "Fast charge mode while on battery? I think not ;-)\n");
    return 0;
  }
  if (toggle && (battery_percentage == 100))
  {
    fprintf(stderr, "Battery is already at maximum capacity: fast charging not enabled\n");
    return 0;
  }

  if (!toggle && fast_charge_mode)
  {
    if (battery_percentage == 100) fprintf(stderr, "battery at maximum capacity: ");
    fprintf(stderr, "disabling fast charge mode\n");
    fast_charge_mode=0;
    internal_set_pm_features(ac_on_line);
    return 0;
  }

  if (toggle)
  {
    fprintf(stderr, "enabling fast charge mode\n");
    internal_set_pm_features(0);
    fast_charge_mode=1;
    return 1;
  }

  return fast_charge_mode;
}


int get_fast_battery_charge_mode(void)
{
	return fast_charge_mode;
}


void set_noflushd_use(int toggle)
{
  use_noflushd = toggle;
}


void set_toshiba_hardware_use(int toggle)
{
  use_toshiba_hardware = toggle;
}


void set_cpufreq_use(int toggle)
{
  use_cpufreq = toggle;
}


void lcdBrightness_UpOneStep()
{
  if (machine == COMPAL)  Compal_lcdBrightness_UpOneStep();
  if (machine == TOSHIBA) Toshiba_lcdBrightness_UpOneStep(use_toshiba_hardware);
}


void lcdBrightness_DownOneStep()
{
  if (machine == COMPAL)  Compal_lcdBrightness_DownOneStep();
  if (machine == TOSHIBA) Toshiba_lcdBrightness_DownOneStep(use_toshiba_hardware);
}


kernel_versions Get_Kernel_version(void)
{
	FILE *fp = fopen("/proc/version", "r");
	char buf[512];

	if (!fp) return IS_OTHER;
	if (!fgets(buf, 512, fp))
	{
		fclose(fp);
		return IS_OTHER;
	}
	fclose(fp);

	if (strstr(buf, "2.6.")) return IS_2_6;
	return IS_OTHER;
}

/* Calculate estimated battery time
 * note that the result this function returns
 * is fairly inaccurate, and will be used only
 * if we fail to get the value trough ACPI, APM
 * or whatever...
 */
int calculate_battery_time(int battery_percentage, int ac_on_line)
{
	static int    first_percentage =  0;
	static int    first_on_line    = -1;
	static time_t first_time       =  0;
	static time_t prev_time        =  0;
	time_t        curr_time        = time(NULL);
	int           battery_time;
	int           elapsed_time;

	/* First time we are run, we cannot return anything meaningful;
	 * also, we have to reinitialize ourselves when our host switches
	 * to/from battery power. Plus, it is best to reinitialize our stuff
	 * when the time elapsed since our last update is significantly
	 * higher than our polling interval, because this likely means that
	 * the system has been suspended (we allow it some play -60 seconds-
	 * because -who knows- the system might be on really heavy load)...
	 */
	if (!first_time || (first_on_line != ac_on_line) || (first_time && (curr_time-prev_time>(waittime + 60))))
	{
		if (!first_time)
		{
			fprintf(stderr, "could not read battery time!\n");
			fprintf(stderr, "I will calculate it directly, but this is FAR from accurate..\n");
		}
		else fprintf(stderr, "re-initializing battery time!\n");

		first_percentage = battery_percentage;
		first_on_line    = ac_on_line;
		first_time       = curr_time;
		prev_time        = curr_time;
		return 0;
	}

	prev_time = curr_time;

	/* Seconds passed since we were initialized */
	elapsed_time = curr_time - first_time;

	if (!ac_on_line)
	{ /* We calculate time left until battery drains out */

		/* How much battery was drained in this time? */
		int drained_battery = first_percentage - battery_percentage;

		if (!drained_battery) return 0;

		/* elapsed_time : drained_battery = ? time : remaining battery */
		battery_time = battery_percentage * elapsed_time / drained_battery;
	}
	else
	{ /* We calculate time left until battery is fully recharged */
		
		/* How much battery was gained in this time? */
		int gained_battery = battery_percentage - first_percentage;

		/* How much battery still to be charged? */
		int remaining_battery = 100 - battery_percentage;

		if (!gained_battery   ) return 0;
		if (!remaining_battery) return 0;

		/* elapsed_time : gained_battery = ? time : battery still to be charged */
		battery_time = remaining_battery * elapsed_time / gained_battery;
	}

	/* the value is in seconds, but we need it in minutes */
	battery_time = battery_time / 60;

	/* Don't allow it to be higher than 99 hours and 59 minutes,
	 * otherwise our GUI will not be able to handle it
	 */
	if (battery_time > 5999) battery_time = 0;

	return battery_time;
}
