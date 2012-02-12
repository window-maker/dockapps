/****************************************************************************/
/* wmckgmail 1.1 : A dockapp to monitor the number of unread mails in a     */
/*                 gmail inbox                                              */
/* Author        : Sylvain Tremblay <stremblay@gmail.com>                   */
/* Release date  : Sep 09, 2006                                             */
/****************************************************************************/

/* -------------------- */
/* Defines              */
/* -------------------- */
#define _GNU_SOURCE
#define DEBUG 0
#define COUNTERWINDOW 0
#define BIG_M_ 0

/* -------------------- */
/* Includes             */
/* -------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"

#include "wmckgmail.xpm"

/* -------------------- */
/* Functions prototypes */
/* -------------------- */
void writeString(char* sString, int iWhichWindow);
void writeChar(char cTheChar, int iWhichWindow, int iPos);
void writeNum(char cTheNum, int iWhichWindow, int iPos);
void trace(char *sMsg);
int  getNewMailsCount(char *sAtomFilePath);
void getAtomFile(void);
void initialize(void);
 
/* -------------------- */
/* Global variables     */
/* -------------------- */
char wmckgmail_mask_bits[64*64]; /* matrix for the XPM */
int  wmckgmail_mask_width = 64;  
int  wmckgmail_mask_height = 64;
char sWorkDir[1024];   /* to store the directory where wmckgmail works */
char sHomeDir[1024];   /* to store the home directory of the user */
char sAtomFile[1024];  /* to store the path to the atom file */
char sConfigFile[1024];/* to store the path of the configuration file */
char sBrowserCmd1[1024];/* to store the command line #1 to launch the browser */
char sBrowserCmd2[1024];/* to store the command line #2 to launch the browser */
char sUname[20];       /* to store the gmail username */
char sPass[20];        /* to store the gmail password */
int iPollInterval;     /* to store the poll interval (in seconds) */
time_t lastPoll;       /* to store the time of the last incoming mails verification */

Display *display;
int screen_num;

static char *progname;

/* -------------------- */
/* main()               */
/* -------------------- */
int main(int argc, char **argv){
  XEvent Event;
  int but_stat = -1;
  int i;
  char sMsg[1024];  /* used to create tracing messages */

  progname = argv[0];

  createXBMfromXPM(wmckgmail_mask_bits, wmckgmail, wmckgmail_mask_width, wmckgmail_mask_height);
  openXwindow(argc, argv, wmckgmail, wmckgmail_mask_bits, wmckgmail_mask_width, wmckgmail_mask_height);
  
  /* Define the mouse regions */
  AddMouseRegion(BIG_M_, 16, 9, 46, 31);

  RedrawWindow();

  /* Read config file and initialize variables */
  initialize();

  /* Do a first verification out of the main loop */
  getAtomFile();
  getNewMailsCount(sAtomFile);

  /* event loop */

  while(1){
    RedrawWindow();
    
    /* Update every iPollInterval seconds */
    if((time(NULL) - lastPoll) >= (time_t) iPollInterval){
      getAtomFile();
      getNewMailsCount(sAtomFile);

      if(DEBUG) system("date\necho \"-----\"");
    }

    while(XPending(display)){
      XNextEvent(display, &Event);
      switch(Event.type){
        case Expose:
          RedrawWindow();
          break;

        case DestroyNotify:
          XCloseDisplay(display);
          exit(0); 

        case ButtonPress:
          i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
          but_stat = i;
          sprintf(sMsg, "button pressed! region index is: %i\n", but_stat); trace(sMsg);
          break;

        case ButtonRelease:
          i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
          if(but_stat == i && but_stat >= 0){
 
            sprintf(sMsg, "button released! region index is: %i\n", i); trace(sMsg);

            switch(but_stat){
              case BIG_M_:
                /* action when the big _M_ is pressed */
                /*system("mozilla -remote \"openURL(http://www.gmail.com)\"");*/
                trace("\nClicked the big M ! Verifying if there is an action to do...\n");
                if(strcmp(sBrowserCmd1, "")){
                  trace("'browsercmd1' has been defined, trying the command...\n");
                  if(system(sBrowserCmd1)){
                    trace("Command failed.\n");
                    if(strcmp(sBrowserCmd2, "")){
                      trace("'browsercmd2 has been defined, trying the command...\n");
                      system(sBrowserCmd2);
                    }
                  } else {
                    trace("Command succed (returned 0)\n"); 
                  }
                } else {
                  trace("No browsercmd1 nor browsercmd2 defined, nothing to do.\n");
                }
                trace("\n");
                break;;

              case 1:
                /* action for region 1 */
                break;;
            }
          }
          but_stat = -1;
          break;
      }
    }
    usleep(100000);
  }
}

