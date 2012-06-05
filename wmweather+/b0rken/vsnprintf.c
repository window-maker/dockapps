#include "config.h"

/*  Copyright (C) 2002  Brad Jorsch <anomie@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifdef vsnprintf
# undef vsnprintf
#endif

#include <stdio.h>
#include <stdarg.h>

#if defined(HAVE_WORKING_VSNPRINTF)

/* vsnprintf works, nothing to do */

#elif !defined(VSNPRINTF_BOGUS_RETVAL)

/* vsnprintf is b0rken, but the return value is ok (thus, the only problem is
 * NULL) */

int rpl_vsnprintf(char *str, size_t size, const char *format, va_list ap){
    if(str==NULL || size==0){{
        char foo[3];
        return vsnprintf(foo, 3, format, ap);
    }} else {
        return vsnprintf(str, size, format, ap);
    }
}

#elif !defined(VSNPRINTF_IS_VSPRINTF) && defined(HAVE_VPRINTF)

/* vsnprintf's retval is bogus, so we compensate. */

static FILE *devnull;

#ifndef va_copy
# ifdef __va_copy
#  define va_copy(dest, src) __va_copy(dest, src)
# else
#  include <string.h>
#  define va_copy(dest, src) memcpy(&dest, &src, sizeof(va_list))
# endif
#endif

int rpl_vsnprintf(char *str, size_t size, const char *format, va_list ap){
    va_list ap2;
    int r;

    va_copy(ap2, ap);

#ifdef VSNPRINTF_NULL_OK
    vsnprintf(str, size, format, ap);
#else
    if(str==NULL || size==0){{
        char foo[3];
        vsnprintf(foo, 3, format, ap);
    }} else {
        vsnprintf(str, size, format, ap);
    }
#endif

    if(devnull==NULL){
        if((devnull=fopen("/dev/null", "w"))==NULL){
            perror("Couldn't open /dev/null for writing");
            exit(72);
        }
    }

    r=vfprintf(devnull, format, ap2);
    va_end(ap2);
    return r;
}

#else

/* OK, we're screwed */

# error "vsnprintf is so broken we can't compensate. Sorry."

#endif
