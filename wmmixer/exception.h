// wmmixer - A mixer designed for WindowMaker
//
// Release 1.5
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// Copyright (C) 2002 Gordon Fraser <gordon@debian.org>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the COPYING file for details.


#ifndef __exception_h__
#define __exception_h__

#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------
class Exception
{
 protected:
  char* error_message_;
  
 public:
  Exception();
  Exception(const Exception&);
  virtual ~Exception();
  char* getErrorMessage() const;
};


//--------------------------------------------------------------------
class MixerDeviceException : public Exception
{
 public:
  MixerDeviceException(char *);
  //  virtual ~MixerDeviceException();
};

#endif _exception_h__
