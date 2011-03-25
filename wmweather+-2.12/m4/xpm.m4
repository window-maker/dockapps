dnl AC_FIND_XPM
dnl ---------------
dnl
dnl Find Xpm libraries and headers.
dnl Put Xpm include directory in xpm_includes,
dnl put Xpm library directory in xpm_libraries,
dnl and add appropriate flags to X_CFLAGS and X_LIBS.
dnl
dnl
AC_DEFUN([AC_FIND_XPM],
[
AC_REQUIRE([AC_PATH_XTRA])
xpm_includes=
xpm_libraries=
AC_ARG_WITH(xpm,
[  --without-xpm           do not use the Xpm library])
dnl Treat --without-xpm like
dnl --without-xpm-includes --without-xpm-libraries.
if test "$with_xpm" = "no"
then
xpm_includes=no
xpm_libraries=no
fi
AC_ARG_WITH(xpm-includes,
[  --with-xpm-includes=DIR Xpm include files are in DIR],
xpm_includes="$withval")
AC_ARG_WITH(xpm-libraries,
[  --with-xpm-libraries=DIR
                          Xpm libraries are in DIR],
xpm_libraries="$withval")
AC_MSG_CHECKING(for Xpm)
#
#
# Search the include files.  Note that XPM can come in <X11/xpm.h> (as
# in X11R6) or in <xpm.h> if installed locally.
#
if test "$xpm_includes" = ""; then
AC_CACHE_VAL(ice_cv_xpm_includes,
[
ice_xpm_save_LIBS="$LIBS"
ice_xpm_save_CFLAGS="$CFLAGS"
ice_xpm_save_CPPFLAGS="$CPPFLAGS"
ice_xpm_save_LDFLAGS="$LDFLAGS"
#
LIBS="$X_PRE_LIBS -lXpm -lXt -lX11 $X_EXTRA_LIBS $LIBS"
CFLAGS="$X_CFLAGS $CFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"
#
AC_TRY_COMPILE([
#include <X11/xpm.h>
],[int a;],
[
# X11/xpm.h is in the standard search path.
ice_cv_xpm_includes=
],
[
# X11/xpm.h is not in the standard search path.
# Locate it and put its directory in `xpm_includes'
#
# /usr/include/Motif* are used on HP-UX (Motif).
# /usr/include/X11* are used on HP-UX (X and Xaw).
# /usr/dt is used on Solaris (Motif).
# /usr/openwin is used on Solaris (X and Xaw).
# Other directories are just guesses.
for dir in "$x_includes" "${prefix}/include" /usr/include /usr/local/include \
           /usr/include/Motif2.0 /usr/include/Motif1.2 /usr/include/Motif1.1 \
           /usr/include/X11R6 /usr/include/X11R5 /usr/include/X11R4 \
           /usr/dt/include /usr/openwin/include \
           /usr/dt/*/include /opt/*/include /usr/include/Motif* \
           /usr/*/include/X11R6 /usr/*/include/X11R5 /usr/*/include/X11R4 \
	   "${prefix}"/*/include /usr/*/include /usr/local/*/include \
	   "${prefix}"/include/* /usr/include/* /usr/local/include/*; do
if test -f "$dir/X11/xpm.h" || test -f "$dir/xpm.h"; then
ice_cv_xpm_includes="$dir"
break
fi
done
if test "$ice_cv_xpm_includes" = "/usr/include"; then
ice_cv_xpm_includes=
fi
])
#
LIBS="$ice_xpm_save_LIBS"
CFLAGS="$ice_xpm_save_CFLAGS"
CPPFLAGS="$ice_xpm_save_CPPFLAGS"
LDFLAGS="$ice_xpm_save_LDFLAGS"
])
xpm_includes="$ice_cv_xpm_includes"
fi
#
#
# Now for the libraries.
#
if test "$xpm_libraries" = ""; then
AC_CACHE_VAL(ice_cv_xpm_libraries,
[
ice_xpm_save_LIBS="$LIBS"
ice_xpm_save_CFLAGS="$CFLAGS"
ice_xpm_save_CPPFLAGS="$CPPFLAGS"
ice_xpm_save_LDFLAGS="$LDFLAGS"
#
LIBS="$X_PRE_LIBS -lXpm -lXt -lX11 $X_EXTRA_LIBS $LIBS"
CFLAGS="$X_CFLAGS $CFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"
#
#
# We use XtToolkitInitialize() here since it takes no arguments
# and thus also works with a C++ compiler.
AC_TRY_LINK([
#include <X11/Intrinsic.h>
#include <X11/xpm.h>
],[XtToolkitInitialize();],
[
# libxpm.a is in the standard search path.
ice_cv_xpm_libraries=
],
[
# libXpm.a is not in the standard search path.
# Locate it and put its directory in `xpm_libraries'
#
#
# /usr/lib/Motif* are used on HP-UX (Motif).
# /usr/lib/X11* are used on HP-UX (X and Xpm).
# /usr/dt is used on Solaris (Motif).
# /usr/openwin is used on Solaris (X and Xpm).
# Other directories are just guesses.
for dir in "$x_libraries" "${prefix}/lib" /usr/lib /usr/local/lib \
	   /usr/lib/Motif2.0 /usr/lib/Motif1.2 /usr/lib/Motif1.1 \
	   /usr/lib/X11R6 /usr/lib/X11R5 /usr/lib/X11R4 /usr/lib/X11 \
           /usr/dt/lib /usr/openwin/lib \
	   /usr/dt/*/lib /opt/*/lib /usr/lib/Motif* \
	   /usr/*/lib/X11R6 /usr/*/lib/X11R5 /usr/*/lib/X11R4 /usr/*/lib/X11 \
	   "${prefix}"/*/lib /usr/*/lib /usr/local/*/lib \
	   "${prefix}"/lib/* /usr/lib/* /usr/local/lib/*; do
if test -d "$dir" && test "`ls $dir/libXpm.* 2> /dev/null`" != ""; then
ice_cv_xpm_libraries="$dir"
break
fi
done
])
#
LIBS="$ice_xpm_save_LIBS"
CFLAGS="$ice_xpm_save_CFLAGS"
CPPFLAGS="$ice_xpm_save_CPPFLAGS"
LDFLAGS="$ice_xpm_save_LDFLAGS"
])
#
xpm_libraries="$ice_cv_xpm_libraries"
fi
#
# Add Xpm definitions to X flags
#
if test "$xpm_includes" != "" && test "$xpm_includes" != "$x_includes" && test "$xpm_includes" != "no"
then
X_CFLAGS="-I$xpm_includes $X_CFLAGS"
fi
if test "$xpm_libraries" != "" && test "$xpm_libraries" != "$x_libraries" && test "$xpm_libraries" != "no"
then
case "$X_LIBS" in
  *-R\ *) X_LIBS="-L$xpm_libraries -R $xpm_libraries $X_LIBS";;
  *-R*)   X_LIBS="-L$xpm_libraries -R$xpm_libraries $X_LIBS";;
  *)      X_LIBS="-L$xpm_libraries $X_LIBS";;
esac
fi
#
#
xpm_libraries_result="$xpm_libraries"
xpm_includes_result="$xpm_includes"

if test "$xpm_libraries_result" != "no" && test "$xpm_includes_result" != "no"
then AC_DEFINE(HAVE_XPM, 1, "Define if you have libxpm")
     LINK_XPM="-lXpm"
else LINK_XPM=""
fi

AC_SUBST(LINK_XPM)

test "$xpm_libraries_result" = "" && 
  xpm_libraries_result="in default path"
test "$xpm_includes_result" = "" && 
  xpm_includes_result="in default path"
test "$xpm_libraries_result" = "no" && 
  xpm_libraries_result="(none)"
test "$xpm_includes_result" = "no" && 
  xpm_includes_result="(none)"
AC_MSG_RESULT(
  [libraries $xpm_libraries_result, headers $xpm_includes_result])
])dnl
