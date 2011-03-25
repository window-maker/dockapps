// wmmixer.h - A mixer designed for WindowMaker
//
// Release 1.5
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// Copyright (C) 2002 Gordon Fraser <gordon@debian.org>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the COPYING file for details.


#ifndef __wmmixer_h__
#define __wmmixer_h_

// Input/Output
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <iostream>

#include <X11/X.h>

// WMMixer
#include "mixctl.h"
#include "xhandler.h"
#include "common.h"
#include "exception.h"

// For repeating next and prev buttons
#define RPTINTERVAL   5


class WMMixer
{
 protected:

  // Mixer
  MixCtl *mixctl_;

  char     mixer_device_[256];
  unsigned num_channels_;
  unsigned current_channel_;
  unsigned current_channel_left_;
  unsigned current_channel_right_;
  bool     current_recording_;
  bool     current_show_recording_;

  XHandler *xhandler_;

  unsigned *channel_list_;

  int repeat_timer_;

  // For draggable volume control
  bool dragging_;
// Default scroll amount
  int wheel_scroll_;

  // Input/Output
  void readConfigurationFile();
  void displayVersion(void);
  void displayUsage(const char*);
  void checkVol(bool);

  void motionEvent(XMotionEvent *xev);
  void releaseEvent(XButtonEvent *xev);
  void pressEvent(XButtonEvent *xev);
  void parseArgs(int , char **);

  void initMixer();
  void initXHandler();

  void updateDisplay();

 public:
  WMMixer();
  ~WMMixer();

  void init(int, char **);
  void loop();
};


#endif //__wmmixer_h__
