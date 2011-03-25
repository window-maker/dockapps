// mixctl.h - MixCtl class provides control of audio mixer functions
// 05/09/98  Release 1.0 Beta1
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the COPYING file for details.

// Although mixctl.h is an integral part of wmmixer, it may also be distributed seperately.

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __NetBSD__
#include <soundcard.h>
#endif
#ifdef __FreeBSD__
#include <machine/soundcard.h>
#endif
#ifdef __linux__
#include <linux/soundcard.h>
#endif

class MixCtl
{
public:
   MixCtl(char *dname){
      device=(char *)malloc(sizeof(char)*(strlen(dname)+1));
      strcpy(device,dname);
      if(mixfdopen=(mixfd=open(device,O_RDONLY | O_NONBLOCK))!=-1){
         nrdevices=SOUND_MIXER_NRDEVICES;
         char *devnames[]=SOUND_DEVICE_NAMES;
         char *devlabels[]=SOUND_DEVICE_LABELS;
         ioctl(mixfd, SOUND_MIXER_READ_DEVMASK, &devmask);
         ioctl(mixfd, SOUND_MIXER_READ_STEREODEVS, &stmask);
         ioctl(mixfd, SOUND_MIXER_READ_RECMASK, &recmask);
         ioctl(mixfd, SOUND_MIXER_READ_CAPS, &caps);
         mixdevs=(struct MixDev *)malloc(sizeof(struct MixDev)*nrdevices);
         int mixmask=1;
         for(int i=0;i<nrdevices;i++){
            mixdevs[i].support=devmask & mixmask;
            mixdevs[i].stereo=stmask & mixmask;
            mixdevs[i].records=recmask & mixmask;
            mixdevs[i].mask=mixmask;
            mixdevs[i].name=devnames[i];
            mixdevs[i].label=devlabels[i];
            mixmask*=2;
         }
         doStatus();
      }
   }
   ~MixCtl(){
      if(mixfdopen){
         if(mixdevs!=NULL)
            free(mixdevs);
         close(mixfd);
      }
   }
   bool openOK(){
      return mixfdopen;
   }
   void doStatus(){
      ioctl(mixfd, SOUND_MIXER_READ_RECSRC, &recsrc);
      for(int i=0;i<nrdevices;i++){
	 if(mixdevs[i].support)
	    ioctl(mixfd, MIXER_READ(i), &mixdevs[i].value);
         mixdevs[i].recsrc=(recsrc & mixdevs[i].mask);
      }
   }

   // Return volume for a device, optionally reading it from device first.
   // Can be used as a way to avoid calling doStatus().
   int readVol(int dev, bool read){
      if(read)
         ioctl(mixfd, MIXER_READ(dev), &mixdevs[dev].value);
      return mixdevs[dev].value;
   }

   // Return left and right componenets of volume for a device.
   // If you are lazy, you can call readVol to read from the device, then these
   // to get left and right values.
   int readLeft(int dev){
      return mixdevs[dev].value%256;
   }
   int readRight(int dev){
      return mixdevs[dev].value/256;
   }

   // Write volume to device. Use setVolume, setLeft and setRight first.
   void writeVol(int dev){
      ioctl(mixfd, MIXER_WRITE(dev), &mixdevs[dev].value);
   }

   // Set volume (or left or right component) for a device. You must call writeVol to write it.
   void setVol(int dev, int value){
      mixdevs[dev].value=value;
   }
   void setBoth(int dev, int l, int r){
      mixdevs[dev].value=256*r+l;
   }
   void setLeft(int dev, int l){
      int r;
      if(mixdevs[dev].stereo)
         r=mixdevs[dev].value/256;
      else
         r=l;
      mixdevs[dev].value=256*r+l;
   }
   void setRight(int dev, int r){
      int l;
      if(mixdevs[dev].stereo)
         l=mixdevs[dev].value%256;
      else
         l=r;
      mixdevs[dev].value=256*r+l;
   }

   // Return record source value for a device, optionally reading it from device first.
   bool readRec(int dev, bool read){
      if(read){
	 ioctl(mixfd, SOUND_MIXER_READ_RECSRC, &recsrc);
         mixdevs[dev].recsrc=(recsrc & mixdevs[dev].mask);
      }
      return mixdevs[dev].recsrc;
   }

   // Write record source values to device. Use setRec first.
   void writeRec(){
      ioctl(mixfd, SOUND_MIXER_WRITE_RECSRC, &recsrc);
   }

   // Make a device (not) a record source.
   void setRec(int dev, bool rec){
      if(rec){
         if(caps & SOUND_CAP_EXCL_INPUT)
            recsrc=mixdevs[dev].mask;
         else
            recsrc|=mixdevs[dev].mask;
      }
      else
         recsrc&=~mixdevs[dev].mask;
   }

   // Return various other info
   char *getDevName(){
      return device;
   }
   int getNrDevices(){
      return nrdevices;
   }
   int getCapabilities(){
      return caps;
   }
   bool getSupport(int dev){
      return mixdevs[dev].support;
   }
   bool getStereo(int dev){
      return mixdevs[dev].stereo;
   }
   bool getRecords(int dev){
      return mixdevs[dev].records;
   }
   char *getName(int dev){
      return mixdevs[dev].name;
   }
   char *getLabel(int dev){
      return mixdevs[dev].label;
   }

private:
   int mixfd;
   int mixfdopen;
   char *device;

   struct MixDev{
      bool support;
      bool stereo;
      bool recsrc;
      bool records;
      char *name;
      char *label;
      int value;
      int mask;
   };

   int nrdevices;       // maximum number of devices
   int devmask;         // supported devices
   int stmask;          // stereo devices
   int recmask;         // devices which can be recorded from
   int caps;            // capabilities
   int recsrc;          // devices which are being recorded from
   struct MixDev *mixdevs;
};
