/*
 * Copyright (C) 2002 Sven Schaepe <schaepe@rz.tu-ilmenau.de>
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/version.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"

#include "wmsm-master.xpm"

#define v_WMSM "0.2.1"

/* scale */
#define xpm_scale_width		35
#define xpm_scale_height	7
/* type 1 */
#define xpm_scale_1_f_x		67
#define xpm_scale_1_f_y		5
#define xpm_scale_1_b_x		67
#define xpm_scale_1_b_y		13
/* type 2 */
#define xpm_scale_2_f_x		67
#define xpm_scale_2_f_y		21
#define xpm_scale_2_b_x		67
#define xpm_scale_2_b_y		29

/* digit */
#define digit_width		6
#define digit_height		7
/* green */
#define digit_1_x		5
#define digit_1_y		66
/* blue */
#define digit_2_x		5
#define digit_2_y		76

/* ativation point */
#define point_width		2
#define point_height		2
/* disable point */
#define point_disable_x		66
#define point_disable_y		37
/* enable point */
#define point_enable_x		68
#define point_enable_y		37
/* read position */
#define point_read_x		5
#define point_read_y		37
/* write position */
#define point_write_x		5
#define point_write_y		46

#define cpu_s_x			24
#define cpu_s_y			5
#define mem_s_x			24
#define mem_s_y			14
#define swap_s_x		24
#define swap_s_y		23
#define ior_s_x			24
#define ior_s_y			32
#define iow_s_x			24
#define iow_s_y			41

typedef struct {
    int cpu_idle;
    int cpu_use;
    long long r_diff_max;
    unsigned long long r_io;
    long long w_diff_max;
    unsigned long long w_io;
    int day;
    int hour;
    int min;
    int sec;
} LAST;
LAST last;

char wmsm_mask_bits[64*64];
int wmsm_mask_width = 64;
int wmsm_mask_height = 64;
int scale_width;
int scale_height;
int scale_f_x;
int scale_f_y;
int scale_b_x;
int scale_b_y;
int mem_correction;
char *dev_name;

static void draw_digit(int value,int dx,int dy)
{
    if(value>99) {
	copyXPMArea(9*digit_width+digit_2_x,digit_2_y,digit_width,digit_height,dx,dy);
	copyXPMArea(9*digit_width+digit_2_x,digit_2_y,digit_width,digit_height,dx+digit_width,dy);
    } else {
	copyXPMArea((value/10)*digit_width+digit_1_x,digit_1_y,digit_width,digit_height,dx,dy);
	copyXPMArea((value%10)*digit_width+digit_1_x,digit_1_y,digit_width,digit_height,dx+digit_width,dy);
    }
    RedrawWindowXY(0,0);
}

static void draw_uptime(int day,int hour,int min,int sec)
{
    if(sec!=last.sec) {
	draw_digit(sec,48,51);
	last.sec=sec;
	if(min!=last.min) {
	    draw_digit(min,34,51);
	    last.min=min;
	    if(hour!=last.hour) {
		draw_digit(hour,20,51);
		last.hour=hour;
		if(day!=last.day) {
		    draw_digit(day,5,51);
		    last.day=day;
		}
	    }
	}
    }
}

static void draw_enable_point(int dx,int dy)
{
    copyXPMArea(point_enable_x,point_enable_y,point_width,point_height,dx,dy);
    RedrawWindowXY(0,0);
}

static void draw_disable_point(int dx,int dy)
{
    copyXPMArea(point_disable_x,point_disable_y,point_width,point_height,dx,dy);
    RedrawWindowXY(0,0);
}

static void draw_scale(int width,int dx,int dy)
{
    copyXPMArea(scale_f_x,scale_f_y,width,scale_height,dx,dy);
    if(width<scale_width) {
	copyXPMArea(scale_b_x+width,scale_b_y,scale_width-width,scale_height,dx+width,dy);
    }
    RedrawWindowXY(0,0);
}

static int getDiskValue24(unsigned long long *r_io,unsigned long long *w_io)
{
    FILE *file;
    char line[256];
    char tag[32];
    int major=0,disk=0,all_io=0,r_blk=0,w_blk=0;
    if((file=fopen("/proc/stat","r"))==NULL) {
	return -1;
    }
    while(fgets(line,sizeof(line),file)!=NULL) {
	sscanf(line,"%s",tag);
	if(strcmp(tag,"disk_io:")==0) {
	    sscanf(line,"%s (%d,%d):(%d,%llu,%d,%llu,%d)",tag,&major,&disk,&all_io
						    ,r_io,&r_blk,w_io,&w_blk);
	    fclose(file);
	    return 0;
	}
    }
    fclose(file);
    return -1;
}

