/* wmymail.c - mail checking dockapp */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <utime.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <libdockapp/dockapp.h>

#include "xpm/main.xpm"
#include "xpm/numbers.xpm"
#include "xpm/unumbers.xpm"
#include "xpm/mbox_1.xpm"
#include "xpm/mbox_2.xpm"
#include "xpm/mbox_3.xpm"

/*
 * Some definitions required by libdockapp
 */

#define NAME      "wmymail"
#define VERSION   "wmymail v0.3  November 6, 2004"

// default mail check interval, in seconds (default) or minutes (with -F)
#define CHECKINTERVAL  1

// global data

char *displayName = "";
char *mailPath = NULL;
char *fontColor = "";
char *background = "";
char *clickcommand = "";
char *newcommand = "";
int numMessages = 0;
int lastnumMessages = 0;
int numRead = 0;
int numUnread = 0;
int lastnumUnread = 0;
int buttonpressed = 0;

/*
 * usefetchmail means to run "fetchmail -c" and parse the output, rather
 * than counting up messages in an mbox file.

 * It will change the interval from seconds to minutes.
 */

int usefetchmail = 0;
int flip = 1;
int checkInterval = CHECKINTERVAL;
time_t lastModifySeconds = 0;
off_t lastSize = 0;

Pixmap mainPixmap;
Pixmap numbersPixmap;
Pixmap unumbersPixmap;
Pixmap mboxonePixmap;
Pixmap mboxtwoPixmap;
Pixmap mboxthreePixmap;
Pixmap outPixmap1;
Pixmap outPixmap2;
GC defaultGC;

static DAProgramOption options[] = {

    {"-display", NULL, "display to use",
        DOString, False, {&displayName}},

    {"-i", "--interval", "seconds between mailbox checks (default 1)",
        DONatural, False, {&checkInterval} },

    {"-fc", "--fontcolor", "custom font color",
        DOString, False, {&fontColor} },

    {"-bg", "--background", "custom background color for non-shaped window",
        DOString, False, {&background} },

    {"-ns", "--noshape", "make the dock app non-shaped (windowed)",
        DONone, False, {NULL} },

    {"-F", "--fetchmail", "check with fetchmail -c instead of the mbox",
        DONone, False, {NULL} },

    {"-c", "--command", "command to run when clicked",
        DOString, False, {&clickcommand} },

    {"-n", "--newcommand", "command to run when new mail is received",
        DOString, False, {&newcommand} },

    {"-m", "--mailbox", "mailbox to use when $MAIL is not set",
        DOString, False, {&mailPath} }

};

// prototypes

void checkForNewMail(int dummy);
void updatePixmap(void);
void parseMailFile( struct stat *fileStat );
char *getHexColorString( char *colorName );
void putnumber (int number, Pixmap pixmap, Pixmap numbers,
  int destx, int desty);
void buttonpress(int button, int state, int x, int y);
void buttonrelease(int button, int state, int x, int y);
void checkfetchmail (void);
void checkmbox (void);
void launch (const char *command);

// functions

int main(int argc, char **argv) {
  Pixmap mainPixmap_mask;

  unsigned width, height;

  DACallbacks callbacks = { NULL, &buttonpress, &buttonrelease,
                            NULL, NULL, NULL, NULL };

  struct sigaction sa;

  sa.sa_handler = SIG_IGN;
#ifdef SA_NOCLDWAIT
  sa.sa_flags = SA_NOCLDWAIT;
#else
  sa.sa_flags = 0;
#endif
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, NULL);


  DAParseArguments(argc, argv, options,
                   sizeof(options) / sizeof(DAProgramOption),
                   NAME, VERSION);

  DAInitialize(displayName, "wmymail", 64, 64, argc, argv);

  // simple recoloring of the raw xpms befor creating Pixmaps of them
  // this works as long as you don't "touch" the images...

  if (options[2].used) { // custom font color ?
    char *colorLine = strdup("+      c #");

    strcat(colorLine, getHexColorString(fontColor));

    colorLine = strdup("+      c #");
    strcat(colorLine, getHexColorString(fontColor));
     numbers_xpm[3] = colorLine;
  }

  if (options[3].used && options[8].used) { // custom window background ?
    char *colorLine = strdup("       c #");

    strcat(colorLine, getHexColorString(background));
    main_xpm[1] = colorLine;
  }

  DAMakePixmapFromData(main_xpm, &mainPixmap, &mainPixmap_mask, &width, &height);
  DAMakePixmapFromData(numbers_xpm, &numbersPixmap, NULL, &width, &height);
  DAMakePixmapFromData(unumbers_xpm, &unumbersPixmap, NULL, &width, &height);

  DAMakePixmapFromData(mbox_1_xpm, &mboxonePixmap, NULL, &width, &height);
  DAMakePixmapFromData(mbox_2_xpm, &mboxtwoPixmap, NULL, &width, &height);
  DAMakePixmapFromData(mbox_3_xpm, &mboxthreePixmap, NULL, &width, &height);

  if (!options[4].used) // no shape to install
    DASetShape(mainPixmap_mask);

  if (options[5].used)  // use fetchmail
    usefetchmail = 1;
  else if (mailPath == NULL) {
    if ((mailPath = getenv("MAIL")) == NULL) {
      perror("Please define your MAIL environment variable!\n");
      exit(1);
    }
  }

  DASetCallbacks( &callbacks );
  DASetTimeout(-1);

  outPixmap1 = DAMakePixmap();
  outPixmap2 = DAMakePixmap();
  defaultGC = XDefaultGC(DADisplay, 0);

  signal(SIGALRM, checkForNewMail);

  updatePixmap();

  DAShow();

  checkForNewMail(0);

  DAEventLoop();

  return 0;
}

