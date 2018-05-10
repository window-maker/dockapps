// AMixer.cc
//
// Copyright (C) 2004, Petr Hlavka
//
// SPDX-License-Identifier: GPL-2.0+

#include "AMixer.h"
#include "AItem.h"

#include <alsa/asoundlib.h>
#include <iostream>
#include <cstdio>
#include <cstring>

static bool mixerChangeIndicator = false;
static bool mixerReinitIndicator = false;
static int mixerCallback(snd_mixer_t *ctl, unsigned int mask,
    			 snd_mixer_elem_t *elem);
static int itemCallback(snd_mixer_elem_t *elem, unsigned int mask);


AMixer::AMixer(const char *card) {
  int err;

  if (card) {
    cardName = new char[std::strlen(card)];
    std::strcpy(cardName, card);
    for (int i = 0; i < MIXER_ITEMS; i++)
      mixerItems[i] = NULL;

    if ((err = snd_mixer_open(&mixerHandle, 0)) < 0) {
      error("snd_mixer_open error", err);
      mixerHandle = NULL;

      return;
    }
    if ((err = snd_mixer_attach(mixerHandle, card)) < 0) {
      error("snd_mixer_attach error", err);
      snd_mixer_close(mixerHandle);
      mixerHandle = NULL;

      return;
    }
    if ((err = snd_mixer_selem_register(mixerHandle, NULL, NULL)) < 0) {
      error("snd_mixer_selem_register error", err);
      snd_mixer_close(mixerHandle);
      mixerHandle = NULL;

      return;
    }
    if ((err = snd_mixer_load(mixerHandle)) < 0) {
      error("snd_mixer_load error", err);
      snd_mixer_close(mixerHandle);
      mixerHandle = NULL;

      return;
    }

    snd_mixer_set_callback(mixerHandle, (snd_mixer_callback_t) &mixerCallback);
  }
}


AMixer::~AMixer() {
  if (mixerHandle) {
    snd_mixer_free(mixerHandle);
    snd_mixer_detach(mixerHandle, cardName);
    delete[] cardName;
    snd_mixer_close(mixerHandle);
  }
}


bool AMixer::opened() {
  return (mixerHandle != NULL);
}


void AMixer::error(const char *errorString, int errorCode) {
  std::cerr << cardName << ": " << errorString << ": " << snd_strerror(errorCode) << std::endl;
}


void AMixer::handleEvents() {
  snd_mixer_handle_events(mixerHandle);
}


AItem *AMixer::attachItem(unsigned int itemNumber, const char *itemName) {
  if (itemNumber >= MIXER_ITEMS || !itemName)
    return (NULL);

  // item was already created, so deregister callback and free it first
  if (mixerItems[itemNumber]) {
    snd_mixer_elem_set_callback(mixerItems[itemNumber]->aElem, NULL);
    delete mixerItems[itemNumber];
  }

  // try to find item by name, register callback, return success
  snd_mixer_elem_t *elem;
  for (elem = snd_mixer_first_elem(mixerHandle); elem; elem = snd_mixer_elem_next(elem)) {
    if (snd_mixer_selem_is_active(elem) &&
	snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE &&
	std::strcmp(snd_mixer_selem_get_name(elem), itemName) == 0) {
      snd_mixer_elem_set_callback(elem, (snd_mixer_elem_callback_t) &itemCallback);
      return (mixerItems[itemNumber] = new AItem(this, elem));
    }
  }

  return (NULL);
}


bool AMixer::itemOK(unsigned int itemNumber) {
  return (itemNumber < MIXER_ITEMS && mixerItems[itemNumber]);
}


int AMixer::itemGetVolume(unsigned int itemNumber) {
  if (itemNumber >= MIXER_ITEMS || !mixerItems[itemNumber])
    return (-1);

  return ((int) mixerItems[itemNumber]->getVolumePerc());
}


void AMixer::itemSetVolume(unsigned int itemNumber, unsigned int volume) {
  if (itemNumber < MIXER_ITEMS && mixerItems[itemNumber])
    mixerItems[itemNumber]->setVolumePerc(volume);
}


int AMixer::itemIsMuted(unsigned int itemNumber) {
  if (itemNumber >= MIXER_ITEMS || !mixerItems[itemNumber])
    return (-1);

  return ((bool) mixerItems[itemNumber]->isMuted());
}


void AMixer::itemToggleMute(unsigned int itemNumber) {
  if (itemNumber < MIXER_ITEMS && mixerItems[itemNumber]) {
    if (itemIsMuted(itemNumber))
      mixerItems[itemNumber]->unmute();
    else
      mixerItems[itemNumber]->mute();
  }
}


// return true if mixer elm sent callback and this callback wasn't picked up yet
bool AMixer::mixerElemsChanged() {
  if (mixerChangeIndicator) {
    mixerChangeIndicator = false;

    return (true);
  }
  else
    return (false);
}


// return true if mixer sent callback and this callback wasn't picked up yet
bool AMixer::mixerChanged() {
  if (mixerReinitIndicator) {
    mixerReinitIndicator = false;

    return (true);
  }
  else
    return (false);
}


// this function should be called after mixer callback, reInit all items
void AMixer::reInit() {
  for (int i = 0; i < MIXER_ITEMS; i++)
    this->attachItem(i, mixerItems[i]->name);
}


int mixerCallback(snd_mixer_t *ctl, unsigned int mask,
			  snd_mixer_elem_t *elem) {
  mixerReinitIndicator = true;

  return (0);
}


int itemCallback(snd_mixer_elem_t *elem, unsigned int mask) {
  mixerChangeIndicator = true;

  return (0);
}


char* AMixer::convertIDToCard(const char* cardId) {
  static char card[32] = "";
  int i = snd_card_get_index(cardId);

  if (i >= 0 && i < 32)
    std::snprintf(card, 32, "hw:%i", i);
  else
    return (NULL);

  return (card);
}
