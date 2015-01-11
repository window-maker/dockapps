/*  wmnet -- X IP accounting monitor
 *  Copyright 1998, 2000 Jesse B. Off, Katharine Osborne
 *  <kaos@digitalkaos.net>
 *
 *  $Id: wmnet.h,v 1.2 1998/10/06 00:06:12 joff Exp $
 *
 *  This software is released under the GNU Public License agreement.
 *  No warranties, whatever.... you know the usuals.... this is free
 *  software.  if you use it, great... if you wanna make a change to it,
 *  great, but please send me the diff.  If you put it on a distributed
 *  CD, great... I'd appreciate a copy of the CD though ;).
 *
 */


#define TOPBOX_X             4
#define TOPBOX_Y             4
#define TOPBOX_WIDTH                          56
#define TOPBOX_HEIGHT                         11

/* Graphing area extents */
#define GRAPHBOX_X           4
#define GRAPHBOX_Y           15
#define GRAPHBOX_WIDTH       56
#define GRAPHBOX_HEIGHT      graphbox_height

#define GRAPHBOX_X_RIGHT     (GRAPHBOX_X + GRAPHBOX_WIDTH - 1)
#define GRAPHBOX_X_LEFT      (GRAPHBOX_X)
#define GRAPHBOX_Y_TOP       (GRAPHBOX_Y)
#define GRAPHBOX_Y_BOTTOM    (GRAPHBOX_Y + GRAPHBOX_HEIGHT - 1)

/* Graphing area minus the borders */
#define GRAPH_X		     (GRAPHBOX_X + 1)
#define GRAPH_Y              (GRAPHBOX_Y)
#define GRAPH_WIDTH          (GRAPHBOX_WIDTH - 2)
#define GRAPH_HEIGHT         (GRAPHBOX_HEIGHT - 1)

#define GRAPH_X_LEFT            (GRAPH_X)
#define GRAPH_X_RIGHT           (GRAPH_X + GRAPH_WIDTH - 1)
#define GRAPH_Y_UPPER            (GRAPH_Y)
#define GRAPH_Y_BOTTOM           (GRAPH_Y + GRAPH_HEIGHT - 1)


/* Label area box */
#define LABEL_X              (GRAPHBOX_X_LEFT)
#define LABEL_Y              (GRAPHBOX_Y_BOTTOM + 1)
#define LABEL_WIDTH          (GRAPHBOX_WIDTH)
#define LABEL_HEIGHT         11

#define LABEL_X_LEFT         (LABEL_X_LEFT)
#define LABEL_X_RIGHT        (LABEL_X_LEFT + LABEL_WIDTH - 1)
#define LABEL_Y_TOP          (LABEL_Y)
#define LABEL_Y_BOTTOM       (LABEL_Y + LABEL_HEIGHT - 1)


#define LOW_INTENSITY		0
#define NORMAL_INTENSITY	1
#define HIGH_INTENSITY		2





/* X Stuff */
Display *dpy;
Window root_window, main_window, icon_window, *visible_window;
Font thefont;
Pixmap arrow;
Atom delete_atom;
int screen, specified_state = -1;
GC graphics_context;
unsigned long tx_pixel[3], rx_pixel[3], labelfg_pixel, labelbg_pixel,  black_pixel, white_pixel, darkgrey_pixel, grey_pixel;
typedef int (*parser_func)(void);

/* I know statically declared buffers are against GNU coding standards, so sue me */
char buffer[256], *click_command = NULL, *label = NULL;
struct timeval timenow, timelast;
unsigned long long int totalbytes_in, totalbytes_out, lastbytes_in, lastbytes_out;
unsigned long long int totalpackets_in, totalpackets_out, lastpackets_in, lastpackets_out;
unsigned int diffbytes_in, diffbytes_out;
unsigned int delayTime = 100000, displayDelay = 55000, maxRate = 120000;
unsigned int out_rule = 2, in_rule = 1, graphbox_height = 44;  /* number of rule in /proc/net/ip_acct to use */
char *in_rule_string = NULL, *out_rule_string = NULL, *device=NULL;
Bool current_tx = False, current_rx = False, rx, tx, logscale = False;
parser_func stat_gather;



void exit_func(void);
void got_signal(int x);
void setup_wmnet(int argc, char **argv);
void setupX(void);
void createWin(Window *win);
int updateStats(void);
void redraw(XExposeEvent *ee);
void tock(void);
int updateSpeedometer(int rxRate, int txRate);
void drawColoredLine(int y1, int y2, unsigned long *shadecolor);
void shadesOf(XColor *shade, unsigned long *returnarray);

extern parser_func setup_driver(char *parser);
extern char * available_drivers(void);

