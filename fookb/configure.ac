dnl Process this file with autoconf to produce a configure script.
dnl
dnl (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
dnl
dnl vim:ts=8:sts=2:shiftwidth=2:expandtab:foldmethod=marker

dnl {{{1
AC_INIT(fookb.c)

dnl Checks for programs.
AC_PROG_CC

WUTIL_LIB="-lWUtil"
AC_SUBST(WUTIL_LIB)
dnl }}}1

dnl Checks for libraries. {{{1

dnl Basic checks: X compilation settings and essential libraries {{{2

AC_PATH_XTRA
dnl Adds C compiler flags to X_CFLAGS and X linker flags to X_CFLAGS
dnl Extra libraries will be in X_EXTRA_LIBS
dnl Libraries wich should be linked before -lX11 will be in X_PRE_LIBS
dnl We do not check X_DISPLAY_MISSING because we need to check for
dnl XkbOpenDisplay in libX11 anyway.

AC_CHECK_LIB(
  [X11],
  [XkbOpenDisplay],
  ,
  [AC_MSG_ERROR([[Fatal: no libX11 or no XkbOpenDisplay() in libX11]])],
  $X_LIBS
)

AC_CHECK_LIB(
  [Xpm],
  [XpmReadFileToImage],
  ,
  [AC_MSG_ERROR([[Fatal: no libXpm found!]])],
  $X_LIBS
)

dnl }}}2

dnl --enable/disable libWUtil (property lists handling) {{{2

AC_ARG_ENABLE(
  [libWUtil],
  [  --enable-libWUtil       try to use property lists handling in libWUtil],
  [
    if test $enable_libWUtil = yes ; then

dnl   enabled: libWUtil and WINGs/WUtil.h check {{{3    
      AC_CHECK_LIB(
        [WUtil],
        [WMReadPropListFromFile],
        [
          dnl library is found; let's check header file {{{4
          AC_CHECK_HEADERS(
            [WINGs/WUtil.h],
            ,
            [
              AC_MSG_WARN([[No WINGs/WUtil.h found -- cannot compile version with property lists support. You should use X resources for customizing fookb; or, if you have libproplist, use older version of fookb]])
              WUTIL_LIB=""
            ]
          )
          dnl }}}4
        ],
        [
          AC_MSG_WARN([[No libWUtil found or old libWUtil version. You should use X resources for setting up fookb. If you have libproplist, use older version of fookb]])
          WUTIL_LIB=""
        ]
        ,
        $X_LIBS
      )
      dnl That's EVEN BETTER than LISP!!!
dnl }}}3
    else
dnl if --disable-libWUtil is given we just do almost nothing
      UTIL_LIB=""
    fi
  ],
  [
dnl neither --enable nor --disable-libWUtil is not given.
dnl By default we will try to search for libWUtil {{{3
    AC_CHECK_LIB(
      [WUtil],
      [WMReadPropListFromFile],
      [
        dnl library is found; let's check header file {{{4
        AC_CHECK_HEADERS(
          [WINGs/WUtil.h],
          ,
          [
            AC_MSG_WARN([[No WINGs/WUtil.h found -- cannot compile version with property lists support. You should use X resources for customizing fookb; or, if you have libproplist, use older version of fookb]])
            WUTIL_LIB=""
          ]
        )
        dnl }}}4
      ],
      [
        AC_MSG_WARN([[No libWUtil found or old libWUtil version. You should use X resources for setting up fookb. If you have libproplist, use older version of fookb]])
        WUTIL_LIB=""
      ]
      ,
      $X_LIBS
    )
dnl }}}3
  ]
)
 
dnl }}}2

dnl }}}1

AC_ARG_ENABLE(
  [wmaker],
  [  --enable-wmaker         create windowmaker dockable application],
  [ 
    if test $enable_wmaker = yes ; then
      AC_DEFINE(WMAKER)
    fi
  ],
  [
    AC_CHECK_PROG(HAVE_WMAKER, wmaker, yes, no)
  
    if test $HAVE_WMAKER = yes ; then
      AC_DEFINE(WMAKER)
    fi
  ]
)

AC_OUTPUT(Makefile)
