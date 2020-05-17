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

// CpuMonitorCC/Solaris

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

void CpuMonitor::initialize()
{
  kstat_t *ksp;
  int i = 0;

  if (kc == NULL)
    if ((kc = kstat_open()) == NULL)
      throw runtime_error("CpuMonitor: can't open /dev/kstat.");
  
  if (which_cpu != -1)
    {
      /*
       * User selected to monitor a particlur CPU. Find it...
     */
      for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next)
	if ((strcmp(ksp->ks_module, "cpu_stat") == 0) && (ksp->ks_instance == which_cpu))
	  {
	    the_cpu = ksp;
	    break;
	  }
      if (the_cpu == NULL) throw runtime_error("CpuMonitor: Cpu not found.");
    }
  else
    {
      /*
       * User selected to monitor all CPUs.  First, count them.
	 */
      for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next)
	if (strcmp(ksp->ks_module, "cpu_stat") == 0) i++;
	
      if (cpu_ksp_list) free(cpu_ksp_list);
	
      cpu_ksp_list = (kstat_t **) calloc(i * sizeof (kstat_t *), 1); 
      ncpus = i;

      /*
       * stash the ksp for each CPU.
       */
      i = 0;
      for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next)
	if (strcmp(ksp->ks_module, "cpu_stat") == 0) 
	  {
	    cpu_ksp_list[i] = ksp;
	    i++;
	  }
    }
}

float CpuMonitor::getLoad()
{
  int i;
  cpu_stat_t stat;
  int used, total, user = 0, wait = 0, kern = 0, idle = 0;
  float t;

  /*
   * Read each cpu's data.  If the kstat chain has changed (a state change
   * has happened, maybe a new cpu was added to the system or one went
   * away), then reinitialize everything with initialize().  Finally,
   * recursively call getLoad().
   *
   * We'll need to do a little better than this in the future, since we
   * could recurse too much in the pathological case here.
   */
  if (which_cpu == -1)
    {
      for (i = 0; i < ncpus; i++)
	{
	  if (kstat_read(kc, cpu_ksp_list[i], (void *)&stat) == -1)
	    {
	      // Dont try-catch, let the caller of first getLoad() do that
	      initialize();
	      
	      return getLoad();
	    }
	  user += stat.cpu_sysinfo.cpu[CPU_USER];   /* user */
	  wait += stat.cpu_sysinfo.cpu[CPU_WAIT];   /* io wait */
	  kern += stat.cpu_sysinfo.cpu[CPU_KERNEL]; /* sys */
	  idle += stat.cpu_sysinfo.cpu[CPU_IDLE]; /*idle("free")*/
	}
    }
  else
    {
      if (kstat_read(kc, the_cpu, (void *)&stat) == -1)
	{
	  // Dont try-catch, let the caller of first getLoad() do that
	  initialize();
	  
	  return getLoad();
	}
      user += stat.cpu_sysinfo.cpu[CPU_USER];    /* user */
      wait += stat.cpu_sysinfo.cpu[CPU_WAIT];    /* io wait */
      kern += stat.cpu_sysinfo.cpu[CPU_KERNEL];  /* sys */
      idle += stat.cpu_sysinfo.cpu[CPU_IDLE];    /* idle("free") */
    }
  
  used = user + wait + kern;
  total = used + idle;
  t = range * (float)(used - previous_used) /  (float)(total - previous_total);
  previous_total = total;
  previous_used = used;
  
  return t;
}
