bin_PROGRAMS = wmcdplay
wmcdplay_SOURCES = cdctl.h cdctl_freebsd.h wmcdplay.cc
AM_CFLAGS = $(x11_CFLAGS) $(xpm_CFLAGS) $(xext_CFLAGS)
AM_CPPFLAGS = -DSYSARTDIR=\"$(pkgdatadir)\"
LIBS += $(x11_LIBS) $(xpm_LIBS) $(xext_LIBS)
pkgdata_DATA = XPM/bluered.art XPM/grey.art XPM/newstyle.art XPM/wmcd.art
