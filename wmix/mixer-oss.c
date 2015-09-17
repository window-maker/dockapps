/* WMix 3.0 -- a mixer using the OSS mixer API.
 * Copyright (C) 2000, 2001
 *	Daniel Richard G. <skunk@mit.edu>,
 *	timecop <timecop@japan.co.jp>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "include/common.h"
#include "include/misc.h"
#include "include/mixer-oss.h"

#define WMVOLUME_CHANNEL_NAMES \
	"Master volume", \
	"Bass", \
	"Treble", \
	"FM Synth volume", \
	"PCM Wave volume", \
	"PC Speaker", \
	"Line In level", \
	"Microphone level", \
	"CD volume", \
	"Recording monitor", \
	"PCM Wave 2 volume", \
	"Recording volume", \
	"Input gain", \
	"Output gain", \
	"Line In 1", \
	"Line In 2", \
	"Line In 3", \
	"Digital In 1", \
	"Digital In 2", \
	"Digital In 3", \
	"Phone input", \
	"Phone output", \
	"Video volume", \
	"Radio volume", \
	"Monitor volume"

#ifdef OSS_CHANNEL_NAMES
#define CHANNEL_NAMES SOUND_DEVICE_LABELS
#else
#define CHANNEL_NAMES WMVOLUME_CHANNEL_NAMES
#endif

typedef struct {
    const char *name;		/* name of channel */
    const char *sname;		/* short name of the channel */
    int dev;			/* channel device number */
    int prev_dev_lr_volume;	/* last known left/right volume
				 * (in device format) */
    float volume;		/* volume, in [0, 1] */
    float balance;		/* balance, in [-1, 1] */
    bool can_record;		/* capable of recording? */
    bool is_recording;		/* is it recording? */
    bool is_stereo;		/* capable of stereo? */
    bool is_muted;		/* is it muted? */
} MixerChannel;

static const char *channel_names[] = { CHANNEL_NAMES };
static const char *short_names[] = SOUND_DEVICE_LABELS;

static int mixer_fd;

static MixerChannel mixer[SOUND_MIXER_NRDEVICES];
static int n_channels = 0;
static int cur_channel = 0;

static int prev_modify_counter = -1;

static bool get_mixer_state(void)
{
    struct mixer_info m_info;
    int dev_lr_volume, dev_left_volume, dev_right_volume;
    float left, right;
    int srcmask;
    int ch;

    /* to really keep track of updates */
    static MixerChannel oldmixer[SOUND_MIXER_NRDEVICES];

    ioctl(mixer_fd, SOUND_MIXER_INFO, &m_info);

    if (m_info.modify_counter == prev_modify_counter)
	/*
	 * Mixer state has not changed
	 */
	return false;

    /* Mixer state was changed by another program, so we need
     * to update. As OSS cannot tell us specifically which
     * channels changed, we read all of them in.
     *
     * prev_modify_counter was initialized to -1, so this part
     * is guaranteed to run the first time this routine is
     * called.
     */

    if (ioctl(mixer_fd, SOUND_MIXER_READ_RECSRC, &srcmask) == -1) {
	fprintf(stderr, "mixer read failed\n");
	perror(NULL);
	exit(EXIT_FAILURE);
    }

    for (ch = 0; ch < n_channels; ch++) {
	if (ioctl(mixer_fd, MIXER_READ(mixer[ch].dev), &dev_lr_volume) ==
	    -1) {
	    fprintf(stderr, "mixer read failed\n");
	    exit(EXIT_FAILURE);
	}

	if (dev_lr_volume != mixer[ch].prev_dev_lr_volume) {
	    dev_left_volume = dev_lr_volume & 0xFF;
	    dev_right_volume = dev_lr_volume >> 8;

	    if ((dev_left_volume > 0) || (dev_right_volume > 0))
		mixer[ch].is_muted = false;

	    left = (float) dev_left_volume / 100.0;
	    right = (float) dev_right_volume / 100.0;

	    if (!mixer[ch].is_muted) {
		if (mixer[ch].is_stereo)
		    lr_to_vb(left,
			     right, &mixer[ch].volume, &mixer[ch].balance);
		else {
		    mixer[ch].volume = left;
		    mixer[ch].balance = 0.0;
		}

		mixer[ch].prev_dev_lr_volume = dev_lr_volume;
	    }
	}
	mixer[ch].is_recording = ((1 << mixer[ch].dev) & srcmask) != 0;
    }
    prev_modify_counter = m_info.modify_counter;
    /* check if this was due to OSS stupidity or if we really changed */
    if (!memcmp(&mixer, &oldmixer, sizeof(mixer))) {
	memcpy(&oldmixer, &mixer, sizeof(mixer));
	return false;
    }
    memcpy(&oldmixer, &mixer, sizeof(mixer));
    return true;
}

