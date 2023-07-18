
/* 
   Mac OS X (Darwin) code written by Ben Hines <bhines@alumni.ucsd.edu> 

*/



#include "defines.h"
#include "aquarium.h"
#include "status.h"

#include <mach/mach_init.h>
#include <mach/mach_host.h>

/* Darwin */

void status_init(void)
{
}

void status_exit(void)
{
}
int status_sensors(int x, int y)
{
    return 0;
}
int status_swap(void)
{
    return 0;
}
int status_net(int type, int direction)
{
    return 0;
}
int status_mem(void)
{
    return 0;
}
int status_disc(char *drive)
{
    return 0;
}


/* Have the old ones global */
static u_int64_t oload = 0, ototal = 0;
static int firsttimes = 0, current = 0;
static int cpu_average_list[CPUSMOOTHNESS];

int status_cpu()
{
    processor_cpu_load_info_data_t *pinfo;
    mach_msg_type_number_t info_count;
    unsigned long composite_user, composite_nice, composite_sys, composite_idle;
    unsigned int cpuload, n_cpus;
    u_int64_t load, total;
    int i;
    if (firsttimes == 0) {
	for (i = 0; i < CPUSMOOTHNESS; i++)
	    cpu_average_list[i] = 0;
    }
    /* Wait until we have CPUSMOOTHNESS messures */
    if (firsttimes != CPUSMOOTHNESS)
	firsttimes++;
	
    if (host_processor_info (mach_host_self (),
			     PROCESSOR_CPU_LOAD_INFO,
			     &n_cpus,
			     (processor_info_array_t*)&pinfo,
			     &info_count)) {
	return 0;
    }

    composite_user = composite_nice = composite_sys = composite_idle = 0;

    for (i = 0; i < n_cpus; i++) {
	composite_user  += pinfo[i].cpu_ticks [CPU_STATE_USER];
	composite_sys   += pinfo[i].cpu_ticks [CPU_STATE_SYSTEM];
	composite_idle  += pinfo[i].cpu_ticks [CPU_STATE_IDLE];
	composite_nice  += pinfo[i].cpu_ticks [CPU_STATE_NICE];
    }
    vm_deallocate (mach_task_self (), (vm_address_t) pinfo, info_count);

    /* user + sys = load
     * total = total */
    load = composite_user + composite_sys;	/* cpu.user + cpu.sys; */
    total = load + composite_idle + composite_nice;	/* cpu.total; */

    /* Calculates an average from the last CPUSMOOTHNESS messures */

    if(total!=ototal)
	cpu_average_list[current] = (100 * (load - oload)) / (total - ototal);
    else
	cpu_average_list[current] = (load - oload);

    current++;
    if (current == CPUSMOOTHNESS)
	current = 0;

    oload = load;
    ototal = total;

    if (firsttimes != CPUSMOOTHNESS)
	return 0;

    cpuload = 0;

    for (i = 0; i < CPUSMOOTHNESS; i++)
	cpuload += cpu_average_list[i];
    return (cpuload / CPUSMOOTHNESS);
	
}

