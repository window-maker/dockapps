dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.52)

dnl AC_INIT(src/main.c)
dnl AM_INIT_AUTOMAKE(wmcpuload, 0.9.0)
AC_INIT(wmcpuload, 1.1.0pre5, wmaker-dev@lists.windowmaker.org)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR(src/main.c)
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S

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

lib_search_path="$lib_search_path $XLFLAGS"
inc_search_path="$inc_search_path $XCFLAGS"

AC_SUBST(X_LIBRARY_PATH)

dnl Shape extension
dnl ===============
AC_CHECK_LIB(Xext, XShapeCombineMask, [XLIBS="$XLIBS -lXext"],
    [echo "The shape extension stuff could not be found in the X client libraries"
     exit 1],
    $X_LIBS $X_EXTRA_LIBS -lX11)

dnl XPM library
dnl ===========
AC_CHECK_LIB(Xpm, XpmCreatePixmapFromData, [XLIBS="$XLIBS -lXpm"],
    [echo "The libXpm library was not found, but is necessary to build this library"
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
             [  --enable-debug          turn on debugging [default=no]],, enable_debug=no)
if test "$enable_debug" = yes; then
  DFLAGS="-Wall -g -ansi -pedantic"
fi
AC_SUBST(DFLAGS)


dnl ============
dnl Determine OS
dnl ============

ignore_nice=no
use_smp=no

case ${target_os} in
linux*)
  OS=linux
  ignore_nice=yes
  use_smp=yes
  ;;
freebsd*)
  OS=freebsd
  ignore_nice=yes
  LIBS="$LIBS -lkvm"
  SETGID_FLAGS="-g kmem -m 2755 -o  root"
  ;;
openbsd*)
  OS=openbsd
  ignore_nice=yes
  ;;
netbsd*)
  OS=netbsd
  ignore_nice=yes
  ;;
bsdi*)
  OS=bsdi
  ignore_nice=yes
  ;;
solaris*)
  OS=solaris
  LIBS="$LIBS -lkstat"
  ;;
cygwin*)
  OS=cygwin
  ;;
irix*)
  OS=irix
  use_smp=yes
  ;;
darwin*)
  OS=darwin
  ;;
*)
  echo ""
  echo "Sorry, ${target_os} is not supported yet"
  echo ""
  exit 1
  ;;
esac

AC_SUBST(OS)
AC_SUBST(SETGID_FLAGS)
AC_SUBST(LIBS)

if test "$ignore_nice" = yes; then
  AC_DEFINE(IGNORE_NICE, 1, [use '--ignore-nice' option])
fi
if test "$use_smp" = yes; then
  AC_DEFINE(USE_SMP, 1, [use SMP support])
fi


dnl =============================
dnl Checks for library functions.
dnl =============================

LIBRARY_SEARCH_PATH="$lib_search_path"
HEADER_SEARCH_PATH="$inc_search_path"

AC_SUBST(LIBRARY_SEARCH_PATH)
AC_SUBST(HEADER_SEARCH_PATH)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS(fcntl.h memory.h stddef.h stdlib.h string.h strings.h sys/param.h sys/time.h unistd.h)

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS(select)

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 libdockapp/Makefile
		 doc/Makefile
		 icons/Makefile
		 contrib/Makefile
		 contrib/wmcpuload.spec])
AC_OUTPUT