char *getHexColorString(char *colorName) {
  XColor color;
  char *hexColorString;

  if (!XParseColor(DADisplay,
      DefaultColormap(DADisplay, DefaultScreen( DADisplay)),
        colorName, &color))
    {
      printf("unknown colorname: \"%s\"\n", colorName);
      exit(1);
    }

  hexColorString = (char *)malloc(7);
  sprintf(hexColorString, "%02X%02X%02X", color.red>>8, color.green>>8,
        color.blue>>8);

  return hexColorString;
}

  /*
   *
   * checkForNewMail
   *
   */

void checkForNewMail(int dummy) {
  struct itimerval timerVal;

  if (usefetchmail) {
    checkfetchmail();
  } else {
    checkmbox();
  }

  if (numMessages != lastnumMessages ||
      numUnread != lastnumUnread) {
    updatePixmap();
    if (numUnread > lastnumUnread && strlen(newcommand) > 0)
        launch(newcommand);
    lastnumMessages = numMessages;
    lastnumUnread = numUnread;
  }

  memset(&timerVal, 0, sizeof(timerVal));

  if (usefetchmail) {
    timerVal.it_value.tv_sec = checkInterval * 60;
  } else {
    timerVal.it_value.tv_sec = checkInterval;
  }

  setitimer(ITIMER_REAL, &timerVal, NULL);
}

  /*
   *
   *  checkfetchmail
   *
   */

void checkfetchmail (void) {
   int msgtotal = 0;
   int msgseen = 0;
   int snpret;
   char tmpfile[20] = "wmymail.XXXXXX";
   char syscmd[120];
   char line[1024];
   char *s, *t;
   int fd;
   FILE *f;


   fd = mkstemp(tmpfile);
   if (fd == -1) {
      perror("wmymail: cannot get a temporay file");
      return;
   }

   snpret = snprintf(syscmd, 120, "fetchmail -c > %s", tmpfile);
   if (snpret < 0) {
      perror("wmymail: error in snprintf() call (should not happen)");
      return;
   }

   if (system(syscmd) < 0) {
      perror("wmymail: error when using system() to run fetchmail -c");
      return;
   }

   f = fdopen(fd, "r");
   if (f == NULL) {
      perror("wmymail: can't reread tempfile\n");
      return;
   }

   /* FIXME: this assumes that fetchmail will never print a line over
    * 1024 characters long, which is fairly safe but you never know */
   while (fgets(line, 1024, f) != NULL) {

      /* Every line beginning with a number is assumed to be a number of
       * messages on the server:
       *
       *   "1 message for userfoo at mail.bar.org."
       *   "3 messages for userfoo at mail.bar.org." */
      if (line[0] >= '0' && line[0] <= '9') {

         /* The first number on the line may be added to the total */
         msgtotal += atoi(line);

         /* Fetchmail may also indicate that some of the messages on the
          * server have already been read:
          *
          *    "5 messages (3 seen) for userfoo at mail.bar.org."  */

         /* To get the number seen, locate the first space */
         s = (char *)strstr(line, " ");
         if (s != NULL) {

            /* Skip over one character */
            s++;

            /* And locate the second space */
            t = (char *)strstr(s, " ");

            /* If this second space is followed by '(' and a digit, it's
             * a number of seen messages */
            if (t != NULL && t[1] == '(' && t[2] >= '0' && t[2] <= '9') {

               /* Position string t on the number seen */
               t += 2;

               /* And get the number */
               msgseen += atoi(t);
            }
         }
      }
   }

   fclose(f);
   remove(tmpfile);

   /* Now that that's been gotten through without major errors,
      move the values to the global variables */

   numMessages = msgtotal;
   numUnread = msgtotal - msgseen;
}
  /*
   *
   *  checkmbox
   *
   */

void checkmbox (void) {
  struct stat fileStat;

  if (stat(mailPath, &fileStat) == -1 || fileStat.st_size == 0) {
    numMessages = 0;
    numUnread = 0;
  } else if (lastModifySeconds != fileStat.st_mtime ||
             lastSize != fileStat.st_size) {

    parseMailFile(&fileStat);

    lastModifySeconds = fileStat.st_mtime;
    lastSize = fileStat.st_size;
  }
}

  /*
   *
   * updatePixmap
   *
   */

