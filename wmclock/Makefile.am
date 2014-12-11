bin_PROGRAMS = wmclock
wmclock_SOURCES = dynlist.c dynlist.h wmclock.c \
	xpm/date.xpm xpm/led.xpm xpm/mask.xbm xpm/mask.xbm
AM_CFLAGS = $(x11_CFLAGS) $(xpm_CFLAGS) $(xext_CFLAGS)
LIBS += $(x11_LIBS) $(xpm_LIBS) $(xext_LIBS)
dist_man_MANS = wmclock.1
nobase_dist_pkgdata_DATA = lang.*/*.xpm
