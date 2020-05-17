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

// CpuMonitorCC/Linux

#include <stdio.h>
#include "CpuMonitor.h"

CpuMonitor::CpuMonitor() : BaseCpuMonitor()
{
  initialize();
}

CpuMonitor::CpuMonitor(int cpu) : BaseCpuMonitor()
{
  which_cpu = cpu;
  initialize();
}

CpuMonitor::CpuMonitor(int cpu, bool nice) : BaseCpuMonitor()
{
  which_cpu = cpu;
  use_nice = nice;
  initialize();
}

void CpuMonitor::initialize()
{
  FILE *fp;
  int i;
  char cpuid[6];
  char check_cpu[6];
  char tmp[32];

  if ((fp = fopen("/proc/stat","r")) == NULL) 
    throw runtime_error("CpuMonitor: no /proc/stat found.");

  if (which_cpu == -1) return;

  snprintf(check_cpu, 6, "cpu%d", which_cpu);
  //printf("Monitoring %s (%d)\n", check_cpu, which_cpu);

  for (i = -2; i < which_cpu; i++) 
    {
      fscanf(fp, "%5s %31s %31s %31s %31s", cpuid, tmp, tmp, tmp, tmp);
      //fscanf(fp, "%5s" , cpuid); printf("%s ", cpuid);
      //fscanf(fp, "%31s", tmp  ); printf("%s ", tmp);
      //fscanf(fp, "%31s", tmp  ); printf("%s ", tmp);
      //fscanf(fp, "%31s", tmp  ); printf("%s ", tmp);
      //fscanf(fp, "%31s", tmp  ); printf("%s\n", tmp); fflush(stdout);
    }
	
  if (strcmp(check_cpu, cpuid) != 0)
    throw runtime_error("CpuMonitor: could not read cpu-load.");
}

float CpuMonitor::getLoad()
{
  int total, used, i;
  char cpuid[6];
  int cpu,nice,system,idle;
  float t;
  FILE *fp;
	
  fp = fopen("/proc/stat","r");

  for (i = -2; i < which_cpu; i++)
    fscanf(fp,"%5s %d %d %d %d", cpuid, &cpu, &nice, &system, &idle);
	
  fclose(fp);

  used = cpu + system + use_nice * nice;
  total = used + idle + (1-use_nice) * nice;

  t = range * (float)(used - previous_used) / (float)(total - previous_total);
  previous_total = total;
  previous_used = used;
	
  return t;
}

