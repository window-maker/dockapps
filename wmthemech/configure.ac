#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([wmThemeCh], [0.2],
	[alexhessemann@users.sourceforge.net],
	[wmthemech])
AC_CONFIG_SRCDIR([src/wmThemeCh.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

AC_PATH_XTRA

LIBS="$LIBS $X_EXTRA_LIBS $X_LIBS"
CFLAGS="$X_CFLAGS $CFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
LDFLAGS="$LDFLAGS $X_PRE_LIBS"

AC_CHECK_LIB(X11,XOpenDisplay, [], [AC_MSG_ERROR([libx11-dev missing])])
AC_CHECK_LIB(Xext,XShapeCombineMask)
AC_CHECK_LIB(Xpm,XpmReadFileToPixmap, [], [AC_MSG_ERROR([libxpm-dev missing])])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h \
	X11/X.h X11/Xlib.h X11/xpm.h X11/extensions/shape.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([strcspn strdup strspn strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
