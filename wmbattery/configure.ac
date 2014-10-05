dnl Process this file with autoconf to produce a configure script.
AC_INIT(wmbattery.c, 2.45)
AC_CONFIG_HEADER(config.h)

AC_CONFIG_AUX_DIR(autoconf)

dnl Checks for the apm device other than /proc/apm.
AC_CHECK_FILES(/dev/apm)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Locate X11.
AC_PATH_X
if test "x${no_x}" = "xyes"; then
   AC_MSG_ERROR(Can't find X windows include files and libraries)
fi
if test "x${x_includes}" != "x"; then
   CPPFLAGS="$CPPFLAGS -I`echo ${x_includes}`"
fi
if test "x${x_libraries}" != "x"; then
   LIBS="$LIBS -L`echo ${x_libraries}`"
fi

dnl Checks for libraries.
AC_CHECK_LIB(X11, XOpenDisplay)
AC_CHECK_LIB(Xext, XShapeCombineMask)
AC_CHECK_LIB(Xpm, XpmReadFileToPixmap)
AC_CHECK_LIB(apm, apm_read)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(X11/xpm.h)
AC_CHECK_HEADERS(X11/extensions/shape.h)
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_HEADERS(apm.h)
dnl FreeBSD needs apm_bios.h
AC_CHECK_HEADERS(machine/apm_bios.h)
dnl NetBSD and OpenBSD need apmvar.h
AC_CHECK_HEADERS(i386/apmvar.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strdup)

AC_OUTPUT(makeinfo:autoconf/makeinfo.in)
