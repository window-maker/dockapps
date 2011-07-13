// wmmixer - A mixer designed for WindowMaker
//
// Release 1.5
// Copyright (C) 1998  Sam Hawker <shawkie@geocities.com>
// Copyright (C) 2002 Gordon Fraser <gordon@debian.org>
// This software comes with ABSOLUTELY NO WARRANTY
// This software is free software, and you are welcome to redistribute it
// under certain conditions
// See the COPYING file for details.

#include "xhandler.h"

//--------------------------------------------------------------------
XHandler::XHandler()
{
  is_wmaker_ = WINDOWMAKER;
  is_ushape_ = USESHAPE;
  is_astep_  = AFTERSTEP;

  strcpy(display_name_, "");
  strcpy(position_name_, "");
  strcpy(ledcolor_name_, LEDCOLOR);
  strcpy(ledcolor_high_name_, LEDCOLOR_HIGH);
  strcpy(backcolor_name_, BACKCOLOR);

  button_state_ = 0;
}

//--------------------------------------------------------------------
XHandler::~XHandler()
{
  XFreeGC(display_default_, graphics_context_);
  XFreePixmap(display_default_, pixmap_main);
  XFreePixmap(display_default_, pixmap_tile);
  XFreePixmap(display_default_, pixmap_disp);
  XFreePixmap(display_default_, pixmap_mask);
  XFreePixmap(display_default_, pixmap_icon);
  XFreePixmap(display_default_, pixmap_nrec);

  XDestroyWindow(display_default_, window_main_);

  if(is_wmaker_)
    XDestroyWindow(display_default_, window_icon_);

  XCloseDisplay(display_default_);
  
  delete[] icon_list_;
}


//--------------------------------------------------------------------
void XHandler::init(int argc, char** argv, int num_channels)
{    
  int display_depth;

  window_size_=is_astep_ ? ASTEPSIZE : NORMSIZE;
  
  if((display_default_ = XOpenDisplay(display_name_))==NULL) 
    {
      std::cerr <<  NAME << " : Unable to open X display '" << XDisplayName(display_name_) << "'." << std::endl;
      exit(1);
    }
  
  initWindow(argc, argv);

  initColors();

  display_depth = DefaultDepth(display_default_, DefaultScreen(display_default_));
  initPixmaps(display_depth);

  initGraphicsContext();

  initMask();

  initIcons(num_channels);
}


//--------------------------------------------------------------------
bool XHandler::isLeftButton(int x, int y)
{
  return(x>=BTN_LEFT_X && y>=BTN_LEFT_Y && x<=BTN_LEFT_X + BTN_WIDTH && y<=BTN_LEFT_Y + BTN_HEIGHT);
}

//--------------------------------------------------------------------
bool XHandler::isRightButton(int x, int y)
{
  return(x>=BTN_RIGHT_X && y>=BTN_RIGHT_Y && x<=BTN_RIGHT_X + BTN_WIDTH && y<=BTN_RIGHT_Y + BTN_HEIGHT);
}

//--------------------------------------------------------------------
bool XHandler::isMuteButton(int x, int y)
{
  return(x>=BTN_MUTE_X && y>=BTN_MUTE_Y && x<=BTN_MUTE_X + BTN_WIDTH && y<=BTN_MUTE_Y + BTN_HEIGHT);
}

//--------------------------------------------------------------------
bool XHandler::isRecButton(int x, int y)
{
  return(x>=BTN_REC_X && y>=BTN_REC_Y && x<=BTN_REC_X + BTN_WIDTH && y<=BTN_REC_Y + BTN_HEIGHT);
}

//--------------------------------------------------------------------
bool XHandler::isVolumeBar(int x, int y)
{
  return(x>=37 && x<=56 && y>=8 && y<=56);
}

