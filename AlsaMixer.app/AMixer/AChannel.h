// AChannel.h
//
// Copyright (C) 2004, Petr Hlavka
//
// SPDX-License-Identifier: GPL-2.0+

#ifndef ACHANNEL_H
#define ACHANNEL_H

#include "AMixer.h"
#include "AItem.h"

#include <alsa/asoundlib.h>

#define SNDCHID_T 	snd_mixer_selem_channel_id_t

class AItem;

class AChannel {
  private:
    SNDCHID_T id;		// channel id (front left, f. right, ...)
    AItem *aItem;		// parent mixer item
  public:
    AChannel(AItem *item, SNDCHID_T cID);	// ctor
    ~AChannel();				// dtor

    long getVolume();		// get channel volume
    void setVolume(long value);	// set channel volume
    bool isMuted();		// return true, if channel is muted
};

#endif
