AC_INIT([wmix], [3.4], [wmaker-dev@googlegroups.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([wmix.c])

AC_PROG_CC

AC_CHECK_LIB([asound], [snd_mixer_selem_get_playback_switch])
AC_CHECK_LIB([m], [cos])
AC_CHECK_LIB([X11], [XOpenDisplay])
AC_CHECK_LIB([Xext], [XShapeCombineMask])
AC_CHECK_LIB([Xpm], [XpmCreatePixmapFromData])
AC_CHECK_LIB([Xrandr], [XRRQueryExtension])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
