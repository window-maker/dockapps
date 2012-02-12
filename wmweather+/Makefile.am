AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = b0rken m4 wmgeneral

bin_PROGRAMS = wmweather+

wmweather__SOURCES = animation.c animation.h avn.c avn.h convert.c convert.h \
	die.c die.h diff.c diff.h dock.c dock.h download.c download.h \
        eta.c eta.h font.c font.h forecast.c forecast.h getLine.c getLine.h \
        metar.c metar.h moon.c moon.h mrf.c mrf.h radar.c radar.h subst.c \
        subst.h sunzenith.c sunzenith.h warnings.c warnings.h wmweather+.c \
        wmweather+.h characters.xpm wmweather_master.xpm
wmweather__LDADD = wmgeneral/libwmgeneral.a wmgeneral/libwmgeneral-x11.a b0rken/libb0rken.a @XLIBS@

man_MANS = wmweather+.1
EXTRA_DIST = HINTS bootstrap example.conf $(man_MANS)

reallyclean: distclean
	-rm -rf autom4te.cache
	-rm Makefile.in */Makefile.in aclocal.m4 config.h.in* config.guess config.sub configure install-sh ltmain.sh missing mkinstalldirs stamp-h.in
