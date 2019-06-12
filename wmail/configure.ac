dnl Process this file with autoconf to produce a configure script.
AC_INIT([wmail], [2.2], [wmaker-dev@googlegroups.com])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SRCDIR([src/wmail.c])
AM_INIT_AUTOMAKE([foreign])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for pkg-config modules.
PKG_CHECK_MODULES([X11], [x11])
PKG_CHECK_MODULES([dockapp], [dockapp])

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h strings.h sys/time.h])
CPPFLAGS_old="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $dockapp_CFLAGS $X11_CFLAGS"
AC_CHECK_HEADERS([libdockapp/dockapp.h dockapp.h])
CPPFLAGS="$CPPFLAGS_old"

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_FNMATCH
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([strdup strstr memcpy])

dnl Checks for libraries.

dnl Enable Debugging?
AC_ARG_ENABLE(debug, [  --enable-debug    enable debugging ],
              enable_debug=yes, enable_debug=no)
AC_MSG_CHECKING([whether to enable debugging])
AC_MSG_RESULT([$enable_debug])
AM_CONDITIONAL([ENABLE_DEBUG], [test x$enable_debug = xyes])

dnl Use delt's pixmaps instead of the default ones?
AC_ARG_ENABLE(delt_xpms, [  --enable-delt-xpms    enable delt's pixmaps ],
              enable_delt_xpms=yes, enable_delt_xpms=no)
AC_MSG_CHECKING([whether to enable delt's pixmaps])
AC_MSG_RESULT([$enable_delt_xpms])
AM_CONDITIONAL([ENABLE_DELT_XPMS], [test x$enable_delt_xpms = xyes])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
