
#ifndef YAWMPPP_ISPRC_H
#define YAWMPPP_ISPRC_H

#define KEY_LONGNAME     "LongName"
#define KEY_SHORTNAME    "ShortName"
#define KEY_STARTACTION  "StartAction"
#define KEY_STOPACTION   "StopAction"
#define KEY_SPEEDACTION  "SpeedAction"
#define KEY_IFDOWNACTION "IfDownAction"
#define KEY_USER         "User"
#define KEY_PHONE        "Phone"

#define KEY_PPP_OVER            "PPPOptionsOverride"
#define KEY_PPP_DEFAULTROUTE    "PPPDefaultRoute"
#define KEY_PPP_PASSIVE         "PPPPassive"
#define KEY_PPP_NOAUTH          "PPPNoAuth"
#define KEY_PPP_NOIPDEFAULT     "PPPNoIPDefault"

#define KEY_PPP_CHAP            "PPPChap"
#define KEY_PPP_PAP             "PPPPap"

#define KEY_NOLOGIN             "NoLogin"

#define KEY_PPPSTUFF            "PPPLine"
#define KEY_CHATSTUFF           "ChatFile"

#define AUTH_REQUIRE  0
#define AUTH_REFUSE   1
#define AUTH_DONTCARE 2

struct ISP_PPP {
  int override;
  int defaultroute;
  int passive;
  int noauth;
  int noipdefault;
  int chap;
  int pap;
  int usepeerdns;
};

struct YAWMPPP_ISP_INFO {
  char LongName[128];
  char ShortName[16];
  char StartAction[512];
  char StopAction[512];
  char SpeedAction[512];
  char IfDownAction[512];
  char PPPLine[512];
  char ChatFile[512];
  char User[32];
  char Phone[32];

  struct ISP_PPP ppp;

  int nologin;
};

int
GetISPInfo(char *rcname,struct YAWMPPP_ISP_INFO *wii,int max);

#endif

