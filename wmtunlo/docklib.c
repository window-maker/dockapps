/*--------------------------------*/
/* a simple dockapp library       */
/* made from scratch              */
/*--------------------------------*/

/* 
	functions were written by following People:

		--- linked list
		Kresten Krab Thorup
		Alfredo K. Kojima

		--- built-in Dock module for WindowMaker
		Alfredo K. Kojima

		---	wmgeneral (taken from wmppp)
		Martijn Pieterse (pieterse@xs4all.nl)

		--- prefs routines
		Tomasz M±ka
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "docklib.h"

/* Return a cons cell produced from (head . tail) */

INLINE LinkedList* 
list_cons(void* head, LinkedList* tail)
{
  LinkedList* cell;

  cell = (LinkedList*)malloc(sizeof(LinkedList));
  cell->head = head;
  cell->tail = tail;
  return cell;
}

/* Return the length of a list, list_length(NULL) returns zero */

INLINE int
list_length(LinkedList* list)
{
  int i = 0;
  while(list)
    {
      i += 1;
      list = list->tail;
    }
  return i;
}

/* Return the Nth element of LIST, where N count from zero.  If N 
   larger than the list length, NULL is returned  */

INLINE void*
list_nth(int index, LinkedList* list)
{
  while(index-- != 0)
    {
      if(list->tail)
	list = list->tail;
      else
	return 0;
    }
  return list->head;
}

/* Remove the element at the head by replacing it by its successor */

INLINE void
list_remove_head(LinkedList** list)
{
  if (!*list) return;  
  if ((*list)->tail)
    {
      LinkedList* tail = (*list)->tail; /* fetch next */
      *(*list) = *tail;		/* copy next to list head */
      free(tail);			/* free next */
    }
  else				/* only one element in list */
    {
      free(*list);
      (*list) = 0;
    }
}

INLINE LinkedList *
list_remove_elem(LinkedList* list, void* elem)
{
    LinkedList *tmp;
    
    if (list) {
	if (list->head == elem) {
	    tmp = list->tail;
	    free(list);
	    return tmp;
	}
	list->tail = list_remove_elem(list->tail, elem);
	return list;
    }
    return NULL;
}


/* Return element that has ELEM as car */

INLINE LinkedList*
list_find(LinkedList* list, void* elem)
{
  while(list)
    {
    if (list->head == elem)
      return list;
    list = list->tail;
    }
  return NULL;
}

/* Free list (backwards recursive) */

INLINE void
list_free(LinkedList* list)
{
  if(list)
    {
      list_free(list->tail);
      free(list);
    }
}

/* Map FUNCTION over all elements in LIST */

INLINE void
list_mapcar(LinkedList* list, void(*function)(void*))
{
  while(list)
    {
      (*function)(list->head);
      list = list->tail;
    }
}


/*
 *----------------------------------------------------------------------
 * parse_command--
 * 	Divides a command line into a argv/argc pair.
 *---------------------------------------------------------------------- 
 */
#define PRC_ALPHA	0
#define PRC_BLANK	1
#define PRC_ESCAPE	2
#define PRC_DQUOTE	3
#define PRC_EOS		4
#define PRC_SQUOTE	5

typedef struct {
    short nstate;
    short output;
} DFA;


static DFA mtable[9][6] = {
    {{3,1},{0,0},{4,0},{1,0},{8,0},{6,0}},
    {{1,1},{1,1},{2,0},{3,0},{5,0},{1,1}},
    {{1,1},{1,1},{1,1},{1,1},{5,0},{1,1}},
    {{3,1},{5,0},{4,0},{1,0},{5,0},{6,0}},
    {{3,1},{3,1},{3,1},{3,1},{5,0},{3,1}},
    {{-1,-1},{0,0},{0,0},{0,0},{0,0},{0,0}}, /* final state */
    {{6,1},{6,1},{7,0},{6,1},{5,0},{3,0}},
    {{6,1},{6,1},{6,1},{6,1},{5,0},{6,1}},
    {{-1,-1},{0,0},{0,0},{0,0},{0,0},{0,0}}, /* final state */
};

char*
next_token(char *word, char **next)
{
    char *ptr;
    char *ret, *t;
    int state, ctype;

    t = ret = malloc(strlen(word)+1);
    ptr = word;
    
    state = 0;
    *t = 0;
    while (1) {
	if (*ptr==0) 
	    ctype = PRC_EOS;
	else if (*ptr=='\\')
	    ctype = PRC_ESCAPE;
	else if (*ptr=='"')
	    ctype = PRC_DQUOTE;
	else if (*ptr=='\'')
	    ctype = PRC_SQUOTE;
	else if (*ptr==' ' || *ptr=='\t')
	    ctype = PRC_BLANK;
	else
	    ctype = PRC_ALPHA;

	if (mtable[state][ctype].output) {
	    *t = *ptr; t++;
	    *t = 0;
	}
	state = mtable[state][ctype].nstate;
	ptr++;
	if (mtable[state][0].output<0) {
	    break;
	}
    }

    if (*ret==0)
	t = NULL;
    else
	t = strdup(ret);

    free(ret);
    
    if (ctype==PRC_EOS)
	*next = NULL;
    else
	*next = ptr;
    
    return t;
}