/* ----------------------------------------------------------------- */
/* Function : writeString                                            */
/* Usage    : Write a string in a specific window                    */
/*                                                                   */
/* Arguments: sString      -> The string to write                    */
/*            iWhichWindow -> The window to write to                 */
/*                                                                   */
/* Returns  : Nothing                                                */
/* ----------------------------------------------------------------- */
void writeString(char* sString, int iWhichWindow){
  int i;

  /* loop to process each character of the string and call the right */
  /* function according if the character is alphabetical or numerical */
  for(i = 0; i < strlen(sString); i++){
    if(sString[i] >= '0' && sString[i] <= '9'){
      writeNum(sString[i], iWhichWindow, i);
    } else {
      writeChar(sString[i], iWhichWindow, i);
    }
  }
}

/* ----------------------------------------------------------------- */
/* Function : writeChar                                              */
/* Usage    : Write a character in a specific window at a specific   */
/*            position                                               */
/*                                                                   */
/* Arguments: cTheChar     -> Character to write                     */
/*            iWhichWindow -> The window to write to                 */
/*            iPos         -> The position of the character          */
/*                                                                   */
/* Returns  : Nothing                                                */
/* ----------------------------------------------------------------- */
void writeChar(char cTheChar, int iWhichWindow, int iPos){
  int iLetterIdx; /* The alphebetical index of the letter (A=0, B=1, ...) */
  int iSrcX;      /* The X coordinate in the XPM of the letter to copy */
  int iSrcY;      /* The Y coordinate in the XPM of the letter to copy */
  int iDestX;     /* The X coordinate in the XPM where we want to copy the letter */
  int iDestY;     /* The Y coordinate in the XPM where we want to copy the letter */
  int iWidth;     /* The width of each letter in the XPM */
  int iHeight;    /* The height of each letter in the XPM */
  int iMaxPos;    /* The maximum value for the position of the letter to write */

  /* Set the variables according to the window in which we want to print a letter */
  if(iWhichWindow == COUNTERWINDOW){
    iSrcX = 1;
    iSrcY = 64;
    iDestX = 23;
    iDestY = 39;
    iWidth = 6;
    iHeight = 9;
    iMaxPos = 2;
  } else {
    return;
  }

  /* If trying to write pass the iMaxPos position, exit the function */
  if(iPos > iMaxPos){
    return;
  }

  /* Clear the actual position in case of something is already there */
  copyXPMArea((iSrcX + (26 * iWidth)), iSrcY, iWidth, iHeight, (iDestX + (iPos * iWidth)), iDestY);

  /* Get the index of the letter we want to print */
  iLetterIdx = (toupper(cTheChar) - 'A');

  /* If the index is NOT between 0 and 25, exit the function */
  /* Using an invalid character is also the way to "erase" a letter */
  if(iLetterIdx < 0 || iLetterIdx > 25){
    return;
  }
  
  /* Let's write this letter at the right place! */
  copyXPMArea((iSrcX + (iLetterIdx * iWidth)), iSrcY, iWidth, iHeight, (iDestX + (iPos * iWidth)), iDestY);
}


