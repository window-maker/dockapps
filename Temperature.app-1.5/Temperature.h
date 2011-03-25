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

#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

#include <fstream>
#include <X11/Xlib.h>

#define APPNAME                 "Temperature.app"
#define VERSION                 "1.5"
#define INSTANCENAME            "temperature_app"
#define CLASSNAME               "Temperature_app"
#define METAR_URL               "http://weather.noaa.gov/pub/data/observations/metar/decoded/%s.TXT"
#define UPDATE_INTERVAL         900
#define TIME_POS                22
#define TEMP_POS                35
#define TEMP_WITH_TIME_POS      42
#define TIME_FONT               "-*-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*"
#define AMPM_FONT               "-*-helvetica-medium-r-*-*-8-*-*-*-*-*-*-*"
#define TEMP_FONT               "-*-helvetica-medium-r-*-*-18-*-*-*-*-*-*-*"
#define UNIT_FONT               "-*-helvetica-medium-r-*-*-12-*-*-*-*-*-*-*"
#define LED_X                   57
#define LED_Y                   59
#define TMP_FILE                "/tmp/temperature.app-XXXXXX"

/* temporary hack */
using namespace std;

enum ChildStatus
{
   ChildRunning, 
   ChildDone, 
   ChildError 
};

class Temperature
{
public:
   Temperature(int argc, char** argv);
   ~Temperature() {};
   void run();

private:
   void tryHelp(char* appname);
   void showHelp();
   void checkArgument(char** argv, int argc, int index);
   void showErrorLed(bool show);
   void calcTimeDiff();
   void setTime(char* utcTime);
   bool updateTemperture(ifstream& file);
   void showLed(bool show);

   Display*  mDisplay;
   Window    mRoot;
   Window    mAppWin;
   Window    mIconWin;
   Window    mStatusLed;
   char*     mInstanceName;
   char*     mStationId;
   char      mTemperature[5];
   char      mTime[6];
   char*     mTimeAMPM;
   double    mTimeDiff;
   bool      mFahrenheit;
   bool      mShowTime;
   bool      mTime12HourFormat;
   bool      mVerbose;
};

#endif
