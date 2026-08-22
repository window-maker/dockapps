/* wmbutton.h - Edward H. Flora - ehf_dockapps@cox.net */
/* Last Modified 3/27/04 */

/******  Include Files ***************************************************/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/keysym.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>

/******  Define Config File Info  ***************************************/
#define CONFFILENAME  "/.wmbutton"   /* Default conf filename $HOME/.wmbutton */
#define CONFIGGLOBAL "/etc/wmbutton.conf" /* system configuration */
#define BUTTONFILENAME "/.wmbutton.xpm"

/******  Define Error Codes *********************************************/
#define FAILDISP      20
#define FAILSWIN      21
#define FAILICON      22
#define FAILXPM       23
#define FAILWNAM      24
#define FAILGC        25
#define FAILCONF      26
#define FAILTMPL      27
#define FAILVIS       28
#define FAILBUT       29

/******   Define Other Options  ****************************************/
#define VERB          0      /* Enable=1, Disable=0: Debugging (verbose) Mode*/
#define LMASK         0      /* left button mask: run app # mask + button #*/
#define MMASK         10     /* middle button mask: run app # mask + button #*/
#define RMASK         20     /* right button mask: run app # mask + button #*/
#define NUMB_OF_APPS  9      /* Define number of apps */

#define EOLN '\n'	     /* Defines the new line character */
#define SIZE1 20	     /* Defines the increment to increase the */
			     /* string by until a newline of EOF is found */

/******   Defines for Tool Tips  ***************************************/
#define TOOLTIP_SUPPORT      1
#define TOOLTIP_FONT         "-*-helvetica-medium-r-normal-*-10-*-*-*-*-*-*-*"
#define TOOLTIP_FONT_LEN     128
#define TOOLTIP_SHOW_DELAY   750
#define TOOLTIP_RESHOW_DELAY 1500

#define TOOLTIP_SPACE   12
#define TOOLTIP_TOP	 0
#define TOOLTIP_BOTTOM	 1
#define TOOLTIP_LEFT	 0
#define TOOLTIP_RIGHT	 2

#define BUTTON_SIZE     18
#define BUTTON_COLS      3

#define BUFFER_SIZE 1024

/******  Typedefs  *******************************************/

struct Config_t {
	char *configfile;
	char *buttonfile;
	char *Geometry_str;
	char *Display_str;
	int mmouse;
	int Verbose;
	char* szTooltipFont;
	int bTooltipSwapColors;
	int bTooltipDisable;
        int bigicon;
};

/******  Function Prototyes  *******************************************/
void RunAppN(int app);             /* function to run app N as found in conf file */
char *Parse(int app);              /* parse data in config file */
void parseargs(int argc, char **argv);
char *readln(FILE *fp);            /* read line from file, return pointer to it */
void err_mess(int err, char *str); /* Error Handling Routine */
void show_usage(void);             /* show usage message to stderr */
int canOpenFile(const char *path);
int flush_expose(Window w);


/******  Tooltip Function Prototypes  **********************************/
void initTooltip(void);
void destroyTooltip(void);
int hasTooltipSupport(void);
void showTooltip(int nButton, int nMouseX, int nMouseY);
void hideTooltip(void);
int hasTooltip(void);
void drawTooltipBalloon(Pixmap pix, GC gc, int x, int y, int w, int h, int side);
Pixmap createTooltipPixmap(int width, int height, int side, Pixmap *mask);

void initTime(void);
long currentTimeMillis(void);
void getWindowOrigin(Window w, int *nX, int *nY);
void getButtonLocation(int nButton, int *nLocationX, int *nLocationY);
char *getButtonAppNames(int nButton);

/**********************************************************************/
