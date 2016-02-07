bin_PROGRAMS = wmget
wmget_SOURCES = cancel.c config.def configure.c iq.c list.c messages.c \
	request.c retrieve.c server.c usage.c wmget.c wmget.h wmget.xpm \
	dockapp/da_mouse.c dockapp/da_mouse.h dockapp/da_run.c \
	dockapp/da_x.c dockapp/dockapp.h

AM_CFLAGS = $(x11_CFLAGS) $(xext_CFLAGS) $(xpm_CFLAGS) $(libcurl_CFLAGS)
LIBS += $(x11_LIBS) $(xext_LIBS) $(xpm_LIBS) $(libcurl_LIBS)

dist_man_MANS = wmget.1
dist_pkgdata_DATA = wmget-test.pl
