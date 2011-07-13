// wmmixer - A mixer designed for WindowMaker
//
// Release 1.5
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// Copyright (C) 2002 Gordon Fraser <gordon@debian.org>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the COPYING file for details.

#include "exception.h"


//--------------------------------------------------------------------
Exception::Exception()
{
  error_message_ = NULL;
}

//--------------------------------------------------------------------
Exception::Exception(const Exception& exc)
{
  char* other_message = exc.getErrorMessage();

  if(other_message != NULL)
    {
      error_message_ = new char[strlen(other_message)+1];
      strcpy(error_message_, other_message);
    }
  else
    error_message_ = NULL;
}

//--------------------------------------------------------------------
Exception::~Exception()
{
  if(error_message_ != NULL)
    delete[] error_message_;
}

//--------------------------------------------------------------------
char* Exception::getErrorMessage() const
{
  return error_message_;
}

//--------------------------------------------------------------------
MixerDeviceException::MixerDeviceException(char* device)
{
  error_message_ = new char[256];
  strcpy(error_message_, "Unable to open mixer device ");
  strcat(error_message_, device);
}