static int getDiskValue26(unsigned long long *r_io,unsigned long long *w_io)
{
    FILE *file;
    char line[256];
    if((file=fopen("/proc/diskstats","r"))==NULL) {
	return -1;
    }
    while(fgets(line,sizeof(line),file)!=NULL) {
	int major,minor;
	char devName[40];
	unsigned int reads,readMerges,readTicks;
	unsigned int writes,writeMerges,writeTicks;
	unsigned int inFlight,ioTicks,timeInQueue;
	unsigned long long readSectors,writeSectors;
	if(sscanf(line,"%4d %4d %s %u %u %llu %u %u %u %llu %u %u %u %u",
	    &major,&minor,devName,&reads,&readMerges,&readSectors,&readTicks,
	    &writes,&writeMerges,&writeSectors,&writeTicks,&inFlight,&ioTicks,
	    &timeInQueue)==14) {
	    *r_io=readSectors;
	    *w_io=writeSectors;
		if(strcmp(devName,dev_name)==0) {
		    fclose(file);
		    return 0;
		}
	    }
    }
    fclose(file);
    return -1;
}

static void get_values(int init,int kernelVersion)
{
    FILE *file;
    char line[256];
    char tag[32];
    int user,nice,system,idle;
    unsigned long long r_io=0,w_io=0;
    int mem_total=0,mem_free=0,mem_cached=0,mem_buffers=0;
    int swap_total=0,swap_free=0;
    float uptime=0.0;

    file=fopen("/proc/stat","r");
    while(fgets(line,sizeof(line),file)!=NULL) {
	sscanf(line,"%s",tag);
	if(strcmp(tag,"cpu")==0) {
	    sscanf(line,"%s %d %d %d %d",tag,&user,&nice,&system,&idle);
	    break;
	}
    }
    fclose(file);
    if(kernelVersion<KERNEL_VERSION(2,6,0)) {
	getDiskValue24(&r_io,&w_io);
    } else {
	getDiskValue26(&r_io,&w_io);
    }
    file=fopen("/proc/meminfo","r");
    while(fgets(line,sizeof(line),file)!=NULL) {
	sscanf(line,"%s",tag);
	if(strcmp(tag,"MemTotal:")==0) {
	    sscanf(line,"%s %d",tag,&mem_total);
	    continue;
	}
	if(strcmp(tag,"MemFree:")==0) {
	    sscanf(line,"%s %d",tag,&mem_free);
	    continue;
	}
	if(strcmp(tag,"Cached:")==0) {
	    sscanf(line,"%s %d",tag,&mem_cached);
	    continue;
	}
	if(strcmp(tag,"Buffers:")==0) {
	    sscanf(line,"%s %d",tag,&mem_buffers);
	    continue;
	}
	if(strcmp(tag,"SwapTotal:")==0) {
	    sscanf(line,"%s %d",tag,&swap_total);
	    continue;
	}
	if(strcmp(tag,"SwapFree:")==0) {
	    sscanf(line,"%s %d",tag,&swap_free);
	    break;
	}
    }
    fclose(file);

    file=fopen("/proc/uptime","r");
    fscanf(file,"%f",&uptime);
    fclose(file);

    if(init) {
	last.cpu_idle=idle;
	last.cpu_use=user+nice+system;
	last.r_diff_max=1;
	last.r_io=r_io;
	last.w_diff_max=1;
	last.w_io=w_io;
    } else {
	long long diff;
	long long diff1;
	int width;

	/* cpu */
	diff=idle-last.cpu_idle;
	diff1=user+nice+system-last.cpu_use;
	width=diff1*scale_width/(diff+diff1+1);
	last.cpu_idle=idle;
	last.cpu_use=user+nice+system;
	draw_scale(width,cpu_s_x,cpu_s_y);

	/* r_io */
	diff=r_io-last.r_io;
	if(diff>last.r_diff_max) {
	    last.r_diff_max=diff;
	}
	width=abs(diff)*scale_width/last.r_diff_max;
	last.r_io=r_io;
	draw_scale(width,ior_s_x,ior_s_y);
	if(diff && !width) {
	    draw_enable_point(point_read_x,point_read_y);
	} else {
	    draw_disable_point(point_read_x,point_read_y);
	}

	/* w_io */
	diff=w_io-last.w_io;
	if(diff>last.w_diff_max) {
	    last.w_diff_max=diff;
	}
	width=abs(diff)*scale_width/last.w_diff_max;
	last.w_io=w_io;
	draw_scale(width,iow_s_x,iow_s_y);
	if(diff && !width) {
	    draw_enable_point(point_write_x,point_write_y);
	} else {
	    draw_disable_point(point_write_x,point_write_y);
	}

	/* mem */
	if(!mem_total) {
	    width=0;
	} else {
	    if(mem_correction) {
		width=(mem_total-mem_free-mem_cached-mem_buffers)*scale_width/mem_total;
	    } else {
		width=(mem_total-mem_free)*scale_width/mem_total;
	    }
	}
	draw_scale(width,mem_s_x,mem_s_y);

	/* swap */
	if(!swap_total) {
	    width=0;
	} else {
	    width=(swap_total-swap_free)*scale_width/swap_total;
	}
	draw_scale(width,swap_s_x,swap_s_y);

	/* uptime */
	{
	    int day,hour,min,sec;

	    day=(int)uptime/(24*3600);
	    uptime-=day*24*3600;
	    hour=(int)uptime/3600;
	    uptime-=hour*3600;
	    min=(int)uptime/60;
	    uptime-=min*60;
	    sec=uptime;
	    draw_uptime(day,hour,min,sec);
	}
    }
}

