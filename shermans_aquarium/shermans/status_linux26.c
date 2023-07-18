/* 

 Routines for fetching useful system information 

 Linux 2.6 only.

 Written by Jonas Aaberg <cja@gmx.net> September 2003.
 Updated for Linux 2.6 August 2004.

 Released under GPL.

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <glob.h>
#include <gtk/gtk.h>

#include "status.h"

static char *status_sensors_dir = NULL;


static char *status_sensors_get_dir(void)
{
    char *chip_dir = NULL, tmp[1024];
    FILE *testfile;
    glob_t globbuf;
    int i;

    glob("/sys/bus/i2c/devices/*",GLOB_NOSORT, NULL, &globbuf);

    for(i=0;i<globbuf.gl_pathc;i++){
	g_snprintf(tmp,1024,"%s/temp1_input", globbuf.gl_pathv[i]);
	testfile = fopen(tmp, "r");
	if(testfile != NULL){
	    chip_dir = g_strdup(globbuf.gl_pathv[i]);
	    break;
	}
    }
    globfree(&globbuf);
    return chip_dir;
}

int status_sensors(int sensor_type)
{
    int status = -1;
    FILE *status_file;
    char chip_buff[1024];


    if(sensor_type == SENSORS_TEMP1 || sensor_type == SENSORS_TEMP2 || sensor_type == SENSORS_TEMP3 || sensor_type == SENSORS_TEMP4)
        sprintf(chip_buff,"%s/temp%d_input",status_sensors_dir, sensor_type);

    if(sensor_type == SENSORS_FAN1 || sensor_type == SENSORS_FAN2 || sensor_type == SENSORS_FAN3)
        sprintf(chip_buff,"%s/fan%d_input",status_sensors_dir, sensor_type - SENSORS_TEMP4);
    
    
    status_file = fopen(chip_buff,"r");
    if(status_file == NULL)
	return status;

    fscanf(status_file, "%d", &status);
    fclose(status_file);

    return status;


}

int status_swap(void)
{
    FILE *swap_file;
    int swap_total, swap_used;

    swap_file = fopen("/proc/swaps", "r");

    fscanf(swap_file, "%*s %*s %*s %*s %*s");
    fscanf(swap_file, "%*s %*s %d %d %*d",
	   &swap_total, &swap_used);
    fclose(swap_file);

    return (int)(100*swap_used/swap_total);
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
    int mem_total, mem_free, mem_buffers, mem_cached;

    mem_file = fopen("/proc/meminfo", "r");

    fscanf(mem_file,"MemTotal: %d kB\n",&mem_total);
    fscanf(mem_file,"MemFree: %d kB\n", &mem_free);
    fscanf(mem_file,"Buffers: %d kB\n", &mem_buffers);
    fscanf(mem_file,"Cached: %d kB\n", &mem_cached);

    fclose(mem_file);
    return (int) (100*(mem_total - (mem_free + mem_buffers + mem_cached))/mem_total);
}

int status_net(int type, int direction)
{
    FILE *net_file;
    char dev[256]; /* dummy[256], */

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
    //    fgets(dummy,256,net_file);
    //    fgets(dummy,256,net_file);

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


void status_exit(void)
{
    if(status_sensors_dir != NULL)
	g_free(status_sensors_dir);
    status_sensors_dir = NULL;
}

void status_init(void)
{
    status_exit();
    status_sensors_dir = status_sensors_get_dir();
}
