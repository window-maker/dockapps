dnl Process this file with autoconf to produce a configure script.
dnl
dnl (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
dnl
dnl vim:ts=8:sts=2:shiftwidth=2:expandtab:foldmethod=marker

dnl {{{1
AC_INIT([fookb], [3.1], [wmaker-dev@lists.windowmaker.org])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([fookb.c])

dnl Checks for programs.
AC_PROG_CC

dnl Checks for libraries.

PKG_CHECK_MODULES([Xlib], [x11])
PKG_CHECK_MODULES([libXpm], [xpm])
PKG_CHECK_MODULES([WUtil], [WUtil])

AC_ARG_ENABLE(
  [wmaker],
  [  --enable-wmaker         create windowmaker dockable application],
  [ 
    if test $enable_wmaker = yes ; then
      AC_DEFINE(WMAKER)
    fi
  ],
  [
    AC_CHECK_PROG(HAVE_WMAKER, wmaker, yes, no)
  
    if test $HAVE_WMAKER = yes ; then
      AC_DEFINE(WMAKER)
    fi
  ]
)

AC_OUTPUT(Makefile)
