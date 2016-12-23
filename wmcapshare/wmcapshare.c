/*********************************************************************
 *
 * Filename:      wmcapshare.c
 * Version:       0.1
 * Description:   Window Maker Dockapp to Control HP Capshare
 * Status:        Development
 * Author:        Ben Moore <ben@netjunki.org>
 * Created at:    Sun Oct 06 10:00:00 2001
 * Modified at:   Sun Oct 06 10:00:00 2001
 * Modified by:   Ben Moore <ben@netjunki.org>
 *
 *     Copyright (c) 2001 Ben Moore, All Rights Reserved.
 *
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of
 *     the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *     MA 02111-1307 USA
 *
 *
 *
 *     Start with --help to see available options
 *     By default captured pages are put in /tmp
 *
 ********************************************************************/

#include <stdio.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include <openobex/obex.h>
#include "obex_put_common.h"
#include "obex_io.h"

#include <dockapp.h>
#include "xpm/base.xpm"
#include "xpm/tstatfld.xpm"
#include "xpm/capshare.xpm"
#include "xpm/connected.xpm"
#include "xpm/disconnected.xpm"
#include "xpm/transfering.xpm"
#include "xpm/lcdchars2.xpm"
#include "xpm/lcdnums.xpm"
#include "xpm/plane00.xpm"
#include "xpm/plane01.xpm"
#include "xpm/plane02.xpm"
#include "xpm/plane03.xpm"
#include "xpm/plane04.xpm"
#include "xpm/plane05.xpm"
#include "xpm/plane06.xpm"
#include "xpm/plane07.xpm"
#include "xpm/plane08.xpm"
#include "xpm/plane09.xpm"
#include "xpm/plane10.xpm"
#include "xpm/plane11.xpm"
#include "xpm/plane12.xpm"
#include "xpm/plane13.xpm"
#include "xpm/plane14.xpm"
#include "xpm/plane15.xpm"
#include "xpm/plane16.xpm"
#include "xpm/plane17.xpm"
#include "xpm/plane18.xpm"
#include "xpm/plane19.xpm"

#include "wmcapshare.h"
#include "version.h"

char *displayName = "";
gchar *savedir = "/tmp/";
GC gc;
Pixmap pixmap, mask;
Pixmap tstatfld_buf, tstatfld_mask;
Pixmap blank_buf, blank_mask;
Pixmap capshare_pic, capshare_mask;
Pixmap plane_anim[20], plane_animmask[20];
Pixmap connect_pic, connect_mask;
Pixmap disconnect_pic, disconnect_mask;
Pixmap transfer_pic, transfer_mask;
Pixmap char_buf, char_mask;
Pixmap num_buf, num_mask;

obex_t *handle = NULL;
volatile int finished = FALSE;
volatile int state = 0;
int planeanimpos = 0;

static DAProgramOption options[] = {
  {"-d", "--dir", "Directory to save to", DOString, False, {&savedir}}
}
;



int charpos(char c) {
   int cn, cai;
   cn = c - 97;
   cai = cn*6;
   return cai;
}

int string2pixmap(char* str, Pixmap *p) {
   int x;
   XCopyArea(DADisplay, blank_buf, *p, gc, 0,0, 42,  8,  0,  0);
   for (x = 0; x < strlen(str); x++) {
      XCopyArea(DADisplay, char_buf, *p, gc, charpos(str[x]),0, 6,  8,  x*6,  0);
   }

}

void updateDisplay() {
   switch(state) {
    case CAP_WAIT: /* wait */
      planeanimpos=0;
      XCopyArea(DADisplay, capshare_pic, pixmap, gc, 0, 0, 60, 29,  0, 15);
      XCopyArea(DADisplay, disconnect_pic, pixmap, gc, 0, 0, 14, 15,  0, 45);
      string2pixmap("waiting", &tstatfld_buf);
      XCopyArea(DADisplay, tstatfld_buf, pixmap, gc, 0, 0, 42,  8, 16, 48);
      DASetPixmap(pixmap);
      break;
    case CAP_RECIEVE: /* recieve */
      XCopyArea(DADisplay, plane_anim[planeanimpos], pixmap, gc, 0, 0, 60, 29, 0, 15);
      planeanimpos++;
      if(planeanimpos==20)
	planeanimpos = 0;
      XCopyArea(DADisplay, connect_pic, pixmap, gc, 0, 0, 14, 15,  0, 45);
      string2pixmap("recieve", &tstatfld_buf);
      XCopyArea(DADisplay, tstatfld_buf, pixmap, gc, 0, 0, 42,  8, 16, 48);
      DASetPixmap(pixmap);
      break;
    case CAP_CONNECT:
      /* connect */
      XCopyArea(DADisplay, disconnect_pic, pixmap, gc, 0, 0, 14, 15,  0, 45);
      string2pixmap("connect", &tstatfld_buf);
      XCopyArea(DADisplay, tstatfld_buf, pixmap, gc, 0, 0, 42,  8, 16, 48);
      DASetPixmap(pixmap);
      break;
   }


}