/* ----------------------------------------------------------------- */
/* Function : writeNum                                               */
/* Usage    : Write a number in a specific window at a specific      */
/*            position                                               */
/*                                                                   */
/* Arguments: cTheNum      -> Number to write                        */
/*            iWhichWindow -> The window to write to                 */
/*            iPos         -> The position of the number             */
/*                                                                   */
/* Returns  : Nothing                                                */
/* ----------------------------------------------------------------- */
void writeNum(char cTheNum, int iWhichWindow, int iPos){
  int iNumberIdx; /* The numerical index of the number */
  int iSrcX;      /* The X coordinate in the XPM of the number to copy */
  int iSrcY;      /* The Y coordinate in the XPM of the number to copy */
  int iDestX;     /* The X coordinate in the XPM where we want to copy the number */
  int iDestY;     /* The Y coordinate in the XPM where we want to copy the number */
  int iWidth;     /* The width of each number in the XPM */
  int iHeight;    /* The height of each number in the XPM */
  int iMaxPos;    /* The maximum value for the position of the number to write */

  /* Set the variables according to the window in which we want to print a number */
  if(iWhichWindow == COUNTERWINDOW){
    iSrcX = 1;
    iSrcY = 76;
    iDestX = 23;
    iDestY = 39;
    iWidth = 6;
    iHeight = 9;
    iMaxPos = 9;
  } else {
    return;
  }

  /* If trying to write pass the iMaxPos position, exit the function */
  if(iPos > iMaxPos){
    return;
  }

  /* Clear the actual position in case of something is already there */
  copyXPMArea((iSrcX + (10 * iWidth)), iSrcY, iWidth, iHeight, (iDestX + (iPos * iWidth)), iDestY);

  /* Get the index of the number we want to print */
  iNumberIdx = (cTheNum - '0');

  /* If the index is NOT between 0 and 9, exit the function */
  /* Using an invalid character is also a way to "erase" a number */
  if(iNumberIdx < 0 || iNumberIdx > 9){
    return;
  }
  
  /* Let's write this letter at the right place! */
  copyXPMArea((iSrcX + (iNumberIdx * iWidth)), iSrcY, iWidth, iHeight, (iDestX + (iPos * iWidth)), iDestY);
}

/* ----------------------------------------------------------------- */
/* Function : trace                                                  */
/* Usage    : Output traces to stdout if the DEBUG #define is not    */
/*            NULL.                                                  */
/*                                                                   */
/* Arguments: sMsg  -> The message to output                         */
/*                                                                   */
/* Returns  : Nothing                                                */
/* ----------------------------------------------------------------- */
void trace(char *sMsg){
  if(DEBUG){
    printf("%s", sMsg);
  }
}

/* ----------------------------------------------------------------- */
/* Function : getNewMailsCount                                       */
/* Usage    : Read the number of new mails from the atom file        */
/*                                                                   */
/* Arguments: sAtomFilePath -> Path to the atom file                 */
/*                                                                   */
/* Returns  : Number of new mails                                    */
/* ----------------------------------------------------------------- */
int getNewMailsCount(char *sAtomFilePath){
  FILE*  fAtomFile;              /* The file pointer to the atom file */
  int    fdAtomFile;             /* The file descriptor of the atom file */
  int    iNewMails       = 0;    /* Return the number of new mails through this var */
  char   sNewMails[5];           /* To contain the string version of the number of new mails */
  int    iLength;                /* used for the length of the strinc containing the new mails count */
  struct stat statBuffer;        /* Struct to contain the infos about the atom file */
  char*  sAtomBuffer     = NULL; /* buffer that will contain the content of the atom file */
  char*  sAtomBufferPtr1 = NULL; /* pointer used to parse the atom buffer */
  char*  sAtomBufferPtr2 = NULL; /* pointer used to parse the atom buffer */
  char   sMsg[1024];             /* buffer used to write trace messages */

  /* try to open the atom file */
  sprintf(sMsg, "Atom file path: %s\n", sAtomFilePath); trace(sMsg);
  fAtomFile = fopen(sAtomFilePath, "r");
  if (fAtomFile == NULL){
    /* unable to open atom file, put "ERR: in the new mesages counter window */
    trace("Error opening atom file.\n");
    iNewMails = -1;
    writeString("ERR", COUNTERWINDOW);

  } else {
    /* atom file opened!Get its size */
    stat(sAtomFilePath, &statBuffer);
    fdAtomFile = fileno(fAtomFile);
    sprintf(sMsg, "Atom file size : %i\n", (int) statBuffer.st_size); trace(sMsg);

    /* allocate memory for a buffer the size of the atom file */
    sAtomBuffer = malloc(((size_t) statBuffer.st_size) + 1);

    /* read the content of the atom file and terminate the string in the buffer */
    read(fdAtomFile, sAtomBuffer, (size_t) statBuffer.st_size);
    sAtomBuffer[((int) statBuffer.st_size)] = '\0';

    /* find the <fullcount> and </fullcount> tags in the buffer then isolate the middle string */
    sAtomBufferPtr1 = strstr(sAtomBuffer, "<fullcount>");
    sAtomBufferPtr2 = strstr(sAtomBuffer, "</fullcount>");
    iLength = (sAtomBufferPtr2 - (sAtomBufferPtr1 + 11));
    strncpy(sNewMails, sAtomBufferPtr1 + 11, iLength);
    sNewMails[iLength] = '\0';
    sprintf(sMsg, "New mails count : %s\n", sNewMails); trace(sMsg);
    
    /* for the return value... not really used but who knows the future... :P */
    iNewMails = atoi(sNewMails);

    /* If new mails counter == 0 -> grey the icon, else put it back in red */
    if(iNewMails == 0){
      copyXPMArea(74, 7, 31, 23, 16, 9);
    } else {
      copyXPMArea(74, 33, 31, 23, 16, 9);
    }

    /* re-create the sNewMails string putting the required '0' at the beginning */
    sprintf(sNewMails, "%03i\n", iNewMails);

    /* Update the counter */
    writeString(sNewMails, COUNTERWINDOW);

    /* close the atom file */
    fclose(fAtomFile);

    /* release the memory allocated for the atom file */
    free(sAtomBuffer);
  }

  return(iNewMails);
}

