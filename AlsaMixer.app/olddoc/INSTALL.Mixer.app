Installation instructions for Mixer.app


Requirements
-------------------------------------------------------------------
- X11, libxpm, C++ development environment
        Most (Linux) systems have these things installed by default.
        If you don't have it look for packages that fit your
        distribution.


Installation
-------------------------------------------------------------------
1) make
2) su
3) make install       (to put it in /usr/local/GNUstep/Apps/Mixer.app)
   or
   make install-x11   (to put it in /usr/X11R6/bin)
5) exit


Running
-------------------------------------------------------------------
To run this program:
/usr/local/GNUstep/Apps/Mixer.app/Mixer &
or
/usr/X11R6/bin/Mixer.app &


For more information about available command line arguments:
/usr/local/GNUstep/Apps/Mixer.app/Mixer --help
or
/usr/X11R6/bin/Mixer.app --help
