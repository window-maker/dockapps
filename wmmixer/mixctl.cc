// mixctl.h - MixCtl class provides control of audio mixer functions
//
// Release 1.5
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// Copyright (C) 2002 Gordon Fraser <gordon@debian.org>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the COPYING file for details.



#include "mixctl.h"

//----------------------------------------------------------------------
MixCtl::MixCtl(char *device_name) throw(MixerDeviceException)
{
  device_ = new char[strlen(device_name)+1];
  strcpy(device_, device_name);

  modify_counter = -1;

  if((mixfd = open(device_,O_RDONLY | O_NONBLOCK)) != -1)
    {
      num_devices_      = SOUND_MIXER_NRDEVICES;
      char *devnames[]  = SOUND_DEVICE_NAMES;
      char *devlabels[] = SOUND_DEVICE_LABELS;
      ioctl(mixfd, SOUND_MIXER_READ_DEVMASK, &devmask);
      ioctl(mixfd, SOUND_MIXER_READ_STEREODEVS, &stmask);
      ioctl(mixfd, SOUND_MIXER_READ_RECMASK, &recmask);
      ioctl(mixfd, SOUND_MIXER_READ_CAPS, &caps);

      mixer_devices_ = new MixerDevice[num_devices_];
      int mixmask = 1;

      for(unsigned count=0; count<num_devices_; count++)
	{
	  mixer_devices_[count].support = devmask & mixmask;
	  mixer_devices_[count].stereo  = stmask  & mixmask;
	  mixer_devices_[count].records = recmask & mixmask;
	  mixer_devices_[count].mask    = mixmask;
	  mixer_devices_[count].name    = devnames[count];
	  mixer_devices_[count].label   = devlabels[count];
	  mixer_devices_[count].muted   = 0;
	  mixmask*=2;
	}
      doStatus();
    }
  else
    {
      throw MixerDeviceException(device_name);
    }
}

//----------------------------------------------------------------------
MixCtl::~MixCtl()
{
  if(mixer_devices_ != NULL)
    delete[](mixer_devices_);
  close(mixfd);

  delete[] device_;
}

//----------------------------------------------------------------------
bool MixCtl::isMuted(int channel)
{
  return mixer_devices_[channel].muted;
}

//----------------------------------------------------------------------
void MixCtl::mute(int channel)
{
  mixer_devices_[channel].muted = mixer_devices_[channel].value;
  mixer_devices_[channel].value = 0;
  writeVol(channel);
}

//----------------------------------------------------------------------
void MixCtl::unmute(int channel)
{
  mixer_devices_[channel].value = mixer_devices_[channel].muted;
  mixer_devices_[channel].muted = 0;
  writeVol(channel);
}

//----------------------------------------------------------------------
void MixCtl::doStatus()
{
  ioctl(mixfd, SOUND_MIXER_READ_RECSRC, &recsrc);
  for(unsigned i=0;i<num_devices_;i++)
    {
      if(mixer_devices_[i].support)
	{
	  ioctl(mixfd, MIXER_READ(i), &mixer_devices_[i].value);
	}
      mixer_devices_[i].recsrc=(recsrc & mixer_devices_[i].mask);
    }
}


//----------------------------------------------------------------------
// Return volume for a device, optionally reading it from device first.
// Can be used as a way to avoid calling doStatus().
int MixCtl::readVol(int dev, bool read)
{
  if(read)
    {
      ioctl(mixfd, MIXER_READ(dev), &mixer_devices_[dev].value);
    }
  return mixer_devices_[dev].value;
}

//----------------------------------------------------------------------
// Return left and right componenets of volume for a device.
// If you are lazy, you can call readVol to read from the device, then these
// to get left and right values.
int MixCtl::readLeft(int dev)
{
  return mixer_devices_[dev].value%256;
}

//----------------------------------------------------------------------
int MixCtl::readRight(int dev)
{
  return mixer_devices_[dev].value/256;
}

//----------------------------------------------------------------------
// Write volume to device. Use setVolume, setLeft and setRight first.
void MixCtl::writeVol(int dev)
{
  ioctl(mixfd, MIXER_WRITE(dev), &mixer_devices_[dev].value);
}

//----------------------------------------------------------------------
// Set volume (or left or right component) for a device. You must call writeVol to write it.
void MixCtl::setVol(int dev, int value)
{
  mixer_devices_[dev].value=value;
}
//----------------------------------------------------------------------
void MixCtl::setBoth(int dev, int l, int r)
{
  mixer_devices_[dev].value=256*r+l;
}
//----------------------------------------------------------------------
void MixCtl::setLeft(int dev, int l)
{
  int r;
  if(mixer_devices_[dev].stereo)
    r=mixer_devices_[dev].value/256;
  else
    r=l;
  mixer_devices_[dev].value=256*r+l;
}
//----------------------------------------------------------------------
void MixCtl::setRight(int dev, int r)
{
  int l;
  if(mixer_devices_[dev].stereo)
    l=mixer_devices_[dev].value%256;
  else
    l=r;
  mixer_devices_[dev].value=256*r+l;
}

//----------------------------------------------------------------------
// Return record source value for a device, optionally reading it from device first.
bool MixCtl::readRec(int dev, bool read)
{
  if(read)
    {
      ioctl(mixfd, SOUND_MIXER_READ_RECSRC, &recsrc);
      mixer_devices_[dev].recsrc=(recsrc & mixer_devices_[dev].mask);
    }
  return mixer_devices_[dev].recsrc;
}

//----------------------------------------------------------------------
// Write record source values to device. Use setRec first.
void MixCtl::writeRec(){
  ioctl(mixfd, SOUND_MIXER_WRITE_RECSRC, &recsrc);
}

//----------------------------------------------------------------------
// Make a device (not) a record source.
void MixCtl::setRec(int dev, bool rec)
{
  if(rec)
    {
      if(caps & SOUND_CAP_EXCL_INPUT)
	recsrc=mixer_devices_[dev].mask;
      else
	recsrc|=mixer_devices_[dev].mask;
    }
  else
    recsrc&=~mixer_devices_[dev].mask;
}

//----------------------------------------------------------------------
// Return various other info
char* MixCtl::getDevName()
{
  return device_;
}
//----------------------------------------------------------------------
unsigned MixCtl::getNrDevices()
{
  return num_devices_;
}
//----------------------------------------------------------------------
int MixCtl::getCapabilities()
{
  return caps;
}
//----------------------------------------------------------------------
bool MixCtl::getSupport(int dev)
{
  return mixer_devices_[dev].support;
}
//----------------------------------------------------------------------
bool MixCtl::getStereo(int dev)
{
  return mixer_devices_[dev].stereo;
}
//----------------------------------------------------------------------
bool MixCtl::getRecords(int dev)
{
  return mixer_devices_[dev].records;
}
//----------------------------------------------------------------------
char* MixCtl::getName(int dev)
{
  return mixer_devices_[dev].name;
}
//----------------------------------------------------------------------
char* MixCtl::getLabel(int dev)
{
  return mixer_devices_[dev].label;
}

//----------------------------------------------------------------------
bool MixCtl::hasChanged()
{
  struct mixer_info mixer_info;
  ioctl(mixfd, SOUND_MIXER_INFO, &mixer_info);
     
  if (mixer_info.modify_counter == modify_counter)
    {
      return false;
    }
  else 
    {
      modify_counter = mixer_info.modify_counter;
      return true;
    }
}
