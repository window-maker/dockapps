/* ===========================================================================
 * AScd: mixer_handlers.c
 * mouse events <-> integrated commands with mixer module
 * ===========================================================================
 * Copyright (c) 1999 Denis Bourez and Rob Malda. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Denis Bourez & Rob Malda
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DENIS BOUREZ AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL DENIS BOUREZ, ROB MALDA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 * ===========================================================================
 */
 								   
#include "ext.h"

#ifdef MIXER

int check_mixer()
{
    int fd;
    if ((fd = open(mixer_device,0))) {
        close(fd);
    } else {
        return FALSE;
    }
    return TRUE;
}

int getvol(int control)
{
    int fd;
    int status = 0;

    if (mixer_ok) {
        if ((fd = open(mixer_device,0))) {
            ioctl(fd, MIXER_READ(control), &mixer_vol);
            mixer_vol = mixer_vol >> 8;
            close(fd);
	    return mixer_vol;
        }
    }
    return 0;
}

void setvol(int control, int setto, int maximum)
{
    int fd;

    if (setto > maximum) setto = maximum;
    if (setto < 0) setto = 0;

    if ((fd = open(mixer_device, 0))) {
        mixer_vol = ((float)setto / (float)maximum) * 100;
        mixer_old = mixer_vol << 8;
        mixer_vol = mixer_old | mixer_vol;
        ioctl(fd, MIXER_WRITE(control), &mixer_vol);
        close(fd);
    }
}

void mixer_event_handle(int event, XEvent Event)
{
    int value;

    if (!mixer_ok) return;

    switch (event) {
    case FAK_MIXER_SET:
	if ((thdata[but_current].xpm.attributes.width > 0) && (max_volume > 0)) {
	    if (thdata[but_current].type == FAK_MIXER_BAR) {
		value = (int)((float)(Event.xbutton.x - thdata[but_current].x) / (float)thdata[but_current].xpm.attributes.width * 100.0);
	    } else if (thdata[but_current].type == FAK_VMIXER_BAR) {
		value = (int)((float)(Event.xbutton.y - thdata[but_current].y) / (float)thdata[but_current].xpm.attributes.height * 100.0);
	    } else {
		value = (int)((float)(Event.xbutton.y - thdata[but_current].y) / (float)thdata[but_current].xpm.attributes.height * 100.0);
		value = 100 - value;
	    }
	    setvol(thdata[but_current].arg, value, 100);
	    redraw = TRUE;
	    fak_redraw();
	}
	break;
    case FAK_MIXER_0:
	setvol(thdata[but_current].arg, 0, 100);
	redraw = TRUE;
	fak_redraw();
	break;
    case FAK_MIXER_50:
	setvol(thdata[but_current].arg, 50, 100);
	redraw = TRUE;
	fak_redraw();
	break;
    case FAK_MIXER_75:
	setvol(thdata[but_current].arg, 75, 100);
	redraw = TRUE;
	fak_redraw();
	break;
    case FAK_MIXER_100:
	setvol(thdata[but_current].arg, 100, 100);
	redraw = TRUE;
	fak_redraw();
	break;
    default:
	break;
    }

    redraw = TRUE;
    fak_singlemask(but_current);
}

#endif
