/***************************************************************************
                        toshiba_lib.c  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003 by Noberasco Michele
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.              *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
    Many thanks to Jonathan A. Buzzard for his Toshiba(tm) Linux Utilities
                   I could never have done this otherwise
***************************************************************************/

#define FAN_OFF 1
#define FAN_ON 0

#define UP_ONE_STEP   -5
#define DOWN_ONE_STEP -6

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef __GLIBC__
#ifdef X86
#include <sys/perm.h>
#endif
#endif

#include "hci.h"
#include "sci.h"
#include "toshiba_lib.h"
#include "lib_utils.h"
#include "power_management.h"


void UglyHack(void);
int Get_Fan_Status(void);



int toshiba_get_fan_status(int use_toshiba_hardware)
{
	if (use_toshiba_hardware != 2)
	{
		FILE *fp = fopen("/proc/acpi/toshiba/fan", "r");
		if (fp)
		{
			int result;
			if (fscanf(fp, "%*s%d", &result) == 1)
			{
				fclose(fp);
				return result;
			}
			fclose(fp);
		}
		if (!use_toshiba_hardware) return PM_Error;
	}

	if (Get_Fan_Status()<0x00)
  {
		fprintf(stderr, "direct access to toshiba hardware failed:\n");
		fprintf(stderr, "cannot get fan status!\n");
		return PM_Error;
	}

	if (Get_Fan_Status()==0x00) return 0;
	else return 1;
}


void toshiba_set_fan_status(int status)
{
	SMMRegisters reg;
  static int enable_spinoff=1;
  static time_t timer;

  if (status == 1)
  {
    if (toshiba_get_fan_status(2) == 1) return;
  	reg.eax = HCI_SET;
	  reg.ebx = HCI_FAN;
	  reg.ecx = HCI_ENABLE;
	  HciFunction(&reg);
    usleep(100000);
    if( toshiba_get_fan_status(2) == 1)
      fprintf(stderr, "toshiba_lib: cooling fan turned on\n");
    else
      fprintf(stderr, "toshiba_lib: unable to turn on cooling fan\n");
	  return;
  }

  if (status == 0)
  {
    if (toshiba_get_fan_status(2) == 0) return;
    if (!enable_spinoff)
    {
      if ( (time(NULL)-timer) >= 15 ) enable_spinoff=1;
      else return;
    }
    enable_spinoff=0;
    timer = time(NULL);
    UglyHack();
    if( toshiba_get_fan_status(2) == 0)
      fprintf(stderr, "toshiba_lib: cooling fan turned off\n");
    else
      fprintf(stderr, "toshiba_lib: unable to turn off cooling fan\n");
	  return;
  }

  fprintf(stderr, "toshiba_lib: selected invalid fan status\n");

}

int acpi_set_lcd_brightness(int brightness)
{
	FILE *fp;
	static int current_brightness=-1;
	int min_brightness = 0;
	static int max_brightness;

	if (current_brightness == -1)
	{
	  fp= fopen("/proc/acpi/toshiba/lcd", "r");
		if (!fp)
		{
			fprintf(stderr, "toshiba_lib: unable to set LCD brightness with ACPI.\n");
			return 0;
		}
		fscanf(fp, "%*s%d%*s%d", &current_brightness, &max_brightness);
		fclose(fp);
		fprintf(stderr, "toshiba_lib: min brightness is %d\n", min_brightness);
		fprintf(stderr, "toshiba_lib: max brightness is %d\n", --max_brightness);
		fprintf(stderr, "toshiba_lib: current brightness is %d\n", current_brightness);
	}

	if (brightness == TOSHIBA_LCD_MAX) brightness = max_brightness;
	if (brightness == TOSHIBA_LCD_MIN) brightness = 0;
	if (brightness == TOSHIBA_LCD_MED) brightness = max_brightness/2;

	if (brightness == UP_ONE_STEP)
	{
		if (current_brightness < max_brightness) brightness = current_brightness + 1;
		else return 1;
	}
	if (brightness == DOWN_ONE_STEP)
	{
		if (current_brightness > min_brightness) brightness = current_brightness - 1;
		else return 1;
	}

	if (maxBrightness != -1 && brightness > maxBrightness) brightness = maxBrightness;
	if (minBrightness != -1 && brightness < minBrightness) brightness = minBrightness;

	if (brightness != current_brightness)
	{
		fp = fopen("/proc/acpi/toshiba/lcd", "w");
		if (!fp)
		{
			fprintf(stderr, "toshiba_lib: unable to set LCD brightness with ACPI.\n");
			return 0;
		}
		fprintf(fp, "brightness:%d\n", brightness);
		fclose(fp);
		fp = fopen("/proc/acpi/toshiba/lcd", "r");
		fscanf(fp, "%*s%d", &current_brightness);
		fclose(fp);
		if (brightness == current_brightness)
			fprintf(stderr, "toshiba_lib: set LCD brightness to %d\n", brightness);
		else
		{
			fprintf(stderr, "toshiba_lib: unable to set LCD brightness with ACPI.\n");
			return 0;
		}
	}

	usleep (125000); /* You must let some time pass between changes */
	return 1;
}


