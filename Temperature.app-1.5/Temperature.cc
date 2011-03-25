//
//  Temperature.app
// 
//  Copyright (c) 2000-2002 Per Liden
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
//  USA.
//

#include <X11/Xlib.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>
#include "Xpm.h"
#include "Temperature.h"

#include "pixmaps/main.xpm"
#include "pixmaps/redlight.xpm"

volatile static ChildStatus childStatus;

static void catchBrokenPipe(int sig) 
{
   exit(0);
}

static void catchChildExit(int sig) 
{
   int status;
   waitpid(-1, &status, 0);

   if (WIFEXITED(status)) {
      if (WEXITSTATUS(status)) {
         childStatus = ChildError;
      } else {
         childStatus = ChildDone;
      }
   } else {
      childStatus = ChildError;
   }

   if (childStatus == ChildError) {
      std::cerr << APPNAME << ": could not fetch temperature (wget failed), try option -V for more information" << std::endl;
   }
}

Temperature::Temperature(int argc, char** argv) 
{
   XClassHint classHint;
   XSizeHints sizeHints;
   XWMHints   wmHints;
   Atom       deleteWindow;
   Xpm*       image;
   char*      displayName = NULL;

   mInstanceName = INSTANCENAME;
   mStationId = 0;
   mTemperature[0] = 0;
   mTime[0] = 0;
   mTimeDiff = 0.0;
   mFahrenheit = false;
   mShowTime = false;
   mTime12HourFormat = false;
   mVerbose = false;

   // Parse command line
   if (argc>1) {
      for (int i=1; i<argc; i++) {
         // Display
         if (!strcmp(argv[i], "-d")) {
            checkArgument(argv, argc, i);
            displayName = argv[i+1];
            i++;
         }

         // Station id
         else if (!strcmp(argv[i], "-s")) {
            checkArgument(argv, argc, i);
            mStationId = argv[i+1];
            i++;
         }

         // Fahrenheit
         else if (!strcmp(argv[i], "-f")) {
            mFahrenheit = true;
         }

         // Time
         else if (!strcmp(argv[i], "-t")) {
            mShowTime = true;
            checkArgument(argv, argc, i);
            if (!strcmp(argv[i+1], "12")) {
               mTime12HourFormat = true;
            } else if (strcmp(argv[i+1], "24")) {
               std::cerr << APPNAME << ": unknown time format, use 12 or 24" << std::endl;
               exit(0);
            }
            i++;
         }

         // Verbose
         else if (!strcmp(argv[i], "-V")) {
            mVerbose = true;
         }

         // Instance name
         else if (!strcmp(argv[i], "-n")) {
            checkArgument(argv, argc, i);
            mInstanceName = argv[i+1];
            i++;
         }

         // Version
         else if (!strcmp(argv[i], "-v")) {
            std::cerr << APPNAME << " version " << VERSION << std::endl;
            exit(0);
         }

         // Help
         else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            showHelp();
            exit(0);
         }

         // Unknown option
         else {
            std::cerr << APPNAME << ": invalid option '" << argv[i] << "'" << std::endl;
            tryHelp(argv[0]);
            exit(0);
         }
      }
   }

   if (mStationId == 0) {
      std::cerr << APPNAME << ": you must supply a station id using -s <id>" << std::endl;
      tryHelp(argv[0]);
      exit(0);
   }

   // Open display
   if ((mDisplay = XOpenDisplay(displayName)) == NULL) {
      std::cerr << APPNAME << ": could not open display " << displayName << std::endl;
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
   image->setWindowPixmapShaped(mIconWin);
   delete image;

   // Create status led
   mStatusLed = XCreateSimpleWindow(mDisplay, mIconWin, LED_X, LED_Y, 3, 2, 0, 0, 0);
   image = new Xpm(mDisplay, mRoot, redlight_xpm);
   image->setWindowPixmap(mStatusLed);
   delete image;

   XMapWindow(mDisplay, mIconWin);
   XMapWindow(mDisplay, mAppWin);
   XSync(mDisplay, False);

   // Catch broker pipe signal
   signal(SIGPIPE, catchBrokenPipe);

   // Catch child exit signal
   signal(SIGCHLD, catchChildExit);
}

void Temperature::tryHelp(char* appname)
{
   std::cerr << std::endl << "Try `" << appname << " --help' for more information" << std::endl;
}

void Temperature::showHelp() 
{
   std::cerr << APPNAME << " Copyright (c) 2000-2002 by Per Liden (per@fukt.bth.se)" << std::endl << std::endl
        << "options:" << std::endl
        << " -s <id>         set station id (ICAO Location Indicator)" << std::endl
        << " -t 12|24        display time of temperature observation (12 or 24 hour format)" << std::endl
        << " -f              display degrees in Fahrenheit" << std::endl
        << " -V              display verbose messages from wget" << std::endl
        << " -n <name>       set client instance name" << std::endl
        << " -d <disp>       set display" << std::endl
        << " -v              print version and exit" << std::endl
        << " -h, --help      display this help text and exit" << std::endl 
        << std::endl
        << "You must supply the ICAO Location Indicator (a 4-character string)" << std::endl
        << "of a weather station near you. You can search for a station on" << std::endl
        << "this site: http://www.nws.noaa.gov/oso/siteloc.shtml" << std::endl;
}

void Temperature::checkArgument(char** argv, int argc, int index)
{
   if (argc-1 < index+1) {
      std::cerr << APPNAME << ": option '" << argv[index] << "' requires an argument" << std::endl;
      tryHelp(argv[0]);
      exit(0);
   }
}

