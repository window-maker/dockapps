/***************************************************************************
                        toshiba_lib.h  -  description
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

#define TOSHIBA_LCD_MIN 0
#define TOSHIBA_LCD_MED 1
#define TOSHIBA_LCD_MAX 2

char toshiba_model[255];

int machine_is_toshiba(int *use_toshiba_hardware);
int toshiba_get_fan_status(int use_toshiba_hardware);
void toshiba_set_fan_status(int status);
void toshiba_set_lcd_brightness(int brightness, int allow_hardware_call);
void Toshiba_lcdBrightness_UpOneStep(int allow_hardware_call);
void Toshiba_lcdBrightness_DownOneStep(int allow_hardware_call);
