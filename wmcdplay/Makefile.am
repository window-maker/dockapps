bin_PROGRAMS = wmcdplay
wmcdplay_SOURCES = cdctl.h cdctl_freebsd.h wmcdplay.cc XPM/standard.art
BUILT_SOURCES = XPM/standard.art
AM_CFLAGS = $(x11_CFLAGS) $(xpm_CFLAGS) $(xext_CFLAGS)
AM_CPPFLAGS = -DSYSARTDIR=\"$(pkgdatadir)\"
LIBS += $(x11_LIBS) $(xpm_LIBS) $(xext_LIBS)

ART = XPM/bluered.art XPM/grey.art XPM/newstyle.art XPM/wmcd.art
pkgdata_DATA = $(ART)
pkgdata_SCRIPTS = XPM/makeart
CLEANFILES = $(ART) XPM/standard.art

XPM/%.art:
	cd XPM && ./makeart $*

XPM/standard.art: XPM/@defaultart@.art
	cd XPM && ln -s @defaultart@.art standard.art
