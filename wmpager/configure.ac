AC_PREREQ([2.60])
AC_INIT([wmpager], [1.2], [wmaker-dev@googlegroups.com])
AC_CONFIG_SRCDIR([src/wmpager.c])
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-dist-gzip])
AM_SILENT_RULES

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

# Checks for libraries.
PKG_CHECK_MODULES([X], [x11 xext xpm])
AC_SUBST([X_CFLAGS])
AC_SUBST([X_LIBS])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday select strdup])

AC_CONFIG_FILES([Makefile
				 man/Makefile
                 src/Makefile
				 themes/Makefile])
AC_OUTPUT
