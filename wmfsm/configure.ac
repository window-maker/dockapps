dnl Hey emacs -*- mode: m4 -*- thanks
dnl Process this file with autoconf to create a configure script
AC_INIT([wmfsm], [0.36], [wmaker-dev@lists.windowmaker.org])
AC_CONFIG_SRCDIR([wmfsm/wmfsm.c])
AM_INIT_AUTOMAKE
AC_CANONICAL_HOST
AM_CONFIG_HEADER(config.h)

AM_SANITY_CHECK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl Checks for libraries
AC_PATH_XTRA
X11LIBS="-L$x_libraries -I$x_includes -lX11 -lXpm -lXext -ldockapp"
AC_SUBST(X11LIBS)
AC_MSG_CHECKING([to see if we can use X])
AC_TRY_LINK([#include <X11/Xlib.h>],,AC_MSG_RESULT([yes]),AC_MSG_RESULT([no]); exit 1)

dnl Checks how to call statfs
AC_MSG_CHECKING([how to call statfs])
AC_TRY_COMPILE([#include <sys/vfs.h>
#include <sys/param.h>
#include <sys/mount.h>
],
[struct statfs a; statfs("/", &a);],
[AC_MSG_RESULT([2 arguments])
AC_DEFINE(
	STATFS_2_ARGUMENTS, 1,
	[Define if statfs takes two arguments (like linux)])
ac_statfs_args=2],

AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/statfs.h>
],
[struct statfs a; statfs("/", &a, sizeof(struct statfs), 0);],
[AC_MSG_RESULT([4 arguments])
AC_DEFINE(
	STATFS_4_ARGUMENTS, 1,
	[Define if statfs takes four arguments (like SunOS)])
],
AC_MSG_RESULT([Can not determine])))


dnl Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/statfs.h sys/vfs.h sys/param.h sys/mount.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_CHECK_FUNCS(gethostname strcspn strdup strspn strstr statfs)
AC_CHECK_FUNCS(getopt_long, have_getopt=true)
AM_CONDITIONAL(GETOPT, test x$have_getopt = xtrue)

dnl Set default XPM
define([xpmlist], esyscmd([find wmfsm/ -name 'wmfsm_master_*.xpm' -exec basename {} .xpm \;| sed 's/wmfsm_master_//']))
AC_ARG_WITH([xpm],
	AS_HELP_STRING([--with-xpm],
	[Configure wmfsm to use specified xpm file for appearance. Defaults to
	'highcolor'. Choices are: ]
	xpmlist),
	[],
        [with_xpm=highcolor])

xpm="wmfsm/wmfsm_master_${with_xpm}.xpm"
if test ! -e "${xpm}"; then
   AC_MSG_ERROR([Cannot find xpm file '${xpm}'.])
fi

echo "configuring for ${with_xpm} appearance..."
rm -f wmfsm/wmfsm_master.xpm
ln -sr ${xpm} wmfsm/wmfsm_master.xpm

AC_OUTPUT(Makefile wmfsm/Makefile)

