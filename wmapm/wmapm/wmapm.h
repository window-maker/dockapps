/*
 *	wmapm.h  -- Header file for WMAPM
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
 *
 */




#define DELAY 10000L		/* Delay between refreshes (in microseconds) */

#define WMAPM_VERSION "3.1"

#ifdef FreeBSD
# define APMDEV "/dev/apm"
#else
# ifdef Linux
#  define APMDEV "/proc/apm"
# endif
#endif

typedef struct my_apm_info {
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
} my_apm_info;

#ifdef Linux
struct my_apm_info apm_info;
# ifndef APM_32_BIT_SUPPORT
#  define APM_32_BIT_SUPPORT      0x0002
# endif
#endif
