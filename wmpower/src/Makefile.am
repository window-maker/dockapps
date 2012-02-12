## Makefile.am for wmpower/src

SUBDIRS = dockapp power_management

INCLUDES = -Idockapp -Ipixmaps -Ipower_management

bin_PROGRAMS = wmpower
wmpower_SOURCES = \
		wmpower.c \
		open_syslog_on_stderr.c  \
		open_syslog_on_stderr.h

wmpower_LDADD   = dockapp/libdockapp.a power_management/libpower.a -lpthread -lXpm -lX11 -lXext
wmpower_LDFLAGS = -L/usr/X11R6/lib

EXTRA_DIST = \
		pixmaps/wmpower_mask.xbm \
		pixmaps/wmpower_master.xpm \
		pixmaps/wmpower_master_LowColor.xpm
