bin_PROGRAMS = wmifs
dist_man_MANS = wmifs.1
EXTRA_DIST = BUGS CHANGES HINTS sample.wmifsrc wmifs-mask.xbm wmifs-master.xpm

AM_CPPFLAGS = -DCONF=\"@sysconfdir@\"
AM_CFLAGS = $(x11_CFLAGS) $(xpm_CFLAGS) $(dockapp_CFLAGS)
LIBS += $(x11_LIBS) $(xpm_LIBS) $(dockapp_LIBS)
