
/*************************************************************************/
/* piece of code and pixmaps from                                        */
/*                                                                       */
/* wmspeedfreq ( Tom Kistner )                                           */
/* wmppp  ( Martijn Pieterse, Antoine Nulle )                            */
/* wmapkill (S.Rozange                                                   */
/*                                                                       */
/* This program is free software; you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation; either version 2, or (at your option)   */
/* any later version.                                                    */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program (see the file COPYING); if not, write to the  */
/*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,         */
/*  Boston, MA  02111-1307, USA                                          */
/*                                                                       */
/* you need libcpufreq for the libarary                                  */
/* and libcpufreq-dev for cpufreq.h                                      */
/*                                                                       */
/*                                                                       */
/*************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include <cpufreq.h>
#include <time.h>

#include <libdockapp/wmgeneral.h>
#include "wmcpufreq_master_1.xpm"
#include "wmcpufreq_master_2.xpm"
#include "wmcpufreq_master_3.xpm"
#include "wmcpufreq_mask_1.xbm"
#include "wmcpufreq_mask_2.xbm"
#include "wmcpufreq_mask_3.xbm"

#define DELAY 200000000    /*nano second */
#define WMCPUFREQ_VERSION "VERSION 3.0 2009/05/14 \n"
#define LN_PATH 40
#define MAX_CPU 4
pid_t execCommand(char *);
void show_mhz(unsigned long*,int);
void show_governor(char* );
void show_driver(char *);
int show_char(int);
static char **wm_xpm;
static unsigned char *wm_bits;
struct cpufreq_policy *policy;
int cpu=0;
unsigned long min,max,f_min,f_max;
int main(int argc, char *argv[])
{
   struct timespec ts,ts1;
   int i,ncpu;
   int idcpu;
   unsigned long freq[8];
   XEvent event;
   char gov[20];
   char drv[20],*ptr,*endptr;
   char prg[LN_PATH];
   ts.tv_sec=0;
   ts.tv_nsec=DELAY;
   prg[0]=0;
   idcpu=0;

   for(i=0;i<MAX_CPU;i++)
     freq[i]=0;
   if(argc >1)
     {
	for (i=1; i<=argc; i++)
	  {
	     if (!strcmp(argv[i], "-v"))
	       {
		  printf(WMCPUFREQ_VERSION);
		  exit(0);
	       }
	     if (!strcmp(argv[i], "-exe"))
	       {
		  if(strlen(argv[i+1]) < LN_PATH )
		    strcpy(prg,argv[i+1]);
		  break;
	       }
	     if (!strcmp(argv[i], "-cpuid"))
	       {
		 if(strlen(argv[i+1]) < LN_PATH )
		   idcpu=strtol(argv[i+1],&endptr,0);
		 printf("cpuid= %d \n",idcpu);
		 break;
	       }
	     printf("only -v, -exe, -cpuid supported \n");
	     exit(0);
	  }
     }

   /* basic checks */
   if ( idcpu < 0 )
     {
       printf("cpuid < 0 \n");
       exit(-1);
     }

   /* get driver name (guess all cpu have the same driver) */
   ptr=cpufreq_get_driver(cpu);
   if(!ptr)
     {
	printf("no driver found \n");
	exit(-1);
     }
   strcpy(drv,ptr);
   cpufreq_put_driver(ptr);


   /* get number of cpu (0=cpu0, 1=cpu1 ...) */

   ncpu=-1;

   for(i=0;i<MAX_CPU;i++)
     {
       if( cpufreq_cpu_exists(idcpu+i) ==0)
	 {
	   printf("cpuid %d found\n",idcpu+i);
	   ncpu=i;
	 }
     }

   switch ( ncpu ) {
   case -1:
     printf("no cpuid found \n");
     exit(-1);
   case 0:
     wm_xpm=wmcpufreq_master_xpm_1;
     wm_bits=wmcpufreq_mask_bits_1;
     break;
   case 1:
     wm_xpm=wmcpufreq_master_xpm_2;
     wm_bits=wmcpufreq_mask_bits_2;
     break;
   case 2:
     wm_xpm=wmcpufreq_master_3;
     wm_bits=wmcpufreq_mask_3_bits;
     break;
   case 3:
     wm_xpm=wmcpufreq_master_3;
     wm_bits=wmcpufreq_mask_3_bits;
     break;
   default:
     printf("no yet implemented: cpuid %d \n",ncpu);
     exit(-1);
     break;
   }

   /* guess every cpu has the same limits */
   if(cpufreq_get_hardware_limits(cpu, &f_min, &f_max))
     {
	printf("can't determine hardware limits \n");
	exit(-1);
     }
   openXwindow(argc,argv,
	       wm_xpm,
	       (char*)wm_bits,
	       wmcpufreq_mask_width,
               wmcpufreq_mask_height);
   while(1)
     {
	/* Process any pending X events */
	while(XPending(display))
	  {
	     XNextEvent(display, &event);
	     switch(event.type)
	       {
		case Expose:
		  RedrawWindow();
		  break;
		case ButtonPress:
		  if(strlen(prg))
		    execCommand(prg);
		  break;
		case ButtonRelease:
		  break;
	       }
	  }
	RedrawWindow();
	/* get info */
	for(i=0;i<=ncpu;i++)
	  freq[i]=cpufreq_get_freq_kernel(i+idcpu);
	policy=cpufreq_get_policy(cpu);
	strcpy(gov,policy->governor);
	max=policy->max;
	min=policy->min;
	cpufreq_put_policy(policy);
	/* show info */
	show_mhz(freq,ncpu);
	if (ncpu==0)
	  show_driver(drv);
	show_governor(gov);
	/* delay */
	nanosleep(&ts,&ts1);
     }
}
void show_driver( char *pt)
{
   int i,a,c;
   for(i=0;i<8;i++)
     {
	a=(int)pt[i];
        c=show_char(a);
	copyXPMArea(6+(6*c),64,6,9, 7+(6*i), 34);
     }
   RedrawWindow();
   return;
}
void show_governor(char *ptr)
{
   int i,a,c;
   for(i=0;i<8;i++)
     {
	a=(int)ptr[i];
	c=show_char(a);
	copyXPMArea(6+(6*c),78,6,9, 7+(6*i), 49);
     }
   RedrawWindow();
   return;
}
void show_mhz(unsigned long *kHz,int nu)
{
   int i,j,delta=0,ddelta=0,odelta=0;
   char buffer[5];
   memset(buffer,0,5);
   switch (nu) {
    case 0:
      snprintf(buffer, 5, "%4ld", (kHz[0] / 1000));
      for (i=0;i<4;i++)
      {
	 if (buffer[i] == ' ')
	 {
	    /* blank zero */
	    copyXPMArea(75, 93, 6, 9, 7+(7*i), 7);
	 }
	 else
	 {
	    /* a standard digit */
	    copyXPMArea(((buffer[i]-48)*7)+5,93, 6, 9, 7+(7*i), 7);
	 }
         /* update  speed bar */
        copyXPMArea(65, 18, 49, 9, 7, 18);
	         //          or x,y
		 //         //                  l,h
		 //         //                         dest x,y
        copyXPMArea(65, 40, ((kHz[0] - f_min) / ((f_max - f_min) / 49)), 9, 7, 18);
        RedrawWindow();
      }
      break;
    case 1:
      for(j=0;j<=nu;j++)
      {
	 snprintf(buffer, 5, "%4ld", (kHz[j] / 1000));

	 for (i=0;i<4;i++)
	 {
	     if (buffer[i] == ' ')
	     {
		/* blank zero */
		copyXPMArea(75, 93, 6, 9, 7+(7*i), 7+delta);
	     }
	     else
	     {
	        /* a standard digit */
		copyXPMArea(((buffer[i]-48)*7)+5, 93, 6, 9, 7+(7*i), 7+delta);
	     }
	  }
       /* update  speed bar */
       copyXPMArea(65, 18, 49, 4, 7, 18+delta);
       copyXPMArea(65, 40, ((kHz[j] - f_min) / ((f_max - f_min) / 49)), 4, 7, 18+delta);

       RedrawWindow();
       delta=19;
     }
     break;
    case 2:
      kHz[3]=0;
    case 3:
      odelta=14;
      ddelta=10;
      for(j=0;j<4;j++)
      {
	 snprintf(buffer, 5, "%4ld", (kHz[j] / 1000));

	 for (i=0;i<4;i++)
	 {
	     if (buffer[i] == ' ')
	     {
		/* blank zero */
		copyXPMArea(126,6+odelta*j ,6,7, 6+(6*i),5+ddelta*j);
	     }
	     else
	     {
	        /* a standard digit */
		copyXPMArea(((buffer[i]-48)*6)+66,6+odelta*j, 6,7, 6+(6*i), 5+ddelta*j);
	     }
	  }
	  /* clean sped bar */
          copyXPMArea(31,67,24,7,34,5+ddelta*j);
	  /* update spped bar */
       	 if( !(nu==2 && j==3) ) /* 3 cpu 4 bar nul */
	  copyXPMArea(6,67 , ((kHz[j] - f_min) / ((f_max - f_min) / 24)),7, 34,5+ddelta*j);
       RedrawWindow();
      }
      break;
   }
   return;
}
int show_char(int c)
{
   switch (c)
     {
      case 0x61 ... 0x7a:
	break;
      case 0x41 ... 0x5a:
	c=c+0x20;
	break;
      case 0x34:
	c=(int)'z'+3;
	break;
      case 0x2d:
	c='z'+4;
	break;
      default:
	c='z'+1;
     }
   c=c-0x61;
   return c;
}

