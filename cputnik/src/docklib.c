
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

#include "docklib.h"

/******************************************************************************
 * variables
 ******************************************************************************/

XSizeHints      mysizehints;
XWMHints        mywmhints;
Window          Root, iconwin, win;
Display         *display;
GC              NormalGC;
XpmIcon         wmgen, fonts_wmgen, leds_wmgen;
Pixmap          pixmask;
Pixel           back_pix, fore_pix;
int             screen, x_fd, d_depth;
MOUSE_REGION    mouse_region[MAX_MOUSE_REGION];

FILE            *prefs_filehandle;
char            *Geometry = "";
char            *fonts_xpm[], *leds_xpm[];


/******************************************************************************
 *
 * dcl_get_xpm
 *
 ******************************************************************************/

void dcl_get_xpm (XpmIcon *wmgen, char *pixmap_bytes[])
{
XWindowAttributes   attributes;
int                 err;

    /* For the colormap */
    XGetWindowAttributes(display, Root, &attributes);

    wmgen->attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);

    err = XpmCreatePixmapFromData(display, Root, pixmap_bytes, &(wmgen->pixmap),
                    &(wmgen->mask), &(wmgen->attributes));

    if (err != XpmSuccess) {
        fprintf(stderr, "ERROR: Not enough free colorcells.\n");
        exit(1);
    }

}

/******************************************************************************
 *
 * dcl_get_color
 *
 ******************************************************************************/

Pixel dcl_get_color (char *name)
{
XColor              color;
XWindowAttributes   attributes;

    XGetWindowAttributes(display, Root, &attributes);

    color.pixel = 0;
    if (!XParseColor(display, attributes.colormap, name, &color)) {
        fprintf(stderr, "ERROR: can't parse %s.\n", name);
    } else if (!XAllocColor(display, attributes.colormap, &color)) {
        fprintf(stderr, "ERROR: can't allocate %s.\n", name);
    }

    return color.pixel;
}

/******************************************************************************
 *
 * dcl_flush_expose
 *
 ******************************************************************************/

int dcl_flush_expose (Window w)
{
XEvent  dummy;
int     i=0;

    while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
        i++;

    return i;
}

/******************************************************************************
 *
 * dcl_redraw_window
 *
 ******************************************************************************/

