bin_PROGRAMS = wmusic
wmusic_SOURCES = src/wmusic.c src/wmusic-digits.xpm src/wmusic-master.xpm
dist_man_MANS = wmusic.1

AM_CFLAGS = $(X11_CFLAGS) $(DOCKAPP_CFLAGS) $(PLAYERCTL_CFLAGS)
LIBS += $(X11_LIBS) $(DOCKAPP_LIBS) $(PLAYERCTL_LIBS)

desktopdir = @datadir@/applications
dist_desktop_DATA = wmusic.desktop