static void set_mixer_state(void)
{
    float left, right;
    int dev_left_volume, dev_right_volume, dev_lr_volume;

    if (mixer[cur_channel].is_muted) {
	left = 0.0;
	right = 0.0;
    } else
	vb_to_lr(mixer[cur_channel].volume,
		 mixer[cur_channel].balance, &left, &right);

    dev_left_volume = (int) (100.0 * left);
    dev_right_volume = (int) (100.0 * right);
    dev_lr_volume = (dev_right_volume << 8) | dev_left_volume;
    ioctl(mixer_fd, MIXER_WRITE(mixer[cur_channel].dev), &dev_lr_volume);
}

static void get_record_state(void)
{
    int srcmask;
    int ch;

    if (ioctl(mixer_fd, SOUND_MIXER_READ_RECSRC, &srcmask) == -1) {
	fprintf(stderr, "mixer read failed\n");
	perror(NULL);
	exit(EXIT_FAILURE);
    }

    for (ch = 0; ch < n_channels; ch++) {
	mixer[ch].is_recording = ((1 << mixer[ch].dev) & srcmask) != 0;
    }
}

static void set_record_state(void)
{
    int srcmask;

    if (ioctl(mixer_fd, SOUND_MIXER_READ_RECSRC, &srcmask) == -1) {
	fputs("error: recording source mask ioctl failed\n", stderr);
	exit(EXIT_FAILURE);
    }

    if (((1 << mixer[cur_channel].dev) & srcmask) == 0)
	srcmask |= (1 << mixer[cur_channel].dev);
    else
	srcmask &= ~(1 << mixer[cur_channel].dev);

    if (ioctl(mixer_fd, SOUND_MIXER_WRITE_RECSRC, &srcmask) == -1) {
	fputs("error: recording source mask ioctl failed\n", stderr);
	exit(EXIT_FAILURE);
    }
}

static bool is_exclude(const char *short_name, const char *exclude[])
{
    int count;
    int len;

    for (count = 0; exclude[count] != NULL; count++) {

        /*
         * Short names may be padded with spaces, because apparently there is a minimum
         * length requirement of 6 characters for the name, and we do not want to
         * include this padding in the match
         */
        len = strlen(short_name);
        while (len > 0) {
            if (short_name[len - 1] == ' ')
                len--;
            else
                break;
        }

        if (strncmp(short_name, exclude[count], len) != 0)
            continue;

        if (exclude[count][len] != '\0')
            continue;

        /* Check the remaining in short name is only space */
        while (short_name[len] == ' ')
            len++;

        if (short_name[len] == '\0')
            return true;
    }
    return false;
}

