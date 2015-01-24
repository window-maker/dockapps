#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/* POSIX */
#include <unistd.h>

#include "utils.h"
#include "error.h"
#include "types.h"

static char * ReadAll (FILE * f, int offset)
{
    char buf [10*1024] ;
    size_t iRead, nRead ;
    char * ret ;

    clearerr (f) ;

    iRead = 0 ;
    nRead = 0 ;
    while (iRead < sizeof buf)
    {
        nRead = fread (buf+iRead, 1, (sizeof buf)-iRead, f) ;
        if (nRead <= 0) break ;
        iRead += nRead ;
    }

    if (nRead > 0)
    {
        assert (iRead == sizeof buf) ;
        ret = ReadAll (f, offset+iRead) ;
    }
    else
    if (ferror (f) != 0)
    {
        error ("read file: %s", strerror (errno)) ;
        ret = NULL ;
    }
    else
    {
        ret = malloc (offset+iRead+1) ;
        if (ret == NULL) error ("failed to allocate memory") ;
        ret[offset+iRead] = EOS ;
    }
    memcpy (ret+offset, buf, iRead) ;

    return ret ;
}

extern char * File_ReadAll (FILE * f)
{
    assert (f != NULL) ;
    return ReadAll (f, 0) ;
}

static const char * getHome (void)
{
    static char homeDir [80] = "" ;
    static int initialized = 0 ;
    if (! initialized)
    {
	const char * env ;
	env = getenv ("HOME") ;
	if (env != NULL && env[0] != EOS)
	{
	    sprintf (homeDir, "%.*s", (int)(sizeof homeDir)-1, env) ;
	}
	initialized = 1 ;
    }
    return homeDir[0] != EOS ? homeDir : NULL ;
}

extern bool File_FindInPath (char * out, int outSz,
    const char * path, const char * basename)
{
    char name [FILENAME_MAX] ;
    int len ;

    assert (path != NULL) ;
    assert (basename != NULL) ;
    assert (out != NULL) ;

    /* regular path */
    if (access (basename, F_OK) == 0)
    {
        sprintf (out, "%.*s", outSz-1, basename) ;
        return true ;
    }
    else
    /* relative to home directory */
    if (strncmp (basename, "~/", 2) == 0 && getHome () != NULL)
    {
	sprintf (name, "%s%s", getHome (), basename+1) ;
	if (access (name, F_OK) == 0)
	{
	    sprintf (out, "%.*s", outSz-1, name) ;
	    return true ;
	}
	else
	{
	    return false ;
	}
    }
    else
    /* forbid relative to PATH just like shell */
    /* NB: absolute non-existent files also drop here */
    if (strchr (basename, '/') != NULL)
    {
        return false ;
    }
    else
    {
        while (*path != EOS)
        {
            len = strcspn (path, ":") ;

	    if (strncmp (path, "~/", 2) == 0)
	    {
		sprintf (name, "%s%.*s/%s",
		    getHome (), len-1, path+1, basename) ;
	    }
	    else
	    {
		sprintf (name, "%.*s/%s", len, path, basename) ;
	    }

            if (access (name, F_OK) == 0)
            {
                sprintf (out, "%.*s", outSz-1, name) ;
                return true ;
            }

            path += len ;
            while (*path == ':') path++ ;
        }

        return false ;
    }
}

extern char * File_ReadOutputFromCommand (const char * cmd)
{
    FILE * out ;
    char * ret ;

    if ((out = popen (cmd, "r")) == NULL)
    {
	error ("when calling command: %s, error: ",
	    cmd, strerror (errno)) ;
	ret = NULL ;
    }
    else
    {
	ret = File_ReadAll (out) ;
	pclose (out) ;

	/* if return value is correct, remove trailing whitespace */
	if (ret != NULL)
	{
	    char *lastNotWhite, *cursor ;
	    char c ;

	    cursor = lastNotWhite = ret ;

	    while ((c = *cursor++) != EOS)
	    {
		if (strchr (" \t\n", c) == NULL)
		{
		    lastNotWhite = cursor ;
		}
	    }

	    *lastNotWhite = EOS ;
	}
    }

    return ret ;
}
