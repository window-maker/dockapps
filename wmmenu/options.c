/* ANSI */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* POSIX */
#include <getopt.h>

#include "options.h"
#include "version.h"
#include "error.h"
#include "utils.h"
#include "types.h"
#include "menu.h"

#define MAXOPTIONS (20)

#ifndef ETCDIR
#define ETCDIR "/usr/local/etc"
#endif
#define DEFAULTPATH (ETCDIR ":.")

#define BLANKS " \t\f\r\n"
#define SPACES " \t\f"
#define ENDLINE "\r\n"

int TileXSize = 0 ;
int TileYSize = 0 ;

char MenuName [40] = "" ;

static char MyPixmapPath [1024] = "" ;
char * PixmapPath = MyPixmapPath ;

static char MyTilePath [FILENAME_MAX] = "" ;
char * TilePath = MyTilePath ;

static char MyHighlightPath [FILENAME_MAX] = "" ;
char * HighlightPath = MyHighlightPath ;

bool ClickOnly = false ;
bool AutoScale = true ;
bool HighlightBehind = false ;
int HideTimeout = 1 ;

int Options_Argc = 0 ;
char * Options_Argv [MAXOPTIONS] ;

static void Usage (void)
{
    printf (
"Usage: wmmenu [<options>...][-- <dockoptions>...]\n"
"Normal options:\n"
"-m MENUNAME    set the name of the menu file to load from ~/.wmmenu\n"
"-g WxH         force width and height of tile\n"
"-l XPMFILE     set the pixmap used to highlight icon under cursor\n"
"-t XPMFILE     set the pixmap used as button bar background\n"
"-O click       bar is only triggered by clicks on the tile, not moves\n"
"-O noautoscale disable automatic pixmap scaling to tile size\n"
"-O behind      draw highlight pixmap behind icon, not above\n"
"-O hide=N      set bar hiding timeout to N ms (default one)\n"
"-r ROWS        set number of menu rows (default one)\n"
"-v             print version information\n"
"-h             print this help message\n"
"Note:\n"
"  -t, -l and '-O behind' can also be specified with the defaults file.\n"
""
    ) ;
}

static void SetGeometry (const char * val)
{
    if (sscanf (val, "%dx%d", &TileXSize, &TileYSize) != 2)
        error ("bad tile geometry \"%s\"", val) ;
    else
    if (64 < TileXSize || TileXSize < 0)
        error ("incorrect tile width %d", TileXSize) ;
    else
    if (64 < TileYSize || TileYSize < 0)
        error ("incorrect tile height %d", TileYSize) ;
}

static void SetTilePath (const char * value)
{
    char *ptrToFree = NULL ;

    if (value != NULL && value[0] == '!')
    {
	ptrToFree = File_ReadOutputFromCommand (value+1) ;

	if ((value = ptrToFree) == NULL)
	    value = "" ;
    }

    sprintf (MyTilePath, "%.*s", (int)(sizeof MyTilePath)-1, value) ;

    if (ptrToFree != NULL)
	free (ptrToFree) ;
}

static void SetHighlightPath (const char * value)
{
    sprintf (MyHighlightPath, "%.*s", (int)(sizeof MyHighlightPath)-1, value) ;
}

static void SetHideTimeout (const char * value)
{
    HideTimeout = atoi (value) ;
    if (HideTimeout <= 0)
    {
	warn ("hide_timeout must be a strictly positive integer") ;
	HideTimeout = 1 ;
    }
    /* no maximum -- for players */
}

static void AddPixmapPath (const char * value)
{
    int curLen ;
    int szLeft ;

    curLen = strlen (MyPixmapPath) ;
    szLeft = (sizeof MyPixmapPath) - curLen ;

    /* don't forget to count space for ':' and ending EOS */
    sprintf (MyPixmapPath+curLen, ":%.*s", szLeft-2, value) ;
}

static void InitDefaults (void)
{
    char dftPixmapPath [FILENAME_MAX] ;
    const char * home ;

    if ((home = getenv ("HOME")) != NULL && home[0] != EOS)
    {
        sprintf (dftPixmapPath, "%.*s/.wmmenu",
            (int)(sizeof dftPixmapPath)-10, home) ;
    }
    else
    {
        strcpy (dftPixmapPath, ".") ;
    }

    strcpy (MyPixmapPath, dftPixmapPath) ;
}

