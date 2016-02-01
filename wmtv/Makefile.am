bin_PROGRAMS = wmtv
wmtv_SOURCES = src/channels.h src/wmtv.c src/wmtv-master.xpm \
	src/wmgeneral/wmgeneral.c src/wmgeneral/wmgeneral.h

AM_CFLAGS = $(x11_CFLAGS) $(xext_CFLAGS) $(xpm_CFLAGS) $(xxf86dga_CFLAGS) \
	$(xxf86vm_CFLAGS) $(libv4l1_CFLAGS)
LIBS += $(x11_LIBS) $(xext_LIBS) $(xpm_LIBS) $(xxf86dga_LIBS) $(xxf86vm_LIBS) \
	$(libv4l1_LIBS)

dist_man_MANS = wmtv.1
EXTRA_DIST = wmtvrc.sample
