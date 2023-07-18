#include "gai-gnome-config.h"
#ifndef GAI_WITH_GNOME
#include "gnome-util.c"
#elif !defined __GNUC__
const char gai_uses_gnome_util = 0;
#endif 


