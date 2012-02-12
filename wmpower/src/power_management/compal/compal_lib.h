/***************************************************************************
                        compal_lib.h  -  description
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
 *   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.              *
 *                                                                         *
 ***************************************************************************/
 
 /***************************************************************************
       Many thanks to Soós Péter <sp@osb.hu> and the omke project team
                   I could never have done this otherwise
 ***************************************************************************/


#define COMPAL_LCD_MIN  	0
#define COMPAL_LCD_MAX 		10
#define COMPAL_MAX_DMI_INFO	1000
#define COMPAL_MAX_BATT_INFO	1000
#define COMPAL_MAX_MODEL_INFO	255

#define COMPAL_PROC_FILE_DMI	"/proc/omnibook/dmi"
#define COMPAL_PROC_FILE_FAN	"/proc/omnibook/fan"
#define COMPAL_PROC_FILE_TEMP	"/proc/omnibook/temperature"
#define COMPAL_PROC_FILE_LCD	"/proc/omnibook/lcd"
#define COMPAL_PROC_FILE_BATT	"/proc/omnibook/battery"


char  compal_model[COMPAL_MAX_MODEL_INFO];

char *getvaluefromhash (char *key, char *hash);

int   machine_is_compal(void);
int   compal_get_fan_status(void);
int   compal_get_temperature(void);
int   compal_get_lcd_brightness(void);
int   compal_set_lcd_brightness(int brightness);
void  Compal_lcdBrightness_UpOneStep(void);
void  Compal_lcdBrightness_DownOneStep(void);