static void ParseDefaults (char * text)
{
    char * p ;
    const char * name ;
    const char * value ;

    assert (text != NULL) ;
    p = text ;
    p += strspn (p, BLANKS) ;

    while (*p != EOS) switch (*p++)
    {
        case '#' :
            p += strcspn (p, ENDLINE) ;
            p += strspn (p, BLANKS) ;
            break ;

        default :
            name = p-1 ;
            p += strcspn (p, SPACES) ;
            *p++ = EOS ;
            p += strspn (p, SPACES) ;
            value = p ;
            p += strcspn (p, ENDLINE) ;
            *p++ = EOS ;
            p += strspn (p, BLANKS) ;

            if (streq (name, "geometry")) SetGeometry (value) ;
            else
            if (streq (name, "xpmpath"))
            {
                AddPixmapPath (value) ;
            }
            else
            if (streq (name, "tile"))
            {
                SetTilePath (value) ;
            }
	    else
	    if (streq (name, "highlight"))
	    {
		SetHighlightPath (value) ;
	    }
	    else
	    if (streq (name, "highlight_behind"))
	    {
		HighlightBehind = true ;
	    }
	    else
	    if (streq (name, "hide_timeout"))
	    {
		SetHideTimeout (value) ;
	    }
            else error ("unknown setting \"%s\"", name) ;

            break ;
    }
}

extern void Options_ParseDefaults (void)
{
    char pathList [1000] ;
    char path [FILENAME_MAX] ;
    const char * home ;
    FILE * f ;

    if ((home = getenv ("HOME")) != NULL && *home != EOS)
    {
        sprintf (pathList, "%s/.wmmenu:%s", home, DEFAULTPATH) ;
    }
    else
    {
        strcpy (pathList, DEFAULTPATH) ;
    }

    if (File_FindInPath (path, sizeof path, pathList, "defaults") &&
        (f = fopen (path, "r")) != NULL)
    {
        char * text ;

        text = File_ReadAll (f) ;
        fclose (f) ;

        InitDefaults () ;
        ParseDefaults (text) ;
        free (text) ;
    }
}

extern void Options_Parse (int argc, char ** argv)
{
    int opt ;

    assert (argv != NULL) ;
    assert (argc > 0) ;

    Options_Argc = 0 ;
    Options_Argv[Options_Argc++] = MenuName ;

    while ((opt = getopt (argc, argv, "hg:l:m:r:t:vO:")) != EOF) switch (opt)
    {
        case 'g' :
            SetGeometry (optarg) ;
            break ;

        case 'm' :
            sprintf (MenuName, "%.*s", (int)(sizeof MenuName)-1, optarg) ;
            break ;

        case 'r' :
            Menu_SetNbRows (optarg) ;
            break ;

        case 't' :
	    SetTilePath (optarg) ;
            break ;

	case 'l' :
	    sprintf (MyHighlightPath, "%.*s",
		(int)(sizeof MyHighlightPath)-1, optarg) ;
	    break ;

        case 'O' :
            if (streq (optarg, "click")) ClickOnly = true ;
            else if (streq (optarg, "noautoscale")) AutoScale = false ;
	    else if (streq (optarg, "behind")) HighlightBehind = true ;
	    else if (streql (optarg, "hide=", 5)) SetHideTimeout (optarg+5) ;
            else printf ("ignoring unknown feature \"%s\"\n", optarg) ;
            break ;

        case 'v' :
            printf ("wmmenu (c) F.COUTANT v%s\n", VERSION) ;
            exit (EXIT_SUCCESS) ;
            break ;

        case 'h' :
            Usage () ;
            exit (EXIT_SUCCESS) ;
            break ;

        case '?' :
            Usage () ;
            exit (EXIT_FAILURE) ;
            break ;
    }

    if (MenuName[0] == EOS)
        error ("no menu file name specified") ;
    if (MyTilePath[0] == EOS)
        warn ("no tile pixmap specified, using internal default") ;

    while (optind < argc && Options_Argc < MAXOPTIONS)
        Options_Argv[Options_Argc++] = argv[optind++] ;
}

extern void Options_SetMenuName (const char * name)
{
    sprintf (MenuName, "%.*s", (int)(sizeof MenuName)-1, name) ;
}
