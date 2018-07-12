AC_INIT([wmmisc], [1.2], [wmaker-dev@googlegroups.com])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_PROG_CC
AC_SEARCH_LIBS([nearbyintf], [m], [], [
  AC_MSG_ERROR([unable to find the nearbyintf() function])
])
PKG_CHECK_MODULES([DOCKAPP], [dockapp])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
