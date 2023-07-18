
/*

This piece of code is heavily based upon some code in timecop's <timecop@japan.co.jp> 
"BubbleMon  dockapp 1.2" The FreeBSD code there is orignally by oleg dashevskii 
<od@iclub.nsu.ru> and changed by Jonas Aaberg <cja@gmx.net to suit Sherman's aquarium.

Thanks goes out to bodnar istvan <bistvan@sliced.hu> for doing the FreeBSD testing for me!

*/

#include <kvm.h>
#include <sys/dkstat.h>
#include <sys/vmmeter.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <vm/vm_param.h>

#include "defines.h"
#include "aquarium.h"
#include "status.h"


#define pagetob(size) ((size) << pageshift)

static kvm_t *kd = NULL;
static struct nlist nlst[] = {
    {"_cp_time", 0},
    {"_cnt", 0},
    {"_bufspace", 0},
    {0, 0}
};
static int pageshift;
static int previous_total=0, previous_load=0;

static u_int64_t oload = 0, ototal = 0;
static int firsttimes = 0, current = 0;
static int cpu_average_list[CPUSMOOTHNESS];


static int status_init_cpu_load_freebsd(void)
{
    /* calculate page shift to convert pages into kilobytes */
    int pagesize = getpagesize();
    pageshift = 0;

    while (pagesize > 1) {
	pageshift++;
	pagesize >>= 1;
    }

    /* open kernel memory */
    kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open");

    if (kd == NULL) {
	puts("Could not open kernel virtual memory");
	return 1;
    }

    kvm_nlist(kd, nlst);

    if (nlst[0].n_type == 0 || nlst[1].n_type == 0 || nlst[2].n_type == 0) {
	puts("Error extracting symbols");
	return 2;
    }

    /* drop setgid & setuid (the latter should not be there really) */
    seteuid(getuid());
    setegid(getgid());

    if (geteuid() != getuid() || getegid() != getgid()) {
	puts("Unable to drop privileges");
	return 3;
    }

    return 0;
}

void status_init(void)
{
    status_init_cpu_load_freebsd();
}

void status_exit(void)
{
    /* I guess it should actually CPU be closed, but I don't know how.*/
}

int status_cpu(void)
{

/* Returns the current CPU load in percent */

    int loadPercentage;
    int total, load;
    unsigned long int cpu_time[CPUSTATES];
    int i, cpuload;


    if (firsttimes == 0) {
	for (i = 0; i < CPUSMOOTHNESS; i++)
	    cpu_average_list[i] = 0;
    }
    /* Wait until we have CPUSMOOTHNESS messures */
    if (firsttimes != CPUSMOOTHNESS)
	firsttimes++;


    if (kvm_read(kd, nlst[0].n_value, &cpu_time, sizeof(cpu_time))
	!= sizeof(cpu_time))
	return 0;

    load = cpu_time[CP_USER] + cpu_time[CP_SYS] + cpu_time[CP_NICE];
    total = load + cpu_time[CP_IDLE];


    if(total!=previous_total)
	cpu_average_list[current] = (100 * (load - previous_load)) / (total - previous_total);
    else
	cpu_average_list[current] = (load - previous_load);
	
    current++;
    if (current == CPUSMOOTHNESS)
	current = 0;

    previous_load = load;
    previous_total = total;

    if (firsttimes != CPUSMOOTHNESS)
	return 0;

    cpuload = 0;

    for (i = 0; i < CPUSMOOTHNESS; i++)
	cpuload += cpu_average_list[i];
    return (cpuload / CPUSMOOTHNESS);

}

}

int status_sensors(int, int)
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

