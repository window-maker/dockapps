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

// CpuMonitorCC/OpenBSD

#include <stdlib.h>
#include "CpuMonitor.h"

CpuMonitor::CpuMonitor() : BaseCpuMonitor()
{
}

float CpuMonitor::getLoad()
{
  float t;
  double avenrun[3];
  getloadavg(avenrun, sizeof(avenrun) / sizeof(avenrun[0]));
  
  // t = 0..100
  t = 2 * (((5.0*avenrun[0] + 0.5) > 50) ? 50 : (5.0*avenrun[0] + 0.5));

  return range * t / 100.0;
}
