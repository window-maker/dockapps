// wmmixer.cc - A mixer designed for WindowMaker
//
// Release 1.5
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// Copyright (C) 2002 Gordon Fraser <gordon@debian.org>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the COPYING file for details.


#include "wmmixer.h"

//--------------------------------------------------------------------
WMMixer::WMMixer()
{
  // Initialize member variables
  current_channel_        = 0;
  num_channels_           = 0;
  current_channel_left_   = 0;
  current_channel_right_  = 0;
  repeat_timer_           = 0;
  wheel_scroll_           = 2;
  current_recording_      = false;
  current_show_recording_ = false;
  dragging_               = false;

  strcpy(mixer_device_, MIXERDEV);

  xhandler_ = new XHandler();
}


//--------------------------------------------------------------------
WMMixer::~WMMixer()
{
  delete[] channel_list_;
  delete mixctl_;
  delete xhandler_;
}


//--------------------------------------------------------------------
void WMMixer::loop()
{
  XEvent xev;
  
  bool done=false;
  while(!done)
    {
      while(XPending(xhandler_->getDisplay())) 
	{
	  XNextEvent(xhandler_->getDisplay(), &xev);
	  switch(xev.type) 
	    {
	    case Expose:
	      xhandler_->repaint();
	      break;
	    case ButtonPress:
	      pressEvent(&xev.xbutton);
	      break;
	    case ButtonRelease:
	      releaseEvent(&xev.xbutton);
	      break;
	    case MotionNotify:
	      motionEvent(&xev.xmotion);
	      break;
	    case ClientMessage:
	      if(xev.xclient.data.l[0] == (int)xhandler_->getDeleteWin())
		done=true;
	      break;
	    }
	}
            
      // keep a button pressed causes scrolling throught the channels
      if(xhandler_->getButtonState() & (BTNPREV | BTNNEXT))
	{
	  repeat_timer_++;
	  if(repeat_timer_ >= RPTINTERVAL)
	    {
	      if(xhandler_->getButtonState() & BTNNEXT)
		{
		  current_channel_++;
		  if(current_channel_ >= num_channels_)
		    current_channel_ = 0;
		}
	      else
		{
		  if(current_channel_ < 1)
		    current_channel_ = num_channels_-1;
		  else
		    current_channel_--;
		}

	      checkVol(true);
	      repeat_timer_ = 0;
	    }
	}
      else 
	{
	  checkVol(false);
	}
      
      XFlush(xhandler_->getDisplay());
      usleep(100000);
    }
}


//--------------------------------------------------------------------
void WMMixer::init(int argc, char **argv)
{
  parseArgs(argc, argv);

  initMixer();

  readConfigurationFile(); 

  xhandler_->init(argc, argv, mixctl_->getNrDevices());

  if(num_channels_ == 0)
    {
      std::cerr << NAME << " : Sorry, no supported channels found." << std::endl;
    }
  else
    {
      checkVol(true);
    }
}

//--------------------------------------------------------------------
void WMMixer::initMixer()
{
  // Initialize Mixer
  try
    {
      mixctl_   = new MixCtl(mixer_device_);
    }
  catch(MixerDeviceException &exc)
    {
      std::cerr << NAME << " : " << exc.getErrorMessage() << "'." << std::endl;
      exit(1);
    }

  channel_list_ = new unsigned[mixctl_->getNrDevices()];
  
  for(unsigned count=0; count<mixctl_->getNrDevices(); count++)
    {
      if(mixctl_->getSupport(count)){
	channel_list_[num_channels_]=count;
	num_channels_++;
      }
    }
}


