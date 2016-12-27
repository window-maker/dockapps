#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <X11/X.h>
#include <X11/xpm.h>

#include <time.h>
#include "wmgeneral.h"
#include "stringlist.h"

#include "xpm/back.xpm"
#include "xpm/star.xpm"
#include "xpm/no_icon.xpm"



/* 
 *  Delay between refreshes (in microseconds) 
 */
#define DELAY 10000L

#define ICONSIZE 64


/* Database structures */

enum th_type {THEME, WALL};

struct theme {
	enum th_type type;
	char       * path;
/*	int          theme;
	char       * options;*/
};

struct category {
	char * name;
	LIST * themes;
};


LIST * thm_db;
LIST * cat_db;

/* Themes directories, last one will be replaced */
char * themes_dir[] = {
	"/usr/share/WindowMaker/Themes",
	"/usr/X11R6/share/WindowMaker/Themes",
	"/usr/local/share/WindowMaker/Themes",
	"$GNUSTEP_USER_ROOT/Library/WindowMaker/Themes"
};


/* Wallpapers directories, last one will be replaced */
char * wallpapers_dir[] = {
	"/usr/share/WindowMaker/Backgrounds",
	"/usr/X11R6/share/WindowMaker/Backgrounds",
	"/usr/local/share/WindowMaker/Backgrounds",
	"$GNUSTEP_USER_ROOT/Library/WindowMaker/Backgrounds"
};


char hid_cat_db[] = ".wmThemeCh.cat.db";
char     cat_db_file[] =  "wmThemeCh.cat.db";
char reg_cat   [] = "categories";

int current_category = 0;
unsigned long int auto_switch_delay = 0;
unsigned long int auto_switch_next = 0;

char * mask_bits;

XpmAttributes	Attributes;
Colormap	cmap;


int
BuildBitmask ()
{
	int i, j, off = 0;
	int size = (ICONSIZE * ICONSIZE - 1) / 8 + 1;

	mask_bits = malloc (size);

	/* must have iconsize % 8 = 0 */

	for (i = 0; i < 4 * ICONSIZE / 8; i++) {
		mask_bits[off++] = 0;
	}
	for (i = 0; i < ICONSIZE - 8; i++) {
		mask_bits[off++] = 0xf0;
		for (j = 0; j < (ICONSIZE / 8 - 2); j++) {
			mask_bits[off++] = 0xff;
		}
		mask_bits[off++] = 0x0f;
	}
	for (i = 0; i < 4 * ICONSIZE / 8; i++) {
		mask_bits[off++] = 0;
	}

	return 0;
}


#define ADD_CATEGORY(string) \
	do {                                                         \
		struct category * newcat;                            \
                                                                     \
		newcat         = malloc (sizeof (struct category) ); \
		newcat->name   = strdup (string);                    \
		newcat->themes = create_list ();                     \
		add_item (cat_db, (unsigned int) newcat);            \
	} while (0);

#define PROCESS_CATEGORY_FILE \
	if ( (catfile = fopen (catname,"r") ) )                     \
	{                                                           \
		while (fscanf (catfile, "%255s", ImageName) == 1) { \
			ADD_CATEGORY(ImageName);                    \
		}                                                   \
		fclose (catfile);                                   \
	}                                                           \
	free (catname);

/* Register categories recognized by system + user */
int
get_categories ()
{
	char * catname;
	char * home = getenv ("HOME");
	char   ImageName[256];
	FILE * catfile;

	ADD_CATEGORY("*");

	catname = malloc (strlen (home) + strlen (PACKAGE_NAME)
		+ strlen (reg_cat) + 4);

	sprintf (catname, "%s/.%s/%s", home, PACKAGE_NAME, reg_cat);

	PROCESS_CATEGORY_FILE

	catname = malloc (strlen (PACKAGE) + strlen (reg_cat) + 13);

	sprintf (catname, "/usr/share/%s/%s", PACKAGE, reg_cat);

	PROCESS_CATEGORY_FILE

	catname = malloc (strlen (PACKAGE) + strlen (reg_cat) + 19);

	sprintf (catname, "/usr/local/share/%s/%s", PACKAGE, reg_cat);

	PROCESS_CATEGORY_FILE

	return 0;
}


enum de_type { ERROR = -1, IGNORED, STYLE, DE_THEME, SUBDIR, IMAGE };

