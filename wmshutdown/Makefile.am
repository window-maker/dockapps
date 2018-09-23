bin_PROGRAMS = wmshutdown
wmshutdown_SOURCES = wmshutdown.c wmshutdown.xpm

AM_CFLAGS = $(gtk_CFLAGS) $(x11_CFLAGS)
AM_CPPFLAGS =  -DDATADIR=\"$(pkgdatadir)\"
LIBS += $(gtk_LIBS) $(x11_LIBS)

dist_man_MANS = wmshutdown.1

icondir = @datadir@/icons/hicolor/48x48/apps
dist_icon_DATA = wmshutdown.png

desktopdir = @datadir@/applications
dist_desktop_DATA = wmshutdown.desktop
