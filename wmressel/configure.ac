dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/wmressel.c)

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC

dnl Checks for libraries.
PKG_CHECK_MODULES(Xlib, x11)
PKG_CHECK_MODULES(libXpm, xpm)
PKG_CHECK_MODULES(Xext, xext)
PKG_CHECK_MODULES(Xxf86vm, xxf86vm)
PKG_CHECK_MODULES(Xinerama, xinerama,
        [AC_DEFINE(HAVE_LIBXINERAMA)],
	[AC_MSG_NOTICE(compiling without Xinerama support)])
PKG_CHECK_MODULES(GTK,gtk+-3.0)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

AC_OUTPUT(Makefile src/Makefile)
