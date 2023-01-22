
/*
 * DOCKLIB - a simple dockapp library
 *
 * Copyright (C) 2000-2005 some people...
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

extern Display      *display;

/* prefs file mode */
#define P_READ              1
#define P_WRITE             2

#define MAX_LINE_LEN        512
#define MAX_VALUE_LEN       256

#define MAX_STRING_LEN      32

/* mouse buttons */
#define LMB                 1
#define MMB                 2
#define RMB                 3

#define MAX_MOUSE_REGION    8

#define FONT_SMALL          1
#define FONT_NORMAL         2
#define FONT_LARGE          3

/*-------------------------------------------------------------------------------*/

typedef struct {
    int     enable;
    int     top;
    int     bottom;
    int     left;
    int     right;
} MOUSE_REGION;


typedef struct {
    Pixmap          pixmap;
    Pixmap          mask;
    XpmAttributes   attributes;
} XpmIcon;

#define NUM_COLORS  10

enum {
    F_NOT_AVAILABLE = 0,
    F_REGULAR,
    F_DIRECTORY,
    F_CHAR_DEVICE,
    F_BLOCK_DEVICE,
    F_LINK,
    F_FIFO,
    F_SOCK
} file_types;

/*-------------------------------------------------------------------------------*/

void    dcl_open_x_window       (int argc, char *argv[], char **, char *, int, int);
void    dcl_redraw_window       (void);
void    dcl_redraw_window_xy    (int x, int y);
void    dcl_copy_xpm_area       (int x, int y, int sx, int sy, int dx, int dy);
void    dcl_copy_font_xpm_area  (int x, int y, int sx, int sy, int dx, int dy);
void    dcl_copy_led_xpm_area   (int x, int y, int sx, int sy, int dx, int dy);
void    dcl_copy_xbm_area       (int x, int y, int sx, int sy, int dx, int dy);
void    dcl_set_mask_xy         (int x, int y);
void    dcl_get_xpm             (XpmIcon *wmgen, char **pixmap_bytes);
Pixel   dcl_get_color           (char *name);
int     dcl_draw_char           (int x, int y, char z, int font_type);
int     dcl_draw_led            (int x, int y, int color);
int     dcl_draw_string         (int x, int y, char *string, int font_type, int length);
void    dcl_add_mouse_region    (int index, int left, int top, int right, int bottom);
int     dcl_check_mouse_region  (int x, int y);
char*   dcl_getfilename_config  (char *config_dir, char *config_filename);
void*   dcl_prefs_openfile      (char *filename, int openmode);
void    dcl_prefs_closefile     (void);
void    dcl_prefs_put_int       (char *tagname, int value);
void    dcl_prefs_put_float     (char *tagname, float value);
void    dcl_prefs_put_string    (char *tagname, char *value);
void    dcl_prefs_put_lf        (void);
void    dcl_prefs_put_comment   (char *comment);
int     dcl_prefs_get_int       (char *tagname);
float   dcl_prefs_get_float     (char *tagname);
char*   dcl_prefs_get_string    (char *tagname);
char*   dcl_strcpy              (char *dest, const char *src, int maxlength);
char*   dcl_strcat              (char *dest, const char *src, int maxlength);
void    dcl_execute_command     (char *command, int flag);
int     dcl_check_file          (char *filename);
void    dcl_draw_point          (int x, int y, Pixel color);
void    dcl_draw_line           (int x0, int y0, int x1, int y1, Pixel color);


/*
 * Created:       Sat 26 Mar 2005 05:46:11 PM CET
 * Last Modified: Sat 26 Mar 2005 10:53:50 PM CET
 */

