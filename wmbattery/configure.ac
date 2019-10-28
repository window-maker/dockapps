dnl Process this file with autoconf to produce a configure script.
AC_INIT(wmbattery.c, 2.53)
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
PKG_CHECK_MODULES([Xlib], [x11])
PKG_CHECK_MODULES([Xext], [xext])
PKG_CHECK_MODULES([libXpm], [xpm])
AC_CHECK_LIB(apm, apm_read)
AC_ARG_ENABLE(hal, AS_HELP_STRING([--enable-hal], [enable hal]),
              [], [enable_hal=no])
AS_IF([test x$enable_hal != xno], [
  PKG_CHECK_MODULES([hal], [hal])
  AC_SUBST(USE_HAL, 1)
])
AC_ARG_ENABLE(upower, AS_HELP_STRING([--disable-upower], [disable upower]),
              [], [enable_upower=yes])
AS_IF([test x$enable_upower != xno], [
  PKG_CHECK_MODULES([upower], [upower-glib])
  AC_SUBST(USE_UPOWER, 1)
])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(getopt.h)
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

AC_OUTPUT(Makefile)
