SUBDIRS = icons
bin_PROGRAMS = wmbutton
dist_man_MANS = wmbutton.1
dist_sysconf_DATA = wmbutton.conf
wmbutton_SOURCES = backdrop.xpm buttons.xpm mask.xbm wmb_libs.c wmbutton.c \
	wmbutton.h

AM_CFLAGS = $(x11_CFLAGS) $(xpm_CFLAGS) $(xext_CFLAGS)
LIBS += $(x11_LIBS) $(xpm_LIBS) $(xext_LIBS)
