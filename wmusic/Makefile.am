bin_PROGRAMS = wmusic
wmusic_SOURCES = src/wmusic.c src/wmusic-digits.xpm src/wmusic-master.xpm
dist_man_MANS = wmusic.1

AM_CFLAGS = $(X11_CFLAGS) $(DOCKAPP_CFLAGS) $(PLAYERCTL_CFLAGS) $(GDK_PIXBUF_CFLAGS)
LIBS += $(X11_LIBS) $(DOCKAPP_LIBS) $(PLAYERCTL_LIBS) $(GDK_PIXBUF_LIBS)

desktopdir = @datadir@/applications
dist_desktop_DATA = wmusic.desktop
