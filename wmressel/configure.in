dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/wmressel.c)

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC

dnl Checks for X.
#AC_PATH_X
AC_PATH_XTRA

dnl Checks for libraries.

#AC_CHECK_LIB(Xpm, XpmCreatePixmapFromData, X_LIBS="$X_LIBS -lXpm", AC_MSG_ERROR("No valid Xpm library found !"), $X_LIBS -lX11)
AC_CHECK_LIB(Xxf86vm,main, X_LIBS="$X_LIBS -lXext -lXxf86vm", AC_MSG_ERROR("No valid Xxf86vm library found !"), $X_LIBS -lX11 -lXext)
AC_CHECK_LIB(Xinerama,main, , , $X_LIBS -lX11)

#AC_CHECK_LIB(gdk,main, X_LIBS="$X_LIBS -lgdk", , $X_LIBS)
#AC_CHECK_LIB(gtk,main, X_LIBS="$X_LIBS -lgtk", , $X_LIBS)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER(sys/types.h, AC_DEFINE(HAVE_SYS_TYPES_H))
AC_CHECK_HEADER(X11/extensions/xf86vmode.h, , AC_MSG_ERROR("C header file not found !"))
AC_CHECK_HEADER(X11/extensions/shape.h, , AC_MSG_ERROR("C header file not found !"))
AC_CHECK_HEADER(X11/xpm.h, X_LIBS="$X_LIBS -lXpm", AC_MSG_ERROR("C header file not found !"))
AC_CHECK_HEADER(X11/extensions/Xinerama.h)
#AC_CHECK_HEADER(gtk/gtk.h)

AC_PATH_PROG(GTK_CONFIG,gtk-config)
if test -z "$GTK_CONFIG"
then
	AC_MSG_ERROR("You need to install the gtk-devel package before compiling: http://www.gtk.org/")
else
	GTK_CFLAGS=`$GTK_CONFIG --cflags`
	GTK_LIBS=`$GTK_CONFIG --libs`
	X_LIBS="$X_LIBS -lgdk -lgtk"
	AC_SUBST(GTK_CFLAGS)
#	AC_SUBST(GTK_LIBS)
fi


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T

dnl Checks for library functions.

#AC_SUBST(LIBS)
#AC_SUBST(DEFS)
AC_SUBST(X_CLAGS)
AC_SUBST(X_LIBS)
AC_SUBST(X_EXTRA_LIBS)
AC_SUBST(X_PRE_LIBS)

AC_OUTPUT(Makefile src/Makefile)
