dnl @synopsis CHECK_LIBWRASTER([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
dnl
dnl This macro searches for an installed libwraster library. If nothing
dnl was specified when calling configure, it searches first in /usr/local
dnl and then in /usr. If the --with-libwraster=DIR is specified, it will try
dnl to find it in DIR/include/wraster.h and DIR/lib/libwraster.a. If --without-libwraster
dnl is specified, the library is not searched at all.
dnl
dnl It defines the symbol HAVE_LIBWRASTER if the library is found. You should
dnl use autoheader to include a definition for this symbol in a config.h
dnl file.
dnl
dnl Sources files should then use something like
dnl
dnl #ifdef HAVE_LIBWRASTER
dnl #include <wraster.h>
dnl #endif /* HAVE_LIBWRASTER */
dnl
dnl @version 1.0
dnl based on CHECK_ZLIB by Loic Dachary <loic@senga.org>
dnl

AC_DEFUN([CHECK_LIBWRASTER],
#
# Handle user hints
#
         [AC_MSG_CHECKING(if libwraster is wanted)
         AC_ARG_WITH(libwraster,
                     [  --with-libwraster=DIR  root directory path of libwraster installation [defaults to
                          /usr/local or /usr if not found in /usr/local]
  --without-libwraster   to disable libwraster usage completely],
                     [if test "$withval" != no ; then
                          LIBWRASTER_HOME="$withval"
                          AC_MSG_RESULT([yes: libraries ${LIBWRASTER_HOME}/lib  includes ${LIBWRASTER_HOME}/include])
                      else
                          AC_MSG_RESULT(no)
                      fi],
                     [LIBWRASTER_HOME=/usr/local
                      if test ! -f "${LIBWRASTER_HOME}/include/wraster.h"
                      then
                         LIBWRASTER_HOME=/usr
                         if test ! -f "${LIBWRASTER_HOME}/include/wraster.h"
                         then
                             LIBWRASTER_HOME=/usr/X11R6
                         fi
                      fi
                      AC_MSG_RESULT([yes: libraries ${LIBWRASTER_HOME}/lib  includes ${LIBWRASTER_HOME}/include])
                     ])

#
# Locate libwraster, if wanted
#
         if test -n "${LIBWRASTER_HOME}"
         then
             LDFLAGS="$LDFLAGS -L${LIBWRASTER_HOME}/lib"
             CPPFLAGS="$CPPFLAGS -I${LIBWRASTER_HOME}/include"
             AC_CHECK_LIB(wraster, RCreateContext, $1, $2)
         else
             $2
         fi
    ])
