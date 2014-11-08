dnl Autoconf is here to detect:
dnl  gnutls, libgcrypt
dnl  default ssh-askpass
dnl and the basics X11, Xext, Xpm
dnl and configure:
dnl  installation prefix
dnl  version

AC_INIT(WMBiff, 0.4.27, wmaker-dev@lists.windowmaker.org, wmbiff)
AC_CONFIG_AUX_DIR(autoconf)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])
dnl make sure autoheader finds version, implicitly defined above.
AH_TEMPLATE([VERSION], [wmbiff's release version])
AC_SUBST(VERSION)

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_RANLIB

if test -n "$GCC"; then
 CFLAGS="$CFLAGS -D_GNU_SOURCE -W -Wall -Wshadow -Wpointer-arith -Wwrite-strings"
 AC_MSG_RESULT(adding -Wall and friends to CFLAGS.)
fi

case `(uname -s) 2>/dev/null` in
    "Darwin")
       CFLAGS="$CFLAGS -no-cpp-precomp"
       AC_MSG_RESULT(adding cpp precompiler workaround for Mac OS X)
       LDFLAGS="$LDFLAGS -framework Security"
       AC_MSG_RESULT(adding -framework Security for Mac OS X)
     ;;
esac

dnl a no-op to force autoconf to seek out the preprocessor now.
AC_CHECK_HEADERS(stdio.h)

dnl for IPv6 support
AC_CHECK_FUNCS(getaddrinfo)

dnl see if we can cheaply "encrypt" passwords in memory
AC_CHECK_FUNCS(memfrob)

dnl AC_CHECK_FUNC(asprintf, , VASPRINTF="vasprintf.c")

dnl replacing the old USE_POLL define
AC_CHECK_FUNCS(poll)

dnl for gnutls-common.h, which defines this if missing.
AC_CHECK_FUNCS(inet_ntop)

dnl declare RETSIGTYPE
AC_TYPE_SIGNAL

dnl solaris
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(socket, connect)
AC_CHECK_LIB(resolv, herror)

dnl Pre-gnutls.
gnutls="ok"
gcrypt="ok"
AC_CHECK_LIB(z, gzopen, [], [gnutls="nope"]) dnl GNUTLS seems to need libz; fail here if it's missing.
dnl perhaps not required anymore:
dnl AC_CHECK_LIB(gdbm, dbminit, [], [gnutls="nope"]) dnl GNUTLS seems to need libgdbm; fail here if it's missing.

dnl Parameter is minimum version
dnl TODO: fix so that GCRYPT is tested only if GNUTLS fails; the dependence
dnl between them makes this turn redundant

AC_ARG_ENABLE(crypto, AC_HELP_STRING([ --disable-crypto ], [ disable gnutls/gcrypt ]),
                      [ if test $enableval != yes; then
                        gnutls="disabled"
                        gcrypt="disabled"
                       fi ])


GNUTLS_MAN_STATUS="This copy of WMBiff was not compiled with GNUTLS."
if test "$gnutls" = "ok"; then
 PKG_CHECK_MODULES([LIBGNUTLS], [gnutls > 2.2.0], [LIBS="$LIBS $LIBGNUTLS_LIBS"
                          CFLAGS="$CFLAGS $LIBGNUTLS_CFLAGS"
                         CPPFLAGS="$CPPFLAGS $LIBGNUTLS_CFLAGS"
                         GNUTLS_COMMON_O="gnutls-common.o"
                         GNUTLS_MAN_STATUS="This copy of WMBiff was compiled with GNUTLS."
                         AC_CHECK_HEADERS(gnutls/gnutls.h) ],
                         [ echo GNUTLS can be found at ftp://gnutls.hellug.gr/pub/gnutls ])
else
 AC_MSG_RESULT(GNUTLS support requires libz.a and libgdbm.a, so will be disabled)
fi

GCRYPT_MAN_STATUS="This copy of WMBiff was not compiled with gcrypt."
if test "$gcrypt" = "ok"; then
 AM_PATH_LIBGCRYPT(1.1.90, [CFLAGS="$CFLAGS $LIBGCRYPT_CFLAGS"
                         CPPFLAGS="$CPPFLAGS $LIBGCRYPT_CFLAGS"
                         GCRYPT_MAN_STATUS="This copy of WMBiff was compiled with gcrypt."
                         AC_CHECK_HEADERS(gcrypt.h) ],
                         [ echo libgcrypt can be found at ftp://ftp.gnupg.org/pub/gcrypt/alpha/libgcrypt/ ]  )
else
        AC_MSG_RESULT(GCRYPT disabled.)
fi;


AC_SUBST(LIBGCRYPT_LIBS)
AC_SUBST(GNUTLS_MAN_STATUS)
AC_SUBST(GCRYPT_MAN_STATUS)
AC_SUBST(GNUTLS_COMMON_O)

dnl regex stuff.
AC_CHECK_HEADERS(regex.h) dnl gnuregex.h
dnl no longer needed AC_CHECK_LIB(gnuregex, re_search) dnl BSD.

dnl X11 stuff.
AC_PATH_XTRA
if test "$no_x" = yes; then
   AC_MSG_ERROR("Really need a working X.  Check config.log to see why configure couldn't find it")
fi
LIBS="$X_LIBS $LIBS"
dnl want to get the -I flags so that later tests for include files work.
dnl the preprocessor is used for check_headers, which doesn't use cflags.
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
AC_CHECK_LIB(X11, XrmParseCommand)
AC_CHECK_LIB(Xext, XShapeCombineMask, , AC_MSG_ERROR(libXext not found or does not include XShape))
AC_CHECK_LIB(Xpm, XpmCreateImageFromXpmImage)

AC_CHECK_HEADERS(X11/xpm.h, [], [AC_CHECK_HEADERS(xpm.h, [], AC_MSG_ERROR([xpm.h is needed]))])
dnl AM_PATH_GTK(1.2.10, , AC_MSG_WARN(Cannot find GTK: Is gtk-config in path?))
dnl AM_PATH_GDK_PIXBUF(0.15.0, [CFLAGS="$CFLAGS $GDK_PIXBUF_CFLAGS"],
                           dnl AC_MSG_ERROR(Cannot find gdk-pixbuf))
dnl GNOME_INIT_HOOK([ ], failure)
dnl GNOME_ORBIT_HOOK([GNOME_LIBS="$GNOME_LIBS $ORBIT_LIBS"
dnl                   GNOME_CFLAGS="$GNOME_CFLAGS $ORBIT_CFLAGS"], failure)
dnl GNOME_GNORBA_HOOK([GNOME_LIBS="$GNOME_LIBS $GNORBA_LIBS"
dnl                   GNOME_CFLAGS="$GNOME_CFLAGS $GNORBA_CFLAGS"], failure)

dnl AC_SUBST(GNOME_LIBS)
dnl AC_SUBST(GNOME_CFLAGS)


dnl see if __attribute__ is supported.  modified from AC_C_INLINE
dnl in the autoconf distribution.
AC_DEFUN([WM_C_ATTRIBUTE],
[AC_CACHE_CHECK([for __attribute__ support], wm_cv_c_attribute,
[wm_cv_c_attribute=no
AC_COMPILE_IFELSE([AC_LANG_SOURCE(
[#ifndef __cplusplus
typedef int foo_t;
foo_t foo () __attribute__((unused));
foo_t foo () {return 0; }
#endif
])],
                    [wm_cv_c_attribute=yes])
])
case $wm_cv_c_attribute in
  yes)AC_DEFINE(HAVE___ATTRIBUTE__, ,
                [Define if '__attribute__' is supported by the compiler])
  ;;
esac
])# WM_C_ATTRIBUTE

WM_C_ATTRIBUTE

dnl Password prompting stuff.
AC_PATH_PROGS(DEFAULT_ASKPASS, ssh-askpass x11-ssh-askpass ssh-askpass-gnome, /usr/bin/ssh-askpass)
AC_PATH_PROG(CVS2CL, cvs2cl)
AC_DEFINE_UNQUOTED(DEFAULT_ASKPASS, "$DEFAULT_ASKPASS", [Program to use for querying the user for a password: redefine if not on a debian system])

AC_CHECK_HEADERS(CoreServices/CoreServices.h Security/Security.h)

dnl Skin files; note - this is duplicated in wmbiff/Makefile.am
dnl haven't thought of a way around it.
if test "x$prefix" != xNONE; then
  SKINDIR="$prefix/share/wmbiff/skins"
else
  SKINDIR="$ac_default_prefix/share/wmbiff/skins"
fi
dnl where to install em
AC_SUBST(SKINDIR)
dnl where to find em
AC_DEFINE_UNQUOTED(DEFAULT_SKIN_PATH,
                   "$SKINDIR:/usr/share/wmbiff/skins:/usr/local/share/wmbiff/skins:.", [Path to use when finding skins (modified pixmaps)] )

dnl We're done.
AC_CONFIG_FILES(Makefile wmbiff/Makefile wmgeneral/Makefile wmbiff/wmbiffrc.5 autoconf/Makefile scripts/Makefile)
AC_OUTPUT
dnl remind not to write the file.
chmod 0444 wmbiff/wmbiffrc.5
