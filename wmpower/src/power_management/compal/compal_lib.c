/***************************************************************************
                        compal_lib.c  -  description
                             -------------------
    begin                : Oct 01 2003
    copyright            : (C) 2003 by Francisco Rodrigo Escobedo Robles
    e-mail               : frer@pepix.net
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
       Many thanks to Soós Péter <sp@osb.hu> and the omke project team
                   I could never have done this otherwise
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "compal_lib.h"
#include "lib_utils.h"
#include "power_management.h"



/* gets a value from a strtok-ed buffer; for reading from /proc */
char *getvaluefromhash (char *key, char *hash)
{
	char *value;
	char *lastchar;


	for ( ;
		 NULL != hash && strcmp (key, hash) ;
		 hash = strtok (NULL, ":\n"))
		;

	value = strtok (NULL, ":\n");

	while (*value == ' ') ++value;

	for (lastchar = value + strlen (value) ;
		 *--lastchar == ' ' ;
		 *lastchar = '\0')
		 ;

	return value;
}



int machine_is_compal (void)
{
	FILE *fp = fopen(COMPAL_PROC_FILE_DMI, "r");
	char  buf [COMPAL_MAX_DMI_INFO + 1];
	char *ptr;
	char *vendor;
	char *prodname;


	if (!fp) return 0;

	fread  (&buf, 1, COMPAL_MAX_DMI_INFO, fp);
	fclose (fp);

	/* always get the values in order! */
	ptr      = strtok (buf, ":\n");
	vendor   = getvaluefromhash ("System Vendor", ptr);
	prodname = getvaluefromhash ("Product Name", ptr);
	sprintf (compal_model, "%s %s", vendor, prodname);

	return 1;
}



int compal_get_fan_status (void)
{
	FILE *fp = fopen (COMPAL_PROC_FILE_FAN, "r");
	char  fan_status[3];

	if (!fp) return PM_Error;

	if (fscanf (fp, "%*s%*s%2s", fan_status) != 1)
	{
		fclose (fp);
		return PM_Error;
	}
	fclose (fp);

	if (!strcmp (fan_status, "on")) return 1;
	return 0;
}



int compal_get_temperature (void)
{
	FILE *fp = fopen (COMPAL_PROC_FILE_TEMP, "r");
	int   result;

	if (!fp) return PM_Error;

	if (fscanf(fp, "%*s%*s%d", &result) == 1)
	{
		fclose(fp);
		return result;
	}

	fclose(fp);
	return PM_Error;
}



int compal_get_lcd_brightness (void)
{
	FILE *fp = fopen (COMPAL_PROC_FILE_LCD, "r");
	int   brightness;

	if (!fp) return PM_Error;

	if (fscanf (fp, "%*s%*s%d", &brightness) == 1)
	{
		fclose (fp);
		return brightness;
	}

	fclose (fp);
	return PM_Error;
}



int compal_set_lcd_brightness (int brightness)
{
	FILE *fp = fopen (COMPAL_PROC_FILE_LCD, "w");

	if (!fp) return PM_Error;

	if (brightness < COMPAL_LCD_MIN) brightness = COMPAL_LCD_MIN;
	if (brightness > COMPAL_LCD_MAX) brightness = COMPAL_LCD_MAX;
	if (maxBrightness != -1 && brightness > maxBrightness) brightness = maxBrightness;
	if (minBrightness != -1 && brightness < minBrightness) brightness = minBrightness;

	fprintf (fp, "%i", brightness);
	fclose  (fp);
	return 0;
}



void Compal_lcdBrightness_UpOneStep (void)
{
	compal_set_lcd_brightness (compal_get_lcd_brightness () + 1);
}



void Compal_lcdBrightness_DownOneStep (void)
{
	compal_set_lcd_brightness (compal_get_lcd_brightness () - 1);
}
