#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "error.h"

extern void warn (const char * fmt, ...)
{
    va_list args ;

    assert (fmt != NULL) ;
    va_start (args, fmt) ;
    fprintf (stderr, "wmmenu warning: ") ;
    vfprintf (stderr, fmt, args) ;
    fprintf (stderr, "\n") ;
    va_end (args) ;
}

extern void error (const char * fmt, ...)
{
    va_list args ;

    assert (fmt != NULL) ;
    va_start (args, fmt) ;
    fprintf (stderr, "wmmenu error: ") ;
    vfprintf (stderr, fmt, args) ;
    fprintf (stderr, "\n") ;
    va_end (args) ;

    exit (EXIT_FAILURE) ;
}