void hardware_set_lcd_brightness(int brightness)
{
	static int current_brightness=-1;
  unsigned short lcd;
  static unsigned short lcdtype=999;
	SMMRegisters reg;

	lcd = brightness;

  if (lcdtype==999)
  {
    SciOpenInterface();
    reg.ebx = SCI_LCD_BRIGHTNESS;
	  if (SciGet(&reg)==SCI_SUCCESS) lcdtype = SCI_LCD_BRIGHTNESS;
	  else lcdtype = SCI_LCD_BACKLIGHT;
	  reg.ebx = SCI_LCD_MAXBRIGHT;
	  if (SciGet(&reg)==SCI_SUCCESS)
    {
		  lcdtype = SCI_LCD_MAXBRIGHT;
		  if (lcd>1) lcd = 1;
	  }
		fprintf(stderr, "toshiba_lib: min brightness is %d\n", TOSHIBA_LCD_MIN);
		fprintf(stderr, "toshiba_lib: max brightness is %d\n", TOSHIBA_LCD_MAX);

    SciCloseInterface();
  }

	if (brightness == UP_ONE_STEP)
	{/* Right now this is broken... */
	  /*if (current_brightness < TOSHIBA_LCD_MAX) lcd = current_brightness + 1;
			else*/ return;
	}
	if (brightness == DOWN_ONE_STEP)
	{/* Right now this is broken... */
	  /*if (current_brightness > TOSHIBA_LCD_MIN) lcd = current_brightness - 1;
			else */return;
	}

	if (maxBrightness != -1 && lcd > maxBrightness) lcd = maxBrightness;
	if (minBrightness != -1 && lcd < minBrightness) lcd = minBrightness;

  if (lcd != current_brightness)
  {
    SciOpenInterface();
  	reg.ebx = lcdtype;
		reg.ecx = lcd;
		if (SciSet(&reg)==SCI_FAILURE)
			fprintf(stderr, "toshiba_lib: unable to set LCD brightness\n");
    else
    {
      fprintf(stderr, "toshiba_lib: changed LCD brightness to %d\n", lcd);
      current_brightness=lcd;
    }
    SciCloseInterface();
  }

}

void toshiba_set_lcd_brightness(int brightness, int use_hardware_call)
{
	static int acpi_ok = 1;

	if (!use_hardware_call)
	{
	  acpi_set_lcd_brightness(brightness);
		return;
	}
	if (!acpi_ok)
	{
		hardware_set_lcd_brightness(brightness);
		return;
	}
	acpi_ok = acpi_set_lcd_brightness(brightness);
	if (!acpi_ok)
	{
		fprintf(stderr, "Trying to control directly Toshiba lcd hardware\n");
		hardware_set_lcd_brightness(brightness);
	}
}

void Toshiba_lcdBrightness_UpOneStep(int use_hardware)
{
	toshiba_set_lcd_brightness(UP_ONE_STEP, use_hardware);
}

void Toshiba_lcdBrightness_DownOneStep(int use_hardware)
{
	toshiba_set_lcd_brightness(DOWN_ONE_STEP, use_hardware);
}