//--------------------------------------------------------------------
void WMMixer::checkVol(bool forced = true)
{
  if(!forced && !mixctl_->hasChanged())
    return;

  if(mixctl_->isMuted(channel_list_[current_channel_]))
    xhandler_->setButtonState(xhandler_->getButtonState() | BTNMUTE);
  else
    xhandler_->setButtonState(xhandler_->getButtonState() & ~BTNMUTE);


  mixctl_->readVol(channel_list_[current_channel_], true);
  unsigned nl   = mixctl_->readLeft(channel_list_[current_channel_]);
  unsigned nr   = mixctl_->readRight(channel_list_[current_channel_]);
  bool     nrec = mixctl_->readRec(channel_list_[current_channel_], true);

  if(forced)
    {
      current_channel_left_  = nl;
      current_channel_right_ = nr;
      current_recording_     = nrec;
      if(nrec)
	xhandler_->setButtonState(xhandler_->getButtonState() | BTNREC);
      else
	xhandler_->setButtonState(xhandler_->getButtonState() & ~BTNREC);
      current_show_recording_=mixctl_->getRecords(channel_list_[current_channel_]);
      updateDisplay();
    }
  else
    {
      if(nl != current_channel_left_ || nr != current_channel_right_ || nrec != current_recording_)
	{
	  if(nl!=current_channel_left_)
	    {
	      current_channel_left_=nl;
	      if(mixctl_->getStereo(channel_list_[current_channel_]))
		xhandler_->drawLeft(current_channel_left_);
	      else
		xhandler_->drawMono(current_channel_left_);
	    }
	  if(nr!=current_channel_right_)
	    {
	      current_channel_right_=nr;
	      if(mixctl_->getStereo(channel_list_[current_channel_]))
		xhandler_->drawRight(current_channel_right_);
	      else
		xhandler_->drawMono(current_channel_left_);
	    }
	  if(nrec!=current_recording_)
	    {
	      current_recording_=nrec;
	      if(nrec)
		xhandler_->setButtonState(xhandler_->getButtonState() | BTNREC);
	      else
		xhandler_->setButtonState(xhandler_->getButtonState() & ~BTNREC);
	      xhandler_->drawBtns(BTNREC, current_show_recording_);
	    }
	  updateDisplay();
	}      
    }
}



//--------------------------------------------------------------------
void WMMixer::parseArgs(int argc, char **argv)
{
  static struct option long_opts[] = {
    {"help",       0, NULL, 'h'},
    {"version",    0, NULL, 'v'},
    {"display",    1, NULL, 'd'},
    {"geometry",   1, NULL, 'g'},
    {"withdrawn",  0, NULL, 'w'},
    {"afterstep",  0, NULL, 'a'},
    {"shaped",     0, NULL, 's'},
    {"led-color",  1, NULL, 'l'},
    {"led-highcolor",  1, NULL, 'L'},
    {"back-color", 1, NULL, 'b'},
    {"mix-device", 1, NULL, 'm'},
    {"scrollwheel",1, NULL, 'r'},
    {NULL,         0, NULL, 0  }};
  int i, opt_index = 0;
  

  // For backward compatibility
  for(i=1; i<argc; i++) 
    {
      if(strcmp("-position", argv[i]) == 0) 
	{
	  sprintf(argv[i], "%s", "-g");
	} 
      else if(strcmp("-help", argv[i]) == 0) 
	{
	  sprintf(argv[i], "%s", "-h");
	} 
      else if(strcmp("-display", argv[i]) == 0) 
	{
	  sprintf(argv[i], "%s", "-d");
	}
    }

  while ((i = getopt_long(argc, argv, "hvd:g:wasl:L:b:m:r:", long_opts, &opt_index)) != -1) 
    {
      switch (i) 
	{
	case 'h':
	case ':':
	case '?':
	  displayUsage(argv[0]);
	  break;
	case 'v':
	  displayVersion();
	  break;
	case 'd':
	  xhandler_->setDisplay(optarg);
	  break;
	case 'g':
	  xhandler_->setPosition(optarg);
	  break;
	case 'w':
	  xhandler_->setWindowMaker();
	  break;
	case 'a':
	  xhandler_->setAfterStep();
	  break;
	case 's':
	  xhandler_->setUnshaped();
	  break;
	case 'l':
	  xhandler_->setLedColor(optarg);
	  break;
	case 'L':
	  xhandler_->setLedHighColor(optarg);
	  break;
	case 'b':
	  xhandler_->setBackColor(optarg);
	  break;
	case 'm':
	  sprintf(mixer_device_, "%s", optarg);
	  break;
	case 'r':
	  if(atoi(optarg)>0)
	    wheel_scroll_ = atoi(optarg);
	  break;
	}
    }
}

