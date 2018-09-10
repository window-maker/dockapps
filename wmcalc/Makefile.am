bin_PROGRAMS = wmcalc
wmcalc_SOURCES = readln.c wmcalc.c wmcalcfunc.c wmcalcswitch.c \
	wmcalc_c.h wmcalc_err.h wmcalc_f.h wmcalc_g.h wmcalc_t.h wmcalc_x.h \
	backdrop.xpm calcbuttons.xpm charmap.xpm mask.xbm
dist_man_MANS = wmcalc.1
dist_sysconf_DATA = wmcalc.conf

AM_CPPFLAGS = -DCONF=\"@sysconfdir@\"
AM_CFLAGS = $(X11_CFLAGS) $(XEXT_CFLAGS) $(XPM_CFLAGS)
LIBS += $(X11_LIBS) $(XEXT_LIBS) $(XPM_LIBS)

icondir = @datadir@/icons/hicolor/48x48/apps
dist_icon_DATA = wmcalc.png