/*
 * Function main (argc, )
 *
 *    Starts all the fun!
 *
 */
int main(int argc, char *argv[])
{
   obex_object_t *object;
   int ret;
   unsigned height,width;
   int i;

   DAParseArguments(argc, argv, options,
		    sizeof(options)/sizeof(DAProgramOption),
		    "CapShare Document Management Dock Applet\nBen Moore ben@netjunki.org\n",
		    VERSION);


   DAInitialize(displayName, "wmcapshare", 60,60, argc, argv);
   DAMakePixmapFromData(lcdchars2_xpm, &char_buf, &char_mask, &height, &width);
   DAMakePixmapFromData(lcdnums_xpm, &num_buf, &num_mask, &height, &width);
   DAMakePixmapFromData(base_xpm, &pixmap, &mask, &height, &width);
   DAMakePixmapFromData(tstatfld_xpm, &tstatfld_buf, &tstatfld_mask, &height, &width);
   DAMakePixmapFromData(tstatfld_xpm, &blank_buf, &blank_mask, &height, &width);
   DAMakePixmapFromData(capshare_xpm, &capshare_pic, &capshare_mask, &height, &width);
   DAMakePixmapFromData(transfering_xpm, &transfer_pic, &transfer_mask, &height, &width);
   DAMakePixmapFromData(connected_xpm, &connect_pic, &connect_mask, &height, &width);
   DAMakePixmapFromData(disconnected_xpm, &disconnect_pic, &disconnect_mask, &height, &width);
   DAMakePixmapFromData( plane00_xpm, &plane_anim[0], &plane_animmask[1], &height, &width);
   DAMakePixmapFromData( plane01_xpm, &plane_anim[1], &plane_animmask[2], &height, &width);
   DAMakePixmapFromData( plane02_xpm, &plane_anim[2], &plane_animmask[3], &height, &width);
   DAMakePixmapFromData( plane03_xpm, &plane_anim[3], &plane_animmask[4], &height, &width);
   DAMakePixmapFromData( plane04_xpm, &plane_anim[4], &plane_animmask[5], &height, &width);
   DAMakePixmapFromData( plane05_xpm, &plane_anim[5], &plane_animmask[6], &height, &width);
   DAMakePixmapFromData( plane06_xpm, &plane_anim[6], &plane_animmask[6], &height, &width);
   DAMakePixmapFromData( plane07_xpm, &plane_anim[7], &plane_animmask[7], &height, &width);
   DAMakePixmapFromData( plane08_xpm, &plane_anim[8], &plane_animmask[8], &height, &width);
   DAMakePixmapFromData( plane09_xpm, &plane_anim[9], &plane_animmask[9], &height, &width);
   DAMakePixmapFromData( plane10_xpm, &plane_anim[10], &plane_animmask[10], &height, &width);
   DAMakePixmapFromData( plane11_xpm, &plane_anim[11], &plane_animmask[11], &height, &width);
   DAMakePixmapFromData( plane12_xpm, &plane_anim[12], &plane_animmask[12], &height, &width);
   DAMakePixmapFromData( plane13_xpm, &plane_anim[13], &plane_animmask[13], &height, &width);
   DAMakePixmapFromData( plane14_xpm, &plane_anim[14], &plane_animmask[14], &height, &width);
   DAMakePixmapFromData( plane15_xpm, &plane_anim[15], &plane_animmask[15], &height, &width);
   DAMakePixmapFromData( plane16_xpm, &plane_anim[16], &plane_animmask[16], &height, &width);
   DAMakePixmapFromData( plane17_xpm, &plane_anim[17], &plane_animmask[17], &height, &width);
   DAMakePixmapFromData( plane18_xpm, &plane_anim[18], &plane_animmask[18], &height, &width);
   DAMakePixmapFromData( plane19_xpm, &plane_anim[19], &plane_animmask[19], &height, &width);

   gc = DefaultGC(DADisplay, DefaultScreen(DADisplay));


/*
 string2pixmap("recieve", &tstatfld_buf);
 XCopyArea(DADisplay, plane_pic, pixmap, gc, 0, 0, 60, 29, 0, 15);
 XCopyArea(DADisplay, tstatfld_buf, pixmap, gc, 0, 0, 42,  8, 16, 48);
 DASetPixmap(pixmap);
 XCopyArea(DADisplay, connect_pic, pixmap, gc, 0, 0, 14, 15, 0, 45);
 DASetPixmap(pixmap);
 */

   XCopyArea(DADisplay, capshare_pic, pixmap, gc, 0, 0, 60, 29,  0, 15);
   DAShow();

   while(1) {
      handle = OBEX_Init(OBEX_TRANS_IRDA, obex_event, 0);

      printf("Waiting for files\n");
      OBEX_ServerRegister(handle, "OBEX");

      while (!finished) {
	 updateDisplay();
	 OBEX_HandleInput(handle, 1);
      }

      finished = FALSE;
   }

   return 0;
}
