dnl @synopsis CHECK_LIBPCRE([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl
dnl This macro searches for an installed libpcre library. If nothing
dnl was specified when calling configure, it searches first in /usr/local
dnl and then in /usr. If the --with-libpcre=DIR is specified, it will try
dnl to find it in DIR/include/pcre.h and DIR/lib/libpcre.a. If --without-libpcre
dnl is specified, the library is not searched at all.
dnl
dnl It defines the symbol HAVE_LIBPCRE if the library is found. You should
dnl use autoheader to include a definition for this symbol in a config.h
dnl file.
dnl
dnl Sources files should then use something like
dnl
dnl #ifdef HAVE_LIBPCRE
dnl #include <pcre.h>
dnl #endif /* HAVE_LIBPCRE */
dnl
dnl @version 1.0
dnl based on CHECK_ZLIB by Loic Dachary <loic@senga.org>
dnl

AC_DEFUN([CHECK_LIBPCRE],
#
# Handle user hints
#
         [AC_MSG_CHECKING(if libpcre is wanted)
         AC_ARG_WITH(libpcre,
                     [  --with-libpcre=DIR  root directory path of libpcre installation [defaults to
                          /usr/local or /usr if not found in /usr/local]
  --without-libpcre   to disable libpcre usage completely],
                     [if test "$withval" != no ; then
                          LIBPCRE_HOME="$withval"
                          AC_MSG_RESULT([yes: libraries ${LIBPCRE_HOME}/lib  includes ${LIBPCRE_HOME}/include])
                      else
                          AC_MSG_RESULT(no)
                      fi],
                     [LIBPCRE_HOME=/usr/local
                      if test ! -f "${LIBPCRE_HOME}/include/pcre.h"
                      then
                         LIBPCRE_HOME=/usr
                      fi
                      AC_MSG_RESULT([yes: libraries ${LIBPCRE_HOME}/lib  includes ${LIBPCRE_HOME}/include])
                     ])

#
# Locate libpcre, if wanted
#
         if test -n "${LIBPCRE_HOME}"
         then
             LDFLAGS="$LDFLAGS -L${LIBPCRE_HOME}/lib"
             CPPFLAGS="$CPPFLAGS -I${LIBPCRE_HOME}/include"
             AC_CHECK_LIB(pcre, pcre_compile, $1, $2)
         else
             $2
         fi
    ])
