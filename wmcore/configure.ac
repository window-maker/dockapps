#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([wmcore], [0.0.3], [wmaker-dev@googlegroups.com])
AC_CONFIG_SRCDIR([wmcore.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-xz])

# Checks for programs.
: ${CFLAGS:=""}
AC_PROG_CC
AC_PROG_INSTALL

# Checks for pkg-config modules.
PKG_CHECK_MODULES([pkg_config], [x11 xext xpm dockapp])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/param.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
