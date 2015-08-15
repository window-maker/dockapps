AC_INIT([libdockapp],[0.7.0],[wmaker-dev@lists.windowmaker.org])
AC_CONFIG_SRCDIR([src/dockapp.h])
AM_INIT_AUTOMAKE

LT_INIT

dnl Specify whether to install font and examples
dnl ============================================

AC_ARG_WITH(font,
    [  --with-font             install the dockapp font (default=yes)],
    with_font=$withval,
    with_font=yes)

AC_ARG_WITH(examples,
    [  --with-examples         install the examples (default=yes)],
    with_examples=$withval,
    with_examples=yes)

AM_CONDITIONAL(COND_FONT, [test "$with_font" = yes])
AM_CONDITIONAL(COND_EXAMPLES, [test "$with_examples" = yes])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for libraries.
AC_PATH_XTRA

PKG_CHECK_MODULES([X11],[x11])
PKG_CHECK_MODULES([Xext],[xext])
PKG_CHECK_MODULES([xpm],[xpm])

dnl Checks for header files.


dnl Checks for typedefs, structures, and compiler characteristics.

dnl Debugging
dnl =========
AC_ARG_ENABLE(debug,
	    [  --enable-debug        enable debugging ],, enable_debug=no)
if test "$enable_debug" = yes; then
    DFLAGS="-Wall -g -ansi -pedantic"
fi
AC_SUBST(DFLAGS)

dnl Font-util macros
dnl ================

m4_ifndef([XORG_FONT_MACROS_VERSION],
	  [m4_fatal([must install X.Org font-util 1.1 or later before running autoconf/autogen])])
XORG_FONT_MACROS_VERSION(1.1)
XORG_FONT_COMMON_UTILS()
XORG_FONTDIR([misc])

AC_CONFIG_FILES([Makefile
	src/Makefile
	fonts/Makefile
	examples/Makefile])
AC_OUTPUT