/* Check if we are running on a Toshiba laptop */
int machine_is_toshiba(int *use_toshiba_hardware)
{
	int version,bios,id;
	int result = 0;
	DIR *toshiba_proc = opendir("/proc/acpi/toshiba");

	strcpy(toshiba_model, "cannot get model without direct hardware access");
	if (toshiba_proc)
	{
		closedir(toshiba_proc);
		result = 1;
	}
	if (!use_toshiba_hardware || !(*use_toshiba_hardware)) return result;

  /* do some quick checks on the laptop */
	if (SciSupportCheck(&version)==1)
	{
		*use_toshiba_hardware = 0;
		return result;
	}
	bios = HciGetBiosVersion();
	if (bios==0)
  {
		*use_toshiba_hardware = 0;
		return result;
	}
	if (HciGetMachineID(&id)==HCI_FAILURE)
  {
		*use_toshiba_hardware = 0;
		return result;
	}

	strcpy(toshiba_model, "unknown model");
	switch (id)
  {
		case 0xfc00: strcpy(toshiba_model, "Satellite 2140CDS/2180CDT/2675DVD"); break;
		case 0xfc01: strcpy(toshiba_model, "Satellite 2710xDVD"); break;
		case 0xfc02: strcpy(toshiba_model, "Satellite Pro 4270CDT//4280CDT/4300CDT/4340CDT"); break;
		case 0xfc04: strcpy(toshiba_model, "Portege 3410CT, 3440CT"); break;
		case 0xfc08: strcpy(toshiba_model, "Satellite 2100CDS/CDT 1550CDS"); break;
		case 0xfc09: strcpy(toshiba_model, "Satellite 2610CDT, 2650XDVD"); break;
		case 0xfc0a: strcpy(toshiba_model, "Portage 7140"); break;
		case 0xfc0b: strcpy(toshiba_model, "Satellite Pro 4200"); break;
		case 0xfc0c: strcpy(toshiba_model, "Tecra 8100x"); break;
		case 0xfc0f: strcpy(toshiba_model, "Satellite 2060CDS/CDT"); break;
		case 0xfc10: strcpy(toshiba_model, "Satellite 2550/2590"); break;
		case 0xfc11: strcpy(toshiba_model, "Portage 3110CT"); break;
		case 0xfc12: strcpy(toshiba_model, "Portage 3300CT"); break;
		case 0xfc13: strcpy(toshiba_model, "Portage 7020CT"); break;
		case 0xfc15: strcpy(toshiba_model, "Satellite 4030/4030X/4050/4060/4070/4080/4090/4100X CDS/CDT"); break;
		case 0xfc17: strcpy(toshiba_model, "Satellite 2520/2540 CDS/CDT"); break;
		case 0xfc18: strcpy(toshiba_model, "Satellite 4000/4010 XCDT"); break;
		case 0xfc19: strcpy(toshiba_model, "Satellite 4000/4010/4020 CDS/CDT"); break;
		case 0xfc1a: strcpy(toshiba_model, "Tecra 8000x"); break;
		case 0xfc1c: strcpy(toshiba_model, "Satellite 2510CDS/CDT"); break;
		case 0xfc1d: strcpy(toshiba_model, "Portage 3020x"); break;
		case 0xfc1f: strcpy(toshiba_model, "Portage 7000CT/7010CT"); break;
		case 0xfc39: strcpy(toshiba_model, "T2200SX"); break;
		case 0xfc40: strcpy(toshiba_model, "T4500C"); break;
		case 0xfc41: strcpy(toshiba_model, "T4500"); break;
		case 0xfc45: strcpy(toshiba_model, "T4400SX/SXC"); break;
		case 0xfc51: strcpy(toshiba_model, "Satellite 2210CDT, 2770XDVD"); break;
		case 0xfc52: strcpy(toshiba_model, "Satellite 2775DVD, Dynabook Satellite DB60P/4DA"); break;
		case 0xfc53: strcpy(toshiba_model, "Portage 7200CT/7220CT, Satellite 4000CDT"); break;
		case 0xfc54: strcpy(toshiba_model, "Satellite 2800DVD"); break;
		case 0xfc56: strcpy(toshiba_model, "Portage 3480CT"); break;
		case 0xfc57: strcpy(toshiba_model, "Satellite 2250CDT"); break;
		case 0xfc5a: strcpy(toshiba_model, "Satellite Pro 4600"); break;
		case 0xfc5d: strcpy(toshiba_model, "Satellite 2805"); break;
		case 0xfc5f: strcpy(toshiba_model, "T3300SL"); break;
		case 0xfc61: strcpy(toshiba_model, "Tecra 8200"); break;
		case 0xfc64: strcpy(toshiba_model, "Satellite 1800"); break;
		case 0xfc69: strcpy(toshiba_model, "T1900C"); break;
		case 0xfc70: strcpy(toshiba_model, "Libretto L2"); break;
		case 0xfc6a: strcpy(toshiba_model, "T1900"); break;
		case 0xfc6c: strcpy(toshiba_model, "Satellite 5005 S504"); break;
		case 0xfc6d: strcpy(toshiba_model, "T1850C"); break;
		case 0xfc6e: strcpy(toshiba_model, "T1850"); break;
		case 0xfc6f: strcpy(toshiba_model, "T1800"); break;
		case 0xfc71: strcpy(toshiba_model, "Satellite Pro 6000"); break;
		case 0xfc72: strcpy(toshiba_model, "Satellite 1800"); break;
		case 0xfc7d: strcpy(toshiba_model, "Satellite Pro 6100"); break;
		case 0xfc7e: strcpy(toshiba_model, "T4600C"); break;
		case 0xfc7f: strcpy(toshiba_model, "T4600"); break;
		case 0xfc8a: strcpy(toshiba_model, "T6600C"); break;
		case 0xfc91: strcpy(toshiba_model, "T2400CT"); break;
		case 0xfc97: strcpy(toshiba_model, "T4800CT"); break;
		case 0xfc99: strcpy(toshiba_model, "T4700CS"); break;
		case 0xfc9b: strcpy(toshiba_model, "T4700CT"); break;
		case 0xfc9d: strcpy(toshiba_model, "T1950"); break;
		case 0xfc9e: strcpy(toshiba_model, "T3400/T3400CT"); break;
		case 0xfca6: strcpy(toshiba_model, "Portege 2010"); break;
		case 0xfca9: strcpy(toshiba_model, "Satellite 2410-303"); break;
		case 0xfcb2: strcpy(toshiba_model, "Libretto 30CT"); break;
		case 0xfcba: strcpy(toshiba_model, "T2150"); break;
		case 0xfcbe: strcpy(toshiba_model, "T4850CT"); break;
		case 0xfcc0: strcpy(toshiba_model, "Satellite Pro 420x"); break;
		case 0xfcc1: strcpy(toshiba_model, "Satellite 100x"); break;
		case 0xfcc3: strcpy(toshiba_model, "Tecra 710x/720x"); break;
		case 0xfcc6: strcpy(toshiba_model, "Satellite Pro 410x"); break;
		case 0xfcca: strcpy(toshiba_model, "Satellite Pro 400x"); break;
		case 0xfccb: strcpy(toshiba_model, "Portage 610CT"); break;
		case 0xfccc: strcpy(toshiba_model, "Tecra 700x"); break;
		case 0xfccf: strcpy(toshiba_model, "T4900CT"); break;
		case 0xfcd0: strcpy(toshiba_model, "Satellite 300x"); break;
		case 0xfcd1: strcpy(toshiba_model, "Tecra 750CDT"); break;
		case 0xfcd2: strcpy(toshiba_model, "Vision Connect -- what is this???"); break;
		case 0xfcd3: strcpy(toshiba_model, "Tecra 730XCDT"); break;
		case 0xfcd4: strcpy(toshiba_model, "Tecra 510x"); break;
		case 0xfcd5: strcpy(toshiba_model, "Satellite 200x"); break;
		case 0xfcd7: strcpy(toshiba_model, "Satellite Pro 430x"); break;
		case 0xfcd8: strcpy(toshiba_model, "Tecra 740x"); break;
		case 0xfcd9: strcpy(toshiba_model, "Portage 660CDT"); break;
		case 0xfcda: strcpy(toshiba_model, "Tecra 730CDT"); break;
		case 0xfcdb: strcpy(toshiba_model, "Portage 620CT"); break;
		case 0xfcdc: strcpy(toshiba_model, "Portage 650CT"); break;
		case 0xfcdd: strcpy(toshiba_model, "Satellite 110x"); break;
		case 0xfcdf: strcpy(toshiba_model, "Tecra 500x"); break;
		case 0xfce0: strcpy(toshiba_model, "Tecra 780DVD"); break;
		case 0xfce2: strcpy(toshiba_model, "Satellite 300x"); break;
		case 0xfce3: strcpy(toshiba_model, "Satellite 310x"); break;
		case 0xfce4: strcpy(toshiba_model, "Satellite Pro 490x"); break;
		case 0xfce5: strcpy(toshiba_model, "Libretto 100CT"); break;
		case 0xfce6: strcpy(toshiba_model, "Libretto 70CT"); break;
		case 0xfce7: strcpy(toshiba_model, "Tecra 540x/550x"); break;
		case 0xfce8: strcpy(toshiba_model, "Satellite Pro 470x/480x"); break;
		case 0xfce9: strcpy(toshiba_model, "Tecra 750DVD"); break;
		case 0xfcea: strcpy(toshiba_model, "Libretto 60"); break;
		case 0xfceb: strcpy(toshiba_model, "Libretto 50CT"); break;
		case 0xfcec: strcpy(toshiba_model, "Satellite 320x/330x, Satellite 2500CDS"); break;
		case 0xfced: strcpy(toshiba_model, "Tecra 520x/530x"); break;
		case 0xfcef: strcpy(toshiba_model, "Satellite 220x, Satellite Pro 440x/460x"); break;
		default:
			fprintf(stderr, "toshiba_lib: unrecognized machine identification:\n");
			fprintf(stderr, "             machine id : 0x%04x    BIOS version : %d.%d    SCI version: %d.%d\n",
							id, (bios & 0xff00)>>8, bios & 0xff,
							(version & 0xff00)>>8, version & 0xff);
			fprintf(stderr, "please report this info, along with your toshiba model,\n");
			fprintf(stderr, "to noberasco.gnu@disi.unige.it\n");
	}

  return 1;
}