//--------------------------------------------------------------------
unsigned long XHandler::getColor(char *colorname)
{
  XColor color;
  XWindowAttributes winattr;

  XGetWindowAttributes(display_default_, window_root_, &winattr);
  color.pixel=0;
  XParseColor(display_default_, winattr.colormap, colorname, &color);
  color.flags=DoRed | DoGreen | DoBlue;
  XAllocColor(display_default_, winattr.colormap, &color);

  return color.pixel;
}

//--------------------------------------------------------------------
unsigned long XHandler::mixColor(char *colorname1, int prop1, char *colorname2, int prop2)
{
  XColor color, color1, color2;
  XWindowAttributes winattr;

  XGetWindowAttributes(display_default_, window_root_, &winattr);

  XParseColor(display_default_, winattr.colormap, colorname1, &color1);
  XParseColor(display_default_, winattr.colormap, colorname2, &color2);

  color.pixel = 0;
  color.red   = (color1.red*prop1+color2.red*prop2)/(prop1+prop2);
  color.green = (color1.green*prop1+color2.green*prop2)/(prop1+prop2);
  color.blue  = (color1.blue*prop1+color2.blue*prop2)/(prop1+prop2);
  color.flags = DoRed | DoGreen | DoBlue;

  XAllocColor(display_default_, winattr.colormap, &color);

  return color.pixel;
}

//--------------------------------------------------------------------
void XHandler::repaint() 
{
  flush_expose(window_icon_);
  XCopyArea(display_default_, pixmap_disp, window_icon_, graphics_context_, 0, 0, 64, 64, window_size_/2-32, window_size_/2-32);
  flush_expose(window_main_);
  XCopyArea(display_default_, pixmap_disp, window_main_, graphics_context_, 0, 0, 64, 64, window_size_/2-32, window_size_/2-32);
  
  XEvent xev;
  while(XCheckTypedEvent(display_default_, Expose, &xev));
}

//--------------------------------------------------------------------
void XHandler::update(unsigned channel) 
{
  if(is_wmaker_ || is_ushape_ || is_astep_) 
    {
      XShapeCombineMask(display_default_, window_icon_, ShapeBounding, window_size_/2-32, window_size_/2-32, pixmap_mask, ShapeSet);
      XShapeCombineMask(display_default_, window_main_, ShapeBounding, window_size_/2-32, window_size_/2-32, pixmap_mask, ShapeSet);
    } 
  else 
    {
      XCopyArea(display_default_, pixmap_tile, pixmap_disp, graphics_context_, 0, 0, 64, 64, 0, 0);
    }
  
  XSetClipMask(display_default_, graphics_context_, pixmap_mask);
  XCopyArea(display_default_, pixmap_main, pixmap_disp, graphics_context_, 0, 0, 64, 64, 0, 0);
  XSetClipMask(display_default_, graphics_context_, None);
  XCopyArea(display_default_, pixmap_icon, pixmap_disp, graphics_context_, icon_list_[channel]*22, 0, 22, 22, 6, 5);
}

//--------------------------------------------------------------------
void XHandler::drawLeft(unsigned curleft) 
{
  XSetForeground(display_default_, graphics_context_, shade_colors_[(curleft*25)/100]);
  for(unsigned i=0;i<25;i++)
    {
      if(i >= (curleft*25)/100)
	{
	  XSetForeground(display_default_, graphics_context_, colors_[3]);
	}
      else
	{
	  XSetForeground(display_default_, graphics_context_, shade_colors_[i]);
	}
      XFillRectangle(display_default_, pixmap_disp, graphics_context_, 37, 55-2*i, 9, 1);
    }
}

//--------------------------------------------------------------------
void XHandler::drawRight(unsigned curright) 
{
  for(unsigned i=0;i<25;i++)
    {
      if(i >= (curright*25)/100)
	{
	  XSetForeground(display_default_, graphics_context_, colors_[3]);
	}
      else
	{
	  XSetForeground(display_default_, graphics_context_, shade_colors_[i]);
	}
      XFillRectangle(display_default_, pixmap_disp, graphics_context_, 48, 55-2*i, 9, 1);
    }
}

