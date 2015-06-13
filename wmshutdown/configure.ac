AC_INIT([wmshutdown], [1.4], [wmaker-dev@lists.windowmaker.org])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADER([config.h])

AC_ARG_WITH([consolekit],
	[AS_HELP_STRING([--with-consolekit],
		[use ConsoleKit instead of logind])],
	[AC_DEFINE([CONSOLEKIT], 1,
		[Define to 1 if ConsoleKit is used instead of logind.])
	AC_MSG_NOTICE([use ConsoleKit])
	],
	[AC_MSG_NOTICE([use logind])])

AC_PROG_CC
PKG_CHECK_MODULES([gtk],[gtk+-3.0])
PKG_CHECK_MODULES([x11],[x11])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