/* ----------------------------------------------------------------- */
/* Function : getAtomFile                                            */
/* Usage    : use wget to grab the atom file from gmail server       */
/*                                                                   */
/* Arguments: None                                                   */
/*                                                                   */
/* Returns  : Nothing                                                */
/* ----------------------------------------------------------------- */
void getAtomFile(void){
  char   sWgetRcFile[1024];  /* to contain the path of the .wgetrc file */
  FILE   *fWgetRcFile;       /* pointer to the .wgetrc file */
  int    iWgetRcFd;          /* to contain the file descriptor of the .wgetrc file */
  int    iWgetRC;            /* the return code of the wget command */
  int    iRcFileExists = 0;  /* flag to know if there was an exising .wgetrc file */
  struct stat statBuf;       /* the buffer for the call of the stat command */
  char   sBuff[1024];        /* buffer */
  char   sMsg[1024];         /* used for tracing messages purpose */

  /* set the lastPoll value to the current time */
  lastPoll = time(NULL);

  /* test if there is already a .wgetrc file. If so, move it temporarily */
  sprintf(sWgetRcFile, "%s/.wgetrc", sHomeDir);
  sprintf(sMsg, ".wgetrc file path : %s\n", sWgetRcFile); trace(sMsg);
  if(!stat(sWgetRcFile, &statBuf)){
    /* if stat returned 0, the file exists, process to the move */
    iRcFileExists = 1;
    trace("The .wgetrc file exists, move it temporarily\n");
    sprintf(sBuff, "%s/.wgetrc", sWorkDir);
    rename(sWgetRcFile, sBuff); 
  } else {
    trace("No .wgetrc file found, good thing! :P\n");
  }

  /* open the .wgetrc file for writing */
  fWgetRcFile = fopen(sWgetRcFile, "w");
  if(fWgetRcFile != NULL){
    iWgetRcFd = fileno(fWgetRcFile);

    /* write its required content (gmail username and password) then close it */
    sprintf(sBuff, "--user=%s\n--password=%s\n", sUname, sPass);
    write(iWgetRcFd, sBuff, strlen(sBuff));

    fclose(fWgetRcFile);

    /* prepare the WGET call (supress output when not in debugging mode) */
    if(DEBUG){
      sprintf(sBuff, 
              "wget --no-check-certificate https://mail.google.com/mail/feed/atom -O %s/atom",
               sWorkDir);
    } else {
      sprintf(sBuff, 
              "wget -q --no-check-certificate https://mail.google.com/mail/feed/atom -O %s/atom", 
               sWorkDir);
    }
 
    /* call wget */
    iWgetRC = system(sBuff);

    /* if the command returns a non-zero integer, something failed, delete the atom file */
    if(iWgetRC != 0){
      remove(sAtomFile);
    }

    /* erase the .wgetrc file */
    remove(sWgetRcFile);

    /* If we backed-up an existing .wgetrc file, bring it back */
    if(iRcFileExists){
      trace("Brining back original .wgetrc file\n");
      sprintf(sBuff, "%s/.wgetrc", sWorkDir);
      rename(sBuff, sWgetRcFile);
    }
  } else {
    trace("Unable to open .wgetrc file for writing\n");
  }
 /* system("cp /tmp/atom /home/sig/.wmckgmail/atom"); */
}

