#include "gai-gnome-config.h"
#ifndef GAI_WITH_GNOME
#include "gnome-config.c"
#elif !defined __GNUC__
const char gai_uses_gnome_config = 0;
#endif 