extern void
parse_command(char *command, char ***argv, int *argc)
{
    LinkedList *list = NULL;
    char *token, *line;
    int count, i;

    line = command;
    do {
	token = next_token(line, &line);
	if (token) {	    
	    list = list_cons(token, list);
	}
    } while (token!=NULL && line!=NULL);

    count = list_length(list);
    *argv = malloc(sizeof(char*)*count);
    i = count;
    while (list!=NULL) {
	(*argv)[--i] = list->head;
	list_remove_head(&list);
    }
    *argc = count;
}

extern pid_t
execCommand(char *command)
{
    pid_t pid;
    char **argv;
    int argc;

    parse_command(command, &argv, &argc);
    
    if (argv==NULL) {
        return 0;
    }
    
    if ((pid=fork())==0) {
        char **args;
        int i;
        
        args = malloc(sizeof(char*)*(argc+1));
        if (!args)
          exit(10);
        for (i=0; i<argc; i++) {
            args[i] = argv[i];
        }
        args[argc] = NULL;
        execvp(argv[0], args);
        exit(10);
    }
    return pid;
}



  /*****************/
 /* X11 Variables */
/*****************/

Window		Root;
int			screen;
int			x_fd;
int			d_depth;
XSizeHints	mysizehints;
XWMHints	mywmhints;
Pixel		back_pix, fore_pix;
char		*Geometry = "";
Window		iconwin, win;
GC			NormalGC;
XpmIcon		wmgen;
Pixmap		pixmask;

  /*****************/
 /* Mouse Regions */
/*****************/

typedef struct {
	int		enable;
	int		top;
	int		bottom;
	int		left;
	int		right;
} MOUSE_REGION;

#define MAX_MOUSE_REGION (8)
MOUSE_REGION	mouse_region[MAX_MOUSE_REGION];

  /***********************/
 /* Function Prototypes */
/***********************/

static void GetXPM(XpmIcon *, char **);
static Pixel GetColor(char *);
void RedrawWindow(void);
void AddMouseRegion(int, int, int, int, int);
int CheckMouseRegion(int, int);

/*******************************************************************************\
|* read_rc_file																   *|
\*******************************************************************************/

void parse_rcfile(const char *filename, rckeys *keys) {

	char	*p;
	char	temp[128];
	char	*tokens = " :\t\n";
	FILE	*fp;
	int		i,key;

	fp = fopen(filename, "r");
	if (fp) {
		while (fgets(temp, 128, fp)) {
			key = 0;
			while (key >= 0 && keys[key].label) {
				if ((p = strstr(temp, keys[key].label))) {
					p += strlen(keys[key].label);
					p += strspn(p, tokens);
					if ((i = strcspn(p, "#\n"))) p[i] = 0;
					free(*keys[key].var);
					*keys[key].var = strdup(p);
					key = -1;
				} else key++;
			}
		}
		fclose(fp);
	}
}


/*******************************************************************************\
|* GetXPM																	   *|
\*******************************************************************************/

static void GetXPM(XpmIcon *wmgen, char *pixmap_bytes[]) {

	XWindowAttributes	attributes;
	int					err;

	/* For the colormap */
	XGetWindowAttributes(display, Root, &attributes);

	wmgen->attributes.valuemask |= (XpmReturnPixels | XpmReturnExtensions);

	err = XpmCreatePixmapFromData(display, Root, pixmap_bytes, &(wmgen->pixmap),
					&(wmgen->mask), &(wmgen->attributes));
	
	if (err != XpmSuccess) {
		fprintf(stderr, "Not enough free colorcells.\n");
		exit(1);
	}
}

/*******************************************************************************\
|* GetColor																	   *|
\*******************************************************************************/

static Pixel GetColor(char *name) {

	XColor				color;
	XWindowAttributes	attributes;

	XGetWindowAttributes(display, Root, &attributes);

	color.pixel = 0;
	if (!XParseColor(display, attributes.colormap, name, &color)) {
		fprintf(stderr, "wm.app: can't parse %s.\n", name);
	} else if (!XAllocColor(display, attributes.colormap, &color)) {
		fprintf(stderr, "wm.app: can't allocate %s.\n", name);
	}
	return color.pixel;
}

