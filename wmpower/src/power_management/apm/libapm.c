/***************************************************************************
                          libapm.c  -  description
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
 *   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.              *
 *                                                                         *
 ***************************************************************************/
 
 /***************************************************************************
        Originally written by Filippo Panessa for his 'wmab' program
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "libapm.h"

int apm_read(struct_apm_data *i);
int apm_exists();

/* Check to see if /proc/apm exists... */
int apm_exists()
{
    if (!access(APMDEV, R_OK)) return 1;

  	return 0;
}

/* Read in the information found in /proc/apm... */
int apm_read(struct_apm_data *i)
{
  FILE 	*str;
  char 	 units[10];
  char 	 buffer[100];

  /* Open /proc/apm for reading */
  if (!(str = fopen(APMDEV, "r"))) return 0;

  /* Scan in the information.... */
  fgets(buffer, sizeof(buffer) - 1, str);
  buffer[sizeof(buffer) - 1] = '\0';
  sscanf(buffer, "%s %d.%d %x %x %x %x %d%% %d %s\n",
    (char *)i->driver_version,
    &i->apm_version_major,
    &i->apm_version_minor,
	  (unsigned int *) &i->apm_flags,
	  (unsigned int *) &i->ac_line_status,
	  (unsigned int *) &i->battery_status,
	  (unsigned int *) &i->battery_flags,
	  &i->battery_percentage,
	  &i->battery_time,
	  units
  );
  i->using_minutes = !strncmp(units, "min", 3) ? 1 : 0;
  /* Old Style */
  if (i->driver_version[0] == 'B')
	{
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
	  sscanf(buffer, "Flags: 0x%02x", (unsigned int *) &i->apm_flags);
	  if (i->apm_flags & APM_32_BIT_SUPPORT)
		{
	    fgets(buffer, sizeof(buffer) - 1, str);
	    fgets(buffer, sizeof(buffer) - 1, str);
	    if (buffer[0] != 'P')
			{
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
		    if (i->apm_version_major >= 1 && i->apm_version_minor >= 1)
				{
		      fgets(buffer, sizeof(buffer) - 1, str);
		      sscanf(buffer, "Battery flag: 0x%02x", (unsigned int *) &i->battery_flags);
		      fgets(buffer, sizeof(buffer) - 1, str);
		      if (strncmp(buffer+14, "unknown", 7))  i->battery_time = atoi(buffer + 14);
		    }
	    }
	  }
  }

  /* Take care of battery percentages > 100% */
  if (i->battery_percentage > 100) i->battery_percentage = -1;
  fclose(str);

	return 1;
}
