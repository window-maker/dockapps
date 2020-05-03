
AUTOMAKE_OPTIONS = no-dependencies

lib_LTLIBRARIES = libdockapp.la

libdockapp_la_LDFLAGS = -version-info 3:1:0

otherincludedir = $(includedir)/libdockapp
otherinclude_HEADERS = dockapp.h \
		  wmgeneral.h \
		  list.h \
		  misc.h

libdockapp_la_SOURCES = \
	dockapp.h \
	daargs.c\
	dacallback.c \
	dacolor.c \
	daevent.c \
	damain.c \
	dapixmap.c \
	darect.c \
	dashaped.c \
	dautil.c \
	list.c \
	misc.c \
	wmgeneral.c

# Include these in a distribution, but don't install
noinst_HEADERS = daargs.h dautil.h

AM_CFLAGS = $(X11_CFLAGS) $(Xext_CFLAGS) $(xpm_CFLAGS)
AM_CPPFLAGS = @CPPFLAGS@ @DFLAGS@
LIBS += $(X11_LIBS) $(Xext_LIBS) $(xpm_LIBS)

LIBLIST = libdockapp.la @XLIBS@