//--------------------------------------------------------------------
void WMMixer::readConfigurationFile()
{
   FILE *rcfile;
   char rcfilen[256];
   char buf[256];
   int done;
   //   int current=-1;
   unsigned current = mixctl_->getNrDevices() + 1;

   sprintf(rcfilen, "%s/.wmmixer", getenv("HOME"));
   if((rcfile=fopen(rcfilen, "r"))!=NULL)
     {
       num_channels_=0;
       do
	 {
	   fgets(buf, 250, rcfile);
	   if((done=feof(rcfile))==0)
	     {
	       buf[strlen(buf)-1]=0;
	       if(strncmp(buf, "addchannel ", strlen("addchannel "))==0)
		 {
		   sscanf(buf, "addchannel %i", &current);
		   if(current >= mixctl_->getNrDevices() || mixctl_->getSupport(current) == false)
		     {
		       fprintf(stderr,"%s : Sorry, this channel (%i) is not supported.\n", NAME, current);
		       current = mixctl_->getNrDevices() + 1;
		     }
		   else
		     {
		       channel_list_[num_channels_] = current;
		       num_channels_++;
		     }
		 }
            if(strncmp(buf, "setchannel ", strlen("setchannel "))==0)
	      {
		sscanf(buf, "setchannel %i", &current);
		if(current >= mixctl_->getNrDevices() || mixctl_->getSupport(current)==false)
		  {
		    fprintf(stderr,"%s : Sorry, this channel (%i) is not supported.\n", NAME, current);
		    current = mixctl_->getNrDevices() + 1;
		  }
	      }
            if(strncmp(buf, "setmono ", strlen("setmono "))==0)
	      {
		if(current== mixctl_->getNrDevices() + 1)
                  fprintf(stderr,"%s : Sorry, no current channel.\n", NAME);
		else{
                  int value;
                  sscanf(buf, "setmono %i", &value);
                  mixctl_->setLeft(current, value);
                  mixctl_->setRight(current, value);
                  mixctl_->writeVol(current);
		}
	      }
            if(strncmp(buf, "setleft ", strlen("setleft "))==0)
	      {
		if(current== mixctl_->getNrDevices() + 1)
                  fprintf(stderr, "%s : Sorry, no current channel.\n", NAME);
		else{
                  int value;
                  sscanf(buf, "setleft %i", &value);
                  mixctl_->setLeft(current, value);
                  mixctl_->writeVol(current);
	       }
            }
            if(strncmp(buf, "setright ", strlen("setright "))==0)
	      {
		if(current== mixctl_->getNrDevices() + 1)
                  fprintf(stderr, "%s : Sorry, no current channel.\n", NAME);
		else
		  {
		    int value;
		    sscanf(buf, "setleft %i", &value);
		    mixctl_->setRight(current, value);
		    mixctl_->writeVol(current);
		  }
	      }
            if(strncmp(buf, "setrecsrc ", strlen("setrecsrc "))==0)
	      {
		if(current== mixctl_->getNrDevices() + 1)
                  fprintf(stderr, "%s : Sorry, no current channel.\n", NAME);
		else
                  mixctl_->setRec(current, (strncmp(buf+strlen("setrecsrc "), "true", strlen("true"))==0));
	      }
	     }
	 }  
       while(done==0);
       fclose(rcfile);
       mixctl_->writeRec();
     }
}

//--------------------------------------------------------------------
void WMMixer::displayUsage(const char* name)
{
  std::cout << "Usage: " << name << "[options]" << std::endl;
  std::cout << "  -h,  --help                    display this help screen" << std::endl;
  std::cout << "  -v,  --version                 display program version" << std::endl;
  std::cout << "  -d,  --display <string>        display to use (see X manual pages)" << std::endl;
  std::cout << "  -g,  --geometry +XPOS+YPOS     geometry to use (see X manual pages)" << std::endl;
  std::cout << "  -w,  --withdrawn               run the application in withdrawn mode" << std::endl;
  std::cout << "                                 (for WindowMaker, etc)" << std::endl;
  std::cout << "  -a,  --afterstep               use smaller window (for AfterStep Wharf)" << std::endl;
  std::cout << "  -s,  --shaped                  shaped window" << std::endl;
  std::cout << "  -l,  --led-color <string>      use the specified color for led display" << std::endl;
  std::cout << "  -L,  --led-highcolor <string>  use the specified color for led shading" << std::endl;
  std::cout << "  -b,  --back-color <string>     use the specified color for backgrounds" << std::endl;
  std::cout << "  -m,  --mix-device              use specified device (rather than /dev/mixer)" << std::endl;
  std::cout << "  -r,  --scrollwheel <number>    volume increase/decrease with mouse wheel (default: 2)" << std::endl;
  std::cout << "\nFor backward compatibility the following obsolete options are still supported:" << std::endl;
  std::cout << "  -help                          display this help screen" << std::endl;
  std::cout << "  -position                      geometry to use (see X manual pages)" << std::endl;
  std::cout << "  -display                       display to use (see X manual pages)" << std::endl;
  exit(0);

}

//--------------------------------------------------------------------
void WMMixer::displayVersion()
{
  std::cout << "wmmixer version 1.5" << std::endl;
  exit(0);
}


