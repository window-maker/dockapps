bin_PROGRAMS = wmmoonclock
dist_man_MANS = src/wmMoonClock.1
wmmoonclock_SOURCES = src/CalcEphem.c src/CalcEphem.h src/Moon.c src/Moon.h \
	src/MoonRise.c src/MoonRise.h src/wmMoonClock.c \
	src/wmMoonClock_mask.xbm \
	src/wmMoonClock_masterLow.xpm \
	src/wmMoonClock_master.xpm \
	src/xutils.c src/xutils.h
EXTRA_DIST = BUGS

AM_CFLAGS = $(x11_CFLAGS) $(xext_CFLAGS) $(xpm_CFLAGS)
LIBS += $(x11_LIBS) $(xext_LIBS) $(xpm_LIBS) -lm

desktopdir = $(datadir)/applications
dist_desktop_DATA = wmmoonclock.desktop