/* Return values :
 * -1 : stat error
 *  0 : Ignored entries : ., .., category
 *  1 : regular file (.style)
 *  2 : theme directory (.themed)
 *  3 : sub-directory
 *  4 : image
 */
enum de_type
entry_type (char * dir, char * entry)
{
	char * name = malloc (strlen (dir) + strlen (entry) + 2);
	struct stat buf;

	sprintf (name, "%s/%s", dir, entry);

	if (stat (name, &buf) ) {
		perror ("stat ");
		free (name);
		return ERROR;
	}

	free (name);

	if (! S_ISDIR (buf.st_mode) ) {
		if (! strcmp (entry,cat_db_file ) ) return IGNORED;
		if (! strcmp (entry,hid_cat_db) ) return IGNORED;
		/* TODO : Use file (1) */
		if (strstr (entry, ".xpm" ) ) return IMAGE;
		if (strstr (entry, ".png" ) ) return IMAGE;
		if (strstr (entry, ".jpg" ) ) return IMAGE;
		if (strstr (entry, ".jpeg") ) return IMAGE;
		if (strstr (entry, ".tif" ) ) return IMAGE;
		if (strstr (entry, ".ppm" ) ) return IMAGE;

		return STYLE;
	}

	if (strstr (entry, ".themed")                  ) return DE_THEME;
	if (strcmp (entry,"." ) && strcmp (entry,"..") ) return SUBDIR;

	return IGNORED;
}


struct theme *
insert_into_thm (char * path, enum th_type type)
{
	struct theme * new_theme = malloc (sizeof (struct theme) );

	new_theme->type = type;
	new_theme->path = strdup (path);
	add_item (thm_db, (int) new_theme);

	return new_theme;
}

int
insert_into_cat (struct theme * theme, LIST * cat_list)
{
	int i, j, lcat;
	char * lcat_nam;

	add_item ( ( (struct category *) get_item (cat_db, 0) ) -> themes,
		(int) theme);

	/* for each category theme is in */
	for (j = 0; (lcat = get_item (cat_list, j) ) != -1; j++) {
		int ucat;
		lcat_nam = (char *) lcat;
		/* for each user category */
		for (i=1; (ucat = get_item (cat_db, i) ) != -1; i++) {
			struct category * cat = (struct category *) ucat;
			/* check if they match */
			if (! strcmp (lcat_nam, cat->name) ) {
				add_item (cat->themes, (int) theme);
				break;
			}
		}
	}

	return 0;
}

LIST *
theme_specific_cat (char * path, LIST * cat_list)
{
	FILE * th_cat;
	char * th_cat_nam = malloc (strlen (path) + strlen (hid_cat_db) + 2);
	int i, ret, version, subversion;
	char cat_nam[512];
	LIST * new_list;

	sprintf (th_cat_nam, "%s/%s", path, cat_db_file);
	th_cat = fopen (th_cat_nam, "r");

	if (! th_cat) {
		sprintf (th_cat_nam, "%s/%s", path, hid_cat_db);
		th_cat = fopen (th_cat_nam, "r");

		if (! th_cat) {
			free (th_cat_nam);
			return cat_list;
		}
	}

	ret = fscanf (th_cat, "# wmThemeCh categories database file version %i.%i",
		&version, &subversion);

	if (ret != 2) {
		printf("%s : invalid signature\n", th_cat_nam);
		fclose (th_cat);
		free (th_cat_nam);
		return cat_list;
	}

	if (version > 1) {
		printf("%s : incompatible version\n", th_cat_nam);
		fclose (th_cat);
		free (th_cat_nam);
		return cat_list;
	}

	new_list = create_list ();
	add_list (new_list, cat_list);

	while ( (ret = fscanf (th_cat, "%s", cat_nam) ) == 1) {
		for (i=0; get_item (cat_db,i) != -1; i++) {
			struct category * cat =
				(struct category *) get_item (cat_db,i);
			if (! strcmp (cat_nam, cat->name) ) {
/*				printf("%s : '%s'\n", th_cat_nam, cat_nam); */
				add_item (new_list, (int) strdup (cat->name) );
				break;
			}
		}
	}

	fclose (th_cat);
	free (th_cat_nam);

	return new_list;
}

/* Read "categories" file in path.
 * Themes in path and its subdirs are belonging to categories listed 
 * in that file if these categories are defined by user profile.
 * Updates cat_list accordingly.
 */
