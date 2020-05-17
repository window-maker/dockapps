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

/*

 This is the base class which all cpu monitors should inherit
 from. Best is to use one of the existing monitors as a
 template when writing a new one. By inheriting this base
 class it can be as simple as implementing just one function
 to support a new operating system. This function is
 getLoad() which should return the (average (in the case of
 a smp-system)) intermediate cpu-load in the range [0.0, 1.0]
 multiplied by the inherited protected variable 'range'.

*/

#ifndef _BASECPUMONITOR_H_
#define _BASECPUMONITOR_H_

#include <stdio.h>
#include <stdexcept>

#define CPUMONCC_VERSION "0.0.1-pre1"
#define CPUMONCC_DATE "2003-02-14"

using namespace std;

class BaseCpuMonitor
{
public:
  
  BaseCpuMonitor();
  virtual float getLoad() = 0;
  void setRange(float);
  bool saveInternals(const char *);
  bool loadInternals(const char *);
  
protected:
  
  int   which_cpu;
  bool  use_nice;
  int   previous_total;
  int   previous_used;
  float range;
};

inline BaseCpuMonitor::BaseCpuMonitor()
{
	which_cpu = -1;
  	use_nice = true;
  	previous_total = 0;
  	previous_used = 0;
  	// Set default range to percent
  	range = 100.0;
}

inline void BaseCpuMonitor::setRange(float irange)
{
  	range = irange;
}

inline bool BaseCpuMonitor::saveInternals(const char *filename)
{
	int count;
	FILE *fp = fopen(filename, "w");
		
	if (fp == 0) return false;
	
	count = fprintf(fp, "%d %d", previous_used, previous_total);
	fclose(fp);
	
	if (count != 2) return false;	

	return true;
}

inline bool BaseCpuMonitor::loadInternals(const char *filename)
{
	int count;
	FILE *fp = fopen(filename, "r");
		
	if (fp == 0) return false;
	
	count = fscanf(fp, "%d %d", &previous_used, &previous_total);
	fclose(fp);
	
	if (count != 2) return false;	

	return true;
}


#endif
