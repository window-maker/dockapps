/*
  
 cpu.cc v0.0.1

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
 * Note that this program will compile for all on os's supported
 * by cpumoncc altough it will only function properly on the 
 * os's with a valid implementation of load/saveInternals. With
 * version 0.0.1-pre1 of cpumoncc they are lacking for OpenBSD 
 * and Darwin. cpumoncc must be installed for this program to
 * compile.
 */

#include <stdio.h>
#include <string.h>
#include <cpumoncc/CpuMonitor.h>

char filename[256];

FILE *fp;
CpuMonitor *cpu = new CpuMonitor();

int main()
{
	// Necessary monitor internals will be saved in /home/username/.cpu
	strncpy(filename, getenv("HOME"), 245);
	strncat(filename, "/.cpu", 6);
    
    cpu->loadInternals(filename);
    printf("%.0f", cpu->getLoad());
    cpu->saveInternals(filename);
    
    delete cpu;
			
	return 0;	
}