void updatePixmap(void) {
  Pixmap outPixmap = flip ? outPixmap1 : outPixmap2;

  flip = !flip;

  XCopyArea(DADisplay, mainPixmap, outPixmap, defaultGC,
            0, 0, 64, 64, 0, 0);

  if (numMessages > 998) {
    putnumber(999, outPixmap, numbersPixmap, 40, 49);
  } else {
    putnumber(numMessages, outPixmap, numbersPixmap, 40, 49);
  }

  if (numUnread > 998) {
    putnumber(999, outPixmap, unumbersPixmap, 6, 49);
  } else if (!numUnread) {
    putnumber(0, outPixmap, numbersPixmap, 6, 49);
  } else {
    putnumber(numUnread, outPixmap, unumbersPixmap, 6, 49);
  }

  if (numUnread == 0) {
    // do nothing.
  } else if (numUnread == 1) {
    XCopyArea(DADisplay, mboxonePixmap, outPixmap, defaultGC,
      0, 0, 40, 34, 14, 6);
  } else if (numUnread == 2) {
    XCopyArea(DADisplay, mboxtwoPixmap, outPixmap, defaultGC,
      0, 0, 40, 34, 14, 6);
  } else {
    XCopyArea(DADisplay, mboxthreePixmap, outPixmap, defaultGC,
      0, 0, 40, 34, 14, 6);
  }

  DASetPixmap(outPixmap);
}

/*
 *
 * putnumber -- draw a number
 *
 */

void putnumber (
      int number,             /* what value should be displayed */
      Pixmap pixmap,          /* pixmap to draw upon */
      Pixmap numbers,         /* pixmap with digit images to use */
      int destx, int desty    /* upper-left corner of rectangle to draw in */
      ) {

   int digit1, digit2, digit3;

   /* Determine the digits */
   digit1 = number / 100;
   digit2 = (number % 100) / 10;
   digit3 = number % 10;

   /* The 100s and 10s digits will only be displayed if the number
      is >99 and >9, respectively */

   if (digit1) XCopyArea(DADisplay, numbers, pixmap, defaultGC,
      digit1 * 5, 0, 5, 9, destx, desty);

   if (digit2 || digit1) XCopyArea(DADisplay, numbers, pixmap, defaultGC,
      digit2 * 5, 0, 5, 9, destx + 6, desty);

   XCopyArea(DADisplay, numbers, pixmap, defaultGC,
      digit3 * 5, 0, 5, 9, destx + 12, desty);
}

/*
 * parseMailFile -- reads the mail file and sets the global variables:
 *
 *    numMessages   --  total number of messages  (displayed on the right)
 *    numRead       --  messages that have been read
 *    numUnread     --  message not yet read      (displayed on the left)
 */

void parseMailFile(struct stat *fileStat) {
   char buf[1024];
   int inHeader = 0;
   int statusRead = 0;
   int longline = 0;
   FILE *f = fopen(mailPath, "r");  /* FIXME check for failure to open */

   numMessages = 0;
   numRead = 0;

   while (fgets(buf, 1024, f) != NULL) {

      /* Keep discarding data if a line over 1024 characters long was found */
      if (longline) {
         longline = index(buf, '\n') != NULL;

      } else {
         /* The "From" line is the marker of an individual message */
         if(!strncmp(buf, "From ", 5)) {
            inHeader = 1;
            numMessages++;

         /* Once inside a header, it only remains to
          * 1) Take note, if the message appears to have been read
          * 2) Locate the end of the header  */
         } else if (inHeader) {

            /* A blank line indicates the end of the header */
            if (!strcmp(buf, "\n")) {
               inHeader = 0;
               if (statusRead) {
                  numRead++;
                  statusRead = 0;
               }

            /* The "Status" line indicates that the message has been read,
             * if it has a "R".  But since we don't trust that there will
             * be only one "Status" line, statusRead will be set to 1,
             * but numRead will only be incremented after the header has
             * been completely read.  That way, multiple "Status" lines
             * would only set statusRead to 1 multiple times (having no
             * effect). */
            } else if (!strncmp(buf, "Status: ", 8) && strchr(buf, 'R')) {
               statusRead = 1;
            }
         }

      /* The 1024 byte buffer can easily be exceeded by long lines...
       * when no newline is present, we must enter the state of "skipping
       * over the rest of a very long line".  Else a line inside the body
       * of a message might be (starting at the 1025th character)
       * "From <foo@bar.org>\n" thus fooling this program into parsing it
       * incorrectly. */
      longline = index(buf, '\n') == NULL;
      }
   }

   fclose(f);
   numUnread = numMessages - numRead;
}

/* Take note of a mouse button being pressed inside the dock app */
void buttonpress (int button, int state, int x, int y) {
    buttonpressed = 1;
}

/* A mouse button was pressed and released.
 * See if it was released while the mouse was still in the bounds of
 * the dock app (a 64x64 square). */
void buttonrelease (int button, int state, int x, int y) {
    if (buttonpressed && x > 0 && x < 64 && y > 0 && y < 64 &&
            strlen(clickcommand) > 0) {
        launch(clickcommand);
    }
    buttonpressed = 0;
}

/* Start another program */
void launch (const char *command) {
    int cpid;

    cpid = fork();
    if (cpid == -1) {
        perror("can't fork");
    } else if (cpid == 0) {
        system(command);
        exit(0);
    }
}

