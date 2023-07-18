/* 

 Routines for fetching useful system information 

 Linux 2.4 only.

 Written by Jonas Aaberg <cja@gmx.net> September 2003.

 Released under GPL.

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <gtk/gtk.h>

#include "status.h"

static char *status_sensors_dir;

static char *status_sensors_get_dir(void)
{
    int foo;
    char *chip_name;
    FILE *chip_file;
    
    chip_file = fopen("/proc/sys/dev/sensors/chips","r");
    if(chip_file == NULL)
	return NULL;

    chip_name = g_malloc0(1024);	
    fscanf(chip_file,"%d %s\n",&foo, chip_name);
    fclose(chip_file);
    return chip_name;
}

int status_sensors(int sensor_type)
{
    int status1, status2;
    float status1f, status2f, status3f;
    FILE *status_file;
    char chip_buff[1024];

    if(sensor_type == SENSORS_TEMP1 || sensor_type == SENSORS_TEMP2)
        sprintf(chip_buff,"/proc/sys/dev/sensors/%s/temp%d",status_sensors_dir, sensor_type);
    if(sensor_type == SENSORS_FAN1 || sensor_type == SENSORS_FAN2)
        sprintf(chip_buff,"/proc/sys/dev/sensors/%s/fan%d",status_sensors_dir, sensor_type-2);
    
    
    status_file = fopen(chip_buff,"r");
    if(status_file == NULL)
	return -1;

    if(sensor_type == SENSORS_TEMP1 || sensor_type == SENSORS_TEMP2){
	fscanf(status_file,"%f %f %f\n", &status1f, &status2f, &status3f);
	fclose(status_file);
	return (int)status3f;
    }

    if(sensor_type == SENSORS_FAN1 || sensor_type == SENSORS_FAN2){
	fscanf(status_file,"%d %d\n", &status1, &status2);
	fclose(status_file);
    	return status2;
    }

    return -1;
}

int status_swap(void)
{
    FILE *swap_file;
    int size, used;
    char dummy[256];
    swap_file = fopen("/proc/swaps", "r");
    if(swap_file == NULL)
	return 0;
    fgets(dummy,256,swap_file);
    fscanf(swap_file,"%*s %*s %d %d", &size, &used);
    fclose(swap_file);
    return (int)((float)used/(float)size*100.0);
}	

int status_disc(char *disk)
{
    struct statfs stat_buf;
    
    if(statfs(disk, &stat_buf)!=0)
	return 0;
    return (100-(int)((float)stat_buf.f_bavail/(float)stat_buf.f_blocks*100.0));
}

/* returns current CPU load in percent, 0 to 100 */

int status_cpu(void)
{
    static int firsttimes = 0, current = 0;
    static int cpu_average_list[CPUSMOOTHNESS];
    static u_int64_t oload = 0, ototal = 0;

    unsigned int cpuload;
    u_int64_t load, total;
    u_int64_t ab, ac, ad, ae;
    FILE *stat;
    int i;

    stat = fopen("/proc/stat", "r");
    fscanf(stat, "%*s %Ld %Ld %Ld %Ld", &ab, &ac, &ad, &ae);
    fclose(stat);

    if (firsttimes == 0) {
	for (i = 0; i < CPUSMOOTHNESS; i++)
	    cpu_average_list[i] = 0;
    }
    /* Wait until we have CPUSMOOTHNESS messures */
    if (firsttimes != CPUSMOOTHNESS)
	firsttimes++;

    /* Find out the CPU load */
    /* user + sys = load
     * total = total */
    load = ab + ac + ad;	/* cpu.user + cpu.sys; */
    total = ab + ac + ad + ae;	/* cpu.total; */

    /* Calculates and average from the last CPUSMOOTHNESS messures */
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

int status_mem(void)
{
    FILE *mem_file;
    char dummy[256];
    unsigned int mem_total,mem_used,mem_free, mem_shared, mem_buffers, mem_cached;
    
    mem_file = fopen("/proc/meminfo","r");
    if(mem_file == NULL)
	return 0;
    fgets(dummy,256,mem_file);
    fscanf(mem_file,"%*s %d %d %d %d %d %d\n",
	   &mem_total,&mem_used,&mem_free,&mem_shared,&mem_buffers,&mem_cached);
    fclose(mem_file);
    return (int)((float)(mem_used-mem_shared-mem_buffers-mem_cached)/(float)mem_total*100.0);
    

}

int status_net(int type, int direction)
{
    FILE *net_file;
    char dummy[256], dev[256];

    char devices[][10] = {{"lo:"},
		      {"eth0:"},
		      {"eth1:"},
		      {"ppp0:"}};

    long int recv_bytes=0, sent_bytes=0, work_at=0;
    int diff;

    static long int old_trans[NET_Y][NET_X];
    static int first_time = 1;

    if(first_time){
	memset(&old_trans,0,NET_Y*NET_X*sizeof(long int));
	first_time = 0;
    }


    net_file = fopen("/proc/net/dev","r");
    if(net_file == NULL)
	return 0;
    fgets(dummy,256,net_file);
    fgets(dummy,256,net_file);

    while(!feof(net_file)){
	fscanf(net_file,"%s "
               "%ld %*d %*d %*d "
               "%*d %*d %*d %*d "
	       "%ld %*d %*d %*d "
               "%*d %*d %*d %*d\n",
	       dev,
	       &recv_bytes,
	       &sent_bytes);
	if(!strcmp(devices[type],dev)){
	    if(direction == NET_RECV)
		work_at = recv_bytes;
	    else
		work_at = sent_bytes;
	    break;
	}
    }
    fclose(net_file);

    diff = (int)(work_at - old_trans[type][direction]);

    if(old_trans[type][direction] == 0)
	diff = 0;

    old_trans[type][direction] = work_at;

    return diff;
}

void status_init(void)
{
    status_sensors_dir= status_sensors_get_dir();
}

void status_exit(void)
{
    g_free(status_sensors_dir);
}
