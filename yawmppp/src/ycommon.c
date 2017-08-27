/*

   YAWMPPP - PPP dock app/helper for WindowMaker
   Copyright (C) 2000,2001:

   Author:  Felipe Bergo     (bergo@seul.org)

   based on the wmppp application by

            Martijn Pieterse (pieterse@xs4all.nl)
            Antoine Nulle    (warp@xs4all.nl)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: ycommon.c,v 1.1.1.1 2001/02/22 07:15:59 bergo Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <sys/socket.h>

#ifdef LINUX
#define _LINUX_SOCKET_H
#endif

/* guessworks for critters beyond the realm of Linux and FreeBSD */

#ifdef OPENBSD
#define FREEBSD
#endif

#ifdef NETBSD
#define FREEBSD
#endif

#ifdef BSDISH
#define FREEBSD
#endif

#ifdef LINUX
  #include <asm/types.h>
  #include <linux/if.h>
  #include <linux/ppp_defs.h>
  #include <linux/if_ppp.h>
#endif

#ifdef FREEBSD
  #include <net/if.h>
  #include <net/ppp_defs.h>
  #include <net/if_ppp.h>
#endif

#include "ycommon.h"
#include "isprc.h"


extern struct LogStruct logconn;

extern int ppp_h;
extern int ppp_open;

extern struct YAWMPPP_ISP_INFO IspData[MAX_ISPS];
extern int num_isps;
extern int current_isp;

extern char *active_interface;

/* LOGS = ~/.yawmppp2/log */

void
clean_guards(void)
{
  char *p;
  char temp[128],aux[128];
  FILE *f;

  p = getenv ("HOME");
  strcpy (temp, p);
  strcat (temp, "/.yawmppp2/.floatlog");
  f=fopen(temp,"r");
  if (!f)
    return; /* fine: no crash file */
  if (!fgets(aux,128,f)) return;
  if ((p=strtok(aux,"\n"))==NULL) return;
  logconn.start=atoi(p);
  if (!fgets(aux,128,f)) return;
  if ((p=strtok(aux,"\n"))==NULL) return;
  logconn.end=atoi(p);
  if (!fgets(aux,128,f)) return;
  if ((p=strtok(aux,"\n"))==NULL) return;
  logconn.status=atoi(p);
  if (!fgets(aux,128,f)) return;
  if ((p=strtok(aux,"\n"))==NULL) return;
  strcpy(logconn.longname,p);
  if (!fgets(aux,128,f)) return;
  if ((p=strtok(aux,"\n"))==NULL) return;
  strcpy(logconn.shortname,p);
  if (!fgets(aux,128,f)) return;
  if ((p=strtok(aux,"\n"))==NULL) return;
  strcpy(logconn.phone,p);
  if (!fgets(aux,128,f)) return;
  if ((p=strtok(aux,"\n"))==NULL) return;
  strcpy(logconn.user,p);
  fclose(f);
  logconn.status=2; /* CRASH */
  write_log();
}

void
make_guards(void)
{
  char *p;
  char temp[128];
  FILE *f;

  p = getenv ("HOME");
  strcpy (temp, p);
  strcat (temp, "/.yawmppp2/.floatlog");
  f=fopen(temp,"w");
  if (!f) return;

  logconn.end=time(NULL);
  fprintf(f,"%lu\n%lu\n%d\n",logconn.start,logconn.end,logconn.status);
  fprintf(f,"%s\n%s\n%s\n",logconn.longname,logconn.shortname,logconn.phone);
  fprintf(f,"%s\n",logconn.user);
  fclose(f);
}

void
write_log(void)
{
  char *p;
  char temp[128];
  FILE *f;
  int i,s;

  p = getenv ("HOME");
  strcpy (temp, p);
  strcat (temp, "/.yawmppp2/logfile");
  f=fopen(temp,"a");
  if (!f) return;

  s=strlen(logconn.phone);
  for(i=0;i<s;i++)
    if (logconn.phone[i]=='\t')
      logconn.phone[i]=' ';
  s=strlen(logconn.shortname);
  for(i=0;i<s;i++)
    if (logconn.shortname[i]=='\t')
      logconn.shortname[i]=' ';
  s=strlen(logconn.longname);
  for(i=0;i<s;i++)
    if (logconn.longname[i]=='\t')
      logconn.longname[i]=' ';
  s=strlen(logconn.user);
  for(i=0;i<s;i++)
    if (logconn.user[i]=='\t')
      logconn.user[i]=' ';

  fprintf(f,"%lu\t%lu\t%d\t%s\t%s\t%s\t%s\n",
	  logconn.start,
	  logconn.end,
	  logconn.status,
	  logconn.longname,
	  logconn.shortname,
	  logconn.phone,
	  logconn.user);
  fclose(f);

  chmod(temp,0600);

  p = getenv ("HOME");
  strcpy (temp, p);
  strcat (temp, "/.yawmppp2/.floatlog");
  unlink(temp);
}