void mixer_oss_init(const char *mixer_device, bool verbose, const char * exclude[])
{
    int devmask, srcmask, recmask, stmask;
    struct mixer_info m_info;
    int count;
    int mask;

    mixer_fd = open(mixer_device, O_RDWR);

    if (mixer_fd == -1) {
	fprintf(stderr, "error: cannot open mixer device %s\n",
		mixer_device);
	exit(EXIT_FAILURE);
    }

    if (ioctl(mixer_fd, SOUND_MIXER_READ_DEVMASK, &devmask) == -1) {
	fputs("error: device mask ioctl failed\n", stderr);
	exit(EXIT_FAILURE);
    }

    if (ioctl(mixer_fd, SOUND_MIXER_READ_RECSRC, &srcmask) == -1) {
	fputs("error: recording source mask ioctl failed\n", stderr);
	exit(EXIT_FAILURE);
    }

    if (ioctl(mixer_fd, SOUND_MIXER_READ_RECMASK, &recmask) == -1) {
	fputs("error: recording mask ioctl failed\n", stderr);
	exit(EXIT_FAILURE);
    }

    if (ioctl(mixer_fd, SOUND_MIXER_READ_STEREODEVS, &stmask) == -1) {
	fputs("error: stereo mask ioctl failed\n", stderr);
	exit(EXIT_FAILURE);
    }

    if (ioctl(mixer_fd, SOUND_MIXER_INFO, &m_info) == -1) {
	fputs("error: could not read mixer info\n", stderr);
	exit(EXIT_FAILURE);
    }

    if (verbose) {
	printf("Sound card: %s (%s)\n", m_info.name, m_info.id);
	puts("Supported channels:");
    }
    for (count = 0; count < SOUND_MIXER_NRDEVICES; count++) {
	mask = 1 << count;
	if (!(mask & devmask))
		continue;

	if (!is_exclude(short_names[count], exclude)) {
	    mixer[n_channels].name = channel_names[count];
	    mixer[n_channels].sname = short_names[count];
	    mixer[n_channels].dev = count;
	    mixer[n_channels].prev_dev_lr_volume = -1;
	    mixer[n_channels].can_record = (mask & recmask) != 0;
	    mixer[n_channels].is_recording = (mask & srcmask) != 0;
	    mixer[n_channels].is_stereo = (mask & stmask) != 0;
	    mixer[n_channels].is_muted = false;
	    ++n_channels;
	    if (verbose)
		printf("  %d: %s \t(%s)\n", n_channels, channel_names[count], short_names[count]);
	} else if (verbose)
	    printf("  x: %s \t(%s) - disabled\n", channel_names[count], short_names[count]);
    }
    get_mixer_state();
}

bool mixer_oss_is_changed(void)
{
    return get_mixer_state();
}

int mixer_oss_get_channel_count(void)
{
    return n_channels;
}

int mixer_oss_get_channel(void)
{
    return cur_channel;
}

const char *mixer_oss_get_channel_name(void)
{
    return mixer[cur_channel].name;
}

const char *mixer_oss_get_short_name(void)
{
    return mixer[cur_channel].sname;
}

void mixer_oss_set_channel(int channel)
{
    assert((channel >= 0) && (channel < n_channels));

    cur_channel = channel;
    get_record_state();
}

void mixer_oss_set_channel_rel(int delta_channel)
{
    cur_channel = (cur_channel + delta_channel) % n_channels;
    if (cur_channel < 0)
	cur_channel += n_channels;
    get_record_state();
}

float mixer_oss_get_volume(void)
{
    get_mixer_state();
    return mixer[cur_channel].volume;
}

void mixer_oss_set_volume(float volume)
{
    assert((volume >= 0.0) && (volume <= 1.0));

    mixer[cur_channel].volume = volume;
    set_mixer_state();
}

void mixer_oss_set_volume_rel(float delta_volume)
{
    mixer[cur_channel].volume += delta_volume;
    mixer[cur_channel].volume = CLAMP(mixer[cur_channel].volume, 0.0, 1.0);
    set_mixer_state();
}

float mixer_oss_get_balance(void)
{
    get_mixer_state();
    return mixer[cur_channel].balance;
}

void mixer_oss_set_balance(float balance)
{
    assert((balance >= -1.0) && (balance <= 1.0));

    if (mixer[cur_channel].is_stereo) {
	mixer[cur_channel].balance = balance;
	set_mixer_state();
    }
}

void mixer_oss_set_balance_rel(float delta_balance)
{
    if (mixer[cur_channel].is_stereo) {
	mixer[cur_channel].balance += delta_balance;
	mixer[cur_channel].balance =
	    CLAMP(mixer[cur_channel].balance, -1.0, 1.0);
	set_mixer_state();
    }
}

void mixer_oss_toggle_mute(void)
{
    mixer[cur_channel].is_muted = !mixer[cur_channel].is_muted;

    set_mixer_state();
}

void mixer_oss_toggle_rec(void)
{
    if (mixer[cur_channel].can_record) {
	mixer[cur_channel].is_recording = !mixer[cur_channel].is_recording;
	set_record_state();
	get_record_state();
    }
}

bool mixer_oss_is_muted(void)
{
    return mixer[cur_channel].is_muted;
}

bool mixer_oss_is_stereo(void)
{
    return mixer[cur_channel].is_stereo;
}

bool mixer_oss_is_rec(void)
{
    return mixer[cur_channel].is_recording;
}

bool mixer_oss_can_rec(void)
{
    return mixer[cur_channel].can_record;
}