static void usage(char *progname)
{
    fprintf(stdout,"%s-%s "
		    "WindowMaker System Monitor\n"
		    "\t-t <num>	scale type num={1,2,3,4}\n"
		    "\t-d <dev>	device name without partion number, default: hda\n"
		    "\t-m 		mem correction\n"
		    "\t-h		this\n"
		    ,progname,v_WMSM);
}

int main(int argc,char *argv[])
{
    FILE *file;
    char line[256];
    char xx[256];
    int revision=0,release=0,patchlevel=0;
    XEvent event;
    int opt;

    scale_width=xpm_scale_width;
    scale_height=xpm_scale_height;
    scale_f_x=xpm_scale_2_f_x;
    scale_f_y=xpm_scale_2_f_y;
    scale_b_x=xpm_scale_1_b_x;
    scale_b_y=xpm_scale_1_b_y;
    mem_correction=0;
    dev_name="hda";

    while((opt=getopt(argc,argv,"hmt:d:"))!=-1) {
	switch(opt) {
	    case 'd':
		dev_name=optarg;
		break;
	    case 'm':
		mem_correction=1;
		break;
	    case 't':
		switch(atoi(optarg)) {
		    case 4:
			scale_f_x=xpm_scale_2_f_x;
			scale_f_y=xpm_scale_2_f_y;
			scale_b_x=xpm_scale_2_b_x;
			scale_b_y=xpm_scale_2_b_y;
			break;
		    case 3:
			scale_f_x=xpm_scale_1_f_x;
			scale_f_y=xpm_scale_1_f_y;
			scale_b_x=xpm_scale_1_b_x;
			scale_b_y=xpm_scale_1_b_y;
			break;
		    case 2:
			scale_f_x=xpm_scale_1_f_x;
			scale_f_y=xpm_scale_1_f_y;
			scale_b_x=xpm_scale_2_b_x;
			scale_b_y=xpm_scale_2_b_y;
			break;
		    case 1:
		    default:
			scale_f_x=xpm_scale_2_f_x;
			scale_f_y=xpm_scale_2_f_y;
			scale_b_x=xpm_scale_1_b_x;
			scale_b_y=xpm_scale_1_b_y;
		}
		break;
	    case 'h':
	    default:
		usage(argv[0]);
		exit(0);
	}
    }

    file=fopen("/proc/version","r");
    fgets(line,sizeof(line),file);
    sscanf(line,"%s %s %d.%d.%d%s",xx,xx,&revision,&release,&patchlevel,xx);
    fclose(file);

    createXBMfromXPM(wmsm_mask_bits,wmsm_master_xpm,wmsm_mask_width,wmsm_mask_height);
    openXwindow(argc,argv,wmsm_master_xpm,wmsm_mask_bits,wmsm_mask_width,wmsm_mask_height);
    RedrawWindowXY(0,0);

    get_values(1,(revision<<16)+(release<<8)+patchlevel);
    while(1) {
	while (XPending(display)) {
	    XNextEvent(display, &event);
	    switch (event.type) {
		case Expose:
		    RedrawWindowXY(0,0);
		    break;
		case DestroyNotify:
		    XCloseDisplay(display);
		    exit(0);
	    }
	}
	get_values(0,(revision<<16)+(release<<8)+patchlevel);
	usleep(150000L);
    }
}
