#define _POSIX_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "menu.h"
#include "utils.h"
#include "types.h"
#include "error.h"

#define MAXENTRIES 50

#define BLANKS " \t\f\r\n"
#define SPACES " \t\f"
#define ENDLINE "\r\n"

static int NbEntries = 0 ;
static int Rows = 1 ;
static const char * Pixmaps [MAXENTRIES] ;
static const char * Commands [MAXENTRIES] ;
static char * MenuFileText = NULL ;
static char MenuPath [FILENAME_MAX] = "" ;
static time_t MenuAge = 0 ;

static void ParseMenu (void)
{
    char * p ;

    assert (MenuFileText != NULL) ;
    p = MenuFileText ;
    p += strspn (p, BLANKS) ;
    NbEntries = 0 ;
    Pixmaps[0] = NULL ;
    Commands[0] = NULL ;

    while (NbEntries < MAXENTRIES && *p != EOS) switch (*p++)
    {
        case '#' :
            p += strcspn (p, ENDLINE) ;
            p += strspn (p, BLANKS) ;
            break ;

        case '"' :
            Pixmaps[NbEntries] = p ;
            p += strcspn (p, "\"") ;
            *p++ = EOS ;
            p += strspn (p, SPACES) ;
            break ;

        default :
            if (Pixmaps[NbEntries] == NULL)
                warn ("entry #%d has no pixmap", NbEntries) ;
            else Commands[NbEntries++] = p-1 ;

            p += strcspn (p, ENDLINE) ;
            *p++ = EOS ;
            p += strspn (p, BLANKS) ;

            Commands[NbEntries] = NULL ;
            Pixmaps[NbEntries] = NULL ;
            break ;
    }

    if (*p != EOS)
        warn ("too many entries") ;
}

extern void Menu_LoadFromFile (const char * name)
{
    char path [FILENAME_MAX] ;
    const char * home ;
    FILE * f ;
    struct stat finfo ;

    assert (name != NULL) ;

    if (strchr (name, '/') == NULL && (home = getenv ("HOME")) != NULL &&
            home[0] != EOS)
        sprintf (path, "%s/.wmmenu/%s", home, name) ;
    else sprintf (path, "%s", name) ;

    if ((f = fopen (path, "r")) == NULL)
        error ("can't open %s", path) ;
    else
    {
        if (fstat (fileno (f), & finfo) == 0)
        {
            MenuAge = finfo.st_mtime ;
        }

        sprintf (MenuPath, "%.*s", (int)(sizeof MenuPath)-1, path) ;

        if (MenuFileText != NULL) free (MenuFileText) ;
        MenuFileText = File_ReadAll (f) ;
    }

    fclose (f) ;

    ParseMenu () ;
}

extern int Menu_GetNbRows(void)
{
    assert (Rows > 0) ;
    return Rows ;
}

extern void Menu_SetNbRows (const char *s)
{
    int h;

    h = atoi(s) ;
    if (h > 0) Rows = h ;
}

extern int Menu_GetNbColumns (void)
{
    assert (NbEntries > 1 && Rows > 0) ;
    /*
    Remove 1 entry used for header, then apply the formula:
        UNITS = int (floor ((USED - 1) / PERUNIT)) + 1
    (USED is NbEntries - 1)
    */
    return ((NbEntries - 2) / Rows) + 1 ;
}

extern int Menu_GetNbEntries (void)
{
    assert (NbEntries > 1) ;
    return NbEntries-1 ;
}

extern const char * Menu_GetEntryPixmap (int i)
{
    assert (0 <= i && i < NbEntries-1) ;
    return Pixmaps[i+1] ;
}

extern const char * Menu_GetEntryCommand (int i)
{
    assert (0 <= i && i < NbEntries-1) ;
    return Commands[i+1] ;
}

extern const char * Menu_GetPixmap (void)
{
    assert (0 < NbEntries) ;
    return Pixmaps[0] ;
}

extern const char * Menu_GetTitle (void)
{
    assert (0 < NbEntries) ;
    return Commands[0] ;
}

extern int Menu_HasChanged (void)
{
    struct stat finfo ;

    if (stat (MenuPath, & finfo) == 0 && finfo.st_mtime > MenuAge)
    {
        return 1 ; /* should reload */
    }
    else
    {
        return 0 ; /* don't try to reload */
    }
}

extern void Menu_Reload (void)
{
    FILE * f ;
    struct stat finfo ;

    if ((f = fopen (MenuPath, "r")) != NULL)
    {
        if (fstat (fileno (f), & finfo) == 0)
        {
            MenuAge = finfo.st_mtime ;
        }

        if (MenuFileText != NULL) free (MenuFileText) ;
        MenuFileText = File_ReadAll (f) ;
        fclose (f) ;

        ParseMenu () ;
    }
}