int
add_dir_cat (const char * path, LIST * cat_list)
{
	FILE * th_cat;
	char * th_cat_nam = malloc (strlen (path) + strlen (hid_cat_db) + 2);
	int ret, version, subversion;
	char cat_nam[512];

	sprintf (th_cat_nam, "%s/%s", path, hid_cat_db);
	th_cat = fopen (th_cat_nam, "r");

	if (! th_cat) {
		free (th_cat_nam);
		return 0;
	}

	ret = fscanf (th_cat, "# wmThemeCh categories database file version %i.%i",
		&version, &subversion);

	if (ret != 2) {
		printf("%s : invalid signature\n", th_cat_nam);
		fclose (th_cat);
		free (th_cat_nam);
		return 0;
	}


	while ( (ret = fscanf (th_cat, "%s", cat_nam) ) == 1) {
		int i;

		for (i=0; get_item (cat_db, i) != -1; i++) {
			struct category * cat =
				(struct category *) get_item (cat_db, i);
			if (! strcmp (cat_nam, cat->name) ) {
/*				printf("%s : '%s'\n", th_cat_nam, cat_nam); */
				add_item (cat_list, (int) strdup (cat->name) );
				break;
			}
		}
	}

	fclose (th_cat);
	free (th_cat_nam);

	return 0;
}


int
get_themes (char * path, LIST * par_cat_list)
{
	struct dirent ** namelist;
	LIST * cat_list = create_list();
	int    themes_count = 0;

	int count = scandir (path, &namelist, NULL, alphasort);

	if (count < 0) {
		free (cat_list);
		return -1;
	}

	if (par_cat_list) add_list (cat_list, par_cat_list);

	add_dir_cat (path, cat_list);

	while (count--) {
		char fullname[512];
		struct theme * new_theme;
		LIST * th_spec_cat;

		sprintf (fullname, "%s/%s", path, namelist[count]->d_name);

		switch (entry_type (path, namelist[count]->d_name) ) {
		case ERROR   :
		case IGNORED : break;
		case STYLE   :
			new_theme = insert_into_thm (fullname, THEME);
			insert_into_cat (new_theme, cat_list);
			break;
		case DE_THEME  :
			new_theme = insert_into_thm (fullname, THEME);
			th_spec_cat = theme_specific_cat (fullname, cat_list);
			insert_into_cat (new_theme, th_spec_cat);
			if (th_spec_cat != cat_list) {
				delete_list (th_spec_cat);
			}
			break;
		case SUBDIR :
			get_themes (fullname, cat_list);
		case IMAGE  : break;
		}
	}

	free (cat_list);

	return 0;
}

int
get_wallpapers (char * path, LIST * par_cat_list)
{
	struct dirent ** namelist;
	LIST * cat_list = create_list();
	int    themes_count = 0;

	int count = scandir (path, &namelist, NULL, NULL);

	if (count < 0) {
		free (cat_list);
		return -1;
	}

	if (par_cat_list) add_list (cat_list, par_cat_list);

	add_dir_cat (path, cat_list);

	while (count--) {
		char fullname[512];
		struct theme * new_theme;

		sprintf (fullname, "%s/%s", path, namelist[count]->d_name);

		switch (entry_type (path, namelist[count]->d_name) ) {
		case ERROR   :
		case IGNORED :
		case STYLE   :
		case DE_THEME: break;
		case SUBDIR  :
			get_wallpapers (fullname, cat_list);
			break;
		case IMAGE   :
			new_theme = insert_into_thm (fullname, WALL);
			insert_into_cat (new_theme, cat_list);
		}
	}

	free (cat_list);

	return 0;
}

int
BuildDatabase ()
{
	char * user_theme_dir     = "/Library/WindowMaker/Themes";
	char * user_wallpaper_dir = "/Library/WindowMaker/Backgrounds";
	char * gnustep_user_root  = getenv("GNUSTEP_USER_ROOT");

	int dir_idx;

	thm_db = create_list();
	cat_db = create_list();

	if (!gnustep_user_root) {
		char * home = getenv ("HOME");
		gnustep_user_root = malloc (strlen (home) + 9);
		sprintf (gnustep_user_root, "%s/GNUstep", home);
	}

	themes_dir[3] = malloc (strlen (gnustep_user_root)
		+ strlen (user_theme_dir) + 1);
	sprintf (themes_dir[3], "%s%s", gnustep_user_root, user_theme_dir);

	wallpapers_dir[3] = malloc (strlen (gnustep_user_root)
		+ strlen (user_wallpaper_dir) + 1);
	sprintf (wallpapers_dir[3], "%s%s", gnustep_user_root, user_wallpaper_dir);

	get_categories ();

	for (dir_idx=0; dir_idx < 4; dir_idx++) {
		get_themes     (    themes_dir[dir_idx], NULL);
		get_wallpapers (wallpapers_dir[dir_idx], NULL); 
	}

	return 0;
}