/*******************************************************************************\
|* flush_expose																   *|
\*******************************************************************************/

static int flush_expose(Window w) {

	XEvent 		dummy;
	int			i=0;

	while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
		i++;

	return i;
}

/*******************************************************************************\
|* RedrawWindow																   *|
\*******************************************************************************/

void RedrawWindow(void) {
	
	flush_expose(iconwin);
	XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, 
				0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
	flush_expose(win);
	XCopyArea(display, wmgen.pixmap, win, NormalGC,
				0,0, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}

/*******************************************************************************\
|* RedrawWindowXY															   *|
\*******************************************************************************/

void RedrawWindowXY(int x, int y) {
	
	flush_expose(iconwin);
	XCopyArea(display, wmgen.pixmap, iconwin, NormalGC, 
				x,y, wmgen.attributes.width, wmgen.attributes.height, 0,0);
	flush_expose(win);
	XCopyArea(display, wmgen.pixmap, win, NormalGC,
				x,y, wmgen.attributes.width, wmgen.attributes.height, 0,0);
}

/*******************************************************************************\
|* AddMouseRegion															   *|
\*******************************************************************************/

void AddMouseRegion(int index, int left, int top, int right, int bottom) {

	if (index < MAX_MOUSE_REGION) {
		mouse_region[index].enable = 1;
		mouse_region[index].top = top;
		mouse_region[index].left = left;
		mouse_region[index].bottom = bottom;
		mouse_region[index].right = right;
	}
}

/*******************************************************************************\
|* CheckMouseRegion															   *|
\*******************************************************************************/

int CheckMouseRegion(int x, int y) {

	int		i;
	int		found;

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

/*******************************************************************************\
|* copyXPMArea																   *|
\*******************************************************************************/

void copyXPMArea(int x, int y, int sx, int sy, int dx, int dy) {

	XCopyArea(display, wmgen.pixmap, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);

}

/*******************************************************************************\
|* copyXBMArea																   *|
\*******************************************************************************/

void copyXBMArea(int x, int y, int sx, int sy, int dx, int dy) {

	XCopyArea(display, wmgen.mask, wmgen.pixmap, NormalGC, x, y, sx, sy, dx, dy);
}


/*******************************************************************************\
|* setMaskXY																   *|
\*******************************************************************************/

void setMaskXY(int x, int y) {

	 XShapeCombineMask(display, win, ShapeBounding, x, y, pixmask, ShapeSet);
	 XShapeCombineMask(display, iconwin, ShapeBounding, x, y, pixmask, ShapeSet);
}

/*******************************************************************************\
|* openXwindow																   *|
\*******************************************************************************/
void openXwindow(int argc, char *argv[], char *pixmap_bytes[], char *pixmask_bits, int pixmask_width, int pixmask_height) {

	unsigned int	borderwidth = 1;
	XClassHint		classHint;
	char			*display_name = NULL;
	char			*wname = argv[0];
	XTextProperty	name;

	XGCValues		gcv;
	unsigned long	gcm;


	int				dummy=0;
	int				i;

	for (i=1; argv[i]; i++) {
		if (!strcmp(argv[i], "-display")) 
			display_name = argv[i+1];
	}

	if (!(display = XOpenDisplay(display_name))) {
		fprintf(stderr, "%s: can't open display %s\n", 
						wname, XDisplayName(display_name));
		exit(1);
	}
	screen  = DefaultScreen(display);
	Root    = RootWindow(display, screen);
	d_depth = DefaultDepth(display, screen);
	x_fd    = XConnectionNumber(display);

	/* Convert XPM to XImage */
	GetXPM(&wmgen, pixmap_bytes);

	/* Create a window to hold the stuff */
	mysizehints.flags = USSize | USPosition;
	mysizehints.x = 0;
	mysizehints.y = 0;

	back_pix = GetColor("white");
	fore_pix = GetColor("black");

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

	XSelectInput(display, win, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
	XSelectInput(display, iconwin, ButtonPressMask | ExposureMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

	if (XStringListToTextProperty(&wname, 1, &name) == 0) {
		fprintf(stderr, "%s: can't allocate window name\n", wname);
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


FILE *prefs_filehandle;
char* p_strcpy (char *dest, const char *src, int maxlength);
char* p_strcat (char *dest, const char *src, int maxlength);

/*---------------------------------------------------------------------------*/

char* p_getdir_config (char *cdirectory)
{
static char cfgdir[MAX_PATH];
struct stat cfg;

	p_strcpy (cfgdir, getenv ("HOME"), MAX_PATH);
	p_strcat (cfgdir, slash, MAX_PATH);
	p_strcat (cfgdir, cdirectory, MAX_PATH);

	if(stat(cfgdir, &cfg) < 0)
		mkdir(cfgdir, S_IRUSR | S_IWUSR | S_IXUSR);

	return cfgdir;
}

/*---------------------------------------------------------------------------*/

char* p_getfilename_config (char *config_dir, char *config_filename)
{
static char filename[MAX_PATH];

	p_strcpy (filename, p_getdir_config(config_dir), MAX_PATH);
	p_strcat (filename, slash, MAX_PATH);
	p_strcat (filename, config_filename, MAX_PATH);

	return filename;
}

/*---------------------------------------------------------------------------*/

void* p_prefs_openfile (char *filename, int openmode)
{
	prefs_filehandle = NULL;

	if (openmode == P_READ)
		prefs_filehandle = fopen (filename, "rb");
	else if (openmode == P_WRITE)
		prefs_filehandle = fopen (filename, "wb");
    
	return prefs_filehandle;
}


void p_prefs_closefile (void)
{
	fclose (prefs_filehandle);
}

/*---------------------------------------------------------------------------*/

void p_prefs_put_int (char *tagname, int value)
{
	fprintf (prefs_filehandle, "%s=%d\n", tagname, value);
}


void p_prefs_put_float (char *tagname, float value)
{
	fprintf (prefs_filehandle, "%s=%f\n", tagname, value);
}


void p_prefs_put_string (char *tagname, char *value)
{
	fprintf (prefs_filehandle, "%s=%s\n", tagname, value);
}


void p_prefs_put_lf (void)
{
	fprintf (prefs_filehandle, "\n");
}


void p_prefs_put_comment (char *comment)
{
char text[MAX_LINE_LEN];

		p_strcpy (text, "# ", MAX_LINE_LEN);
		p_strcat (text, comment, MAX_LINE_LEN);
		fprintf (prefs_filehandle, text);
}

/*---------------------------------------------------------------------------*/

char* p_prefs_get_line_with_tag (char *tagname)
{
static char prfline[MAX_LINE_LEN];
int i;
char c;

	fseek (prefs_filehandle, 0, SEEK_SET);	
    
	while (!feof (prefs_filehandle)) {
		i = 0;

		while (((c = fgetc (prefs_filehandle)) != crlf_char) && c!= EOF && i < MAX_LINE_LEN)
			prfline[i++] = c;
            
		prfline[i] = null_char;
        
		if (prfline[0] != '#')
			if (!strncmp (tagname, prfline, strlen (tagname))) break;
	}
        
	return prfline;
}


char* p_prefs_get_value_field (char *tagname)
{
static char valuestr[MAX_VALUE_LEN];
char *valpos, c;
int i;

	i = 0;

	if ((valpos = strchr (p_prefs_get_line_with_tag (tagname), '='))) {
		while((c = valpos[i+1]) != null_char && i < MAX_VALUE_LEN) valuestr[i++] = c;
	}
    
	valuestr[i] = null_char;
	return valuestr;
}


int p_prefs_get_int (char *tagname)
{
	return (atoi (p_prefs_get_value_field (tagname)));
}


float p_prefs_get_float (char *tagname)
{
	return (atof (p_prefs_get_value_field (tagname)));
}


char* p_prefs_get_string (char *tagname)
{
	return (p_prefs_get_value_field (tagname));
}

/*---------------------------------------------------------------------------*/
/* following functions based on samba sources.                               */
/* safe_strcpy and safe_strcat routines written by Andrew Tridgell           */
/*---------------------------------------------------------------------------*/

char* p_strcpy (char *dest, const char *src, int maxlength)
{
int len;

	if (!dest) {
		printf ("ERROR: NULL dest in safe_strcpy\n");
		return NULL;
	}

	if (!src) {
		*dest = 0;
		return dest;
	}

	len = strlen(src);

	if (len > maxlength) {
		printf ("ERROR: string overflow by %d in safe_strcpy [%.50s]\n",
				(int)(len-maxlength), src);
		len = maxlength;
	}
      
	memcpy(dest, src, len);
	dest[len] = 0;
	return dest;
}

/*---------------------------------------------------------------------------*/

char* p_strcat (char *dest, const char *src, int maxlength)
{
int src_len, dest_len;

	if (!dest) {
		printf ("ERROR: NULL dest in safe_strcat\n");
		return NULL;
	}

	if (!src) {
		return dest;
	}

	src_len = strlen(src);
	dest_len = strlen(dest);

	if (src_len + dest_len > maxlength) {
		printf ("ERROR: string overflow by %d in safe_strcat [%.50s]\n",
				(int)(src_len + dest_len - maxlength), src);
		src_len = maxlength - dest_len;
	}
      
	memcpy(&dest[dest_len], src, src_len);
	dest[dest_len + src_len] = 0;
	return dest;
}

/*---------------------------------------------------------------------------*/


