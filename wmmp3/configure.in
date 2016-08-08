AC_INIT(main.c)
AM_INIT_AUTOMAKE(wmmp3,0.12)
AC_PROG_CC
AC_PROG_INSTALL
AC_PATH_XTRA

if test "$x_includes" != "NONE"; then
        CFLAGS="$CFLAGS -I$x_includes"
fi

if test "$x_libraries" != "NONE"; then
        LDFLAGS="$LDFLAGS -L$x_libraries"
fi

AC_CHECK_LIB(X11, XOpenDisplay)
AC_CHECK_LIB(Xext, XShapeCombineMask)
AC_CHECK_LIB(Xpm, XpmCreatePixmapFromData, , , $LIBS)

AC_OUTPUT(Makefile)
