# $Id: configure.ac,v 1.19 2005/02/14 19:14:58 godisch Exp $
# copyright (c) 2002-2005 Martin A. Godisch <martin@godisch.de>
# ----------------------------------------------------------------------------
AC_PREREQ(2.59)
AC_INIT([wmwork],[0.2.5],[martin@godisch.de])
AC_REVISION([$Revision: 1.19 $])
AC_CONFIG_SRCDIR([wmwork.c])
AC_CONFIG_HEADER([config.h])
# ----------------------------------------------------------------------------
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
# ----------------------------------------------------------------------------
AC_PATH_XTRA
LDFLAGS="$LDFLAGS $X_LIBS"
AC_CHECK_LIB([X11],  [XNextEvent],,              [AC_MSG_ERROR(cannot find libX11.so)],  [$X_LIBS])
AC_CHECK_LIB([Xext], [XShapeCombineMask],,       [AC_MSG_ERROR(cannot find libXext.so)], [$X_LIBS])
AC_CHECK_LIB([Xpm],  [XpmCreatePixmapFromData],, [AC_MSG_ERROR(cannot find libXpm.so)],  [$X_LIBS])
# ----------------------------------------------------------------------------
AC_LANG([C])
AC_C_CONST
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h],, [AC_MSG_ERROR([required header file missing])])
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([atexit gettimeofday mkdir strcspn strdup strerror strspn strstr strtol],, [AC_MSG_ERROR([required standard library function missing])])
# ----------------------------------------------------------------------------
AC_CONFIG_FILES([Makefile wmwork.1])
AC_OUTPUT
# ----------------------------------------------------------------------------
