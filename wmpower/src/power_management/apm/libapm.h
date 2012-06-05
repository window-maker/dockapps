/***************************************************************************
                          libapm.h  -  description
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
        Originally written by Filippo Panessa for his 'wmab' program
 ***************************************************************************/

#define APMDEV "/proc/apm"

typedef struct 
{
    const char driver_version[10];
    int        apm_version_major;
    int        apm_version_minor;
    int        apm_flags;
    int        ac_line_status;
    int        battery_status;
    int        battery_flags;
    int        battery_percentage;
    int        battery_time;
    int        using_minutes;
} struct_apm_data;

# ifndef APM_32_BIT_SUPPORT
#  define APM_32_BIT_SUPPORT      0x0002
# endif

int apm_exists(void);
int apm_read(struct_apm_data *);
