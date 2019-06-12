///////////////////////////////////////////////////////////////////////////////
// common.h
// common defines and typedefs, part of wmail
//
// Copyright 2000-2002, Sven Geisenhainer <sveng@informatik.uni-jena.de>.
// Copyright 2016-2017, Doug Torrance <dtorrance@piedmont.edu>.
// Copyright 2019, Jeremy Sowden <jeremy@azazel.net>.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions, and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions, and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef _COMMON_H_fdda99de15ce3f21ce7faf607a5b4985_
#define _COMMON_H_fdda99de15ce3f21ce7faf607a5b4985_


///////////////////////////////////////////////////////////////////////////////
// defines

// X display to open
#define WMAIL_DISPLAY        ""

// ticker scroll frame rate per seconds
#define WMAIL_FPS            60

// default mail check interval in seconds
#define WMAIL_CHECK_INTERVAL 1

// default client-button command
#define WMAIL_CLIENT_CMD     "xterm -e mail"

// filename of the checksum-file
#define WMAIL_CHECKSUM_FILE  ".wmail-cksums"

// filename of the config-file
#define WMAIL_RC_FILE        ".wmailrc"

#define WMAIL_READSTATUS     "O"

///////////////////////////////////////////////////////////////////////////////
// typedefs

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
# define bool _Bool
# define false 0
# define true 1
# define __bool_true_false_are_defined 1
#endif


///////////////////////////////////////////////////////////////////////////////
// prototypes

void ABORT( const char *fmt, ... );
void WARNING( const char *fmt, ... );
char *MakePathName( const char *dir, const char *file );

#ifdef DEBUG

void TRACE( const char *fmt, ... );
#define ASSERT( EXPR ) ((void)(EXPR ? 0 : ABORT( "%s(%i): Assertion failed: \"%s\"\n", __FILE__, __LINE__, #EXPR )))
#define VERIFY( EXPR ) ASSERT( EXPR )

#else

#define TRACE( fmt... )
#define ASSERT( EXPR )
#define VERIFY( EXPR )

#endif

#define PREFIX_MATCHES(S, P, CS) ( CS ? strncmp : strncasecmp ) ( (S), (P), sizeof (P) - 1) == 0

#endif