/* ----------------------------------------------------------------- */
/* Function : initialize                                             */
/* Usage    : Read the config file, initialize global variables, ... */
/*                                                                   */
/* Arguments: None                                                   */
/*                                                                   */
/* Returns  : Nothing                                                */
/* ----------------------------------------------------------------- */
void initialize(void){
  char sMsg[1024];       /* buffer used for tracing purposes */
  FILE *fCfgFile;        /* File pointer to the config file */
  size_t len = 0;        /* used by the getline function */
  char *sBuff;           /* buffer used by the getline function */
  int iGetLineRes;       /* to get the result code of the getline function */
  char sAttr[20];        /* to contain attributes names read in the config file */
  char sVal[256];        /* to contain attributes values read in the config file */
  char sPollInterval[5]; /* where we keep the poll interval in string form */

  trace("----- Initializing application -----\n");

  /* Get the home directory of the user */
  strcpy(sHomeDir, getenv("HOME"));
  sprintf(sMsg, "User home directory : %s\n", sHomeDir); trace(sMsg);

  /* Set the working directory */
  sprintf(sWorkDir, "%s/.wmckgmail", sHomeDir);
  sprintf(sMsg, "Working directory   : %s\n", sWorkDir); trace(sMsg);

  /* Set the path of the atom file */
  sprintf(sAtomFile, "%s/atom", sWorkDir);
  sprintf(sMsg, "Atom file path      : %s\n", sAtomFile); trace(sMsg);

  /* Set the path of the config file */
  sprintf(sConfigFile, "%s/config", sWorkDir);
  sprintf(sMsg, "Config file path    : %s\n", sConfigFile); trace(sMsg);

  /* try to open the config file */
  fCfgFile = fopen(sConfigFile, "r");
  if(fCfgFile == NULL){
    /* config file not found, abort the program */
    sprintf(sMsg, "** ERROR ** Cannot open config file : \"%s\"\n            Program aborted.", sConfigFile);
    printf("\n%s\n\n", sMsg); 
    exit(1);

  } else {
    /* ok, let's now read the config file and set the attributes read from it */
    trace("\nReading config file :\n");

    iGetLineRes = getline(&sBuff, &len, fCfgFile);
    while(iGetLineRes != EOF){
      sprintf(sMsg, "  _____\n  Read line : %s", sBuff); trace(sMsg);
      sscanf(sBuff, "%s %s", sAttr, sVal);
      sprintf(sMsg, "  Attribute : %s\n  Value     : %s\n", sAttr, sVal); trace(sMsg);

      if(!strcmp(sAttr, "uname")){
        strcpy(sUname, sVal);  
        trace("  * Got username!\n"); 
      } else if(!strcmp(sAttr, "pass")){
        strcpy(sPass, sVal);
        trace("  * Got password!\n");
      } else if(!strcmp(sAttr, "pollinterval")){
        trace("  * Got poll interval!\n");
        iPollInterval = atoi(sVal);
        if(iPollInterval < 1 || iPollInterval > 6000) {
          trace("    - Warning - Value not understood, setting to 60 seconds\n"); 
          iPollInterval = 300;
        } else {
          strcpy(sPollInterval, sVal);
        }
      } else if(!strcmp(sAttr, "browsercmd1")){

        char* ptr = NULL;
        /* use sBuff because sAttr is not the full line */
        sprintf(sMsg, "  Full line : %s", (sBuff + (strlen(sAttr)) + 1)); trace(sMsg);
        trace("  * Got browser launch command #1 !\n");
        trace("\n  Searching for the $@ string in the line\n");
        if((ptr = strchr(sBuff, (int) '$')) != NULL){
          trace("  Found '$' ! Now verifying if followed by '@'\n");
          /* found '$', verify if followed by '@' (to instruct to replace $@ by gmail url)*/
          if((ptr + 1)[0] == '@'){
              /*system("mozilla -remote \"openURL(http://www.gmail.com)\"");*/
            trace("  Found '@' ! let's replace it by http://www.gmail.com\n");
            strncpy(sBrowserCmd1, (sBuff + (strlen(sAttr) + 1)), 
                                  ((ptr - 1) - (sBuff + strlen(sAttr))));
            sBrowserCmd1[ptr - sBuff] = '\0';
            sprintf(sBrowserCmd1, "%s%s%s", sBrowserCmd1, "http://www.gmail.com", 
                                           sBuff + strlen(sBrowserCmd1) + strlen(sAttr) + 3);
          } else {
            strcpy(sBrowserCmd1, ptr);
          }
        } else {
          strcpy(sBrowserCmd1, (sBuff + (strlen(sAttr) + 1)));
        }
        sprintf(sMsg, "  Browser Command #1 is : %s\n", sBrowserCmd1); trace(sMsg);

      } else if(!strcmp(sAttr, "browsercmd2")){

        char* ptr = NULL;
        /* use sBuff because sAttr is not the full line */
        sprintf(sMsg, "  Full line : %s", (sBuff + (strlen(sAttr)) + 1)); trace(sMsg);
        trace("  * Got browser launch command #2 !\n");
        trace("\n  Searching for the $@ string in the line\n");
        if((ptr = strchr(sBuff, (int) '$')) != NULL){
          trace("  Found '$' ! Now verifying if followed by '@'\n");
          /* found '$', verify if followed by '@' (to instruct to replace $@ by gmail url)*/
          if((ptr + 1)[0] == '@'){
              /*system("mozilla -remote \"openURL(http://www.gmail.com)\"");*/
            trace("  Found '@' ! let's replace it by http://www.gmail.com\n");
            strncpy(sBrowserCmd2, (sBuff + (strlen(sAttr) + 1)), 
                                  ((ptr - 1) - (sBuff + strlen(sAttr))));
            sBrowserCmd2[ptr - sBuff] = '\0';
            sprintf(sBrowserCmd2, "%s%s%s", sBrowserCmd2, "http://www.gmail.com", 
                                           sBuff + strlen(sBrowserCmd2) + strlen(sAttr) + 3);
          } else {
            strcpy(sBrowserCmd2, ptr);
          }
        } else {
          strcpy(sBrowserCmd2, (sBuff + (strlen(sAttr) + 1)));
        }
        sprintf(sMsg, "  Browser Command #2 is : %s\n", sBrowserCmd2); trace(sMsg);

      } else {
        trace("  ** Warning ** Line not understood, skipped.\n");
      }

      iGetLineRes = getline(&sBuff, &len, fCfgFile);
    } 
    trace("\n");

    /* Verify that we got all the required parameters from the config file */
    trace("Validating config file parameters\n\n");
    if(!strcmp(sUname, "")){
      printf("  ** ERROR ** Username not found in config file.\n            Program aborted.\n");
      exit(2);
    }

    if(!strcmp(sPass, "")){
      printf("  ** ERROR ** Password not found in config file.\n            Program aborted.\n");
      exit(3);
    }

    if(!strcmp(sPollInterval, "")){
      printf("  ** Warning ** Poll interval not defined, setting to 5 minutes.\n\n");
      iPollInterval = 300;
    }

    /* release getline buffer memory */
    free(sBuff);
  }

  trace("----- Initialization completed -----\n");
}
