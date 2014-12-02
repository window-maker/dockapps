// AChannel.cc, Petr Hlavka, 2004

#include "AMixer.h"
#include "AItem.h"
#include "AChannel.h"

#include <iostream>


AChannel::AChannel(AItem *item, SNDCHID_T cID) {
  aItem = item;
  id = cID;
}


AChannel::~AChannel() {
}


long AChannel::getVolume() {
  long vol = 0;

  snd_mixer_selem_get_playback_volume(aItem->aElem, (SNDCHID_T) id, &vol);

  return (vol);
}


// it isn't necessery when using snd_mixer_selem_set_x_volume_all
void AChannel::setVolume(long value) {
  std::cerr << "AChannel::setVolume not implemented!" << std::endl;
}


bool AChannel::isMuted() {
  int val;

  if (!snd_mixer_selem_has_playback_switch(aItem->aElem)) {
	  return (false); /* can't be muted? isn't muted. */
  }
  snd_mixer_selem_get_playback_switch(aItem->aElem, (SNDCHID_T) id, &val);

  return (! (bool) val);
}