//--------------------------------------------------------------------
void WMMixer::pressEvent(XButtonEvent *xev) 
{
  bool forced_update = true;
  int x = xev->x-(xhandler_->getWindowSize()/2-32);
  int y = xev->y-(xhandler_->getWindowSize()/2-32);

  if(xhandler_->isLeftButton(x, y))
    {
      if(current_channel_ < 1)
	current_channel_=num_channels_-1;
      else
	current_channel_--;

      xhandler_->setButtonState(xhandler_->getButtonState() | BTNPREV);
      repeat_timer_ = 0;
      xhandler_->drawBtns(BTNPREV, current_show_recording_);
    }

  if(xhandler_->isRightButton(x, y))
    {
      current_channel_++;
      if(current_channel_ >= num_channels_)
	current_channel_=0;

      xhandler_->setButtonState(xhandler_->getButtonState() | BTNNEXT);
      repeat_timer_ = 0;
      xhandler_->drawBtns(BTNNEXT, current_show_recording_);
    }

  // Volume settings
  if(xhandler_->isVolumeBar(x, y))
    {
      int vl = 0, vr = 0;

      if(xev->button < 4) 
	{
	  vl = ((60-y)*100)/(2*25);
	  vr = vl;
	  dragging_ = true;
	} 
      else if(xev->button == 4) 
	{
	  vr = mixctl_->readRight(channel_list_[current_channel_]) + wheel_scroll_;
	  vl = mixctl_->readLeft(channel_list_[current_channel_])  + wheel_scroll_;
	  
	} 
      else if(xev->button == 5) 
	{
	  vr = mixctl_->readRight(channel_list_[current_channel_]) - wheel_scroll_;
	  vl = mixctl_->readLeft(channel_list_[current_channel_])  - wheel_scroll_;
	}

      if(vl <= 0)
	vl = 0;
      if(vr <= 0)
	vr = 0;

      if(x <= 50)
	mixctl_->setLeft(channel_list_[current_channel_], vl);
      if(x >= 45)
	mixctl_->setRight(channel_list_[current_channel_], vr);
      mixctl_->writeVol(channel_list_[current_channel_]);
      
      forced_update = false;
    }

  // Toggle record
  if(xhandler_->isRecButton(x, y))
    {
      mixctl_->setRec(channel_list_[current_channel_], !mixctl_->readRec(channel_list_[current_channel_], false));
      mixctl_->writeRec();
      forced_update = false;
    }

  // Toggle mute
  if(xhandler_->isMuteButton(x, y))
    {
      if(mixctl_->isMuted(channel_list_[current_channel_]))
	{
	  xhandler_->setButtonState(xhandler_->getButtonState() & ~BTNMUTE);
	  mixctl_->unmute(channel_list_[current_channel_]);
	}
      else
	{
	  mixctl_->mute(channel_list_[current_channel_]);
	  xhandler_->setButtonState(xhandler_->getButtonState() | BTNMUTE);
	}

      xhandler_->drawBtns(BTNMUTE, current_show_recording_);
    }

  // Update volume display
  checkVol(forced_update);
}

//--------------------------------------------------------------------
void WMMixer::releaseEvent(XButtonEvent *xev)
{
  dragging_ = false;
  xhandler_->setButtonState(xhandler_->getButtonState() & ~(BTNPREV | BTNNEXT));
  xhandler_->drawBtns(BTNPREV | BTNNEXT, current_show_recording_);
  xhandler_->repaint();
}

//--------------------------------------------------------------------
void WMMixer::motionEvent(XMotionEvent *xev)
{
  int x=xev->x-(xhandler_->getWindowSize()/2-32);
  int y=xev->y-(xhandler_->getWindowSize()/2-32);
  //  if(x>=37 && x<=56 && y>=8 && dragging_){
  if(xhandler_->isVolumeBar(x, y) && dragging_){
    int v=((60-y)*100)/(2*25);
    if(v<0)
      v=0;
    if(x<=50)
      mixctl_->setLeft(channel_list_[current_channel_], v);
    if(x>=45)
      mixctl_->setRight(channel_list_[current_channel_], v);
    mixctl_->writeVol(channel_list_[current_channel_]);
    checkVol(false);
  }
}

//--------------------------------------------------------------------
void WMMixer::updateDisplay()
{
  xhandler_->update(channel_list_[current_channel_]);
  if(mixctl_->getStereo(channel_list_[current_channel_])) 
    {
      xhandler_->drawLeft(current_channel_left_);
      xhandler_->drawRight(current_channel_right_);
    }
  else
    {
      xhandler_->drawMono(current_channel_right_);
    }
  xhandler_->drawBtns(BTNREC | BTNNEXT | BTNPREV | BTNMUTE, current_show_recording_);
  xhandler_->repaint();
}



//====================================================================
int main(int argc, char** argv)
{
  WMMixer mixer = WMMixer();
  mixer.init(argc, argv);
  mixer.loop();
}
