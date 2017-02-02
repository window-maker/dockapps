AC_INIT([wmgtemp], [1.2], [wmaker-dev@lists.windowmaker.org])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_PROG_CC
AC_CHECK_LIB([sensors],[sensors_get_features])
PKG_CHECK_MODULES([X11],[x11])
PKG_CHECK_MODULES([dockapp],[dockapp])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
