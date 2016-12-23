/*
 * Copyright (C) 12 Jun 2003 Tomas Cermak
 *
 * This file is part of wmradio program.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <linux/videodev.h>

char *device = "/dev/radio";

int get_freq_fact(int fd)
{
    struct video_tuner tuner;

    tuner.tuner = 0;
    if (ioctl (fd, VIDIOCGTUNER, &tuner) < 0)
        return 16;
    if ((tuner.flags & VIDEO_TUNER_LOW) == 0)
        return 16;
    return 16000;
}

int radio_setfreq(int fd, int freq)
{
    int ifreq = (freq/100.0+1.0/32.0)*get_freq_fact(fd);
    return ioctl(fd, VIDIOCSFREQ, &ifreq);
}

void radio_unmute(int fd)
{
    struct video_audio vid_aud;

    if (ioctl(fd, VIDIOCGAUDIO, &vid_aud))
        perror("VIDIOCGAUDIO");
    if (vid_aud.volume == 0)
        vid_aud.volume = 65535;
    vid_aud.flags &= ~VIDEO_AUDIO_MUTE;
    if (ioctl(fd, VIDIOCSAUDIO, &vid_aud))
        perror("VIDIOCSAUDIO");
}

void radio_mute(int fd)
{
    struct video_audio vid_aud;

    if (ioctl(fd, VIDIOCGAUDIO, &vid_aud))
        perror("VIDIOCGAUDIO");
    vid_aud.flags |= VIDEO_AUDIO_MUTE;
    if (ioctl(fd, VIDIOCSAUDIO, &vid_aud))
        perror("VIDIOCSAUDIO");
}

int radio_getstereo(int fd)
{
    struct video_audio va;
    va.mode=-1;
    if(ioctl(fd, VIDIOCGAUDIO, &va) < 0) return 0;
    return va.mode == VIDEO_SOUND_STEREO ? 1 : 0;
}

int radio_getsignal(int fd)
{
    struct video_tuner vt;
    int signal;

    memset(&vt,0,sizeof(vt));
    ioctl (fd, VIDIOCGTUNER, &vt);
    signal=vt.signal>>13;

    return signal;
};

