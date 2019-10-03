dnl Process this file with autoconf to produce a configure script.
dnl
dnl (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
dnl
dnl vim:ts=8:sts=2:shiftwidth=2:expandtab:foldmethod=marker

dnl {{{1
AC_INIT([fookb], [4.1], [wmaker-dev@googlegroups.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([fookb.c])

dnl Checks for programs.
AC_PROG_CC

dnl Checks for libraries.

PKG_CHECK_MODULES([Xlib], [x11])
PKG_CHECK_MODULES([libXpm], [xpm])
PKG_CHECK_MODULES([WUtil], [WUtil])
PKG_CHECK_MODULES([libdockapp], [dockapp])

AC_OUTPUT(Makefile fookb.1)
