AC_INIT([wmtop], [0.85], [wmaker-dev@googlegroups.com])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_CANONICAL_HOST
AS_CASE([$host_os],
  [linux*],
	[AC_DEFINE(LINUX)],
  [*freebsd*],
	[AC_DEFINE(FREEBSD)],
  [AC_MSG_ERROR([unknown operating system "$host_os".])]
)
AC_PROG_CC
PKG_CHECK_MODULES([x11],[x11])
PKG_CHECK_MODULES([xext],[xext])
PKG_CHECK_MODULES([xpm],[xpm])
PKG_CHECK_MODULES([dockapp],[dockapp])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
