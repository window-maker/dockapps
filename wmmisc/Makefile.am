bin_PROGRAMS = wmmisc
wmmisc_SOURCES = src/dockapp_draw.c src/dockapp_draw.h \
	src/dockapp_main.c src/dockapp_main.h \
	src/dockapp_utils.c src/dockapp_utils.h \
	src/wmmisc-master.xpm
dist_man_MANS = wmmisc.1

AM_CFLAGS = $(DOCKAPP_CFLAGS)
LIBS += $(DOCKAPP_LIBS)

desktopdir = $(datadir)/applications
dist_desktop_DATA = wmmisc.desktop