char *
get_pixmap_path (const char * icon_name)
{
	char * path;
	char * home = getenv ("HOME");
	struct stat stat_buf;

	path = malloc ( strlen (home) + strlen (PACKAGE_NAME)
	              + strlen (icon_name) + 8);

	sprintf (path, "%s/.%s/%s.xpm", home, PACKAGE_NAME, icon_name);

	if (! stat (path, &stat_buf) ) {
		return path;
	}

	free (path);
	path = malloc (strlen (PACKAGE) + strlen (icon_name) + 23);

	sprintf (path, "/usr/local/share/%s/%s.xpm", PACKAGE, icon_name);

	if (! stat (path, &stat_buf) ) {
		return path;
	}

	free (path);
	path = malloc (strlen (PACKAGE) + strlen (icon_name) + 17);

	sprintf (path, "/usr/share/%s/%s.xpm", PACKAGE, icon_name);

	if (! stat (path, &stat_buf) ) {
		return path;
	}

	free (path);
	return NULL;
}


int
DrawPixmap (char * XpmFileName)
{
	static Pixmap NewPixmap, NewShapeMask = 0;
	static int ret, havePixmap= 0;

/*	copyXPMArea(5, 69, 54, 54, 5, 5); * Clear window */
	copyXPMArea(4, 4, 56, 56, 4, 4); /* Clear window */

	if (havePixmap) {
		/* 
		* free up the colors, if we alloc'd some before 
		*/
		if (Attributes.nalloc_pixels > 0) 
			XFreeColors(display, cmap,  Attributes.alloc_pixels, 
				Attributes.nalloc_pixels, 0);
		/*
		 *  Free last pixmap -- we dont need it anymore...
		 *  A ShapeMask is returned if the Pixmap
		 *  had the color None used.
		 *  We could probably change Transparent to None
		 *  to make use of this, but for now,
		 *  lets just ignore it...
		 */
		if (NewShapeMask != 0) XFreePixmap (display, NewShapeMask);

		XFreePixmap(display, NewPixmap);

		XpmFreeAttributes (&Attributes);

		havePixmap= 0;
	} /* havePixmap */

	/*
	 *   Grab new pixmap. Accept a reasonable color match.
	 */
	Attributes.valuemask   = XpmExactColors | XpmCloseness
		                       | XpmReturnAllocPixels;
	Attributes.exactColors = 0;
	Attributes.closeness   = 40000;

	/* TODO : We might cache icons */

	if (XpmFileName) {
		if (strcmp (XpmFileName,"*") ) {
			ret = XpmReadFileToPixmap (display, Root, XpmFileName,
				&NewPixmap, &NewShapeMask, &Attributes);
		} else {
			ret = XpmCreatePixmapFromData (display, Root, star_xpm,
				&NewPixmap, &NewShapeMask, &Attributes);
		}
	} else {
		ret = XpmCreatePixmapFromData (display, Root, no_icon_xpm,
			&NewPixmap, &NewShapeMask, &Attributes);
	}

	if (ret >= 0)
	{
		int Height = Attributes.height;
		XCopyArea (display, NewPixmap, wmgen.pixmap, NormalGC,
			0, 0, Attributes.width, Height, 4, 4);
		havePixmap= 1;
	}

	/*
	 * Make changes visible
	 */
	RedrawWindow();

	return 0;
}





int
ChangeCategory ()
{
	char * XpmFileName = NULL;

	current_category++;
	if (get_item (cat_db, current_category) == -1) current_category = 0;
	if (current_category) {
		XpmFileName = get_pixmap_path (
			( (struct category *) get_item
				(cat_db, current_category) ) -> name);
	} else {
		XpmFileName = strdup("*");
	}

	DrawPixmap (XpmFileName);

	if (XpmFileName) free (XpmFileName);

	return 0;
}


