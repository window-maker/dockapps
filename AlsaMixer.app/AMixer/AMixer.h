// AMixer.h, Petr Hlavka, 2004

#ifndef AMIXER_H
#define AMIXER_H

#include "AItem.h"

#include <alsa/asoundlib.h>

#define MIXER_ITEMS	3


class AItem;

class AMixer {
  private:
    snd_mixer_t *mixerHandle;
    AItem *mixerItems[MIXER_ITEMS];
    char *cardName;

    void error(const char *errorString, int errorCode);

  public:
    AMixer(const char *card);		// ctor
    ~AMixer();				// dtor

    bool opened();
    void handleEvents();
    AItem *attachItem(unsigned int itemNumber, const char *itemName);
    bool itemOK(unsigned int itemNumber);
    int itemGetVolume(unsigned int itemNumber);
    void itemSetVolume(unsigned int itemNumber, unsigned int volume);
    int itemIsMuted(unsigned int itemNumber);
    void itemToggleMute(unsigned int itemNumber);
    void reInit();

    static bool mixerElemsChanged();
    static bool mixerChanged();
    static char* convertIDToCard(const char* cardId);
};

#endif