void dcl_redraw_window (void)
{

    dcl_flush_expose(iconwin);
    XCopyArea(display, wmgen.pixmap, iconwin, NormalGC,
                0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
    dcl_flush_expose(win);
    XCopyArea(display, wmgen.pixmap, win, NormalGC,
                0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}

/******************************************************************************
 *
 * dcl_redraw_window_xy
 *
 ******************************************************************************/

void dcl_redraw_window_xy (int x, int y)
{

    dcl_flush_expose(iconwin);
    XCopyArea(display, wmgen.pixmap, iconwin, NormalGC,
                x,y, wmgen.attributes.width, wmgen.attributes.height, 0,0);
    dcl_flush_expose(win);
    XCopyArea(display, wmgen.pixmap, win, NormalGC,
                x,y, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}

/******************************************************************************
 *
 * dcl_add_mouse_region
 *
 ******************************************************************************/

void dcl_add_mouse_region (int index, int left, int top, int right, int bottom)
{
    if (index < MAX_MOUSE_REGION) {
        mouse_region[index].enable = 1;
        mouse_region[index].top = top;
        mouse_region[index].left = left;
        mouse_region[index].bottom = bottom;
        mouse_region[index].right = right;
    }
}

/******************************************************************************
 *
 * dcl_check_mouse_region
 *
 ******************************************************************************/

int dcl_check_mouse_region (int x, int y)
{
int i, found;

    found = 0;

    for (i=0; i<MAX_MOUSE_REGION && !found; i++) {
        if (mouse_region[i].enable &&
            x <= mouse_region[i].right &&
            x >= mouse_region[i].left &&
            y <= mouse_region[i].bottom &&
            y >= mouse_region[i].top)
            found = 1;
    }

    if (!found) return -1;
    return (i-1);
}

/******************************************************************************
 *
 * dcl_copy_xpm_area
 *
 ******************************************************************************/

void dcl_copy_xpm_area (int x, int y, int sx, int sy, int dx, int dy)
{

    XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);

}


/******************************************************************************
 *
 * dcl_copy_font_xpm_area
 *
 ******************************************************************************/

void dcl_copy_font_xpm_area (int x, int y, int sx, int sy, int dx, int dy)
{
    XCopyArea(display, fonts_wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);

}


/******************************************************************************
 *
 * dcl_copy_led_xpm_area
 *
 ******************************************************************************/

void dcl_copy_led_xpm_area (int x, int y, int sx, int sy, int dx, int dy)
{

    XCopyArea(display, leds_wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);

}

/******************************************************************************
 *
 * dcl_copy_xbm_area
 *
 ******************************************************************************/

void dcl_copy_xbm_area (int x, int y, int sx, int sy, int dx, int dy)
{
    XCopyArea(display, wmgen.mask, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}


/******************************************************************************
 *
 * dcl_set_mask_xy
 *
 ******************************************************************************/

void dcl_set_mask_xy (int x, int y)
{
     XShapeCombineMask(display, win, ShapeBounding, x, y, pixmask, ShapeSet);
     XShapeCombineMask(display, iconwin, ShapeBounding, x, y, pixmask, ShapeSet);
}

/******************************************************************************
 *
 * dcl_open_x_window
 *
 ******************************************************************************/

void dcl_open_x_window (int argc, char *argv[], char *pixmap_bytes[], char *pixmask_bits, int pixmask_width, int pixmask_height)
{
unsigned int    borderwidth = 1;
XClassHint      classHint;
char            *display_name = NULL;
char            *wname = argv[0];
XTextProperty   name;
XGCValues       gcv;
unsigned long   gcm;
int             dummy=0, i, flags;

    for (i=1; argv[i]; i++) {
        if (!strcmp(argv[i], "-display"))
            display_name = argv[i+1];
    }

    if (!(display = XOpenDisplay(display_name))) {
        fprintf(stderr, "ERROR: can't open display %s\n", XDisplayName(display_name));
        exit(1);
    }

    screen  = DefaultScreen(display);
    Root    = RootWindow(display, screen);
    d_depth = DefaultDepth(display, screen);
    x_fd    = XConnectionNumber(display);

    /* Convert XPM to XImage */
    dcl_get_xpm(&wmgen, pixmap_bytes);
    dcl_get_xpm(&fonts_wmgen, fonts_xpm);
    dcl_get_xpm(&leds_wmgen, leds_xpm);

    /* Create a window to hold the stuff */
    mysizehints.flags = USSize | USPosition;
    mysizehints.x = 0;
    mysizehints.y = 0;

    back_pix = dcl_get_color("white");
    fore_pix = dcl_get_color("black");

    XWMGeometry(display, screen, Geometry, NULL, borderwidth, &mysizehints,
                &mysizehints.x, &mysizehints.y,&mysizehints.width,&mysizehints.height, &dummy);

    mysizehints.width = 64;
    mysizehints.height = 64;

    win = XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,
                mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

    iconwin = XCreateSimpleWindow(display, win, mysizehints.x, mysizehints.y,
                mysizehints.width, mysizehints.height, borderwidth, fore_pix, back_pix);

    /* Activate hints */
    XSetWMNormalHints(display, win, &mysizehints);
    classHint.res_name = wname;
    classHint.res_class = wname;
    XSetClassHint(display, win, &classHint);

    flags = ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

    XSelectInput(display, win, flags);
    XSelectInput(display, iconwin, flags);

    if (XStringListToTextProperty(&wname, 1, &name) == 0) {
        fprintf(stderr, "ERROR: can't allocate window name\n");
        exit(1);
    }

    XSetWMName(display, win, &name);

    /* Create GC for drawing */

    gcm = GCForeground | GCBackground | GCGraphicsExposures;
    gcv.foreground = fore_pix;
    gcv.background = back_pix;
    gcv.graphics_exposures = 0;
    NormalGC = XCreateGC(display, Root, gcm, &gcv);

    /* ONLYSHAPE ON */

    pixmask = XCreateBitmapFromData(display, win, pixmask_bits, pixmask_width, pixmask_height);

    XShapeCombineMask(display, win, ShapeBounding, 0, 0, pixmask, ShapeSet);
    XShapeCombineMask(display, iconwin, ShapeBounding, 0, 0, pixmask, ShapeSet);

    /* ONLYSHAPE OFF */

    mywmhints.initial_state = WithdrawnState;
    mywmhints.icon_window = iconwin;
    mywmhints.icon_x = mysizehints.x;
    mywmhints.icon_y = mysizehints.y;
    mywmhints.window_group = win;
    mywmhints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;

    XSetWMHints(display, win, &mywmhints);

    XSetCommand(display, win, argv, argc);
    XMapWindow(display, win);

}


/******************************************************************************
 *
 * dcl_getdir_config
 *
 ******************************************************************************/

char* dcl_getdir_config (char *cdirectory)
{
static char cfgdir[PATH_MAX];
struct stat cfg;

    dcl_strcpy (cfgdir, getenv ("HOME"), PATH_MAX);
    dcl_strcat (cfgdir, "/", PATH_MAX);
    dcl_strcat (cfgdir, cdirectory, PATH_MAX);

    if(stat(cfgdir, &cfg) < 0)
        mkdir(cfgdir, S_IRUSR | S_IWUSR | S_IXUSR);

    return cfgdir;
}


/******************************************************************************
 *
 * dcl_getfilename_config
 *
 ******************************************************************************/

char* dcl_getfilename_config (char *config_dir, char *config_filename)
{
static char filename[PATH_MAX];

    dcl_strcpy (filename, dcl_getdir_config(config_dir), PATH_MAX);
    dcl_strcat (filename, "/", PATH_MAX);
    dcl_strcat (filename, config_filename, PATH_MAX);

    return filename;
}


/******************************************************************************
 *
 * dcl_prefs_openfile
 *
 ******************************************************************************/

void* dcl_prefs_openfile (char *filename, int openmode)
{
    prefs_filehandle = NULL;

    if (openmode == P_READ)
        prefs_filehandle = fopen (filename, "rb");
    else if (openmode == P_WRITE)
        prefs_filehandle = fopen (filename, "wb");

    return prefs_filehandle;
}


/******************************************************************************
 *
 * dcl_prefs_closefile
 *
 ******************************************************************************/

void dcl_prefs_closefile (void)
{
    fclose (prefs_filehandle);
}


/******************************************************************************
 *
 * dcl_prefs_put_int
 *
 ******************************************************************************/

void dcl_prefs_put_int (char *tagname, int value)
{
    fprintf (prefs_filehandle, "%s=%d\n", tagname, value);
}


/******************************************************************************
 *
 * dcl_prefs_put_float
 *
 ******************************************************************************/

void dcl_prefs_put_float (char *tagname, float value)
{
    fprintf (prefs_filehandle, "%s=%f\n", tagname, value);
}


/******************************************************************************
 *
 * dcl_prefs_put_string
 *
 ******************************************************************************/

void dcl_prefs_put_string (char *tagname, char *value)
{
    fprintf (prefs_filehandle, "%s=%s\n", tagname, value);
}


/******************************************************************************
 *
 * dcl_prefs_put_lf
 *
 ******************************************************************************/

void dcl_prefs_put_lf (void)
{
    fprintf (prefs_filehandle, "\n");
}


/******************************************************************************
 *
 * dcl_prefs_put_comment
 *
 ******************************************************************************/

void dcl_prefs_put_comment (char *comment)
{
char text[MAX_LINE_LEN];

        dcl_strcpy (text, "# ", MAX_LINE_LEN);
        dcl_strcat (text, comment, MAX_LINE_LEN);
        fprintf (prefs_filehandle, text);
}


/******************************************************************************
 *
 * dcl_prefs_get_line_with_tag
 *
 ******************************************************************************/

char* dcl_prefs_get_line_with_tag (char *tagname)
{
static char prfline[MAX_LINE_LEN];
int i;
char c;

    fseek (prefs_filehandle, 0, SEEK_SET);

    while (!feof (prefs_filehandle)) {
        i = 0;

        while (((c = fgetc (prefs_filehandle)) != '\n') && c!= EOF && i < MAX_LINE_LEN)
            prfline[i++] = c;

        prfline[i] = '\0';

        if (prfline[0] != '#')
            if (!strncmp (tagname, prfline, strlen (tagname))) break;
    }

    return prfline;
}


/******************************************************************************
 *
 * dcl_prefs_get_value_field
 *
 ******************************************************************************/

char* dcl_prefs_get_value_field (char *tagname)
{
static char valuestr[MAX_VALUE_LEN];
char *valpos, c;
int i;

    i = 0;

    if ((valpos = strchr (dcl_prefs_get_line_with_tag (tagname), '='))) {
        while((c = valpos[i+1]) != '\0' && i < MAX_VALUE_LEN) valuestr[i++] = c;
    }

    valuestr[i] = '\0';
    return valuestr;
}


/******************************************************************************
 *
 * dcl_prefs_get_int
 *
 ******************************************************************************/

int dcl_prefs_get_int (char *tagname)
{
    return (atoi (dcl_prefs_get_value_field (tagname)));
}


/******************************************************************************
 *
 * dcl_prefs_get_float
 *
 ******************************************************************************/

float dcl_prefs_get_float (char *tagname)
{
    return (atof (dcl_prefs_get_value_field (tagname)));
}


/******************************************************************************
 *
 * dcl_prefs_get_string
 *
 ******************************************************************************/

char* dcl_prefs_get_string (char *tagname)
{
    return (dcl_prefs_get_value_field (tagname));
}


/******************************************************************************
 *
 * dcl_strcpy
 *
 ******************************************************************************/

char* dcl_strcpy (char *dest, const char *src, int maxlength)
{
int len;

    if (!dest) {
        fprintf (stderr, "ERROR: NULL dest in safe_strcpy\n");
        return NULL;
    }

    if (!src) {
        *dest = 0;
        return dest;
    }

    len = strlen(src);

    if (len > maxlength) {
        fprintf (stderr, "ERROR: string overflow by %d in safe_strcpy [%.50s]\n",
                (int)(len-maxlength), src);
        len = maxlength;
    }

    memcpy(dest, src, len);
    dest[len] = 0;
    return dest;
}


/******************************************************************************
 *
 * dcl_strcat
 *
 ******************************************************************************/

char* dcl_strcat (char *dest, const char *src, int maxlength)
{
int src_len, dest_len;

    if (!dest) {
        fprintf (stderr, "ERROR: NULL dest in safe_strcat\n");
        return NULL;
    }

    if (!src) {
        return dest;
    }

    src_len = strlen(src);
    dest_len = strlen(dest);

    if (src_len + dest_len > maxlength) {
        fprintf (stderr, "ERROR: string overflow by %d in safe_strcat [%.50s]\n",
                (int)(src_len + dest_len - maxlength), src);
        src_len = maxlength - dest_len;
    }

    memcpy(&dest[dest_len], src, src_len);
    dest[dest_len + src_len] = 0;
    return dest;
}


/******************************************************************************
 *
 * dcl_draw_char
 *
 ******************************************************************************/

int dcl_draw_char(int x, int y, char z, int font_type)
{
char *ctable = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.[]-_:'@" };
int ctable_len = 44;
int k, font_line, font_width, font_height;

    switch(font_type) {

        case FONT_SMALL:
                font_width = 4;
                font_height = 5;
                font_line = 12;
                break;

        case FONT_NORMAL:
                font_width = 5;
                font_height = 5;
                font_line = 7;
                break;

        case FONT_LARGE:
                font_width = 5;
                font_height = 7;
                font_line = 0;
                break;

        default:
                fprintf(stderr, "ERROR: Unknown font type...\n");
                return false;
    }

    for(k=0; k < ctable_len; k++)
        if(toupper(z)==ctable[k]) {

            dcl_copy_font_xpm_area(k*font_width, font_line, font_width, font_height, x, y);
            break;

        }

    return true;
}

/******************************************************************************
 *
 * dcl_draw_string
 *
 ******************************************************************************/

int dcl_draw_string(int x, int y, char *string, int font_type, int length)
{
int j, len, font_width;
char a;

    if(length == -1)
        len = strlen(string);
    else
        len = length;

    if(len <= 0 || len > MAX_STRING_LEN) {

        fprintf(stderr, "ERROR: Wrong string length...\n");
        return false;

    }

    switch(font_type) {

        case FONT_SMALL:
                font_width = 4;
                break;

        case FONT_NORMAL:
                font_width = 5;
                break;

        case FONT_LARGE:
                font_width = 5;
                break;

        default:
                fprintf(stderr, "ERROR: Unknown font type...\n");
                return false;
    }

	for(j=0; j<len; j++) {

		if((a=string[j]) == '\0') break;
		dcl_draw_char(x+j*(font_width+1), y, a, font_type);

	}

    return true;
}


/******************************************************************************
 *
 * dcl_draw_led
 *
 ******************************************************************************/

int dcl_draw_led(int x, int y, int color)
{

    if(color > NUM_COLORS) {
        fprintf(stderr,"ERROR: Color doesn't exist...");
        return false;
    }

    dcl_copy_led_xpm_area(5*color, 0, 5, 5, x, y);

    return true;
}


/******************************************************************************
 *
 * dcl_execute_command
 *
 ******************************************************************************/

void dcl_execute_command(char *command, int flag)
{
char *cmdline[4];
pid_t pid, pid_status;

    pid = fork();

    if (pid == 0) {

        cmdline[0] = "sh";
        cmdline[1] = "-c";
        cmdline[2] = command;
        cmdline[3] = 0;
        execvp("/bin/sh", cmdline);
        _exit(121);

    } else
        if(flag)
            waitpid(pid, &pid_status, 0);

}

/******************************************************************************
 *
 * dcl_check_file
 *
 ******************************************************************************/

int dcl_check_file(char *filename)
{
struct stat ftype;
int state = F_NOT_AVAILABLE;

    if(stat(filename, &ftype)==-1)
        state = F_NOT_AVAILABLE;

    else if(S_ISREG(ftype.st_mode))
        state = F_REGULAR;

    else if(S_ISDIR(ftype.st_mode))
        state = F_DIRECTORY;

    else if(S_ISCHR(ftype.st_mode))
        state = F_CHAR_DEVICE;

    else if(S_ISBLK(ftype.st_mode))
        state = F_BLOCK_DEVICE;

    else if(S_ISLNK(ftype.st_mode))
        state = F_LINK;

    else if(S_ISFIFO(ftype.st_mode))
        state = F_FIFO;

    else if(S_ISSOCK(ftype.st_mode))
        state = F_SOCK;

    return state;
}


/******************************************************************************
 *
 * dcl_draw_point
 *
 ******************************************************************************/

void dcl_draw_point(int x, int y, Pixel color)
{
    XSetForeground(display, NormalGC, color);
    XDrawPoint(display, wmgen.pixmap, NormalGC, x, y);
}

/******************************************************************************
 *
 * dcl_draw_line
 *
 ******************************************************************************/

void dcl_draw_line(int x0, int y0, int x1, int y1, Pixel color)
{
    XSetForeground(display, NormalGC, color);
    XDrawLine(display, wmgen.pixmap, NormalGC, x0, y0, x1, y1);
}

/*-------------------------------------------------------------------------------------------
 * fonts and leds definitions
 * gfx by sill
 *-------------------------------------------------------------------------------------------*/

char * fonts_xpm[] = {
"220 17 8 1",
"   c None",
".  c #145B53",
"+  c #20B2AE",
"@  c #004941",
"#  c #007D71",
"$  c #188A86",
"%  c #202020",
"&  c #22332F",
".+++.++++.@+++#++++@#+++##+++#@+++$+@%@+%#+#%#+++#+%%%++%%%%+%%%++@%%+@+++@++++@@+++@++++@@++++++++++%%%++%%%++%%%++%%%++%%%+++++#%%%#%#+++##+++##%%%##+++##+++##+++##+++##+++##+++#%%%%%+++$%%$+++%%%%%%%%%%%%%%%%%%%%@+++@",
"+%%%++%%%++@%%%+%%@++%%%%+%%%%+%%%@+@%@+%%+%%%%%%++%%##+%%%%+@%@+++%%++@%@++%%@++@%@++%%@++%%%@@%+%@+%%%++%%%++%%%++@%@++%%%+@%%@+%%%+%%%%%+%%%%++%%%++%%%%+%%%%%%%%++%%%++%%%++%%%+%%%%%+%%%%%%%%+%%%%%%%%%%%++%%%+$%%+@%@+",
"+%%%++%%%++%%%%+%%%++%%%%+%%%%+%%%%+@%@+%%+%%%%%%++@@+%+%%%%++@++++.%++%%%++%%%++%%%++%%%++%%%%%%+%%+%%%++%%%++%%%+%+@+%+@%@+%%@+.%%%+%%%%%+%%%%++%%%++%%%%+%%%%%%%%++%%%++%%%++%%%+%%%%%+%%%%%%%%+%%%%%%%%%%%$$%%%+$%%+%#%+",
"+%%%+++++%+%%%%+%%%++++#%+++#%+%++$+++++%%+%%%%%%++++%%+%%%%+@+@++%+%++%%%++%%@++%#%++%%@+@+++@%%+%%+%%%++@%@++%#%+%@+@%@+.+@%.+.%%%%#%#+++#%+++##+++##+++##+++#%%%%#%+++%#+++##%%%#%%%%%+%%%%%%%%+$+++$%%%%%%%%%%%$.%%+%+@+",
"++++++%%%++%%%%+%%%++%%%%+%%%%+%%%++@%@+%%+%%%%%%++@@+%+%%%%+%@%++%.+++%%%+++++@+%+@+++++%%%%%+%%+%%+%%%+##%##+@+@+%+@+%%@+@%.+@%%%%%+%+%%%%%%%%+%%%%+%%%%++%%%+%%%%++%%%+%%%%++%%%+%%%%%+%%%%%%%%+%%%%%%%%%%%%%%%%%%%%+%#+@",
"+%%%++%%%++@%%%+%%@++%%%%+%%%%+%%%++@%@+%%+%%+%%@++%%##+@%%%+%%%++%%+++@%@++%%%%+@@+.+%%@+@%%%+%%+%%+@%@+%+@+%++@+++@%@+%%+%%+@%%@%%%+%+%%%%%%%%+%%%%+%%%%++%%%+%%%%++%%%+%%%%++%%%+%$$%%+%%%%%%%%+%%%%%%%%%%%$$%%%%%%%+@%%%",
"+%%%+++++.@+++#++++@#+++#+%%%%@+++.+@%@+%#+#%@+++@+%%%+.+++#+%%%++%%@+@+++@+%%%%@++.++%%%+++++@%%+%%@+++@%@+@%+@%@++%%%+%%+%%#++++%%%#%#+++##+++#%%%%##+++##+++#%%%%##+++##+++##+++#%++%%+++$%%$+++%%%%%$+++$%++%%%%%%%@+++@",
"&+++&++++&&+++$++++&$+++$$+++$.+++.+%%%+%&+&%$+++$+%%%++%%%%++&+++%%%+&+++&++++&&+++&++++&&+++$$+++$+%%%++%%%++%%%++%%%++%%%+$++++%%%$%$++$%$++$%$%%$%$++$%$++$%$++$%$++$%$++$%$++$%%%%%%%++&%%&++%%%%%%%%%%%%$$%%%+$%%@+++@",
"+%%%++%%%++%%%%+%%%++%%%%+%%%%+%%%%+%%%+%%+%%%%%%++%%+%+%%%%+&+&+++%%++%%%++%%%++%%%++%%%++%%%%%%+%%+%%%++%%%++%+%+%+%+%+%%%+%%%+%%%%+%%%%+%%%%+%+%%+%+%%%%+%%%%%%%+%+%%+%+%%+%+%%+%%%%%%%+%%%%%%+%%%%%%%%%%%%++%%%+$%%+$$@+",
"+%%%+++++&+%%%%+%%%++++$%+++$%+%%+++++++%%+%%%%%%++++%%+%%%%+%+%++%+%++%%%++%%%++%+%++%%%+&+++&%%+%%+%%%+&+%+&+%+%+%%+%%%+%+%%%+%%%%%$%$++$%%++$%$++$%$++$%$++$%%%%$%$++$%$++$%$%%$%%%%%%%+%%%%%%+%$+++$%%%%%%%%%%%$.%%+%++$",
"++++++%%%++%%%%+%%%++%%%%+%%%%+%%%++%%%+%%+%%+%%%++%%+%+%%%%+%&%++%%+++%%%+++++&+%%+$++++&%%%%+%%+%%+%%%+%+%+%+&+&+%+%+%%%+%%%+%%%%%%+%+%%%%%%%+%%%%+%%%%+%+%%+%%%%+%+%%+%%%%+%+%%+%%$$%%%+%%%%%%+%%%%%%%%%%%%$$%%%%%%%+@@@%",
"+%%%+++++&&+++$++++&$+++$+%%%%.+++.+%%%+%&+&%&+++&+%%%+&+++++%%%++%%%+&+++&+%%%%&++$++%%%+$+++&%%+%%&+++&%&+&%&+&+&+%%%+%%+%%++++$%%%$%$++$%$++$%%%%$%$++$%$++$%%%%$%$++$%$++$%$++$%%++%%%++&%%&++%%%%%%$+++$%++%%%%%%%@+++@",
".++.+++.&++$$++.$++$$++$.++$$%%$%$.%$++$$%%$$%%%$%%$$%%$.++.+++..++.+++..++$++++$%%$$%%$$%%$$%%$$%%$$++$%%.$$++.$++.$%%$$++$.++$$++..++..++..++.%%%%++&%%&++%%%%%%%%%$$%%+$%@++@%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
"+%%++%%++%%%+%%++%%%+%%%+%%%+%%+%+.%%%%++%+.+%%%+$$+++%++%%++%%++%%++%%++%%%%$$%+%%++%%++%%++%%++%%+%%+%%.++%%%+%%%++%%++%%%+%%%%%%++%%++%%++%%+%%%%+%%%%%%+%%%%%%%%%++%%+$%+$@+%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
"+%%++++&+%%%+%%+++$%++$%+%+$++++%+.%%%%+++.%+%%%+..++&+++%%++%%++%$++%%+.++.%$$%+%%+$%%$+..+&++&%++%%+%%%%%+.++.%++$.+++$++.+++.%%%+.++..++++%%+%%%%+%%%%%%+$++$%%%%%%%%%$.%+%+$%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
"+++++%%++%%%+%%++%%%+%%%+%%++%%+%+.%+%%++%+.+%%%+%%++%&++%%++++.+%+$+++&%%%+%$$%+%%+$..$+++++%%+%&+%+%%%%%%++%%%%%%+%%%+%%%++%%+%%%++%%+%%%++%%+%$$%+%%%%%%+%%%%%%%%%$$%%%%%+@@%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
"+%%++++.&++$$++.$++$+%%%.++.$%%$%$.%.++.$%%$$++$$%%$$%%$.++.+%%%.+$++%%+$++.%$$%.++.&++&$&&$$%%$%&$%$++$%%%$$++$$++.%%%$$++..++.%%%$.++..++..++.%++%++&%%&++%%%%$++$%++%%%%%@++@%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"};

char * leds_xpm[] = {
"50 5 72 1",
"   c #262626",
".  c #3F4D4A",
"+  c #4A5754",
"@  c #3E3E1B",
"#  c #B2B26F",
"$  c #DCDC30",
"%  c #97970A",
"&  c #1F431F",
"*  c #5CA65C",
"=  c #2DE02D",
"-  c #0F8D0F",
";  c #282828",
">  c #448986",
",  c #4EA9A5",
"'  c #3B7F7D",
")  c #032E50",
"!  c #2776E2",
"~  c #6CA5EB",
"{  c #396CC4",
"]  c #662F00",
"^  c #E58325",
"/  c #F2AF74",
"(  c #C66100",
"_  c #4A1F1F",
":  c #BB4545",
"<  c #FF2424",
"[  c #B61111",
"}  c #630000",
"|  c #C32284",
"1  c #C2519E",
"2  c #B4197C",
"3  c #650000",
"4  c #9B4B55",
"5  c #AB6A72",
"6  c #8C434E",
"7  c #3D3D3D",
"8  c #898989",
"9  c #CDCDCD",
"0  c #F3F3DD",
"a  c #F3F3BA",
"b  c #D4D400",
"c  c #D8F9D8",
"d  c #8BEE8B",
"e  c #17D717",
"f  c #D6FAF9",
"g  c #6ADAD7",
"h  c #46A19E",
"i  c #BFE0EC",
"j  c #A5CEEB",
"k  c #4C93FB",
"l  c #F7E4D4",
"m  c #FFDCBA",
"n  c #FFA147",
"o  c #F7DADA",
"p  c #FF7373",
"q  c #F81515",
"r  c #FBFAFD",
"s  c #D6ACD9",
"t  c #C13E9E",
"u  c #FBFEFD",
"v  c #CBBCBF",
"w  c #9F626C",
"x  c #EBEBEB",
"y  c #FFFFFF",
"z  c #EBEB8B",
"A  c #72EA72",
"B  c #55B7B3",
"C  c #93C2EB",
"D  c #FFD1A5",
"E  c #FF5454",
"F  c #C170AD",
"G  c #B57D85",
" .+. @#$%@&*=-&;>,';)!~{)]^/(]_:<[_}|12}3456378987",
".+++.#0ab%*cde->fgh'!ijk{^lmn(:opq[|rst24uvw68xyx8",
"+++++$az$b=dA=e,gB,h~jC~k/mD/n<pE<q1sF1t5vG5w9yyy9",
".+++.%b$b%-e=e-'h,h'{k~k{(n/n([q<q[2t1t26w5w68xyx8",
" .+. @%b%@&-e-&;'h';){k{)](n(]_[q[_}2t2}36w6378987"};


/*
 * Created:       Sat 26 Mar 2005 05:45:53 PM CET
 * Last Modified: Sat 26 Mar 2005 10:53:38 PM CET
 */