int
ChangeTheme ()
{
	int fd;
	char Command[512];

	static unsigned int oldidx = -1;
	unsigned int val, i;
	struct category * cur_cat = (struct category *)
		get_item (cat_db, current_category);

	for (i = 0; get_item (cur_cat->themes, i) != -1; i++);

	switch (i) {
	case  0 : return 1;
	case  1 : val = 0; break;
	default :
		fd = open ("/dev/random", O_RDONLY);
		
		/* We're disallowing "changing" to the same */
		read (fd, &val, sizeof (int) );
		val = val % (i - 1);
		if (val >= oldidx) val++;

		close (fd);
		oldidx = val;
	}

	switch ( ( (struct theme *) get_item (cur_cat->themes, val) ) -> type) {
	case THEME : 
		sprintf (Command, "setstyle \"%s\"", ( (struct theme *)
			get_item (cur_cat->themes, val) ) -> path);
		break;
	case WALL :
		sprintf (Command, "wmsetbg -u \"%s\"", ( (struct theme *)
			get_item (cur_cat->themes, val) ) -> path);
	}

	system(Command);

	auto_switch_next = time(NULL) + (auto_switch_delay * 60);
	return 0;
}


void
pressEvent (XButtonEvent *xev)
{
	/* Mouse's left button clicked */
	if (xev->button == Button1) ChangeTheme ();

	/* Mouse's right button clicked */
	if (xev->button == Button3) ChangeCategory ();
}


int
ProcessXEvents ()
{
	while (XPending (display) )
	{
		XEvent event;

		XNextEvent (display, &event);

		switch (event.type) {
		case Expose        : RedrawWindow ();             break;
		case ButtonPress   : pressEvent (&event.xbutton); break;
		case ButtonRelease : break;
		}
	}

	return 0;
}





void
print_usage ()
{
	printf ("%s\n", PACKAGE_STRING);
	printf ("Report problems to <%s>\n", PACKAGE_BUGREPORT);
	printf ("\nusage: %s [options]...\n\n", PACKAGE_NAME);
	printf ("\t-display <Display>\tUse alternate X display.\n");
	printf ("\t-h\t\t\tDisplay help screen.\n\n");
}


void
ParseCMDLine (int argc, char *argv[])
{
	int  i, x;

	for (i = 1; i < argc; i++) {
        	if (strcmp(argv[i], "-a") == 0) {
                        if (argc > i+1) {
				x = sscanf(argv[i+1], "%u", &auto_switch_delay);
				if (x != 1) {
					auto_switch_delay = 0;
					printf("Found auto-option but could not parse parameter\n");
					exit(-1);
				} else {
					++i;
				}
			} else {
				printf("Found auto-option but number is missing!\n");
				exit(-1);
			}
		} else {

			if (!strcmp (argv[i], "-display") ) {
				++i;
			} else {
				print_usage();
				exit (1);
			}
		}
	}
}


int
Init (int argc, char * argv[])
{
	char * basename = current_category
		? ( (struct category *)
			get_item (cat_db, current_category)
		  ) -> name
		: NULL;

	char * path = NULL;

	ParseCMDLine  (argc, argv);
	BuildDatabase ();

	if (current_category) {
		path = get_pixmap_path (basename);
	} else {
		path = strdup ("*");
	}

	BuildBitmask();
	openXwindow (argc, argv, back_xpm, mask_bits,
		ICONSIZE, ICONSIZE);

	cmap = DefaultColormap(display, DefaultScreen(display));

	Attributes.nalloc_pixels = 0;

	DrawPixmap (path);

	if (path) free (path);

	return 0;
}


int
main (int argc, char *argv[])
{
	unsigned int delay_timer = 0;
	Init (argc, argv);

        if (auto_switch_delay > 0) {
		auto_switch_next = time(NULL) + (auto_switch_delay * 60);
	}

	/* TODO : use sigsuspend */
	
	while (1) {
		ProcessXEvents ();
		delay_timer += DELAY;
		if ( delay_timer > 10000000L ) { /* check every 10 seconds */
			delay_timer = 0;
        		if (auto_switch_delay > 0) {
				if (auto_switch_next < time(NULL)) {
					ChangeTheme();
				}
			}
		}
		usleep (DELAY);
	}
}
