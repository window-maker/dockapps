AC_INIT([wmcdplay], [1.0beta1])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([configure.ac])
AC_PROG_CXX

define([artlist], esyscmd([cd XPM && ls -d */ | sed 's/\///g']))
AC_ARG_WITH([art],
	AS_HELP_STRING([--with-art],
	[Choose art style.  Defaults to 'newstyle'.  Choices are: ]
	artlist),
	[],
        [with_art=newstyle])
if test ! -d "XPM/${with_art}"; then
   AC_MSG_ERROR([cannot find art directory '${with_art}'.])
fi
AC_MSG_NOTICE([setting art style to '${with_art}'...])
AC_SUBST([defaultart], [$with_art])

PKG_CHECK_MODULES([x11], [x11])
PKG_CHECK_MODULES([xpm], [xpm])
PKG_CHECK_MODULES([xext], [xext])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
