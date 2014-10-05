
                       Mixer.app
            <http://www.fukt.bth.se/~per/mixer/>

                      by Per Liden
                    <per@fukt.bth.se>


Description
--------------------------------------------------------------
Mixer.app is a mixer utility for Linux/FreeBSD/OpenBSD systems.
It is designed to be docked in Window Maker. This utility has
three volume controllers that can be configured to handle any
sound source, the default sources are master-, cd- and pcm-volume.
Sound sources can easily be muted and there is also wheel mouse
support.


Hints
--------------------------------------------------------------
Error led:
	If the led on Mixer.app is red an error message has
	been printed to stderr and something is not working
	correctly. If the led is green everything is working ok.

Mute:
	Right click on a volume controller to mute the sound
	source. The button will then have a red led in one corner.
	Right click again to restore the volume. If a muted sound
	source is modified by another application Mixer.app will
	automaticaly release its muted state.

Wheel mouse:
	If you have a wheel mouse (where the wheel is configured as
	Button4 and Button5) you can control the volume by just moving
	the mouse over Mixer.app and roll the wheel up and down. Use
	the command line option -w to specify which slider that should
	react to the wheel movement.

Label:
	If you run multiple instances of Mixer.app you might want
	to be able to tell them apart. This can easily be done
	by setting a label to each one of them. By using the -l
	option you can add a little text label at the bottom of
	the mixer.

Save volume settings:
	Use the option -s or -S <file> if you want the volume
	settings to be saved when Mixer.app exits, and then
	loaded again when Mixer.app is started the next time.
	When using -s the settings are saved in/loaded from
	~/GNUstep/Defaults/Mixer. Use the -S <file> option
	if you want to use a different file.

Bugs
--------------------------------------------------------------
If you discover any bugs in this software, please send a
bugreport to per@fukt.bth.se and describe the problem.


Special thanks to
--------------------------------------------------------------
David Sauer <davids@iol.cz>
Theo Schlossnagle <theo@omniti.com>
Alban Hertroys <dalroi@wit401310.student.utwente.nl>


Copyright
--------------------------------------------------------------
Mixer.app is copyright (c) 1998-2002 by Per Liden and is
licensed through the GNU General Public License. Read the
COPYING file for the complete license.

Minor parts of this code were taken from asmixer by Rob Malda
(malda@slashdot.org)
