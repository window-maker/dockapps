bin_PROGRAMS = wmgtemp
wmgtemp_SOURCES = src/wmgtemp.c src/wmgtemp-interface-mask.xbm \
	src/wmgtemp-interface.xpm
dist_man_MANS = wmgtemp.1

AM_CFLAGS = $(X11_CFLAGS) $(dockapp_CFLAGS)
LIBS += $(X11_LIBS) $(dockapp_LIBS)

EXTRA_DIST = Artistic BUGS CREDITS examples/wmgtemprc
