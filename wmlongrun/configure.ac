dnl Process this file with autoconf to produce a configure script.
AC_INIT([wmlongrun], [0.3.0pre1], [wmaker-dev@lists.windowmaker.org])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Checks for header files.
AC_HEADER_STDC

dnl
dnl Specify paths to look for libraries and headers
dnl ===============================================
AC_ARG_WITH(libs-from,
                [  --with-libs-from        pass compiler flags to look for libraries],
                        [lib_search_path="$withval $lib_search_path"])

AC_ARG_WITH(incs-from,
                [  --with-incs-from        pass compiler flags to look for header files],
                        [inc_search_path="$withval $inc_search_path"])


dnl ===========================================
dnl             Stuff that uses X
dnl ===========================================

AC_PATH_XTRA

X_LIBRARY_PATH=$x_libraries

XCFLAGS="$X_CFLAGS"

XLFLAGS="$X_LIBS"

XLIBS="-lX11 $X_EXTRA_LIBS"


lib_search_path="$lib_search_path $XLFLAGS -L/usr/local/lib"
inc_search_path="$inc_search_path $XCFLAGS -I/usr/local/include"

AC_SUBST(X_LIBRARY_PATH)

AC_CHECK_LIB(Xext, XShapeCombineMask,,
    [echo "The shape extension stuff could not be found in the X client
    libraries"
    exit 1],
    $X_LIBS $X_EXTRA_LIBS -lX11)

AC_CHECK_LIB(Xpm, XpmCreatePixmapFromData,,
    [echo "The libXpm library was not found, but is necessary to build this
    library"
    exit 1],
    $X_LIBS $X_EXTRA_LIBS -lX11)

AC_SUBST(XCFLAGS)
AC_SUBST(XLFLAGS)
AC_SUBST(XLIBS)

AC_SUBST(X_EXTRA_LIBS)

dnl ===============================================
dnl             End of stuff that uses X
dnl ===============================================

dnl =========
dnl Debugging
dnl =========
AC_ARG_ENABLE(debug,
             [  --enable-debug          enable debugging ],,enable_debug=no)
if test "$enable_debug" = yes; then
  DFLAGS="-Wall -g -ansi -pedantic"
fi
AC_SUBST(DFLAGS)

dnl ============
dnl Check for OS
dnl ============

AC_CANONICAL_HOST

arch=""
case ${host_os} in
linux* | *-gnu | gnu*)
  SETUID_FLAGS="-m 4755 -o root -g root"
  OS=linux
  AC_DEFINE([LINUX], [1], [use Linux specific code])
  ;;
freebsd*)
  OS=freebsd
dnl  AC_DEFINE(FREEBSD)
  ;;
*)
  echo ""
  echo "${host_os} is NOT supported"
  echo ""
  exit 1
  ;;
esac

AC_SUBST(OS)
AC_SUBST(SETUID_FLAGS)
AC_SUBST(LIBS)


dnl =============================
dnl Checks for library functions.
dnl =============================

LIBRARY_SEARCH_PATH="$lib_search_path"
HEADER_SEARCH_PATH="$inc_search_path"

AC_SUBST(LIBRARY_SEARCH_PATH)
AC_SUBST(HEADER_SEARCH_PATH)


dnl Checks for header files.
AC_CHECK_HEADERS(fcntl.h sys/time.h unistd.h)
AC_CHECK_HEADERS(string.h,, [
AC_CHECK_HEADERS(strings.h,,)])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_FUNCS(select)

dnl Checks for library functions.

AC_OUTPUT(Makefile src/Makefile doc/Makefile)