/* INTERNAL FUNCTIONS */


int Get_Fan_Status(void)
{
	SMMRegisters reg;

	reg.eax = HCI_GET;
	reg.ebx = HCI_FAN;
	HciFunction(&reg);

	if ((reg.eax & 0xff00)!=HCI_SUCCESS)
		return -1;
	else
		return (int) (reg.ecx & 0xff);
}

void UglyHack(void)
{
	unsigned short save=0,display=512,sleep=0,speed=1,cooling=1;
	SMMRegisters reg;
	int user;

	SciOpenInterface();
	reg.ebx = SCI_BATTERY_SAVE;
	reg.ecx = save;
	if (SciSet(&reg)==SCI_SUCCESS) user = 0;
  else user = 1;
	if ((save==SCI_USER_SETTINGS) || (user==1))
  {
		reg.ebx = SCI_DISPLAY_AUTO;
		reg.ecx = display;
		if (SciSet(&reg)==SCI_FAILURE) {}
		reg.ebx = SCI_SLEEP_MODE;
		reg.ecx = sleep;
		if (SciSet(&reg)==SCI_FAILURE) {}
		reg.ebx = SCI_PROCESSING;
		reg.ecx = speed;
		if (SciSet(&reg)==SCI_FAILURE) {}
		reg.ebx = SCI_COOLING_METHOD;
		reg.ecx = cooling;
		if (SciSet(&reg)==SCI_FAILURE) {}
		if (user==0)
    {
			reg.ebx = SCI_BATTERY_SAVE;
			reg.ecx = 1;
			if (SciSet(&reg)==SCI_FAILURE) {}
			reg.ebx = SCI_BATTERY_SAVE;
			reg.ecx = save;
			if (SciSet(&reg)==SCI_FAILURE) {}
		}
	}
	SciCloseInterface();
  reg.eax = HCI_SET;
  reg.ebx = HCI_FAN;
  reg.ecx = HCI_DISABLE;
  HciFunction(&reg);
  usleep(100000);
  SciOpenInterface();
  reg.ebx = SCI_COOLING_METHOD;
	reg.ecx = FAN_OFF;
  SciCloseInterface();
  usleep(100000);

	return;
}
