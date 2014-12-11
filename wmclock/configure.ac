AC_INIT([wmclock], [1.0.14])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_SRCDIR([configure.ac])
AC_PROG_CC

define([langlist], esyscmd([ls -d lang.* | sed 's/lang.//']))
AC_ARG_WITH([lang],
	AS_HELP_STRING([--with-lang],
	[Configure wmclock to use day and month names for the specified
	language.  Defaults to 'english'.  Choices are: ]
	langlist),
	[],
        [with_lang=english])

LangDir="lang.${with_lang}"
if test ! -d "${LangDir}"; then
   AC_MSG_ERROR([Cannot find language directory '${LangDir}'.])
fi

echo "configuring for ${with_lang} day-of-week and month names..."
rm -f month.xpm weekday.xpm
ln -s "./${LangDir}/month.xpm" .
ln -s "./${LangDir}/weekday.xpm" .

PKG_CHECK_MODULES([x11], [x11])
PKG_CHECK_MODULES([xpm], [xpm])
PKG_CHECK_MODULES([xext], [xext])
AC_CONFIG_FILES([Makefile wmclock.1])
AC_OUTPUT
