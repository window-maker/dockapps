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

/* WMVolume mixer interface
 *
 * Some notes:
 *
 * - The volume argument is a floating-point value between 0 (no sound) and
 *   1 (full sound) inclusively.
 *
 * - The balance argument is a floating-point value betwen -1 (full left)
 *   and 1 (full right) inclusively. A value of 0 indicates centered sound.
 *   If the device does not natively support a balance parameter, use the
 *   lr_to_vb() and vb_to_lr() functions in misc.c.
 *
 * - The volume and balance arguments passed to these functions, given that
 *   they are (usually 32-bit) floating-point values, may be capable of
 *   specifying the audio parameters at a finer resolution than the
 *   hardware itself. However, the mixer driver must not quantize the
 *   volume and balance levels returned by the mixer_get_*() routines.
 *
 *   Example: Suppose the mixer device supports specifying the volume as an
 *   integer in the range of 0 to 100. If mixer_set_volume() is called with
 *   an argument of 0.7351, then the device will be set to a volume of 74,
 *   yet subsequent calls to mixer_get_volume() (assuming mixer state has
 *   not changed since) will return 0.7351. Only if the mixer state is
 *   changed by another program (that, say, sets the volume to 51) is the
 *   value returned by this interface quantized (in such a case returning
 *   0.51).
 *
 *   The reason why this quantization must be avoided whenever possible is
 *   that otherwise, a large number of minuscule increases to the volume
 *   level will have no cumulative effect. Calling mixer_set_volume_rel()
 *   ten thousand times with an argument of 0.0001 should successfully
 *   increase the volume to its maximum, even if the device actually
 *   supports only 64 discrete volume levels.
 *
 * - Muting must occur independently of the volume level.
 */

void (*mixer_init)(const char *mixer_device,
                   bool verbose,
                   const char *exclude[]);
bool (*mixer_is_changed)(void);
int (*mixer_get_channel_count)(void);
int (*mixer_get_channel)(void);
const char *(*mixer_get_channel_name)(void);
const char *(*mixer_get_short_name)(void);
void (*mixer_set_channel)(int channel);
void (*mixer_set_channel_rel)(int delta_channel);
float (*mixer_get_volume)(void);
void (*mixer_set_volume)(float volume);
void (*mixer_set_volume_rel)(float delta_volume);
float (*mixer_get_balance)(void);
void (*mixer_set_balance)(float balance);
void (*mixer_set_balance_rel)(float delta_balance);
void (*mixer_toggle_mute)(void);
void (*mixer_toggle_rec)(void);
bool (*mixer_is_muted)(void);
bool (*mixer_is_stereo)(void);
bool (*mixer_is_rec)(void);
bool (*mixer_can_rec)(void);
bool (*is_exclude)(const char *short_name,
                   const char *exclude[]);
void (*mixer_tick)(void);
