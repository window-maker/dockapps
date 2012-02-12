#!/bin/sh

# $Id: acinclude.m4,v 1.3 1999/07/24 22:01:51 daeron Exp $

dnl
dnl DA_CHECK_LIB(NAME, FUNCTION, EXTRALIBS)
dnl
AC_DEFUN(DA_CHECK_LIB,
[
LDFLAGS_old="$LDFLAGS"
LDFLAGS="$LDFLAGS $lib_search_path"
AC_CHECK_LIB([$1],[$2],yes=yes,no=no,[$3])
LDFLAGS="$LDFLAGS_old"
])


dnl
dnl DA_CHECK_HEADER(NAME)
dnl
AC_DEFUN(DA_CHECK_HEADER,
[
CPPFLAGS_old="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $inc_search_path"
AC_CHECK_HEADER([$1])
CPPFLAGS="$CPPFLAGS_old"
])

