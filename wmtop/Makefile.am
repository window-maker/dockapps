bin_PROGRAMS = wmtop
wmtop_SOURCES = wmtop.c xpm/wmtop-rainbow.xpm xpm/wmtop-neon1.xpm \
	xpm/wmtop-default.xpm xpm/wmtop-neon2.xpm xpm/wmtop-lcd.xpm

AM_CFLAGS = $(x11_CFLAGS) $(xext_CFLAGS) $(xpm_CFLAGS) $(dockapp_CFLAGS)
LIBS += $(x11_LIBS) $(xext_LIBS) $(xpm_LIBS) $(dockapp_LIBS)

dist_man_MANS = wmtop.1
EXTRA_DIST = BUGS
