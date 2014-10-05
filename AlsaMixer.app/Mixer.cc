//
//  Mixer.app
//
//  Copyright (c) 1998-2002 Per Liden
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307,
//  USA.
//

#include <X11/Xlib.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include "Xpm.h"
#include "Mixer.h"

#include "AMixer/AMixer.h"

#include "pixmaps/main.xpm"
#include "pixmaps/button.xpm"
#include "pixmaps/mutebutton.xpm"
#include "pixmaps/redlight.xpm"

#define ROUND_POS(x)	(int ((x) + 0.5) > int (x)) ? (int) ((x) + 1) : (int) (x)

using namespace std;

static const int ButtonX[] = {6, 24, 42};
static const char* MixerSources[] = { "Master", "PCM", "CD" };

extern Mixer* app;

void catchBrokenPipe(int sig)
{
   app->saveVolumeSettings();
   exit(0);
}

int positionToPercent(int position) {
  return ROUND_POS(100 - (((position - BUTTON_MAX) * 100.0) / (BUTTON_MIN - BUTTON_MAX)));
}

int percentToPosition(int percent) {
  return ROUND_POS(BUTTON_MIN - (percent * (BUTTON_MIN - BUTTON_MAX)) / 100.0);
}

Mixer::Mixer(int argc, char** argv)
{
   XClassHint classHint;
   XSizeHints sizeHints;
   XWMHints   wmHints;
   Atom       deleteWindow;
   Xpm*       image;
   char*      displayName = NULL;
   char*      card = "default";

   mError = 0;
   mInstanceName = INSTANCENAME;
   mVolumeSource[0] = -1;
   mVolumeSource[1] = -1;
   mVolumeSource[2] = -1;
   mVolumeMute[0] = 0;
   mVolumeMute[1] = 0;
   mVolumeMute[2] = 0;
   mWheelButton = 1;
   mLabelText = 0;
   mSettingsFile = 0;
   mSaveSettings = false;
   mLoadSettings = false;
   mCommand = NULL;

   // Parse command line
   if (argc>1) {
      for (int i=1; i<argc; i++) {
         // Display
         if (!strcmp(argv[i], "-d")) {
            checkArgument(argv, argc, i);
            displayName = argv[i+1];
            i++;
         }

         // Sound source
         else if (!strcmp(argv[i], "-1") || !strcmp(argv[i], "-2") || !strcmp(argv[i], "-3")) {
            checkArgument(argv, argc, i);
	    MixerSources[argv[i][1] - '1'] = argv[i + 1];
	    i++;
         }

         // Wheel binding
         else if (!strcmp(argv[i], "-w")) {
            checkArgument(argv, argc, i);
            mWheelButton = atoi(argv[i+1]);

            if (mWheelButton < 1 || mWheelButton > 3) {
               cerr << APPNAME << ": invalid wheel binding, must be 1, 2 or 3, not " << argv[i+1] << endl;
               tryHelp(argv[0]);
               exit(0);
            }

            i++;
         }

         // Label text
         else if (!strcmp(argv[i], "-l")) {
            checkArgument(argv, argc, i);
            mLabelText = argv[i+1];
            i++;
         }

         // Save settings on exit
         else if (!strcmp(argv[i], "-S")) {
	    mSaveSettings = true;
         }

	 // Load settings on startup
	 else if (!strcmp(argv[i], "-L")) {
	    mLoadSettings = true;
	 }

         // Load/Save settings file
         else if (!strcmp(argv[i], "-f")) {
            checkArgument(argv, argc, i);
            mSettingsFile = argv[i+1];
            i++;
         }

	 // Execute command on middle click
         else if (!strcmp(argv[i], "-e")) {
           checkArgument(argv, argc, i);
	   mCommand = argv[i + 1];
	   i++;
	 }

         // Instance name
         else if (!strcmp(argv[i], "-n")) {
            checkArgument(argv, argc, i);
            mInstanceName = argv[i+1];
            i++;
         }

         // Version
         else if (!strcmp(argv[i], "-v")) {
            cerr << APPNAME << " version " << VERSION << endl;
            exit(0);
         }

         // Help
         else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            showHelp();
            exit(0);
         }

	 // card
	 else if (!strcmp(argv[i], "--card")) {
	   card = AMixer::convertIDToCard(argv[i + 1]);
	   if (!card) {
	     cerr << APPNAME << ": invalid card number '" << argv[i + 1] << "'" << endl;
	     tryHelp(argv[0]);
	     exit(0);
	   }
	   i++;
	 }

	 // device
	 else if (!strcmp(argv[i], "--device")) {
	   card = argv[i + 1];
	   i++;
	 }

         // Unknown option
         else {
            cerr << APPNAME << ": invalid option '" << argv[i] << "'" << endl;
            tryHelp(argv[0]);
            exit(0);
         }
      }
   }

   // default settings file
   if (!mSettingsFile) {
     char* home = getenv("HOME");
     if (home) {
       mSettingsFile = new char[strlen(home) + strlen(SETTINGS) + 1];
       strcpy(mSettingsFile, home);
       strcat(mSettingsFile, SETTINGS);
     } else {
       cerr << APPNAME << ": $HOME not set, could not find saved settings" << endl;
     }
   }

   // init mixer
   aMixer = new AMixer(card);
   if (!aMixer->opened()) {
     cerr << APPNAME << ": could not open mixer device for card '" << card << "'" << endl;
     exit(0);
   }

   // open mixer sources
   for (int i = 0; i < 3; i++) {
     aMixer->attachItem(i, MixerSources[i]);
     if (!aMixer->itemOK(i))
       cerr << APPNAME << ": could not select mixer source '" << MixerSources[i] << "'" << endl;
   }

   // Open display
   if ((mDisplay = XOpenDisplay(displayName)) == NULL) {
      cerr << APPNAME << ": could not open display " << displayName << endl;
      exit(0);
   }

   // Get root window
   mRoot = RootWindow(mDisplay, DefaultScreen(mDisplay));

   // Create windows
   mAppWin = XCreateSimpleWindow(mDisplay, mRoot, 1, 1, 64, 64, 0, 0, 0);
   mIconWin = XCreateSimpleWindow(mDisplay, mAppWin, 0, 0, 64, 64, 0, 0, 0);

   // Set classhint
   classHint.res_name =  mInstanceName;
   classHint.res_class = CLASSNAME;
   XSetClassHint(mDisplay, mAppWin, &classHint);

   // Create delete atom
   deleteWindow = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(mDisplay, mAppWin, &deleteWindow, 1);
   XSetWMProtocols(mDisplay, mIconWin, &deleteWindow, 1);

   // Set windowname
   XStoreName(mDisplay, mAppWin, APPNAME);
   XSetIconName(mDisplay, mAppWin, APPNAME);

   // Set sizehints
   sizeHints.flags= USPosition;
   sizeHints.x = 0;
   sizeHints.y = 0;
   XSetWMNormalHints(mDisplay, mAppWin, &sizeHints);

   // Set wmhints
   wmHints.initial_state = WithdrawnState;
   wmHints.icon_window = mIconWin;
   wmHints.icon_x = 0;
   wmHints.icon_y = 0;
   wmHints.window_group = mAppWin;
   wmHints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
   XSetWMHints(mDisplay, mAppWin, &wmHints);

   // Set command
   XSetCommand(mDisplay, mAppWin, argv, argc);

   // Set background image
   image = new Xpm(mDisplay, mRoot, main_xpm);
   if (mLabelText) {
      image->drawString(LABEL_X, LABEL_Y, mLabelText);
   }
   image->setWindowPixmapShaped(mIconWin);
   delete image;

   // Create buttons
   mButton[0] = XCreateSimpleWindow(mDisplay, mIconWin, ButtonX[0], BUTTON_MIN, 5, 5, 0, 0, 0);
   mButton[1] = XCreateSimpleWindow(mDisplay, mIconWin, ButtonX[1], BUTTON_MIN, 5, 5, 0, 0, 0);
   mButton[2] = XCreateSimpleWindow(mDisplay, mIconWin, ButtonX[2], BUTTON_MIN, 5, 5, 0, 0, 0);

   image = new Xpm(mDisplay, mRoot, button_xpm);
   image->setWindowPixmap(mButton[0]);
   image->setWindowPixmap(mButton[1]);
   image->setWindowPixmap(mButton[2]);
   delete image;

   XSelectInput(mDisplay, mButton[0], ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
   XSelectInput(mDisplay, mButton[1], ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
   XSelectInput(mDisplay, mButton[2], ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
   XSelectInput(mDisplay, mIconWin, ButtonPressMask);

   XMapWindow(mDisplay, mButton[0]);
   XMapWindow(mDisplay, mButton[1]);
   XMapWindow(mDisplay, mButton[2]);

   XMapWindow(mDisplay, mIconWin);
   XMapWindow(mDisplay, mAppWin);
   XSync(mDisplay, False);

   // Catch broker pipe signal
   signal(SIGPIPE, catchBrokenPipe);

   // Check if error
   if (mError) {
      showErrorLed();
   } else {
      getVolume();
      if (mLoadSettings)
	loadVolumeSettings();
   }
}

void Mixer::tryHelp(char* appname)
{
   cerr << "Try `" << appname << " --help' for more information" << endl;
}

void Mixer::showHelp()
{
   cerr << APPNAME << " Copyright (c) 1998-2002 by Per Liden (per@fukt.bth.se), Petr Hlavka (xhlavk00@stud.fit.vutbr.cz)" << endl << endl
        << "options:" << endl
        << " -1 <source>     set sound source for control 1 (default is Master)" << endl
        << " -2 <source>     set sound source for control 2 (default is PCM)" << endl
        << " -3 <source>     set sound source for control 3 (default is CD)" << endl
        << " -w 1|2|3        bind a control button to the mouse wheel (default is 1)" << endl
        << " -l <text>       set label text" << endl
        << " -S              save volume settings on exit" << endl
	<< " -L              load volume settings on start up" << endl
	<< " -f	<file>       use setting <file> instead of ~/GNUstep/Defaults/AlsaMixer" << endl
	<< " --card <id>     select card" << endl
	<< " --device <dev>  select device, default 'default'" << endl
	<< " -e <command>    execute <command> on middle click" << endl
        << " -n <name>       set client instance name" << endl
        << " -d <disp>       set display" << endl
        << " -v              print version and exit" << endl
        << " -h, --help      display this help and exit" << endl << endl;
}

void Mixer::checkArgument(char** argv, int argc, int index)
{
   if (argc-1 < index+1) {
      cerr << APPNAME << ": option '" << argv[index] << "' requires an argument" << endl;
      tryHelp(argv[0]);
      exit(0);
   }
}

void Mixer::showErrorLed()
{
   Window led;
   Xpm*   image;

   led = XCreateSimpleWindow(mDisplay, mIconWin, LED_X, LED_Y, 3, 2, 0, 0, 0);

   // Set background image
   image = new Xpm(mDisplay, mRoot, redlight_xpm);
   image->setWindowPixmap(led);
   delete image;

   // Show window
   XMapWindow(mDisplay, led);
   mError = 1;
}

void Mixer::loadVolumeSettings()
{
   if (mSettingsFile) {
      ifstream file(mSettingsFile);
      if (file) {
         // This could fail if the user has edited the file by hand and destroyed the structure
         char dummy[1024];
         file >> dummy; // {
         file >> dummy; // Volume1
         file >> dummy; // =
         file >> mVolume[0];
         file >> dummy; // ;

         file >> dummy; // Volume2
         file >> dummy; // =
         file >> mVolume[1];
         file >> dummy; // ;

         file >> dummy; // Volume3
         file >> dummy; // =
         file >> mVolume[2];

         file.close();
	 for (int i = 0; i < 3; i++) {
	   setVolume(i, mVolume[i]);
	   setButtonPosition(i, percentToPosition(mVolume[i]));
	 }
      }
   }
}

void Mixer::saveVolumeSettings()
{
   if (mSaveSettings) {
      ofstream file(mSettingsFile);
      if (file) {
         // Files in ~/GNUstep/Defaults/ should follow the property list format
         file << "{" << endl
              << "  Volume1 = " << mVolumePos[0] << ";" << endl
              << "  Volume2 = " << mVolumePos[1] << ";" << endl
              << "  Volume3 = " << mVolumePos[2] << ";" << endl
              << "}" << endl;
         file.close();
      } else {
         cerr << APPNAME << ": failed to save volume settings in " << mSettingsFile << endl;
      }
   }
}

void Mixer::getVolume()
{
   static int lastVolume[3] = {-1, -1, -1};
   static int lastVolumeMute[3] = {-1, -1, -1};

   if (mError) {
      return;
   }

   // Read from device
   for (int i=0; i<3; i++) {
      mVolume[i] = aMixer->itemGetVolume(i);
      mVolumeMute[i] = aMixer->itemIsMuted(i);

      if (lastVolume[i] != mVolume[i]) {
         int y;

         // Set button position
         if (mError) {
            y = BUTTON_MIN;
         } else {
	    y = percentToPosition(mVolume[i]);
         }

	 setButtonPosition(i, y);
         lastVolume[i] = mVolume[i];
      }

      // set buttom type muted/unmuted
      if (lastVolumeMute[i] != mVolumeMute[i]) {
	setButtonType(i);
	lastVolumeMute[i] = mVolumeMute[i];
      }
   }

   if (mError) {
      cerr << APPNAME << ": unable to read from " << mMixerDevice << endl;
      showErrorLed();
      return;
   }
}

void Mixer::setVolume(int button, int volume)
{
   if (mError) {
      return;
   }

   // Store volume
   mVolume[button] = volume;

   // Write to device
   aMixer->itemSetVolume(button, mVolume[button]);
}

void Mixer::toggleMute(int button)
{
  mVolumeMute[button] = !mVolumeMute[button];
  aMixer->itemToggleMute(button);
  setButtonType(button);
}

void Mixer::setButtonType(int button)
{
   Xpm* image;

   if (mVolumeMute[button] == 1) {		// muted
      image = new Xpm(mDisplay, mRoot, mutebutton_xpm);
      image->setWindowPixmap(mButton[button]);
      delete image;

      XClearWindow(mDisplay, mButton[button]);
   } else {
      image = new Xpm(mDisplay, mRoot, button_xpm);
      image->setWindowPixmap(mButton[button]);
      delete image;

      XClearWindow(mDisplay, mButton[button]);
   }
}

void Mixer::setButtonPosition(int button, int position) {
   if (position > BUTTON_MIN) {
      position = BUTTON_MIN;
   } else if (position < BUTTON_MAX) {
      position = BUTTON_MAX;
   }

   XMoveWindow(mDisplay, mButton[button], ButtonX[button], position);

   mVolumePos[button] = position;
}

void Mixer::setButtonPositionRelative(int button, int relativePosition)
{
   int y;

   // Calc new button position
   y = mVolumePos[button] + relativePosition;

   if (y > BUTTON_MIN) {
      y = BUTTON_MIN;
   } else if (y < BUTTON_MAX) {
      y = BUTTON_MAX;
   }

   // Set button position and volume
   XMoveWindow(mDisplay, mButton[button], ButtonX[button], y);

   mVolumePos[button] = y;

   // set volume
   setVolume(button, positionToPercent(y));
}

void Mixer::run()
{
   XEvent event;
   int    buttonDown = 0;
   int    buttonDownPosition = 0;

   // Start handling events
   while(1) {
      while(XPending(mDisplay) || buttonDown) {
         XNextEvent(mDisplay, &event);

         switch(event.type) {
         case ButtonPress:
            if (event.xbutton.button == Button4 || event.xbutton.button == Button5) {
               // Wheel scroll
               setButtonPositionRelative(mWheelButton - 1, event.xbutton.button == Button5? 3: -3);
            } else if (event.xbutton.button == Button1 && event.xbutton.window != mIconWin) {
               // Volume change
               buttonDown = 1;
               buttonDownPosition = event.xbutton.y;
            } else if (event.xbutton.button == Button3 && buttonDown == 0 && event.xbutton.window != mIconWin) {
               // Mute
               for (int i=0; i<3; i++) {
                  if (mButton[i] == event.xbutton.window) {
		     toggleMute(i);
                     break;
                  }
               }
            } else if (event.xbutton.button == Button2) {
	       // Load defaults or execute command
	       if (mCommand) {
		 char command[512];

		 snprintf(command, 512, "%s &", mCommand);
		 system(command);
	       }
	       else
		 loadVolumeSettings();
	    }
            break;

         case ButtonRelease:
            if (event.xbutton.button == Button1) {
               buttonDown = 0;
            }
            break;

         case MotionNotify:
            if (buttonDown) {
               // Find button
               for (int i=0; i<3; i++) {
                  if (mButton[i] == event.xmotion.window) {
                     setButtonPositionRelative(i, event.xmotion.y - buttonDownPosition);
                     break;
                  }
               }
            }
            break;
         }
      }

      // Idle for a moment
      usleep(100000);

      // Update volume status
      aMixer->handleEvents();
      if (AMixer::mixerChanged())
	aMixer->reInit();
      else if (AMixer::mixerElemsChanged())
	getVolume();
      XSync(mDisplay, False);
   }
}