//--------------------------------------------------------------------
// Based on wmsmixer by Damian Kramer <psiren@hibernaculum.demon.co.uk>
void XHandler::drawMono(unsigned curright)
{
  XSetForeground(display_default_, graphics_context_, colors_[1]);
  for(unsigned i=0;i<25;i++)
    {
      if(i >= (curright*25)/100)
	{
	  XSetForeground(display_default_, graphics_context_, colors_[3]);
	}
      else
	{
	  XSetForeground(display_default_, graphics_context_, shade_colors_[i]);
	}
      XFillRectangle(display_default_, pixmap_disp, graphics_context_, 37, 55-2*i, 20, 1);
    }
}


//--------------------------------------------------------------------
void XHandler::drawBtns(int buttons, bool curshowrec) 
{
  if(buttons & BTNPREV)
    drawButton(BTN_LEFT_X, BTN_LEFT_Y, BTN_WIDTH, BTN_HEIGHT, (button_state_ & BTNPREV));

  if(buttons & BTNNEXT)
    drawButton(BTN_RIGHT_X, BTN_RIGHT_Y, BTN_WIDTH, BTN_HEIGHT, (button_state_ & BTNNEXT));

  if(buttons & BTNMUTE)
    drawButton(BTN_MUTE_X, BTN_MUTE_Y, BTN_WIDTH, BTN_HEIGHT, (button_state_ & BTNMUTE));

  if(buttons & BTNREC){
    drawButton(BTN_REC_X, BTN_REC_Y, BTN_WIDTH, BTN_HEIGHT, (button_state_ & BTNREC));
    
    if(!curshowrec)
      XCopyArea(display_default_, pixmap_nrec, pixmap_disp, graphics_context_, 0, 0, 9, 8, 6, 47);
    else
      XCopyArea(display_default_, pixmap_main, pixmap_disp, graphics_context_, 6, 48, 9, 8, 6, 47);
  }
}

//--------------------------------------------------------------------
void XHandler::drawButton(int x, int y, int w, int h, bool down) 
{
  if(!down)
    XCopyArea(display_default_, pixmap_main, pixmap_disp, graphics_context_, x, y, w, h, x, y);
  else {
    XCopyArea(display_default_, pixmap_main, pixmap_disp, graphics_context_, x, y, 1, h-1, x+w-1, y+1);
    XCopyArea(display_default_, pixmap_main, pixmap_disp, graphics_context_, x+w-1, y+1, 1, h-1, x, y);
    XCopyArea(display_default_, pixmap_main, pixmap_disp, graphics_context_, x, y, w-1, 1, x+1, y+h-1);
    XCopyArea(display_default_, pixmap_main, pixmap_disp, graphics_context_, x+1, y+h-1, w-1, 1, x, y);
  }
}

//--------------------------------------------------------------------
int XHandler::flush_expose(Window w) 
{
  XEvent dummy;
  int i=0;
  
  while (XCheckTypedWindowEvent(display_default_, w, Expose, &dummy))
    i++;
  
  return i;
}


//--------------------------------------------------------------------
int XHandler::getWindowSize()
{
  return window_size_;
}

//--------------------------------------------------------------------
// --> inline
//Display* XHandler::getDisplay()
//{
//  return display_default_;
//}


//--------------------------------------------------------------------
int XHandler::getButtonState()
{
  return button_state_;
}

//--------------------------------------------------------------------
void XHandler::setButtonState(int button_state)
{
  button_state_ = button_state;
}

//--------------------------------------------------------------------
void XHandler::setDisplay(char* arg)
{
  sprintf(display_name_, "%s", arg);
}

//--------------------------------------------------------------------
void XHandler::setPosition(char* arg)
{
  sprintf(position_name_, "%s", arg);
}

//--------------------------------------------------------------------
void XHandler::setLedColor(char* arg)
{
  sprintf(ledcolor_name_, "%s", arg);
}

//--------------------------------------------------------------------
void XHandler::setLedHighColor(char* arg)
{
  sprintf(ledcolor_high_name_, "%s", arg);
}

