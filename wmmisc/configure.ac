AC_INIT([wmmisc], [1.1], [wmaker-dev@googlegroups.com])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_PROG_CC
AC_SEARCH_LIBS([nearbyintf], [m], [], [
  AC_MSG_ERROR([unable to find the nearbyintf() function])
])
PKG_CHECK_MODULES([X11], [x11])
PKG_CHECK_MODULES([XEXT], [xext])
PKG_CHECK_MODULES([XPM], [xpm])
PKG_CHECK_MODULES([DOCKAPP], [dockapp])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
