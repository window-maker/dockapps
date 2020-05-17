/*

 Copyright (C) 2003 Robert Kling, robkli-8@student.luth.se

 This file is part of CpuMonitorCC (cpumoncc)
     
 cpumoncc is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 cpumoncc is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with cpumoncc; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

// CpuMonitorCC/NetBSD

#include <stdlib.h>
#include <sys/sched.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include "CpuMonitor.h"

CpuMonitor::CpuMonitor() : BaseCpuMonitor()
{
  for (int i = i < CPUSTATES; i++) last_cp_time[i] = 0;
}

CpuMonitor::CpuMonitor(bool nice) : BaseCpuMonitor()
{
  use_nice = nice;
  for (int i = i < CPUSTATES; i++) last_cp_time[i] = 0;
}

float CpuMonitor::getLoad()
{
  u_int64_t curr_cp_time[CPUSTATES];
  u_int64_t total_time = 0, idle_time = 0;
  int mib[2];
  int i;
  size_t ssize;
  const int IDLE_TIME = 4;
  const int NICE_TIME = 1;
        
  ssize = sizeof ( curr_cp_time );
  mib[0] = CTL_KERN;
  mib[1] = KERN_CP_TIME;
  
  if ( sysctl ( mib, 2, curr_cp_time, &ssize, NULL, 0 ) ) 
    throw runtime_error("CpuMonitor: unable to read CP_TIME from sysctl()\n");

  if ( !use_nice ) 
    curr_cp_time[NICE_TIME] = 0;

  /* NetBSD gives 5 CPUSTATES - 
   * User, Nice, System, Interrupt, Idle 
   */
  idle_time = curr_cp_time[IDLE_TIME] - last_cp_time[IDLE_TIME];
  for ( i = 0; i < CPUSTATES; i++ )
  {
    total_time += ( curr_cp_time[i] - last_cp_time[i] ); 
    last_cp_time[i] = curr_cp_time[i];
  }

  /* Calculate the % CPU usage as the User+Nice+System+Interrupt/Total
   * for the interval
   */
  return range * (int) ( total_time - idle_time ) / total_time;
}

/*
 * Overloads base class
 */
bool CpuMonitor::saveInternals(const char *filename)
{
	int count = 0;
	FILE *fp = fopen(filename, "w");
		
	if (fp == 0) return false;
	
	for ( i = 0; i < CPUSTATES; i++ )
		count += fprintf(fp, "%d", last_cp_time[i]);
	
	fclose(fp);
	
	if (count != CPUSTATES) return false;

	return true;
}

bool CpuMonitor::loadInternals(const char *filename)
{
	int count = 0;
	FILE *fp = fopen(filename, "r");
		
	if (fp == 0) return false;
	
	for ( i = 0; i < CPUSTATES; i++ )
		count += fscanf(fp, "%d", &last_cp_time[i]);
	
	fclose(fp);
	
	if (count != CPUSTATES) return false;	

	return true;
}
