AC_INIT([wmhdplop], [0.9.11], [wmaker-dev@googlegroups.com])
AC_CONFIG_SRCDIR(wmhdplop.c)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

AC_SUBST(VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

AC_PROG_INSTALL
AC_PROG_CC


AC_DEFINE(ENABLE_HDDTEMP_QUERY,[],[enable queries to hddtemp])
AC_DEFINE(_GNU_SOURCE,[],[uses GNU extensions])


if test x$GCC = xyes; then
  CFLAGS="-O3 -fomit-frame-pointer -ffast-math -Wall -W";
fi

dnl check for X
AC_PATH_X
AC_PATH_XTRA
if test x$no_x = xyes; then
  echo "You need to supply the path to the X headers and libraries with --x-includes=dir and --x-libraries=dir";
  exit 1;
fi;

LIBS="${X_PRE_LIBS} ${X_LIBS} -lX11 -lXext ${LIBS} ${X_EXTRA_LIBS}"
CFLAGS="$CFLAGS $X_CFLAGS"

dnl Checks for library functions.
AC_CHECK_HEADERS(linux/hdreg.h)

PKG_CHECK_MODULES([IMLIB2], [imlib2])

YELLOW="\\033\\1331;33m"
RESET="\\033\\1330m"

dnl Checks for the gkrellm plugin
build_gkhdplop="yes"
AC_ARG_ENABLE(gkrellm,
 [  --enable-gkrellm        build the gkrellm plugin (requires gtk2 and gkrellm developpement files) (Default is yes)
  --disable-gkrellm       do not build the gkrellm plugin],
 [ if   test "x$enableval" = "xno" ; then
	build_gkhdplop="no"
	echo you disabled the gkhdplop gkrellm plugin
   fi
 ], [build_gkhdplop="yes"])

GKHDPLOP=""
if test "x$build_gkhdplop" = "xno"; then
  echo gkhdplop disabled...
else
  echo building with GKHDPLOP, checking for gtk2..
  pkg_modules="gtk+-2.0 >= 2.0.0"
  PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
  GTK2_CFLAGS=$PACKAGE_CFLAGS;
  GTK2_LIBS=$PACKAGE_LIBS;
  AC_SUBST(GTK2_CFLAGS)
  AC_SUBST(GTK2_LIBS)
  SAVECPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CFLAGS $GTK2_CFLAGS"
  AC_CHECK_HEADERS([gkrellm2/gkrellm.h], [ok=1], [ok=0])
  if test "x$ok" = "x0"; then
     echo -e "${YELLOW}gkrellm2/gkrellm.h header not found or unusable${RESET}... cannot build gkhdplop.so"
     build_gkhdplop="no"
  else
    GKHDPLOP="gkhdplop.so"
  fi
  CPPFLAGS="$SAVECPPFLAGS"
fi
AC_SUBST(GKHDPLOP)


AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo "- - - - - - - - - - - - - - - -"
echo -e "${YELLOW}wmhdplop    will be built${RESET}"
if test "x$build_gkhdplop" = "xno"; then
  echo -e "${YELLOW}gkhdplop.so will NOT be built${RESET}"
else
  echo -e "${YELLOW}gkhdplop.so will be built${RESET}"
  echo "Check that the plugin is working properly before installing it with:"
  echo "   gkrellm -p ./gkhdplop.so"
  echo "and then install the gkrellm plugin with:"
  echo "     mv ./gkhdplop.so ~/.gkrellm2/plugins"
fi
