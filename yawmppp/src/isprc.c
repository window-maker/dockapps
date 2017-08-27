/*

   YAWMPPP - PPP dock app/helper for WindowMaker
   Copyright (C) 2000:

   Authors: Felipe Bergo     (bergo@seul.org)

   contains code from the wmppp application by
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

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "isprc.h"

int
GetISPInfo(char *rcname,struct YAWMPPP_ISP_INFO *wii,int max)
{
  FILE *f;
  int num_isps;
  char temp[512],work[512];
  char *q;
  char *tokens=" :\n\t";
  char *tokens2="\n";

  memset(&wii[0],0,sizeof(struct YAWMPPP_ISP_INFO));

  f=fopen(rcname,"r");
  if (!f) return 0;

  num_isps=0;

  /* level 0 */
  while( fgets(temp,128,f) ) {
    strcpy(work,temp);
    q=work;
    q=strtok(q,tokens);
    if (!q) continue;
    if (!strcmp(q,"ISP.BEGIN")) {
      if (num_isps>=max) continue;

      /* enter ISP.BEGIN block */
      memset(&wii[num_isps],0,sizeof(struct YAWMPPP_ISP_INFO));

      wii[num_isps].ppp.override=0;
      wii[num_isps].ppp.noipdefault=1;
      wii[num_isps].ppp.noauth=1;
      wii[num_isps].ppp.passive=0;
      wii[num_isps].ppp.defaultroute=1;
      wii[num_isps].ppp.chap=AUTH_DONTCARE;
      wii[num_isps].ppp.pap=AUTH_DONTCARE;

      while( fgets(temp,128,f) ) {
	strcpy(work,temp);
	q=work;
	q=strtok(q,tokens);

	if (!strcmp(q,KEY_LONGNAME)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].LongName,q,128);
	  continue;
	}
	if (!strcmp(q,KEY_SHORTNAME)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].ShortName,q,16);
	  continue;
	}
	if (!strcmp(q,KEY_STARTACTION)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].StartAction,q,512);
	  continue;
	}
	if (!strcmp(q,KEY_STOPACTION)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].StopAction,q,512);
	  continue;
	}
	if (!strcmp(q,KEY_IFDOWNACTION)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].IfDownAction,q,512);
	  continue;
	}
	if (!strcmp(q,KEY_SPEEDACTION)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].SpeedAction,q,512);
	  continue;
	}
	if (!strcmp(q,KEY_PPPSTUFF)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].PPPLine,q,512);
	  continue;
	}
	if (!strcmp(q,KEY_CHATSTUFF)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].ChatFile,q,512);
	  continue;
	}
	if (!strcmp(q,KEY_USER)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].User,q,32);
	  continue;
	}
	if (!strcmp(q,KEY_PHONE)) {
	  q=strtok(NULL,tokens2);
	  if (q) strncpy(wii[num_isps].Phone,q,32);
	  continue;
	}
	/* ppp options per ISP */
	if (!strcmp(q,KEY_PPP_OVER)) {
	  q=strtok(NULL,tokens2);
	  if (q) wii[num_isps].ppp.override=atoi(q);
	  continue;
	}
	if (!strcmp(q,KEY_PPP_DEFAULTROUTE)) {
	  q=strtok(NULL,tokens2);
	  if (q) wii[num_isps].ppp.defaultroute=atoi(q);
	  continue;
	}
	if (!strcmp(q,KEY_PPP_PASSIVE)) {
	  q=strtok(NULL,tokens2);
	  if (q) wii[num_isps].ppp.passive=atoi(q);
	  continue;
	}
	if (!strcmp(q,KEY_PPP_NOAUTH)) {
	  q=strtok(NULL,tokens2);
	  if (q) wii[num_isps].ppp.noauth=atoi(q);
	  continue;
	}
	if (!strcmp(q,KEY_PPP_NOIPDEFAULT)) {
	  q=strtok(NULL,tokens2);
	  if (q) wii[num_isps].ppp.noipdefault=atoi(q);
	  continue;
	}
	if (!strcmp(q,KEY_PPP_CHAP)) {
	  q=strtok(NULL,tokens2);
	  if (q) wii[num_isps].ppp.chap=atoi(q);
	  continue;
	}
	if (!strcmp(q,KEY_PPP_PAP)) {
	  q=strtok(NULL,tokens2);
	  if (q) wii[num_isps].ppp.pap=atoi(q);
	  continue;
	}
	if (!strcmp(q,KEY_NOLOGIN)) {
	  q=strtok(NULL,tokens2);
	  if (q) wii[num_isps].nologin=atoi(q);
	  continue;
	}
	if (!strcmp(q,"ISP.END")) {
	  num_isps++;
	  break;
	}
      }
    }
  }
  fclose(f);
  return(num_isps);
}
