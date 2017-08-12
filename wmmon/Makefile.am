bin_PROGRAMS = wmmon
dist_man_MANS = wmmon.1
wmmon_SOURCES = wmmon.c wmmon-mask.xbm wmmon-master.xpm ulllib.c ulllib.h
EXTRA_DIST = BUGS CHANGES HINTS

AM_CFLAGS = $(x11_CFLAGS) $(dockapp_CFLAGS)
LIBS += $(x11_LIBS) $(dockapp_LIBS)

desktopdir = $(datadir)/applications
dist_desktop_DATA = wmmon.desktop
