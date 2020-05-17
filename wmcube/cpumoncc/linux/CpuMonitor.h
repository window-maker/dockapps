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

#ifndef _CPUMONITOR_H_
#define _CPUMONITOR_H_

#include "BaseCpuMonitor.h"

class CpuMonitor : public BaseCpuMonitor
{
public:
  
  CpuMonitor();
  CpuMonitor(int cpu);
  CpuMonitor(int cpu, bool nice);
  
  float getLoad();

private:

  void initialize();
};

#endif
