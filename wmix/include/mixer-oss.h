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

void mixer_oss_init(const char *mixer_oss_device,
                    bool verbose,
                    const char *exclude[]);
bool mixer_oss_is_changed(void);
int mixer_oss_get_channel_count(void);
int mixer_oss_get_channel(void);
const char *mixer_oss_get_channel_name(void);
const char *mixer_oss_get_short_name(void);
void mixer_oss_set_channel(int channel);
void mixer_oss_set_channel_rel(int delta_channel);
float mixer_oss_get_volume(void);
void mixer_oss_set_volume(float volume);
void mixer_oss_set_volume_rel(float delta_volume);
float mixer_oss_get_balance(void);
void mixer_oss_set_balance(float balance);
void mixer_oss_set_balance_rel(float delta_balance);
void mixer_oss_toggle_mute(void);
void mixer_oss_toggle_rec(void);
bool mixer_oss_is_muted(void);
bool mixer_oss_is_stereo(void);
bool mixer_oss_is_rec(void);
bool mixer_oss_can_rec(void);