//--------------------------------------------------------------------
void XHandler::setBackColor(char* arg)
{
  sprintf(backcolor_name_, "%s", arg);
}

//--------------------------------------------------------------------
void XHandler::setUnshaped()
{
  is_ushape_ = 1;
}

//--------------------------------------------------------------------
void XHandler::setWindowMaker()
{
  is_wmaker_ = 1;
}

//--------------------------------------------------------------------
void XHandler::setAfterStep()
{
  is_astep_ = 1;
}

//--------------------------------------------------------------------
Atom XHandler::getDeleteWin()
{
  return deleteWin;
}


//--------------------------------------------------------------------
void XHandler::initIcons(int num) 
{
  if(icon_list_)
    delete[] icon_list_;
  
  icon_list_ = new unsigned[num];
  
  icon_list_[0] = 0;
  icon_list_[1] = 7;
  icon_list_[2] = 8;
  icon_list_[3] = 2;
  icon_list_[4] = 1;
  icon_list_[5] = 6;
  icon_list_[6] = 4;
  icon_list_[7] = 5;
  icon_list_[8] = 3;
  for(int counter=9; counter<num; counter++)
    icon_list_[counter] = 9;
}

//--------------------------------------------------------------------
void XHandler::initGraphicsContext()
{
  XGCValues gcv;
  unsigned long gcm;

  gcm = GCForeground | GCBackground | GCGraphicsExposures;
  gcv.graphics_exposures = 0;
  gcv.foreground = fore_pix;
  gcv.background = back_pix;
  graphics_context_ = XCreateGC(display_default_, window_root_, gcm, &gcv);  
}

//--------------------------------------------------------------------
void XHandler::initPixmaps(int display_depth)
{
  XpmColorSymbol xpmcsym[4]={{"back_color",     NULL, colors_[0]},
			     {"led_color_high", NULL, colors_[1]},
			     {"led_color_med",  NULL, colors_[2]},
			     {"led_color_low",  NULL, colors_[3]}};
  XpmAttributes xpmattr;

  xpmattr.numsymbols   = 4;
  xpmattr.colorsymbols = xpmcsym;
  xpmattr.exactColors  = false;
  xpmattr.closeness    = 40000;
  xpmattr.valuemask    = XpmColorSymbols | XpmExactColors | XpmCloseness;
  
  XpmCreatePixmapFromData(display_default_, window_root_, wmmixer_xpm, &pixmap_main, &pixmap_mask, &xpmattr);
  XpmCreatePixmapFromData(display_default_, window_root_, tile_xpm, &pixmap_tile, NULL, &xpmattr);
  XpmCreatePixmapFromData(display_default_, window_root_, icons_xpm, &pixmap_icon, NULL, &xpmattr);
  XpmCreatePixmapFromData(display_default_, window_root_, norec_xpm, &pixmap_nrec, NULL, &xpmattr);

  pixmap_disp = XCreatePixmap(display_default_, window_root_, 64, 64, display_depth);
}


