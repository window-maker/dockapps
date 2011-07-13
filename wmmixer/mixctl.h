// mixctl.h - MixCtl class provides control of audio mixer functions
//
// Release 1.5
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// Copyright (C) 2002 Gordon Fraser <gordon@debian.org>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the COPYING file for details.

#ifndef __mixctl_h__
#define __mixctl_h__


#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#ifdef __NetBSD__
#include <soundcard.h>
#endif
#ifdef __FreeBSD__
#include <machine/soundcard.h>
#endif
#ifdef __linux__
#include <linux/soundcard.h>
#endif

#include "exception.h"

//----------------------------------------------------------------------
struct _MixerDevice_{
  bool support;
  bool stereo;
  bool recsrc;
  bool records;
  char *name;
  char *label;
  int value;
  int mask;
  int muted;
};
  
typedef struct _MixerDevice_ MixerDevice;

//----------------------------------------------------------------------
class MixCtl
{
 protected:
  int mixfd;
  int mixfdopen;
  char* device_;
  int muted_;

  unsigned num_devices_;       // maximum number of devices
  int devmask;         // supported devices
  int stmask;          // stereo devices
  int recmask;         // devices which can be recorded from
  int caps;            // capabilities
  int recsrc;          // devices which are being recorded from
  int modify_counter;
  MixerDevice* mixer_devices_;

  void doStatus();
  
 public:
  MixCtl(char *dname) throw(MixerDeviceException);
  virtual ~MixCtl();
  int readVol(int, bool);
  int readLeft(int);
  int readRight(int);
  void writeVol(int);

  void setVol(int, int);
  void setBoth(int, int, int);
  void setLeft(int, int);
  void setRight(int, int);

  bool readRec(int, bool);
  void writeRec();
  void setRec(int, bool);

  char *getDevName();
  unsigned getNrDevices();
  int getCapabilities();
  bool getSupport(int);
  bool getStereo(int);
  bool getRecords(int);
  char *getName(int);
  char *getLabel(int);
  bool hasChanged();

  bool isMuted(int);
  void mute(int);
  void unmute(int);
};

#endif // __mixctl_h__