void Temperature::showLed(bool show)
{
   if (show) {
      XMapWindow(mDisplay, mStatusLed);
   } else {
      XUnmapWindow(mDisplay, mStatusLed);
   }
   XSync(mDisplay, False);
}

void Temperature::calcTimeDiff()
{
   struct tm* t;
   double localTime;
   double universalTime;
   time_t currentTime;

   currentTime = time(0);
   t = gmtime(&currentTime);
   universalTime = (double)t->tm_hour + (double)t->tm_min / 60.0 + (double)t->tm_sec / 3600.0;

   currentTime = time(0);
   t = localtime(&currentTime);
   localTime = (double)t->tm_hour + (double)t->tm_min / 60.0 + (double)t->tm_sec / 3600.0;

   mTimeDiff = universalTime - localTime;
   if (mTimeDiff > 24.0) {
      mTimeDiff -= 24.0;
   } else if (mTimeDiff < 0.0) {
      mTimeDiff += 24.0;
   }
}

void Temperature::setTime(char* utcTime)
{
   char unit[3];
   int hour = 0;
   int min = 0;   

   strncpy(unit, &utcTime[0], 2);
   hour = atoi(unit);
   strncpy(unit, &utcTime[2], 2);
   min = atoi(unit);

   double time = ((double)hour + (double)min / 60.0) - mTimeDiff;
   if (time < 0.0) {
      time += 24.0;
   } else if (time > 24.0) {
      time -= 24.0;
   }

   hour = (int)time;
   min = (int)((time - (double)hour) * 60.0 + 0.5);
   if (min >= 60){
      min = 0;
      if (++hour >= 24) {
         hour = 0;
      }
   }

   if (mTime12HourFormat) {
      if (hour >= 0 && hour <= 11) {
         mTimeAMPM = "AM";         
      } else {
         mTimeAMPM = "PM";
      }

      if (hour == 0) {
         hour = 12;
      } else if (hour > 12) {
         hour -= 12;
      }
   }
   sprintf(mTime, "%d:%.2d", hour, min);
}

bool Temperature::updateTemperture(ifstream& file)
{
   const int MAX_LINE = 1024;
   char buffer[MAX_LINE];

   if (mShowTime) {
      // Find time of observation
      char* start;
      char time[5];
      file.getline(buffer, MAX_LINE - 1);
      file.getline(buffer, MAX_LINE - 1);
      if ((start = strstr(buffer, "UTC")) == 0) {
         return false;
      }
      strncpy(time, start - 5, 4);
      setTime(time);
   }
   
   // Find temperature
   while (!file.eof()) {
      file >> buffer;
      if (!strcmp(buffer, "Temperature:")) {
         file >> buffer;
         if (buffer && strlen(buffer) < 5) {
            char* unit;
            if (mFahrenheit) {
               strcpy(mTemperature, buffer);
               unit = " °F";
            } else {
               sprintf(mTemperature, "%d", (int)rint((atoi(buffer) - 32) / 1.8));
               unit = " °C";
            }
            
            Xpm* image = new Xpm(mDisplay, mRoot, main_xpm);
            if (mShowTime) {
               if (mTime12HourFormat) {
                  image->drawComposedString(TIME_POS, TIME_FONT, mTime, AMPM_FONT, mTimeAMPM);
               } else {
                  image->drawString(TIME_POS, TIME_FONT, mTime);
               }
               image->drawComposedString(TEMP_WITH_TIME_POS, TEMP_FONT, mTemperature, UNIT_FONT, unit);
            } else {
               image->drawComposedString(TEMP_POS, TEMP_FONT, mTemperature, UNIT_FONT, unit);
            }
            image->setWindowPixmap(mIconWin);
            delete image;
            XSync(mDisplay, False);
            return true;
         }
      }
   }

   std::cerr << APPNAME << ": could not fetch temperature (unknown file format)" << std::endl;
   return false;
}

void Temperature::run() 
{
   if (mShowTime) {
      calcTimeDiff();
   }

   int counter = 0;
   while(1) {
      if (counter <= 0) {
         char tmpFile[sizeof(TMP_FILE)] = TMP_FILE;
         int fd = mkstemp(tmpFile);
         if (fd == -1) {
            std::cerr << APPNAME << ": could not create temporary file " << tmpFile << ": " << strerror(errno) << std::endl;
            exit(1);
         }
         close(fd);

         counter = UPDATE_INTERVAL;
         childStatus = ChildRunning;
         signal(SIGCHLD, catchChildExit);
         showLed(true);
         int pid = fork();
         if (pid == 0) {
            const char* verbose = (mVerbose ? "--verbose" : "--quiet");
            char* URL = new char[strlen(METAR_URL) + strlen(mStationId) + 1];
            sprintf(URL, METAR_URL, mStationId);
            execlp("wget", "wget", "--cache=off", "--tries=0", verbose, "-O", tmpFile, URL, 0);
            std::cerr << APPNAME << ": could not fetch temperature (wget not found in $PATH)" << std::endl;
            remove(tmpFile);
            exit(0);
         } else if (pid == -1) {
            std::cerr << APPNAME << ": could not fetch temperature (fork() failed)" << std::endl;
         } else {
            bool toggle = true;
            while (childStatus == ChildRunning) {
               showLed(toggle);
               toggle ^= true;
               sleep(1);
            }
            showLed(true);
            if (childStatus == ChildDone) {
               ifstream file(tmpFile);
               if (file) {
                  if (updateTemperture(file)) {
                     showLed(false);
                  }
                  file.close();
               }
            }
            remove(tmpFile);
         }
      } else {
         counter--;
         sleep(1);
         XSync(mDisplay, False);
      }
   }
}
