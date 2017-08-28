bin_PROGRAMS = wmmixer
dist_man_MANS = wmmixer.1
wmmixer_SOURCES = common.h exception.cc exception.h mixctl.cc mixctl.h \
	wmmixer.cc wmmixer.h xhandler.cc xhandler.h \
	XPM/icons.xpm XPM/norec.xpm XPM/tile.xpm XPM/wmmixer.xpm
EXTRA_DIST = CHANGES

AM_CFLAGS = $(Xlib_CFLAGS) $(Xext_CFLAGS) $(libXpm_CFLAGS)
LIBS += $(Xlib_LIBS) $(Xext_LIBS) $(libXpm_LIBS)