void
warn_pref(void)
{
  char *p;
  char temp[128];
  FILE *f;

  p=getenv("HOME");
  strcpy (temp, p);
  strcat (temp, "/.yawmppp2/.delayedupdate");

  f=fopen(temp,"w");
  if (!f) return;
  fprintf(f,"This file shouldn't last more than a few millisecs...\n");
  fclose(f);
}

/* PPP */

void
open_ppp(void)
{
  /* Open the ppp device. */

  if (ppp_open)
    return;

  if ((ppp_h = socket (AF_INET, SOCK_DGRAM, 0)) >= 0)
    ppp_open = 1;
}

void
close_ppp(void)
{
  if (!ppp_open)
    return;
  close(ppp_h);
  ppp_open=0;
}

/* other stuff */

void
write_pid_file(void)
{
    char *p;
    char temp[128];
    FILE *f;

    p = getenv ("HOME");
    strcpy (temp, p);
    strcat (temp, "/.yawmppp2/yawmppp.pid");

    f=fopen(temp,"w");
    if (f) {
      fprintf(f,"%d",getpid());
      fclose(f);
    }
}

void
remove_pid_file(void)
{
    char *p;
    char temp[128];

    p = getenv ("HOME");
    strcpy (temp, p);
    strcat (temp, "/.yawmppp2/yawmppp.pid");

    unlink(temp);
}

void
make_config_dir(void)
{
  struct stat ss;
  char tmp[256];

  sprintf(tmp,"%s/.yawmppp2",getenv("HOME"));

  if (stat(tmp,&ss)<0)
    mkdir(tmp,0700);
}


void
grab_isp_info(int rof)
{
    char *p;
    char temp[128];

    p = getenv ("HOME");
    strcpy (temp, p);
    strcat (temp, "/.yawmppp2/yawmppprc");

    num_isps=GetISPInfo(temp,&IspData[0],MAX_ISPS);

    if ((!num_isps)&&(rof)) {
      run_pref_app();
    }
}

void
run_pref_app(void)
{
  if (!fork()) {
    execlp("yawmppp.pref","yawmppp.pref",NULL);
    execlp("/usr/local/bin/yawmppp.pref","yawmppp.pref",NULL);
    execlp("/usr/bin/yawmppp.pref","yawmppp.pref",NULL);
    exit(2);
  }
}

void
run_log_app(void)
{
  if (!fork()) {
    execlp("yawmppp.log","yawmppp.log",NULL);
    execlp("/usr/local/bin/yawmppp.log","yawmppp.log",NULL);
    execlp("/usr/bin/yawmppp.log","yawmppp.log",NULL);
    exit(2);
  }
}

/* essential */


/* get_statistics */

int
get_statistics (char *devname, long *ip, long *op, long *is, long *os)
{

    struct ifpppstatsreq req;
    struct ppp_stats ppp_cur;

    memset (&ppp_cur, 0, sizeof (ppp_cur));

    open_ppp();
    if (!ppp_open)
      return(-1);

memset(&req,0,sizeof(req));

#ifdef LINUX
req.stats_ptr=(caddr_t) & req.stats;
strcpy(req.ifr__name, active_interface);
#endif

#ifdef FREEBSD
strcpy(req.ifr_name, active_interface);
#endif

 if (ioctl(ppp_h,SIOCGPPPSTATS,&req) >= 0)
   ppp_cur=req.stats;

 *op = ppp_cur.p.ppp_opackets;
 *ip = ppp_cur.p.ppp_ipackets;
 *is = ppp_cur.p.ppp_ibytes;
 *os = ppp_cur.p.ppp_obytes;

 return 0;
}

/* stillonline */

int
stillonline (char *ifs)
{

#ifdef LINUX
    FILE *fp;
    char temp[128];
#endif
#ifdef FREEBSD
	struct ifreq req;
#endif
    int i=0;

#ifdef LINUX
    fp = fopen ("/proc/net/route", "r");
    if (fp)
      {
	  while (fgets (temp, 128, fp))
	    {
		if (strstr (temp, ifs))
		  {
		      i = 1;	/* Line is alive */
		  }
	    }
	  fclose (fp);
      }
#endif

#ifdef FREEBSD
	strcpy(req.ifr_name,active_interface);
	open_ppp();
	if (!ppp_open)
	  return 0;
	if (ioctl(ppp_h,SIOCGIFFLAGS,&req)>=0) {
		if (req.ifr_flags&IFF_UP)
		  i=1;
	}
#endif
	return i;
}
