/*
   This code is based upon some lines(actually two lines :-)
   in E-Leds by Mathias Meisfjordskar<mathiasm@ifi.uio.no>
   Released under GPL.
*/

#include <stdio.h>
#include <X11/XKBlib.h>
#include <libdockapp/dockapp.h>

#include "include/kleds.h"

/*
  Returns the turned on leds:
   Bit 0 is Capslock
   Bit 1 is Numlock
   Bit 2 is Scrollock
*/

int check_kleds()
{
    unsigned int states;

    if (XkbGetIndicatorState(DADisplay, XkbUseCoreKbd, &states) != Success)
	{
		perror("Error while reading Indicator status\n");
		return -1;
    }
    return (states & 0x7);
}
