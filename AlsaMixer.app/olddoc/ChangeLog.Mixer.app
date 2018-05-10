Version		Description
--------------------------------------------------------------------------------
1.8.0		- Released 2002-09-15
		- Now supports all mixer sources. NOTE! The source names have
		  been changed. See "Mixer.app -h".
		- Added alternative skin submitted by Hans Dembinski <barross@web.de>.
		- Fixed warnings when compiling with GCC 3.2.
		- Added OpenBSD support (thanks to ptiJo <ptiJo@noos.fr>).

1.7.0		- Released 2001-06-25
		- Fixed compilation problems.

1.6.0		- Released 2001-03-17
		- Mixer.app is now devfs friendly, which means it will try to
		  use /dev/sound/mixer or /dev/sound/mixer1 first and fall back
		  to /dev/mixer if that didn't work.
		- New Makefile.

1.5.0		- Released 2000-02-21.
		- Added command line options -s and -S <file>, which cause
		  Mixer.app to load/save volume settings when starting/exiting.
		  When the option -s is used, settings are loaded from/saved in
		  ~/GNUstep/Defaults/Mixer. Use -S <file> if you want Mixer.app
		  to use a different file.
		- Fixed potential bug in command line parsing.

1.4.1		- Released 1999-05-28.
		- Added command line option -l <text> that can be used to
		  add a text label in the corner of the mixer.

1.4.0		- Released 1999-05-09.
		- Added support for wheel mice. One of the sliders (use -w to
		  specify which one) will react on wheel movement.
		- Misc. code clean up.

1.3.3		- Released 1999-05-02.
		- Fixed problem that caused Mixer.app to die.

1.3.2		- Released 1999-04-18.
		- Fixed exit bug. Mixer.app will now exit properly when
		  the windowmanager terminates.

1.3.1		- Released 1999-02-09.
		- Added new mute function (right click).
		- Rewrote command-line parsing, not using getopt anymore.
		- Minor code clean up.

1.3.0		- Released 1999-02-04.
		- New design.
		- I didn't find the mute function very usefull so I removed it.
		- Doing 'make install' will now install it in
                  /usr/local/GNUstep/Apps/Mixer.app/.

1.2.0		- Released 1998-12-01.
		- Moved back to old design. The design of version 1.1.1 made
		  Mixer.app (and Window Maker) unstable due to some strange
                  race condition at startup. It worked on some machines and
		  some not, so I desided to go back to the old design.
		- Increased idle interval to reduce CPU usage.

1.1.1		- Released 1998-11-14.
		- Fixed XGetImage errors, (slow machines may still have this
		  problem, please report any errors).
		- Added command line option -m <dev> to set mixer device.
		- Added command line option -n <name> to set instance name.
		- Compiling under FreeBSD now works fine.

1.1.0		- Released 1998-11-14.
		- Alarm singals are no longer used so now the "Alarm Clock"
		  problem is solved for sure!
		- Mute function.
		- New design.

1.0.5		- Released 1998-09-05.
		- Fixed the "Alarm Clock" problem.
		- Fixed problem when using WindowPlacement = manual
		  in WindowMaker.

1.0.4		- Released 1998-03-19.
		- Just some minor internal changes.

1.0.3		- Released 1998-02-24.
		- Changed default timerinterval.
		- Minor changes to eventhandling code.

1.0.2		- Released 1998-02-23.
		- If the red led goes on (an error has occured) the control
		  buttons are set to volume 0.

1.0.1		- Released 1998-02-23.
		- Changed classname to "mixer_app.Mixer_app".

1.0.0		- Released 1998-02-22.
		- First official version.