//--------------------------------------------------------------------
void XHandler::initWindow(int argc, char** argv)
{
  char *wname = argv[0];
  int screen, dummy = 0;
  XWMHints wmhints;
  XSizeHints shints;
  XClassHint classHint;
  XTextProperty	name;

  screen = DefaultScreen(display_default_);
  _XA_GNUSTEP_WM_FUNC = XInternAtom(display_default_, "_GNUSTEP_WM_FUNCTION", false);
  deleteWin = XInternAtom(display_default_, "WM_DELETE_WINDOW", false);


  shints.x = 0;
  shints.y = 0;
  //  shints.flags  = USSize;
  shints.flags  = 0; // Gordon
  
  bool pos = (XWMGeometry(display_default_, DefaultScreen(display_default_),
			  position_name_, NULL, 0, &shints, &shints.x, &shints.y,
			  &shints.width, &shints.height, &dummy)
	      & (XValue | YValue));
  shints.min_width   = window_size_;
  shints.min_height  = window_size_;
  shints.max_width   = window_size_;
  shints.max_height  = window_size_;
  shints.base_width  = window_size_;
  shints.base_height = window_size_;
  shints.width       = window_size_;
  shints.height      = window_size_;
  shints.flags=PMinSize | PMaxSize | PBaseSize; // Gordon


  window_root_ = RootWindow(display_default_, screen);

  back_pix = getColor("white");
  fore_pix = getColor("black");

  window_main_ = XCreateSimpleWindow(display_default_, window_root_, shints.x, shints.y,
			    shints.width, shints.height, 0, fore_pix, back_pix);
  
  window_icon_ = XCreateSimpleWindow(display_default_, window_root_, shints.x, shints.y,
				shints.width, shints.height, 0, fore_pix, back_pix);
  
  XSetWMNormalHints(display_default_, window_main_, &shints);
  
  
  wmhints.icon_x = shints.x;
  wmhints.icon_y = shints.y;
  
  if(is_wmaker_ || is_astep_ || pos)
    shints.flags |= USPosition;

  if(is_wmaker_)
    {
      wmhints.initial_state = WithdrawnState;
      wmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
      wmhints.icon_window = window_icon_;
      
      wmhints.icon_x = shints.x;
      wmhints.icon_y = shints.y;
      wmhints.window_group = window_main_;
    } 
  else 
    {
      wmhints.initial_state = NormalState;
      wmhints.flags = WindowGroupHint | StateHint;
    }
  
  classHint.res_name=NAME;
  classHint.res_class=CLASS;
  
  XSetClassHint(display_default_, window_main_, &classHint);
  XSetClassHint(display_default_, window_icon_, &classHint);
   
  
  if (XStringListToTextProperty(&wname, 1, &name) == 0)
    {
      std::cerr << wname << ": can't allocate window name" << std::endl;
      exit(1);
    }
  
  XSetWMName(display_default_, window_main_, &name);
  XSetWMHints(display_default_, window_main_, &wmhints);
  XSetCommand(display_default_, window_main_, argv, argc);
  XSetWMProtocols(display_default_, window_main_, &deleteWin, 1); // Close
}


//--------------------------------------------------------------------
// Initialize main colors and shaded color-array for bars
void XHandler::initColors()
{    
  colors_[0] = mixColor(ledcolor_name_, 0,   backcolor_name_, 100);
  colors_[1] = mixColor(ledcolor_name_, 100, backcolor_name_, 0);
  colors_[2] = mixColor(ledcolor_name_, 60,  backcolor_name_, 40);
  colors_[3] = mixColor(ledcolor_name_, 25,  backcolor_name_, 75);

  for(int count=0; count<25; count++)
    {
      shade_colors_[count] = mixColor(ledcolor_high_name_, count*2, ledcolor_name_, 100-count*4);
    }
}


//--------------------------------------------------------------------
void XHandler::initMask()
{    
  XSetClipMask(display_default_, graphics_context_, pixmap_mask);
  XCopyArea(   display_default_, pixmap_main, pixmap_disp, graphics_context_, 0, 0, 64, 64, 0, 0);
  XSetClipMask(display_default_, graphics_context_, None);
  XStoreName(  display_default_, window_main_, NAME);
  XSetIconName(display_default_, window_main_, NAME); 

  if(is_wmaker_ || is_ushape_ || is_astep_) 
    {
      XShapeCombineMask(display_default_, window_icon_, ShapeBounding, window_size_/2-32, window_size_/2-32, pixmap_mask, ShapeSet);
      XShapeCombineMask(display_default_, window_main_, ShapeBounding, window_size_/2-32, window_size_/2-32, pixmap_mask, ShapeSet);
    } 
  else 
    {
      XCopyArea(display_default_, pixmap_tile, pixmap_disp, graphics_context_, 0, 0, 64, 64, 0, 0);
    }
  
  XSelectInput(display_default_, window_main_, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
  XSelectInput(display_default_, window_icon_, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
  XMapWindow(display_default_, window_main_);
}

