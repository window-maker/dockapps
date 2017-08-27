bin_PROGRAMS = wmressel
wmressel_SOURCES = src/wmressel.c src/wmressel-mask.xbm src/wmressel-master.xpm

dist_man_MANS = wmressel.1

EXTRA_DIST = BUGS HINTS

AM_CFLAGS = $(Xlib_CFLAGS) $(libXpm_CFLAGS) $(Xext_CFLAGS) $(Xxf86vm_CFLAGS) \
	$(Xinerama_CFLAGS) $(GTK_CFLAGS)
LIBS += $(Xlib_LIBS) $(libXpm_LIBS) $(Xext_LIBS)  $(Xxf86vm_LIBS) \
	$(Xinerama_LIBS) $(GTK_LIBS)
