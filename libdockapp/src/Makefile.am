
AUTOMAKE_OPTIONS = no-dependencies

lib_LTLIBRARIES = libdockapp.la

libdockapp_la_LDFLAGS = -version-info 2:0:0 @X_LIBS@

include_HEADERS = dockapp.h

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
	dautil.c

# Include these in a distribution, but don't install
noinst_HEADERS = daargs.h dautil.h

AM_CFLAGS = $(X11_CFLAGS) $(Xext_CFLAGS) $(xpm_CFLAGS)
AM_CPPFLAGS = @X_CFLAGS@ @CPPFLAGS@ @DFLAGS@
LIBS += $(X11_LIBS) $(Xext_LIBS) $(xpm_LIBS)

LIBLIST = libdockapp.la @XLIBS@
