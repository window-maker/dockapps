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

// CpuMonitorCC/FreeBSD

#include "CpuMonitor.h"

CpuMonitor::CpuMonitor() : BaseCpuMonitor()
{
  initialize();
}

CpuMonitor::CpuMonitor(bool nice) : BaseCpuMonitor()
{
  use_nice = nice;
  initialize();
}

CpuMonitor::~CpuMonitor()
{
  kvm_close(kd);
}

void CpuMonitor::initialize()
{
  nlst[0].n_name = "_cp_time";
  nlst[1].n_name = NULL;

  if ((kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open")) == NULL)
    throw runtime_error("CpuMonitor: unable to open kvm.");
  
  kvm_nlist(kd, nlst);
  
  if (nlst[0].n_type == 0)
    throw runtime_error("CpuMonitor: unable to get nlist.");
}

float CpuMonitor::getLoad()
{
  int total, used;
  int cpu,nice,system,idle;
  unsigned long int cpu_time[CPUSTATES];
  float t;

  if (kvm_read(kd, nlst[0].n_value, &cpu_time, sizeof(cpu_time)) != sizeof(cpu_time))
    throw runtime_error("CpuMonitor: error reading kvm.");
  
  cpu = cpu_time[CP_USER];
  nice = cpu_time[CP_NICE];
  system = cpu_time[CP_SYS];
  idle = cpu_time[CP_IDLE];
  
  used = cpu + system + use_nice*nice;
  total = used + idle + (1-use_nice)*nice;
  
  t = range * (float)(used - previous_used) / (float)(total - previous_total);
  previous_total = total;
  previous_used = used;
  
  return t;
}
