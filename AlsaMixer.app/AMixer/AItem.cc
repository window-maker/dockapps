// AItem.cc
//
// Copyright (C) 2004, Petr Hlavka
//
// SPDX-License-Identifier: GPL-2.0+

#include "AItem.h"
#include "AMixer.h"

#include <alsa/asoundlib.h>
#include <vector>

#define ROUND_POS(x)	(long ((long (x) + 0.5 > (x)) ? (x) : (x) + 1))


// load only playback channels, don't care about common items
AItem::AItem(AMixer *m, snd_mixer_elem_t *e) {
  mixer = m;
  aElem = e;

  name = snd_mixer_selem_get_name(aElem);

  hPVolume = snd_mixer_selem_has_playback_volume(aElem);
  hPSwitch = snd_mixer_selem_has_playback_switch(aElem);

  if (hPVolume)
    snd_mixer_selem_get_playback_volume_range(aElem, &minPVolume, &maxPVolume);
  else
    minPVolume = maxPVolume = 0;

  for (int channel = 0; channel <= (int) SND_MIXER_SCHN_LAST; channel++) {
    if (snd_mixer_selem_has_playback_channel(aElem, (SNDCHID_T) channel)) {
      AChannel *ch = new AChannel(this, (SNDCHID_T) channel);
      pbChannels.push_back(ch);
    }
  }
}


AItem::~AItem() {
  for (unsigned int i = 0; i < pbChannels.size(); i++)
    delete pbChannels[i];
}


// set same volume for all channels in this item in percent
void AItem::setVolumePerc(unsigned int percent) {
  if (percent > 100)
    percent = 100;

  snd_mixer_selem_set_playback_volume_all(aElem, (long) ROUND_POS((minPVolume + (maxPVolume - minPVolume) * percent / 100.0)));
}


// get max channel volume of all channels in this item in percent
unsigned int AItem::getVolumePerc() {
  long max_vol = 0, act_vol;

  // find the max volume
  for (unsigned int i = 0; i < pbChannels.size(); i++)
    if ((act_vol = pbChannels[i]->getVolume()) > max_vol)
      max_vol = act_vol;

  // convert it into percent
  if (minPVolume != maxPVolume)
    return ((unsigned int) ROUND_POS((max_vol - minPVolume) * 100.0 / (maxPVolume - minPVolume)));
  else
    return (0);
}


// in this app side of view, item is muted if all channels is muted
bool AItem::isMuted() {
  for (unsigned int i = 0; i < pbChannels.size(); i++)
    if (!pbChannels[i]->isMuted())
      return (false);

  return (true);
}


// mute all channels
void AItem::mute() {
  snd_mixer_selem_set_playback_switch_all(aElem, false);
}


// unmute all channels
void AItem::unmute() {
  snd_mixer_selem_set_playback_switch_all(aElem, true);
}
