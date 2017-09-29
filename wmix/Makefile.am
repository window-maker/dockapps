bin_PROGRAMS = wmix
wmix_SOURCES = misc.c config.c mixer-alsa.c mixer-oss.c ui_x.c mmkeys.c wmix.c \
	include/common.h include/config.h include/led-off.xpm \
	include/led-on.xpm include/master.xpm include/misc.h \
	include/mixer-alsa.h include/mixer.h include/mixer-oss.h \
	include/mmkeys.h include/ui_x.h

dist_man_MANS = wmix.1x
EXTRA_DIST = BUGS sample.wmixrc

desktopdir = @datadir@/applications
dist_desktop_DATA = wmix.desktop
