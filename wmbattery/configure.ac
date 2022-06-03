dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.61])
AC_INIT([wmbattery], [2.55], [wmaker-dev@googlegroups.com])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([wmbattery.c])
AC_CONFIG_AUX_DIR([autoconf])

dnl Checks for the apm device other than /proc/apm.
AC_CHECK_FILES([/dev/apm])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl Locate X11.
AC_PATH_X
if test "x${no_x}" = "xyes"; then
   AC_MSG_ERROR([Can't find X windows include files and libraries])
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
AC_CHECK_LIB([apm], [apm_read])
AC_ARG_ENABLE([hal], [AS_HELP_STRING([--enable-hal], [enable hal])],
              [], [enable_hal=no])
AS_IF([test x$enable_hal != xno], [
  PKG_CHECK_MODULES([hal], [hal])
  AC_SUBST([USE_HAL], [1])
])
AC_ARG_ENABLE([upower], [AS_HELP_STRING([--disable-upower], [disable upower])],
              [], [enable_upower=yes])
AS_IF([test x$enable_upower != xno], [
  PKG_CHECK_MODULES([upower], [upower-glib])
  AC_SUBST([USE_UPOWER], [1])
])

dnl Checks for header files.
AC_CHECK_HEADERS([fcntl.h getopt.h stdint.h sys/ioctl.h sys/socket.h unistd.h])
dnl FreeBSD needs apm_bios.h
AC_CHECK_HEADERS([machine/apm_bios.h])
dnl NetBSD and OpenBSD need apmvar.h
AC_CHECK_HEADERS([i386/apmvar.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

dnl Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([alarm strcasecmp strchr strdup strerror strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
