# configure.ac -- Process this file with autoconf to produce configure.

dnl Initialization stuff.
AC_INIT(wmnotify, 1.0.0)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(src/wmnotify.c)
AM_CONFIG_HEADER(config.h:config-h.in)
dnl Checking if the NEWS file has been updated to reflect the current version.
AM_INIT_AUTOMAKE(check-news)
AM_SILENT_RULES([yes])

dnl Testing the C compiler.
AM_PROG_CC_C_O
AC_LANG_C

dnl Checking for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(stdlib.h unistd.h errno.h assert.h)
AC_CHECK_HEADERS(string.h strings.h)

dnl Checking for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T

dnl Basic CFLAGS values
CFLAGS="${CFLAGS} -Wall"

dnl Checking for POSIX threads library.
ACX_PTHREAD([CFLAGS="$CFLAGS $PTHREAD_CFLAGS" LDFLAGS="$PTHREAD_LIBS $LDFLAGS" CC="$PTHREAD_CC"],
            [AC_MSG_ERROR([POSIX thread support required])])

dnl Trying to locate the X window system's includes and libraries, and sets the
dnl variables x_includes and x_libraries to their locations. Also adds the
dnl required include flags to X_CFLAGS and required linking flags to X_LIBS.
AC_PATH_XTRA
CFLAGS="${CFLAGS} ${X_CFLAGS}"
LIBS="${LIBS} ${X_PRE_LIBS} ${X_LIBS} ${X_EXTRA_LIBS}"

dnl Checking for X11 library.
AC_CHECK_LIB(X11, XOpenDisplay, LIBS="${LIBS} -lX11",
             echo "Can't find X11 library" ; exit 1, "${X_LIBS}")

dnl Checking for Xpm library and headers.
AC_CHECK_HEADERS(X11/xpm.h, ,echo "Can't find header file for library Xpm" ; exit 1)
AC_CHECK_LIB(Xpm, XpmCreatePixmapFromXpmImage, LIBS="${LIBS} -lXpm",
             echo "Can't find Xpm library" ; exit 1, "${X_LIBS}")

dnl Checking for Xext library and headers.
AC_CHECK_HEADERS(X11/extensions/shape.h, ,
echo "Can't find header file for library Xext" ; exit 1)
AC_CHECK_LIB(Xext, XShapeCombineMask, LIBS="${LIBS} -lXext",
             echo "Can't find Xext library" ; exit 1, "${X_LIBS}")

dnl Checks for libsndfile
AC_ARG_ENABLE(libsndfile,
	[  --enable-libsndfile     Enable libsndfile support for audible notification (default=yes)],
	[ac_cv_enable_libsndfile=$enableval], [ac_cv_enable_libsndfile=yes])
AC_MSG_CHECKING([whether to use libsndfile])
if test x$ac_cv_enable_libsndfile = xyes; then
	AC_MSG_RESULT(yes)
	PKG_CHECK_MODULES([SNDFILE], sndfile >= 1.0.2, :, ac_cv_enable_libsndfile=no)
	if test x$ac_cv_enable_libsndfile = xyes; then
	    AC_DEFINE([HAVE_SNDFILE],1,[Set to 1 to enable libsndfile support.])
	fi
else
	AC_MSG_RESULT(no)
fi

dnl Checks for OpenSSL
AC_ARG_ENABLE(ssl,
	[  --enable-ssl            Enable SSL support using OpenSSL (default=yes)],
	[ac_cv_enable_ssl=$enableval], [ac_cv_enable_ssl=yes])
AC_MSG_CHECKING([whether to use OpenSSL])
if test x$ac_cv_enable_ssl = xyes; then
	AC_MSG_RESULT(yes)
	PKG_CHECK_MODULES([OPENSSL], openssl >= 0.9.6, :, ac_cv_enable_ssl=no)
	if test x$ac_cv_enable_ssl = xyes; then
            AC_DEFINE([HAVE_SSL],1,[Set to 1 to enable OpenSSL support.])
	fi
else
	AC_MSG_RESULT(no)
fi

CFLAGS="${CFLAGS} ${SNDFILE_CFLAGS} ${OPENSSL_CFLAGS}"
LIBS="${SNDFILE_LIBS} ${OPENSSL_LIBS} ${LIBS}"

AC_SUBST(CFLAGS)
AC_SUBST(LIBS)
AC_SUBST(ac_aux_dir)

dnl Creating output file(s)
AC_OUTPUT(Makefile src/Makefile doc/Makefile)

dnl Output the configuration summary
echo
echo "Configuration summary:"
echo
echo "  Install path:   ${prefix}"
echo "  Compiler:       ${CC}"
echo "  Compiler flags: ${CFLAGS}"
echo "  Linker flags:   ${LIBS}"
echo "  OpenSSL support:    $ac_cv_enable_ssl"
echo "  libsndfile support: $ac_cv_enable_libsndfile"
if test "x${ac_cv_enable_libsndfile}" = xno; then
  echo
  echo "  WARNING: Althought the 'libsndfile' library is not mandatory, without it"
  echo "           you won't have audible notifications."
fi

echo
echo "Configure finished. Type 'make' to build."
