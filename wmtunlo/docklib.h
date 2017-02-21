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


#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
# define INLINE inline
#else
# define INLINE
#endif

typedef struct LinkedList {
  void *head;
  struct LinkedList *tail;
} LinkedList;

INLINE LinkedList* list_cons(void* head, LinkedList* tail);

INLINE int list_length(LinkedList* list);

INLINE void* list_nth(int index, LinkedList* list);

INLINE void list_remove_head(LinkedList** list);

INLINE LinkedList *list_remove_elem(LinkedList* list, void* elem);

INLINE void list_mapcar(LinkedList* list, void(*function)(void*));

INLINE LinkedList*list_find(LinkedList* list, void* elem);

INLINE void list_free(LinkedList* list);


extern void parse_command(char *, char ***, int *);

extern pid_t execCommand(char *);


  /***********/
 /* Defines */
/***********/

#define MAX_MOUSE_REGION (8)

  /************/
 /* Typedefs */
/************/

typedef struct _rckeys rckeys;

struct _rckeys {
	const char	*label;
	char		**var;
};

typedef struct {
	Pixmap			pixmap;
	Pixmap			mask;
	XpmAttributes	attributes;
} XpmIcon;

  /*******************/
 /* Global variable */
/*******************/

Display		*display;
Window		Root;
int			d_depth;
GC			NormalGC;
XpmIcon		wmgen;
Window		iconwin, win;

  /***********************/
 /* Function Prototypes */
/***********************/

void AddMouseRegion(int index, int left, int top, int right, int bottom);
int CheckMouseRegion(int x, int y);

void openXwindow(int argc, char *argv[], char **, char *, int, int);
void RedrawWindow(void);
void RedrawWindowXY(int x, int y);

void copyXPMArea(int, int, int, int, int, int);
void copyXBMArea(int, int, int, int, int, int);
void setMaskXY(int, int);

void parse_rcfile(const char *, rckeys *);


#define P_READ			1
#define P_WRITE			2

#define null_char		'\0'
#define crlf_char		'\n'
#define slash 			"/"

#define MAX_LINE_LEN	512
#define MAX_VALUE_LEN	256

#define MAX_PATH 		1024

char* p_getfilename_config (char *config_dir, char *config_filename);
void* p_prefs_openfile (char *filename, int openmode);
void p_prefs_closefile (void);
void p_prefs_put_int (char *tagname, int value);
void p_prefs_put_float (char *tagname, float value);
void p_prefs_put_string (char *tagname, char *value);
void p_prefs_put_lf (void);
void p_prefs_put_comment (char *comment);
int p_prefs_get_int (char *tagname);
float p_prefs_get_float (char *tagname);
char* p_prefs_get_string (char *tagname);


